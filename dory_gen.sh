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
    echo "COMPUTE: 0 (PULP GVSOC), 1 (GAP9 multicore), 2 (GAP9 NE16)"
    echo "NETWORK_DIR_DEST: Destination directory"
    echo "NETWORK_DIR_SRC: Source directory"
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
export SDK=$1 # pulp_sdk, gap_sdk
export MEMORY=$2 # 2, 3
export PLATFORM=$3 # gvsoc, fpga, rtl
export MFCC=$4 # 0 - offline, 1 - online
export COMPUTE=$5 # 0 - PULP GVSOC, 1 - GAP9 multicore, 2 - GAP9 NE16
export NETWORK_DIR_DEST=$6
export NETWORK_DIR_DEST_DORY=$6_DORY
export NETWORK_DIR_SRC=$7
export CORES=$8
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
  if [[ $COMPUTE == "0" ]]
  then
    source /usr/scratch/wetterhorn/cioflanc/tools/gap_sdk_mar23/gap_sdk/sourceme.sh #newest GAP8
  else
    source /usr/scratch/wetterhorn/cioflanc/tools/gap_sdk_private/configs/gap9_evk_audio.sh # GAP9
  fi
fi

# Copy model and it's activations to Dory
cd dory/
mkdir -p $NETWORK_DIR_DEST_DORY
rm $CUR_DIR/dory/$NETWORK_DIR_DEST_DORY/example_quantized_ql_integerized.onnx
rm $CUR_DIR/dory/$NETWORK_DIR_DEST_DORY/out_layer*.txt
rm $CUR_DIR/dory/$NETWORK_DIR_DEST_DORY/input.txt

cp $CUR_DIR/$NETWORK_DIR_SRC/input.txt $CUR_DIR/dory/$NETWORK_DIR_DEST_DORY/
cp $CUR_DIR/$NETWORK_DIR_SRC/example_quantized_ql_integerized.onnx  $CUR_DIR/dory/$NETWORK_DIR_DEST_DORY/
cp $CUR_DIR/$NETWORK_DIR_SRC/out_layer*.txt $CUR_DIR/dory/$NETWORK_DIR_DEST_DORY/

# Generate source code and weights for model inference
# We use 64 bits for the BatchNorm and ReLU
# Verbose

if [[ $MEMORY == "3" ]]
then
  if [[ $COMPUTE == "0" ]]
  then
    python network_generate.py Quantlab PULP.PULP_gvsoc $CUR_DIR/$NETWORK_DIR_SRC/config_example_quantized.json --app_dir $NETWORK_DIR_DEST_DORY/ --verbose_level Check_all+Perf_final --perf_layer
  elif [[ $COMPUTE == "1" ]]
  then
    python network_generate.py Quantlab PULP.GAP9 $CUR_DIR/$NETWORK_DIR_SRC/config_example_quantized.json --app_dir $NETWORK_DIR_DEST_DORY/ --verbose_level Check_all+Perf_final --perf_layer
  elif [[ $COMPUTE == "2" ]]
  then
    python network_generate.py Quantlab PULP.GAP9_NE16 $CUR_DIR/$NETWORK_DIR_SRC/config_example_quantized.json --app_dir $NETWORK_DIR_DEST_DORY/ --verbose_level Check_all+Perf_final --perf_layer
  fi
else
  python network_generate.py Quantlab PULP.GAP8_L2 $CUR_DIR/$NETWORK_DIR_SRC/config_example_quantized.json --app_dir $NETWORK_DIR_DEST_DORY/ --verbose_level Check_all+Perf_final --perf_layer
fi

# Copy the files into our directory, preparing the MFCC integration
mkdir -p $CUR_DIR/$NETWORK_DIR_DEST/ && cp -r $NETWORK_DIR_DEST_DORY/* $CUR_DIR/$NETWORK_DIR_DEST/
if [[ $MEMORY == "2" ]]
then
  # Save .WAV as .h for L2
  python $CUR_DIR/wav_to_header.py --file $AUDIO_SAMPLE --sdk $SDK
fi

cd $CUR_DIR/$NETWORK_DIR_DEST/

# Parametrized
make clean all run sample=$AUDIO_SAMPLE sdk=$SDK memory=$MEMORY platform=$PLATFORM mfcc=$MFCC CORE=$CORES # runner_args="--trace=insn"

if [[ $PLATFORM == "rtl" ]]
then
  cd $CUR_DIR
  python utils/slm_to_hex.py  --input $CUR_DIR/testnet/BUILD/PULP/GCC_RISCV/slm_files/flash_stim.slm
fi

# Copy DORY-generated code to main application
mkdir -p $CUR_DIR/../$NETWORK_DIR_DEST/
cp -r $CUR_DIR/$NETWORK_DIR_DEST/src/ $CUR_DIR/../$NETWORK_DIR_DEST/
cp -r $CUR_DIR/$NETWORK_DIR_DEST/inc/ $CUR_DIR/../$NETWORK_DIR_DEST/
cp -r $CUR_DIR/$NETWORK_DIR_DEST/hex/ $CUR_DIR/../$NETWORK_DIR_DEST/
rm $CUR_DIR/../$NETWORK_DIR_DEST/src/main.c