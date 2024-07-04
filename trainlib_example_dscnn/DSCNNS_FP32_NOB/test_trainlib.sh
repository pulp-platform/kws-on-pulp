#!/bin/bash

export PATH=/usr/pack/gcc-9.2.0-af/linux-x64/bin:$PATH 
export LD_LIBRARY_PATH=/usr/pack/gcc-9.2.0-af/linux-x64/lib64/:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/scratch/wetterhorn/cioflanc/miniconda3/pkgs/mpfr-4.0.2-hb69a4c5_1/lib/

export GAP_RISCV_GCC_TOOLCHAIN=/usr/scratch/wetterhorn/cioflanc/tools/gap_riscv_toolchain
export KCONFIG_CONFIG="sdk.config"

# source /usr/scratch/wetterhorn/cioflanc/tools/gap_sdk_private/sourceme.sh # Choose your board/config
source /usr/scratch/wetterhorn/cioflanc/tools/gap_sdk_private/configs/gap9_evk_audio.sh

/usr/scratch/wetterhorn/cioflanc/tools/cmake-3.19.5-Linux-x86_64/bin/cmake -B build
/usr/scratch/wetterhorn/cioflanc/tools/cmake-3.19.5-Linux-x86_64/bin/cmake --build build --target menuconfig
/usr/scratch/wetterhorn/cioflanc/tools/cmake-3.19.5-Linux-x86_64/bin/cmake --build build --target run


# CONFIG_MODEL_DSP_FLOAT16_TYPE_BFLOAT16=y
# CONFIG_MODEL_FLOAT16_TYPE_BFLOAT16=y
# Error at index: 0   (Ideal = -0.0016021730000000 [HEX: 0xbad2]  vs  Actual = -0.0015945430000000 [HEX: 0xbad1])
# Error at index: 2   (Ideal = -0.0006256100000000 [HEX: 0xba24]  vs  Actual = -0.0006294250000000 [HEX: 0xba25])
# Error at index: 3   (Ideal = -0.0001945500000000 [HEX: 0xb94c]  vs  Actual = -0.0001955030000000 [HEX: 0xb94d])
# Error at index: 5   (Ideal = -0.0001897810000000 [HEX: 0xb947]  vs  Actual = -0.0001888280000000 [HEX: 0xb946])
# Error at index: 6   (Ideal = -0.0003356930000000 [HEX: 0xb9b0]  vs  Actual = -0.0003376010000000 [HEX: 0xb9b1])
# Error at index: 7   (Ideal = -0.0001897810000000 [HEX: 0xb947]  vs  Actual = -0.0001916890000000 [HEX: 0xb949])
# Error at index: 8   (Ideal = -0.0000395770000000 [HEX: 0xb826]  vs  Actual = -0.0000410080000000 [HEX: 0xb82c])
# Error at index: 9   (Ideal = -0.0003948210000000 [HEX: 0xb9cf]  vs  Actual = -0.0004005430000000 [HEX: 0xb9d2])
# Error at index: 10   (Ideal = -0.0000619890000000 [HEX: 0xb882]  vs  Actual = -0.0000648500000000 [HEX: 0xb888])
# Error at index: 11   (Ideal = -0.0003242490000000 [HEX: 0xb9aa]  vs  Actual = -0.0003261570000000 [HEX: 0xb9ab])

# CONFIG_MODEL_DSP_FLOAT16_TYPE_IEEE16=y
# CONFIG_MODEL_FLOAT16_TYPE_IEEE16=y
# Error at index: 0   (Ideal = -0.0016021730000000 [HEX: 0xbad2]  vs  Actual = -0.0015945430000000 [HEX: 0xbad1])
# Error at index: 2   (Ideal = -0.0006256100000000 [HEX: 0xba24]  vs  Actual = -0.0006294250000000 [HEX: 0xba25])
# Error at index: 3   (Ideal = -0.0001945500000000 [HEX: 0xb94c]  vs  Actual = -0.0001955030000000 [HEX: 0xb94d])
# Error at index: 5   (Ideal = -0.0001897810000000 [HEX: 0xb947]  vs  Actual = -0.0001888280000000 [HEX: 0xb946])
# Error at index: 6   (Ideal = -0.0003356930000000 [HEX: 0xb9b0]  vs  Actual = -0.0003376010000000 [HEX: 0xb9b1])
# Error at index: 7   (Ideal = -0.0001897810000000 [HEX: 0xb947]  vs  Actual = -0.0001916890000000 [HEX: 0xb949])
# Error at index: 8   (Ideal = -0.0000395770000000 [HEX: 0xb826]  vs  Actual = -0.0000410080000000 [HEX: 0xb82c])
# Error at index: 9   (Ideal = -0.0003948210000000 [HEX: 0xb9cf]  vs  Actual = -0.0004005430000000 [HEX: 0xb9d2])
# Error at index: 10   (Ideal = -0.0000619890000000 [HEX: 0xb882]  vs  Actual = -0.0000648500000000 [HEX: 0xb888])
# Error at index: 11   (Ideal = -0.0003242490000000 [HEX: 0xb9aa]  vs  Actual = -0.0003261570000000 [HEX: 0xb9ab])

