[EXTERN print_registers_c]

[GLOBAL print_registers]



print_registers:
    ; Push all general-purpose registers to the stack
    ; Storing all general purpose registers state
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    call print_registers_c  ; Call the C function to print them

    ; Restore all general-purpose registers
    ; Clear all general purpose registers state
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    
    iretq
