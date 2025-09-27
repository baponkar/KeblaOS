
.text
global switch_to_user_mode   ; switch_to_user_mode(stack_ptr, entry_ptr)

switch_to_user_mode:
    cli                      ; Disable interrupts (safe before switching)
    mov ax, 0x23             ; User data segment selector (user DS = 0x23)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push 0x23                ; SS for user mode
    push rdi                 ; RSP (user-mode stack pointer), argument 1

    pushfq                   ; Push RFLAGS
    pop rax
    or rax, 0x200            ; Set IF (interrupt flag)
    push rax

    push 0x1B                ; CS for user mode
    push rsi                 ; RIP (user-mode entry point), argument 2

    iretq                    ; Return to user mode!




