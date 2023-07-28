PULP_APP = main

PULP_APP_FC_SRCS = src/network.c src/test_template.c src/dory.c 
PULP_APP_FC_SRCS += src/pulp_nn_conv_i8_u8.c src/pulp_nn_matmul_4x2_i8_u8.c src/pulp_nn_relu_u8.c src/pulp_nn_add_u8_u8.c
PULP_APP_FC_SRCS += src/pulp_nn_dw_conv_i8_u8.c src/pulp_nn_conv_i8_u8_first.c src/pulp_nn_quant_u8.c src/pulp_nn_bn_quant_u8.c 
PULP_APP_FC_SRCS += src/pulp_nn_avgpool_u8.c src/pulp_nn_maxpool_u8.c src/dw_fast_C_parallel_int8.c
PULP_APP_FC_SRCS += src/utils.c
PULP_APP_FC_SRCS += src/mem_controller.c

PULP_APP_FC_SRCS += src/layerConvBNRelu0.c
PULP_APP_FC_SRCS += src/layerConvBNRelu6.c
PULP_APP_FC_SRCS += src/layerConvBNRelu4.c
PULP_APP_FC_SRCS += src/layerConvBNRelu2.c
PULP_APP_FC_SRCS += src/layerConvDWBNRelu7.c
PULP_APP_FC_SRCS += src/layerConvDWBNRelu3.c
PULP_APP_FC_SRCS += src/layerConvDWBNRelu9.c
PULP_APP_FC_SRCS += src/layerConvDWBNRelu5.c
PULP_APP_FC_SRCS += src/layerConvDWBNRelu1.c
PULP_APP_FC_SRCS += src/layerConvBNRelu8.c
ifndef CORE
CORE=1
else
CORE= $(CORE)
endif

PULP_CFLAGS += -DNUM_CORES=$(CORE) -Iinc -O3 -g3  -fno-tree-loop-distribute-patterns -flto
PULP_LDFLAGS = -lm -lg -flto
 
include $(PULP_SDK_HOME)/install/rules/pulp_rt.mk
