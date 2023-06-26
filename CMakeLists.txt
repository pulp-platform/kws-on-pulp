cmake_minimum_required(VERSION 3.19.5)

# Application

set(TARGET_NAME         "application")
set(MAIN                "application.c")


# Cmake pre-inits


set(CONFIG_GAP_SDK_HOME $ENV{GAP_SDK_HOME})
include ($ENV{GAP_SDK_HOME}/utils/cmake/setup.cmake)
set(BOARD_NAME gap9_evk)
set(PMSIS_OS freertos)
set(platform $ENV{PLATFORM})


# kconfig options

set (io   host)
set (DEMO 1)


# Not needed for MRAM
# set (CONFIG_FLASH_LAYOUT_CUSTOM 1)


# set (FLASH_TYPE MRAM) # HYPERFLASH # Working for small things to flash
set (FLASH_TYPE DEFAULT) # HYPERFLASH
set (RAM_TYPE   DEFAULT) # DEFAULT
# set (MODEL_L3_FLASH   AT_MEM_L3_MRAMFLASH) # Working for small things to flash
set (MODEL_L3_FLASH   AT_MEM_L3_DEFAULTFLASH)
# set (READFS_FLASH  mram) # Not needed for MRAM
set (EXEC_FROM_FLASH true)
set (MODEL_L3_RAM   AT_MEM_L3_DEFAULTRAM)

set (FREQ_CL 50)
set (FREQ_FC 50)
set (FREQ_SFU 50)
set (VOLTAGE 650)
set (CLUSTER_STACK_SIZE 8192)
set (CLUSTER_SLAVE_STACK_SIZE 4096)
set (TOTAL_STACK_SIZE $(shell expr $(CLUSTER_STACK_SIZE) \+ $(CLUSTER_SLAVE_STACK_SIZE) \* $(CLUSTER_NUM_CORES)))
set (MODEL_L1_MEMORY $(shell expr 120000 \- $(TOTAL_STACK_SIZE)))
set (MODEL_L2_MEMORY 1000000)
set (MODEL_L3_MEMORY 8000000)

set (WAV_FILE $ENV{WAV_FILE})
set (MFCC $ENV{MFCC})
set (INPUT $ENV{INPUT})

# PULP trainlib
set(MATMUL_TYPE_FW_L0  0)         # Selects which optimized matmul to be used in FW (see mm_manager_list.txt or "MM_manager()" body to verify which one is called)
set(MATMUL_TYPE_WG_L0  0)         # Selects which optimized matmul to be used in WEIGHT GRAD (see mm_manager_list.txt or "MM_manager()" body to verify which one is called)
set(MATMUL_TYPE_IG_L0  0)         # Selects which optimized matmul to be used in IN GRAD (see mm_manager_list.txt or "MM_manager()" body to verify which one is called)

set (NUM_MATMULS 24)


set(GAP9_SDK_DIR        /home/cioflanc/odda_gap9/gap_sdk_private/)
set(CURDIR              /home/cioflanc/odda_gap9/tiny_denoiser/)
set(BUILD_DIR           ${CURDIR}/build)
set(TRAINLIB_DIR        ${CURDIR}/trainlib_example_dscnn/DSCNN)
set(DSP_DIR             ${GAP9_SDK_DIR}tools/autotiler_v3/DSP_Libraries/)
set(DSP_LUT_DIR         ${GAP9_SDK_DIR}tools/autotiler_v3/DSP_Libraries/LUT_Tables/)
set(EMUL_DIR            ${GAP9_SDK_DIR}tools/autotiler_v3/Emulation/)
set(AUTOTILER_DIR       ${GAP9_SDK_DIR}tools/autotiler_v3/Autotiler/)
set(AUTOTILER_MFCC_DIR  ${GAP9_SDK_DIR}tools/autotiler_v3/Generators/MFCC/)
set(WAVIO_DIR           ${GAP9_SDK_DIR}libs/gap_lib/include/)
set(DSP_GEN_DIR         ${GAP9_SDK_DIR}/tools/autotiler_v3/DSP_Generators/)

set(MFCC_SRCG                    ${GAP9_SDK_DIR}/tools/autotiler_v3/DSP_Generators/DSP_Generators.c)
set(MFCC_MODEL_GEN               ${BUILD_DIR}/GenMFCC)
set(MFCC_HEAD                    ${BUILD_DIR}/MFCC_params.h)
set(MFCC_PARAMS_JSON             ${CURDIR}/MfccConfig.json)
set(MFCC_SRC_CODE                ${BUILD_DIR}/MfccKernels.c)

# Application

