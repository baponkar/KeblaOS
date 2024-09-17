#gcc=/home/scott/Documents/cross/tools/bin/i686-elf-gcc
#ld=/home/scott/Documents/cross/tools/bin/i686-elf-ld


gcc = /home/baponkar/opt/cross/bin/i686-elf-gcc
GCC_FLAG = -ffreestanding -Wall -Wextra -g -O2
ld = /home/baponkar/opt/cross/bin/i686-elf-ld

CFLAGS = -ffreestanding -g

all: clean kernel boot image run

clean:
	rm -rf *.o

kernel:
	$(gcc) $(CFLAGS) -c src/kernel/kernel.c -o kernel.o
	$(gcc) $(CFLAGS) -c src/driver/vga.c -o vga.o
	$(gcc) $(CFLAGS) -c src/gdt/gdt.c -o gdt.o
	$(gcc) $(CFLAGS) -c src/kernel/util.c -o util.o
	$(gcc) $(CFLAGS) -c src/interrupts/idt.c -o idt.o
	$(gcc) $(CFLAGS) -c src/interrupts/timer.c -o timer.o
	$(gcc) $(CFLAGS) -c src/stdlib/stdio.c -o stdio.o
	$(gcc) $(CFLAGS) -c src/driver/keyboard.c -o keyboard.o
	$(gcc) $(CFLAGS) -c src/mmu/memory.c -o memory.o
	$(gcc) $(CFLAGS) -c src/mmu/kmalloc.c -o kmalloc.o
	$(gcc) $(CFLAGS) -c src/stdlib/math.c -o math.o
	$(gcc) $(CFLAGS) -c src/stdlib/stdlib.c -o stdlib.o
	$(gcc) $(CFLAGS) -c src/stdlib/string.c -o string.o

boot:
	nasm -f elf32 src/bootloader/boot.s -o boot.o
	nasm -f elf32 src/gdt/gdt.s -o gdts.o
	nasm -f elf32 src/interrupts/idt.s -o idts.o
image:
	$(ld) -T src/linker.ld -o kernel boot.o kernel.o vga.o gdt.o gdts.o util.o idt.o idts.o timer.o stdio.o keyboard.o memory.o kmalloc.o math.o stdlib.o string.o
	mv kernel Kebla/boot/kernel
	cp src/bootloader/grub.cfg Kebla/boot/grub/grub.cfg
	grub-mkrescue -o kernel.iso Kebla/
	rm *.o

run: kernel.iso
	qemu-system-i386 -cdrom kernel.iso # -s -S flag for debuging
