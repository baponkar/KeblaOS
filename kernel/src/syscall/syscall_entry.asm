
global syscall_entry
extern syscall_handler


section .text
syscall_entry:
    swapgs               ; switch GS base with KernelGSBase

    mov [gs:8], rsp     ; Save current user RSP in [gs:16]
    mov rsp, [gs:0]      ; Load per-core kernel stack
    sub rsp, 9*8         ; make room for pushes (align if needed)

    ; Saving Registers into kernel stack
    push rcx 
    push r11 
    push rdi
    push rsi
    push rdx
    push rax
    push r8
    push r9
    push r10

    ; Arguments to syscall_handler(syscall_num, arg1, arg2)   
    mov rdi, rax         ; syscall number
    mov rsi, rdi         ; argument 1
    mov rdx, rsi         ; argument 2

    call syscall_handler

    ; Restore registers from kernel stack
    pop r10
    pop r9
    pop r8
    pop rax
    pop rdx
    pop rsi
    pop rdi
    pop r11 
    pop rcx 

    mov rsp, [gs:8]      ; Restore user rsp from gs:16
    swapgs                ; switch GS base with KernelGSBase

    ; Return to user — RCX and R11 must still hold original values
    sysretq

