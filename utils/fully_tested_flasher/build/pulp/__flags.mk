PULP_LDFLAGS      += 
PULP_CFLAGS       += 
PULP_CL_ARCH_CFLAGS ?=  -march=rv32imcxgap9 -mPE=8 -mFC=1 -D__riscv__
PULP_CL_CFLAGS    += -fdata-sections -ffunction-sections -I/usr/scratch/wetterhorn/cioflanc/kws_on_fpga/local_deployment/pulp-sdk/pkg/sdk/dev/install/include/io -I/usr/scratch/wetterhorn/cioflanc/kws_on_fpga/local_deployment/pulp-sdk/pkg/sdk/dev/install/include -include /usr/scratch/wetterhorn/cioflanc/kws_on_fpga/local_deployment/fully_tested_flasher/build/pulp/cl_config.h
PULP_CL_OMP_CFLAGS    += -fopenmp -mnativeomp
ifdef PULP_RISCV_GCC_TOOLCHAIN
PULP_CL_CC = $(PULP_RISCV_GCC_TOOLCHAIN)/bin/riscv32-unknown-elf-gcc 
PULP_CC = $(PULP_RISCV_GCC_TOOLCHAIN)/bin/riscv32-unknown-elf-gcc 
PULP_AR ?= $(PULP_RISCV_GCC_TOOLCHAIN)/bin/riscv32-unknown-elf-ar
PULP_LD ?= $(PULP_RISCV_GCC_TOOLCHAIN)/bin/riscv32-unknown-elf-gcc
PULP_CL_OBJDUMP ?= $(PULP_RISCV_GCC_TOOLCHAIN)/bin/riscv32-unknown-elf-objdump
PULP_OBJDUMP ?= $(PULP_RISCV_GCC_TOOLCHAIN)/bin/riscv32-unknown-elf-objdump
else
PULP_CL_CC = $(PULP_RISCV_GCC_TOOLCHAIN_CI)/bin/riscv32-unknown-elf-gcc 
PULP_CC = $(PULP_RISCV_GCC_TOOLCHAIN_CI)/bin/riscv32-unknown-elf-gcc 
PULP_AR ?= $(PULP_RISCV_GCC_TOOLCHAIN_CI)/bin/riscv32-unknown-elf-ar
PULP_LD ?= $(PULP_RISCV_GCC_TOOLCHAIN_CI)/bin/riscv32-unknown-elf-gcc
PULP_CL_OBJDUMP ?= $(PULP_RISCV_GCC_TOOLCHAIN_CI)/bin/riscv32-unknown-elf-objdump
PULP_OBJDUMP ?= $(PULP_RISCV_GCC_TOOLCHAIN_CI)/bin/riscv32-unknown-elf-objdump
endif
PULP_ARCH_CL_OBJDFLAGS ?= -Mmarch=rv32imcxgap9
PULP_ARCH_OBJDFLAGS ?= -Mmarch=rv32imcxgap9
PULP_FC_ARCH_CFLAGS ?=  -march=rv32imcxgap9 -mPE=8 -mFC=1 -D__riscv__
PULP_FC_CFLAGS    += -fdata-sections -ffunction-sections -I/usr/scratch/wetterhorn/cioflanc/kws_on_fpga/local_deployment/pulp-sdk/pkg/sdk/dev/install/include/io -I/usr/scratch/wetterhorn/cioflanc/kws_on_fpga/local_deployment/pulp-sdk/pkg/sdk/dev/install/include -include /usr/scratch/wetterhorn/cioflanc/kws_on_fpga/local_deployment/fully_tested_flasher/build/pulp/fc_config.h
PULP_FC_OMP_CFLAGS    += -fopenmp -mnativeomp
ifdef PULP_RISCV_GCC_TOOLCHAIN
PULP_FC_CC = $(PULP_RISCV_GCC_TOOLCHAIN)/bin/riscv32-unknown-elf-gcc 
PULP_CC = $(PULP_RISCV_GCC_TOOLCHAIN)/bin/riscv32-unknown-elf-gcc 
PULP_AR ?= $(PULP_RISCV_GCC_TOOLCHAIN)/bin/riscv32-unknown-elf-ar
PULP_LD ?= $(PULP_RISCV_GCC_TOOLCHAIN)/bin/riscv32-unknown-elf-gcc
PULP_FC_OBJDUMP ?= $(PULP_RISCV_GCC_TOOLCHAIN)/bin/riscv32-unknown-elf-objdump
else
PULP_FC_CC = $(PULP_RISCV_GCC_TOOLCHAIN_CI)/bin/riscv32-unknown-elf-gcc 
PULP_CC = $(PULP_RISCV_GCC_TOOLCHAIN_CI)/bin/riscv32-unknown-elf-gcc 
PULP_AR ?= $(PULP_RISCV_GCC_TOOLCHAIN_CI)/bin/riscv32-unknown-elf-ar
PULP_LD ?= $(PULP_RISCV_GCC_TOOLCHAIN_CI)/bin/riscv32-unknown-elf-gcc
PULP_FC_OBJDUMP ?= $(PULP_RISCV_GCC_TOOLCHAIN_CI)/bin/riscv32-unknown-elf-objdump
endif
PULP_ARCH_FC_OBJDFLAGS ?= -Mmarch=rv32imcxgap9
PULP_ARCH_LDFLAGS ?=  -march=rv32imcxgap9 -mPE=8 -mFC=1 -D__riscv__
PULP_LDFLAGS_flasher = -nostartfiles -nostdlib -Wl,--gc-sections -L/usr/scratch/wetterhorn/cioflanc/kws_on_fpga/local_deployment/pulp-sdk/pkg/sdk/dev/install/rules -Tpulp/link.ld -L/usr/scratch/wetterhorn/cioflanc/kws_on_fpga/local_deployment/pulp-sdk/pkg/sdk/dev/install/lib/pulp -L/usr/scratch/wetterhorn/cioflanc/kws_on_fpga/local_deployment/pulp-sdk/pkg/sdk/dev/install/lib/pulp/pulp_genesys2 -lrt -lrtio -lrt -lgcc
PULP_OMP_LDFLAGS_flasher = -lomp
pulpRunOpt        += --dir=/usr/scratch/wetterhorn/cioflanc/kws_on_fpga/local_deployment/fully_tested_flasher/build/pulp --binary=flasher/flasher
