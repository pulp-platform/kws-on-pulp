# Copyright (C) 2021 ETH Zurich
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
# Author: Cristian Cioflan, ETH (cioflanc@iis.ee.ethz.ch)


import torch
import dataset
import os
import time
import math
import nemo

from torchsummary import summary
from model import DSCNN
from utils import remove_txt, parameter_generation
from copy import deepcopy
from pthflops import count_ops
from train import Train

from collections import OrderedDict

import numpy as np

# Device setup
if torch.cuda.is_available():
    device = torch.device('cuda')
else:
    device = torch.device('cpu')
print (torch.version.__version__)
print(device)

# Parameter generation
training_parameters, data_processing_parameters = parameter_generation()  # To be parametrized

# Dataset generation
audio_processor = dataset.AudioProcessor(training_parameters, data_processing_parameters)

train_size = audio_processor.get_size('training')
valid_size = audio_processor.get_size('validation')
test_size = audio_processor.get_size('testing')
tinytrain_size = audio_processor.get_size('tinytrain')
print("Dataset split (Train/valid/test/tinytrain): "+ str(train_size) +"/"+str(valid_size) + "/" + str(test_size) + "/" + str(tinytrain_size))

# Model generation and analysis
model = DSCNN(use_bias = False) # Put to FALSE to reproduce FC layer
# model = DSCNN(use_bias = True) # Put to TRUE to reproduce model_bias layer
model.to(device)
summary(model,(1,49,data_processing_parameters['feature_bin_count']))
dummy_input = torch.rand(1, 1,49,data_processing_parameters['feature_bin_count']).to(device)
count_ops(model, dummy_input)

# Training initialization
trainining_environment = Train(audio_processor, training_parameters, model, device)

# Removing stored inputs and activations
remove_txt()

# ###### Load model before to finetune - GVSOC ###### 
model.load_state_dict(torch.load('./model_nobias_pretrain.pth', map_location=torch.device('cpu')))
model = model.to(device)
print ("Pretrain validation acc")
acc = trainining_environment.validate(model, mode='validation', statistics=False)

if training_parameters['freezebb']:
  # Freeze weights except for FC
  for name, param in model.named_parameters():
      param.requires_grad = False
  model.fc1.weight.requires_grad = True
  # model.fc1.bias.requires_grad = True

  print ("Test freezing")
  for name, param in model.named_parameters():
      print ("For ", str(name), " we require grad? ", str(param.requires_grad))

start=time.clock_gettime(0)
# trainining_environment.train(model, mode='training') # TRAIN
# trainining_environment.train(model, mode='tinytrain') # FINETUNE
print('Finished Training on GPU in {:.2f} seconds'.format(time.clock_gettime(0)-start))

# Ignoring training, load pretrained model
# model.load_state_dict(torch.load('./model.pth', map_location=torch.device('cuda')))
# model.load_state_dict(torch.load('./best_model_nobias.pth', map_location=torch.device('cuda')))

dummy_input = torch.randn(1, 1, 49, 10, requires_grad=True).to(device)
# Export the model
# torch.onnx.export(model,               # model being run
#                   dummy_input,                         # model input (or a tuple for multiple inputs)
#                   "floatmodel.onnx",   # where to save the model (can be a file or file-like object)
#                   export_params=True,        # store the trained parameter weights inside the model file
#                   opset_version=10,          # the ONNX version to export the model to
#                   do_constant_folding=True,  # whether to execute constant folding for optimization
#                   input_names = ['input'],   # the model's input names
#                   output_names = ['output'], # the model's output names
#                   dynamic_axes={'input' : {0 : 'batch_size'},    # variable length axes
#                                 'output' : {0 : 'batch_size'}})


# # torch.set_printoptions(profile="full")
# # fcweights = model.state_dict()['fc1.weight']
# # fcweights = fcweights.reshape(-1)
# # # print (["{0:0.8f}".format(i) for i in fcweights])

