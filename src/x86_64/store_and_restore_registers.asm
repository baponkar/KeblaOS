;
; This programe will store the current registers into stack and restore from stack
; The order is follow by registers_t structure defined in util.h 
;


section .text
[global save_registers]
save_registers:
    push rax
    push rbx
    push rcx
    push rdx
    push rbp
    push rsi
    push rdi

    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    push gs
    push fs

    mov ax, es
    push rax

    mov ax, ds
    push rax

    ret


section .text
[global restore_registers]
restore_registers:
    pop rax
    mov ds, ax

    pop rax
    mov es, ax

    pop fs
    pop gs

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8

    pop rdi
    pop rsi
    pop rbp
    pop rdx
    pop rcx
    pop rbx
    pop rax

    ret






