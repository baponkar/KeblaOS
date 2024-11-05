section .text
global _start
extern kmain

; Bootstrap code to set a stack in our kernel BSS space
; and then call our kernel
_start:
    lea rsp, [rel stack.top]
    call kmain
    cli
.hltloop:
    hlt
    jmp .hltloop

section .bss
; 8KiB stack
stack:
    resb 8*1024
.top:

