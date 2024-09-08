
ASM=nasm
ASM_FLAG = -f elf32

GCC=gcc
GCC_FLAG = -g -m32 -fno-stack-protector -fno-builtin	# -g flag used for debugging info

SRC_DIR=src
BUILD_DIR=build

OS_NAME=KeblaOS
VERSION=0.0.0.6

default: run

# Generating bootloader.bin file in build directory
$(BUILD_DIR)/bootloader.bin: $(SRC_DIR)/bootloader.s
	$(ASM) -f bin $(SRC_DIR)/bootloader.s -o $(BUILD_DIR)/bootloader.bin


# Generating bootloader.o object file in build directory
#$(BUILD_DIR)/bootloader.o: $(SRC_DIR)/bootloader.s
#	$(ASM) $(ASM_FLAG) $(SRC_DIR)/bootloader.s -o $(BUILD_DIR)/bootloader.o

# Generating KeblaOS-0.0.0.1.iso Iso image file
#$(BUILD_DIR)/$(OS_NAME)-$(VERSION).iso: $(BUILD_DIR)/bootloader.o
#	$(GCC) $(GCC_FLAG) -o $(BUILD_DIR)/$(OS_NAME)-$(VERSION).iso $(BUILD_DIR)/bootloader.o


# Running ISO in QEmu
# run: $(BUILD_DIR)/$(OS_NAME)-$(VERSION).iso
#	qemu-system-i386 -cdrom $(BUILD_DIR)/$(OS_NAME)-$(VERSION).iso # -s -S flag for debuging

# Running Binary File in QEmu
run: $(BUILD_DIR)/bootloader.bin 
	qemu-system-i386 $(BUILD_DIR)/bootloader.bin


clean:
	rm -f $(BUILD_DIR)/*.o