# # with open("fcweights.txt", "w") as output:
# #     output.write(str(["{0:0.10f}f".format(i) for i in fcweights]))
# # # print(fcweights)

# mfcc_paths = [
# "utterances_mfcc/backward_18f8afd5_nohash_1_mfcc.txt", 
# "utterances_mfcc/bed_aa80f517_nohash_0_mfcc.tutterances_mfcc/xt", 
# "utterances_mfcc/bird_1f653d27_nohash_0_mfcc.txt", 
# "utterances_mfcc/cat_d5ca80c6_nohash_0_mfcc.txt", 
# "utterances_mfcc/dog_5ff3f9a1_nohash_1_mfcc.txt", 
# "utterances_mfcc/down_1b88bf70_nohash_0_mfcc.txt", 
# "utterances_mfcc/down_3659fc1c_nohash_1_mfcc.txt", 
# "utterances_mfcc/eight_80c45ed6_nohash_0_mfcc.txt", 
# "utterances_mfcc/five_563aa4e6_nohash_2_mfcc.txt", 
# "utterances_mfcc/follow_fb7eb481_nohash_4_mfcc.txt", 
# "utterances_mfcc/forward_87070229_nohash_2_mfcc.txt", 
# "utterances_mfcc/four_ad6a46f1_nohash_1_mfcc.txt", 
# "utterances_mfcc/go_b7e9f841_nohash_1_mfcc.txt", 
# "utterances_mfcc/happy_d91a159e_nohash_1_mfcc.txt", 
# "utterances_mfcc/house_1f653d27_nohash_2_mfcc.txt", 
# "utterances_mfcc/learn_964c7c9e_nohash_0_mfcc.txt", 
# "utterances_mfcc/left_e1469561_nohash_1_mfcc.txt", 
# "utterances_mfcc/marvin_82d0d3ba_nohash_0_mfcc.txt", 
# "utterances_mfcc/nine_6f2f57c1_nohash_0_mfcc.txt", 
# "utterances_mfcc/no_e49428d9_nohash_3_mfcc.txt", 
# "utterances_mfcc/off_3659fc1c_nohash_1_mfcc.txt", 
# "utterances_mfcc/on_e49428d9_nohash_3_mfcc.txt", 
# "utterances_mfcc/one_5e3dde6b_nohash_1_mfcc.txt", 
# "utterances_mfcc/right_e49428d9_nohash_3_mfcc.txt", 
# "utterances_mfcc/seven_cd85758f_nohash_1_mfcc.txt", 
# "utterances_mfcc/sheila_fb7eb481_nohash_0_mfcc.txt", 
# "utterances_mfcc/six_1b4c9b89_nohash_2_mfcc.txt", 
# "utterances_mfcc/stop_3659fc1c_nohash_1_mfcc.txt", 
# "utterances_mfcc/three_b737ee80_nohash_0_mfcc.txt", 
# "utterances_mfcc/tree_db24628d_nohash_0_mfcc.txt", 
# "utterances_mfcc/two_8056e897_nohash_0_mfcc.txt", 
# "utterances_mfcc/up_0cb74144_nohash_2_mfcc.txt", 
# "utterances_mfcc/wow_20d3f11f_nohash_1_mfcc.txt", # visual/ missing from the list
# "utterances_mfcc/yes_e49428d9_nohash_3_mfcc.txt", 
# "utterances_mfcc/zero_e1469561_nohash_3_mfcc.txt"
# ]

# # labels = [
# # 1, 1, 1, 1, 1, 5, 5, 1, 1, 1, 1, 1, 11, 1, 1, 1, 6, 1, 1, 3, X, X, 1, X, 1, 1, 1, X, 1, 1,1 1, X, 1, 1, 1
# # ]

