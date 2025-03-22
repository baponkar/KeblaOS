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
%define REG_IRET_RSP      8*24
%define REG_IRET_SS 8*25


.text
global restore_cpu_state

restore_cpu_state:
    cli                             ; Disable interrupts

    mov rcx, rdi                    ; Save registers_t pointer

    ; Restore segment registers
    mov rax, [rcx + SEG_REG_GS]  
    mov gs, ax            
    mov rax, [rcx + SEG_REG_FS]  
    mov fs, ax            
    mov rax, [rcx + SEG_REG_ES]  
    mov es, ax            
    mov rax, [rcx + SEG_REG_DS]  
    mov ds, ax            

    ; Restore general-purpose registers 
    mov rax, [rcx + GEN_REG_RAX]
    mov rbx, [rcx + GEN_REG_RBX]
    ; mov rcx, [rcx + GEN_REG_RCX] ; The rcx will store at last
    mov rdx, [rcx + GEN_REG_RDX]
    mov rbp, [rcx + GEN_REG_RBP]
    mov rdi, [rcx + GEN_REG_RDI]  
    mov rsi, [rcx + GEN_REG_RSI]
    mov r8,  [rcx + GEN_REG_R8 ]
    mov r9,  [rcx + GEN_REG_R9 ]
    mov r10, [rcx + GEN_REG_R10]
    mov r11, [rcx + GEN_REG_R11]
    mov r12, [rcx + GEN_REG_R12]
    mov r13, [rcx + GEN_REG_R13]
    mov r14, [rcx + GEN_REG_R14]
    mov r15, [rcx + GEN_REG_R15]

    ; Restore RSP from saved state
    mov rsp, [rcx + REG_IRET_RSP]

    ; Push iretq frame (RIP, CS, RFLAGS)
    push qword [rcx + REG_IRET_SS ]
    push qword [rcx + REG_IRET_RSP]
    push qword [rcx + REG_IRET_RFLAGS]
    or   qword [rsp], 0x200  ; Enable interrupts
    push qword [rcx + REG_IRET_CS]
    push qword [rcx + REG_IRET_RIP]

    mov rcx, [rcx + GEN_REG_RCX]    ; Ultimately set rcx registers

    ; sti                             ; Store interrupt 

    iretq                           ; Return from interrupt

    