

section .text
global user_stub
global user_stub_end

user_stub:
    mov rax, 1
    mov rdi, msg
    syscall

    mov rax, 3
    syscall

.hang:
    hlt
    jmp .hang

user_stub_end:


section .data
msg: db "Hello from userspace!", 10, 0  ; 10 is asci code of newline character \n



