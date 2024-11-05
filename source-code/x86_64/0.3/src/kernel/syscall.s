section .text
global sys_write
global sys_read

; Function to write a string to stdout
; int sys_write(const char *str);
sys_write:
    ; Prepare the syscall
    mov rax, 1           ; Syscall number for write (1)
    mov rdi, 1           ; File descriptor (1 = stdout)
    mov rsi, rdi         ; Move the address of the string to rsi (argument)
    mov rdx, rsi         ; Set the length of the string in rdx (for example, hard-coded length or calculated)
    
    ; Assuming that rsi points to the string and rdx holds the length
    ; Note: You need to provide a way to determine the string length before calling this
    syscall               ; Invoke syscall
    ret                   ; Return from the function

; Function to read a string from stdin
; int sys_read(char *buffer);
sys_read:
    ; Prepare the syscall
    mov rax, 0           ; Syscall number for read (0)
    mov rdi, 0           ; File descriptor (0 = stdin)
    mov rsi, rdi         ; Move the address of the buffer to rsi
    mov rdx, rsi         ; Set the maximum number of bytes to read (same as buffer size)
    
    ; Assuming that rsi points to the buffer and rdx holds the size
    syscall               ; Invoke syscall
    ret                   ; Return from the function
