; This switch_to_process will save current process state and switch into a new process 
; Build Date : 23/02/2025

; Offsets for the registers_t structure
%define SEG_REG_GS  8*0
%define SEG_REG_FS  8*1
%define SEG_REG_ES  8*2
%define SEG_REG_DS  8*3

%define GEN_REG_RAX 8*4
%define GEN_REG_RBX 8*5
%define GEN_REG_RCX 8*6
%define GEN_REG_RDX 8*7
%define GEN_REG_RBP 8*8
%define GEN_REG_RDI 8*9
%define GEN_REG_RSI 8*10
%define GEN_REG_R8  8*11
%define GEN_REG_R9  8*12
%define GEN_REG_R10 8*13
%define GEN_REG_R11 8*14
%define GEN_REG_R12 8*15
%define GEN_REG_R13 8*16
%define GEN_REG_R14 8*17
%define GEN_REG_R15 8*18

%define INT_NO      8*19
%define ERR_CODE    8*20

%define REG_IRET_RIP  8*21
%define REG_IRET_CS   8*22
%define REG_IRET_RFLAGS   8*23
%define REG_IRET_RSP    8*24
%define REG_IRET_SS 8*25

section .text
global switch_to_process

switch_to_process:
    cli                   ; Clear Interrupt Flag (IF) - Disables interrupts
    ; Load process_t* argument (RDI) into RDX to access process->registers
    mov rdx, [rdi + 16]   ; process->registers

    ; Restore general-purpose registers
    mov rax, [rdx + GEN_REG_RAX]
    mov rbx, [rdx + GEN_REG_RBX]
    mov rcx, [rdx + GEN_REG_RCX]
    mov rbp, [rdx + GEN_REG_RBP]
    mov rdi, [rdx + GEN_REG_RDI]
    mov rsi, [rdx + GEN_REG_RSI]
    mov r8,  [rdx + GEN_REG_R8]
    mov r9,  [rdx + GEN_REG_R9]
    mov r10, [rdx + GEN_REG_R10]
    mov r11, [rdx + GEN_REG_R11]
    mov r12, [rdx + GEN_REG_R12]
    mov r13, [rdx + GEN_REG_R13]
    mov r14, [rdx + GEN_REG_R14]
    mov r15, [rdx + GEN_REG_R15]

    ; Restore segment registers
    mov gs, [rdx + SEG_REG_GS]
    mov fs, [rdx + SEG_REG_FS]
    mov es, [rdx + SEG_REG_ES]
    mov ds, [rdx + SEG_REG_DS]

    ; Load the saved stack pointer
    mov rsp, [rdx + REG_IRET_RSP]

    ; Prepare to return to the saved process state using IRETQ
    push qword [rdx + REG_IRET_SS]   ; SS (stack segment)
    push qword [rdx + REG_IRET_RSP]   ; RSP (stack pointer)
    push qword [rdx + REG_IRET_RFLAGS]   ; RFLAGS (flags register)
    push qword [rdx + REG_IRET_CS]   ; CS (code segment)
    push qword [rdx + REG_IRET_RIP]   ; RIP (return instruction pointer)

    mov rdx, [rdx + GEN_REG_RDX]
    sti

    ; Perform IRETQ to restore execution context
    iretq
