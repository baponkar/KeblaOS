;
; This prgrame will load idt in idtr
; The ISR's which not push any error code we are pushing a dummy error code 0
; 0x10 is represent kernel data selector in GDT
; 0x8 is represent kernel code selector in GDT
; 


[extern save_registers]     ; defined in store_and_restore_registers.asm
[extern restore_registers]  ; defined in store_and_restore_registers.asm
[extern isr_handler]        ; defined in idt.c
[extern irq_handler]        ; defined in idt.c 


section .text
[global idt_flush]
idt_flush:
    lidt [rdi]              ; Load the IDT pointer
    ret

; Setup Interrupt Service Routine(ISR)
%macro ISR_NOERRCODE 1
    [global isr%1]
    isr%1:
        push 0               ; Dummy error code
        push %1              ; Interrupt number
        jmp isr_common_stub
%endmacro

%macro ISR_ERRCODE 1
    [global isr%1]
    isr%1:
        ; don't need to push error code as it is autometically push
        push %1              ; Interrupt number only
        jmp isr_common_stub
%endmacro


isr_common_stub:
    call save_registers

    mov eax, 0x10             ; Load the kernel data segment descriptor
    mov ds, eax
    mov es, eax
    mov fs, eax
    mov gs, eax

    mov rdi, rsp             ; Pass the current stack pointer to `isr_handler`
    cld                      ; Required by AMD64 System V ABI
    call isr_handler
    
    add rsp, 16
    call restore_registers
    sti
    iretq                     ; Return from Interrupt


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
    [global irq%1]
    irq%1:
        push 0        ; Push a dummy error code
        mov rax, %2   
        push rax      ; Push irq code
        jmp irq_common_stub
%endmacro


; This is a stub that we have created for IRQ based ISRs. This calls
irq_common_stub:
    call save_registers

    mov ax, 0x10    ; Load the Kernel Data Segment descriptor!
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov rdi, rsp    ; Pass stack pointer to `irq_handler`
    cld             ; Required by AMD64 System V ABI
    call irq_handler

    add rsp, 16     ; Clean up pushed error code and IRQ number 
    call restore_registers
    sti 
    iretq           ; Return from Interrupt

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






