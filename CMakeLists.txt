cmake_minimum_required(VERSION 3.16)

# Application

set(TARGET_NAME         "denoiser")
set(MAIN                "denoiser.c")

# Cmake pre-inits

include ($ENV{GAP_SDK_HOME}/utils/cmake/setup.cmake)

# kconfig options

set (io,   host)
set (DEMO, 1)
set (FLASH_TYPE, MRAM)
set (RAM_TYPE, DEFAULT)
set (FREQ_CL, 200)
set (FREQ_FC, 200)
set (FREQ_SFU, 200)
set (VOLTAGE, 650)


if (${CONFIG_MODEL_FLASH_TYPE_MRAM})
        set(EXEC_FROM_FLASH true)
else()
        set(EXEC_FROM_FLASH false)
endif()

# MFCC

set(BUILD_DIR           BUILD)
set(MFCCBUILD_DIR       ${CURDIR}/BUILD_MFCC)
set(GAP9_SDK_DIR        /usr/scratch/wetterhorn/cioflanc/tools/gap_sdk_private/)
set(DSP_DIR             ${GAP9_SDK_DIR}tools/autotiler_v3/DSP_Libraries/)
set(AUTOTILER_DIR       ${GAP9_SDK_DIR}tools/autotiler_v3/Autotiler/)
set(DSP_LUT_DIR         ${GAP9_SDK_DIR}tools/autotiler_v3/DSP_Libraries/LUT_Tables/)
set(AUTOTILER_MFCC_DIR  ${GAP9_SDK_DIR}tools/autotiler_v3/Generators/MFCC/)
set(WAVIO_DIR           ${GAP9_SDK_DIR}libs/gap_lib/include/)

list (APPEND TARGET_SRCS ${MAIN}
                         ${DSP_DIR}FFT_Library.c 
                         ${DSP_DIR}CmplxFunctions.c
                         ${DSP_DIR}PreProcessing.c
                         ${DSP_DIR}math_funcs.c
                         ${DSP_DIR}MfccBasicKernels.c
                         ${MFCCBUILD_DIR}/MfccKernels.c 
                         ${GAP_LIB_PATH}/wav_io/wavIO.c  
)

list(APPEND TARGET_INCS -I${MFCCBUILD_DIR}
                        -I${DSP_DIR}
                        -I${DSP_LUT_DIR}
                        -I${AUTOTILER_DIR}
                        -I${AUTOTILER_MFCC_DIR}
                        -I${WAVIO_DIR}
)

# DORY

set (NUM_CORES, 8)
set (GAP_SDK, 1)
list (APPEND TARGET_SRCS DORY_network/src/BNReluConvolution0.c
                         DORY_network/src/BNReluConvolution1.c 
                         DORY_network/src/BNReluConvolution2.c 
                         DORY_network/src/BNReluConvolution3.c 
                         DORY_network/src/BNReluConvolution4.c 
                         DORY_network/src/BNReluConvolution5.c 
                         DORY_network/src/BNReluConvolution6.c 
                         DORY_network/src/BNReluConvolution7.c 
                         DORY_network/src/BNReluConvolution8.c 
                         DORY_network/src/dory.c 
                         DORY_network/src/FullyConnected10.c 
                         DORY_network/src/mem.c 
                         DORY_network/src/net_utils.c 
                         DORY_network/src/network.c 
                         DORY_network/src/pulp_nn_add.c 
                         DORY_network/src/pulp_nn_avgpool.c 
                         DORY_network/src/pulp_nn_conv_Co_parallel.c 
                         DORY_network/src/pulp_nn_conv_Ho_parallel.c 
                         DORY_network/src/pulp_nn_conv_HoWo_parallel.c
                         DORY_network/src/pulp_nn_depthwise_3x3_s1.c 
                         DORY_network/src/pulp_nn_depthwise_generic.c
                         DORY_network/src/pulp_nn_depthwise_generic_less_4_weights.c
                         DORY_network/src/pulp_nn_linear.c
                         DORY_network/src/pulp_nn_linear_out_32.c
                         DORY_network/src/pulp_nn_matmul.c
                         DORY_network/src/pulp_nn_maxpool.c
                         DORY_network/src/pulp_nn_pointwise_Co_parallel.c
                         DORY_network/src/pulp_nn_pointwise_Ho_parallel.c
                         DORY_network/src/pulp_nn_pointwise_HoWo_parallel.c
                         DORY_network/src/pulp_nn_utils.c
                         DORY_network/src/ReluPooling9.c
                         DORY_network/src/thorir_dma.c

                         
)

list(APPEND TARGET_INCS -IDORY_network/inc
)
