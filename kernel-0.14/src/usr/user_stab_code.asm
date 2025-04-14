

section .text


global user_stub 
global user_stub_end

user_stub:
    mov eax, 0xCAFEBABE

.loop:
    jmp .loop

user_stub_end: 

