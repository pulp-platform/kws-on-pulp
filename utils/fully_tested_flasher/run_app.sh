export PULP_RISCV_GCC_TOOLCHAIN=/usr/scratch/wetterhorn/cioflanc/tools/pulp_riscv_toolchain/v1.0.16-pulp-riscv-gcc-centos-7/
# source /usr/scratch/wetterhorn/cioflanc/kws_on_fpga/local_deployment/pulp-sdk/configs/pulp.sh
export PULP_SDK_HOME=/usr/scratch/wetterhorn/cioflanc/kws_on_fpga/local_deployment/pulp-runtime
# export PULP_SDK_HOME=/usr/scratch/wetterhorn/cioflanc/tools/pulp_sdk_fpga/pulp-sdk/

source /usr/scratch/wetterhorn/cioflanc/kws_on_fpga/local_deployment/pulp-sdk/configs/pulp.sh
source /usr/scratch/wetterhorn/cioflanc/kws_on_fpga/local_deployment/pulp-sdk/sourceme.sh
FLASHER_FOLDER=$(pwd)

export TCL=$FLASHER_FOLDER/tcl
export APP_FOLDER=$FLASHER_FOLDER/test
export BOARD_CONFIG=$FLASHER_FOLDER/script/openocd-zcu102-digilent-jtag-hs2.cfg
export IMAGE_FOLDER=$FLASHER_FOLDER/components

#rm -rf $FLASHER_FOLDER/Image.hex

#python3 $FLASHER_FOLDER/script/PULP_FlashImageBuilder.py

IMAGE=$FLASHER_FOLDER/Image.hex

export IMAGE_SIZE=$(stat -c %s "$IMAGE")

make clean all io=uart

cd $APP_FOLDER

make clean all io=uart

cd $FLASHER_FOLDER

../openocd-bologna/bin/openocd -f $BOARD_CONFIG -c "script $TCL/flash_image.tcl; pulp_flash_raw $IMAGE $IMAGE_SIZE $FLASHER_FOLDER; exit;"
