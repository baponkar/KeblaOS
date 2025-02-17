
section .text
[global interrupt_flush]
interrupt_flush:
    lidt [rdi]              ; Load the IDT pointer
    ret
