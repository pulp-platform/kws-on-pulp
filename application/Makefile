# Makefile
# Alessio Burrello <alessio.burrello@unibo.it>
#
# Copyright (C) 2019-2020 University of Bologna
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0
#


MFCCBUILD_DIR ?= $(CURDIR)/BUILD_MFCC
DSP_DIR ?= $(GAP_SDK_DIR)tools/autotiler_v3/DSP_Libraries/
AUTOTILER_DIR ?= $(GAP_SDK_DIR)tools/autotiler_v3/Autotiler/
DSP_LUT_DIR ?= $(GAP_SDK_DIR)tools/autotiler_v3/DSP_Libraries/LUT_Tables/
WAVIO_DIR ?= $(GAP_SDK_DIR)libs/gap_lib/include/
WAVIO_SRC ?= $(GAP_SDK_DIR)libs/gap_lib/wav_io/

ifeq '$(TARGET_CHIP_FAMILY)' 'GAP8'
	COMPILE_MODEL_EXTRA_FLAGS += -DNO_FLOAT16
endif

WAV_FILE = $(sample)
SDK      = $(sdk)

include $(CURDIR)/MfccModel.mk
clean:: clean_mfcc_code
all:: $(MFCCBUILD_DIR)/MFCCKernels.c

APP = main
APP_SRCS = DORY_network/src/mem_controller.c
APP_SRCS+=DORY_network/src/dory.c
APP_SRCS+=DORY_network/src/pulp_nn_add.c
APP_SRCS+=DORY_network/src/pulp_nn_avgpool.c
APP_SRCS+=DORY_network/src/pulp_nn_conv_Co_parallel.c
APP_SRCS+=DORY_network/src/pulp_nn_conv_Ho_parallel.c
APP_SRCS+=DORY_network/src/pulp_nn_conv_HoWo_parallel.c
APP_SRCS+=DORY_network/src/pulp_nn_depthwise_3x3_s1.c
APP_SRCS+=DORY_network/src/pulp_nn_depthwise_generic.c
APP_SRCS+=DORY_network/src/pulp_nn_depthwise_generic_less_4_weights.c
APP_SRCS+=DORY_network/src/pulp_nn_linear.c
APP_SRCS+=DORY_network/src/pulp_nn_linear_out_32.c
APP_SRCS+=DORY_network/src/pulp_nn_matmul.c
APP_SRCS+=DORY_network/src/pulp_nn_maxpool.c
APP_SRCS+=DORY_network/src/pulp_nn_pointwise_Co_parallel.c
APP_SRCS+=DORY_network/src/pulp_nn_pointwise_Ho_parallel.c
APP_SRCS+=DORY_network/src/pulp_nn_pointwise_HoWo_parallel.c
APP_SRCS+=DORY_network/src/pulp_nn_utils.c
APP_SRCS+=DORY_network/src/layerConvBNRelu0.c
APP_SRCS+=DORY_network/src/layerConvDWBNRelu1.c
APP_SRCS+=DORY_network/src/layerConvBNRelu2.c
APP_SRCS+=DORY_network/src/layerAveragePoolRelu9.c
APP_SRCS+=DORY_network/src/layerGemm10_last.c
APP_SRCS+=DORY_network/src/network.c


APP_SRCS        += $(CURDIR)/main.c $(MFCCBUILD_DIR)/MFCCKernels.c $(DSP_LUT_DIR)TwiddlesDef.c $(DSP_LUT_DIR)RFFTTwiddlesDef.c $(DSP_LUT_DIR)SwapTablesDef.c
APP_SRCS        += $(DSP_DIR)MfccBasicKernels.c $(DSP_DIR)FFT_Library.c $(DSP_DIR)math_funcs.c $(DSP_DIR)CmplxFunctions.c $(DSP_DIR)PreProcessing.c 
APP_SRCS        += $(WAVIO_SRC)wavIO.c

ifndef CORE
CORE=1
else
CORE= $(CORE)
endif

ifeq ($(USE_ABS), 1)
	APP_CFLAGS += -DUSE_ABS
endif
ifeq ($(USE_HIGH_PREC), 1)
	APP_CFLAGS += -DHIGH_PREC_FFT
endif

APP_CFLAGS      += -DAT_WAV=$(WAV_FILE) -DSDK=$(SDK) -DPERF -DPRINT_INOUT -DPRINTDEB 
APP_CFLAGS      += -I. -I$(MFCCBUILD_DIR) -I$(DSP_DIR) -I$(DSP_LUT_DIR) -I$(AUTOTILER_DIR) -I$(WAVIO_DIR) 

PULP_SDK_STR = pulp_sdk
ifeq ($(SDK), $(PULP_SDK_STR))
	APP_CFLAGS += -DNUM_CORES=$(CORE) -DPULP_SDK=1 -IDORY_network/inc -O3 -w -fno-tree-loop-distribute-patterns -flto
	APP_CFLAGS += -I$(GAP_SDK_DIR)tools/autotiler_v3/Emulation 
else
	APP_CFLAGS += -DNUM_CORES=$(CORE) -DGAP_SDK=1 -IDORY_network/inc -O3 -w -fno-tree-loop-distribute-patterns -flto
endif

APP_LDFLAGS += -lm -flto

CONFIG_HYPERRAM = 1
CONFIG_HYPERFLASH = 1

FLASH_FILES = DORY_network/ConvBNRelu0_weights.hex
FLASH_FILES += DORY_network/ConvDWBNRelu1_weights.hex
FLASH_FILES += DORY_network/ConvBNRelu2_weights.hex
FLASH_FILES += DORY_network/ConvDWBNRelu3_weights.hex
FLASH_FILES += DORY_network/ConvBNRelu4_weights.hex
FLASH_FILES += DORY_network/ConvDWBNRelu5_weights.hex
FLASH_FILES += DORY_network/ConvBNRelu6_weights.hex
FLASH_FILES += DORY_network/ConvDWBNRelu7_weights.hex
FLASH_FILES += DORY_network/ConvBNRelu8_weights.hex
FLASH_FILES += DORY_network/Gemm10_weights.hex
FLASH_FILES += DORY_network/inputs.hex
READFS_FILES := $(FLASH_FILES)
PLPBRIDGE_FLAGS += -f

include $(RULES_DIR)/pmsis_rules.mk
