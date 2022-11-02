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


export PATH=/usr/pack/gcc-4.9.1-af/x86_64-rhe6-linux/bin:$PATH
export LD_LIBRARY_PATH=/usr/pack/gcc-4.9.1-af/x86_64-rhe6-linux/lib64/:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/usr/pack/gcc-4.9.1-af/x86_64-rhe6-linux/lib/:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/scratch/wetterhorn/cioflanc/miniconda3/pkgs/mpfr-4.0.2-hb69a4c5_1/lib/

export GAP_SDK_DIR=/usr/scratch/wetterhorn/cioflanc/tools/gap_sdk/
# export AUDIO_SAMPLE=/usr/scratch/wetterhorn/cioflanc/kws-on-pulp/kws-on-pulp/dataset/train/right/aa48c94a_nohash_2.wav
export AUDIO_SAMPLE=aa48c94a_nohash_2.wav
export SDK=$1
export MEMORY=$2
export PLATFORM=$3
export MFCC=$4
export NETWORD_DIR=DSCNN
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
  source /usr/scratch/wetterhorn/cioflanc/tools/gap_sdk/sourceme.sh
fi

# Copy model and it's activations to Dory
cd dory/
mkdir -p $NETWORD_DIR
rm $NETWORD_DIR/model.onnx
rm $NETWORD_DIR/out_layer*.txt
rm $NETWORD_DIR/input.txt
cp $CUR_DIR/quantization/input.txt $NETWORD_DIR/
cp $CUR_DIR/quantization/model.onnx  $NETWORD_DIR/
cp $CUR_DIR/quantization/out_layer*.txt $NETWORD_DIR/

# TODO: Fix target's SDK (e.g., dory/dory/Hardware_targets/GAP8/GAP8_gvsoc/HW_description.json)

# Generate source code and weights for model inference
# We use 64 bits for the BatchNorm and ReLU
if [[ $MEMORY == "3" ]]
then
  python network_generate.py NEMO GAP8.GAP8_gvsoc ../config_NEMO_DSCNN.json --app_dir $NETWORD_DIR/ --perf_layer Yes
else
  python network_generate.py NEMO GAP8.GAP8_board_L2 ../config_NEMO_DSCNN.json --app_dir $NETWORD_DIR/ --perf_layer Yes
fi

# Copy the files into our directory, preparing the MFCC integration
# mkdir -p $CUR_DIR/application/ && cp -r $NETWORD_DIR/DORY_network/ "$_"
mkdir -p $CUR_DIR/application/ && cp -r $NETWORD_DIR/DORY_network/ $CUR_DIR/application/
if [[ $MEMORY == "2" ]]
then
  # Save .WAV as .h for L2
  python $CUR_DIR/wav_to_header.py --file $AUDIO_SAMPLE --sdk $SDK
  # mv $CUR_DIR/wav.h $CUR_DIR/application
fi
cd $CUR_DIR/application/

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
    # python utils/flash_to_hyperflash.py --input $CUR_DIR/application/BUILD/PULP/GCC_RISCV/slm_files/flash_stim.slm --output hyperflash_stim.slm
    # python utils/slm_to_hex.py --input hyperflash_stim.slm

    # Out size: 294 K
    python utils/slm_to_hex.py  --input $CUR_DIR/application/BUILD/PULP/GCC_RISCV/slm_files/flash_stim.slm
  fi



# Instructions
# screen -L /dev/ttyUSB2 115200
# ./openocd -f openocd-zcu102-digilent-jtag-hs2.cfg
# /usr/scratch/wetterhorn/cioflanc/tools/pulp_riscv_toolchain/v1.0.16-pulp-riscv-gcc-centos-7/bin/riscv32-unknown-elf-gdb executable