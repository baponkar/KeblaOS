;
; This file also use reloadSegments function from gdt_load.asm file
;

[BITS 64]

section .data
%define KERNEL_CODE 0x08
%define KERNEL_DATA 0x10

section .text
global enable_paging
global disable_paging
global reload_segments
global reload_DS

disable_paging:
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

    jmp reload_segments


reload_segments:
    ; Make sure stack is 16-byte aligned if needed
    ; Reload CS (Code Segment) using RETFQ
    push    KERNEL_CODE          ; 0x08
    lea     rax, [rel reload_DS] ; RIP for next instruction
    push    rax
    retfq                        ; Far return to reload CS

reload_DS:
    mov     ax, KERNEL_DATA      ; 0x10
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax
    mov     ss, ax
    ret                          ; Return if this function was called


