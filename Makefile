
ASM=nasm
ASM_FLAG = -f elf32

GCC=gcc
GCC_FLAG = -g -m32 -fno-stack-protector -fno-builtin	# -g flag used for debugging info

SRC_DIR=src
BUILD_DIR=build

OS_NAME=KeblaOS
VERSION=0.0.0.8

default: run

# Generating bootsect.bin file in build directory
$(BUILD_DIR)/bootsect.o: $(SRC_DIR)/bootsect.asm
	$(ASM) $(ASM_FLAG) $(SRC_DIR)/bootsect.asm -o $(BUILD_DIR)/bootsect.o


# Generating kernel.o object file in build directory
$(BUILD_DIR)/kernel.o: $(SRC_DIR)/kernel.c
	gcc -m32 -fno-stack-protector -fno-builtin -c $(SRC_DIR)/kernel.c -o $(BUILD_DIR)/kernel.o

# Generating KeblaOS-0.0.0.8.iso Iso image file
$(BUILD_DIR)/$(OS_NAME)-$(VERSION).iso: $(BUILD_DIR)/bootsect.o $(BUILD_DIR)/kernel.o
	$(GCC) $(GCC_FLAG) -o $(BUILD_DIR)/$(OS_NAME)-$(VERSION).iso $(BUILD_DIR)/bootsect.o $(BUILD_DIR)/kernel.o


# Running ISO in QEmu
run: $(BUILD_DIR)/$(OS_NAME)-$(VERSION).iso
	qemu-system-i386 -cdrom $(BUILD_DIR)/$(OS_NAME)-$(VERSION).iso # -s -S flag for debuging

# Running Binary File in QEmu
#run: $(BUILD_DIR)/32bit-main.bin 
#	qemu-system-i386 $(BUILD_DIR)/32bit-main.bin


clean:
	rm -f $(BUILD_DIR)/*.o


