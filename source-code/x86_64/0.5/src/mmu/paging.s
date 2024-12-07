; Skip these 3 lines if paging is already disabled
  mov ebx, cr0
  and ebx, ~(1 << 31)
  mov cr0, ebx

  ; Enable PAE
  mov edx, cr4
  or  edx, (1 << 5)
  mov cr4, edx

  ; Set LME (long mode enable)
  mov ecx, 0xC0000080
  rdmsr
  or  eax, (1 << 8)
  wrmsr

  ; Replace 'pml4_table' with the appropriate physical address (and flags, if applicable)
  mov eax, pml4_table
  mov cr3, eax

  ; Enable paging (and protected mode, if it isn't already active)
  or ebx, (1 << 31) | (1 << 0)
  mov cr0, ebx

  ; Now reload the segment registers (CS, DS, SS, etc.) with the appropriate segment selectors...

  mov ax, DATA_SEL
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  
  ; Reload CS with a 64-bit code selector by performing a long jmp

  jmp CODE_SEL:reloadCS

[BITS 64]
reloadCS:
  hlt   ; Done. Replace these lines with your own code
  jmp reloadCS