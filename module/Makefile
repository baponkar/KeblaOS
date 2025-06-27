
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
LINKER_FILE = user_linker_x86_64.ld

file = user_program.c

user_program.bin: $(file)
#	$(NASM) $(NASM_FLAG) -o user_program.o $(file)
	$(GCC) $(GCC_FLAG) -c $(file) -o user_program_c.o
	$(LD) $(LD_FLAG) -T $(LINKER_FILE) user_program_c.o --oformat=elf64-x86-64 -o user_program_c.elf
	
clean:
	rm -f user_program.o user_program.elf user_program.bin


