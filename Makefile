

# Automatic Bulding Process by GNU Makefile.
# Reference: https://www.gnu.org/software/make/manual/html_node/index.html
# Reference: https://wiki.osdev.org/Makefile

# Last Updated : 10-09-2025
# Author : Bapon Kar
# Repository url : https://github.com/baponkar/KeblaOS




START_TIME := $(shell date +%s)
OS_NAME = KeblaOS
OS_VERSION = 1.2
HOST_HOME = /home/bapon

BUILD_DIR := build
BUILD_INFO_FILE = $(BUILD_DIR)/build_info.txt

ISO_DIR = build/iso_root

DEBUG_DIR = ./debug
DEBUG_FILE = $(DEBUG_DIR)/qemu_log.txt

DISK_DIR = disk
MAIN_DISK = $(DISK_DIR)/main__disk.img
USB_DISK = $(DISK_DIR)/usb_disk.img

USER_MODULE_DIR = module
USER_PROGRAM_FILE = user_main



# GCC Compiler
GCC = $(HOST_HOME)/opt/cross/bin/x86_64-elf-gcc-14.2.0 
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
GCC_STDLIB_FLAG = -Ikernel/src/lib -Iext_lib/lvgl-9.3.0 -Iext_lib/lvgl-9.3.0/src


# Assembler
NASM = nasm
NASM_FLAG = -g -Wall -f elf64


# Linker
LD = $(HOST_HOME)/opt/cross/bin/x86_64-elf-ld
LD_FLAG = -m elf_x86_64 -nostdlib -static -z max-page-size=0x1000


OBJDUMP = $(HOST_HOME)/opt/cross/bin/x86_64-elf-objdump



# Building externel library
EXT_LIB_DIR = ext_lib


# Creating build directory
$(BUILD_DIR):
	mkdir -p $@

# ================================= Externel Library Build Start =============================================

# 1. FatFs Library
FATFS_BUILD_DIR = build/ext_lib/FatFs
FATFS_SRC_DIR   = ext_lib/FatFs-R0.15b
FATFS_SRC_FILES := $(shell find ext_lib/FatFs-R0.15b -name '*.c')
FATFS_OBJ_FILES := $(patsubst $(FATFS_SRC_DIR)/%.c, $(FATFS_BUILD_DIR)/%.o, $(FATFS_SRC_FILES))
FATFS_LIB_FILE  := build/libfatfs.a

build/ext_lib/FatFs:
	mkdir -p $@

build/ext_lib/FatFs/%.o: ext_lib/FatFs-R0.15b/%.c
	@mkdir -p $(dir $@)
	$(GCC) $(GCC_FLAG) $(GCC_STDLIB_FLAG) -c $< -o $@


build/libfatfs.a: $(FATFS_OBJ_FILES)
	ar rcs $@ $^

fatfs: build/libfatfs.a
	@echo "FatFs Build completed."




# 2. LvGL Library

# Defined in ext_lib/lvgl-9.3.0/lvgl.mk
LVGL_PATH = ext_lib/lvgl-9.3.0 


ASRCS += $(shell find $(LVGL_PATH)/src -type f -name '*.S')
CSRCS += $(shell find $(LVGL_PATH)/src -type f -name '*.c')
CSRCS += $(shell find $(LVGL_PATH)/demos -type f -name '*.c')
CSRCS += $(shell find $(LVGL_PATH)/examples -type f -name '*.c')
CXXEXT := .cpp
CXXSRCS += $(shell find $(LVGL_PATH)/src -type f -name '*${CXXEXT}')

AFLAGS += "-I$(LVGL_PATH)"
CFLAGS += "-I$(LVGL_PATH)"
CXXFLAGS += "-I$(LVGL_PATH)"


LVGL_BUILD_DIR = build/ext_lib/lvgl
LVGL_SRC_DIR   = ext_lib/lvgl-9.3.0
LVGL_SRC_FILES := $(shell find ext_lib/lvgl-9.3.0/src -name '*.c')
LVGL_OBJ_FILES := $(patsubst $(LVGL_SRC_DIR)/%.c, $(LVGL_BUILD_DIR)/%.o, $(LVGL_SRC_FILES))
LVGL_LIB_FILE  := build/liblvgl.a

build/ext_lib/lvgl:
	mkdir -p $@

