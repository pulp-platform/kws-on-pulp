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
# Author: Cristian Cioflan, ETH (cioflanc@iis.ee.ethz.ch)


import torch
import torch.nn as nn
import torch.nn.functional as F

from utils import npy_to_txt


class DSCNN(torch.nn.Module):
    def __init__(self, n_channels = 64, n_blocks = 4, n_classes = 12, use_bias = False, stem = 'asym', padding='asym', device = 'cpu'):
        super(DSCNN, self).__init__()

        self.n_channels = n_channels
        self.n_blocks = n_blocks
        self.n_classes = n_classes
        self.use_bias = use_bias
        self.device = device

        self.stem = 'sym'
        self.padding =  'asym'
        self.stem_block =  Conv_Stem_Sym(n_channels = self.n_channels, use_bias = self.use_bias, block_idx = 0).to(self.device)        
        self.pad_block = nn.ConstantPad2d((1, 1, 1, 0), value=0.)
        self.avg   = torch.nn.AvgPool2d(kernel_size=(20, 5), stride=1)


        self.conv_blocks_list = [] 
        for block_idx in range (0, self.n_blocks):
            self.conv_blocks_list.append((DSCNN_block(n_channels = self.n_channels, use_bias = self.use_bias, block_idx = 2*n_blocks+1).to(self.device)))

        self.conv_blocks = nn.ModuleList(self.conv_blocks_list)

        self.fc1   = torch.nn.Linear(self.n_channels, self.n_classes, bias=self.use_bias)


    def forward(self, x):

        x = self.pad_block(x)

        x = self.stem_block(x)

        for block_idx in range(0, self.n_blocks):
            x = self.pad_block(x)
            x = self.conv_blocks[block_idx](x)

        x = self.avg (x)
        x = torch.flatten(x, 1) 
        x = self.fc1 (x)

        return x


class Conv_Stem_Asym(torch.nn.Sequential):
    def __init__(self, n_channels = 64, use_bias = True, block_idx = 0):

        self.use_bias = use_bias
        self.block_idx = block_idx

        modules = []
        modules += [torch.nn.Conv2d(in_channels = 1, out_channels = n_channels, kernel_size = (10, 4), stride = (2, 2), bias = self.use_bias)]
        modules += [torch.nn.BatchNorm2d(n_channels)]
        modules += [torch.nn.ReLU(inplace=True)]

        super().__init__(*modules)


class Conv_Stem_Sym(torch.nn.Sequential):
    def __init__(self, n_channels = 64, use_bias = True, block_idx = 0):

        self.use_bias = use_bias
        self.block_idx = block_idx

        modules = []
        modules += [torch.nn.Conv2d(in_channels = 1, out_channels = n_channels, kernel_size = (3, 3), stride = (2, 2), bias = self.use_bias)]
        modules += [torch.nn.BatchNorm2d(n_channels)]
        modules += [torch.nn.ReLU(inplace=True)]

        super().__init__(*modules)


class DSCNN_block(torch.nn.Sequential):
    def __init__(self, n_channels = 64, use_bias = True, block_idx = 0):

        self.use_bias = use_bias
        self.block_idx = block_idx

        modules = []

        modules += [ torch.nn.Conv2d(in_channels = n_channels, out_channels = n_channels, kernel_size = (3, 3), stride = (1, 1), groups = n_channels, bias = self.use_bias) ]
        modules += [ torch.nn.BatchNorm2d(n_channels) ]
        modules += [ torch.nn.ReLU(inplace=True) ]
        modules += [ torch.nn.Conv2d(in_channels = n_channels, out_channels = n_channels, kernel_size = (1, 1), stride = (1, 1), bias = self.use_bias) ]
        modules += [ torch.nn.BatchNorm2d(n_channels) ]
        modules += [ torch.nn.ReLU(inplace=True) ]

        super().__init__(*modules)

