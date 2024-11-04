section .text
global check_cpuid

check_cpuid:
    pushfq                               ; Save RFLAGS
    pushfq                               ; Store RFLAGS
    xor qword [rsp], 0x00200000          ; Invert the ID bit in stored RFLAGS
    popfq                                ; Load modified RFLAGS (with ID bit inverted)
    pushfq                               ; Store RFLAGS again (ID bit may or may not be inverted)
    pop rax                              ; RAX = modified RFLAGS (ID bit may or may not be inverted)
    xor rax, [rsp]                       ; RAX = bits that were changed (ID bit, if changed)
    popfq                                ; Restore original RFLAGS
    and rax, 0x00200000                  ; RAX = zero if ID bit can't be changed, else non-zero
    ret