build/ext_lib/lvgl/%.o: ext_lib/lvgl-9.3.0/%.c
	@mkdir -p $(dir $@)
	$(GCC) $(GCC_FLAG) $(GCC_STDLIB_FLAG) $(AFLAGS) $(CFLAGS) $(CXXFLAGS) -c $< -o $@


build/liblvgl.a: $(LVGL_OBJ_FILES)
	ar rcs $@ $^

lvgl: build/liblvgl.a
	@echo "LVGL Build completed."


# 3. limine
# Nothing to build
LIMINE_BUILD_DIR := build/ext_lib/limine-9.2.3
LIMINE_SRC_DIR := ext_lib/limine-9.2.3


# 4. tiny-regex-c
TINY_REGEX_BUILD_DIR = build/ext_lib/tiny-regex-c
TINY_REGEX_SRC_DIR   = ext_lib/tiny-regex-c
TINY_REGEX_SRC_FILES := $(shell find ext_lib/tiny-regex-c -name '*.c')
TINY_REGEX_OBJ_FILES := $(patsubst $(TINY_REGEX_SRC_DIR)/%.c, $(TINY_REGEX_BUILD_DIR)/%.o, $(TINY_REGEX_SRC_FILES))
TINY_REGEX_LIB_FILE  := build/lib-tiny-regex.a

build/ext_lib/tiny-regex-c:
	mkdir -p $@

build/tiny-regex-c/%.o: ext_lib/tiny-regex-c/%.c
	@mkdir -p $(dir $@)
	$(GCC) $(GCC_FLAG) $(GCC_STDLIB_FLAG) -c $< -o $@


build/lib-tiny-regex.a: $(LVGL_OBJ_FILES)
	ar rcs $@ $^

tiny-regex: build/lib-tiny-regex.a
	@echo "Tiny-Regex-C Build completed."





# 4. nuklear-4.12.7
NUKLEAR_SRC_DIR   = ext_lib/Nuklear-4.12.7/src
NUKLEAR_INCLUDE   = -I$(NUKLEAR_SRC_DIR)

# Just compile your backend that includes nuklear.h
NUKLEAR_OBJ_FILES = build/nuklear_vga_backend.o

$(NUKLEAR_OBJ_FILES): kernel/src/gui/nuklear_vga_backend.c
	@mkdir -p $(dir $@)
	$(GCC) $(GCC_FLAG) $(GCC_STDLIB_FLAG) $(NUKLEAR_INCLUDE) -c $< -o $@

nuklear: $(NUKLEAR_OBJ_FILES)
	@echo "Nuklear (header-only) build completed."



# 5. ugui
UGUI_BUILD_DIR = build/ext_lib/UGUI
UGUI_SRC_DIR   = ext_lib/UGUI

UGUI_SRC_FILES := $(shell find $(UGUI_SRC_DIR) -name '*.c')
UGUI_OBJ_FILES := $(patsubst $(UGUI_SRC_DIR)/%.c, $(UGUI_BUILD_DIR)/%.o, $(UGUI_SRC_FILES))
UGUI_LIB_FILE  := build/lib-ugui.a

$(UGUI_BUILD_DIR):
	mkdir -p $@

$(UGUI_BUILD_DIR)/%.o: $(UGUI_SRC_DIR)/%.c | $(UGUI_BUILD_DIR)
	@mkdir -p $(dir $@)
	find $(UGUI_BUILD_DIR) -type f \( -name '*.o' -o -name '*.d' \) -delete
	$(GCC) $(GCC_FLAG) $(GCC_STDLIB_FLAG) -Iext_lib/UGUI -c $< -o $@

$(UGUI_LIB_FILE): $(UGUI_OBJ_FILES)
	ar rcs $@ $^

ugui: $(UGUI_LIB_FILE)
	@echo "UGUI Build completed."




# ============================================== Kernel Build  Start ===============================================


# 5. Kernel build

KERNEL_DIR = kernel

KERNEL_BUILD_DIR = build/kernel
KERNEL_SRC_DIR   = kernel
KERNEL_C_SRC_FILES := $(shell find kernel -name '*.c')
KERNEL_C_OBJ_FILES := $(patsubst $(KERNEL_SRC_DIR)/%.c, $(KERNEL_BUILD_DIR)/%.o, $(KERNEL_C_SRC_FILES))
KERNEL_LIB_FILE  := build/libkernel.a

