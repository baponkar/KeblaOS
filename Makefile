# Automatic Bulding Process by GNU Makefile.
# Reference: https://www.gnu.org/software/make/manual/html_node/index.html
# Reference: https://wiki.osdev.org/Makefile

# Last Updated : 04-01-2025
# Author : Bapon Kar
# Repository url : https://github.com/baponkar/KeblaOS


SRC_DIR = src
DEBUG_DIR = debug
BUILD_DIR = build
ISO_DIR = iso_root
BOOTLOADER_DIR = $(SRC_DIR)/bootloader
KERNEL_DIR = $(SRC_DIR)/kernel
UTIL_DIR = $(SRC_DIR)/util
DRIVER_DIR = $(SRC_DIR)/driver
LIB_DIR = $(SRC_DIR)/lib
X86_64_DIR = $(SRC_DIR)/x86_64
GDT_DIR = $(SRC_DIR)/x86_64/gdt
IDT_DIR = $(SRC_DIR)/x86_64/idt
PIT_DIR = $(SRC_DIR)/x86_64/pit
RTC_DIR = $(SRC_DIR)/x86_64/rtc
MMU_DIR = $(SRC_DIR)/mmu
USR_DIR = $(SRC_DIR)/usr
PS_DIR = $(SRC_DIR)/pcb



OS_NAME = KeblaOS
OS_VERSION = 0.12

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

# My local machine path of the following tools
# Linker
LD = /usr/local/x86_64-elf/bin/x86_64-elf-ld
LD_FLAG = -m elf_x86_64 -nostdlib -static -z max-page-size=0x1000

# Assembler
NASM = nasm
NASM_FLAG = -g -Wall -f elf64
OBJDUMP = /usr/local/x86_64-elf/bin/x86_64-elf-objdump

# Default target
default: build

