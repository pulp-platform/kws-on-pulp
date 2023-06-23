#!/bin/bash

if [ "$1" == "-h" ] ; then
    echo "PLATFORM: gvsoc, board"
    echo "INPUT: 0 (record)"
    echo "MFCC computation: 0 (online)"
    exit 0
fi

source /home/cioflanc/odda_gap9/gap_sdk_private/sourceme.sh # Choose your board/config
source /home/cioflanc/odda_gap9/gap_sdk_private/configs/gap9_evk_audio.sh

gcc --version

export WAV_FILE=/home/cioflanc/odda_gap9/tiny_denoiser/right_94de6a6a_nohash_4.wav # ORIGINAL

export PLATFORM=$1
export INPUT=$2
export MFCC=$3

# CMake
cmake -B build
cmake --build build --target menuconfig
cmake --build build --target run