add_custom_command (
    
    OUTPUT    ${MFCC_MODEL_GEN}
    COMMAND   python ${DSP_GEN_DIR}/DSP_LUTGen.py ${MFCC_PARAMS_JSON} --build_dir ${BUILD_DIR} --save_params_header ${MFCC_HEAD} --save_text

)

add_custom_command (
    
    OUTPUT    ${MFCC_SRC_CODE}
    COMMAND   ${MFCC_MODEL_GEN} -o ${BUILD_DIR} -c ${BUILD_DIR} ${MODEL_GEN_EXTRA_FLAGS}
    DEPENDS   ${MFCC_MODEL_GEN}

)

add_custom_target (
    MFCC_HEAD
    WORKING_DIRECTORY    ${BUILD_DIR}
    COMMAND              python ${DSP_GEN_DIR}/DSP_LUTGen.py ${MFCC_PARAMS_JSON} --build_dir ${BUILD_DIR} --save_params_header ${MFCC_HEAD} --save_text
    SOURCES              ${MFCC_PARAMS_JSON}
    BYPRODUCTS           ${MFCC_HEAD}
)


add_custom_target (
    MFCC_MODEL_GEN
    WORKING_DIRECTORY    ${BUILD_DIR}
    COMMAND              gcc -g -o ${MFCC_MODEL_GEN} -I${BUILD_DIR} -I${DSP_GEN_DIR} -I${TILER_INC} -I${TILER_EMU_INC} ${CURDIR}/MfccModel.c ${MFCC_SRCG} ${TILER_LIB} ${TABLE_CFLAGS} ${COMPILE_MODEL_EXTRA_FLAGS} -DUSE_POWER=${USE_POWER} ${SDL_FLAGS}
    BYPRODUCTS           ${MFCC_MODEL_GEN}   
    DEPENDS              ${MFCC_HEAD}          

)

add_custom_target (
    mfcc_src
    WORKING_DIRECTORY    ${BUILD_DIR}
    COMMAND              
    DEPENDS              ${MFCC_MODEL_GEN}
)


add_custom_target(
    graph
    WORKING_DIRECTORY   ${BUILD_DIR}
    COMMAND             SFU -i ${CURDIR}/Graph.src -C
    SOURCES             ${CURDIR}/Graph.src
    BYPRODUCTS          ${BUILD_DIR}/Graph_L2_Descr.c ${BUILD_DIR}/Graph_L2_Descr.h ${BUILD_DIR}/L2_Descr_Table.c ${BUILD_DIR}/L2_Descr_Table.h
    )


list (APPEND TARGET_SRCS 
                         
                         
                         ${DSP_DIR}FFT_Library.c 
                         ${DSP_DIR}CmplxFunctions.c
                         ${DSP_DIR}PreProcessing.c
                         ${DSP_DIR}math_funcs.c
                         ${DSP_DIR}MfccBasicKernels.c
                         ${BUILD_DIR}/MfccKernels.c 
                         ${GAP9_SDK_DIR}/libs/gap_lib/wav_io/wavIO.c
                         ${MAIN}
                         dac.c
                         localutil.c
                         ${BUILD_DIR}/Graph_L2_Descr.c
                         ${BUILD_DIR}/L2_Descr_Table.c
                         ${GAP9_SDK_DIR}rtos/sfu/SFU_RT.c
                         ${GAP_SDK_HOME}/tools/audio-framework/runtime/include
)


list (APPEND TARGET_INCS 
                         
                         -I${TILER_INC}
                         -I${TILER_EMU_INC}
                         -I${GAP_SDK_HOME}/libs/gap_lib/include
                         -I${CURDIR}DORY_network/inc
                         -I${GAP9_SDK_DIR}rtos/pmsis/bsp/include/bsp
                         -I${GAP9_SDK_DIR}rtos/pmsis/bsp/include/bsp/ram
                         -I${GAP9_SDK_DIR}rtos/pmsis/bsp/include/bsp/flash
                         -I${GAP9_SDK_DIR}rtos/sfu/include
                         -I.
                         -I${DSP_DIR}
                         -I${DSP_LUT_DIR}
                         -I${AUTOTILER_DIR}
                         -I${AUTOTILER_MFCC_DIR}
                         -I${WAVIO_DIR}
                         -I${DSP_GEN_DIR}
                         -I${TRAINLIB_DIR}/lib/include
                         -I${TRAINLIB_DIR}

)


# DORY

set (NUM_CORES 8)
set (GAP_SDK 1)

