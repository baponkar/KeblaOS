
.text
global restore_cpu_state 


restore_cpu_state:
    cli                   ; Clear Interrupt Flag (IF) - Disables interrupts

    mov r15, rdi          ; Save original rdi (points to registers_t structure)

    ; Optional: Restore segment registers if needed
    mov rax, [r15 + 8*0]  ; Load gs
    mov gs, ax            ; Store gs
    mov rax, [r15 + 8*1]  ; Load fs
    mov fs, ax            ; Store fs
    mov rax, [r15 + 8*2]  ; Load es
    mov es, ax            ; Store es
    mov rax, [r15 + 8*3]  ; Load ds
    mov ds, ax            ; Store ds

    ; Restore general-purpose registers 
    mov rax, [r15 + 8*4]
    mov rbx, [r15 + 8*5]
    mov rcx, [r15 + 8*6]
    mov rdx, [r15 + 8*7]
    mov rbp, [r15 + 8*8]
    mov rdi, [r15 + 8*9]
    mov rsi, [r15 + 8*10]
    mov r8,  [r15 + 8*11]
    mov r9,  [r15 + 8*12]
    mov r10, [r15 + 8*13]
    mov r11, [r15 + 8*14]
    mov r12, [r15 + 8*15]
    mov r13, [r15 + 8*16]
    mov r14, [r15 + 8*17]
    
    ; skip for 19, 20

    ; Restore stack for iretq
    mov rax, [r15 + 8*25]   ; Load iret_ss
    push rax                ; Push SS

    mov rax, [r15 + 8*24]   ; Load iret_rsp
    push rax                ; Push RSP

    mov rax, [r15 + 8*23]   ; Load iret_rflags
    push rax                ; Push RFLAGS

    mov rax, [r15 + 8*22]   ; Load iret_cs
    push rax                ; Push CS
    
    mov rax, [r15 + 8*21]   ; Load iret_rip
    push rax                ; Push RIP

    mov r15, [r15 + 8*18]   ; Restore r15 (AFTER using it) 

    sti                     ; Set Interrupt Flag (IF) - Enables interrupts
    iretq                   ; Return from interrupt
    
