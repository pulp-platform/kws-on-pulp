# ----------------------------------------------------------------------
#
# File: quantizeEEGFormerMHSA.py
#
# Last edited: 10.04.2024
#
# Copyright (C) 2024, ETH Zurich and University of Bologna.
#
# Author:
# - Victor Jung, jungvi@iis.ee.ethz.ch, ETH Zurich
# - Moritz Scherer, scheremo@iis.ee.ethz.ch, ETH Zurich
# - Philip Wiese, pwiese@iis.ee.ethz.ch, ETH Zurich
#
# ----------------------------------------------------------------------
# SPDX-License-Identifier: Apache-2.0
#
# Licensed under the Apache License, Version 2.0 (the License); you may
# not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an AS IS BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
 

import copy
import torch 
import numpy as np
from functools import partial

# from quantgraphtools.QuantGraphTools.utils import foldConstant

from quantUtils import fakeTrain, fakeValidate, roundTensors, getAdhocEpsList, \
    matchShapeNode, ViTCanonAndApprox

import quantlib
from quantlib.editing.fx.util.tracing import LeafTracer
# from quantlib.backends.deeploy.pact_export import export_net
from quantlib.editing.fx.passes.pact.integerize import IntegerizePACTNetPass
from quantlib.editing.fx.passes.pact.pact_util import PACT_OPS, PACT_OPS_INT, \
    custom_symbolic_trace, PACT_symbolic_trace
from quantlib.editing.fx.passes.pact import AnnotateEpsPass

from dscnn import DSCNN, DSCNNFlat, LinearTester


NLEVELSACTS = 2**8
NLEVELSWEIGHTS = 2**8
UPPERPERCENTILE = 99.9
LOWERPERCENTILE = 0.1
EPOCHS = 5

_ActQuantArgs = {
    'n_levels': NLEVELSACTS,
    'act_kind': 'identity',
    'init_clip': "percentile",
    'learn_clip': True,
    'symm': True,
    'leaky': 0.0,
    'rounding': True,
    'upper_percentile': UPPERPERCENTILE,
    'lower_percentile': LOWERPERCENTILE,
    'tqt': True,
    "num_bins": 2**12
}

_LinQuantArgs = {
    'n_levels': NLEVELSWEIGHTS,
    'init_clip': 'max',
    'learn_clip': True,
    'symm_wts': True,
    'rounding': True,
    'quantize': 'per_channel',
    'tqt': True
}

_IntegerQuantArgs = copy.deepcopy(_ActQuantArgs)
_LinearQuantArgs = copy.deepcopy(_LinQuantArgs)
_PactifyQuantArgs = copy.deepcopy(_ActQuantArgs)
_IntegerQuantArgs['tqt'] = False
_IntegerQuantArgs['learn_clip'] = False
_LinearQuantArgs['quantize'] = 'per_layer'


