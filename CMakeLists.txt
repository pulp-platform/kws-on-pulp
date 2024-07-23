cmake_minimum_required(VERSION 3.19.5)

# Application

set(TARGET_NAME         "application")
set(MAIN                "application.c")

# Cmake pre-inits

# set(CMAKE_VERBOSE_MAKEFILE ON)

set(CONFIG_GAP_SDK_HOME $ENV{GAP_SDK_HOME})
include ($ENV{GAP_SDK_HOME}/utils/cmake/setup.cmake)
set(BOARD_NAME gap9_evk)
set(PMSIS_OS freertos)
set(platform $ENV{PLATFORM})

# kconfig options

set (io   host)
set (DEMO 1)

set (FLASH_TYPE DEFAULT)
set (RAM_TYPE   DEFAULT)
set (MODEL_L3_FLASH   AT_MEM_L3_DEFAULTFLASH)
set (EXEC_FROM_FLASH true)
set (MODEL_L3_RAM   AT_MEM_L3_DEFAULTRAM)

set (FREQ_CL 50) # DEMO
set (FREQ_FC 50) # DEMO
set (FREQ_SFU 50) # DEMO
# set (FREQ_CL 370) # MEASUREMENT
# set (FREQ_FC 370) # MEASUREMENT
# set (FREQ_SFU 370) # MEASUREMENT
# set (VOLTAGE 650)
set (VOLTAGE 800)
set (CLUSTER_STACK_SIZE 8192)
set (CLUSTER_SLAVE_STACK_SIZE 4096)
set (TOTAL_STACK_SIZE $(shell expr $(CLUSTER_STACK_SIZE) \+ $(CLUSTER_SLAVE_STACK_SIZE) \* $(CLUSTER_NUM_CORES)))
set (MODEL_L1_MEMORY $(shell expr 120000 \- $(TOTAL_STACK_SIZE)))
set (MODEL_L2_MEMORY 1000000)
set (MODEL_L3_MEMORY 8000000)

set (WAV_FILE $ENV{WAV_FILE})
set (MFCC $ENV{MFCC})
set (NOISE_EVAL $ENV{NOISE_EVAL})
set (UTTR_EVAL $ENV{UTTR_EVAL})
set (APPL $ENV{APPL})

# PULP trainlib
set(MATMUL_TYPE_FW_L0  0)         # Selects which optimized matmul to be used in FW (see mm_manager_list.txt or "MM_manager()" body to verify which one is called)
set(MATMUL_TYPE_WG_L0  0)         # Selects which optimized matmul to be used in WEIGHT GRAD (see mm_manager_list.txt or "MM_manager()" body to verify which one is called)
set(MATMUL_TYPE_IG_L0  0)         # Selects which optimized matmul to be used in IN GRAD (see mm_manager_list.txt or "MM_manager()" body to verify which one is called)

set (NUM_MATMULS 24)


set(GAP9_SDK_DIR         $ENV{GAP_SDK_HOME})
set(BUILD_DIR            ${CMAKE_CURRENT_SOURCE_DIR}/build)
set(DSP_DIR              ${GAP9_SDK_DIR}/tools/autotiler_v3/DSP_Libraries/)
set(DSP_LUT_DIR          ${GAP9_SDK_DIR}/tools/autotiler_v3/DSP_Libraries/LUT_Tables/)
set(EMUL_DIR             ${GAP9_SDK_DIR}/tools/autotiler_v3/Emulation/)
set(AUTOTILER_DIR        ${GAP9_SDK_DIR}/tools/autotiler_v3/Autotiler/)
set(AUTOTILER_MFCC_DIR   ${GAP9_SDK_DIR}/tools/autotiler_v3/Generators/MFCC/)
set(WAVIO_DIR            ${GAP9_SDK_DIR}/libs/gap_lib/include/)
set(DSP_GEN_DIR          ${GAP9_SDK_DIR}/tools/autotiler_v3/DSP_Generators/)
set(MFCC_SRCG            ${GAP9_SDK_DIR}/tools/autotiler_v3/DSP_Generators/DSP_Generators.c)
set(MFCC_MODEL_GEN       ${BUILD_DIR}/GenMFCC)
set(MFCC_HEAD            ${BUILD_DIR}/MFCC_params.h)
# set(MFCC_PARAMS_JSON   ${CMAKE_CURRENT_SOURCE_DIR}/MfccConfigLibrosa.json)
set(MFCC_PARAMS_JSON     ${CMAKE_CURRENT_SOURCE_DIR}/MfccConfigTF.json)
set(MFCC_SRC_CODE        ${BUILD_DIR}/MfccKernels.c)


set (NCHANNELS 64)
set(TRAINLIB_DIR         ${CMAKE_CURRENT_SOURCE_DIR}/DSCNNS_DIR_DEST_NEMO_FP32)
set(DORY_DIR             ${CMAKE_CURRENT_SOURCE_DIR}/DSCNNS_DIR_DEST_NEMO)

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
    COMMAND              gcc -g -o ${MFCC_MODEL_GEN} -I${BUILD_DIR} -I${DSP_GEN_DIR} -I${TILER_INC} -I${TILER_EMU_INC} ${CMAKE_CURRENT_SOURCE_DIR}/MfccModel.c ${MFCC_SRCG} ${TILER_LIB} ${TABLE_CFLAGS} ${COMPILE_MODEL_EXTRA_FLAGS} -DUSE_POWER=${USE_POWER} ${SDL_FLAGS}
    BYPRODUCTS           ${MFCC_MODEL_GEN}   
    DEPENDS              ${MFCC_HEAD}         
)


