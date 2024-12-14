

SRC_DIR = src

BUILD_DIR = build

ISO_DIR = iso_root

OS_NAME = KeblaOS
OS_VERSION = 0.7

HOST_HOME = /home/baponkar


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

LD = /usr/local/x86_64-elf/bin/x86_64-elf-ld
LD_FLAG = -m elf_x86_64 -nostdlib -static -z max-page-size=0x1000

NASM = nasm
NASM_FLAG = -Wall -f elf64

# Default target
all: run

# Building kernel.o
$(BUILD_DIR)/kernel.o: $(SRC_DIR)/kernel/kernel.c
	$(GCC) $(GCC_FLAG) -c $(SRC_DIR)/kernel/kernel.c -o $(BUILD_DIR)/kernel.o

	$(GCC) $(GCC_FLAG) -c $(SRC_DIR)/util/util.c -o $(BUILD_DIR)/util.o

	$(GCC) $(GCC_FLAG) -c $(SRC_DIR)/driver/ports.c -o $(BUILD_DIR)/ports.o
	$(GCC) $(GCC_FLAG) -c $(SRC_DIR)/driver/vga.c -o $(BUILD_DIR)/vga.o
	$(GCC) $(GCC_FLAG) -c $(SRC_DIR)/driver/font.c -o $(BUILD_DIR)/font.o


	$(GCC) $(GCC_FLAG) -c $(SRC_DIR)/lib/string.c -o $(BUILD_DIR)/string.o
	$(GCC) $(GCC_FLAG) -c $(SRC_DIR)/lib/stdlib.c -o $(BUILD_DIR)/stdlib.o

	$(GCC) $(GCC_FLAG) -c $(SRC_DIR)/x86_64/gdt.c -o $(BUILD_DIR)/gdt.o
	$(NASM) $(NASM_FLAG) $(SRC_DIR)/x86_64/gdt_load.asm -o $(BUILD_DIR)/gdt_load.o

	$(GCC) $(GCC_FLAG) -c $(SRC_DIR)/x86_64/idt.c -o $(BUILD_DIR)/idt.o
	$(NASM) $(NASM_FLAG) $(SRC_DIR)/x86_64/idt_load.asm -o $(BUILD_DIR)/idt_load.o

	$(GCC) $(GCC_FLAG) -c $(SRC_DIR)/x86_64/pit_timer.c -o $(BUILD_DIR)/pit_timer.o

	$(GCC) $(GCC_FLAG) -c $(SRC_DIR)/driver/keyboard.c -o $(BUILD_DIR)/keyboard.o
	$(GCC) $(GCC_FLAG) -c $(SRC_DIR)/driver/speaker.c -o $(BUILD_DIR)/speaker.o
	$(GCC) $(GCC_FLAG) -c $(SRC_DIR)/usr/shell.c -o $(BUILD_DIR)/shell.o
	$(NASM) $(NASM_FLAG) $(SRC_DIR)/usr/print_reg_values.asm -o $(BUILD_DIR)/print_reg_values.o

	$(GCC) $(GCC_FLAG) -c $(SRC_DIR)/mmu/kheap.c -o $(BUILD_DIR)/kheap.o
	$(NASM) $(NASM_FLAG) $(SRC_DIR)/mmu/load_paging.asm -o $(BUILD_DIR)/load_paging.o
	$(GCC) $(GCC_FLAG) -c $(SRC_DIR)/mmu/paging.c -o $(BUILD_DIR)/paging.o
	$(GCC) $(GCC_FLAG) -c $(SRC_DIR)/mmu/pmm.c -o $(BUILD_DIR)/pmm.o


