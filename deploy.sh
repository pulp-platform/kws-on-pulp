#!/bin/bash

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


export GAP_RISCV_GCC_TOOLCHAIN=/path/to/gap_riscv_toolchain
source /path/to/gap_sdk/sourceme.sh

cd dory/dory_examples
mkdir dscnn
cd dscnn
cp ../../../quantization/input.txt .
rm model.onnx
rm out_layer*.txt
cp ../../../quantization/model.onnx  model.onnx
cp ../../../quantization/out_layer*.txt .
cd ../
python network_generate.py --network_dir dscnn/ --Bn_Relu_Bits 64 --sdk gap_sdk --perf_layer Yes
cd application/
mkdir -p ../../../quantization/dory_examples/application/ && cp -r DORY_network/ "$_"
mkdir -p ../../../quantization/dory_examples/application/ && cp Makefile "$_"
mkdir -p ../../../quantization/dory_examples/dscnn/ && cp -r DORY_network/ "$_"
cd ../../../quantization/dory_examples/application/

make VERBOSE=1 clean all run CORE=8 platform=gvsoc 

