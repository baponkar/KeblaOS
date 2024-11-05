
# Copied from : https://wiki.osdev.org/Limine_Bare_Bones
SRC_DIR = src
BUILD_DIR = build
ISO_DIR = iso_root
OS_NAME = KeblaOS
VERSION = 0.2
HOME = /home/baponkar

GCC = /usr/local/x86_64-elf/bin/x86_64-elf-gcc
GCC_FLAG = -Wall \
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
	$(GCC) $(GCC_FLAG) -c $(SRC_DIR)/kernel/info.c -o $(BUILD_DIR)/info.o

	$(GCC) $(GCC_FLAG) -c $(SRC_DIR)/util/util.c -o $(BUILD_DIR)/util.o
	
	
	$(GCC) $(GCC_FLAG) -c $(SRC_DIR)/driver/vga/vga.c -o $(BUILD_DIR)/vga.o
	$(GCC) $(GCC_FLAG) -c $(SRC_DIR)/driver/vga/font.c -o $(BUILD_DIR)/font.o
	$(GCC) $(GCC_FLAG) -c $(SRC_DIR)/driver/ports.c -o $(BUILD_DIR)/ports.o
	$(GCC) $(GCC_FLAG) -c $(SRC_DIR)/driver/keyboard.c -o $(BUILD_DIR)/keyboard.o

	$(GCC) $(GCC_FLAG) -c $(SRC_DIR)/usr/shell.c -o $(BUILD_DIR)/shell.o
	$(NASM) $(NASM_FLAG) $(SRC_DIR)/usr/print_reg_values.s -o $(BUILD_DIR)/print_reg_values.o


	$(GCC) $(GCC_FLAG) -c $(SRC_DIR)/stdlib/stdlib.c -o $(BUILD_DIR)/stdlib.o
	$(GCC) $(GCC_FLAG) -c $(SRC_DIR)/stdlib/stdio.c -o $(BUILD_DIR)/stdio.o
	$(GCC) $(GCC_FLAG) -c $(SRC_DIR)/stdlib/string.c -o $(BUILD_DIR)/string.o

	$(GCC) $(GCC_FLAG) -c $(SRC_DIR)/gdt/gdt.c -o $(BUILD_DIR)/gdt.o
	$(NASM) $(NASM_FLAG) $(SRC_DIR)/gdt/gdt.s -o $(BUILD_DIR)/gdt_s.o
	
	$(NASM) $(NASM_FLAG) $(SRC_DIR)/idt/idt.s -o $(BUILD_DIR)/idt_s.o
	$(GCC) $(GCC_FLAG) -c $(SRC_DIR)/idt/idt.c -o $(BUILD_DIR)/idt.o
	$(GCC) $(GCC_FLAG) -c $(SRC_DIR)/idt/timer.c -o $(BUILD_DIR)/timer.o

	$(GCC) $(GCC_FLAG) -c $(SRC_DIR)/kernel/syscall.c -o $(BUILD_DIR)/syscall.o
	$(NASM) $(NASM_FLAG) $(SRC_DIR)/kernel/syscall.s -o $(BUILD_DIR)/syscall_s.o

	$(NASM) $(NASM_FLAG) $(SRC_DIR)/mmu/load_paging.s -o $(BUILD_DIR)/load_paging.o
	$(GCC) $(GCC_FLAG) -c $(SRC_DIR)/mmu/paging.c -o $(BUILD_DIR)/paging.o
	$(GCC) $(GCC_FLAG) -c $(SRC_DIR)/mmu/cpu.c -o $(BUILD_DIR)/cpu.o
	$(NASM) $(NASM_FLAG) $(SRC_DIR)/mmu/cpu.s -o $(BUILD_DIR)/cpu_s.o
	

