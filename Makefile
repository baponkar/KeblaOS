
ASM=nasm
SRC_DIR=src
BUILD_DIR=build
OS_NAME=KeblaOS
VERSION=0.0



#
# Floppy Disk Image
#

flopy_img: $(BUILD_DIR)/main.img 
$(BUILD_DIR)/main.img: bootloader kernel 
	dd if=/dev/zero of=$(BUILD_DIR)/main.img bs=512 count=2880 
	mkfs.fat -F 12 -n KeblaOS $(BUILD_DIR)/main.img 
	dd if=$(BUILD_DIR)/bootloader.bin of=$(BUILD_DIR)/main.img conv=notrunc
	mcopy -i $(BUILD_DIR)/main.img $(BUILD_DIR)/kernel.bin "::kernel.bin"
	qemu-system-i386 -fda build/main.img

#
# Bootloader
#

bootloader: $(BUILD_DIR)/bootloader.bin 
$(BUILD_DIR)/bootloader.bin:
	$(ASM) -f bin $(SRC_DIR)/bootloader/bootloader.asm -o $(BUILD_DIR)/bootloader.bin 


#
# Kernel
#

kernel: $(BUILD_DIR)/kernel.bin 
$(BUILD_DIR)/kernel.bin:
	$(ASM) -f bin $(SRC_DIR)/kernel/kernel.asm -o $(BUILD_DIR)/kernel.bin 



