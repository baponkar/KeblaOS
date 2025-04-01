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
CPU_DIR = $(SRC_DIR)/cpu
PCI_DIR = $(SRC_DIR)/pci
AHCI_DIR = $(SRC_DIR)/ahci
DISK_DIR = $(SRC_DIR)/disk
ACPI_DIR = $(SRC_DIR)/acpi
KERNEL_DIR = $(SRC_DIR)/kernel
UTIL_DIR = $(SRC_DIR)/util
DRIVER_DIR = $(SRC_DIR)/driver
LIB_DIR = $(SRC_DIR)/lib
X86_64_DIR = $(SRC_DIR)/x86_64
GDT_DIR = $(SRC_DIR)/x86_64/gdt
INT_DIR = $(SRC_DIR)/x86_64/interrupt
TIMER_DIR = $(SRC_DIR)/x86_64/timer
MEMORY_DIR = $(SRC_DIR)/memory
USR_DIR = $(SRC_DIR)/usr
PS_DIR = $(SRC_DIR)/process
FS_DIR = $(SRC_DIR)/file_system


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

	$(GCC) $(GCC_FLAG) -c $(BOOTLOADER_DIR)/firmware.c -o $(BUILD_DIR)/firmware.o
	$(GCC) $(GCC_FLAG) -c $(BOOTLOADER_DIR)/boot.c -o $(BUILD_DIR)/boot.o

# acpi
	$(GCC) $(GCC_FLAG) -c $(ACPI_DIR)/acpi.c -o $(BUILD_DIR)/acpi.o
	$(GCC) $(GCC_FLAG) -c $(ACPI_DIR)/descriptor_table/fadt.c -o $(BUILD_DIR)/fadt.o
	$(GCC) $(GCC_FLAG) -c $(ACPI_DIR)/descriptor_table/hpet.c -o $(BUILD_DIR)/hpet.o
	$(GCC) $(GCC_FLAG) -c $(ACPI_DIR)/descriptor_table/madt.c -o $(BUILD_DIR)/madt.o
	$(GCC) $(GCC_FLAG) -c $(ACPI_DIR)/descriptor_table/mcfg.c -o $(BUILD_DIR)/mcfg.o
	$(GCC) $(GCC_FLAG) -c $(ACPI_DIR)/descriptor_table/rsdt.c -o $(BUILD_DIR)/rsdt.o
	

# ahci
	$(GCC) $(GCC_FLAG) -c $(AHCI_DIR)/ahci.c -o $(BUILD_DIR)/ahci.o

# pci
	$(GCC) $(GCC_FLAG) -c $(PCI_DIR)/pci.c -o $(BUILD_DIR)/pci.o

# Symmetric Multi processor
	$(GCC) $(GCC_FLAG) -c $(CPU_DIR)/cpuid.c -o $(BUILD_DIR)/cpuid.o
	$(GCC) $(GCC_FLAG) -c $(CPU_DIR)/cpu.c -o $(BUILD_DIR)/cpu.o
	$(NASM) $(NASM_FLAG) $(CPU_DIR)/cpuid.asm -o $(BUILD_DIR)/cpuid_asm.o

#VGA DRIVER
	$(GCC) $(GCC_FLAG) -c $(DRIVER_DIR)/vga/fonts/eng/eng_8x8.c -o $(BUILD_DIR)/eng_8x8.o
	$(GCC) $(GCC_FLAG) -c $(DRIVER_DIR)/vga/fonts/eng/eng_8x16.c -o $(BUILD_DIR)/eng_8x16.o
	$(GCC) $(GCC_FLAG) -c $(DRIVER_DIR)/vga/framebuffer.c -o $(BUILD_DIR)/framebuffer.o
	$(GCC) $(GCC_FLAG) -c $(DRIVER_DIR)/vga/vga_term.c -o $(BUILD_DIR)/vga_term.o
	$(GCC) $(GCC_FLAG) -c $(DRIVER_DIR)/vga/vga_gfx.c -o $(BUILD_DIR)/vga_gfx.o
	$(GCC) $(GCC_FLAG) -c $(DRIVER_DIR)/vga/emoji/emoji.c -o $(BUILD_DIR)/emoji.o
	$(GCC) $(GCC_FLAG) -c $(DRIVER_DIR)/image_data.c -o $(BUILD_DIR)/image_data.o

# Driver
	$(GCC) $(GCC_FLAG) -c $(DRIVER_DIR)/keyboard/keyboard.c -o $(BUILD_DIR)/keyboard.o
	$(GCC) $(GCC_FLAG) -c $(DRIVER_DIR)/mouse/mouse.c -o $(BUILD_DIR)/mouse.o
	$(GCC) $(GCC_FLAG) -c $(DRIVER_DIR)/io/ports.c -o $(BUILD_DIR)/ports.o
	$(GCC) $(GCC_FLAG) -c $(DRIVER_DIR)/io/serial.c -o $(BUILD_DIR)/serial.o

