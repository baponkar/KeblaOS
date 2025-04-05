

section .text
global switch_to_user_mode

switch_to_user_mode:
    cli                 ; Disable interrupts
    mov ax, 0x23        ; User-mode data segment (ring 3)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push 0x23           ; SS = user data segment
    push rdi            ; Stack address
    pushfq              ; Push RFLAGS (Interrupts enabled)
    push 0x1B           ; CS = user code segment (ring 3)
    push rsi            ; Code address (Entry point)

    iretq               ; Return to user mode


; user_stub.asm
global user_stub
section .text
user_stub:
    ; Minimal stub code that runs in user mode.
    ; For testing, it can loop indefinitely.
    mov rax, 0xDEADBEEF       ; Load a test value into rax (optional)
.loop:
    jmp .loop                 ; Infinite loop
