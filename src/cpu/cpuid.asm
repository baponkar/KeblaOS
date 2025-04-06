;
; Checking is CPUID present or not
;


section .text

global is_cpuid_present

is_cpuid_present:
    pushfq                               ; Save RFLAGS
    pushfq                               ; Store RFLAGS
    xor qword [rsp], 0x00200000          ; Invert the ID bit in stored RFLAGS
    popfq                                ; Load stored RFLAGS (with ID bit inverted)
    pushfq                               ; Store RFLAGS again (ID bit may or may not be inverted)
    pop rax                              ; rax = modified RFLAGS (ID bit may or may not be inverted)
    xor rax, [rsp]                       ; rax = whichever bits were changed
    popfq                                ; Restore original RFLAGS
    and rax, 0x00200000                  ; rax = zero if ID bit can't be changed, else non-zero
    setnz al                             ; Set al to 1 if rax is non-zero, 0 otherwise
    movzx rax, al                        ; Zero-extend al to rax to return as a boolean
    ret


