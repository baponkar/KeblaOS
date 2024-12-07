[bits 32]

[global _read_cr0]
_read_cr0:
	mov eax, cr0
	retn

[global _write_cr0]
_write_cr0:
	push ebp
	mov ebp, esp
	mov eax, [ebp+8]
	mov cr0,  eax
	pop ebp
	retn

[global _read_cr3]
_read_cr3:
	mov eax, cr3
	retn

[global _write_cr3]
_write_cr3:
	push ebp
	mov ebp, esp
	mov eax, [ebp+8] ; Taking First argument as input
	mov cr3, eax	 ; Write cr3 with first argument
	pop ebp
	retn
