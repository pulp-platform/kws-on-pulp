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

DSP_GEN_DIR ?= $(GAP9_SDK_DIR)/tools/autotiler_v3/DSP_Generators/
MFCC_SRCG ?= $(GAP9_SDK_DIR)/tools/autotiler_v3/DSP_Generators/DSP_Generators.c
MFCC_MODEL_GEN = $(MFCCBUILD_DIR)/GenMFCC
MFCC_HEAD = $(MFCCBUILD_DIR)/MFCC_params.h
MFCC_PARAMS_JSON ?= $(CURDIR)/MfccConfig.json
MFCC_SRC_CODE = $(MFCCBUILD_DIR)/MfccKernels.c

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

$(MFCC_HEAD): $(MFCC_PARAMS_JSON) | $(MFCCBUILD_DIR)
	python $(DSP_GEN_DIR)/DSP_LUTGen.py $(MFCC_PARAMS_JSON) --build_dir $(MFCCBUILD_DIR) --save_params_header $(MFCC_HEAD) --save_text

# # Build the code generator from the model code
$(MFCC_MODEL_GEN): $(MFCC_HEAD) | $(MFCCBUILD_DIR)
	gcc -g -o $(MFCC_MODEL_GEN) -I$(MFCCBUILD_DIR) -I$(DSP_GEN_DIR) -I$(TILER_INC) -I$(TILER_EMU_INC) $(CURDIR)/MfccModel.c $(MFCC_SRCG) $(TILER_LIB) $(TABLE_CFLAGS) $(COMPILE_MODEL_EXTRA_FLAGS) -DUSE_POWER=$(USE_POWER) $(SDL_FLAGS)

# Run the code generator  kernel code
$(MFCC_SRC_CODE): $(MFCC_MODEL_GEN) | $(MFCCBUILD_DIR)
	$(MFCC_MODEL_GEN) -o $(MFCCBUILD_DIR) -c $(MFCCBUILD_DIR) $(MODEL_GEN_EXTRA_FLAGS)

gen_mfcc_code: $(MFCC_SRC_CODE)

clean_mfcc_code:
	rm -rf $(MFCCBUILD_DIR)

.PHONY: gen_mfcc_code clean_mfcc_code

