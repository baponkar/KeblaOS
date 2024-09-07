; bootloader.s
; Build Date : 07/09/2024
; Last Update : 07/09/2024
; Developer : Mr. Bapon Kar
; Website : https://github.com/baponkar/KeblaOS
; Description :
; A simple boot sector that prints a message 'Hello' to the screen using a BIOS routine and stack.
;
;


[ORG 0x7C00]                        ; Shift Memory address 

[bits 16]

section .text
global main

main:
    pusha 
    mov bx, my_string
    call my_print_function
    jmp $

my_print_function:
    mov al, [bx]
    
    cmp al, 0
    je done 
    mov ah, 0x0e        ; tty mode
    int 0x10            ; print the character by interrupt routine 
    add bx, 1
    jmp my_print_function

done:
    popa                ; restore all register 
    ret

my_string:
    db "Kebla OS is booting",0




times 510 - ($-$$) db 0             ; Filling with zero
dw 0xaa55                           ; Magic Number i.e. boot sector signature