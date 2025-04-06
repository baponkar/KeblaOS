# Automatic Bulding Process by GNU Makefile.
# Reference: https://www.gnu.org/software/make/manual/html_node/index.html
# Reference: https://wiki.osdev.org/Makefile

# Last Updated : 04-01-2025
# Author : Bapon Kar
# Repository url : https://github.com/baponkar/KeblaOS

START_TIME := $(shell date +%s)

LIMINE_DIR = limine-8.6.0
LVGL_DIR = lvgl-9.2.2

SRC_DIR = kernel
ISO_DIR = iso_root
BUILD_DIR = build
DEBUG_DIR = debug

OS_NAME = KeblaOS
OS_VERSION = 0.13

HOST_HOME = /home/baponkar


# GCC Compiler
GCC = /usr/local/x86_64-elf/bin/x86_64-elf-gcc
GCC_FLAG = -g -Wall \
	-Wextra -std=gnu11 \
	-ffreestanding \
	-fno-stack-protector \
	-fno-stack-check \
	-fno-lto -fno-PIC \
	-m64 \
	-march=x86-64 \
	-mno-80387 \
	-mno-mmx \
	-mno-sse \
	-mno-sse2 \
	-mno-red-zone \
	-mcmodel=kernel \
	-msse \
	-msse2


# Linker
LD = /usr/local/x86_64-elf/bin/x86_64-elf-ld
LD_FLAG = -m elf_x86_64 -nostdlib -static -z max-page-size=0x1000

# Assembler
NASM = nasm
NASM_FLAG = -g -Wall -f elf64
OBJDUMP = /usr/local/x86_64-elf/bin/x86_64-elf-objdump



# Find all .c files inside $(LVGL_DIR)/src recursively
LVGL_SRC_FILES := $(shell find $(LVGL_DIR)/src -name '*.c')

# Create corresponding .o file paths inside $(BUILD_DIR)
LVGL_OBJ_FILES := $(patsubst $(LVGL_DIR)/src/%.c, $(BUILD_DIR)/lvgl/%.o, $(LVGL_SRC_FILES))

# Compile rule for all .o files found from LVGL src directory
lvgl: $(LVGL_OBJ_FILES)

# Rule to compile each .c file to .o for LVGL
$(BUILD_DIR)/lvgl/%.o: $(LVGL_DIR)/src/%.c
	@mkdir -p $(dir $@)
	$(GCC) $(GCC_FLAG) -c $< -o $@


# Find all .c files inside of src directory recursively
KERNEL_SRC_FILES := $(shell find $(SRC_DIR) -name '*.c')

# Create corresponding .o file paths inside $(BUILD_DIR)
KERNEL_OBJ_FILES := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(KERNEL_SRC_FILES))

# Find all .c files inside of src directory recursively
KERNEL_SRC_ASM_FILES := $(shell find $(SRC_DIR) -name '*.asm')

# Create corresponding .o file paths inside $(BUILD_DIR)
KERNEL_OBJ_ASM_FILES := $(patsubst $(SRC_DIR)/%.asm, $(BUILD_DIR)/%.o, $(KERNEL_SRC_ASM_FILES))

# Compile rule for all .o files found from kernel src directory
kernel: $(KERNEL_OBJ_FILES) $(KERNEL_OBJ_ASM_FILES)
	@echo "Compiling kernel..."

# Rule to compile each .c file to .o
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(GCC) $(GCC_FLAG) -c $< -o $@

# Rule to compile each .asm file to .o
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.asm
	@mkdir -p $(dir $@)
	$(NASM) $(NASM_FLAG) $< -o $@

linking: $(KERNEL_OBJ_FILES) $(KERNEL_OBJ_ASM_FILES) $(LVGL_OBJ_FILES)
	@echo "Linking kernel..."
	$(LD) $(LD_FLAG) -T linker-x86_64.ld -o $(BUILD_DIR)/kernel.bin $(KERNEL_OBJ_FILES) $(KERNEL_OBJ_ASM_FILES) $(LVGL_OBJ_FILES)

$(BUILD_DIR)/kernel.bin: $(KERNEL_OBJ_FILES) $(KERNEL_OBJ_ASM_FILES) $(LVGL_OBJ_FILES)
	@echo "Linking kernel..."
	$(LD) $(LD_FLAG) -T linker-x86_64.ld -o $@ $^
	@echo "Kernel linked successfully."

#$(DEBUG_DIR)/objdump.txt: $(BUILD_DIR)/kernel.bin
#	$(OBJDUMP) -DxS $< >$@

build_image: $(BUILD_DIR)/$(OS_NAME)-$(OS_VERSION)-image.iso 

