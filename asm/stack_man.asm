; A simple boot sector program that demonstrates the stack.

mov ah, 0x0e        ; int 10/ ah = 0eh -> scrolling teletype BIOS routine

mov bp, 0x8000      ; Set the base of the stack a little above where BIOS
mov sp, bp          ; loads our boot sector - so it won't overwrite us.

push 'A'            ; Push the ASCII value of 'A' onto the stack
push 'B'            ; Push the ASCII value of 'B' onto the stack
push 'C'            ; Push the ASCII value of 'C' onto the stack

pop bx              ; Pop the top of the stack into BX
mov al, bl          ; Move the lower byte of BX (which was 'C') into AL
int 0x10            ; Display the character in AL

pop bx              ; Pop the next value off the stack into BX
mov al, bl          ; Move the lower byte of BX (which was 'B') into AL
int 0x10            ; Display the character in AL

pop bx              ; Pop the next value off the stack into BX
mov al, bl          ; Move the lower byte of BX (which was 'A') into AL
int 0x10            ; Display the character in AL

jmp $               ; Infinite loop to keep the program running

times 510-($-$$) db 0  ; Fill the rest of the boot sector with zeros

dw 0xaa55           ; Boot sector signature

