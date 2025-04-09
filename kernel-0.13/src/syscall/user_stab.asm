
section .text
global user_stub

user_stub:
    ; Set syscall number in RAX (1 in this case)
    mov rax, 1

    ; Set argument in RDI (for example, 1234)
    mov rdi, 1234

    ; Call syscall
    syscall

    ; After syscall, halt or loop
    hlt

.loop:
    jmp .loop