# Creating ISO image
$(BUILD_DIR)/$(OS_NAME)-$(OS_VERSION)-image.iso: $(BUILD_DIR)/kernel.bin #$(DEBUG_DIR)/objdump.txt
	# CLONING LIMINE BOOTLOADER
	# git clone https://github.com/limine-bootloader/limine.git --branch=v8.x-binary --depth=1
	# make -C limine

	# Creating build directory which will be used to create ISO image, kernel.bin and object files
	mkdir -p build
	
	# Creating ISO directory which will be used to create ISO image 
	mkdir -p $(ISO_DIR)/boot

	# Copying files to ISO directory and creating directories 
	cp $(SRC_DIR)/img/boot_loader_wallpaper.bmp  $(ISO_DIR)/boot/boot_loader_wallpaper.bmp
	cp -v $(BUILD_DIR)/kernel.bin $(ISO_DIR)/boot/
	mkdir -p $(ISO_DIR)/boot/limine
	cp -v limine.conf $(LIMINE_DIR)/limine-bios.sys $(LIMINE_DIR)/limine-bios-cd.bin $(LIMINE_DIR)/limine-uefi-cd.bin $(ISO_DIR)/boot/limine/
	mkdir -p $(ISO_DIR)/EFI/BOOT
	cp -v $(LIMINE_DIR)/BOOTX64.EFI $(ISO_DIR)/EFI/BOOT/
	cp -v $(LIMINE_DIR)/BOOTIA32.EFI $(ISO_DIR)/EFI/BOOT/

	# Creating initrd.cpio module for bootloader. These files can be used for various purposes,
	cp -v $(SRC_DIR)/initrd/initrd.cpio $(ISO_DIR)/boot/initrd.cpio

	# Creating disk image file which will be used to create ISO image
	dd if=/dev/zero of=$(BUILD_DIR)/disk.img bs=1M count=512

	# Creating KeblaOS-0.11-image.iso file by using xorriso.
	xorriso -as mkisofs -b boot/limine/limine-bios-cd.bin -no-emul-boot -boot-load-size 4 -boot-info-table --efi-boot boot/limine/limine-uefi-cd.bin -efi-boot-part --efi-boot-image --protective-msdos-label $(ISO_DIR) -o $(BUILD_DIR)/$(OS_NAME)-$(OS_VERSION)-image.iso
	$(LIMINE_DIR)/limine bios-install $(BUILD_DIR)/$(OS_NAME)-$(OS_VERSION)-image.iso

	# automatically clean up all object files from build directory.
	# make clean


all : build

build: kernel lvgl linking build_image
	make clean
	@echo "Cleaned object files and empty directories."

	@END_TIME=$$(date +%s); \
	ELAPSED_TIME=$$((END_TIME - $(START_TIME))); \
	echo "Build took $$ELAPSED_TIME seconds."

	make run

partial_build: kernel linking build_image
	@echo "Cleaned object files and empty directories."

	@END_TIME=$$(date +%s); \
	ELAPSED_TIME=$$((END_TIME - $(START_TIME))); \
	echo "Build took $$ELAPSED_TIME seconds."

	make run

# Clean build files
clean:
	# Delete all .o and .d files recursively inside build directory
	find $(BUILD_DIR) -type f \( -name '*.o' -o -name '*.d' \) -delete

	# Remove empty directories (which had only .o, .d files) inside build directory
	find $(BUILD_DIR) -type d -empty -delete

	# Remove all files from iso_root directory
	rm -rf $(ISO_DIR)/*

	@echo "Cleaned object files and empty directories."



# Hard clean build files
hard_clean:
	# Delete all .o and .d files recursively inside build directory
	find $(BUILD_DIR) -type f \( -name '*.o' -o -name '*.d' \) -delete

	# Remove empty directories (which had only .o, .d files) inside build directory
	find $(BUILD_DIR) -type d -empty -delete

	# Remove all files from build directory
	rm -rf $(BUILD_DIR)/*

	@echo "Cleaned object files and empty directories."

# Running by qemu
run: 
	# GDB Debuging
	# qemu-system-x86_64 -cdrom $(BUILD_DIR)/$(OS_NAME)-$(OS_VERSION)-image.iso -m 4096 -serial file:$(DEBUG_DIR)/serial_output.log -d guest_errors,int,cpu_reset -D $(DEBUG_DIR)/qemu.log -vga std -machine q35 -smp cores=2,threads=2,sockets=1,maxcpus=4 -s -S -rtc base=utc,clock=host

	# UEFI Boot
	# qemu-system-x86_64 -hda $(BUILD_DIR)/disk.img -cdrom $(BUILD_DIR)/$(OS_NAME)-$(OS_VERSION)-image.iso -m 4096 -serial file:$(DEBUG_DIR)/serial_output.log -d guest_errors,int,cpu_reset -D $(DEBUG_DIR)/qemu.log -vga std -machine q35 -smp cores=2,threads=2,sockets=1,maxcpus=4 -bios /usr/share/OVMF/OVMF_CODE.fd  -rtc base=utc,clock=host

	# BIOS Boot
	qemu-system-x86_64 \
		-machine q35 \
		-m 4096 \
		-smp cores=4,threads=1,sockets=1,maxcpus=4 \
		-hda $(BUILD_DIR)/disk.img \
		-cdrom $(BUILD_DIR)/$(OS_NAME)-$(OS_VERSION)-image.iso \
		-serial stdio \
		-d guest_errors,int,cpu_reset \
		-D $(DEBUG_DIR)/qemu.log \
		-vga std

default: build
	@echo "Default target executed. Use 'make help' for more options."

help:
	@echo "Available targets:"
	@echo "  make -B              - For Fresh rebuild"
	@echo "  make all             - Build the project (default target)"
	@echo "  make kernel          - Compile the kernel source files"
	@echo "  make lvgl            - Compile the LVGL library source files"
	@echo "  make linking         - Link the kernel and LVGL object files"
	@echo "  make build_image     - Create the ISO image for the OS"
	@echo "  make build           - Build the project (default target)"
	@echo "  make debug           - Run the OS using QEMU with GDB debugging"
	@echo "  make run             - Run the default target (displays this help message)"
	@echo "  make clean           - Clean up build artifacts"
	@echo "  make help            - Display this help menu"
	@echo "  make hard_clean      - Clean up build artifacts and remove empty directories and iso_root files *.iso, *bin , disk.img files"
	@echo "  make partial_build   - Use previous lvgl objects build only kernel without cleaning up"



# This is a phony target, meaning it doesn't correspond to a file.
.PHONY: all kernel lvgl linking build build_image clean run help hard_clean partial_build









	