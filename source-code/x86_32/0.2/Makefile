#Automated Building Process

SRC_DIR = src
BOOTLOADER_SRC_DIR = $(SRC_DIR)/bootloader
DRIVER_SRC_DIR = $(SRC_DIR)/driver
KERNEL_SRC_DIR = $(SRC_DIR)/kernel
STDLIB_SRC_DIR = $(SRC_DIR)/stdlib

BUILD_DIR = build

ISO_DIR = iso

OS_NAME = KeblaOS
VERSION = 0.1

HOME = /home/baponkar


GCC = $(HOME)/opt/cross/bin/i686-elf-gcc
GCC_FLAG = -ffreestanding -Wall -Wextra -g -O2
LD = $(HOME)/opt/cross/bin/i686-elf-ld

NASM = nasm
NASM_FLAG = -f elf32

default: run



########################## BootLoader START ########################################

# Generating boot.o
$(BUILD_DIR)/boot.o: $(BOOTLOADER_SRC_DIR)/boot.s
	$(NASM) $(NASM_FLAG) $(BOOTLOADER_SRC_DIR)/boot.s -o $(BUILD_DIR)/boot.o

########################## BootLoader EEND #########################################




############################ DRIVER Build START ######################################

# Generating ports.o
$(BUILD_DIR)/ports.o: $(DRIVER_SRC_DIR)/ports.c
	$(GCC) $(GCC_FLAG) -c $(DRIVER_SRC_DIR)/ports.c -o $(BUILD_DIR)/ports.o


# Generating vga.o
$(BUILD_DIR)/vga.o: $(DRIVER_SRC_DIR)/vga.c
	$(GCC) $(GCC_FLAG) -c $(DRIVER_SRC_DIR)/vga.c -o $(BUILD_DIR)/vga.o

############################ DRIVER Build END ########################################



######################### Kernel START ################################################

# Generating kernel.o
$(BUILD_DIR)/kernel.o: $(KERNEL_SRC_DIR)/kernel.c
	$(GCC) $(GCC_FLAG) -c $(KERNEL_SRC_DIR)/kernel.c -o $(BUILD_DIR)/kernel.o

######################### Kernel END ###################################################



############################# Building ISO image file START ############################

# Generating kernel.bin file
$(BUILD_DIR)/kernel.bin: $(BUILD_DIR)/kernel.o \
						$(SRC_DIR)/linker.ld  \
						$(BUILD_DIR)/boot.o  \
						$(BUILD_DIR)/ports.o \
						$(BUILD_DIR)/vga.o 
	# Linking all object file
	$(LD) -m elf_i386 -T $(SRC_DIR)/linker.ld -o $(BUILD_DIR)/kernel.bin \
						$(BUILD_DIR)/kernel.o  \
						$(BUILD_DIR)/boot.o  \
						$(BUILD_DIR)/ports.o \
						$(BUILD_DIR)/vga.o
	# Automatic cleaning object files	
	make clean

#Building ISO image
$(BUILD_DIR)/$(OS_NAME).iso: $(BUILD_DIR)/kernel.bin $(BOOTLOADER_SRC_DIR)/grub.cfg
	mkdir -p build
	mkdir -p iso/boot/grub
	cp $(BOOTLOADER_SRC_DIR)/grub.cfg iso/boot/grub/grub.cfg 	#Copy grub.cfg file 
	cp $(BUILD_DIR)/kernel.bin iso/boot/kernel.bin	#Copy kernel.bin file
	grub-mkrescue -o $(BUILD_DIR)/$(OS_NAME).iso iso		#Creating ISO image 

############################# Building ISO image file END ###########################


clean:
	rm -f $(BUILD_DIR)/*.o


build: $(BUILD_DIR)/$(OS_NAME).iso

run: $(BUILD_DIR)/$(OS_NAME).iso
	qemu-system-i386 -cdrom $(BUILD_DIR)/$(OS_NAME).iso -serial file:serial_output.log -d guest_errors,int -D qemu.log -soundhw pcspk

help:
	@echo "Available targets:"
	@echo "  make run             - Run the default target (displays this help message)"
	@echo "  make build           - Compile the project (simulated in this example)"
	@echo "  make clean           - Clean up build artifacts"
	@echo "  make help            - Display this help menu"


# The process can be implemented by make along with the default, build,run  and clean
.PHONY: default build run clean help
