# Automatic Bulding Process by GNU Makefile.
# Reference: https://www.gnu.org/software/make/manual/html_node/index.html
# Reference: https://wiki.osdev.org/Makefile

# Last Updated : 10-04-2025
# Author : Bapon Kar
# Repository url : https://github.com/baponkar/KeblaOS

START_TIME := $(shell date +%s)

OS_NAME = KeblaOS
OS_VERSION = 0.14

LIMINE_DIR = limine-8.6.0

KERNEL_DIR = kernel-$(OS_VERSION)
ISO_DIR = build/iso_root
BUILD_DIR = build
DEBUG_DIR = debug
CONFIG_DIR = config

DISK_DIR = disk

BUILD_INFO_FILE = $(BUILD_DIR)/build_info.txt

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

# Assembler
NASM = nasm
NASM_FLAG = -g -Wall -f elf64

OBJDUMP = /usr/local/x86_64-elf/bin/x86_64-elf-objdump

# Linker
LD = /usr/local/x86_64-elf/bin/x86_64-elf-ld
LD_FLAG = -m elf_x86_64 -nostdlib -static -z max-page-size=0x1000


# Find all .c files inside of kernel directory recursively
KERNEL_SRC_FILES := $(shell find $(KERNEL_DIR)/src -name '*.c')

# Create corresponding .o file paths inside $(BUILD_DIR)
KERNEL_OBJ_FILES := $(patsubst $(KERNEL_DIR)/src/%.c, $(BUILD_DIR)/kernel/%.o, $(KERNEL_SRC_FILES))

# Find all .c files inside of src directory recursively
KERNEL_SRC_ASM_FILES := $(shell find $(KERNEL_DIR)/src -name '*.asm')

# Create corresponding .o file paths inside $(BUILD_DIR)
KERNEL_OBJ_ASM_FILES := $(patsubst $(KERNEL_DIR)/src/%.asm, $(BUILD_DIR)/kernel/%.o, $(KERNEL_SRC_ASM_FILES))

# Compile rule for all .o files found from kernel src directory
kernel: $(KERNEL_OBJ_FILES) $(KERNEL_OBJ_ASM_FILES)


# Rule to compile each .c file to .o
$(BUILD_DIR)/kernel/%.o: $(KERNEL_DIR)/src/%.c
	@mkdir -p $(dir $@)
	$(GCC) $(GCC_FLAG) -c $< -o $@

# Rule to compile each .asm file to .o
$(BUILD_DIR)/kernel/%.o: $(KERNEL_DIR)/src/%.asm
	@mkdir -p $(dir $@)
	$(NASM) $(NASM_FLAG) $< -o $@

# Linking kernel object files and LVGL object files
linking: $(BUILD_DIR)/kernel.bin

# Rule to link all object files into a single kernel binary
$(BUILD_DIR)/kernel.bin: $(KERNEL_OBJ_FILES) $(KERNEL_OBJ_ASM_FILES) user_programe.o
	$(LD) $(LD_FLAG) -T kernel_linker_x86_64.ld -o $@ $^


#$(DEBUG_DIR)/objdump.txt: $(BUILD_DIR)/kernel.bin
#	$(OBJDUMP) -DxS $< >$@

build_image: $(BUILD_INFO_FILE) $(BUILD_DIR)/$(OS_NAME)-$(OS_VERSION)-image.iso 

# Creating ISO image
$(BUILD_DIR)/$(OS_NAME)-$(OS_VERSION)-image.iso: $(BUILD_DIR)/kernel.bin #$(DEBUG_DIR)/objdump.txt
	# Cloning Limine bootloader repository
	# git clone https://github.com/limine-bootloader/limine.git --branch=v8.x-binary --depth=1
	# make -C limine

	# Creating build directory which will be used to create ISO image, kernel.bin and object files
	mkdir -p build
	
	# Creating ISO directory which will be used to create ISO image 
	mkdir -p $(ISO_DIR)/boot

	# Copying files to ISO directory and creating directories 
	cp $(KERNEL_DIR)/src/img/boot_loader_wallpaper.bmp  $(ISO_DIR)/boot/boot_loader_wallpaper.bmp

	cp -v $(BUILD_DIR)/kernel.bin $(ISO_DIR)/boot/

	mkdir -p $(ISO_DIR)/boot/limine
	cp -v limine.conf $(LIMINE_DIR)/limine-bios.sys $(LIMINE_DIR)/limine-bios-cd.bin $(LIMINE_DIR)/limine-uefi-cd.bin $(ISO_DIR)/boot/limine/
	
	mkdir -p $(ISO_DIR)/EFI/BOOT
	cp -v $(LIMINE_DIR)/BOOTX64.EFI $(ISO_DIR)/EFI/BOOT/
	cp -v $(LIMINE_DIR)/BOOTIA32.EFI $(ISO_DIR)/EFI/BOOT/

	# Creating initrd.cpio module for bootloader. These files can be used for various purposes,
	cp -v $(KERNEL_DIR)/src/initrd/initrd.cpio $(ISO_DIR)/boot/initrd.cpio

	# Creating KeblaOS-0.11-image.iso file by using xorriso.
	xorriso \
		-as mkisofs \
		-b boot/limine/limine-bios-cd.bin -no-emul-boot \
		-boot-load-size 4 \
		-boot-info-table --efi-boot \
		boot/limine/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image \
		--protective-msdos-label $(ISO_DIR) \
		-o $(BUILD_DIR)/$(OS_NAME)-$(OS_VERSION)-image.iso
		
	# install the Limine bootloader into an ISO file, specifically for BIOS-based booting.
	$(LIMINE_DIR)/limine bios-install $(BUILD_DIR)/$(OS_NAME)-$(OS_VERSION)-image.iso


