section .text
[GLOBAL idt_flush]
idt_flush:
    lidt [rdi]              ; Load the IDT pointer
    ret


; Setup Interrupt Service Routine(ISR)

%macro ISR_NOERRCODE 1
    [GLOBAL isr%1]
    isr%1:
        push 0               ; Dummy error code
        push %1              ; Interrupt number
        jmp isr_common_stub
%endmacro

%macro ISR_ERRCODE 1
    [GLOBAL isr%1]
    isr%1:
        push %1              ; Interrupt number only
        jmp isr_common_stub
%endmacro

[EXTERN isr_handler]
isr_common_stub:
    ; Todo use swapgs

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

    ; Todo - should use swapgs
    push gs                  ; Save previous state of segment registers
    push fs
    mov eax, es
    push rax
    mov eax, ds
    push rax

    mov eax, 0x10             ; Load the kernel data segment descriptor
    mov ds, eax
    mov es, eax
    mov fs, eax
    mov gs, eax

    mov rdi, rsp             ; Pass the current stack pointer to `isr_handler`
    cld                      ; Required by AMD64 System V ABI
    call isr_handler

    ; Todo - should use swapgs
    pop rax                  ; Restore previous state of segment registers
    mov ds, eax
    pop rax
    mov es, eax
    pop fs
    pop gs

    ; Restore general-purpose register state
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

    add rsp, 16               ; Clean up pushed error code and IRQ number 
    iretq


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


; Setup Interrupt Request(IRQ)

%macro IRQ 2
    global irq%1
    irq%1:
        push 0        ; Push a dummy error code
        push %2      ; Push irq code
        jmp irq_common_stub
%endmacro


IRQ   0,    32
IRQ   1,    33
IRQ   2,    34
IRQ   3,    35
IRQ   4,    36
IRQ   5,    37
IRQ   6,    38
IRQ   7,    39
IRQ   8,    40
IRQ   9,    41
IRQ  10,    42
IRQ  11,    43
IRQ  12,    44
IRQ  13,    45
IRQ  14,    46
IRQ  15,    47


; This is a stub that we have created for IRQ based ISRs. This calls
[extern irq_handler]
irq_common_stub:
    ; Todo use swapgs

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

    ; Todo - should use swapgs
    push gs                  ; Save previous state of segment registers
    push fs
    mov eax, es
    push rax
    mov eax, ds
    push rax

    mov eax, 0x10  ; Load the Kernel Data Segment descriptor!
    mov ds, eax
    mov es, eax
    mov fs, eax
    mov gs, eax

    mov rdi, rsp    ; Pass stack pointer to `irq_handler`
    cld                      ; Required by AMD64 System V ABI
    call irq_handler

    ; Todo - should use swapgs
    pop rax                  ; Restore previous state of segment registers
    mov ds, eax
    pop rax
    mov es, eax
    pop fs
    pop gs

    ; Restore general-purpose register state
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

    add rsp, 16     ; Clean up pushed error code and IRQ number 
    iretq           ; Return from Interrupt
