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

; Save all segment and general purpose registers
%macro SAVE_REGISTERS 0
    ; Save general-purpose registers
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

    ; Save segment registers
    mov ax, ds
    push rax
    mov ax, es
    push rax
    push fs
    push gs
%endmacro


; Restore all segment and general purpose registers
%macro RESTORE_REGISTERS 0
    ; Restore segment registers
    pop gs
    pop fs
    pop rax
    mov es, ax
    pop rax
    mov ds, ax

    ; Restore general-purpose registers
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
%endmacro



isr_common_stub:
    SAVE_REGISTERS

    ; Load the kernel data segment descriptor
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Save `iret_*` values into the stack as part of `registers_t`
    ; Calculate the correct offsets based on the new structure layout
    mov rax, [rsp + 8*0]     ; `iret_rip`
    mov [rsp + 8*19], rax    ; Offset 19 corresponds to `iret_rip` in `registers_t`

    mov rax, [rsp + 8*1]     ; `iret_cs`
    mov [rsp + 8*20], rax    ; Offset 20 corresponds to `iret_cs`

    mov rax, [rsp + 8*2]     ; `iret_rflags`
    mov [rsp + 8*21], rax    ; Offset 21 corresponds to `iret_rflags`

    mov rax, [rsp + 8*3]     ; `iret_rsp`
    mov [rsp + 8*22], rax    ; Offset 22 corresponds to `iret_rsp`

    mov rax, [rsp + 8*4]     ; `iret_ss`
    mov [rsp + 8*23], rax    ; Offset 23 corresponds to `iret_ss`

    ; Save interrupt number and error code
    mov rax, [rsp + 8*5]     ; Interrupt number
    mov [rsp + 8*24], rax    ; Offset 24 corresponds to `int_no`
    mov rax, [rsp + 8*6]     ; Error code
    mov [rsp + 8*25], rax    ; Offset 25 corresponds to `err_code`

    ; Pass the current stack pointer to `isr_handler`
    mov rdi, rsp
    cld
    call isr_handler

    RESTORE_REGISTERS
    add rsp, 16              ; Clean up pushed error code and IRQ number
    
    iretq                    ; Return from Interrupt



section .text
[global idt_flush]
idt_flush:
    lidt [rdi]              ; Load the IDT pointer
    ret

; Setup Interrupt Service Routine(ISR)
%macro ISR_NOERRCODE 1
    [global isr%1]
    isr%1:
        push 0               ; Pushing Dummy error code into stack
        push %1              ; Pushing Interrupt number into stack
        jmp isr_common_stub
%endmacro

%macro ISR_ERRCODE 1
    [global isr%1]
    isr%1:
        ; don't need to push error code as it is autometically push
        push %1              ; Pusing Interrupt number only into the stack
        jmp isr_common_stub
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
        push 0        ; Push a dummy error code  
        push %2       ; Push irq code
        jmp irq_common_stub
%endmacro


; This is a stub that we have created for IRQ based ISRs. This calls
irq_common_stub:
    SAVE_REGISTERS

    ; Load the kernel data segment descriptor
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Save `iret_*` values into the stack as part of `registers_t`
    ; Calculate the correct offsets based on the new structure layout
    mov rax, [rsp + 8*0]     ; `iret_rip`
    mov [rsp + 8*19], rax    ; Offset 19 corresponds to `iret_rip` in `registers_t`

    mov rax, [rsp + 8*1]     ; `iret_cs`
    mov [rsp + 8*20], rax    ; Offset 20 corresponds to `iret_cs`

    mov rax, [rsp + 8*2]     ; `iret_rflags`
    mov [rsp + 8*21], rax    ; Offset 21 corresponds to `iret_rflags`

    mov rax, [rsp + 8*3]     ; `iret_rsp`
    mov [rsp + 8*22], rax    ; Offset 22 corresponds to `iret_rsp`

    mov rax, [rsp + 8*4]     ; `iret_ss`
    mov [rsp + 8*23], rax    ; Offset 23 corresponds to `iret_ss`

    ; Save interrupt number and error code
    mov rax, [rsp + 8*5]     ; Interrupt number
    mov [rsp + 8*24], rax    ; Offset 24 corresponds to `int_no`
    mov rax, [rsp + 8*6]     ; Error code
    mov [rsp + 8*25], rax    ; Offset 25 corresponds to `err_code`

    ; Pass the current stack pointer to `irq_handler`
    mov rdi, rsp
    cld
    call irq_handler


    RESTORE_REGISTERS
    add rsp, 16              ; Clean up pushed error code and IRQ number
    

    iretq                    ; Return from Interrupt




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