# Building kernel.o
$(BUILD_DIR)/kernel.o: $(KERNEL_DIR)/kernel.c

	$(GCC) $(GCC_FLAG) -c $(KERNEL_DIR)/kernel.c -o $(BUILD_DIR)/kernel.o
	$(GCC) $(GCC_FLAG) -c $(UTIL_DIR)/util.c -o $(BUILD_DIR)/util.o

	$(GCC) $(GCC_FLAG) -c $(BOOTLOADER_DIR)/boot.c -o $(BUILD_DIR)/boot.o
	$(GCC) $(GCC_FLAG) -c $(BOOTLOADER_DIR)/acpi.c -o $(BUILD_DIR)/acpi.o
	$(GCC) $(GCC_FLAG) -c $(BOOTLOADER_DIR)/fadt.c -o $(BUILD_DIR)/fadt.o
	$(GCC) $(GCC_FLAG) -c $(BOOTLOADER_DIR)/madt.c -o $(BUILD_DIR)/madt.o
	$(GCC) $(GCC_FLAG) -c $(BOOTLOADER_DIR)/ahci.c -o $(BUILD_DIR)/ahci.o
	$(GCC) $(GCC_FLAG) -c $(BOOTLOADER_DIR)/pci.c -o $(BUILD_DIR)/pci.o
	$(GCC) $(GCC_FLAG) -c $(BOOTLOADER_DIR)/apic.c -o $(BUILD_DIR)/apic.o
	$(GCC) $(GCC_FLAG) -c $(BOOTLOADER_DIR)/disk.c -o $(BUILD_DIR)/disk.o
	$(GCC) $(GCC_FLAG) -c $(BOOTLOADER_DIR)/cpu.c -o $(BUILD_DIR)/cpu.o
	$(GCC) $(GCC_FLAG) -c $(BOOTLOADER_DIR)/memory.c -o $(BUILD_DIR)/memory.o
	$(GCC) $(GCC_FLAG) -c $(BOOTLOADER_DIR)/framebuffer.c -o $(BUILD_DIR)/framebuffer.o
	$(GCC) $(GCC_FLAG) -c $(BOOTLOADER_DIR)/firmware.c -o $(BUILD_DIR)/firmware.o

	
	$(GCC) $(GCC_FLAG) -c $(DRIVER_DIR)/ports.c -o $(BUILD_DIR)/ports.o
	$(GCC) $(GCC_FLAG) -c $(DRIVER_DIR)/vga.c -o $(BUILD_DIR)/vga.o
	$(GCC) $(GCC_FLAG) -c $(DRIVER_DIR)/font.c -o $(BUILD_DIR)/font.o
	$(GCC) $(GCC_FLAG) -c $(DRIVER_DIR)/greek_font.c -o $(BUILD_DIR)/greek_font.o

	$(GCC) $(GCC_FLAG) -c $(DRIVER_DIR)/image_data.c -o $(BUILD_DIR)/image_data.o

	$(GCC) $(GCC_FLAG) -c $(DRIVER_DIR)/window.c -o $(BUILD_DIR)/window.o


	$(GCC) $(GCC_FLAG) -c $(LIB_DIR)/string.c -o $(BUILD_DIR)/string.o
	$(GCC) $(GCC_FLAG) -c $(LIB_DIR)/stdlib.c -o $(BUILD_DIR)/stdlib.o
	$(GCC) $(GCC_FLAG) -c $(LIB_DIR)/stdio.c -o $(BUILD_DIR)/stdio.o

	$(GCC) $(GCC_FLAG) -c $(GDT_DIR)/gdt.c -o $(BUILD_DIR)/gdt.o
	$(NASM) $(NASM_FLAG) $(GDT_DIR)/gdt_load.asm -o $(BUILD_DIR)/gdt_load.o

	$(GCC) $(GCC_FLAG) -c $(IDT_DIR)/idt.c -o $(BUILD_DIR)/idt.o
	$(NASM) $(NASM_FLAG) $(IDT_DIR)/idt_flush.asm -o $(BUILD_DIR)/idt_flush.o
	$(NASM) $(NASM_FLAG) $(IDT_DIR)/isr.asm -o $(BUILD_DIR)/isr.o
	$(NASM) $(NASM_FLAG) $(IDT_DIR)/irq.asm -o $(BUILD_DIR)/irq.o


	$(GCC) $(GCC_FLAG) -c $(PIT_DIR)/pit_timer.c -o $(BUILD_DIR)/pit_timer.o


	$(GCC) $(GCC_FLAG) -c $(DRIVER_DIR)/keyboard.c -o $(BUILD_DIR)/keyboard.o

	$(GCC) $(GCC_FLAG) -c $(DRIVER_DIR)/mouse.c -o $(BUILD_DIR)/mouse.o

	$(GCC) $(GCC_FLAG) -c $(USR_DIR)/shell.c -o $(BUILD_DIR)/shell.o

	$(GCC) $(GCC_FLAG) -c $(MMU_DIR)/kmalloc.c -o $(BUILD_DIR)/kmalloc.o
	$(GCC) $(GCC_FLAG) -c $(MMU_DIR)/pmm.c -o $(BUILD_DIR)/pmm.o
	$(NASM) $(NASM_FLAG) $(MMU_DIR)/load_paging.asm -o $(BUILD_DIR)/load_paging.o
	$(GCC) $(GCC_FLAG) -c $(MMU_DIR)/paging.c -o $(BUILD_DIR)/paging.o
	$(GCC) $(GCC_FLAG) -c $(MMU_DIR)/vmm.c -o $(BUILD_DIR)/vmm.o
	$(GCC) $(GCC_FLAG) -c $(MMU_DIR)/kheap.c -o $(BUILD_DIR)/kheap.o

	$(GCC) $(GCC_FLAG) -c $(PS_DIR)/process.c -o $(BUILD_DIR)/process.o
	


