FLASH_FILES += DORY_network/hex/BNReluConvolution0_weights.hex
FLASH_FILES += DORY_network/hex/BNReluConvolution1_weights.hex
FLASH_FILES += DORY_network/hex/BNReluConvolution2_weights.hex
FLASH_FILES += DORY_network/hex/BNReluConvolution3_weights.hex
FLASH_FILES += DORY_network/hex/BNReluConvolution4_weights.hex
FLASH_FILES += DORY_network/hex/BNReluConvolution5_weights.hex
FLASH_FILES += DORY_network/hex/BNReluConvolution6_weights.hex
FLASH_FILES += DORY_network/hex/BNReluConvolution7_weights.hex
FLASH_FILES += DORY_network/hex/BNReluConvolution8_weights.hex
FLASH_FILES += DORY_network/hex/FullyConnected10_weights.hex
FLASH_FILES += DORY_network/hex/inputs.hex
FLASH_FILES += $(WAV_FILE)

READFS_FILES := $(FLASH_FILES)
APP_CFLAGS += -DFS_READ_FS
#PLPBRIDGE_FLAGS += -f