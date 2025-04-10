

section .text
global user_stub
global user_stub_end

user_stub:
    mov rax, 1          ; syscall number (e.g. print)
    mov rdi, msg1        ; argument (message)
    syscall
    ;hlt                 ; halt or infinite loop
    
    mov rax, 3          ; syscall number (e.g. exit)
    mov rdi, msg2
    syscall
    hlt                 ; halt or infinite loop
    
user_stub_end:

section .data
msg1: db "Hello from userspace!", 10, 0  ; 10 is asci code of newline character \n
msg2: db "Exit from userspace!", 10, 0

