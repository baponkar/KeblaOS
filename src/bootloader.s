
; bootloader.s
; Build Date : 07/09/2024
; Last Update : 08/09/2024
; Developer : Mr. Bapon Kar
; Website : https://github.com/baponkar/KeblaOS
; Description :
; This Programm will read the disk and print the disk content
;
;



[ORG 0x7C00]                        ; Shift Memory address 

[BITS 16]

section .text
global main

; Read some sector by using disk_load function 


main:
    mov bp, 0x9000 ; set the stack safely away from us
    mov sp, bp

    mov bx, 0x9000 ; es:bx = 0x0000:0x9000 = 0x09000
    mov dh, 2 ; read 2 sectors
    ; the bios sets 'dl' for our boot disk number
    ; if you have trouble, use the '-fda' flag: 'qemu -fda file.bin'
    call disk_load

    mov dx, [0x9000] ; retrieve the first loaded word, 0xdada
    call print_hex

    call print_nl

    mov dx, [0x9000 + 512] ; first word from second loaded sector, 0xface
    call print_hex

    jmp $




%include "src/boot_sect_disk.asm"
%include "src/boot_sect_print.asm"
%include "src/boot_sect_print_hex.asm"


BOOT_DRIVE:
    db 0


times 510 - ($-$$) db 0             ; Filling with zero
dw 0xaa55                           ; Magic Number i.e. boot sector signature


; We know that BIOS will load only the first 512 - byte sector from the disk ,
; so if we purposely add a few more sectors to our code by repeating some
; familiar numbers , we can prove to ourselfs that we actually loaded those
; additional two sectors from the disk we booted from.

times 256 dw 0 xdada    ; Sector 2 = 512 byte
times 256 dw 0 xface    ; sector 3 = 512 byte 
