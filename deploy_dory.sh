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

if [ "$1" == "-h" ] ; then
    echo "SDK: pulp_sdk, gap_sdk"
    echo "MEMORY: (L)2, (L)3"
    echo "PLATFORM: gvsoc, fpga, rtl"
    echo "MFCC computation: 0 (offline), 1 (online)"
    exit 0
fi


# export PATH=/usr/pack/gcc-4.9.1-af/x86_64-rhe6-linux/bin:$PATH
# export LD_LIBRARY_PATH=/usr/pack/gcc-4.9.1-af/x86_64-rhe6-linux/lib64/:$LD_LIBRARY_PATH
# export LD_LIBRARY_PATH=/usr/pack/gcc-4.9.1-af/x86_64-rhe6-linux/lib/:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/scratch/wetterhorn/cioflanc/miniconda3/pkgs/mpfr-4.0.2-hb69a4c5_1/lib/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/scratch/wetterhorn/cioflanc/teaching/classes/mlonmcu/mlonmcu_exercise6/exercise6/local_libs/

export CC=gcc-9.2.1
export CXX=g++-9.2.1

export GAP_SDK_DIR=/usr/scratch/wetterhorn/cioflanc/tools/gap_sdk/
# export AUDIO_SAMPLE=/usr/scratch/wetterhorn/cioflanc/kws-on-pulp/kws-on-pulp/dataset/train/right/aa48c94a_nohash_2.wav
export AUDIO_SAMPLE=/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/kws-on-pulp/aa48c94a_nohash_2.wav
export SDK=$1
export MEMORY=$2
export PLATFORM=$3
export MFCC=$4
export COMPUTE=$5
export NETWORK_DIR=DSCNN
export NETWORK_SRC_DIR=DSCNN_SRC
export CUR_DIR=$PWD



if [[ $SDK == "pulp_sdk" ]]
then
  export PULP_RISCV_GCC_TOOLCHAIN=/usr/scratch/wetterhorn/cioflanc/tools/pulp_riscv_toolchain/v1.0.16-pulp-riscv-gcc-centos-7/
  # Select target
  if [[ $PLATFORM == "gvsoc" ]]
  then
    source /usr/scratch/wetterhorn/cioflanc/tools/pulp-sdk/configs/pulp-open.sh
  elif [[ $PLATFORM == "fpga" ]]
  then
    source /usr/scratch/wetterhorn/cioflanc/tools/pulp_sdk_fpga/pulp-sdk/configs/pulp-open.sh
  elif [[ $PLATFORM == "rtl" ]]
  then
    source /usr/scratch/wetterhorn/cioflanc/tools/pulp_sdk_fpga/pulp-sdk/configs/pulp-open.sh
  fi
else
  export GAP_RISCV_GCC_TOOLCHAIN=/usr/scratch/wetterhorn/cioflanc/tools/gap_riscv_toolchain/
  # Select target
  # source /usr/scratch/wetterhorn/cioflanc/tools/gap_sdk/sourceme.sh # original
  # source /usr/scratch/wetterhorn/cioflanc/tools/gap_sdk_mar23/gap_sdk/sourceme.sh #newest GAP8
  source /usr/scratch/wetterhorn/cioflanc/tools/gap_sdk_private/configs/gap9_evk_audio.sh # GAP9
fi

mkdir $NETWORK_SRC_DIR

cp $CUR_DIR/quantization/input.txt $NETWORK_SRC_DIR/
cp $CUR_DIR/quantization/model_int8.onnx $NETWORK_SRC_DIR/model.onnx
cp $CUR_DIR/quantization/out_layer*.txt $NETWORK_SRC_DIR/
cp $CUR_DIR/config_DSCNN.json $NETWORK_SRC_DIR/

# Copy model and it's activations to Dory
cd deployment/dory/
mkdir -p $NETWORK_DIR
rm $NETWORK_DIR/model.onnx
rm $NETWORK_DIR/out_layer*.txt
rm $NETWORK_DIR/input.txt


