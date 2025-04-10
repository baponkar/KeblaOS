;
; This code push a dummy error code which isr not pushing error code and pushing interrupt number
; inside the stack. Then call the isr_handler.
;



[extern isr_handler]        ; defined in pic.c

%macro ISR_NOERRCODE 1
    [global isr%1]
    isr%1:
        cli;

        push 0          ; Dummy error code
        push %1         ; Interrupt number
        
        push r15        ; Save general-purpose registers in reverse order (to match RESTORE_REGISTERS)
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
        mov ax, ds      ; Save segment registers
        push rax
        mov ax, es
        push rax
        push fs
        push gs
        
        mov rdi, rsp         ; Pass pointer to the `registers_t` structure
        cld                  ; Clear the direction flag
        call isr_handler     ; Call the interrupt handler

        pop gs               ; Restore segment registers
        pop fs
        pop rax
        mov es, ax
        pop rax
        mov ds, ax
        pop rax             ; Restore general-purpose registers
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
        
        add rsp, 16         ; Clean up interrupt no and dummy error code

        iretq               ; Return from the interrupt using IRETQ (iret values remain intact)
%endmacro



%macro ISR_ERRCODE 1
    [global isr%1]
    isr%1:
        cli
                            ; Do not need to push dummy error code 
        push %1             ; Interrupt number
        
        push r15            ; Save general-purpose registers in reverse order (to match RESTORE_REGISTERS)
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
        
        mov ax, ds           ; Save segment registers
        push rax
        mov ax, es
        push rax
        push fs
        push gs

        mov rdi, rsp         ; Pass pointer to the `registers_t` structure
        cld                  ; Clear the direction flag
        call isr_handler     ; Call the interrupt handler

        pop gs               ; Restore segment registers
        pop fs
        pop rax
        mov es, ax
        pop rax
        mov ds, ax
        pop rax             ; Restore general-purpose registers
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

        iretq                ; Return from the interrupt using IRETQ
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


ISR_NOERRCODE 48   ; APIC Timer 
ISR_NOERRCODE 49   ; HPET Timer


