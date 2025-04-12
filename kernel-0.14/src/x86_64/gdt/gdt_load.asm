;
; https://wiki.osdev.org/GDT_Tutorial
; 

%define KERNEL_CODE 0x08
%define KERNEL_DATA 0x10



section .text
global gdt_flush        ; gd_flush(gdtr_t *gdtr)
global reloadSegments   ; 
global tss_flush        ; void tss_flush(uint16_t selector)

gdt_flush:
   cli
   LGDT  [RDI]
   jmp reloadSegments
   sti
   RET


reloadSegments:
   ; Reload CS register:
   PUSH KERNEL_CODE          ; Push code segment to stack, 0x08 is a stand-in for kernel code segment
   LEA RAX, [rel reload_DS]  ; Load address of reload_DS into RAX, LEA (Load Effective Address), 
   PUSH RAX                  ; Push this value to the stack
   RETFQ                     ; Perform a far return, RETFQ or LRETQ depending on syntax


reload_DS:
   ; Reload data segment registers
   MOV   AX, KERNEL_DATA     ; 0x10 is a stand-in for kernel data segment
   MOV   DS, AX
   MOV   ES, AX
   MOV   FS, AX
   MOV   GS, AX
   MOV   SS, AX
   RET


tss_flush:
   mov ax, di      ; Move the selector into AX (DI is used to pass the argument in x86_64 System V ABI)
   ltr ax          ; Load Task Register with the selector in AX
   ret


