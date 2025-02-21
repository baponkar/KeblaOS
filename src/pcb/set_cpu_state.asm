
global restore_cpu_state

restore_cpu_state:
    mov r15, rdi           ; Save original rdi in r15

    mov rax, [r15 + 8*0]   ; Load gs
    mov gs, ax
    mov rax, [r15 + 8*1]   ; Load fs
    mov fs, ax
    mov rax, [r15 + 8*2]   ; Load es
    mov es, ax
    mov rax, [r15 + 8*3]   ; Load ds
    mov ds, ax

    ; Restore general-purpose registers
    mov rax, [r15 + 8*4]   ; Restore rax
    mov rbx, [r15 + 8*5]   ; Restore rbx
    mov rcx, [r15 + 8*6]   ; Restore rcx
    mov rdx, [r15 + 8*7]   ; Restore rdx
    mov rbp, [r15 + 8*8]   ; Restore rbp
    mov rdi, [r15 + 8*9]   ; Restore rdi
    mov rsi, [r15 + 8*10]  ; Restore rsi
    mov r8,  [r15 + 8*11]  ; Restore r8
    mov r9,  [r15 + 8*12]  ; Restore r9
    mov r10, [r15 + 8*13]  ; Restore r10
    mov r11, [r15 + 8*14]  ; Restore r11
    mov r12, [r15 + 8*15]  ; Restore r12
    mov r13, [r15 + 8*16]  ; Restore r13
    mov r14, [r15 + 8*17]  ; Restore r14
    mov r15, [r15 + 8*18]  ; Restore r15 (AFTER using it to hold rdi)

    ; Skip int_no (8*19) and err_code (8*20)
    add rdi, 8*21          ; Use rdi again safely

    ; Restore the stack frame for iretq
    mov rax, [rdi + 8*0]   ; Load iret_rip
    mov rcx, [rdi + 8*1]   ; Load iret_cs
    mov rdx, [rdi + 8*2]   ; Load iret_rflags
    mov rbx, [rdi + 8*3]   ; Load iret_rsp
    mov rbp, [rdi + 8*4]   ; Load iret_ss

    ; Push onto stack for iretq
    push rbp
    push rbx
    push rdx
    push rcx
    push rax

    iretq   ; Restore full CPU state and return
