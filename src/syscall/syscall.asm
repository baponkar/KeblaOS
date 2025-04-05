;
;
;

extern syscall_dispatcher;

; syscall_entry.S
global syscall_entry

section .text
syscall_entry:
    ; Save registers (you can choose which ones to save)
    push    rcx            ; Save RCX (will be overwritten by syscall)
    push    r11            ; Save R11 (contains RFLAGS copy)
    ; Optionally save more registers if needed

    ; Call the C syscall dispatcher
    call    syscall_dispatcher

    ; Restore registers and return to user mode with sysret.
    pop     r11
    pop     rcx
    sysret
