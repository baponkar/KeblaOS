#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "../driver/vga.h"


// Saved State in register
struct registers
{
    uint64_t ds, es, fs, gs;    // Segment registers
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;  // General-purpose registers
    uint64_t rdi, rsi, rbp, rdx, rcx, rbx, rax;
    uint64_t int_no, err_code;  // Interrupt number and error code
    uint64_t iret_rip, iret_cs, iret_rflags, iret_rsp, iret_ss; // CPU state
}__attribute__((packed));
typedef struct registers registers_t;


void halt_kernel(void);
void print_regs_content(registers_t *regs);
void print_size_with_units(uint64_t size);
