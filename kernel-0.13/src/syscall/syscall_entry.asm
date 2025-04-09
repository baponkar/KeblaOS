; syscall_entry.asm
global syscall_entry
extern syscall_handler

section .text
global syscall_entry
extern syscall_handler

section .text
syscall_entry:
    ; Save necessary registers
    push r11             ; RFLAGS from SYSCALL
    push rcx             ; Return address from SYSCALL
    push rax             ; Save syscall number
    push rdi             ; Save user's RDI (first argument)

    ; Set up arguments for syscall_handler
    mov rdi, rax         ; First argument: syscall number (from RAX)
    mov rsi, [rsp]       ; Second argument: user's RDI (saved on stack)

    ; Call the C handler
    call syscall_handler

    ; Clean up stack and restore registers
    add rsp, 16          ; Pop RDI and RAX (discard saved values)
    pop rcx              ; Restore RCX (return address)
    pop r11              ; Restore R11 (RFLAGS)

    ; Return to user mode with SYSRET
    sysretq
