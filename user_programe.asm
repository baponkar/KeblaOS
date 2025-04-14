; user_program.asm
[bits 32]
global user_main

section .user_program

user_main:
    mov eax, 0x12345678       ; Some dummy code
    int 172
hang:
    jmp hang                  ; Stay in user mode forever
