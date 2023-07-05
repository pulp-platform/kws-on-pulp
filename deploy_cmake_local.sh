#!/bin/bash

if [ "$1" == "-h" ] ; then
    echo "PLATFORM: gvsoc, board"
    echo "INPUT: 0 (record)"
    echo "MFCC computation: 0 (online)"
    echo "SYSTEM: WORK / HOME"
    exit 0
fi

export PLATFORM=$1
export INPUT=$2
export MFCC=$3
export SYSTEM=$4

HOME="HOME"
WORK="WORK"

if [ "$SYSTEM" = "$HOME" ]
then
    source /home/cioflanc/odda_gap9/gap_sdk_private/sourceme.sh # Choose your board/config
    source /home/cioflanc/odda_gap9/gap_sdk_private/configs/gap9_evk_audio.sh
    export WAV_FILE=/home/cioflanc/odda_gap9/tiny_denoiser/right_94de6a6a_nohash_4.wav # ORIGINAL
elif [ "$SYSTEM" = "$WORK" ]
then
    export GAP_RISCV_GCC_TOOLCHAIN=/usr/scratch/wetterhorn/cioflanc/tools/gap_riscv_toolchain
    source /usr/scratch/wetterhorn/cioflanc/tools/gap_sdk_private/sourceme.sh # Choose your board/config
    source /usr/scratch/wetterhorn/cioflanc/tools/gap_sdk_private/configs/gap9_evk_audio.sh
    export WAV_FILE=/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser_audiov2/tiny_denoiser/right_94de6a6a_nohash_4.wav # ORIGINAL
fi

# CMake
cmake -B build
cmake --build build --target menuconfig
cmake --build build --target run

