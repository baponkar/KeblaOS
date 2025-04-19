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
    ;int 170              ; Reading Systemcall
    ;int 171              ; Printing Systemcall


loop:
    jmp loop             ; Stay in user mode forever



