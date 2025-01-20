

global switch_to_process
switch_to_process:
    ; Save the current process state
    mov [rdi + 0x08], rsp       ; Save stack pointer
    mov rax, cr3
    mov [rdi + 0x10], rax       ; Save CR3 register

    ; Optionally save general-purpose registers (if required)
    mov [rdi + 0x18], rbx       ; Save RBX
    mov [rdi + 0x20], r12       ; Save R12
    mov [rdi + 0x28], r13       ; Save R13
    mov [rdi + 0x30], r14       ; Save R14
    mov [rdi + 0x38], r15       ; Save R15

    ; Switch to the new process
    mov rax, [rsi + 0x10]       ; Load new CR3 register
    mov cr3, rax
    mov rsp, [rsi + 0x08]       ; Load new stack pointer

    ; Optionally restore general-purpose registers (if required)
    mov rbx, [rsi + 0x18]       ; Restore RBX
    mov r12, [rsi + 0x20]       ; Restore R12
    mov r13, [rsi + 0x28]       ; Restore R13
    mov r14, [rsi + 0x30]       ; Restore R14
    mov r15, [rsi + 0x38]       ; Restore R15

    ret
