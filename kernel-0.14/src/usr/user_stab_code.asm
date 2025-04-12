

section .text
global user_stub
global user_stub_end

extern start_kshell
extern print

user_stub:

    int 172
    ;int 173
    ;int 174

    mov rax, msg 
    mov rdi, rax 
    call print 
    call start_kshell

    ;mov rax, 1  ; Printing System Call
    ;syscall

    ;mov rax, 3  ; Exit System Call 
    ;syscall 



.hang:
    hlt
    jmp .hang

user_stub_end: 


section .data
msg: db "Hello from userspace!", 10, 0  ; 10 is asci code of newline character \n



