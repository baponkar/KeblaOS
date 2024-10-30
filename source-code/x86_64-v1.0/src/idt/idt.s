[GLOBAL idt_flush]
idt_flush:
    mov rax, [rsp+8]      ; Get the pointer to the IDT, passed as a parameter
    lidt [rax]            ; Load the IDT pointer
    sti                   ; Enable interrupts
    iret

%macro ISR_NOERRCODE 1
    [GLOBAL isr%1]
    isr%1:
        cli
        push 0              ; Dummy error code
        push %1
        jmp isr_common_stub
%endmacro


%macro ISR_ERRCODE 1
    [GLOBAL isr%1]
    isr%1:
        cli
        push %1
        jmp isr_common_stub
%endmacro



[EXTERN isr_handler]
isr_common_stub:
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

    mov ax, 0x10           ; Load the kernel data segment descriptor
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov rdi, rsp           ; Pass the current stack pointer to `isr_handler`
    call isr_handler

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

    add rsp, 16            ; Clean up error code and ISR number
    sti
    iret                  ; Use iretq to return in 64-bit mode


ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7

ISR_ERRCODE 8
ISR_NOERRCODE 9 
ISR_ERRCODE 10
ISR_ERRCODE 11
ISR_ERRCODE 12
ISR_ERRCODE 13
ISR_ERRCODE 14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_NOERRCODE 17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_NOERRCODE 30
ISR_NOERRCODE 31

ISR_NOERRCODE 128   ; System Call
ISR_NOERRCODE 177   ; System Call