# Disk
	$(GCC) $(GCC_FLAG) -c $(DISK_DIR)/disk.c -o $(BUILD_DIR)/disk.o

# Library
	$(GCC) $(GCC_FLAG) -c $(LIB_DIR)/string.c -o $(BUILD_DIR)/string.o
	$(GCC) $(GCC_FLAG) -c $(LIB_DIR)/stdlib.c -o $(BUILD_DIR)/stdlib.o
	$(GCC) $(GCC_FLAG) -c $(LIB_DIR)/stdio.c -o $(BUILD_DIR)/stdio.o

# GDT
	$(GCC) $(GCC_FLAG) -c $(GDT_DIR)/gdt.c -o $(BUILD_DIR)/gdt.o
	$(NASM) $(NASM_FLAG) $(GDT_DIR)/gdt_load.asm -o $(BUILD_DIR)/gdt_load.o

# Interrupt
	$(GCC) $(GCC_FLAG) -c $(INT_DIR)/interrupt.c -o $(BUILD_DIR)/interrupt.o
	$(NASM) $(NASM_FLAG) $(INT_DIR)/interrupt_flush.asm -o $(BUILD_DIR)/interrupt_flush.o
	$(GCC) $(GCC_FLAG) -c $(INT_DIR)/pic.c -o $(BUILD_DIR)/pic.o
	$(NASM) $(NASM_FLAG) $(INT_DIR)/isr.asm -o $(BUILD_DIR)/isr.o
	$(NASM) $(NASM_FLAG) $(INT_DIR)/irq.asm -o $(BUILD_DIR)/irq.o
	$(GCC) $(GCC_FLAG) -c $(INT_DIR)/apic.c -o $(BUILD_DIR)/apic.o
	$(GCC) $(GCC_FLAG) -c $(INT_DIR)/ioapic.c -o $(BUILD_DIR)/ioapic.o

# Timer
	$(GCC) $(GCC_FLAG) -c $(TIMER_DIR)/tsc.c -o $(BUILD_DIR)/tsc.o
	$(GCC) $(GCC_FLAG) -c $(TIMER_DIR)/pit_timer.c -o $(BUILD_DIR)/pit_timer.o
	$(GCC) $(GCC_FLAG) -c $(TIMER_DIR)/apic_timer.c -o $(BUILD_DIR)/apic_timer.o
	$(GCC) $(GCC_FLAG) -c $(TIMER_DIR)/hpet_timer.c -o $(BUILD_DIR)/hpet_timer.o
	$(GCC) $(GCC_FLAG) -c $(TIMER_DIR)/rtc.c -o $(BUILD_DIR)/rtc.o

#memory management
	$(GCC) $(GCC_FLAG) -c $(MEMORY_DIR)/detect_memory.c -o $(BUILD_DIR)/detect_memory.o
	$(GCC) $(GCC_FLAG) -c $(MEMORY_DIR)/kmalloc.c -o $(BUILD_DIR)/kmalloc.o
	$(GCC) $(GCC_FLAG) -c $(MEMORY_DIR)/pmm.c -o $(BUILD_DIR)/pmm.o
	$(NASM) $(NASM_FLAG) $(MEMORY_DIR)/load_paging.asm -o $(BUILD_DIR)/load_paging.o
	$(GCC) $(GCC_FLAG) -c $(MEMORY_DIR)/paging.c -o $(BUILD_DIR)/paging.o
	$(GCC) $(GCC_FLAG) -c $(MEMORY_DIR)/vmm.c -o $(BUILD_DIR)/vmm.o
	$(GCC) $(GCC_FLAG) -c $(MEMORY_DIR)/kheap.c -o $(BUILD_DIR)/kheap.o

	$(GCC) $(GCC_FLAG) -c $(MEMORY_DIR)/umalloc.c -o $(BUILD_DIR)/umalloc.o
	$(GCC) $(GCC_FLAG) -c $(MEMORY_DIR)/uheap.c -o $(BUILD_DIR)/uheap.o

#process management
	$(GCC) $(GCC_FLAG) -c $(PS_DIR)/process.c -o $(BUILD_DIR)/process.o
	$(NASM) $(NASM_FLAG) $(PS_DIR)/set_cpu_state.asm -o $(BUILD_DIR)/set_cpu_state.o
	$(GCC) $(GCC_FLAG) -c $(PS_DIR)/thread.c -o $(BUILD_DIR)/thread.o
	$(GCC) $(GCC_FLAG) -c $(PS_DIR)/test_process.c -o $(BUILD_DIR)/test_process.o

	$(GCC) $(GCC_FLAG) -c $(FS_DIR)/fs.c -o $(BUILD_DIR)/fs.o