# Linking object files into kernel binary
$(BUILD_DIR)/kernel.bin: $(BUILD_DIR)/kernel.o \
						$(BUILD_DIR)/boot.o \
						$(BUILD_DIR)/acpi.o \
						$(BUILD_DIR)/fadt.o \
						$(BUILD_DIR)/madt.o \
						$(BUILD_DIR)/ahci.o \
						$(BUILD_DIR)/pci.o \
						$(BUILD_DIR)/apic.o \
						$(BUILD_DIR)/ports.o \
						$(BUILD_DIR)/font.o \
						$(BUILD_DIR)/image_data.o \
						$(BUILD_DIR)/string.o \
						$(BUILD_DIR)/stdlib.o \
						$(BUILD_DIR)/stdio.o \
						$(BUILD_DIR)/vga.o \
						$(BUILD_DIR)/gdt.o \
						$(BUILD_DIR)/gdt_load.o \
						$(BUILD_DIR)/util.o \
						$(BUILD_DIR)/idt.o \
						$(BUILD_DIR)/idt_flush.o \
						$(BUILD_DIR)/isr.o \
						$(BUILD_DIR)/irq.o \
						$(BUILD_DIR)/load_paging.o \
						$(BUILD_DIR)/paging.o \
						$(BUILD_DIR)/pmm.o \
						$(BUILD_DIR)/vmm.o \
						$(BUILD_DIR)/kmalloc.o \
						$(BUILD_DIR)/pit_timer.o \
						$(BUILD_DIR)/keyboard.o  \
						$(BUILD_DIR)/shell.o \
						$(BUILD_DIR)/greek_font.o \
						$(BUILD_DIR)/kheap.o \
						$(BUILD_DIR)/window.o \
						$(BUILD_DIR)/mouse.o \
						$(BUILD_DIR)/process.o \
						$(BUILD_DIR)/disk.o \
						$(BUILD_DIR)/cpu.o \
						$(BUILD_DIR)/memory.o \
						$(BUILD_DIR)/framebuffer.o \
						$(BUILD_DIR)/firmware.o


	$(LD) $(LD_FLAG) -T $(SRC_DIR)/linker-x86_64.ld -o $(BUILD_DIR)/kernel.bin \
						$(BUILD_DIR)/boot.o \
						$(BUILD_DIR)/acpi.o \
						$(BUILD_DIR)/fadt.o \
						$(BUILD_DIR)/madt.o \
						$(BUILD_DIR)/ahci.o \
						$(BUILD_DIR)/pci.o \
						$(BUILD_DIR)/apic.o \
						$(BUILD_DIR)/kernel.o \
						$(BUILD_DIR)/ports.o \
						$(BUILD_DIR)/font.o \
						$(BUILD_DIR)/image_data.o \
						$(BUILD_DIR)/string.o \
						$(BUILD_DIR)/stdlib.o \
						$(BUILD_DIR)/stdio.o \
						$(BUILD_DIR)/vga.o \
						$(BUILD_DIR)/gdt.o \
						$(BUILD_DIR)/gdt_load.o \
						$(BUILD_DIR)/util.o \
						$(BUILD_DIR)/idt.o \
						$(BUILD_DIR)/idt_flush.o \
						$(BUILD_DIR)/isr.o \
						$(BUILD_DIR)/irq.o \
						$(BUILD_DIR)/load_paging.o \
						$(BUILD_DIR)/paging.o \
						$(BUILD_DIR)/pmm.o \
						$(BUILD_DIR)/vmm.o \
						$(BUILD_DIR)/kmalloc.o \
						$(BUILD_DIR)/pit_timer.o \
						$(BUILD_DIR)/keyboard.o \
						$(BUILD_DIR)/shell.o \
						$(BUILD_DIR)/greek_font.o \
						$(BUILD_DIR)/kheap.o \
						$(BUILD_DIR)/window.o \
						$(BUILD_DIR)/mouse.o \
						$(BUILD_DIR)/process.o \
						$(BUILD_DIR)/disk.o \
						$(BUILD_DIR)/cpu.o \
						$(BUILD_DIR)/memory.o \
						$(BUILD_DIR)/framebuffer.o \
						$(BUILD_DIR)/firmware.o


