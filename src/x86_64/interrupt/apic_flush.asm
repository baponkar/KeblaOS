section .text
[global apic_flush]
apic_flush:
    lidt [rdi]              ; Load the IDT pointer
    ret

   