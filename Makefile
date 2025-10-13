

# Automatic Bulding Process by GNU Makefile.
# Reference: https://www.gnu.org/software/make/manual/html_node/index.html
# Reference: https://wiki.osdev.org/Makefile

# Last Updated : 27-09-2025
# Author : Bapon Kar
# Repository url : https://github.com/baponkar/KeblaOS
# To see inside of hex file https://hexed.it/
# Help : Run 'make help'



START_TIME := $(shell date +%s)
OS_NAME = KeblaOS
OS_VERSION = 1.2
HOST_HOME = /home/bapon

BUILD_DIR := build
BUILD_INFO_FILE = $(BUILD_DIR)/build_info.txt

ISO_DIR = build/iso_root

DEBUG_DIR = ./debug
DEBUG_FILE = $(DEBUG_DIR)/qemu_log.txt

DISK_DIR = disk_img
DISK_1 = $(DISK_DIR)/disk_1.img
DISK_2 = $(DISK_DIR)/disk_2.img

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
# FATFS_BUILD_DIR = build/ext_lib/FatFs
# FATFS_SRC_DIR   = ext_lib/FatFs-R0.15b
# FATFS_SRC_FILES := $(shell find ext_lib/FatFs-R0.15b -name '*.c')
# FATFS_OBJ_FILES := $(patsubst $(FATFS_SRC_DIR)/%.c, $(FATFS_BUILD_DIR)/%.o, $(FATFS_SRC_FILES))
# FATFS_LIB_FILE  := build/libfatfs.a

# build/ext_lib/FatFs:
# 	mkdir -p $@

# build/ext_lib/FatFs/%.o: ext_lib/FatFs-R0.15b/%.c
# 	@mkdir -p $(dir $@)
# 	$(GCC) $(GCC_FLAG) $(GCC_STDLIB_FLAG) -c $< -o $@


# build/libfatfs.a: $(FATFS_OBJ_FILES)
# 	ar rcs $@ $^

# fatfs: build/libfatfs.a
# 	@echo "FatFs Build completed."




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


# 3. limine (I am using previus building files )
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


build/lib-tiny-regex.a: $(TINY_REGEX_LIB_FILE)
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



# external_libs: $(FATFS_LIB_FILE) $(LVGL_LIB_FILE) $(UGUI_LIB_FILE) $(NUKLEAR_LIB_FILE) $(TINY_REGEX_LIB_FILE)
external_libs: $(LVGL_LIB_FILE) $(UGUI_LIB_FILE) $(NUKLEAR_LIB_FILE) $(TINY_REGEX_LIB_FILE)


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

# =======================================================================================================================================


# Rule to link all object files into a single kernel binary
# $(BUILD_DIR)/kernel.bin: $(KERNEL_LIB_FILE) $(FATFS_LIB_FILE) $(LVGL_LIB_FILE) $(TINY_REGEX_LIB_FILE) $(UGUI_LIB_FILE) $(NUKLEAR_LIB_FILE)
$(BUILD_DIR)/kernel.bin: $(KERNEL_LIB_FILE) $(LVGL_LIB_FILE) $(TINY_REGEX_LIB_FILE) $(UGUI_LIB_FILE) $(NUKLEAR_LIB_FILE)
	$(LD) $(LD_FLAG) -T kernel_linker_x86_64.ld -o $@ $^


linking: $(BUILD_DIR)/kernel.bin
	@echo "Successfully all lib files linked."

# ======================================================================================================================================

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

# =====================================================================================================================================

# Creating ISO image
$(BUILD_DIR)/$(OS_NAME)-$(OS_VERSION)-image.iso: $(BUILD_DIR)/kernel.bin #$(DEBUG_DIR)/objdump.txt

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

# ========================================================================================================================================


blank_disk_1:
	# Ensure disk directory exists
	mkdir -p $(DISK_DIR)

	# Ensure mount1 directory exists
	mkdir -p $(DISK_DIR)/mnt1

	# Clean previous mounts and loop device
	sudo umount $(DISK_DIR)/mnt1 || true
	sudo umount /dev/loop1p1 || true
	sudo losetup -d /dev/loop1 || true

	# 1. Create Disk Image (1024 MiB)
	dd if=/dev/zero of=$(DISK_1) bs=1M count=1024
	@echo "Created blank Disk-1 image"

blank_disk_2:
	# Ensure disk directory exists
	mkdir -p $(DISK_DIR)

	# Ensure mount2 directory exists
	mkdir -p $(DISK_DIR)/mnt2

	# Clean previous mounts and loop device
	sudo umount $(DISK_DIR)/mnt2 || true
	sudo umount /dev/loop2p1 || true
	sudo losetup -d /dev/loop2 || true

	# 1. Create Disk Image (512 MiB)
	dd if=/dev/zero of=$(DISK_2) bs=1M count=512
	@echo "Created blank Disk-2 image"

# Creating two blank disks
blank_disks:
	make blank_disk_1
	make blank_disk_2

