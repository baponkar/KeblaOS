[BITS 64]
section .text
global enable_paging

enable_paging:
    ; Load the address of the PML4 table (passed via RDI)
    mov rax, rdi          ; RDI contains the physical address of the PML4 table
    mov cr3, rax          ; Load PML4 physical address into CR3

    ; Enable PAE (Physical Address Extension) in CR4
    mov rax, cr4
    or rax, (1 << 5)      ; Set the PAE bit
    mov cr4, rax

    ; Enable long mode by setting the LME (Long Mode Enable) bit in the IA32_EFER MSR
    mov ecx, 0xC0000080   ; IA32_EFER MSR
    rdmsr
    or eax, (1 << 8)      ; Set the LME bit
    wrmsr

    ; Enable paging by setting the PG (Paging) bit in CR0
    mov rax, cr0
    or rax, (1 << 31)     ; Set the PG bit (bit 31)
    mov cr0, rax

    ret
