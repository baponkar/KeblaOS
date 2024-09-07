; bootloader.s
; Build Date : 07/09/2024
; Last Update : 07/09/2024
; Developer : Mr. Bapon Kar
; Website : https://github.com/baponkar/KeblaOS
; Description :
; A simple boot sector that prints a message 'Hello' to the screen using a BIOS routine and stack.
;
;


;[ORG 0x7C00]                        ; Shift Memory address 

[bits 16]

section .text
global main

main:
    mov ah, 0x0e                     ; activate BIOS teletype mode to print message

    mov bp, 0x8000                  ; Set Base pointer a high address value to store the data which should  not Boot sector value 
    mov sp, bp                      ; set stack pointer with the same value of bp
                                    ; Stack grows downward 

    push 'o'
    push 'l'
    push 'l'
    push 'e'
    push 'H'

    pop bx
    mov al, bl 
    int 0x10 

    pop bx
    mov al, bl 
    int 0x10 

    pop bx
    mov al, bl 
    int 0x10 

    pop bx
    mov al, bl 
    int 0x10 

    pop bx
    mov al, bl 
    int 0x10 


    hlt




times 510 - ($-$$) db 0             ; Filling with zero
dw 0xaa55                           ; Magic Number i.e. boot sector signature