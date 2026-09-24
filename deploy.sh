#!/bin/bash

# Copyright (C) 2023-2024 ETH Zurich
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

if [ "$1" == "-h" ] ; then
    echo "PLATFORM: gvsoc, board"
    echo "APPL: 0 (record)"
    echo "NOISE EVAL: 0 (record)"
    echo "UTTR EVAL: 0 (record)"
    echo "MFCC computation: 0 (online)"
    echo "SYSTEM: WORK / HOME"
    exit 0
fi

export PLATFORM=$1
export APPL=$2
export NOISE_EVAL=$3
export UTTR_EVAL=$4
export MFCC=$5
export SYSTEM=$6

HOME="HOME"
WORK="WORK"
FEDERI="FEDERI"

if [ "$SYSTEM" = "$HOME" ]
then
    source /home/cioflanc/odda_gap9/gap_sdk_private/sourceme.sh # Choose your board/config
    source /home/cioflanc/odda_gap9/gap_sdk_private/configs/gap9_evk_audio.sh
    export WAV_FILE=/home/cioflanc/odda_gap9/tiny_denoiser/res/right_94de6a6a_nohash_4.wav # ORIGINAL
elif [ "$SYSTEM" = "$WORK" ]
then
    export PATH=/usr/pack/gcc-9.2.0-af/linux-x64/bin:$PATH 
    export LD_LIBRARY_PATH=/usr/pack/gcc-9.2.0-af/linux-x64/lib64/:$LD_LIBRARY_PATH
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/scratch/wetterhorn/cioflanc/miniconda3/pkgs/mpfr-4.0.2-hb69a4c5_1/lib/

    export GAP_RISCV_GCC_TOOLCHAIN=/usr/scratch/wetterhorn/cioflanc/tools/gap_riscv_toolchain

    source /usr/scratch/wetterhorn/cioflanc/tools/gap_sdk_private/sourceme.sh # Choose your board/config
    source /usr/scratch/wetterhorn/cioflanc/tools/gap_sdk_private/configs/gap9_evk_audio.sh

    export WAV_FILE=/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/right_94de6a6a_nohash_4.wav # ORIGINAL
fi


if [ "$SYSTEM" = "$HOME" ]
then
    cmake -B build
    cmake --build build --target menuconfig
    cmake --build build --target run
elif [ "$SYSTEM" = "$WORK" ]
then
    /usr/scratch/wetterhorn/cioflanc/tools/cmake-3.19.5-Linux-x86_64/bin/cmake -B build
    /usr/scratch/wetterhorn/cioflanc/tools/cmake-3.19.5-Linux-x86_64/bin/cmake --build build --target menuconfig
    # runner_args="--trace=cluster/pe0/insn:tracer.txt" /usr/scratch/wetterhorn/cioflanc/tools/cmake-3.19.5-Linux-x86_64/bin/cmake --build build --target run
    /usr/scratch/wetterhorn/cioflanc/tools/cmake-3.19.5-Linux-x86_64/bin/cmake --build build --target run
elif [ "$SYSTEM" = "$FEDERI" ]
then
    cmake -B build
    cmake --build build --target menuconfig
    cmake --build build --target run
fi


# function cmake { /usr/scratch/wetterhorn/cioflanc/tools/cmake-3.19.5-Linux-x86_64/bin/cmake "$@" ; }
# ./openocd -f $GAP_SDK_HOME/utils/openocd/tcl/interface/ftdi/olimex-arm-usb-ocd-h.cfg -f $GAP_SDK_HOME/utils/openocd_tools/tcl/gap9revb.tcl
