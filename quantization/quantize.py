# Copyright (C) 2021-2024 ETH Zurich
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# SPDX-License-Identifier: Apache-2.0
# ==============================================================================
#
# Author: Cristian Cioflan, ETH Zurich (cioflanc@iis.ee.ethz.ch)


import nemo
import torch
import json

from utils import remove_txt
from copy import deepcopy
from collections import OrderedDict


# TODO: parametrize
def nemo_quantize(device, model, training_environment, precision_dict_path, tool = 'nemo', model_path = 'model', precision = 8):

    dummy_input = torch.randn(1, 1, 49, 10, requires_grad=True).to(device)
    model_fp32_copy = deepcopy(model).to(device)
    torch.onnx.export(model_fp32_copy,               # model being run
                  dummy_input,                         # model input (or a tuple for multiple inputs)
                  "model_fp32.onnx",   # where to save the model (can be a file or file-like object)
                  export_params=True,        # store the trained parameter weights inside the model file
                  opset_version=10,          # the ONNX version to export the model to
                  do_constant_folding=True,  # whether to execute constant folding for optimization
                  input_names = ['input'],   # the model's input names
                  output_names = ['output'], # the model's output names
                  dynamic_axes={'input' : {0 : 'batch_size'},    # variable length axes
                                'output' : {0 : 'batch_size'}})

    if tool == 'nemo':

        # Initiating quantization process: making the model quantization aware
        quantized_model = nemo.transform.quantize_pact(deepcopy(model), dummy_input=torch.randn((1,1,49,10)).to(device))

        with open(precision_dict_path) as precision_dict_file:
            precision_dict = json.load(precision_dict_file)

        print (quantized_model)

        quantized_model.change_precision(bits=1, min_prec_dict=precision_dict, scale_weights=True, scale_activations=True)

        # Calibrating model's scaling by collecting largest activations
        with quantized_model.statistics_act():
                training_environment.validate(model=quantized_model, mode='validation', batch_size=128)
        quantized_model.reset_alpha_act()

        # Remove biases after FQ stage
        # quantized_model.remove_bias()

        print("\nFakeQuantized @ 8b accuracy (calibrated):")
        acc = training_environment.validate(model=quantized_model, mode='testing', batch_size=128)

        quantized_model.qd_stage(eps_in=255./255)    # The activations are already in 0-255

        print("\nQuantizedDeployable @ mixed-precision accuracy:")
        acc = training_environment.validate(model=quantized_model, mode='testing', batch_size=128)

        quantized_model.id_stage()

        print("\nIntegerDeployable @ mixed-precision accuracy:")
        acc = training_environment.validate(model=quantized_model, mode='testing', batch_size=128, integer=True)

        l = len(list(quantized_model.named_modules()))
        eps = OrderedDict([])
        for i,(n,l) in enumerate(quantized_model.named_modules()):
            eps[n] = quantized_model.get_eps_at(n, eps_in=255./255)

        # Save eps
        f = open("epsilons.txt", "w")
        for i,(n,l) in enumerate(quantized_model.named_modules()):
          f.write(str(i)+ ", " + str(n) + ", " + str(l) + ", " + str(eps[n])+ ", " + "\n")
        f.close()


        # Saving the model
        nemo.utils.export_onnx(model_path + '.onnx', quantized_model, quantized_model, (1, 49, 10))
        # Saving the activations for comparison within Dory
        acc = training_environment.validate(model=quantized_model, mode='testing', batch_size=1, integer=True, save=True)

    # elif tool == 'quantlib':

    #     # TODO
