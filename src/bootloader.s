
; bootloader.s
; Build Date : 08/09/2024
; Last Update : 08/09/2024
; Developer : Mr. Bapon Kar
; Website : https://github.com/baponkar/KeblaOS
; Description :
; This Programm will read the disk and print the disk content
; 16 Bit Protected Mode
; Version - 0.0.0.7
;



[ORG 0x7C00]                        ; Shift Memory address 

[BITS 32]


; Define some constants
VIDEO_MEMORY equ 0xb8000
WHITE_ON_BLACK equ 0x0f

my_string:
    db 'Tello',0

; prints a null - terminated string pointed to by EDX
print_string_pm :
    pusha                   ; store all register condition in memory
    mov edx , VIDEO_MEMORY  ; Set edx to the start of vid mem

print_string_pm_loop :
    mov ebx, my_string
    mov al , [ebx] ; Store the char at EBX in AL
    mov ah , WHITE_ON_BLACK ; Store the attributes in AH
    cmp al , 0 ; if (al == 0) , at end of string , so
    je print_string_pm_done ; jump to done
    mov [edx] , ax ; Store char and attributes at current
    ; character cell.
    add ebx , 1 ; Increment EBX to the next char in string.
    add edx , 2 ; Move to next character cell in vid mem.
    jmp print_string_pm_loop ; loop around to print the next char.

print_string_pm_done :
    popa
    ret ; Return from the function



times 510 - ($-$$) db 0             ; Filling with zero
dw 0xaa55                           ; Magic Number i.e. boot sector signature