# gvsoc_labels = [
# 1, 
# 1, 
# 1, 
# 1, 
# 1, 
# 5, 
# 5, 
# 1, 
# 1, 
# 1,
# 1,
# 9, 
# 11, 
# 1, 
# 1, 
# 10, 
# 6, 
# 9, 
# 1, 
# 3, 
# 4, 
# 8, 
# 8, 
# 7, 
# 1, 
# 1, 
# 1, 
# 3, 
# 1, 
# 1, 
# 1, 
# 4, 
# 8, 
# 2, 
# 1
# ]

# # print ("Evaluation on GVSOC mfccs")
# # acc = trainining_environment.evaluate_mfcc(model, mode='tinytest', paths=mfcc_paths, labels=gvsoc_labels)

# # Accuracy on the training set. 
# # # print ("Training acc")
# # acc = trainining_environment.validate(model, mode='training', batch_size=-1, statistics=False)
# # # Accuracy on the validation set. 
# # print ("Validation acc")
# # acc = trainining_environment.validate(model, mode='validation', batch_size=-1, statistics=False)
# # # Accuracy on the testing set. 
# # print ("Testing acc")
# # acc = trainining_environment.validate(model, mode='testing', batch_size=-1, statistics=False)
# print ("tinytesting acc")
# # acc = trainining_environment.validate(model, mode='tinytest', batch_size=-1, statistics=False, save = False)
# acc = trainining_environment.validate(model, mode='testing', batch_size=-1, statistics=False, save = False)

# floatavg = model.postavg

# # Initiating quantization process: making the model quantization aware
model_copy = deepcopy(model).to(device)
quantized_model = nemo.transform.quantize_pact(model_copy, dummy_input=torch.randn((1,1,49,10)).to(device))

precision_8 = {
          "conv1": {
            "W_bits": 7
          },
          "relu1": {
            "x_bits": 8
          },
          "conv2": {
            "W_bits": 7
          },
          "relu2": {
            "x_bits": 8
          },
          "conv3": {
            "W_bits": 7
          },
          "relu3": {
            "x_bits": 8
          },
          "conv4": {
            "W_bits": 7
          },
          "relu4": {
            "x_bits": 8
          },
          "conv5": {
            "W_bits": 7
          },
          "relu5": {
            "x_bits": 8
          },
          "conv6": {
            "W_bits": 7
          },
          "relu6": {
            "x_bits": 8
          },
          "conv7": {
            "W_bits": 7
          },
          "relu7": {
            "x_bits": 8
          },
          "conv8": {
            "W_bits": 7
          },
          "relu8": {
            "x_bits": 8
          },
          "conv9": {
            "W_bits": 7
          },
          "relu9": {
            "x_bits": 8
          },
          "fc1": {
            "W_bits": 7
          }

    }
quantized_model.change_precision(bits=1, min_prec_dict=precision_8, scale_weights=True, scale_activations=True)

# Calibrating model's scaling by collecting largest activations
with quantized_model.statistics_act():
    trainining_environment.validate(model=quantized_model, mode='validation', batch_size=128)
    # trainining_environment.validate(model=quantized_model, mode='tinytest', batch_size=10)
quantized_model.reset_alpha_act()

# Remove biases after FQ stage
# quantized_model.remove_bias()

print("\nFakeQuantized @ 8b accuracy (calibrated):")
# acc = trainining_environment.validate(model=quantized_model, mode='tinytest', batch_size=-1)
acc = trainining_environment.validate(model=quantized_model, mode='testing', batch_size=-1)

# Save FC weights after FQ
f = open("fcweights_nobias_pretrain.txt", "w")
weights = model.fc1.weight.data.cpu().numpy()
weights_reshaped = np.reshape(weights, -1)
for weight in weights_reshaped:

  f.write(str(weight)+", ")
f.close()


quantized_model.qd_stage(eps_in=255./255)  # The activations are already in 0-255

print("\nQuantizedDeployable @ mixed-precision accuracy:")
# acc = trainining_environment.validate(model=quantized_model, mode='tinytest', batch_size=-1)
acc = trainining_environment.validate(model=quantized_model, mode='testing', batch_size=-1)

quantized_model.id_stage()

