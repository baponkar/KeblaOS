; user_shell.asm
BITS 64
GLOBAL _start

SECTION .data
prompt_msg db "user@keblaOS> ", 0
newline    db 10, 0
buffer     times 128 db 0

SECTION .text

_start:
.loop:
    ; Print prompt
    mov rax, 1              ; syscall: print
    mov rbx, prompt_msg
    int 0xac

    ; Read input into buffer
    mov rax, 0              ; syscall: read
    mov rbx, buffer         ; user buffer
    mov rcx, 127            ; max 127 bytes (leave space for null)
    int 0xac                ; syscall

    ; Null-terminate the input (buffer[rax] = 0)
    mov rdx, rax            ; rdx = bytes read
    mov rax, 0              ; clear rax
    mov [buffer + rdx], al  ; buffer[rdx] = 0

    ; Print newline
    mov rax, 1
    mov rbx, newline
    int 0xac

    ; Echo back user input
    mov rax, 1
    mov rbx, buffer
    int 0xac

    ; Print newline again
    mov rax, 1
    mov rbx, newline
    int 0xac

    jmp .loop               ; repeat forever