build/kernel:
	mkdir -p $@

build/kernel/%.o: kernel/%.c
	@mkdir -p $(dir $@)
	$(GCC) $(GCC_FLAG) $(GCC_STDLIB_FLAG) -c $< -o $@

KERNEL_ASM_SRC_FILES := $(shell find kernel -name '*.asm')
KERNEL_ASM_OBJ_FILES := $(patsubst $(KERNEL_SRC_DIR)/%.asm, $(KERNEL_BUILD_DIR)/%.o, $(KERNEL_ASM_SRC_FILES))


build/kernel/%.o: kernel/%.asm
	@mkdir -p $(dir $@)
	$(NASM) $(NASM_FLAG) $< -o $@

KERNEL_OBJ_FILES := $(KERNEL_C_OBJ_FILES) $(KERNEL_ASM_OBJ_FILES)

build/libkernel.a: $(KERNEL_OBJ_FILES)
	ar rcs $@ $^

kernel: build/libkernel.a
	@echo "Kernel C Build completed."

# ==================================================================================================


# Rule to link all object files into a single kernel binary
$(BUILD_DIR)/kernel.bin: $(KERNEL_LIB_FILE) $(FATFS_LIB_FILE) $(LVGL_LIB_FILE) $(TINY_REGEX_LIB_FILE) $(UGUI_LIB_FILE)  $(NUKLEAR_LIB_FILE)
	$(LD) $(LD_FLAG) -T kernel_linker_x86_64.ld -o $@ $^


linking: $(BUILD_DIR)/kernel.bin
	@echo "Successfully all lib files linked."

# ====================================================================================================

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

# ====================================================================================================

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
	cp image/boot_loader_wallpaper.bmp  $(ISO_DIR)/boot/boot_loader_wallpaper.bmp

	cp -v $(BUILD_DIR)/kernel.bin $(ISO_DIR)/boot/

	cp -v $(LIMINE_SRC_DIR)/limine.conf $(ISO_DIR)/boot/

	mkdir -p $(ISO_DIR)/boot/limine
	cp -v $(LIMINE_SRC_DIR)/limine-bios.sys $(LIMINE_SRC_DIR)/limine-bios-cd.bin $(LIMINE_SRC_DIR)/limine-uefi-cd.bin $(ISO_DIR)/boot/limine/
	
	mkdir -p $(ISO_DIR)/EFI/BOOT
	cp -v $(LIMINE_SRC_DIR)/BOOTX64.EFI $(ISO_DIR)/EFI/BOOT/
	cp -v $(LIMINE_SRC_DIR)/BOOTIA32.EFI $(ISO_DIR)/EFI/BOOT/

	# Copy user_programe.elf file into boot 
	cp -v $(USER_MODULE_DIR)/build/$(USER_PROGRAM_FILE).elf $(ISO_DIR)/boot/$(USER_PROGRAM_FILE).elf

	# Creating KeblaOS-0.11-image.iso file by using xorriso.
	xorriso \
		-as mkisofs \
		-b boot/limine/limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot boot/limine/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image \
		--protective-msdos-label $(ISO_DIR) \
		-o $(BUILD_DIR)/$(OS_NAME)-$(OS_VERSION)-image.iso
		
	# install the Limine bootloader into an ISO file, specifically for BIOS-based booting.
	$(LIMINE_SRC_DIR)/limine bios-install $(BUILD_DIR)/$(OS_NAME)-$(OS_VERSION)-image.iso

image: $(BUILD_DIR)/$(OS_NAME)-$(OS_VERSION)-image.iso $(BUILD_INFO_FILE)

# =============================================================================================

