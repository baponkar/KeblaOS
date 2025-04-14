

section .text
global switch_to_user_mode  ; using switch_to_user_mode(uint64_t stack_addr, uint64_t code_addr) in c

switch_to_user_mode:
    cli                     ; Disable interrupts
    mov ax, 0x23            ; User-mode data segment (ring 3)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push 0x23               ; SS = user data segment
    push rdi                ; Stack address
    ;pushfq                  ; Push RFLAGS (Interrupts enabled)
    push 0x202
    push 0x1B               ; CS = user code segment (ring 3)
    push rsi                ; Code address (Entry point)

    iretq                   ; Return to user mode

