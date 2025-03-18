;
; This Programe will set cpu with supplied registers_t cpu state 
; This is building on before 26/02/2025
; This code is also have some error handling
; 

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


.text
global restore_cpu_state 


section .data
    rsp_zero_msg db "rsp is zero", 0x0A, 0x00  ; Message for RSP being zero
    rip_zero_msg db "rip is zero", 0x0A, 0x00  ; Message for RIP being zero

section .text
    global restore_cpu_state
    extern printf                   ; Declare printf in stdio.c

restore_cpu_state:
    cli                             ; Disable interrupts

    ; Restore segment registers if needed
    mov rax, [rdi + SEG_REG_GS]  
    mov gs, ax            
    mov rax, [rdi + SEG_REG_FS]  
    mov fs, ax            
    mov rax, [rdi + SEG_REG_ES]  
    mov es, ax            
    mov rax, [rdi + SEG_REG_DS]  
    mov ds, ax            

    ; Restore general-purpose registers 
    mov rax, [rdi + GEN_REG_RAX]
    mov rbx, [rdi + GEN_REG_RBX]
    mov rcx, [rdi + GEN_REG_RCX]
    mov rdx, [rdi + GEN_REG_RDX]
    mov rbp, [rdi + GEN_REG_RBP]
    ; mov rdi, [rdi + GEN_REG_RDI]  ; The rdi will store at last
    mov rsi, [rdi + GEN_REG_RSI]
    mov r8,  [rdi + GEN_REG_R8]
    mov r9,  [rdi + GEN_REG_R9]
    mov r10, [rdi + GEN_REG_R10]
    mov r11, [rdi + GEN_REG_R11]
    mov r12, [rdi + GEN_REG_R12]
    mov r13, [rdi + GEN_REG_R13]
    mov r14, [rdi + GEN_REG_R14]
    mov r15, [rdi + GEN_REG_R15]

    ; Validate RSP before continuing
    mov rax, [rdi + REG_IRET_RSP]
    test rax, rax
    jz .rsp_zero                    ; If RSP is zero, jump to debug section

    ; Validate RIP before continuing
    mov rax, [rdi + REG_IRET_RIP]
    test rax, rax
    jz .rip_zero                    ; If RIP is zero, jump to debug section

    ; Restore stack for iretq
    mov rax, [rdi + REG_IRET_SS]   
    push rax                        ; Storing iret_ss reg. value into stack by iretq 

    mov rax, [rdi + REG_IRET_RSP]   
    push rax                

    mov rax, [rdi + REG_IRET_RFLAGS]   
    or rax, 0x200                   ; Ensure Interrupt Flag (IF) is set
    push rax                        ; Storing iret_eflags reg. value into stack by iretq 

    mov rax, [rdi + REG_IRET_CS]   
    push rax                        ; Storing iret_cs reg. value into stack by iretq 

    mov rax, [rdi + REG_IRET_RIP]   
    push rax                        ; Storing iret_rip reg. value into stack by iretq 

    mov rdi, [rdi + GEN_REG_RDI]    ; Ultimately set rdi registers

    ; Align the stack to 16 bytes
    and rsp, -16

    sti                             ; Store interrupt 

    iretq                           ; Return from interrupt



; Debug section for RSP being zero
.rsp_zero:
    ; Align the stack to 16 bytes
    and rsp, -16

    ; Prepare arguments for printf
    lea rdi, [rsp_zero_msg]  ; First argument: address of the format string
    xor eax, eax             ; Clear EAX (no floating-point arguments)

    ; Call printf
    call printf

    ; Hang the system
    jmp hang_debug

; Debug section for RIP being zero
.rip_zero:
    ; Align the stack to 16 bytes
    and rsp, -16

    ; Prepare arguments for printf
    lea rdi, [rip_zero_msg]         ; First argument: address of the format string
    xor eax, eax                    ; Clear EAX (no floating-point arguments)

    ; Call printf
    call printf

    ; Hang the system
    jmp hang_debug



; Hang debug section
hang_debug:
    cli                             ; Disable interrupts to prevent unwanted execution


.hang_loop:
    hlt                             ; Halt CPU to save power
    jmp .hang_loop                  ; Infinite loop



    