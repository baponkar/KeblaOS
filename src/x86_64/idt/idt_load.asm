
;
; This prgrame will load idt in idtr
; The ISR's which not push any error code we are pushing a dummy error code 0
; 0x10 is represent kernel data selector in GDT
; 0x8 is represent kernel code selector in GDT
; https://stackoverflow.com/questions/79282721/why-is-isr-common-stub-not-calling-my-isr-handler-function?noredirect=1#comment139806502_79282721
;

[extern isr_handler]        ; defined in idt.c
[extern irq_handler]        ; defined in idt.c 

;
; These macros will store the current registers into stack and restore from stack
; The order is follow by registers_t structure defined in util.h 
;

section .text
[global idt_flush]
idt_flush:
    lidt [rdi]              ; Load the IDT pointer
    ret

%macro ISR_NOERRCODE 1
    [global isr%1]
    isr%1:
        cli;

        push 0               ; Dummy error code
        push %1              ; Interrupt number
        ; Save general-purpose registers in reverse order (to match RESTORE_REGISTERS)
        push r15
        push r14
        push r13
        push r12
        push r11
        push r10
        push r9
        push r8
        push rsi
        push rdi
        push rbp
        push rdx
        push rcx
        push rbx
        push rax
        ; Save segment registers
        mov ax, ds
        push rax
        mov ax, es
        push rax
        push fs
        push gs
        ; Load kernel data segment selectors
        mov ax, 0x10
        mov ds, ax
        mov es, ax
        mov fs, ax
        mov gs, ax
        
        ; Pass the current stack pointer (RSP) to the ISR handler
        mov rdi, rsp         ; Pass pointer to the `registers_t` structure
        cld                  ; Clear the direction flag
        call isr_handler     ; Call the interrupt handler

        ; Restore segment registers
        pop gs
        pop fs
        pop rax
        mov es, ax
        pop rax
        mov ds, ax
        ; Restore general-purpose registers
        pop rax
        pop rbx
        pop rcx
        pop rdx
        pop rbp
        pop rdi
        pop rsi
        pop r8
        pop r9
        pop r10
        pop r11
        pop r12
        pop r13
        pop r14
        pop r15
        add rsp, 16 ; Clean up interrupt no and dummy error code

        ; Return from the interrupt using IRETQ (iret values remain intact)
        iretq
%endmacro



%macro ISR_ERRCODE 1
    [global isr%1]
    isr%1:
        cli

        ; Do not need to push dummy error code 
        push %1              ; Interrupt number
        ; Save general-purpose registers in reverse order (to match RESTORE_REGISTERS)
        push r15
        push r14
        push r13
        push r12
        push r11
        push r10
        push r9
        push r8
        push rsi
        push rdi
        push rbp
        push rdx
        push rcx
        push rbx
        push rax
        ; Save segment registers
        mov ax, ds
        push rax
        mov ax, es
        push rax
        push fs
        push gs
        ; Load kernel data segment selectors
        mov ax, 0x10
        mov ds, ax
        mov es, ax
        mov fs, ax
        mov gs, ax


        ; Pass the current stack pointer (RSP) to the ISR handler
        mov rdi, rsp         ; Pass pointer to the `registers_t` structure
        cld                  ; Clear the direction flag
        call isr_handler     ; Call the interrupt handler

        ; Restore segment registers
        pop gs
        pop fs
        pop rax
        mov es, ax
        pop rax
        mov ds, ax
        ; Restore general-purpose registers
        pop rax
        pop rbx
        pop rcx
        pop rdx
        pop rbp
        pop rdi
        pop rsi
        pop r8
        pop r9
        pop r10
        pop r11
        pop r12
        pop r13
        pop r14
        pop r15
        add rsp, 8           ; Remove the pushed interrupt number only
        ; Return from the interrupt using IRETQ
        iretq
%endmacro

ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6     
ISR_NOERRCODE 7

ISR_ERRCODE   8

ISR_NOERRCODE 9 

ISR_ERRCODE   10
ISR_ERRCODE   11
ISR_ERRCODE   12
ISR_ERRCODE   13
ISR_ERRCODE   14

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
        cli
        
        push 0               ; Dummy error code
        push %1              ; Interrupt number
        ; Save general-purpose registers in reverse order (to match RESTORE_REGISTERS)
        push r15
        push r14
        push r13
        push r12
        push r11
        push r10
        push r9
        push r8
        push rsi
        push rdi
        push rbp
        push rdx
        push rcx
        push rbx
        push rax
        ; Save segment registers
        mov ax, ds
        push rax
        mov ax, es
        push rax
        push fs
        push gs
        ; Load kernel data segment selectors
        mov ax, 0x10
        mov ds, ax
        mov es, ax
        mov fs, ax
        mov gs, ax

        
        mov rdi, rsp                    ; Pass the current stack pointer to `irq_handler`
        cld
        call irq_handler

        ; Restore segment registers
        pop gs
        pop fs
        pop rax
        mov es, ax
        pop rax
        mov ds, ax
        ; Restore general-purpose registers
        pop rax
        pop rbx
        pop rcx
        pop rdx
        pop rbp
        pop rdi
        pop rsi
        pop r8
        pop r9
        pop r10
        pop r11
        pop r12
        pop r13
        pop r14
        pop r15
        add rsp, 16 ; Clean up interrupt no and dummy error code
        
        iretq                    ; Return from Interrupt
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
    