add_custom_target(
    graph
    WORKING_DIRECTORY   ${BUILD_DIR}
    COMMAND             SFU -i ${CMAKE_CURRENT_SOURCE_DIR}/Graph.src -C
    SOURCES             ${CMAKE_CURRENT_SOURCE_DIR}/Graph.src
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
                         acquire.c
                         preprocess.c
                         train.c
                         ${BUILD_DIR}/Graph_L2_Descr.c
                         ${BUILD_DIR}/L2_Descr_Table.c
                         ${GAP9_SDK_DIR}/rtos/sfu/SFU_RT.c
                         ${GAP9_SDK_DIR}/rtos/sfu/sfu_pmsis_runtime.c
                         ${GAP_SDK_HOME}/tools/audio-framework/runtime/include
)


list (APPEND TARGET_INCS 
                         -I${TILER_INC}
                         -I${TILER_EMU_INC}
                         -I${GAP_SDK_HOME}/libs/gap_lib/include
                         -I${DORY_DIR}/inc
                         -I${GAP9_SDK_DIR}/rtos/pmsis/bsp/include/bsp
                         -I${GAP9_SDK_DIR}/rtos/pmsis/bsp/include/bsp/ram
                         -I${GAP9_SDK_DIR}/rtos/pmsis/bsp/include/bsp/flash
                         -I${GAP9_SDK_DIR}/rtos/sfu/include
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

FILE (GLOB DORY_C_Sources ${DORY_DIR}/src/*.c)
list (APPEND TARGET_SRCS ${DORY_C_Sources})
list (REMOVE_ITEM TARGET_SRCS ${DORY_DIR}/src/main.c) # Incompatible declaration

# TRAINLIB

FILE (GLOB TRAINLIB_C_Sources ${TRAINLIB_DIR}/lib/sources/*.c)
list (APPEND TARGET_SRCS ${TRAINLIB_C_Sources})
list (REMOVE_ITEM TARGET_SRCS ${TRAINLIB_DIR}/lib/sources/pulp_residual_fp16.c) # Incompatible declaration
list (REMOVE_ITEM TARGET_SRCS ${TRAINLIB_DIR}/lib/sources/pulp_residual_fp32.c) # Incompatible declaration
list (APPEND TARGET_SRCS ${TRAINLIB_DIR}/net.c)

list(APPEND TARGET_CFLAGS   -O2
                            -s
                            -w
                            -mno-memcpy
                            -fno-tree-loop-distribute-patterns
                            -Wno-incompatible-pointer-types
)

list(APPEND TARGET_PREPROCESSOR # -DPERF
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
                                -DNOISE_EVAL=${NOISE_EVAL}
                                -DUTTR_EVAL=${UTTR_EVAL}
                                -DAPPL=${APPL}
                                -DNCHANNELS=${NCHANNELS}
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
                                -DFABRIC
                                -DCLUSTER
                                # -DDEBUG
                                # -DPROF_NET
                                -mhwloopalign
                                # -DSTATS
                                -DMATMUL_TYPE_FW_L0=${MATMUL_TYPE_FW_L0}
                                -DMATMUL_TYPE_WG_L0=${MATMUL_TYPE_WG_L0}
                                -DMATMUL_TYPE_IG_L0=${MATMUL_TYPE_IG_L0}

)

# HOME
# set (UTTR_FILE /home/cioflanc/odda_gap9/tiny_denoiser/res/utterances.txt)
# set (TINYTEST_FILE /home/cioflanc/odda_gap9/tiny_denoiser/res/tinytest.txt)
# set (RESTAURANT_NOISE_WAV /home/cioflanc/odda_gap9/tiny_denoiser/res/meeting_ch01_mancrop1.wav)

# WORK
set (VALIDATION_FILE /usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/validation.txt)
set (UTTR_FILE /usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/utterances.txt)
set (TINYTEST_FILE /usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/tinytest.txt)
set (RESTAURANT_NOISE_WAV /usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/meeting_ch01_mancrop1.wav)


FILE(STRINGS ${UTTR_FILE} UTTERANCES)
FILE(STRINGS ${TINYTEST_FILE} TINYTEST_UTTR)
FILE(STRINGS ${VALIDATION_FILE} VALIDATION_UTTR)


# FLASH
FILE(GLOB DORY_WEIGHTS ${DORY_DIR}/hex/*.hex)
list (APPEND READFS_FILES ${DORY_WEIGHTS})
list(APPEND READFS_FILES ${WAV_FILE}
                         ${RESTAURANT_NOISE_WAV}
                         # ${VALIDATION_UTTR}
                         # ${UTTERANCES} 
                         # ${TINYTEST_UTTR}
                        
)


project (${TARGET_NAME} C ASM)

add_executable (${TARGET_NAME} ${TARGET_SRCS})


#===============================================================================
# SFU graph generation
#===============================================================================

sfu_add_graphs(${TARGET_NAME} SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/Graph.src)


target_compile_options(${TARGET_NAME} PUBLIC   ${TARGET_CFLAGS}
                                               ${TARGET_INCS}
                                               ${TARGET_PREPROCESSOR}

)
target_link_libraries(${TARGET_NAME} PRIVATE m)
target_link_libraries(${TARGET_NAME} PRIVATE ${TILER_LIB})
list(APPEND TABLE_CFLAGS -lm)

# CMake post initialization

setupos(${TARGET_NAME})