build_disk:
	# Create Disk Image (1GiB size, change 'count' as needed)
	dd if=/dev/zero of=$(DISK_DIR)/disk.img bs=1M count=512
	@echo "Created blank Disk image"

	# Make Partition on the Disk Image
	parted $(DISK_DIR)/disk.img --script -- mklabel msdos
	parted $(DISK_DIR)/disk.img --script -- mkpart primary fat32 1MiB 100%
	@echo "Disk image Partitioned"

	# Unmount the Disk Image (If previously mounted)
	sudo umount $(DISK_DIR)/mnt || true
	sudo losetup -d /dev/loop0 || true
	@echo "Unmount any existing Disk Image from loop0"

	# Setup loop device and detect partitions
	sudo losetup -fP $(DISK_DIR)/disk.img    # Creates /dev/loopX and /dev/loop0p1
	sudo mkfs.vfat -F 32 /dev/loop0p1        # Format as FAT32
	@echo "Formatted Disk Image with FAT32"

	# Mount the Disk Image
	mkdir -p $(DISK_DIR)/mnt
	sudo mount /dev/loop0p1 $(DISK_DIR)/mnt
	@echo "Mounted Disk Image into loop0"




# Running by qemu
run:
	# GDB Debuging
	#qemu-system-x86_64 -cdrom $(BUILD_DIR)/$(OS_NAME)-$(OS_VERSION)-image.iso -s -S
		
	
	# UEFI Boot
	# qemu-system-x86_64 \
	#	-machine q35 \
	#	-m 4096 \
	#	-smp cores=2,threads=2,sockets=1,maxcpus=4 \
	#	-boot d \
	#	-hda $(DISK_DIR)/disk.img \
	#	-cdrom $(BUILD_DIR)/$(OS_NAME)-$(OS_VERSION)-image.iso \
	#	-serial file:$(DEBUG_DIR)/serial_output.log \
	#	-d guest_errors,int,cpu_reset \
	#	-D $(DEBUG_DIR)/qemu.log \
	#	-vga std \
	#	-bios /usr/share/OVMF/OVMF_CODE.fd  \
	#	-rtc base=utc,clock=host

	# BIOS Boot
	qemu-system-x86_64 \
		-machine q35 \
		-m 4096 \
		-smp cores=4,threads=1,sockets=1,maxcpus=4 \
		-boot d \
		-hda $(DISK_DIR)/disk.img \
		-cdrom $(BUILD_DIR)/$(OS_NAME)-$(OS_VERSION)-image.iso \
		-serial stdio \
		-d guest_errors,int,cpu_reset \
		-D $(DEBUG_DIR)/qemu.log \
		-vga std \
		-rtc base=utc,clock=host


# clean all things from inside of build directory
clean:
	# Delete all .o and .d files recursively inside build directory
	find $(BUILD_DIR) -type f \( -name '*.o' -o -name '*.d' \) -delete

	# Remove empty directories (which had only .o, .d files) inside build directory
	find $(BUILD_DIR) -type d -empty -delete

	# Remove all files from build directory
	rm -rf $(BUILD_DIR)/*

	@echo "Cleaned object files and empty directories."


# Cleaning without binary and iso files
soft_clean:
	# Delete all .o and .d files recursively inside build directory
	find $(BUILD_DIR) -type f \( -name '*.o' -o -name '*.d' \) -delete

	# Deleting iso_root directory
	rm -rf $(BUILD_DIR)/iso_root

	# Deleting kernel directory
	rm -rf $(BUILD_DIR)/kernel


all: clean kernel linking build_image run
build: kernel linking build_image
default: all


help:
	@echo "Available targets:"
	@echo "  make -B              - For Fresh rebuild"
	@echo "  make all             - Build the project (default target)"
	@echo "  make kernel          - Compile the kernel source files"
	@echo "  make linking         - Link the kernel and LVGL object files"
	@echo "  make build_image     - Create the ISO image for the OS"
	@echo "  make build           - Build the iso image"
	@echo "  make run             - Run the default target (displays this help message)"
	@echo "  make clean           - Clean up build artifacts"
	@echo "	 make build_disk_image - Create Format Disk image which will be use in Kernel as disk"
	@echo "  make help            - Display this help menu"


# Create a file with the current timestamp and custom message
$(BUILD_INFO_FILE):
	@mkdir -p $(BUILD_DIR)
	@echo "Build Information for $(OS_NAME) v$(OS_VERSION)" > $@
	@echo "Build Time: $$(date)" >> $@
	@echo "Build started by: $$(whoami)@$$(hostname)" >> $@
	@echo "---------------------------------------" >> $@
	@echo "Build Project by: make build" >> $@
	@echo "Build Project and then Run iso by: make all" >> $@
	@echo "Get make help by: make help" >> $@


build_user_programe:
	nasm -g -Wall -f elf64 user_programe.asm -o user_programe.o
	ld -T user_linker_x86_64.ld -o user_programe.elf user_programe.o

	@echo "Successfully build user_programe.elf" 

# This is a phony target, meaning it doesn't correspond to a file.
.PHONY: all build clean help