# Formatting disk_1.img and disk_2.img as FAT32
fat32_format:
	# Disk - 1
	parted $(DISK_DIR)/disk_1.img --script -- mklabel msdos
	parted $(DISK_DIR)/disk_1.img --script -- mkpart primary fat32 1MiB 100%
	@echo "Disk-1 image Partitioned (FAT32)"

	# Setup loop device, format, and detach
	@bash -c '\
		LOOPDEV=$$(sudo losetup --show -fP $(DISK_DIR)/disk_1.img); \
		sudo mkfs.vfat -F 32 $${LOOPDEV}p1; \
		sudo losetup -d $$LOOPDEV; \
	'
	@echo "Disk-1 Formatted first partition as FAT32"

	# Disk - 2
	parted $(DISK_DIR)/disk_2.img --script -- mklabel msdos
	parted $(DISK_DIR)/disk_2.img --script -- mkpart primary fat32 1MiB 100%
	@echo "Disk-2 image Partitioned (FAT32)"

	@bash -c '\
		LOOPDEV=$$(sudo losetup --show -fP $(DISK_DIR)/disk_2.img); \
		sudo mkfs.vfat -F 32 $${LOOPDEV}p1; \
		sudo losetup -d $$LOOPDEV; \
	'
	@echo "Disk-2 Formatted first partition as FAT32"


# Formatting disk_1.img and disk_2.img as EXT2
ext2_format:

	# Disk - 1
	parted $(DISK_DIR)/disk_1.img --script -- mklabel msdos
	parted $(DISK_DIR)/disk_1.img --script -- mkpart primary ext2 1MiB 100%
	@echo "Disk-1 image Partitioned (EXT2)"

	# Setup loop device with partitions
	sudo losetup -fP $(DISK_1)
	@LOOPDEV_1=$$(sudo losetup -j $(DISK_1) | cut -d: -f1); \
	sudo mkfs.ext2 $${LOOPDEV_1}p1; \
	sudo losetup -d $${LOOPDEV_1}

	# Disk - 2
	parted $(DISK_DIR)/disk_2.img --script -- mklabel msdos
	parted $(DISK_DIR)/disk_2.img --script -- mkpart primary ext2 1MiB 100%
	@echo "Disk-2 image Partitioned (EXT2)"

	sudo losetup -fP $(DISK_2)
	@LOOPDEV_2=$$(sudo losetup -j $(DISK_2) | cut -d: -f1); \
	sudo mkfs.ext2 $${LOOPDEV_2}p1; \
	sudo losetup -d $${LOOPDEV_2}