fat_disk:
	# Ensure disk directory exists
	mkdir -p $(DISK_DIR)

	# Clean previous mounts and loop device
	sudo umount $(DISK_DIR)/mnt || true
	sudo umount /dev/loop0p1 || true
	sudo losetup -d /dev/loop0 || true

	# 1. Create Disk Image (1024 MiB)
	dd if=/dev/zero of=$(DISK_DIR)/disk.img bs=1M count=1024
	@echo "Created blank Disk image"

	# 2. Partition the Disk Image as FAT32
	parted $(DISK_DIR)/disk.img --script -- mklabel msdos
	parted $(DISK_DIR)/disk.img --script -- mkpart primary fat32 1MiB 100%
	@echo "Disk image Partitioned (FAT32)"

	# 3. Setup loop device and partition mapping
	sudo losetup -Pf $(DISK_DIR)/disk.img
	sleep 1

	# 4. Format the partition as FAT32
	sudo mkfs.vfat -F 32 /dev/loop0p1
	@echo "Formatted loop0p1 as FAT32"

	# 5. Mount partition
	mkdir -p $(DISK_DIR)/mnt
	sudo mount /dev/loop0p1 $(DISK_DIR)/mnt
	@echo "Mounted /dev/loop0p1"

	# 6. Copy files (optional - enable for real usage)
	#sudo mkdir -p $(DISK_DIR)/mnt/boot/limine
	#sudo mkdir -p $(DISK_DIR)/mnt/EFI/BOOT
	#sudo cp -v $(BUILD_DIR)/kernel.bin $(DISK_DIR)/mnt/boot/
	#sudo cp -v $(LIMINE_SRC_DIR)/limine.conf \
	#	$(LIMINE_SRC_DIR)/limine-bios.sys \
	#	$(LIMINE_SRC_DIR)/limine-bios-cd.bin \
	#	$(LIMINE_SRC_DIR)/limine-uefi-cd.bin \
	#	$(DISK_DIR)/mnt/boot/limine/
	#sudo cp -v $(LIMINE_SRC_DIR)/BOOTX64.EFI $(DISK_DIR)/mnt/EFI/BOOT/
	#sudo cp -v $(LIMINE_SRC_DIR)/BOOTIA32.EFI $(DISK_DIR)/mnt/EFI/BOOT/
	#@echo "Copied Limine and kernel files to mounted disk"

	# 7. Create test files and directories
	echo "FAT32 root test file" | sudo tee $(DISK_DIR)/mnt/TESTFILE.TXT
	sudo mkdir -p $(DISK_DIR)/mnt/SUBDIR
	echo "FAT32 nested test file" | sudo tee $(DISK_DIR)/mnt/SUBDIR/NESTED.TXT

	# 8. Cleanup
	sudo umount $(DISK_DIR)/mnt
	sudo losetup -d /dev/loop0
	@echo "Disk image is ready and FAT32-formatted"

#============================================================================================

ext2_disk:
	# Ensure disk directory exists
	mkdir -p $(DISK_DIR)

	# Clean previous mounts and loop device
	sudo umount $(DISK_DIR)/mnt || true
	sudo umount /dev/loop0p1 || true
	sudo losetup -d /dev/loop0 || true

	# 1. Create Disk Image (1024 MiB)
	dd if=/dev/zero of=$(DISK_DIR)/disk.img bs=1M count=1024
	@echo "Created blank Disk image"

	# 2. Partition the Disk Image
	parted $(DISK_DIR)/disk.img --script -- mklabel msdos
	# parted $(DISK_DIR)/disk.img --script -- mkpart primary fat32 1MiB 100%
	parted $(DISK_DIR)/disk.img --script mkpart primary ext2 1MiB 100%
	@echo "Disk image Partitioned"

	# 3. Setup loop device and partition mapping
	sudo losetup -Pf $(DISK_DIR)/disk.img # Automatically creates /dev/loop0 and /dev/loop0p1
	sleep 1                               # Wait a bit to let /dev/loop0p1 appear
	# sudo mkfs.vfat -F 32 /dev/loop0p1	  # To Create FAT Filesystem
	sudo mkfs.ext2 /dev/loop0p1           # To Create EXT2 Filesystem
	@echo "Formatted loop0p1 as EXT2"

	# 4. Mount partition
	mkdir -p $(DISK_DIR)/mnt
	sudo mount /dev/loop0p1 $(DISK_DIR)/mnt
	@echo "Mounted /dev/loop0p1"

	# 5. Copy kernel and Limine files
	#sudo mkdir -p $(DISK_DIR)/mnt/boot/limine
	#sudo mkdir -p $(DISK_DIR)/mnt/EFI/BOOT
	#sudo cp -v $(BUILD_DIR)/kernel.bin $(DISK_DIR)/mnt/boot/
	#sudo cp -v $(MODULE_DIR)/$(USER_PROGRAM_NAME).elf $(DISK_DIR)/mnt/boot/
	#sudo cp -v $(LIMINE_DIR)/limine.conf \
	#	$(LIMINE_DIR)/limine-bios.sys \
	#	$(LIMINE_DIR)/limine-bios-cd.bin \
	#	$(LIMINE_DIR)/limine-uefi-cd.bin \
	#	$(DISK_DIR)/mnt/boot/limine/
	#sudo cp -v $(LIMINE_DIR)/BOOTX64.EFI $(DISK_DIR)/mnt/EFI/BOOT/
	#sudo cp -v $(LIMINE_DIR)/BOOTIA32.EFI $(DISK_DIR)/mnt/EFI/BOOT/
	#@echo "Copied Limine and kernel files to mounted disk"

	# 6. Install Limine to raw disk image
	#sudo sync
	#sudo $(LIMINE_DIR)/limine bios-install $(DISK_DIR)/disk.img
	#@echo "Installed Limine to disk image"

	# 7. Create testfile and testdirectory
	# Create test files
	echo "root test file" | sudo tee $(DISK_DIR)/mnt/testfile.txt
	sudo mkdir -p $(DISK_DIR)/mnt/subdir
	echo "nested test file" | sudo tee $(DISK_DIR)/mnt/subdir/nested.txt

	# 8. Cleanup
	sudo umount $(DISK_DIR)/mnt
	sudo losetup -d /dev/loop0
	@echo "Disk image is ready and bootable"

