
section .text

global idt_flush
global load_tss

idt_flush:
    lidt [rdi]              ; Load the IDT pointer
    ret


load_tss:
    ltr [rdi]      ; Load Task Register with the selector
    ret            ; Return to caller
