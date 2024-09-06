;
;This is Program will print the booting message "KeblaOS is Booting... by using BIOS interrupt routine"
;To make the main.img run make
;To run on Emulator run qemu-system-i386 -fda build/main.img
;Development Date : 06/09/2024
;Version : 0.0.0
;Youtube Tutorial : https://www.youtube.com/watch?v=zHJ2vlBnNr0&list=PL2EF13wm-hWCoj6tUBGUmrkJmH1972dBB&index=31
;
;




[ORG 0x7C00]		;Shift the initial memory address

[BITS 16]


main:
	mov ax, 0		;Clear ax register 
	mov ds, ax		;Clear data segment register
	mov es, ax		;clear extra segment register
	mov ss, ax		;clear stack segment register 

	mov sp, 0x7C00	;initialize stack pointer register with start of memory address 

	mov si, OS_boot_msg
	call print

	HLT


hlt:
	JMP hlt


print:
	push si 
	push ax 
	push bx 

print_loop:
	lodsb 
	or al, al 
	jz done_print	;if al is zero then jump into the done_print label

					;BIOS interrupt
	mov ah, 0x0E	;printing character on output
	mov bh, 0x0		;page number
	int 0x10		;video interrupt 

	jmp print_loop


done_print:
	pop bx
	pop ax 
	pop si
	ret



OS_boot_msg: dd "KeblaOS is Booting...", 0x0D, 0x0A, 0


TIMES 510-($-$$) DB 0	;Fill the remaining address with 0

DW 0x0AA55
