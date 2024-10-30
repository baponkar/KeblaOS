;
; https://web.archive.org/web/20160326060959/http://jamesmolloy.co.uk/tutorial_html/2.-Genesis.html
; boot.s -- Kernel start location. Also defines multiboot header.
; Based on Bran's kernel development tutorial file start.asm
;

MBOOT_PAGE_ALIGN    equ 1<<0    ; Load kernel and modules on a page boundary
MBOOT_MEM_INFO      equ 1<<1    ; Provide your kernel with memory info
MBOOT_FRAMEBUFFER_INFO equ 0   ; Framebuffer tag request for graphical mode change into 1 << 2

MBOOT_HEADER_MAGIC  equ 0x1BADB002 ; Multiboot Magic valuez

STACK_SIZE equ 0x4000 

MBOOT_HEADER_FLAGS  equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO | MBOOT_FRAMEBUFFER_INFO
MBOOT_CHECKSUM      equ -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)

section .multiboot
align 4
mboot:
  dd  MBOOT_HEADER_MAGIC        ; GRUB will search for this value on each
                                ; 4-byte boundary in your kernel file
  dd  MBOOT_HEADER_FLAGS        ; How GRUB should load your file / settings
  dd  MBOOT_CHECKSUM            ; To ensure that the above values are correct
  DD 0, 0, 0, 0, 0              ; 4 bytes each, total 20 bytes
   
  dd  mboot                     ; Location of this descriptor
  dd  code                      ; Start of kernel '.text' (code) section.
  dd  bss                       ; End of kernel '.data' section.
  dd  end                       ; End of kernel.
  dd  start                     ; Kernel entry point (initial EIP).

section .text
code:
[BITS 32]                       ; All instructions should be 32-bit.

[GLOBAL mboot]                  ; Make 'mboot' accessible from C.
[EXTERN code]                   ; Start of the '.text' section.
[EXTERN bss]                    ; Start of the .bss section.
[EXTERN end]                    ; End of the last loadable section.



[GLOBAL start]                  ; Kernel entry point.
[EXTERN kmain]                  ; This is the entry point of our C code

start:
  mov esp, stack_top; Set stack pointer to the top of the stack
  push ebx                    ; Load multiboot header location, Load multiboot info structure pointer (passed by GRUB)
  push eax                    ; Pass the magic number
  mov eax, [esp + 4]          ; eax should hold the magic number
  mov ebx, [esp + 8]          ; ebx should hold the address of the Multiboot info structure

  cli                         ; Disable interrupts.
  call kmain                  ; call our kmain() function.
  jmp $                       ; Enter an infinite loop, to stop the processor
                              ; executing whatever rubbish is in the memory
                              ; after our kernel!

section .bss
bss:
  resb 1024   ; Reserve 1024 bytes of uninitialized data (example)
stack_bottom:        ; The bottom of the stack (lower address)
    resb 16384 * 8       ; Reserve 16KB for the stack
stack_top:           ; The top of the stack (higher address)