cp $CUR_DIR/$NETWORK_SRC_DIR/input.txt $NETWORK_DIR/
cp $CUR_DIR/$NETWORK_SRC_DIR/model.onnx  $NETWORK_DIR/
cp $CUR_DIR/$NETWORK_SRC_DIR/out_layer*.txt $NETWORK_DIR/

# TODO: Fix target's SDK (e.g., dory/dory/Hardware_targets/GAP8/GAP8_gvsoc/HW_description.json)

# Generate source code and weights for model inference
# We use 64 bits for the BatchNorm and ReLU
if [[ $MEMORY == "3" ]]
then
  if [[ $COMPUTE == "0" ]]
  then
    python network_generate.py NEMO PULP.PULP_gvsoc $CUR_DIR/$NETWORK_SRC_DIR/config_DSCNN.json --app_dir $NETWORK_DIR/ --verbose_level Check_all+Perf_final --perf_layer
  else
    python network_generate.py NEMO PULP.GAP9_NE16 $CUR_DIR/$NETWORK_SRC_DIR/config_DSCNN.json --app_dir $NETWORK_DIR/ --verbose_level Check_all+Perf_final --perf_layer
  fi
else
  python network_generate.py NEMO PULP.GAP8_L2 $CUR_DIR/$NETWORK_SRC_DIR/config_DSCNN.json --app_dir $NETWORK_DIR/ --verbose_level Check_all+Perf_final --perf_layer
fi

# Copy the files into our directory, preparing the MFCC integration
# mkdir -p $CUR_DIR/application_dscnnl_gap8/ && cp -r $NETWORK_DIR/DORY_network/ "$_"
# mkdir -p $CUR_DIR/application_dscnnl_gap8/ && cp -r $NETWORK_DIR/DORY_network/ $CUR_DIR/application_dscnnl_gap8/
mkdir -p $CUR_DIR/application_dscnnl_gap8/ && cp -r $NETWORK_DIR/* $CUR_DIR/application_dscnnl_gap8/
if [[ $MEMORY == "2" ]]
then
  # Save .WAV as .h for L2
  python $CUR_DIR/wav_to_header.py --file $AUDIO_SAMPLE --sdk $SDK
  # mv $CUR_DIR/wav.h $CUR_DIR/application_dscnnl_gap8
fi
cd $CUR_DIR/application_dscnnl_gap8/

# Run end-to-end KWS on selected 8-core platform (e.g., PULP-OPEN) using the selected SDK (e.g., pulp_sdk)
# Compute MFCC for selected audio sample and perform inference using the MFCCs on GVSOC
# Dory will compare the intermediate features agains the ones generated in Python (quantization/main.py) 

# VERBOSE=1 requires MAGICK?! TODO: understand
# make VERBOSE=1 clean all run sample=$AUDIO_SAMPLE sdk=$SDK memory=$MEMORY CORE=8 platform=$PLATFORM 

# Parametrized
make clean all run sample=$AUDIO_SAMPLE sdk=$SDK memory=$MEMORY platform=$PLATFORM mfcc=$MFCC CORE=8 # runner_args="--trace=insn"

if [[ $PLATFORM == "rtl" ]]
  then
    cd $CUR_DIR
    # Convert .slm to .hex
    # Out size: 144 K
    # python utils/flash_to_hyperflash.py --input $CUR_DIR/application_dscnnl_gap8/BUILD/PULP/GCC_RISCV/slm_files/flash_stim.slm --output hyperflash_stim.slm
    # python utils/slm_to_hex.py --input hyperflash_stim.slm

    # Out size: 294 K
    python utils/slm_to_hex.py  --input $CUR_DIR/application_dscnnl_gap8/BUILD/PULP/GCC_RISCV/slm_files/flash_stim.slm
  fi



# Instructions
# screen -L /dev/ttyUSB2 115200
# ./openocd -f openocd-zcu102-digilent-jtag-hs2.cfg
# /usr/scratch/wetterhorn/cioflanc/tools/pulp_riscv_toolchain/v1.0.16-pulp-riscv-gcc-centos-7/bin/riscv32-unknown-elf-gdb executable


# DSCNN - all good :) with l2_pulp_sdk