set (TARGET_CHIP_FAMILY_GAP9 1)
list (APPEND TARGET_SRCS ${CURDIR}DORY_network/src/BNReluConvolution0.c
                         ${CURDIR}DORY_network/src/BNReluConvolution1.c 
                         ${CURDIR}DORY_network/src/BNReluConvolution2.c 
                         ${CURDIR}DORY_network/src/BNReluConvolution3.c 
                         ${CURDIR}DORY_network/src/BNReluConvolution4.c 
                         ${CURDIR}DORY_network/src/BNReluConvolution5.c 
                         ${CURDIR}DORY_network/src/BNReluConvolution6.c 
                         ${CURDIR}DORY_network/src/BNReluConvolution7.c 
                         ${CURDIR}DORY_network/src/BNReluConvolution8.c 
                         ${CURDIR}DORY_network/src/dory.c # GAP8GAP9
                         # ${CURDIR}DORY_network/src/dory_dma.c # ONESHOT
                         ${CURDIR}DORY_network/src/FullyConnected10.c 
                         ${CURDIR}DORY_network/src/mem.c 
                         ${CURDIR}DORY_network/src/net_utils.c 
                         ${CURDIR}DORY_network/src/network.c 
                         ${CURDIR}DORY_network/src/pulp_nn_add.c 
                         ${CURDIR}DORY_network/src/pulp_nn_avgpool.c 
                         ${CURDIR}DORY_network/src/pulp_nn_conv_Co_parallel.c 
                         ${CURDIR}DORY_network/src/pulp_nn_conv_Ho_parallel.c 
                         ${CURDIR}DORY_network/src/pulp_nn_conv_HoWo_parallel.c
                         ${CURDIR}DORY_network/src/pulp_nn_depthwise_3x3_s1.c 
                         ${CURDIR}DORY_network/src/pulp_nn_depthwise_generic.c
                         ${CURDIR}DORY_network/src/pulp_nn_depthwise_generic_less_4_weights.c
                         ${CURDIR}DORY_network/src/pulp_nn_linear.c
                         ${CURDIR}DORY_network/src/pulp_nn_linear_out_32.c
                         ${CURDIR}DORY_network/src/pulp_nn_matmul.c
                         ${CURDIR}DORY_network/src/pulp_nn_maxpool.c
                         ${CURDIR}DORY_network/src/pulp_nn_pointwise_Co_parallel.c
                         ${CURDIR}DORY_network/src/pulp_nn_pointwise_Ho_parallel.c
                         ${CURDIR}DORY_network/src/pulp_nn_pointwise_HoWo_parallel.c
                         ${CURDIR}DORY_network/src/pulp_nn_utils.c
                         ${CURDIR}DORY_network/src/ReluPooling9.c
                         ${CURDIR}DORY_network/src/thorir_dma.c # GAP8GAP9                
)

list (APPEND TARGET_SRCS ${TRAINLIB_DIR}/net.c
                         ${TRAINLIB_DIR}/lib/sources/pulp_act_fp16.c
                         ${TRAINLIB_DIR}/lib/sources/pulp_act_fp32.c
                         ${TRAINLIB_DIR}/lib/sources/pulp_conv2d_fp16.c
                         ${TRAINLIB_DIR}/lib/sources/pulp_conv2d_fp32.c
                         ${TRAINLIB_DIR}/lib/sources/pulp_conv_dw_fp16.c
                         ${TRAINLIB_DIR}/lib/sources/pulp_conv_dw_fp32.c
                         ${TRAINLIB_DIR}/lib/sources/pulp_conv_pw_fp16.c
                         ${TRAINLIB_DIR}/lib/sources/pulp_conv_pw_fp32.c
                         ${TRAINLIB_DIR}/lib/sources/pulp_im2col_fp16.c
                         ${TRAINLIB_DIR}/lib/sources/pulp_im2col_fp32.c
                         ${TRAINLIB_DIR}/lib/sources/pulp_linear_fp16.c
                         ${TRAINLIB_DIR}/lib/sources/pulp_linear_fp32.c
                         ${TRAINLIB_DIR}/lib/sources/pulp_losses_fp16.c
                         ${TRAINLIB_DIR}/lib/sources/pulp_losses_fp32.c
                         ${TRAINLIB_DIR}/lib/sources/pulp_matmul_fp16.c
                         ${TRAINLIB_DIR}/lib/sources/pulp_matmul_fp32.c
                         ${TRAINLIB_DIR}/lib/sources/pulp_mhsa_fp32.c
                         ${TRAINLIB_DIR}/lib/sources/pulp_optimizers_fp16.c
                         ${TRAINLIB_DIR}/lib/sources/pulp_optimizers_fp32.c
                         ${TRAINLIB_DIR}/lib/sources/pulp_pooling_fp16.c
                         ${TRAINLIB_DIR}/lib/sources/pulp_pooling_fp32.c
                         # ${TRAINLIB_DIR}/lib/sources/pulp_residual_fp16.c
                         # ${TRAINLIB_DIR}/lib/sources/pulp_residual_fp32.c
                         ${TRAINLIB_DIR}/lib/sources/pulp_rnn_fp32.c
                         ${TRAINLIB_DIR}/lib/sources/pulp_train_utils_fp16.c
                         ${TRAINLIB_DIR}/lib/sources/pulp_train_utils_fp32.c
)