#$(DEBUG_DIR)/objdump.txt: $(BUILD_DIR)/kernel.bin
#	$(OBJDUMP) -DxS $< >$@

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
	cp -v $(BOOTLOADER_DIR)/limine.conf $(SRC_DIR)/limine/limine-bios.sys $(SRC_DIR)/limine/limine-bios-cd.bin $(SRC_DIR)/limine/limine-uefi-cd.bin $(ISO_DIR)/boot/limine/
	mkdir -p $(ISO_DIR)/EFI/BOOT
	cp -v $(SRC_DIR)/limine/BOOTX64.EFI $(ISO_DIR)/EFI/BOOT/
	cp -v $(SRC_DIR)/limine/BOOTIA32.EFI $(ISO_DIR)/EFI/BOOT/

	# Creating initrd.cpio module for bootloader. These files can be used for various purposes,
	cp -v $(SRC_DIR)/initrd/initrd.cpio $(ISO_DIR)/boot/initrd.cpio

	# Creating KeblaOS-0.11-image.iso file by using xorriso.
	xorriso -as mkisofs -b boot/limine/limine-bios-cd.bin -no-emul-boot -boot-load-size 4 -boot-info-table --efi-boot boot/limine/limine-uefi-cd.bin -efi-boot-part --efi-boot-image --protective-msdos-label $(ISO_DIR) -o $(BUILD_DIR)/$(OS_NAME)-$(OS_VERSION)-image.iso
	$(SRC_DIR)/limine/limine bios-install $(BUILD_DIR)/$(OS_NAME)-$(OS_VERSION)-image.iso

	# automatically clean up all object files from build directory.
	make clean

build: $(BUILD_DIR)/$(OS_NAME)-$(OS_VERSION)-image.iso
	make run


# Clean build files
clean:
	rm -f $(BUILD_DIR)/*.o
	#rm -rf $(ISO_DIR)

# Running by qemu
run: 
	# GDB Debuging
	# qemu-system-x86_64 -cdrom $(BUILD_DIR)/$(OS_NAME)-$(OS_VERSION)-image.iso -m 4096 -serial file:$(DEBUG_DIR)/serial_output.log -d guest_errors,int,cpu_reset -D $(DEBUG_DIR)/qemu.log -vga std -machine q35 cores=2,threads=2,sockets=1,maxcpus=4 -s -S

	# UEFI Boot
	# qemu-system-x86_64 -cdrom $(BUILD_DIR)/$(OS_NAME)-$(OS_VERSION)-image.iso -m 4096 -serial file:$(DEBUG_DIR)/serial_output.log -d guest_errors,int,cpu_reset -D $(DEBUG_DIR)/qemu.log -vga std -machine q35 -smp cores=2,threads=2,sockets=1,maxcpus=4 -bios /usr/share/OVMF/OVMF_CODE.fd

	# BIOS Boot
	qemu-system-x86_64 -cdrom $(BUILD_DIR)/$(OS_NAME)-$(OS_VERSION)-image.iso -m 4096 -serial file:$(DEBUG_DIR)/serial_output.log -d guest_errors,int,cpu_reset -D $(DEBUG_DIR)/qemu.log -vga std -machine q35 -smp cores=2,threads=2,sockets=1,maxcpus=4

	

help:
	@echo "Available targets:"
	@echo "  make -B              - For Fresh rebuild"
	@echo "  make run             - Run the default target (displays this help message)"
	@echo "  make build           - Compile the project (simulated in this example)"
	@echo "  make clean           - Clean up build artifacts"
	@echo "  make help            - Display this help menu"


.PHONY: default build run clean help









	