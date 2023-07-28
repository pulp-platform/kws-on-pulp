PULP_APP = flasher
PULP_APP_FC_SRCS = src/flasher.c

ifndef CORE
CORE=1
else
CORE= $(CORE)
endif

PULP_CFLAGS += -DNUM_CORES=$(CORE) -Iinc -O3 -g3  -fno-tree-loop-distribute-patterns -flto
PULP_LDFLAGS = -lm -lg -flto

include $(PULP_SDK_HOME)/install/rules/pulp_rt.mk