#user shell
	$(GCC) $(GCC_FLAG) -c $(USR_DIR)/shell.c -o $(BUILD_DIR)/shell.o
	$(GCC) $(GCC_FLAG) -c $(USR_DIR)/ring_buffer.c -o $(BUILD_DIR)/ring_buffer.o
	$(GCC) $(GCC_FLAG) -c $(USR_DIR)/switch_to_user.c -o $(BUILD_DIR)/switch_to_user.o
	$(NASM) $(NASM_FLAG) $(USR_DIR)/switch_to_user.asm -o $(BUILD_DIR)/switch_to_user_asm.o
#	$(GCC) $(GCC_FLAG) -c $(USR_DIR)/user.c -o $(BUILD_DIR)/user.o


# Linking object files into kernel binary
$(BUILD_DIR)/kernel.bin: $(BUILD_DIR)/kernel.o \
						$(BUILD_DIR)/boot.o \
						$(BUILD_DIR)/acpi.o \
						$(BUILD_DIR)/rsdt.o \
						$(BUILD_DIR)/fadt.o \
						$(BUILD_DIR)/madt.o \
						$(BUILD_DIR)/mcfg.o \
						$(BUILD_DIR)/ahci.o \
						$(BUILD_DIR)/pci.o \
						$(BUILD_DIR)/ports.o \
						$(BUILD_DIR)/image_data.o \
						$(BUILD_DIR)/string.o \
						$(BUILD_DIR)/stdlib.o \
						$(BUILD_DIR)/stdio.o \
						$(BUILD_DIR)/vga_term.o \
						$(BUILD_DIR)/gdt.o \
						$(BUILD_DIR)/gdt_load.o \
						$(BUILD_DIR)/util.o \
						$(BUILD_DIR)/interrupt.o \
						$(BUILD_DIR)/interrupt_flush.o \
						$(BUILD_DIR)/pic.o \
						$(BUILD_DIR)/isr.o \
						$(BUILD_DIR)/irq.o \
						$(BUILD_DIR)/apic.o \
						$(BUILD_DIR)/ioapic.o \
						$(BUILD_DIR)/load_paging.o \
						$(BUILD_DIR)/paging.o \
						$(BUILD_DIR)/pmm.o \
						$(BUILD_DIR)/vmm.o \
						$(BUILD_DIR)/kmalloc.o \
						$(BUILD_DIR)/umalloc.o \
						$(BUILD_DIR)/uheap.o \
						$(BUILD_DIR)/tsc.o \
						$(BUILD_DIR)/pit_timer.o \
						$(BUILD_DIR)/apic_timer.o \
						$(BUILD_DIR)/hpet_timer.o \
						$(BUILD_DIR)/rtc.o \
						$(BUILD_DIR)/keyboard.o  \
						$(BUILD_DIR)/shell.o \
						$(BUILD_DIR)/ring_buffer.o \
						$(BUILD_DIR)/switch_to_user.o \
						$(BUILD_DIR)/switch_to_user_asm.o \
						$(BUILD_DIR)/kheap.o \
						$(BUILD_DIR)/process.o \
						$(BUILD_DIR)/set_cpu_state.o \
						$(BUILD_DIR)/thread.o \
						$(BUILD_DIR)/test_process.o \
						$(BUILD_DIR)/disk.o \
						$(BUILD_DIR)/fs.o \
						$(BUILD_DIR)/cpu.o \
						$(BUILD_DIR)/cpuid.o \
						$(BUILD_DIR)/cpuid_asm.o \
						$(BUILD_DIR)/detect_memory.o \
						$(BUILD_DIR)/framebuffer.o \
						$(BUILD_DIR)/firmware.o \
						$(BUILD_DIR)/eng_8x8.o \
						$(BUILD_DIR)/eng_8x16.o \
						$(BUILD_DIR)/vga_gfx.o \
						$(BUILD_DIR)/serial.o \
						$(BUILD_DIR)/emoji.o

	$(LD) $(LD_FLAG) -T $(SRC_DIR)/linker-x86_64.ld -o $(BUILD_DIR)/kernel.bin \
														$(BUILD_DIR)/boot.o \
														$(BUILD_DIR)/acpi.o \
														$(BUILD_DIR)/rsdt.o \
														$(BUILD_DIR)/fadt.o \
														$(BUILD_DIR)/madt.o \
														$(BUILD_DIR)/mcfg.o \
														$(BUILD_DIR)/ahci.o \
														$(BUILD_DIR)/pci.o \
														$(BUILD_DIR)/kernel.o \
														$(BUILD_DIR)/ports.o \
														$(BUILD_DIR)/image_data.o \
														$(BUILD_DIR)/string.o \
														$(BUILD_DIR)/stdlib.o \
														$(BUILD_DIR)/stdio.o \
														$(BUILD_DIR)/vga_term.o \
														$(BUILD_DIR)/gdt.o \
														$(BUILD_DIR)/gdt_load.o \
														$(BUILD_DIR)/util.o \
														$(BUILD_DIR)/interrupt.o \
														$(BUILD_DIR)/interrupt_flush.o \
														$(BUILD_DIR)/tsc.o \
														$(BUILD_DIR)/pic.o \
														$(BUILD_DIR)/isr.o \
														$(BUILD_DIR)/irq.o \
														$(BUILD_DIR)/apic.o \
														$(BUILD_DIR)/ioapic.o \
														$(BUILD_DIR)/load_paging.o \
														$(BUILD_DIR)/paging.o \
														$(BUILD_DIR)/pmm.o \
														$(BUILD_DIR)/vmm.o \
														$(BUILD_DIR)/kmalloc.o \
														$(BUILD_DIR)/umalloc.o \
														$(BUILD_DIR)/uheap.o \
														$(BUILD_DIR)/pit_timer.o \
														$(BUILD_DIR)/apic_timer.o \
														$(BUILD_DIR)/hpet_timer.o \
														$(BUILD_DIR)/rtc.o \
														$(BUILD_DIR)/keyboard.o \
														$(BUILD_DIR)/shell.o \
														$(BUILD_DIR)/ring_buffer.o \
														$(BUILD_DIR)/switch_to_user.o \
														$(BUILD_DIR)/switch_to_user_asm.o \
														$(BUILD_DIR)/kheap.o \
														$(BUILD_DIR)/process.o \
														$(BUILD_DIR)/set_cpu_state.o \
														$(BUILD_DIR)/thread.o \
														$(BUILD_DIR)/test_process.o \
														$(BUILD_DIR)/disk.o \
														$(BUILD_DIR)/fs.o \
														$(BUILD_DIR)/cpu.o \
														$(BUILD_DIR)/cpuid.o \
														$(BUILD_DIR)/cpuid_asm.o \
														$(BUILD_DIR)/detect_memory.o \
														$(BUILD_DIR)/framebuffer.o \
														$(BUILD_DIR)/firmware.o \
														$(BUILD_DIR)/eng_8x8.o \
														$(BUILD_DIR)/eng_8x16.o \
														$(BUILD_DIR)/vga_gfx.o \
														$(BUILD_DIR)/serial.o \
														$(BUILD_DIR)/emoji.o
						


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
	cp -v $(SRC_DIR)/limine.conf $(SRC_DIR)/limine/limine-bios.sys $(SRC_DIR)/limine/limine-bios-cd.bin $(SRC_DIR)/limine/limine-uefi-cd.bin $(ISO_DIR)/boot/limine/
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
	# qemu-system-x86_64 -cdrom $(BUILD_DIR)/$(OS_NAME)-$(OS_VERSION)-image.iso -m 4096 -serial file:$(DEBUG_DIR)/serial_output.log -d guest_errors,int,cpu_reset -D $(DEBUG_DIR)/qemu.log -vga std -machine q35 -smp cores=2,threads=2,sockets=1,maxcpus=4 -s -S -rtc base=utc,clock=host

	# UEFI Boot
	# qemu-system-x86_64 -hda $(BUILD_DIR)/disk.img -cdrom $(BUILD_DIR)/$(OS_NAME)-$(OS_VERSION)-image.iso -m 4096 -serial file:$(DEBUG_DIR)/serial_output.log -d guest_errors,int,cpu_reset -D $(DEBUG_DIR)/qemu.log -vga std -machine q35 -smp cores=2,threads=2,sockets=1,maxcpus=4 -bios /usr/share/OVMF/OVMF_CODE.fd  -rtc base=utc,clock=host

	# BIOS Boot
	qemu-system-x86_64 -machine q35 -hda $(BUILD_DIR)/disk.img -cdrom $(BUILD_DIR)/$(OS_NAME)-$(OS_VERSION)-image.iso -m 4096 -serial stdio -d guest_errors,int,cpu_reset -D $(DEBUG_DIR)/qemu.log -vga std -machine q35 -smp cores=4,threads=1,sockets=1,maxcpus=4 -rtc base=utc,clock=host



	

help:
	@echo "Available targets:"
	@echo "  make -B              - For Fresh rebuild"
	@echo "  make run             - Run the default target (displays this help message)"
	@echo "  make build           - Compile the project (simulated in this example)"
	@echo "  make clean           - Clean up build artifacts"
	@echo "  make help            - Display this help menu"


.PHONY: default build run clean help









	