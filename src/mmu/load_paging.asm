;
; This file also use reloadSegments function from gdt_load.asm file
;

[BITS 64]

section .data
DATA_SEL dw 0x10       ; Data Segment Selector
CODE_SEL dw 0x08       ; Code Segment Selector

section .text
global enable_paging
global disable_paging
extern reloadSegments  ; Defined in gdt_load.asm file

disable_paging:
    ; Skip these 3 lines if paging is already disabled
    mov rbx, cr0
    and rbx, ~(1 << 31)
    mov cr0, rbx

enable_paging:
    ; Enable PAE
    mov rdx, cr4
    or  rdx, (1 << 5)
    mov cr4, rdx

    ; Set LME (long mode enable)
    mov rcx, 0xC0000080
    rdmsr
    or  rax, (1 << 8)
    wrmsr

    and rdi, 0xFFFFFFFFFFFFF000 ; Ensure alignment to 4 KiB

    ; Replace 'pml4_table' with the appropriate physical address (and flags, if applicable)
    mov cr3, rdi

    ; Enable paging (and protected mode, if it isn't already active)
    mov rbx, cr0
    or rbx, (1 << 31)   ; Set PG bit
    mov cr0, rbx

    ; Now reload the segment registers (CS, DS, SS, etc.) with the appropriate segment selectors...

    jmp reloadSegments



