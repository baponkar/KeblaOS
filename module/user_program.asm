;
; nasm -g -Wall -f elf64 module/user_program.asm -o user_program.o
; /usr/local/x86_64-elf/bin/x86_64-elf-ld -m elf_x86_64 -nostdlib -static -z max-page-size=0x1000 -T user_linker_x86_64.ld module/user_program.o --oformat=i386:x86-64 -o module/user_program.elf
; /usr/local/x86_64-elf/bin/x86_64-elf-objcopy -O binary module/user_program.elf module/user_program.bin
; objdump -D -b binary -m i386:x86-64 module/user_program.bin
;

;
; user_program.asm
; To test interrupt based system call
;


[BITS 64]
global _start

section .text
_start:
    ;xor ecx, ecx         ; Divisor = 0
    ;div ecx              ; Triggers #DE exception
    ;mov eax, 0x12345678  ; Some dummy code
    ;int 172              ; Int. Reading Systemcall
    ;int 173              ; Int. Printing Systemcall

    ;syscall(SYSCALL_PRINT, (uint64_t)msg, 0)
    mov     rax, 1                  ; SYSCALL_PRINT = 1
    lea     rdi, [rel msg]          ; First argument: pointer to message
    xor     rsi, rsi                ; Second argument (not used)
    syscall                         ; Perform syscall

    ; syscall(120, 0, 0)
    mov rax, 120                       ; Unknown System call  = 0
    xor rdi, rdi                     ; First argument: 0
    xor rsi, rsi                     ; Second argument: 0
    syscall                          ; Perform syscall

    ; syscall(SYSCALL_EXIT, 0, 0)
    mov     rax, 3                  ; SYSCALL_EXIT = 3
    xor     rdi, rdi                ; First argument: exit code 0
    xor     rsi, rsi                ; Second argument (unused)
    syscall                         ; Exit syscall

.hang:
    jmp     .hang                   ; If syscall fails, loop forever

section .data
msg: db "Hello from user via syscall!", 0