print("\nIntegerDeployable @ mixed-precision accuracy:")
# acc = trainining_environment.validate(model=quantized_model, mode='tinytest', batch_size=-1, integer=True)
acc = trainining_environment.validate(model=quantized_model, mode='testing', batch_size=-1, integer=True)


# intavgmodel = quantized_model.postavg
# # model_bias
# # intavglog = [10.,  5.,  4.,  3.,  6.,  6.,  7.,  6.,  7.,  9., 11.,  8.,  8.,  8.,
# #           9., 11.,  7., 11., 10., 14.,  7.,  8.,  7.,  6.,  9.,  6., 10.,  5.,
# #           5., 12., 15., 10., 10., 12., 16.,  6.,  8.,  8., 13.,  5.,  8.,  6.,
# #           7.,  2., 10.,  9.,  8.,  4., 11.,  5.,  9.,  4., 10.,  4., 13.,  4.,
# #           6.,  6.,  1.,  7.,  8., 11., 11.,  9.]

# # model
# floatlog = [1.2891, 1.1325, 0.7934, 0.8711, 1.5614, 0.6453, 2.3204, 1.6178, 1.0879,
#          1.1957, 1.5591, 0.6227, 0.2808, 0.5643, 1.1200, 1.1179, 0.6563, 1.2392,
#          0.3207, 2.2530, 0.9251, 1.0823, 0.3515, 1.5727, 0.7494, 0.4947, 0.8007,
#          0.6926, 1.2692, 0.6857, 0.7056, 0.9667, 1.6910, 1.7396, 0.1724, 0.7312,
#          1.4514, 1.4013, 0.8520, 0.6831, 0.3204, 1.3741, 0.4185, 0.3420, 1.1421,
#          0.6119, 0.6184, 1.4712, 0.6477, 0.3082, 1.3002, 0.5070, 1.5846, 1.0176,
#          0.6457, 1.4625, 0.6278, 0.4942, 0.8052, 0.4375, 0.3354, 0.4915, 1.1417,
#          0.5416]

# intavglog = [11.,  9.,  8.,  5., 14.,  6., 15., 10., 10.,  9., 13.,  5.,  4.,  8.,
#           7., 11.,  5., 11.,  2., 18.,  7., 10.,  2., 12.,  7.,  4.,  6.,  6.,
#          10.,  8.,  7.,  8., 12., 14.,  2.,  9., 10.,  9.,  9.,  8.,  4., 12.,
#           3.,  4., 11.,  4.,  4., 12.,  5.,  2., 13.,  3., 11.,  7.,  5., 11.,
#           3.,  3.,  9.,  3.,  2.,  4., 10.,  7.]


l = len(list(quantized_model.named_modules()))
eps = OrderedDict([])
for i,(n,l) in enumerate(quantized_model.named_modules()):
    eps[n] = quantized_model.get_eps_at(n, eps_in=255./255)
print (eps)

# eps_avg = eps['avg'] # 0.1138 # OLD
eps_avg = eps['avg'] 

print (eps_avg)

# requant = []
# err = 0.
# summation = 0.
# for elem in intavglog:
#   requant.append(eps_avg.cpu().numpy()[0] * elem)

# floatavgarr = floatavg.cpu().numpy()[0]


# for idx in range (0 , len(floatavgarr)):
#   err = err + (requant[idx] - floatavgarr[idx])*(requant[idx] - floatavgarr[idx])
#   summation += floatavgarr[idx]*floatavgarr[idx]

# avgerr = err / len(floatavgarr)
# relerr = err / summation


# print ("requantized: ")
# print (requant)
# print ("error is: ",err)
# print ("AVG error is: ",avgerr)
# print ("REL error is: ",relerr)


# Saving the model
nemo.utils.export_onnx('model_nobias_pretrain_int.onnx', quantized_model, quantized_model, (1, 49, 10))
# Saving the activations for comparison within Dory
acc = trainining_environment.validate(model=quantized_model, mode='tinytest', batch_size=1, integer=True, save=True)
