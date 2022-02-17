#!/bin/bash

# Copyright (C) 2021-2022 ETH Zurich

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at

#     http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# SPDX-License-Identifier: Apache-2.0
# ==============================================================================

# Author: Cristian Cioflan, ETH (cioflanc@iis.ee.ethz.ch)


# Set up constants
export GAP_SDK_DIR=path/to/gap_sdk/
export AUDIO_SAMPLE=path/to/audiosample.wav
export SDK=$1
export NETWORD_DIR=DSCNN
export CUR_DIR=$PWD

if [[ $SDK == "pulp_sdk" ]]
then
  export PULP_RISCV_GCC_TOOLCHAIN=path/to/toolchain
  # Select target
  source path/to/pulp-sdk/configs/pulp-open.sh
else
  export GAP_RISCV_GCC_TOOLCHAIN=path/to/toolchain
  # Select target
  source path/to/gap_sdk/sourceme.sh
fi

# Copy model and it's activations to Dory
cd dory/dory_examples
mkdir -p $NETWORD_DIR
rm $NETWORD_DIR/model.onnx
rm $NETWORD_DIR/out_layer*.txt
rm $NETWORD_DIR/input.txt
cp $CUR_DIR/quantization/input.txt $NETWORD_DIR/
cp $CUR_DIR/quantization/model.onnx  $NETWORD_DIR/
cp $CUR_DIR/quantization/out_layer*.txt $NETWORD_DIR/

# Generate source code and weights for model inference
# We use 64 bits for the BatchNorm and ReLU
python network_generate.py --network_dir $NETWORD_DIR/ --Bn_Relu_Bits 64 --sdk $SDK --perf_layer Yes --l2_buffer_size 300000

# Copy the files into our directory, preparing the MFCC integration
mkdir -p $CUR_DIR/application/ && cp -r application/DORY_network/ "$_"
cd $CUR_DIR/application/

# Run end-to-end KWS on selected 8-core platform (e.g., PULP-OPEN) using the selected SDK (e.g., pulp_sdk)
# Compute MFCC for selected audio sample and perform inference using the MFCCs on GVSOC
# Dory will compare the intermediate features agains the ones generated in Python (quantization/main.py)
make VERBOSE=1 clean all run sample=$AUDIO_SAMPLE sdk=$SDK CORE=8 platform=gvsoc

