#!/bin/bash

export PATH=/usr/pack/gcc-9.2.0-af/linux-x64/bin:$PATH 
export LD_LIBRARY_PATH=/usr/pack/gcc-9.2.0-af/linux-x64/lib64/:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/usr/pack/gcc-9.2.0-af/linux-x64/lib/:$LD_LIBRARY_PATH 
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/scratch/wetterhorn/cioflanc/miniconda3/pkgs/mpfr-4.0.2-hb69a4c5_1/lib/
export GAP_RISCV_GCC_TOOLCHAIN=/usr/scratch/wetterhorn/cioflanc/tools/gap_riscv_toolchain_2023/gap_riscv_toolchain_ubuntu_install/

export CC=gcc-9.2.0
export CXX=g++-9.2.0

if [ "$1" == "-h" ] ; then
    echo "PLATFORM: gvsoc, board"
    echo "INPUT: 0 (record)"
    echo "MFCC computation: 0 (online)"
    exit 0
fi

source /usr/scratch/wetterhorn/cioflanc/tools/gap_sdk_private/sourceme.sh # Choose your board/config
source /usr/scratch/wetterhorn/cioflanc/tools/gap_sdk_private/configs/gap9_evk_audio.sh

function cmake { /usr/scratch/wetterhorn/cioflanc/tools/cmake-3.19.5-Linux-x86_64/bin/cmake "$@" ; }

export AUDIO_SAMPLE=/usr/scratch/sassauna2/cioflanc/dolphinGSC/speech_commands_v0.02/right/94de6a6a_nohash_4.wav # ORIGINAL
# export AUDIO_SAMPLE=/usr/scratch/sassauna2/cioflanc/dolphinGSC/speech_commands_v0.02/down/42a99aec_nohash_3.wav
# export AUDIO_SAMPLE=/usr/scratch/sassauna2/cioflanc/dolphinGSC/speech_commands_v0.02/stop/3143fdff_nohash_0.wav

export PLATFORM=$1
export INPUT=$2
export MFCC=$3

make clean mfcc all run platform=$PLATFORM WAV_FILE=$AUDIO_SAMPLE INPUT=$INPUT MFCC=$MFCC


