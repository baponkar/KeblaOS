; Offsets for the task_t structure
%define TASK_STRUCT_REGISTERS_OFFSET 24  ; Offset of registers_t* in task_t (adjust if needed)

; Offsets for the registers_t structure
%define REGISTERS_GS          0
%define REGISTERS_FS          8
%define REGISTERS_ES          16
%define REGISTERS_DS          24
%define REGISTERS_RAX         32
%define REGISTERS_RBX         40
%define REGISTERS_RCX         48
%define REGISTERS_RDX         56
%define REGISTERS_RBP         64
%define REGISTERS_RDI         72
%define REGISTERS_RSI         80
%define REGISTERS_R8          88
%define REGISTERS_R9          96
%define REGISTERS_R10         104
%define REGISTERS_R11         112
%define REGISTERS_R12         120
%define REGISTERS_R13         128
%define REGISTERS_R14         136
%define REGISTERS_R15         144
%define REGISTERS_INT_NO      152
%define REGISTERS_ERR_CODE    160
%define REGISTERS_IRET_RIP    168
%define REGISTERS_IRET_CS     176
%define REGISTERS_IRET_RFLAGS 184
%define REGISTERS_IRET_RSP    192
%define REGISTERS_IRET_SS     200

global switch_to_task
switch_to_task:
    ; rdi contains the pointer to the task_t structure (next_task)
    ; Dereference the registers_t* pointer from the task_t structure
    mov rbx, [rdi + TASK_STRUCT_REGISTERS_OFFSET]  ; rbx now points to the registers_t structure

    ; Load the next task's stack pointer
    mov rsp, [rbx + REGISTERS_IRET_RSP]  ; Set the stack pointer to the next task's stack

    ; Restore the next task's registers
    mov rax, [rbx + REGISTERS_RAX]
    mov rbx, [rbx + REGISTERS_RBX]
    mov rcx, [rbx + REGISTERS_RCX]
    mov rdx, [rbx + REGISTERS_RDX]
    mov rbp, [rbx + REGISTERS_RBP]
    mov rsi, [rbx + REGISTERS_RSI]
    mov rdi, [rbx + REGISTERS_RDI]
    mov r8,  [rbx + REGISTERS_R8]
    mov r9,  [rbx + REGISTERS_R9]
    mov r10, [rbx + REGISTERS_R10]
    mov r11, [rbx + REGISTERS_R11]
    mov r12, [rbx + REGISTERS_R12]
    mov r13, [rbx + REGISTERS_R13]
    mov r14, [rbx + REGISTERS_R14]
    mov r15, [rbx + REGISTERS_R15]

    ; Restore the interrupt return state
    mov rax, [rbx + REGISTERS_IRET_RIP]    ; Load the instruction pointer
    mov rcx, [rbx + REGISTERS_IRET_CS]     ; Load the code segment
    mov rdx, [rbx + REGISTERS_IRET_RFLAGS] ; Load the flags register
    mov rbx, [rbx + REGISTERS_IRET_RSP]    ; Load the stack pointer
    mov rsi, [rbx + REGISTERS_IRET_SS]     ; Load the stack segment

    ; Set up the stack for the iretq instruction
    push rsi        ; Push the stack segment
    push rbx        ; Push the stack pointer
    push rdx        ; Push the flags register
    push rcx        ; Push the code segment
    push rax        ; Push the instruction pointer

    ; Perform the context switch using iretq
    iretq