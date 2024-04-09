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
        self.stem_block =  Conv_Stem_Sym(n_channels = self.n_channels, use_bias = self.use_bias).to(self.device)        
        self.pad_block = nn.ConstantPad2d((1, 1, 1, 0), value=0.)
        self.avg   = torch.nn.AvgPool2d(kernel_size=(20, 5), stride=1)


        self.conv_blocks_list = [] 
        for block_idx in range (0, self.n_blocks):
            self.conv_blocks_list.append((DSCNN_block(n_channels = self.n_channels, use_bias = self.use_bias).to(self.device)))

        self.conv_blocks = nn.ModuleList(self.conv_blocks_list)

        self.fc1   = torch.nn.Linear(self.n_channels, self.n_classes, bias=self.use_bias)

        self._initialize_weights(seed=42)


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

    def _initialize_weights(self, seed : int = -1):

        if seed >= 0:
            torch.manual_seed(seed)

        for m in self.modules():

            if isinstance(m, torch.nn.Conv2d):
                torch.nn.init.kaiming_normal_(m.weight, mode='fan_out')
                if m.bias is not None:
                    torch.nn.init.normal_(m.bias)

            elif isinstance(m, torch.nn.BatchNorm2d):
                torch.nn.init.normal_(m.weight)
                if m.bias is not None:
                    torch.nn.init.normal_(m.bias)

            elif isinstance(m, torch.nn.Linear):
                torch.nn.init.normal_(m.weight, 0, 0.01)
                if m.bias is not None:
                    torch.nn.init.normal_(m.bias)


class Conv_Stem_Asym(torch.nn.Module):
    def __init__(self, n_channels = 64, use_bias = False, block_idx = 0):
        super(Conv_Stem_Asym, self).__init__()

        self.use_bias = use_bias
        self.block_idx = block_idx

        self.conv = torch.nn.Conv2d(in_channels = 1, out_channels = n_channels, kernel_size = (10, 4), stride = (2, 2), bias = self.use_bias)
        self.bn   = torch.nn.BatchNorm2d(n_channels)
        self.relu = torch.nn.ReLU()

    def forward(self, x, save = False):

        x = self.conv(x)
        x = self.bn(x)
        x = self.relu(x)
        if (save):
            npy_to_txt(self.block_idx, x.int().cpu().detach().numpy())
            print ("Sum: ", str(torch.sum(x.int())))

        return x


class Conv_Stem_Sym(torch.nn.Module):
    def __init__(self, n_channels = 64, use_bias = False, block_idx = 0):
        super(Conv_Stem_Sym, self).__init__()

        self.use_bias = use_bias
        self.block_idx = block_idx

        self.conv = torch.nn.Conv2d(in_channels = 1, out_channels = n_channels, kernel_size = (3, 3), stride = (2, 2), bias = self.use_bias)
        self.bn   = torch.nn.BatchNorm2d(n_channels)
        self.relu = torch.nn.ReLU()

    def forward(self, x, save = False):

        x = self.conv(x)
        x = self.bn(x)
        x = self.relu(x)
        if (save):
            npy_to_txt(self.block_idx, x.int().cpu().detach().numpy())
            print ("Sum: ", str(torch.sum(x.int())))

        return x


class DSCNN_block(torch.nn.Module):
    def __init__(self, n_channels = 64, use_bias = False, block_idx = 0):
        super(DSCNN_block, self).__init__()

        self.use_bias = use_bias
        self.block_idx = block_idx

        self.conv_dw  = torch.nn.Conv2d(in_channels = n_channels, out_channels = n_channels, kernel_size = (3, 3), stride = (1, 1), groups = n_channels, bias = self.use_bias)
        self.bn_dw    = torch.nn.BatchNorm2d(n_channels)
        self.relu_dw  = torch.nn.ReLU()
        self.conv_pw  = torch.nn.Conv2d(in_channels = n_channels, out_channels = n_channels, kernel_size = (1, 1), stride = (1, 1), bias = self.use_bias)
        self.bn_pw    = torch.nn.BatchNorm2d(n_channels)
        self.relu_pw  = torch.nn.ReLU()

    def forward(self, x, save = False):
        
        x = self.conv_dw(x)
        x = self.bn_dw(x)    
        x = self.relu_dw(x) 
        if (save):
            npy_to_txt(self.block_idx, x.int().cpu().detach().numpy())
            print ("Sum: ", str(torch.sum(x.int())))
        x = self.conv_pw(x)
        x = self.bn_pw(x)
        x = self.relu_pw(x) 
        if (save):
            npy_to_txt(self.block_idx+1, x.int().cpu().detach().numpy())
            print ("Sum: ", str(torch.sum(x.int())))

        return x

