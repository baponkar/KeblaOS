


[BITS 16]
[ORG 0x8000]  ; Match the physical address where the trampoline is loaded

section .text
global trampoline_start
extern get_gdt_ptr_for_ap

trampoline_start:
    cli

    ; Load core_id from fixed address (0x9000)
    mov eax, 0x9000
    mov al, [eax]

    ; Pass core_id as argument (zero-extended to 32 bits)
    movzx edi, al
    call get_gdt_ptr_for_ap  ; Returns GDT pointer in EAX
    a32 lgdt [eax]           ; Use 32-bit address override

    ; Enable protected mode
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; Far jump with explicit 16-bit offset
    jmp 0x08:protected_mode



[BITS 32]
protected_mode:
    ; Set up segment registers
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Jump to kernel entry
    jmp 0x08:0x10000




    