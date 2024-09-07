; Bootloader.s
; Build Date : 07/09/2024
; Last Update : 07/09/2024
; Developer : Mr. Bapon Kar
; Website : https://github.com/baponkar/KeblaOS
; Description :
; A simple boot sector program that loop foreever
;
;


;[ORG 0x7C00]                        ; Shift Memory address 

[bits 16]

section .text
global main

main:
    jmp main

times 510 - ($-$$) db 0             ; Filling with zero
dw 0xaa55                           ; Magic Number i.e. boot sector signature