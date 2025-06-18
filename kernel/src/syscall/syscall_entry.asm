
global syscall_entry
extern syscall_handler
extern kernel_stack     ; pointer to kernel stack, should be per-core ideally

section .text
syscall_entry:
    swapgs                    ; switch GS base with KernelGSBase

    mov [gs:0x10], rsp       ; Save old kernel RSP
    mov rsp, [gs:8]            ; Load per-core kernel stack
    sub rsp, 8*6              ; make room for pushes (align if needed)

    ; Don't touch RCX or R11! They are used by sysretq.
    push rcx 
    push r11 

    ; Save other registers
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

    swapgs

    ; Return to user — RCX and R11 must still hold original values
    sysretq
