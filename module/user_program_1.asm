;
; To test msr based system call;
;

bits 64

global _start

section .text
_start:
    mov rdi, message
    syscall
    jmp _start


section .data
message: db "Hello from Userland!\n", 0