create_disks:
	# Clean previous disk directory and created again disk directory
	rm -rf $(DISK_DIR)/*
	mkdir -p $(DISK_DIR)/mnt1

	make blank_disk_1
	make blank_disk_2
	make fat32_format
	# make ext2_format
	@echo "All Disk images are created and formatted."

# =======================================================================================================================================

# Run the OS in QEMU with BIOS boot and two sata disks drives
bios_run:
	qemu-system-x86_64 \
	-machine q35 \
	-m 4096 \
	-smp cores=4,threads=1,sockets=1,maxcpus=4 \
	-boot d \
	-drive id=sata_disk1,file=$(DISK_1),if=none,format=raw \
	-drive id=sata_disk2,file=$(DISK_2),if=none,format=raw \
	-device ahci,id=ahci \
	-device ide-hd,drive=sata_disk1,bus=ahci.0 \
	-device ide-hd,drive=sata_disk2,bus=ahci.1 \
	-device ide-cd,drive=cdrom,bus=ahci.2 \
	-drive id=cdrom,media=cdrom,file=$(BUILD_DIR)/$(OS_NAME)-$(OS_VERSION)-image.iso,if=none \
	-serial stdio \
	-vga std \
	-rtc base=utc,clock=host \
	-netdev user,id=n1 -device e1000,netdev=n1 \
	-d guest_errors,int,cpu_reset \
	-D $(DEBUG_DIR)/qemu.log \
	-trace enable=all,file=./debug/trace.log
	#-no-reboot
# To see available trace events about start_dma: qemu-system-x86_64 -trace help | grep -i start_dma
# We can add -noo--rebboot to prevent rebooting after kernel panic


bios_run_nvme:
	qemu-system-x86_64 \
	-machine q35 \
	-m 4096 \
	-smp cores=4,threads=1,sockets=1,maxcpus=4 \
	-boot d \
	-device nvme,serial=nvme1,drive=nvme1 \
	-drive id=nvme1,file=$(DISK_1),if=none,format=raw \
	-device nvme,serial=nvme2,drive=nvme2 \
	-drive id=nvme2,file=$(DISK_2),if=none,format=raw \
	-cdrom $(BUILD_DIR)/$(OS_NAME)-$(OS_VERSION)-image.iso \
	-serial stdio \
	-vga std \
	-rtc base=utc,clock=host \
	-netdev user,id=n1 -device e1000,netdev=n1 \
	-d guest_errors,int,cpu_reset \
	-D $(DEBUG_DIR)/qemu.log
	#-no-reboot

# Run the OS in QEmu By using two ahci sata disks
uefi_run:
	# UEFI Boot
	qemu-system-x86_64 \
		-machine q35 \
		-m 4096 \
		-smp cores=2,threads=2,sockets=1,maxcpus=4 \
		-boot d \
		-device ahci,id=ahci \
		-drive id=disk1,file=$(DISK_DIR)/disk_1.img,if=none,format=raw \
		-device ide-hd,drive=disk1,bus=ahci.0 \
		-drive id=disk2,file=$(DISK_DIR)/disk_2.img,if=none,format=raw \
		-device ide-hd,drive=disk2,bus=ahci.1 \
		-cdrom $(BUILD_DIR)/$(OS_NAME)-$(OS_VERSION)-image.iso \
		-serial stdio \
		-d guest_errors,int,cpu_reset \
		-D $(DEBUG_DIR)/qemu.log \
		-vga std \
		-bios /usr/share/ovmf/OVMF.fd \
		-rtc base=utc,clock=host \
		-net nic \
		-net user


# Running OS by QEmu by using two NVMe SATA Disks
uefi_nvme_run:
	qemu-system-x86_64 \
		-machine q35 \
		-m 4096 \
		-smp cores=2,threads=2,sockets=1 \
		-boot d \
		-device nvme,serial=nvme1,drive=nvme1 \
		-drive id=nvme1,file=$(DISK_1),if=none,format=raw \
		-device nvme,serial=nvme2,drive=nvme2 \
		-drive id=nvme2,file=$(DISK_2),if=none,format=raw \
		-cdrom $(BUILD_DIR)/$(OS_NAME)-$(OS_VERSION)-image.iso \
		-serial stdio \
		-vga std \
		-bios /usr/share/ovmf/OVMF.fd \
		-rtc base=utc,clock=host \
		-net nic -net user

if disk_run:
	qemu-system-x86_64 \
	-machine q35 \
	-m 4096 \
	-smp cores=4,threads=1,sockets=1,maxcpus=4 \
	-boot c \
	-drive id=sata_disk1,file=$(DISK_1),if=none,format=raw \
	-drive id=sata_disk2,file=$(DISK_2),if=none,format=raw \
	-device ahci,id=ahci \
	-device ide-hd,drive=sata_disk1,bus=ahci.0 \
	-device ide-hd,drive=sata_disk2,bus=ahci.1 \
	-serial stdio \
	-vga std \
	-rtc base=utc,clock=host \
	-netdev user,id=n1 -device e1000,netdev=n1 \
	-d guest_errors,int,cpu_reset \
	-D $(DEBUG_DIR)/qemu.log \
	-trace enable=all,file=./debug/trace.log


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
	# -o stands or 
	# Deleting libkernel.a and iso file
	find build -type f \( -name '*.iso' -o -name 'libkernel.a' \) -delete

	# Delete all .o and .d files recursively inside build/kernel directory
	find $(BUILD_DIR)/kernel -type f \( -name '*.o' -o -name '*.d' \) -delete

hard_clean:
# Deleteing all file inside build directory , -o stands or and f stands file
	find $(BUILD_DIR) -type f \( -name '*.o' -o -name '*' \) -delete

#deleting all directories inside build directory , d stand for directory
	find $(BUILD_DIR) -type d -empty ! -path "$(BUILD_DIR)" -delete


# =======================================================================================================================================
user_programe:
	make -C $(USER_MODULE_DIR)/

	@echo "Successfully build user_program.elf"
# =======================================================================================================================================



# Full build (with external libraries)
all: hard_clean kernel fatfs lvgl tiny-regex ugui linking user_programe image create_disks bios_run


# Kernel-only build (no ext_lib)
build: clean kernel linking user_programe image bios_run


default: build

.PHONY: all build fatfs lvgl tiny-regex ugui external_libs kernel linking user_program build_image create_disks blank_disks ext2_format fat32_format disk_run help

help:
	@echo "Available targets:"
	@echo "  make                 - For Kernel build only.(default target)"
	@echo "  make -B              - For Fresh kernel rebuild"
	@echo "  make all             - Build the whole project along with externel library."
	@echo "  make kernel          - Compile the kernel source files only."
	@echo "  make linking         - Link the kernel and LVGL static library(.a) files"
	@echo "  make image           - Create the ISO image for the OS"

	@echo "  make bios_run        - Run the default target with two SATA disks"
	@echo "  make bios_run_nvme   - Run the OS with two NVMe Disks."

	@echo "  make uefi_run        - UEFI Run the target with two SATA Disks"
	@echo "  make uefi_run_nvme   - UEFI Run with two NVMe Disks"

	@echo "  make gdb_debug       - Debugging By GDB"

	@echo "  make clean           - Clean up kernel build artifacts"
	@echo "  make hard_clean      - Clean up all build artifacts"

	@echo "  make blank_disks     - Create Two Blank Disks image"
	@echo "  make fat32_format    - Format two disks with FAT32 Filesystem"
	@echo "  make xt2_format      - Format two disks with EXT2 Filesystem"

	@echo "  make user_programe   - Build user_program.elf from module/user_main.c"

	@echo "  make help            - To display this help menu."



