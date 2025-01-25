#pragma once

#include <stdint.h>

struct registers { // Total 26*8 = 208 bytes which is 16 bytes aligned
    // CPU state
    uint64_t iret_rip;      // Offset 8*0
    uint64_t iret_cs;       // Offset 8*1
    uint64_t iret_rflags;   // Offset 8*2
    uint64_t iret_rsp;      // Offset 8*3
    uint64_t iret_ss;       // Offset 8*4

    uint64_t err_code;      // Offset 8*5
    uint64_t int_no;        // Offset 8*6

    // General purpose registers
    uint64_t r15;           // Offset 8*7
    uint64_t r14;           // Offset 8*8
    uint64_t r13;           // Offset 8*9
    uint64_t r12;           // Offset 8*10
    uint64_t r11;           // Offset 8*11
    uint64_t r10;           // Offset 8*12
    uint64_t r9;           // Offset 8*13
    uint64_t r8;            // Offset 8*14
    uint64_t rsi;            // Offset 8*15
    uint64_t rdi;           // Offset 8*16
    uint64_t rbp;           // Offset 8*17
    uint64_t rdx;           // Offset 8*18
    uint64_t rcx;           // Offset 8*19
    uint64_t rbx;           // Offset 8*20
    uint64_t rax;           // Offset 8*21

    // Segment registers
    uint64_t ds;            // Offset 8*22
    uint64_t es;            // Offset 8*23
    uint64_t fs;            // Offset 8*24
    uint64_t gs;            // Offset 8*25
} __attribute__((packed));
typedef struct registers registers_t;



void halt_kernel(void);
void print_regs_content(registers_t *regs);
void print_size_with_units(uint64_t size);