if __name__ == "__main__":

    torch.manual_seed(0)
    np.random.seed(0)

    inputs_fp = torch.randn(1, 1, 49, 10)
    # inputs_fp = torch.randint(0, 255, (1, 1, 49, 10))
    # inputs_fp = torch.randint(0, 255, (1, 10))
    eps_in = tuple(getAdhocEpsList(NLEVELSACTS, inputs_fp))
    rounded_input = roundTensors([inputs_fp], eps_in)

    EEGFormerMHSA_fp = DSCNN()
    EEGFormerMHSA_traced_fp = PACT_symbolic_trace(EEGFormerMHSA_fp)

    golden_output = EEGFormerMHSA_fp(*rounded_input)
    traced_fp_output = EEGFormerMHSA_traced_fp(*rounded_input)
    print(f"[EEGFormer] MAE FP32 (Traced)       : {torch.abs(golden_output - traced_fp_output).mean():.6f}")

    vitPass = ViTCanonAndApprox(_ActQuantArgs, _LinearQuantArgs, _PactifyQuantArgs, _IntegerQuantArgs)
    # EEGFormerMHSA_traced_fp = foldConstant(EEGFormerMHSA_traced_fp, matchShapeNode, inputs_fp)
    EEGFormerMHSA_traced_fq = vitPass.apply(EEGFormerMHSA_traced_fp)

    linop_list = [i for i in EEGFormerMHSA_traced_fq.modules() if isinstance(i, quantlib.algorithms.pact._PACTLinOp)]
    act_list = [i for i in EEGFormerMHSA_traced_fq.modules() if isinstance(i, quantlib.algorithms.pact._PACTActivation)]
    adder_list = [i for i in EEGFormerMHSA_traced_fq.modules() if isinstance(i, tuple(PACT_OPS_INT))]
    eps_list = [i for i in EEGFormerMHSA_traced_fq.modules() if isinstance(i, quantlib.algorithms.pact._PACTEps)]

    _AnnotateEpsPass = AnnotateEpsPass(eps_in, n_levels_in=NLEVELSACTS)

    schedule = {1: "start", (EPOCHS - 2): ["freeze"]}
    actSchedule = {1: "start", (EPOCHS - 2): ["freeze"]}
    epsSchedule = {(EPOCHS - 3): 'start'}

    actController = quantlib.algorithms.pact.PACTActController(modules=act_list,
                                                            schedule=actSchedule,
                                                            init_clip_hi=6.,
                                                            init_clip_lo=-6.,
                                                            verbose=True)
    linearController = quantlib.algorithms.pact.PACTLinearController(modules=linop_list,
                                                                    schedule=schedule,
                                                                    init_clip_hi=16.,
                                                                    init_clip_lo=-16.,
                                                                    verbose=True)
    integerController = quantlib.algorithms.pact.PACTIntegerModulesController(adder_list)
    epsController = quantlib.algorithms.pact.PACTEpsController(fx_model=EEGFormerMHSA_traced_fq,
                                                            modules=eps_list,
                                                            schedule=epsSchedule,
                                                            tracer=LeafTracer(leaf_types=list(PACT_OPS)),
                                                            eps_pass=_AnnotateEpsPass,
                                                            verbose=True)

    quantControllers = [actController, linearController, integerController, epsController]

    optimizer = torch.optim.Adam(EEGFormerMHSA_traced_fq.parameters(), lr=0)

    fakeTrain(EEGFormerMHSA_traced_fq, rounded_input, 0, optimizer)

    for epoch in range(EPOCHS):

        for ctrlr in quantControllers:
            ctrlr.step_pre_training_epoch(epoch, optimizer)

        EEGFormerMHSA_traced_fq.train()
        fakeTrain(EEGFormerMHSA_traced_fq, rounded_input, epoch, optimizer, quantControllers)

        for ctrlr in quantControllers:
            ctrlr.step_pre_validation_epoch(epoch)

        EEGFormerMHSA_traced_fq.eval()
        fakeValidate(EEGFormerMHSA_traced_fq, rounded_input, epoch)

    output_fq = EEGFormerMHSA_traced_fq(*rounded_input)
    print(f"[EEGFormer] MAE FakeQuant (Post-PQT): {torch.abs(golden_output - output_fq).mean():.6f}")

    _AnnotateEpsPass.apply(EEGFormerMHSA_traced_fq)

    epsOut = []
    outNode = list(EEGFormerMHSA_traced_fq.graph.nodes)[-1]
    for arg in outNode._input_nodes.keys():
        epsOut.append(arg.meta["quant"].eps_out)

    integerizeTracer = LeafTracer(leaf_types=list(PACT_OPS))
    symbolicTrace = partial(custom_symbolic_trace, tracer=integerizeTracer)

    genericIntegerizePass = partial(IntegerizePACTNetPass,
                                    enable_add_first=True,
                                    requant_node=True,
                                    export_gelu_node=True,
                                    export_softmax_node=True,
                                    export_div_node=True,
                                    export_rmsnorm_node=True,
                                    export_hardswish_node=True,
                                    skip_identity_rqs=False,
                                    symbolic_trace=symbolicTrace,
                                    D=2**16)

    integerizePass = genericIntegerizePass(shape_in=[tensor.shape for tensor in rounded_input], eps_in=eps_in)
    int_fx_model = integerizePass.apply(EEGFormerMHSA_traced_fq)

    integerizedInputs = []
    for inp, eps in zip(rounded_input, eps_in):
        inp[torch.isposinf(inp)] = NLEVELSACTS // 2 - 1
        inp[torch.isneginf(inp)] = -NLEVELSACTS // 2
        integerizedInputs.append(torch.round(inp / eps))

    outputInt = int_fx_model(*integerizedInputs)
    outputEpsInt = [out * eps for out, eps in zip(outputInt, epsOut)]
    print(f"[EEGFormer] MAE TrueQuant (Post-INT): {torch.abs(golden_output - outputEpsInt[0]).mean():.6f}")
 
    # export_net(net=copy.deepcopy(int_fx_model),
               # in_data=tuple(integerizedInputs),
               # name=f"network",
               # out_dir="EEGFormerMHSA",
               # eps_in=eps_in,
               # integerize=False,
               # D=2**16,
               # n_levels_in=NLEVELSACTS)
    

