;
; https://wiki.osdev.org/GDT_Tutorial
; 

section .text

extern reload_segments
extern reload_DS

global gdt_flush        ; gd_flush(gdtr_t *gdtr)
global tss_flush        ; void tss_flush(uint16_t selector)


gdt_flush:
   cli
   lgdt [rdi]
   call reload_segments
   sti
   ret


tss_flush:
   cli
   mov ax, di      ; Move the selector into AX (DI is used to pass the argument in x86_64 System V ABI)
   ltr ax          ; Load Task Register with the selector in AX
   sti 
   ret


