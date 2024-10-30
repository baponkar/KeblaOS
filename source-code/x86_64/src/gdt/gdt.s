; gdt.s
    section .data
gdtr:
    dw 0               ; For GDT limit
    dq 0               ; For GDT base

    section .text
    global setGdt      ; Make setGdt available to link with C

setGdt:
    mov [gdtr], di     ; Set limit from DI (GDT limit)
    mov [gdtr + 2], rsi ; Set base from RSI (GDT base address)
    lgdt [gdtr]        ; Load the GDT register with new GDT
    ret

