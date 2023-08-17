#!/bin/bash

if [ "$1" == "-h" ] ; then
    echo "PLATFORM: gvsoc, board"
    echo "EVAL: 0 (record)"
    echo "APPL: 0 (record)"
    echo "MFCC computation: 0 (online)"
    echo "SYSTEM: WORK / HOME"
    exit 0
fi

export PLATFORM=$1
export EVAL=$2
export APPL=$3
export MFCC=$4
export SYSTEM=$5

HOME="HOME"
WORK="WORK"

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

    export WAV_FILE=/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser_audiov2/tiny_denoiser/res/right_94de6a6a_nohash_4.wav # ORIGINAL
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
    /usr/scratch/wetterhorn/cioflanc/tools/cmake-3.19.5-Linux-x86_64/bin/cmake --build build --target run
fi

