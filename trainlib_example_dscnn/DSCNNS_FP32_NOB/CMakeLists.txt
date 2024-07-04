cmake_minimum_required(VERSION 3.19.5)

# Application

set(TARGET_NAME         "dscnns_fp16")
set(MAIN                "main.c")

# Cmake pre-inits

# set(CMAKE_VERBOSE_MAKEFILE ON)

set(CONFIG_GAP_SDK_HOME $ENV{GAP_SDK_HOME})
include ($ENV{GAP_SDK_HOME}/utils/cmake/setup.cmake)
set(BOARD_NAME gap9_evk)
set(PMSIS_OS freertos)
set(platform $ENV{PLATFORM})

# kconfig options
set (io   host)
set (GAP_SDK 1)
set (DEMO 1)

set (FLASH_TYPE DEFAULT)
set (RAM_TYPE   DEFAULT)
set (MODEL_L3_FLASH   AT_MEM_L3_DEFAULTFLASH)
set (EXEC_FROM_FLASH true)
set (MODEL_L3_RAM   AT_MEM_L3_DEFAULTRAM)
set (FREQ_CL 370)
set (FREQ_FC 370)
set (FREQ_SFU 370)
# set (VOLTAGE 650)
set (VOLTAGE 800)
set (CLUSTER_STACK_SIZE 8192)
set (CLUSTER_SLAVE_STACK_SIZE 4096)
set (TOTAL_STACK_SIZE $(shell expr $(CLUSTER_STACK_SIZE) \+ $(CLUSTER_SLAVE_STACK_SIZE) \* $(CLUSTER_NUM_CORES)))
set (MODEL_L1_MEMORY $(shell expr 120000 \- $(TOTAL_STACK_SIZE)))
set (MODEL_L2_MEMORY 1000000)
set (MODEL_L3_MEMORY 8000000)




set (NUM_CORES           8)
set (MATMUL_TYPE_FW_L0   0)
set (MATMUL_TYPE_WG_L0   0)
set (MATMUL_TYPE_IG_L0   0)
set (NUM_MATMULS         24)


set(BUILD_DIR            ${CMAKE_CURRENT_SOURCE_DIR}/build)
set(TRAIN_LIB 			 ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(TRAIN_LIB_SRCS       ${TRAIN_LIB}/sources)

list (APPEND TARGET_INCS 
						 -I.
                         -I${TRAIN_LIB}/include
                         -I${GAP_SDK_HOME}/tools/autotiler_v3/Emulation # cycle counter
                         -I${GAP_SDK_HOME}/libs/gap_lib/include
                         -I${GAP_SDK_HOME}/rtos/pmsis/bsp/include/bsp
                         -I${GAP_SDK_HOME}/rtos/pmsis/bsp/include/bsp/ram
                         -I${GAP_SDK_HOME}/rtos/pmsis/bsp/include/bsp/flash
                         -I${GAP_SDK_HOME}/rtos/sfu/include
)

list (APPEND TARGET_CFLAGS 
                            -O3
                            -g3
                            -s
                            -w
                            -mhwloopalign
                            -mno-memcpy
                            -fno-tree-loop-distribute-patterns
                            -Wno-incompatible-pointer-types
)


list(APPEND TARGET_PREPROCESSOR
							-DPROF_NET
							-DFABRIC
							-DCLUSTER
							-DNUM_CORES=${NUM_CORES}
							-DMATMUL_TYPE_FW_L0=${MATMUL_TYPE_FW_L0}
							-DMATMUL_TYPE_WG_L0=${MATMUL_TYPE_WG_L0}
							-DMATMUL_TYPE_IG_L0=${MATMUL_TYPE_IG_L0}
							-DSTATS

							-DGAP_SDK=${GAP_SDK}
							-DCONFIG_GAP9_EVK
							-DTARGET_CHIP_FAMILY_GAP9

							-DPROF_NET
							# -DDEBUG							


)



FILE (GLOB TRAINLIB_C_Sources ${TRAIN_LIB_SRCS}/*.c)
list (APPEND TARGET_SRCS ${TRAINLIB_C_Sources})

list (APPEND TARGET_SRCS
						    ${MAIN}
						    net.c

)


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
