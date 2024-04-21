# ----------------------------------------------------------------------
#
# File: quant.py
#
# Last edited: 30.01.2024
#
# Copyright (C) 2024, ETH Zurich and University of Bologna.
#
# Author: Moritz Scherer, ETH Zurich
#         Victor Jung, ETH Zurich
#
# ----------------------------------------------------------------------
# SPDX-License-Identifier: Apache-2.0
#
# Licensed under the Apache License, Version 2.0 (the License); you may
# not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an AS IS BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
from typing import Iterable

import builtins
import argparse
import builtins
import torch
import numpy as np
from transformers.utils.dummy_pt_objects import BridgeTowerForMaskedLM

from torch.fx import Interpreter, Node, GraphModule

from pactIntegerizationProto.pactify import *
from pactIntegerizationProto.float2fake import *

from quantlib.editing.fx.util.tracing import LeafTracer, custom_symbolic_trace
from quantlib.editing.fx.passes import SequentialPass
import quantlib.editing.fx.passes.pact as passes
from quantlib.algorithms.pact.pact_ops import PACTHardswish, RequantShift
from quantlib.algorithms.generic import Multiply
from quantlib.editing.fx.passes.pact.pact_util import PACT_OPS
from quantlib.editing.fx.passes.pact import ApproximateSiLUWithGELUPass, \
                                            CanonicalizeRMSNormPass, \
                                            ApproximateSoftmaxPass, \
                                            InsertActivationsAfterLinearsPass, \
                                            MatmulReplacementPass, \
                                            TruedivReplacementPass, \
                                            AddTreeReplacementPass, \
                                            MulReplacementPass, \
                                            ConcatTreeReplacementPass, \
                                            AnnotateEpsPass

from QuantGraphTools.utils import foldConstant, delistifyInputs, foldConstantWithReqauntShift

from basicModelSetup import generateAttentionMask, \
                            generateKVCache, \
                            generateRotationMatrix, \
                            setupModels, \
                            formatInternetLlamaModelOutput, \
                            testModelOutputIsClose
from modelCoreSetup import SingleStepLayerStack, TokenEmbedder

from llamaModel import LlamaRMSNorm
from llamaTracer import Llama_PACT_symbolic_trace, LLAMA_PACT_OPS, LLAMA_OPS


_NLEVELSACTS = 2**8
_NLEVELSWEIGHTS = 2**8
_UPPERPERCENTILE = 99.9
_LOWERPERCENTILE = 0.1
_EPOCHS = 5

_ActQuantArgs = {
    'n_levels': _NLEVELSACTS, 'act_kind':'identity',
    'init_clip': "percentile", 'learn_clip':True,
    'symm':True, 'leaky':0.0, 'rounding':True,
    'upper_percentile': _UPPERPERCENTILE,
    'lower_percentile':_LOWERPERCENTILE,
    'tqt':True, "num_bins": 2**12
}

_IntegerQuantArgs = copy.deepcopy(_ActQuantArgs)
_IntegerQuantArgs['tqt'] = False
_IntegerQuantArgs['learn_clip'] = False

_PactifyQuantArgs = copy.deepcopy(_ActQuantArgs)

_LinQuantArgs = {
    'n_levels':_NLEVELSWEIGHTS,
    'init_clip':'max', 'learn_clip':True,
    'symm_wts':True,  'rounding':True,
    'quantize': 'per_channel',
    'tqt':True
}

_LinearQuantArgs = copy.deepcopy(_LinQuantArgs)
_LinearQuantArgs['quantize'] = 'per_layer'

def fakeTrain(model, fakeBatch, epoch, quantControllers=[], scheduler=None, device="cpu"):
    model.train()

    for ctrlr in quantControllers:
        ctrlr.step_pre_training_batch(epoch, optimizer)

    optimizer.zero_grad()
    outputs = model(*fakeBatch)


def fakeValidate(model, fakeBatch, epoch, scheduler=None, device="cpu"):
    model.eval()
    outputs = model(*fakeBatch)

    if scheduler is not None:
        scheduler.step()

