
# GCC Compiler
GCC = /usr/local/x86_64-elf/bin/x86_64-elf-gcc
GCC_FLAG = -g -Wall -Wextra -std=gnu11 \
			-m64 \
			-fno-pie -no-pie \
			-nostdlib -nostartfiles \
			-ffreestanding \
			-O2

# Assembler
NASM = nasm
NASM_FLAG = -g -Wall -f elf64

OBJDUMP = /usr/local/x86_64-elf/bin/x86_64-elf-objdump

# Linker
LD = /usr/local/x86_64-elf/bin/x86_64-elf-ld
LD_FLAG = -m elf_x86_64 -nostdlib -static -z max-page-size=0x1000
LINKER_FILE = user_linker_x86_64.ld

user_cfile = user_program_2


user_program_2.elf: $(user_cfile).elf
#	$(NASM) $(NASM_FLAG) -o user_program.o $(file)
	$(GCC) $(GCC_FLAG) -c $(user_cfile).c -o $(user_cfile).o
	$(LD) $(LD_FLAG) -T $(LINKER_FILE) $(user_cfile).o --oformat=elf64-x86-64 -o $(user_cfile).elf

	@ echo "Linking completed. Generating binary..."
	make clean
	
clean:
	rm -f *.o 

	@ echo "Cleaned up object files and binaries."
