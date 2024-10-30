[EXTERN print_registers_c]

[GLOBAL print_registers]



print_registers:
    pusha               ; Push all general-purpose registers to the stack
    call print_registers_c  ; Call the C function to print them
    popa                ; Restore all general-purpose registers
    ret
