section .text
global switch_to_user_mode

; Function to switch from kernel mode to user mode
; Parameters:
;   esp_user_stack - the user-mode stack pointer
;   user_entry - the entry point for the user-mode code
switch_to_user_mode:
    cli                              ; Clear interrupts
    mov ax, 0x23                     ; Load user data segment selector (DPL=3)
    mov ds, ax                       ; Set DS register to user data segment
    mov es, ax                       ; Set ES register to user data segment
    mov fs, ax                       ; Set FS register to user data segment
    mov gs, ax                       ; Set GS register to user data segment

    ; Load user-mode stack
    mov eax, [esp + 4]               ; Get the user stack pointer (first argument)
    mov ss, ax                       ; Set SS to user data segment
    mov esp, eax                     ; Load ESP with the user-mode stack

    ; Set up the stack for iret
    push 0x23                        ; Push user data segment for SS
    push eax                         ; Push user mode stack pointer for ESP
    pushf                            ; Push EFLAGS
    pop eax                          ; Modify EFLAGS to enable interrupts in user mode
    or eax, 0x200                    ; Set the IF flag to enable interrupts
    push eax                         ; Push modified EFLAGS
    push 0x1B                        ; Push user code segment (DPL=3) for CS
    mov eax, [esp + 8]               ; Get the user entry point address (second argument)
    push eax                         ; Push user entry point for EIP

    iret                             ; Perform interrupt return, switching to user mode


