#!/bin/bash

# Copyright (C) 2021-2024 ETH Zurich

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
    echo "COMPUTE: 0 (PULP GVSOC), 1 (GAP9 multicore), 2 (GAP9 NE16)"
    echo "NETWORK_DIR_DEST: Destination directory"
    echo "NETWORK_DIR_SRC: Source directory"
    echo "CORE: number of inference cores"
    echo "TRAINABLE_LAYERS: number of trainable layers"
    echo "QUANTIZER: quantization tool"
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
export COMPUTE=$4 # 0 - PULP GVSOC, 1 - GAP9 multicore, 2 - GAP9 NE16
export NETWORK_DIR_DEST=$5
export NETWORK_DIR_SRC=$6
export CORES=$7
export TRAINABLE_LAYERS=$8
export QUANTIZER=$9
export CUR_DIR=$PWD

export TMP_DIR="TMP_DIR"


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

rm -rf $NETWORK_DIR_DEST
mkdir -p $NETWORK_DIR_DEST

rm -rf $TMP_DIR
mkdir -p $TMP_DIR
cp $CUR_DIR/$NETWORK_DIR_SRC/input.txt $TMP_DIR/
if [[ $QUANTIZER == "NEMO" ]]
then
  cp $CUR_DIR/$NETWORK_DIR_SRC/model.onnx $TMP_DIR/model.onnx
elif [[ $QUANTIZER == "Quantlab" ]]
then
  cp $CUR_DIR/$NETWORK_DIR_SRC/example_quantized_ql_integerized.onnx $TMP_DIR/model.onnx 
fi
cp $CUR_DIR/$NETWORK_DIR_SRC/out_layer*.txt $TMP_DIR/

if [[ $QUANTIZER == "NEMO" ]]
then
  # Generate .json
  JSON_STRING='{"BNRelu_bits": 32, "onnx_file": "'${CUR_DIR}'/'${TMP_DIR}'/model.onnx", "code reserved space": 1320000}'
elif [[ $QUANTIZER == "Quantlab" ]]
then 
  # Generate .json
  JSON_STRING='{"BNRelu_bits": 32, "onnx_file": "'${CUR_DIR}'/'${TMP_DIR}'/model.onnx", "code reserved space": 150000, "n_inputs": 1, "input_bits": 8, "input_signed": true}'
fi
echo $JSON_STRING > config_network.json
mv $CUR_DIR/config_network.json $TMP_DIR

cd dory/

# Generate source code and weights for model inference
# We use 64 bits for the BatchNorm and ReLU

if [[ $MEMORY == "3" ]]
then
  if [[ $COMPUTE == "0" ]]
  then
    python network_generate.py $QUANTIZER PULP.PULP_gvsoc $CUR_DIR/$TMP_DIR/config_network.json --app_dir ../$NETWORK_DIR_DEST/ --verbose_level None --n_trainable_layers $TRAINABLE_LAYERS
  elif [[ $COMPUTE == "1" ]]
  then
    python network_generate.py $QUANTIZER PULP.GAP9 $CUR_DIR/$TMP_DIR/config_network.json --app_dir ../$NETWORK_DIR_DEST/ --verbose_level None --n_trainable_layers $TRAINABLE_LAYERS
  elif [[ $COMPUTE == "2" ]]
  then
    python network_generate.py $QUANTIZER PULP.GAP9_NE16 $CUR_DIR/$TMP_DIR/config_network.json --app_dir ../$NETWORK_DIR_DEST/ --verbose_level None --n_trainable_layers $TRAINABLE_LAYERS
  fi
else
  python network_generate.py $QUANTIZER PULP.GAP8_L2 $CUR_DIR/$TMP_DIR/config_network.json --app_dir ../$NETWORK_DIR_DEST/ --verbose_level None --n_trainable_layers $TRAINABLE_LAYERS
fi


if [[ $MEMORY == "2" ]]
then
  # Save .WAV as .h for L2
  python $CUR_DIR/wav_to_header.py --file $AUDIO_SAMPLE --sdk $SDK
fi

cd $CUR_DIR/$NETWORK_DIR_DEST/

# Parametrized
make clean all run sample=$AUDIO_SAMPLE sdk=$SDK platform=$PLATFORM CORE=$CORES # runner_args="--trace=insn"

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
rm -rf $CUR_DIR/$NETWORK_DIR_DEST