# ================================================================================

bios_run:
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
# We can add -noo--rebboot to prevent rebooting after kernel panic

uefi_run:
	# UEFI Boot
	qemu-system-x86_64 \
		-machine q35 \
		-m 4096 \
		-smp cores=2,threads=2,sockets=1,maxcpus=4 \
		-boot d \
		-hda $(DISK_DIR)/disk.img \
		-cdrom $(BUILD_DIR)/$(OS_NAME)-$(OS_VERSION)-image.iso \
		-serial stdio \
		-d guest_errors,int,cpu_reset \
		-D $(DEBUG_DIR)/qemu.log \
		-vga std \
		-bios /usr/share/OVMF/OVMF_CODE.fd  \
		-rtc base=utc,clock=host

gdb_debug:
	# GDB Debuging
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
		-rtc base=utc,clock=host \
		-s -S

clean:
	# Delete all .o and .d files recursively inside build/kernel directory
	find $(BUILD_DIR)/kernel -type f \( -name '*.o' -o -name '*.d' \) -delete

hard_clean:
# Deleteing all file inside build directory , -o stands or and f stands file
	find $(BUILD_DIR) -type f \( -name '*.o' -o -name '*' \) -delete

#deleting all directories inside build directory , d stand for directory
	find $(BUILD_DIR) -type d -empty ! -path "$(BUILD_DIR)" -delete


# ==============================================================================
user_programe:
	make -C $(USER_MODULE_DIR)/

	@echo "Successfully build user_program.elf"
# ==============================================================================



# Full build (with external libraries)
all: hard_clean fatfs lvgl tiny-regex ugui kernel linking user_programe image fat_disk bios_run


# Kernel-only build (no ext_lib)
build: clean kernel linking user_programe image bios_run


default: build

.PHONY: all build fatfs lvgl tiny-regex ugui kernel linking user_program build_image fat_disk

help:
	@echo "Available targets:"
	@echo "  make                 - For Kernel build only.(default target)"
	@echo "  make -B              - For Fresh kernel rebuild"
	@echo "  make all             - Build the whole project along with externel library."
	@echo "  make kernel          - Compile the kernel source files only."
	@echo "  make linking         - Link the kernel and LVGL static library(.a) files"
	@echo "  make image           - Create the ISO image for the OS"
	@echo "  make bios_run        - Run the default target (displays this help message)"
	@echo "  make uefi_run        - UEFI Run the target"
	@echo "  make gdb_debug       - Debugging By GDB"
	@echo "  make clean           - Clean up kernel build artifacts"
	@echo "  make hard_clean      - Clean up all build artifacts"
	@echo "  make ext2_disk       - Create EXT2 Format raw Disk image which will be use in Kernel as disk"
	@echo "  make fat_disk        - Create FAT Format raw Disk image which will be use in Kernel as disk"
	@echo "  make help            - Display this help menu"
	@echo "  make user_programe   - Build user_program.elf from module/user_program.asm"



