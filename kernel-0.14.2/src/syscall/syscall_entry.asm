
global syscall_entry
extern syscall_handler

section .text
syscall_entry:
    ; Don't touch RCX or R11! They are used by sysretq.
    push rcx 
    push r11 

    ; Save other registers if needed
    push rdi
    push rsi
    push rdx
    push rax

    ; Arguments to syscall_handler(syscall_num, arg1)
    mov rsi, rdi         ; user argument → RSI
    mov rdi, rax         ; syscall number → RDI

    call syscall_handler

    ; Restore registers
    pop rax
    pop rdx
    pop rsi
    pop rdi

    pop r11 
    pop rcx 

    ; Return to user — RCX and R11 must still hold original values
    sysretq
