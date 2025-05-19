

.text
global switch_to_user_mode ; Get extern void switch_to_user_mode(uint64_t stack_addr, uint64_t code_addr) function from c

switch_to_user_mode:
    cli
    mov ax, 0x23
    mov ds, ax
    mov es, ax
    mov fs, ax 
    mov gs, ax 

    push 0x23
    push rdi

    pushfq 
    pop rax
    or rax, 0x200
    push rax
    
    push 0x1B
    push rsi
    sti
    iretq

    
