;
; A Simple boot sector that prints a message to the screen using a BIOS routine
;

mov ah, 0x0e ; int 10/ah = 0eh -> scrolling teletype BIOS routine

mov al, 'H' ; Store asci value of  'H' in a register
int 0x10 ; print 'H'

mov al , 'e' ;Store asci value of e in register a
int 0x10 ; print 'e'

mov al, 'l'
int 0x10

mov al, 'l'
int 0x10

mov al, 'o'
int 0x10

jmp $ ; jump to the current address (i.e. foreever)

;
;Padding the magic number

times 510-($-$$) db 0;Pad the boot sector out with zeros

dw 0xaa55 ; Last two bytes from the magic number
	  ; So BIOS knows we are a bootsector
