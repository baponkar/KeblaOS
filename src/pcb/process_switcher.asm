section .text
global switch_to_task

extern current_process         ; Points to the currently executing process (defined in C)
extern processes_list          ; Head of the process list (for task switching)

switch_to_task:
    ; Save current task's state in its PCB

    mov rax, [current_process]         ; Get the current process control block (PCB)
    test rax, rax
    jz .load_next                      ; If no process is running, just load the next process

    mov [rax + process_t.registers], rsp  ; Save the stack pointer in PCB

    ; Save segment registers
    mov [rax + registers_t.gs], gs
    mov [rax + registers_t.fs], fs
    mov [rax + registers_t.es], es
    mov [rax + registers_t.ds], ds

    ; Save general purpose registers
    mov [rax + registers_t.rax], rax
    mov [rax + registers_t.rbx], rbx
    mov [rax + registers_t.rcx], rcx
    mov [rax + registers_t.rdx], rdx
    mov [rax + registers_t.rbp], rbp
    mov [rax + registers_t.rdi], rdi
    mov [rax + registers_t.rsi], rsi
    mov [rax + registers_t.r8],  r8
    mov [rax + registers_t.r9],  r9
    mov [rax + registers_t.r10], r10
    mov [rax + registers_t.r11], r11
    mov [rax + registers_t.r12], r12
    mov [rax + registers_t.r13], r13
    mov [rax + registers_t.r14], r14
    mov [rax + registers_t.r15], r15

    ; Save interrupt return state (iret registers)
    pushfq                               ; Push RFLAGS to stack
    pop  qword [rax + registers_t.iret_rflags]  ; Save it in PCB

    mov qword [rax + registers_t.iret_rip], [rsp]  ; Save RIP from the stack
    mov qword [rax + registers_t.iret_cs], [rsp+8] ; Save CS
    mov qword [rax + registers_t.iret_rflags], [rsp+16] ; Save RFLAGS
    mov qword [rax + registers_t.iret_rsp], [rsp+24] ; Save RSP
    mov qword [rax + registers_t.iret_ss], [rsp+32] ; Save SS

.load_next:
    ; Load next task's PCB
    mov rbx, [current_process]       ; rbx = current process
    mov rcx, [rbx + process_t.next]  ; rcx = next process in list

    test rcx, rcx                    ; If next process is NULL, loop back
    jnz .set_new_task
    mov rcx, [processes_list]         ; Restart from the first process if end of list is reached

.set_new_task:
    mov [current_process], rcx       ; Update current process to the next task

    ; Restore stack pointer (for new process)
    mov rsp, [rcx + process_t.registers]

    ; Restore segment registers
    mov gs, [rcx + registers_t.gs]
    mov fs, [rcx + registers_t.fs]
    mov es, [rcx + registers_t.es]
    mov ds, [rcx + registers_t.ds]

    ; Restore general purpose registers
    mov rax, [rcx + registers_t.rax]
    mov rbx, [rcx + registers_t.rbx]
    mov rcx, [rcx + registers_t.rcx]
    mov rdx, [rcx + registers_t.rdx]
    mov rbp, [rcx + registers_t.rbp]
    mov rdi, [rcx + registers_t.rdi]
    mov rsi, [rcx + registers_t.rsi]
    mov r8,  [rcx + registers_t.r8]
    mov r9,  [rcx + registers_t.r9]
    mov r10, [rcx + registers_t.r10]
    mov r11, [rcx + registers_t.r11]
    mov r12, [rcx + registers_t.r12]
    mov r13, [rcx + registers_t.r13]
    mov r14, [rcx + registers_t.r14]
    mov r15, [rcx + registers_t.r15]

    ; Restore interrupt return state
    push qword [rcx + registers_t.iret_ss]
    push qword [rcx + registers_t.iret_rsp]
    push qword [rcx + registers_t.iret_rflags]
    push qword [rcx + registers_t.iret_cs]
    push qword [rcx + registers_t.iret_rip]

    ; Resume execution of the next task
    iretq  ; Load new process state and return