list(APPEND TARGET_CFLAGS   -O2
                            -s
                            -w
                            -mno-memcpy
                            -fno-tree-loop-distribute-patterns
                            -Wno-incompatible-pointer-types

)


list(APPEND TARGET_PREPROCESSOR  # -DPERF
                                -DNUM_CORES=${NUM_CORES}
                                -DGAP_SDK=${GAP_SDK}
                                -DFLASH_TYPE=${FLASH_TYPE}
                                -DUSE_${FLASH_TYPE}
                                -DUSE_${RAM_TYPE}
                                -DSTACK_SIZE=${CLUSTER_STACK_SIZE}
                                -DSLAVE_STACK_SIZE=${CLUSTER_SLAVE_STACK_SIZE}
                                -DFREQ_FC=${FREQ_FC}
                                -DFREQ_CL=${FREQ_CL}
                                -DFREQ_SFU=${FREQ_SFU}
                                -DVOLTAGE=${VOLTAGE}
                                -DWAV_FILE=${WAV_FILE}
                                -DMFCC=${MFCC}
                                -DINPUT=${INPUT}
                                # -DPERF
                                # -DPRINTDEB
                                # -DPRINT_INOUT
                                # -DCHECKSUM
                                # -DVERBOSE
                                -DSILENT
                                -DCONFIG_GAP9_EVK
                                -DFS_READ_FS
                                -DCONFIG_FLASH_LAYOUT_CUSTOM
                                -DMODEL_L2_MEMORY=${MODEL_L2_MEMORY}
                                -DTARGET_CHIP_FAMILY_GAP9
                                # TrainLib
                                -DFABRIC
                                -DCLUSTER
                                # -DPROF_NET
                                -mhwloopalign
                                # -DSTATS
                                -DMATMUL_TYPE_FW_L0=${MATMUL_TYPE_FW_L0}
                                -DMATMUL_TYPE_WG_L0=${MATMUL_TYPE_WG_L0}
                                -DMATMUL_TYPE_IG_L0=${MATMUL_TYPE_IG_L0}



)


# Working well for small files
# list(APPEND READFS_MRAM_FILES 

# list(APPEND READFS_FILES 
#                          ${CURDIR}DORY_network/hex/BNReluConvolution0_weights.hex
#                          ${CURDIR}DORY_network/hex/BNReluConvolution1_weights.hex
#                          ${CURDIR}DORY_network/hex/BNReluConvolution2_weights.hex
#                          ${CURDIR}DORY_network/hex/BNReluConvolution3_weights.hex
#                          ${CURDIR}DORY_network/hex/BNReluConvolution4_weights.hex
#                          ${CURDIR}DORY_network/hex/BNReluConvolution5_weights.hex
#                          ${CURDIR}DORY_network/hex/BNReluConvolution6_weights.hex
#                          ${CURDIR}DORY_network/hex/BNReluConvolution7_weights.hex
#                          ${CURDIR}DORY_network/hex/BNReluConvolution8_weights.hex
#                          ${CURDIR}DORY_network/hex/FullyConnected10_weights.hex
#                          ${CURDIR}DORY_network/hex/inputs.hex
#                          ${WAV_FILE}
# )


set (UTTR_FILE /home/cioflanc/odda_gap9/tiny_denoiser/utterances.txt)

FILE(STRINGS ${UTTR_FILE} UTTERANCES)


# Working well for small files
# list(APPEND READFS_MRAM_FILES 

# list(APPEND READFS_FILES 
#                        ${UTTERANCES}
#     )



project (${TARGET_NAME} C ASM)

add_executable (${TARGET_NAME} ${TARGET_SRCS})



target_compile_options(${TARGET_NAME} PUBLIC   ${TARGET_CFLAGS}
                                               ${TARGET_INCS}
                                               ${TARGET_PREPROCESSOR}

)
target_link_libraries(${TARGET_NAME} PRIVATE m)
target_link_libraries(${TARGET_NAME} PRIVATE ${TILER_LIB})
list(APPEND TABLE_CFLAGS -lm)


# CMake post initialization

setupos(${TARGET_NAME})






