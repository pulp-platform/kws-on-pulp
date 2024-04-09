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

export PATH=/usr/pack/gcc-9.2.0-af/linux-x64/bin:$PATH 
export LD_LIBRARY_PATH=/usr/pack/gcc-9.2.0-af/linux-x64/lib64/:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/scratch/wetterhorn/cioflanc/miniconda3/pkgs/mpfr-4.0.2-hb69a4c5_1/lib/

export GAP_RISCV_GCC_TOOLCHAIN=/usr/scratch/wetterhorn/cioflanc/tools/gap_riscv_toolchain

source /usr/scratch/wetterhorn/cioflanc/tools/gap_sdk_private/sourceme.sh

export WAV_FILE=/usr/scratch/wetterhorn/cioflanc/teaching/classes/mlonmcu/fs2024/application/right_94de6a6a_nohash_4.wav # ORIGINAL

cd application/

/usr/scratch/wetterhorn/cioflanc/tools/cmake-3.19.5-Linux-x86_64/bin/cmake -B build
/usr/scratch/wetterhorn/cioflanc/tools/cmake-3.19.5-Linux-x86_64/bin/cmake --build build --target menuconfig
/usr/scratch/wetterhorn/cioflanc/tools/cmake-3.19.5-Linux-x86_64/bin/cmake --build build --target run # --verbose
