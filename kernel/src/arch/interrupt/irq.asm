;
; This code push a dummy error code(0) and interrupt number inside
; the stack. Then call irq_handler function
;


[extern irq_handler]   ; defined in pic.c 
[extern irq_handler]   ; defined in pic.c


; Setup Interrupt Request(IRQ)
%macro IRQ 2
    [global irq%1]
    irq%1:
        cli
        ; Stack already has 5*8=40 bytes data
        push 0               ; Dummy error code
        push %2              ; Interrupt number
        
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
        
        mov rdi, rsp                    ; Pass the current stack pointer to `pic_irq_handler`
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


; The first number will define irq function number like irq0, irq2 etc second is interrupt number 
; This will push error code 0
IRQ   0,    32
IRQ   1,    33      ; Keyboard Interrupt
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
IRQ  12,    44      ; Mouse Interrupt
IRQ  13,    45
IRQ  14,    46
IRQ  15,    47

IRQ  16,    48      ; APIC Timer Interrupt
IRQ  17,    49      ; HPET Timer Interrupt
IRQ  18,    50      ; IPI
IRQ  19,    51

; Custom System Call
IRQ  96,    128    ; System Call