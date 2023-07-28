#
# Copyright (C) 2020 GreenWaves Technologies
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

DSP_GEN_DIR ?= $(GAP_SDK_DIR)/tools/autotiler_v3/DSP_Generators/
EMUL_DIR ?= $(GAP_SDK_DIR)tools/autotiler_v3/Emulation/
MFCC_MODEL_GEN = $(MFCCBUILD_DIR)/GenMFCC
FFT_LUT = $(MFCCBUILD_DIR)/LUT.def
MFCC_LUT = $(MFCCBUILD_DIR)/MFCC_FB.def
MFCC_HEAD = $(MFCCBUILD_DIR)/MFCC_params.h

# Everything bellow is not application specific
TABLE_CFLAGS=-lm

#SDL_FLAGS= -lSDL2 -lSDL2_ttf -DAT_DISPLAY
CLUSTER_STACK_SIZE?=2048
CLUSTER_SLAVE_STACK_SIZE?=1024
TOTAL_STACK_SIZE = $(shell expr $(CLUSTER_STACK_SIZE) \+ $(CLUSTER_SLAVE_STACK_SIZE) \* 7)
ifeq '$(TARGET_CHIP_FAMILY)' 'GAP9'
	MODEL_L1_MEMORY=$(shell expr 125000 \- $(TOTAL_STACK_SIZE))
else
	MODEL_L1_MEMORY=$(shell expr 60000 \- $(TOTAL_STACK_SIZE))
endif
ifdef MODEL_L1_MEMORY
  MODEL_GEN_EXTRA_FLAGS += --L1 $(MODEL_L1_MEMORY)
endif
ifdef MODEL_L2_MEMORY
  MODEL_GEN_EXTRA_FLAGS += --L2 $(MODEL_L2_MEMORY)
endif
ifdef MODEL_L3_MEMORY
  MODEL_GEN_EXTRA_FLAGS += --L3 $(MODEL_L3_MEMORY)
endif

USE_POWER?=1

$(MFCCBUILD_DIR):
	mkdir $(MFCCBUILD_DIR)

# Build the code generator from the model code
$(MFCC_MODEL_GEN): $(MFCCBUILD_DIR)
	gcc -g -o $(MFCC_MODEL_GEN) -I. -I$(CURDIR) -I$(AUTOTILER_DIR) -I$(EMUL_DIR) -I$(DSP_GEN_DIR) -I$(DSP_DIR) -I$(MFCCBUILD_DIR) \
	$(CURDIR)/MfccModel.c $(DSP_GEN_DIR)DSP_Generators.c $(AUTOTILER_DIR)/LibTile.a $(TABLE_CFLAGS) $(COMPILE_MODEL_EXTRA_FLAGS) -DUSE_POWER=$(USE_POWER)

$(MFCC_LUT): $(MFCCBUILD_DIR)
	python $(DSP_LUT_DIR)gen_scripts/GenMFCCLUT.py --fft_lut_file $(FFT_LUT) --mfcc_bf_lut_file $(MFCC_LUT) \
	--save_params_header $(MFCC_HEAD) --sample_rate 16000 --frame_size 640 --frame_step 320 \
	--n_fft 1024 --n_dct 40 --mfcc_bank_cnt 40 --fmin 20 --fmax 4000 --use_tf_mfcc --dtype fix16

# Run the code generator kernel code
$(MFCCBUILD_DIR)/MFCCKernels.c: $(MFCC_LUT) $(MFCC_MODEL_GEN)
	$(MFCC_MODEL_GEN) -o $(MFCCBUILD_DIR) -c $(MFCCBUILD_DIR) $(MODEL_GEN_EXTRA_FLAGS)

clean_mfcc_code:
	rm -rf $(MFCCBUILD_DIR)
