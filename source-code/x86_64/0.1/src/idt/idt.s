
; 
; https://stackoverflow.com/questions/52214531/x86-64-order-of-passing-parameters-in-registers
; 
;

section .text
[GLOBAL idt_flush]
idt_flush:
    lidt [rdi]              ; Load the IDT pointer
    sti                     ; Enable interrupts
    ret


; Setup Interrupt Service Routine(ISR)

%macro ISR_NOERRCODE 1
    [GLOBAL isr%1]
    isr%1:
        cli
        push 0               ; Dummy error code
        push %1              ; Interrupt number
        jmp isr_common_stub
%endmacro

%macro ISR_ERRCODE 1
    [GLOBAL isr%1]
    isr%1:
        cli
        push %1              ; Interrupt number only
        jmp isr_common_stub
%endmacro

[EXTERN isr_handler]
isr_common_stub:
    pushaq                   ; Push all general-purpose registers, this works only inside of macro
    
    mov ax, 0x10             ; Load the kernel data segment descriptor
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov rdi, rsp             ; Pass the current stack pointer to `isr_handler`
    call isr_handler
    
    pophaq                   ; Pop all general-purpose registers

    ; Adjust stack pointer depending on whether error code was pushed
    cmp qword [rsp + 8], 0   ; Check if dummy error code was pushed
    jnz .skip_cleanup        ; If real error code, jump to .skip_cleanup
    add rsp, 16               ; If dummy error code, adjust rsp by 8
    sti
    iretq

.skip_cleanup:
    add rsp, 8               ; Clean up interrupt number 8 for dummy error code and another 8 for interrupt no
    sti
    iretq                



ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6     ; Return from interrupt 6
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
        cli           ; Clear Interrupts
        push 0        ; Push a dummy error code
        mov rax, %2   
        push rax      ; Push irq code
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

    ; Save the CR2 register (holds page fault linear address)
    mov rax, cr2
    push rax

    mov ax, 0x10  ; Load the Kernel Data Segment descriptor!
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov rdi, rsp    ; Pass stack pointer to `irq_handler`
    call irq_handler

    add rsp, 16     ; Clean up pushed error code and IRQ number 
    pop rax

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
      
    ;add rsp, 16     ; Adjust stack to clean up any pushed error code or ISR number (if present)
    sti             ; Store Interrupts
    iretq           ; pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP!

