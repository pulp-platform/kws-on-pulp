#!/bin/bash

# export PATH=/usr/pack/gcc-9.2.0-af/linux-x64/bin:$PATH 
# export LD_LIBRARY_PATH=/usr/pack/gcc-9.2.0-af/linux-x64/lib64/:$LD_LIBRARY_PATH
# export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/scratch/wetterhorn/cioflanc/miniconda3/pkgs/mpfr-4.0.2-hb69a4c5_1/lib/

export GAP_RISCV_GCC_TOOLCHAIN=/usr/scratch/wetterhorn/cioflanc/tools/gap_riscv_toolchain

source /usr/scratch/wetterhorn/cioflanc/tools/gap_sdk_private/sourceme.sh # Choose your board/config
source /usr/scratch/wetterhorn/cioflanc/tools/gap_sdk_private/configs/gap9_evk_audio.sh

/usr/scratch/wetterhorn/cioflanc/tools/cmake-3.19.5-Linux-x86_64/bin/cmake -B build
/usr/scratch/wetterhorn/cioflanc/tools/cmake-3.19.5-Linux-x86_64/bin/cmake --build build --target menuconfig
/usr/scratch/wetterhorn/cioflanc/tools/cmake-3.19.5-Linux-x86_64/bin/cmake --build build --target run


