
section .text
[global idt_flush]
idt_flush:
    lidt [rdi]              ; Load the IDT pointer
    ret

    