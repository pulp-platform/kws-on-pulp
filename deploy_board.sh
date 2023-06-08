#!/bin/bash

export PATH=/usr/pack/gcc-9.2.0-af/linux-x64/bin:$PATH 
export LD_LIBRARY_PATH=/usr/pack/gcc-9.2.0-af/linux-x64/lib64/:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/usr/pack/gcc-9.2.0-af/linux-x64/lib/:$LD_LIBRARY_PATH 
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/scratch/wetterhorn/cioflanc/miniconda3/pkgs/mpfr-4.0.2-hb69a4c5_1/lib/
export GAP_RISCV_GCC_TOOLCHAIN=/usr/scratch/wetterhorn/cioflanc/tools/gap_riscv_toolchain_2023/gap_riscv_toolchain_ubuntu_install/


export CC=gcc-9.2.0
export CXX=g++-9.2.0

source /usr/scratch/wetterhorn/cioflanc/tools/gap_sdk_private/sourceme.sh # Choose your board/config
source /usr/scratch/wetterhorn/cioflanc/tools/gap_sdk_private/configs/gap9_evk_audio.sh


function cmake { /usr/scratch/wetterhorn/cioflanc/tools/cmake-3.19.5-Linux-x86_64/bin/cmake "$@" ; }

# make clean all run platform=board APP_MODE=0 SILENT=1
# make -j8 run platform=board APP_MODE=0


# BOARD
# /usr/scratch/wetterhorn/cioflanc/tools/cmake-3.19.5-Linux-x86_64/bin/cmake -B build
# /usr/scratch/wetterhorn/cioflanc/tools/cmake-3.19.5-Linux-x86_64/bin/cmake --build clean
# /usr/scratch/wetterhorn/cioflanc/tools/cmake-3.19.5-Linux-x86_64/bin/cmake --build build --target menuconfig # Select your board in the menu
# /usr/scratch/wetterhorn/cioflanc/tools/cmake-3.19.5-Linux-x86_64/bin/cmake --build build --target run --verbose

# cmake -B build
# cmake --build build --target menuconfig # Select your board in the menu
# cmake --build build --target run --verbose



# using the files generated in: /usr/scratch/wetterhorn/cioflanc/mlonmcu_exercise6/exercise6/curr



# GVSOC
# make clean all run platform=board APP_MODE=0 [WAV_FILE=/usr/scratch/wetterhorn/cioflanc/mlonmcu_exercise6/kws-on-pulp/94de6a6a_nohash_4.wav]
make clean mfcc all run platform=gvsoc APP_MODE=0 [WAV_FILE=/usr/scratch/wetterhorn/cioflanc/mlonmcu_exercise6/kws-on-pulp/94de6a6a_nohash_4.wav]
# make clean

# GAP9 board
# Final performance:
#   - num cycles: 1007870
#   - MACs: 2656768
#   - MAC/cycle: 2.63602
#   - n. of Cores: 8

# GAP9 GVSOC
# Final performance:
#   - num cycles: 1006487
#   - MACs: 2656768
#   - MAC/cycle: 2.63964
#   - n. of Cores: 8


# GAPOC GVSOC
# Final performance:
#   - num cycles: 668871
#   - MACs: 2656768
#   - MAC/cycle: 3.97202
#   - n. of Cores: 8


# GAP9 GVSOC
# Final performance:
#   - num cycles: 717865
#   - MACs: 2656768
#   - MAC/cycle: 3.70093
#   - n. of Cores: 8

# GAP9 board
# Final performance:
#   - num cycles: 1007468
#   - MACs: 2656768
#   - MAC/cycle: 2.63707
#   - n. of Cores: 8