def _getAbsMinAbsMax(tensor, n_levels = _NLEVELSACTS):
    _max = tensor.max()
    _min = tensor.min()

    if _max == 0 and _min == 0:
        _max = 1

    absMax = max(_max, torch.abs(_min))

    if min == 0:
        absMin = 0
    else:
        absMin = - absMax/((n_levels//2)-1)*(n_levels//2)

    return absMin, absMax

def _getAdhocEpsList(n_levels : int = _NLEVELSACTS, *inputTensors) -> List[float]:

    epsList = []

    for tensor in inputTensors:

        absMin, absMax = _getAbsMinAbsMax(tensor)

        eps = torch.tensor((absMax - absMin)/(n_levels-1))
        epsList.append(eps)

    return epsList


def matchSizeNode(node):
    return node.op == "call_method" and node.target == "size"

def matchShapeNode(node):
    return node.op == "call_function" and node.target == builtins.getattr

def matchConstantRequantShift(gm: GraphModule, node):
    # Match requantshift nodes with constant inputs
    if node.op == "call_module":
        return np.all([
            isinstance(gm.get_submodule(node.target), RequantShift),
            len(node.args) == 1,
            len(node.args[0].users) == 1,
            node.args[0].op == "placeholder",
        ])
    return False

def roundTensors(fakeBatch, eps_in):

    allRounded = []

    for tensor, eps in zip(fakeBatch, eps_in):

        absMin, absMax = _getAbsMinAbsMax(tensor)

        rounded = torch.trunc(torch.clamp(tensor, min=absMin, max=absMax) / eps) * eps
        allRounded.append(rounded)

    return allRounded

class LinActPass(InsertActivationsAfterLinearsPass):
    before_modules = (
        *(InsertActivationsAfterLinearsPass.before_modules),
        PACTHardswish,
        Multiply
    )

class LlamaCanonAndApprox(SequentialPass):
    def __init__(self):
        _passes = []
        _passes.append(CanonicalizeRMSNormPass(custom_trace=Llama_PACT_symbolic_trace, custom_module=LlamaRMSNorm(1)))
        _passes.append(PactifyPass(_NLEVELSACTS, _NLEVELSWEIGHTS, _PactifyQuantArgs, _LinQuantArgs, _LinearQuantArgs))
        _passes.append(ApproximateSiLUWithGELUPass(custom_trace=Llama_PACT_symbolic_trace))
        _passes.append(ApproximateSoftmaxPass(custom_trace=Llama_PACT_symbolic_trace, mode="I-BERT"))
        _passes.append(MatmulReplacementPass())
        _passes.append(TruedivReplacementPass())
        _passes.append(AddTreeReplacementPass(**_ActQuantArgs))
        _passes.append(MulReplacementPass())
        _passes.append(ConcatTreeReplacementPass(**_IntegerQuantArgs))
        _passes.append(LinActPass(**_ActQuantArgs))
        super().__init__(*_passes, name_prefix="_LLAMA_APPROXIMATE_PASS")


if __name__ == "__main__":

    parser = argparse.ArgumentParser(description="Description of your program")
    parser.add_argument("--output_prefix", type=str, default="llama")
    parser.add_argument("--step", type=int, default=1)
    args = parser.parse_args()

    love_input = torch.tensor([[5360]], dtype=torch.int) # id of the word "love" in the vocab
    step = args.step
    onnx_filename = f"{args.output_prefix}_{args.step}.onnx"

    goldenModel, traceableModel = setupModels()

    embedder = TokenEmbedder(traceableModel)
    layerStack = SingleStepLayerStack(traceableModel, goldenModel.config)
    tracedModel = Llama_PACT_symbolic_trace(root=layerStack)

    input_embeds = embedder(love_input)
    kvCache = generateKVCache(tracedModel, goldenModel, step, input_embeds)
    attention_mask = generateAttentionMask(input_embeds, kvCache)
    rotation_matrix = generateRotationMatrix(goldenModel.config.hidden_size // goldenModel.config.num_attention_heads, step)

    unfoldedKVCache = []
    for entry in kvCache:
        unfoldedKVCache += [entry[0], entry[1]]

    fakeBatch = [input_embeds, attention_mask, rotation_matrix, *unfoldedKVCache]
    eps_in = tuple(_getAdhocEpsList(_NLEVELSACTS, *fakeBatch))
    roundedFakeBatch = roundTensors(fakeBatch, eps_in)

    roundedkvCache = []
    for idx, entry in enumerate(roundedFakeBatch[3:]):
        if idx % 2:
            roundedkvCache[-1].append(entry)
        else:
            roundedkvCache.append([entry])

    foldConstant(tracedModel, matchSizeNode, input_embeds, attention_mask, rotation_matrix, kvCache, None)
    foldConstant(tracedModel, matchShapeNode, input_embeds, attention_mask, rotation_matrix, kvCache, None)
    delistifyInputs(tracedModel)

    output = tracedModel(*roundedFakeBatch)
    goldenOutput = goldenModel(inputs_embeds=roundedFakeBatch[0], attention_mask=roundedFakeBatch[1], past_key_values=roundedkvCache)

    formattedGoldenOutput = formatInternetLlamaModelOutput(goldenOutput)
    testModelOutputIsClose(formattedGoldenOutput, output, tolerance=1e-1)

    llamaPass = LlamaCanonAndApprox()
    tracedModel = llamaPass.apply(tracedModel)

    outputTraced = tracedModel(*roundedFakeBatch)
    print(f"MAE pre PQT: {torch.abs(goldenOutput[0] - outputTraced[0]).mean()}")

    linop_list = [i for i in tracedModel.modules() if isinstance(i, qa.pact._PACTLinOp)]
    act_list = [i for i in tracedModel.modules() if isinstance(i, qa.pact._PACTActivation)]
    adder_list = [i for i in tracedModel.modules() if isinstance(i, tuple(util.PACT_OPS_INT))]
    eps_list = [i for i in tracedModel.modules() if isinstance(i, qa.pact._PACTEps)]

    # SCHEREMO: First fix acts and linears, then fix epses

    _AnnotateEpsPass = AnnotateEpsPass(eps_in, n_levels_in=_NLEVELSACTS)

    schedule = {1: "start", (_EPOCHS-2): ["freeze"]}
    actSchedule = {1: "start", (_EPOCHS-2): ["freeze"]}
    epsSchedule = {(_EPOCHS-3): 'start'}

    actController = qa.pact.PACTActController(act_list, actSchedule, init_clip_hi=6., init_clip_lo=-6.)
    linearController = qa.pact.PACTLinearController(linop_list, schedule, init_clip_hi=16., init_clip_lo=-16.)
    integerController = qa.pact.PACTIntegerModulesController(adder_list)
    epsController = qa.pact.PACTEpsController(tracedModel, eps_list, epsSchedule, LeafTracer(leaf_types=list(LLAMA_PACT_OPS)), _AnnotateEpsPass, _NLEVELSACTS)

    quantControllers = [actController, linearController, integerController, epsController]

    optimizer = torch.optim.Adam(tracedModel.parameters(), lr=0)

    fakeTrain(tracedModel, roundedFakeBatch, 0, [])

    for epoch in range(_EPOCHS):

        for ctrlr in quantControllers:
            ctrlr.step_pre_training_epoch(epoch, optimizer)

        tracedModel.train()
        fakeTrain(tracedModel, roundedFakeBatch, epoch, quantControllers)

        for ctrlr in quantControllers:
            ctrlr.step_pre_validation_epoch(epoch)

        tracedModel.eval()
        fakeValidate(tracedModel, roundedFakeBatch, epoch)

    outputTraced = tracedModel(*roundedFakeBatch)
    print(f"MAE post PQT: {torch.abs(goldenOutput[0] - outputTraced[0]).mean()}")

    _AnnotateEpsPass.apply(tracedModel)

    epsOut = []
    outNode = list(tracedModel.graph.nodes)[-1]
    for arg in outNode._input_nodes.keys():
        epsOut.append(arg.meta["quant"].eps_out)

    integerizeTracer = LeafTracer(leaf_types=list(LLAMA_OPS | PACT_OPS))
    symbolicTrace = partial(custom_symbolic_trace, tracer=integerizeTracer)

    genericIntegerizePass = partial(passes.integerize.IntegerizePACTNetPass,
                                    enable_add_first = True,
                                    requant_node = True,
                                    export_gelu_node = True,
                                    export_softmax_node = True,
                                    export_div_node = True,
                                    export_rmsnorm_node = True,
                                    export_hardswish_node = True,
                                    skip_identity_rqs = False,
                                    symbolic_trace = symbolicTrace,
                                    D = 2**16)


    integerizePass = genericIntegerizePass(shape_in = [tensor.shape for tensor in roundedFakeBatch], eps_in = eps_in)
    int_fx_model = integerizePass.apply(tracedModel)

    # WIESEP: Serialize the integerized model and save it
    # torch.save(int_fx_model, f"{args.output_prefix}_integerized.pt")
    # int_fx_model = torch.load(f"{args.output_prefix}_integerized.pt")

    integerizedInputs = []
    for inp, eps in zip(roundedFakeBatch, eps_in):
        integerizedInputs.append(torch.round(inp / eps))

    def delistify(_list):
        retList = []
        for arg in _list:
            if isinstance(arg, Iterable) and not isinstance(arg, torch.Tensor):
                retList += delistify(arg)
            else:
                retList.append(arg)
        return retList

    outputInt = int_fx_model(*integerizedInputs)
    outputEpsInt = [out * eps for out, eps in zip(delistify(outputInt), epsOut)]
    print(f"MAE post INT: {torch.abs(goldenOutput[0] - outputEpsInt[0]).mean()}")

    from quantlib.backends.deeploy.pact_export import export_net
    export_net(net=copy.deepcopy(int_fx_model),
            in_data=tuple(integerizedInputs),
            name="llama_integerized",
            out_dir=".",
            eps_in=eps_in,
            integerize=False,
            D=2**16,
            n_levels_in=_NLEVELSACTS)

    # Merge requant shift nodes with constant inputs
    int_fx_folded_model, integerizedFoldedInputs = foldConstantWithReqauntShift(int_fx_model, matchConstantRequantShift, *integerizedInputs)

    outputIntFolded = int_fx_folded_model(*integerizedFoldedInputs)
    outputEpsIntFolded = [out * eps for out, eps in zip(delistify(outputIntFolded), epsOut)]
    print(f"MAE post INT OPT: {torch.abs(goldenOutput[0] - outputEpsIntFolded[0]).mean()}")
    export_net(net=copy.deepcopy(int_fx_folded_model),
            in_data=tuple(integerizedFoldedInputs),
            name="llama_integerized_optimized",
            out_dir=".",
            eps_in=eps_in,
            integerize=False,
            D=2**16,
            n_levels_in=_NLEVELSACTS)