# Linking object files into kernel binary
$(BUILD_DIR)/kernel.bin: $(BUILD_DIR)/kernel.o \
						$(BUILD_DIR)/util.o \
						$(BUILD_DIR)/ports.o \
						$(BUILD_DIR)/font.o \
						$(BUILD_DIR)/string.o \
						$(BUILD_DIR)/stdlib.o \
						$(BUILD_DIR)/vga.o \
						$(BUILD_DIR)/gdt.o \
						$(BUILD_DIR)/gdt_load.o \
						$(BUILD_DIR)/idt.o \
						$(BUILD_DIR)/idt_load.o \
						$(BUILD_DIR)/pit_timer.o \
						$(BUILD_DIR)/keyboard.o \
						$(BUILD_DIR)/speaker.o \
						$(BUILD_DIR)/shell.o \
						$(BUILD_DIR)/print_reg_values.o \
						$(BUILD_DIR)/kheap.o \
						$(BUILD_DIR)/load_paging.o \
						$(BUILD_DIR)/paging.o \
						$(BUILD_DIR)/pmm.o




	$(LD) $(LD_FLAG) -T $(SRC_DIR)/linker.ld -o $(BUILD_DIR)/kernel.bin \
						$(BUILD_DIR)/kernel.o \
						$(BUILD_DIR)/util.o \
						$(BUILD_DIR)/ports.o \
						$(BUILD_DIR)/font.o \
						$(BUILD_DIR)/string.o \
						$(BUILD_DIR)/stdlib.o \
						$(BUILD_DIR)/vga.o \
						$(BUILD_DIR)/gdt.o \
						$(BUILD_DIR)/gdt_load.o \
						$(BUILD_DIR)/idt.o \
						$(BUILD_DIR)/idt_load.o \
						$(BUILD_DIR)/pit_timer.o \
						$(BUILD_DIR)/keyboard.o \
						$(BUILD_DIR)/speaker.o \
						$(BUILD_DIR)/shell.o \
						$(BUILD_DIR)/print_reg_values.o \
						$(BUILD_DIR)/kheap.o \
						$(BUILD_DIR)/load_paging.o \
						$(BUILD_DIR)/paging.o \
						$(BUILD_DIR)/pmm.o


# Creating ISO image
$(BUILD_DIR)/image.iso: $(BUILD_DIR)/kernel.bin
	# git clone https://github.com/limine-bootloader/limine.git --branch=v8.x-binary --depth=1
	# make -C limine
	
	mkdir -p $(ISO_DIR)/boot
	cp $(SRC_DIR)/img/boot_loader_wallpaper.bmp  $(ISO_DIR)/boot/boot_loader_wallpaper.bmp
	cp -v $(BUILD_DIR)/kernel.bin $(ISO_DIR)/boot/
	mkdir -p $(ISO_DIR)/boot/limine
	cp -v $(SRC_DIR)/limine.conf $(SRC_DIR)/limine/limine-bios.sys $(SRC_DIR)/limine/limine-bios-cd.bin $(SRC_DIR)/limine/limine-uefi-cd.bin $(ISO_DIR)/boot/limine/
	mkdir -p $(ISO_DIR)/EFI/BOOT
	cp -v $(SRC_DIR)/limine/BOOTX64.EFI $(ISO_DIR)/EFI/BOOT/
	cp -v $(SRC_DIR)/limine/BOOTIA32.EFI $(ISO_DIR)/EFI/BOOT/
	xorriso -as mkisofs -b boot/limine/limine-bios-cd.bin -no-emul-boot -boot-load-size 4 -boot-info-table --efi-boot boot/limine/limine-uefi-cd.bin -efi-boot-part --efi-boot-image --protective-msdos-label $(ISO_DIR) -o $(BUILD_DIR)/image.iso
	$(SRC_DIR)/limine/limine bios-install $(BUILD_DIR)/image.iso

	make clean



# Clean build files
clean:
	rm -f $(BUILD_DIR)/*.o
	#rm -rf $(ISO_DIR)

# Running by qemu
run: $(BUILD_DIR)/image.iso
	# GDB Debuging
	# qemu-system-x86_64 -cdrom $(BUILD_DIR)/image.iso  -m 4096 -serial file:serial_output.log -d guest_errors,int,cpu_reset -D qemu.log -vga std -machine ubuntu -s -S

	# UEFI Boot
	# qemu-system-x86_64 -cdrom $(BUILD_DIR)/image.iso  -m 4096 -serial file:serial_output.log -d guest_errors,int,cpu_reset -D qemu.log -vga std -machine ubuntu -bios /usr/share/OVMF/OVMF_CODE.fd

	# BIOS Boot
	qemu-system-x86_64 -cdrom $(BUILD_DIR)/image.iso  -m 4096 -serial file:serial_output.log -d guest_errors,int,cpu_reset -D qemu.log -vga std -machine ubuntu
	

.PHONY: all clean