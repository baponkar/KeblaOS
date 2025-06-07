;
; nasm -g -Wall -f elf64 module/user_program.asm -o user_program.o
; /usr/local/x86_64-elf/bin/x86_64-elf-ld -m elf_x86_64 -nostdlib -static -z max-page-size=0x1000 -T user_linker_x86_64.ld module/user_program.o --oformat=i386:x86-64 -o module/user_program.elf
; /usr/local/x86_64-elf/bin/x86_64-elf-objcopy -O binary module/user_program.elf module/user_program.bin
; objdump -D -b binary -m i386:x86-64 module/user_program.bin
;

;
; user_program.asm
; To test interrupt based system call
;


[bits 32]
global user_main
extern user_shell.c

section .text

user_main:
    ;xor ecx, ecx         ; Divisor = 0
    ;div ecx              ; Triggers #DE exception
    ;mov eax, 0x12345678  ; Some dummy code
    ;int 172              ; Reading Systemcall
    ;int 173              ; Printing Systemcall


loop:
    jmp loop             ; Stay in user mode forever



