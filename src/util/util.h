#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


// Saved State in registers
// typedef struct registers
// {
//     uint64_t rax, rbx, rcx, rdx, rbp, rsi, rdi;    // General-purpose registers
//     uint64_t r8, r9, r10, r11, r12, r13, r14, r15; // General-purpose registers
//     uint64_t gs, fs, es, ds;                       // Segment registers

//     uint64_t int_no, err_code;                     // Interrupt number and error code (if applicable)
//     uint64_t iret_rip, iret_cs, iret_rflags, iret_rsp, iret_ss; // CPU state
// } registers_t;



// Saved State in register
typedef struct registers
{
    uint64_t ds, es, fs, gs;    // Segment registers
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;  // General-purpose registers
    uint64_t rdi, rsi, rbp, rdx, rcx, rbx, rax;
    uint64_t int_no, err_code;  // Interrupt number and error code
    uint64_t iret_rip, iret_cs, iret_rflags, iret_rsp, iret_ss; // CPU state
} registers_t;


void disable_interrupts();
void enable_interrupts();
void halt_kernel(void);


