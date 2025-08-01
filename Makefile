# Automatic Bulding Process by GNU Makefile.
# Reference: https://www.gnu.org/software/make/manual/html_node/index.html
# Reference: https://wiki.osdev.org/Makefile

# Last Updated : 10-04-2025
# Author : Bapon Kar
# Repository url : https://github.com/baponkar/KeblaOS

START_TIME := $(shell date +%s)

OS_NAME = KeblaOS
OS_VERSION = 0.17.5

LIMINE_DIR = limine-9.2.3

KERNEL_DIR = kernel
ISO_DIR = build/iso_root
BUILD_DIR = build
DEBUG_DIR = debug
CONFIG_DIR = config

DISK_DIR = disk

BUILD_INFO_FILE = $(BUILD_DIR)/build_info.txt

HOST_HOME = /home/baponkar

MODULE_DIR = module


USER_PROGRAM_NAME = user_shell

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
$(BUILD_DIR)/kernel.bin: $(KERNEL_OBJ_FILES) $(KERNEL_OBJ_ASM_FILES)
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
	cp $(KERNEL_DIR)/src/bootloader/img/boot_loader_wallpaper.bmp  $(ISO_DIR)/boot/boot_loader_wallpaper.bmp

	cp -v $(BUILD_DIR)/kernel.bin $(ISO_DIR)/boot/

	cp -v limine.conf $(ISO_DIR)/boot/

	mkdir -p $(ISO_DIR)/boot/limine
	cp -v $(LIMINE_DIR)/limine-bios.sys $(LIMINE_DIR)/limine-bios-cd.bin $(LIMINE_DIR)/limine-uefi-cd.bin $(ISO_DIR)/boot/limine/
	
	mkdir -p $(ISO_DIR)/EFI/BOOT
	cp -v $(LIMINE_DIR)/BOOTX64.EFI $(ISO_DIR)/EFI/BOOT/
	cp -v $(LIMINE_DIR)/BOOTIA32.EFI $(ISO_DIR)/EFI/BOOT/

	# Copy user_programe.elf file into boot 
	cp -v $(MODULE_DIR)/build/$(USER_PROGRAM_NAME).elf $(ISO_DIR)/boot/$(USER_PROGRAM_NAME).elf

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
	$(LIMINE_DIR)/limine bios-install $(BUILD_DIR)/$(OS_NAME)-$(OS_VERSION)-image.iso


build_ext2_disk:
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
	#sudo cp -v limine.conf \
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

build_fat_disk:
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
	#sudo cp -v limine.conf \
	#	$(LIMINE_DIR)/limine-bios.sys \
	#	$(LIMINE_DIR)/limine-bios-cd.bin \
	#	$(LIMINE_DIR)/limine-uefi-cd.bin \
	#	$(DISK_DIR)/mnt/boot/limine/
	#sudo cp -v $(LIMINE_DIR)/BOOTX64.EFI $(DISK_DIR)/mnt/EFI/BOOT/
	#sudo cp -v $(LIMINE_DIR)/BOOTIA32.EFI $(DISK_DIR)/mnt/EFI/BOOT/
	#@echo "Copied Limine and kernel files to mounted disk"

	# 7. Create test files and directories
	echo "FAT32 root test file" | sudo tee $(DISK_DIR)/mnt/TESTFILE.TXT
	sudo mkdir -p $(DISK_DIR)/mnt/SUBDIR
	echo "FAT32 nested test file" | sudo tee $(DISK_DIR)/mnt/SUBDIR/NESTED.TXT

	# 8. Cleanup
	sudo umount $(DISK_DIR)/mnt
	sudo losetup -d /dev/loop0
	@echo "Disk image is ready and FAT32-formatted"


# To Convert the disk image into vmdk which can be used in Vmwire
# qemu-img convert -f raw Disk/disk.img -O vmdk Disk/disk.vmdk



# Running by qemu
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

run:
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

disk_run:
	# Running from Disk Image
	qemu-system-x86_64 \
    -machine q35 \
    -m 3072 \
    -smp cores=4,threads=1,sockets=1,maxcpus=4 \
    -boot c \
    -hda $(DISK_DIR)/disk.img \
    -serial stdio \
    -d guest_errors,int,cpu_reset \
    -D $(DEBUG_DIR)/qemu_diskboot.log \
    -vga std \
    -rtc base=utc,clock=host
	-no-reboot


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
	@echo "  make uefi_run        - UEFI Run the target"
	@echo "  make gdb_debug       - Debugging By GDB"
	@echo "  make clean           - Clean up build artifacts"
	@echo "  make build_ext2_disk - Create EXT2 Format Disk image which will be use in Kernel as disk"
	@echo "  make build_fat_disk  - Create FAT Format Disk image which will be use in Kernel as disk"
	@echo "  make disk_run        - Run OS by using Disk Image"
	@echo "  make help            - Display this help menu"
	@echo "  make build_user_programe - Build user_program.elf from module/user_program.asm"


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
# $(NASM) $(NASM_FLAG) $(MODULE_DIR)/user_program.asm -o $(MODULE_DIR)/user_program.o
	ld -T $(MODULE_DIR)/user_linker_x86_64.ld -o $(MODULE_DIR)/(USER_PROGRAM_NAME).elf $(MODULE_DIR)/$(USER_PROGRAM_NAME).o

	@echo "Successfully build user_program.elf" 


# This is a phony target, meaning it doesn't correspond to a file.
.PHONY: all build clean help



