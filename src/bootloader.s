; bootloader.s
; Build Date : 07/09/2024
; Last Update : 07/09/2024
; Developer : Mr. Bapon Kar
; Website : https://github.com/baponkar/KeblaOS
; Description :
; A simple boot sector that prints a message 'Hello' to the screen using a BIOS routine.
;
;


;[ORG 0x7C00]                        ; Shift Memory address 

[bits 16]

section .text
global main

main:
    mov ah, 0x0e                     ; activate BIOS teletype mode to print message

    mov al, 'H'                     ; put asci code of H in al register
    int 0x10                        ; interrupt code to print H

    mov al, 'e'
    int 0x10

    mov al, 'l'
    int 0x10

    mov al, 'l'
    int 0x10

    mov al, 'o'
    int 0x10

    hlt

times 510 - ($-$$) db 0             ; Filling with zero
dw 0xaa55                           ; Magic Number i.e. boot sector signature