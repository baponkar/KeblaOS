
section .text
global load_cr3

; Function to set CR3 to the given address (passed from C as an argument)
load_cr3:
    mov rax, rdi               ; Load the first argument (PML4_BASE_ADDRESS) into RAX
    mov cr3, rax               ; Set CR3 to the address in RAX
    ret

