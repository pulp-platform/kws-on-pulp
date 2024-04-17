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
 

import argparse
import os
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
from utils import parameter_generation

from torch.utils.data import DataLoader
from dataset import DatasetProcessor
from datagenerator import DatasetCreator


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


def validate (network, dataloader):
    
    network.eval()
    n_tot = 0
    n_correct = 0

    for i, batched_input in enumerate(dataloader):
        xb, yb = batched_input
        yn = network(xb.to(device))
        n_tot += xb.shape[0]
        n_correct += (yn.to('cpu').argmax(dim=1) == yb).sum()
        if ((i+1)%10 == 0):
            print(f'Accuracy after {i+1} batches: {n_correct/n_tot}')
        if (i+1) == 10:
            break
    print(f'Final accuracy: {n_correct/n_tot}')



if __name__ == "__main__":

    parser = argparse.ArgumentParser()
    parser.add_argument("--net", type=str, default='DSCNN', help='Network to quantize')
    parser.add_argument("--pretrained", type=str, default='model.pth', help='Path to pretrained model')
    parser.add_argument('--fix_channels', action='store_true', help='Fix channels of conv layers for compatibility with DORY')
    parser.add_argument('--no_dory_harmonize', action='store_true',
                        help='If supplied, don\'t align averagePool nodes\' associated requantization nodes and replace adders with DORYAdders')
    parser.add_argument('--word_align_channels', action='store_true',
                        help='Fix channels of conv layers so (#input_ch * #input_bits) is a multiple of 32 to work around XpulpNN HW bug')
    parser.add_argument('--requant_node', action='store_true',
                        help='Export RequantShift nodes instead of mul-add-div sequences in ONNX graph')
    parser.add_argument('--clip_inputs', action='store_true',
                        help='ghettofix to clip inputs to be unsigned')
    parser.add_argument('--config_net_file', type=str, default='config_net_tqt_8b.json', help = 'Network configuration file')
    parser.add_argument('--config_env_file', type=str, default='config_env.json', help = 'Environment configuration file')

    args = vars(parser.parse_args())

    # Parameter generation
    environment_parameters, preprocessing_parameters, training_parameters, experimental_parameters = parameter_generation(args) 

    # Device setup
    os.environ["CUDA_VISIBLE_DEVICES"] = environment_parameters['device_id']
    if torch.cuda.is_available() and environment_parameters['device'] == 'gpu':
        device = torch.device('cuda')        
    else:
        device = torch.device('cpu')
    device = torch.device('cpu')
    print (torch.version.__version__)
    print (device)

    torch.manual_seed(0)
    np.random.seed(0)

    audio_processor = DatasetCreator(environment_parameters, training_parameters, preprocessing_parameters)
    mdataset = DatasetProcessor("training", audio_processor, training_parameters, task = -1, device = 'cpu')
    mdataloader = DataLoader(mdataset, batch_size=training_parameters['batch_size'], shuffle=False, num_workers=0)

    torch.manual_seed(0)
    np.random.seed(0)

    # inputs_fp = torch.randn(1, 1, 49, 10)
    inputs_fp = torch.randint(0, 255, (1, 1, 49, 10)).float()
    # inputs_fp = torch.randn(1, 10)
    eps_in = tuple(getAdhocEpsList(NLEVELSACTS, inputs_fp))
    print (eps_in)
    rounded_input = roundTensors([inputs_fp], eps_in)

    EEGFormerMHSA_fp = DSCNN()

    # load pretrained model
    EEGFormerMHSA_fp.load_state_dict(torch.load(args['pretrained'], map_location='cpu'))

    EEGFormerMHSA_traced_fp = PACT_symbolic_trace(EEGFormerMHSA_fp)

    validate (EEGFormerMHSA_fp, mdataloader)

    golden_output = EEGFormerMHSA_fp(*rounded_input)
    traced_fp_output = EEGFormerMHSA_traced_fp(*rounded_input)

    mdataloader = DataLoader(mdataset, batch_size=training_parameters['batch_size'], shuffle=False, num_workers=0)
    validate (EEGFormerMHSA_traced_fp, mdataloader)

    print(f"[EEGFormer] MAE FP32 (Traced)       : {torch.abs(golden_output - traced_fp_output).mean():.6f}")

    mse = torch.sum((golden_output-traced_fp_output)*(golden_output-traced_fp_output))
    prsum = torch.sum(traced_fp_output*traced_fp_output)
    gtsum = torch.sum(golden_output*golden_output)

    if (mse == 0):
        print ("[EEGFormer] FP32 mse: 0")
    elif (gtsum < mse):
        qserrnr = -10*np.log10(mse.detach().numpy()/gtsum.detach().numpy())
        qquantsnr = -10*np.log10(prsum.detach().numpy()/gtsum.detach().numpy())
        print("[EEGFormer] FP32 qserrnr: ", qserrnr)
        # print("[EEGFormer] FP32 qquantsnr: ", qquantsnr)
    else:
        qserrnr =  10*np.log10(gtsum.detach().numpy()/mse.detach().numpy())
        qquantsnr = 10*np.log10(gtsum.detach().numpy()/prsum.detach().numpy())
        print("[EEGFormer] FP32 qserrnr: ", qserrnr)
        # print("[EEGFormer] FP32 qquantsnr: ", qquantsnr)

    print ("______________________________________________________________________")


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

    print ("______________________________________________________________________")

    output_fq = EEGFormerMHSA_traced_fq(*rounded_input)

    mdataloader = DataLoader(mdataset, batch_size=training_parameters['batch_size'], shuffle=False, num_workers=0)
    validate (EEGFormerMHSA_traced_fq, mdataloader)
    print(f"[EEGFormer] MAE FakeQuant (Post-PQT): {torch.abs(golden_output - output_fq).mean():.6f}")

    mse = torch.sum((golden_output-output_fq)*(golden_output-output_fq))
    prsum = torch.sum(output_fq*output_fq)
    gtsum = torch.sum(golden_output*golden_output)

    if (mse == 0):
        print ("[EEGFormer] FP32 mse: 0")
    elif (gtsum < mse):
        qserrnr = -10*np.log10(mse.detach().numpy()/gtsum.detach().numpy())
        qquantsnr = -10*np.log10(prsum.detach().numpy()/gtsum.detach().numpy())
        print("[EEGFormer] FP32 qserrnr: ", qserrnr)
        # print("[EEGFormer] FP32 qquantsnr: ", qquantsnr)
    else:
        qserrnr =  10*np.log10(gtsum.detach().numpy()/mse.detach().numpy())
        qquantsnr = 10*np.log10(gtsum.detach().numpy()/prsum.detach().numpy())
        print("[EEGFormer] FP32 qserrnr: ", qserrnr)
        # print("[EEGFormer] FP32 qquantsnr: ", qquantsnr)
    print ("______________________________________________________________________")

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

    mdataloader = DataLoader(mdataset, batch_size=training_parameters['batch_size'], shuffle=False, num_workers=0)
    validate (int_fx_model, mdataloader)
    print(f"[EEGFormer] MAE TrueQuant (Post-INT): {torch.abs(golden_output - outputEpsInt[0]).mean():.6f}")

    mse = torch.sum((golden_output-outputEpsInt[0])*(golden_output-outputEpsInt[0]))
    prsum = torch.sum(outputEpsInt[0]*outputEpsInt[0])
    gtsum = torch.sum(golden_output*golden_output)

    if (mse == 0):
        print ("[EEGFormer] FP32 mse: 0")
    elif (gtsum < mse):
        qserrnr = -10*np.log10(mse.detach().numpy()/gtsum.detach().numpy())
        qquantsnr = -10*np.log10(prsum.detach().numpy()/gtsum.detach().numpy())
        print("[EEGFormer] FP32 qserrnr: ", qserrnr)
        # print("[EEGFormer] FP32 qquantsnr: ", qquantsnr)
    else:
        qserrnr =  10*np.log10(gtsum.detach().numpy()/mse.detach().numpy())
        qquantsnr = 10*np.log10(gtsum.detach().numpy()/prsum.detach().numpy())
        print("[EEGFormer] FP32 qserrnr: ", qserrnr)
        # print("[EEGFormer] FP32 qquantsnr: ", qquantsnr)
 
    # export_net(net=copy.deepcopy(int_fx_model),
               # in_data=tuple(integerizedInputs),
               # name=f"network",
               # out_dir="EEGFormerMHSA",
               # eps_in=eps_in,
               # integerize=False,
               # D=2**16,
               # n_levels_in=NLEVELSACTS)
    

