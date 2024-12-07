;
; This Program remapped kernel into higher memory
;

section .bss
    align 4096
    global table_768         ; Expose table_768 symbol globally
    table_768: resd 1024      ; Reserve space for 1024 entries (4 KB page table)

    
section .text
global fill_table  ; Make the function visible to the linker

fill_table:
    mov eax, 0x0        ; eax = 0
    mov ebx, 0x100000   ; ebx = 0x100000 (1MB start address)

.fill_table_loop:
    mov ecx, ebx        ; ecx = ebx (physical address)
    or ecx, 3           ; ecx = ecx | 3 (mark as present and writable)
    mov [table_768+eax*4], ecx ; Store the page table entry (table_768[eax] = ecx)

    add ebx, 4096       ; ebx = ebx + 4096 (increment to the next 4KB page)
    inc eax             ; eax++

    cmp eax, 1024       ; Compare eax to 1024 (end of loop)
    jne .fill_table_loop; If not 1024, loop again

    ret                 ; Return to caller
