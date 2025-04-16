; user_program.asm
[bits 32]
global user_main

section .text

user_main:
    xor ecx, ecx         ; Divisor = 0
    div ecx              ; Triggers #DE exception
    mov eax, 0x12345678       ; Some dummy code
    int 172

loop:
    jmp loop                  ; Stay in user mode forever



