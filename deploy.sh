#!/bin/bash

if [ "$1" == "-h" ] ; then
    echo "PLATFORM: gvsoc, board"
    echo "APPL: 0 (record), 1 (read .wav)"
    echo "MFCC computation: 0 (online), 1 (precomputed)"
    exit 0
fi

export PLATFORM=$1
export APPL=$2
export MFCC=$3

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/scratch/wetterhorn/cioflanc/miniconda3/pkgs/mpfr-4.0.2-hb69a4c5_1/lib/
export GAP_RISCV_GCC_TOOLCHAIN=/usr/scratch/wetterhorn/cioflanc/tools/gap_riscv_toolchain
source /usr/scratch/wetterhorn/cioflanc/tools/gap_sdk_private/configs/gap9_evk_audio.sh
export WAV_FILE=/usr/scratch/wetterhorn/cioflanc/teaching/classes/mlonmcu/fs2024/application/right_94de6a6a_nohash_4.wav # ORIGINAL

mkdir -p application/generate/
cp -r generate/src/ application/generate/
rm application/generate/src/main.c
cp -r generate/inc/ application/generate/
cp -r generate/hex/ application/generate/

cd application/

/usr/scratch/wetterhorn/cioflanc/tools/cmake-3.19.5-Linux-x86_64/bin/cmake -B build
/usr/scratch/wetterhorn/cioflanc/tools/cmake-3.19.5-Linux-x86_64/bin/cmake --build build --target menuconfig
/usr/scratch/wetterhorn/cioflanc/tools/cmake-3.19.5-Linux-x86_64/bin/cmake --build build --target run