# Linking object files into kernel binary
$(BUILD_DIR)/kernel.bin: $(BUILD_DIR)/kernel.o \
						$(BUILD_DIR)/info.o \
						$(BUILD_DIR)/ports.o \
						$(BUILD_DIR)/util.o \
						$(BUILD_DIR)/vga.o \
						$(BUILD_DIR)/font.o \
						$(BUILD_DIR)/stdlib.o \
						$(BUILD_DIR)/string.o \
						$(BUILD_DIR)/stdio.o \
						$(BUILD_DIR)/gdt.o \
						$(BUILD_DIR)/gdt_s.o \
						$(BUILD_DIR)/idt.o \
						$(BUILD_DIR)/idt_s.o \
						$(BUILD_DIR)/timer.o \
						$(BUILD_DIR)/keyboard.o \
						$(BUILD_DIR)/shell.o \
						$(BUILD_DIR)/print_reg_values.o \
						$(BUILD_DIR)/paging.o \
						$(BUILD_DIR)/cpu.o \
						$(BUILD_DIR)/cpu_s.o \
						$(BUILD_DIR)/load_paging.o \
						$(BUILD_DIR)/syscall.o \
						$(BUILD_DIR)/syscall_s.o
						
						

	$(LD) $(LD_FLAG) -T $(SRC_DIR)/linker.ld -o $(BUILD_DIR)/kernel.bin \
						$(BUILD_DIR)/kernel.o \
						$(BUILD_DIR)/info.o \
						$(BUILD_DIR)/ports.o \
						$(BUILD_DIR)/util.o \
						$(BUILD_DIR)/vga.o \
						$(BUILD_DIR)/font.o \
						$(BUILD_DIR)/stdlib.o \
						$(BUILD_DIR)/string.o \
						$(BUILD_DIR)/stdio.o \
						$(BUILD_DIR)/gdt.o \
						$(BUILD_DIR)/gdt_s.o \
						$(BUILD_DIR)/idt.o \
						$(BUILD_DIR)/idt_s.o \
						$(BUILD_DIR)/timer.o \
						$(BUILD_DIR)/keyboard.o \
						$(BUILD_DIR)/shell.o \
						$(BUILD_DIR)/print_reg_values.o \
						$(BUILD_DIR)/paging.o \
						$(BUILD_DIR)/cpu.o \
						$(BUILD_DIR)/cpu_s.o \
						$(BUILD_DIR)/load_paging.o \
						$(BUILD_DIR)/syscall.o \
						$(BUILD_DIR)/syscall_s.o

						

# Creating ISO image
$(BUILD_DIR)/image.iso: $(BUILD_DIR)/kernel.bin
	# git clone https://github.com/limine-bootloader/limine.git --branch=v8.x-binary --depth=1
	# make -C limine
	
	mkdir -p $(ISO_DIR)/boot
	cp image/boot_loader_wallpaper.bmp  $(ISO_DIR)/boot/boot_loader_wallpaper.bmp
	cp -v $(BUILD_DIR)/kernel.bin $(ISO_DIR)/boot/
	mkdir -p $(ISO_DIR)/boot/limine
	cp -v $(SRC_DIR)/limine.conf limine/limine-bios.sys limine/limine-bios-cd.bin limine/limine-uefi-cd.bin $(ISO_DIR)/boot/limine/
	mkdir -p $(ISO_DIR)/EFI/BOOT
	cp -v limine/BOOTX64.EFI $(ISO_DIR)/EFI/BOOT/
	cp -v limine/BOOTIA32.EFI $(ISO_DIR)/EFI/BOOT/
	xorriso -as mkisofs -b boot/limine/limine-bios-cd.bin -no-emul-boot -boot-load-size 4 -boot-info-table --efi-boot boot/limine/limine-uefi-cd.bin -efi-boot-part --efi-boot-image --protective-msdos-label $(ISO_DIR) -o $(BUILD_DIR)/image.iso
	./limine/limine bios-install $(BUILD_DIR)/image.iso

	make clean



# Clean build files
clean:
	rm -f $(BUILD_DIR)/*.o
	rm -rf $(ISO_DIR)

# Running by qemu
run: $(BUILD_DIR)/image.iso
	qemu-system-x86_64 -cdrom $(BUILD_DIR)/image.iso  -m 4096 -serial file:serial_output.log -d guest_errors,int,cpu_reset -D qemu.log -vga std -machine ubuntu

.PHONY: all clean
