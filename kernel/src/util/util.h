#pragma once

#include <stdint.h>

// cpu state 
struct registers { // Total 26*8 = 208 bytes, 16 bytes aligned
    // Segment registers
    uint64_t gs;            // Offset 8*0 bytes
    uint64_t fs;            // Offset 8*1 bytes
    uint64_t es;            // Offset 8*2 bytes
    uint64_t ds;            // Offset 8*3 bytes

    // General purpose registers
    uint64_t rax;           // Offset 8*4 bytes
    uint64_t rbx;           // Offset 8*5 bytes
    uint64_t rcx;           // Offset 8*6 bytes
    uint64_t rdx;           // Offset 8*7 bytes
    uint64_t rbp;           // Offset 8*8 bytes
    uint64_t rdi;           // Offset 8*9 bytes
    uint64_t rsi;           // Offset 8*10 bytes
    uint64_t r8;            // Offset 8*11 bytes
    uint64_t r9;            // Offset 8*12 bytes
    uint64_t r10;           // Offset 8*13 bytes
    uint64_t r11;           // Offset 8*14 bytes
    uint64_t r12;           // Offset 8*15 bytes
    uint64_t r13;           // Offset 8*16 bytes
    uint64_t r14;           // Offset 8*17 bytes
    uint64_t r15;           // Offset 8*18 bytes

    uint64_t int_no;        // Offset 8*19 bytes
    uint64_t err_code;      // Offset 8*20 bytes

    // the processor pushes the below registers automatically when an interrupt occurs
    uint64_t iret_rip;      // Offset 8*21 bytes, instruction pointer
    uint64_t iret_cs;       // Offset 8*22 bytes, code segment
    uint64_t iret_rflags;   // Offset 8*23 bytes, flags register
    uint64_t iret_rsp;      // Offset 8*24 bytes, stack pointer
    uint64_t iret_ss;       // Offset 8*25 bytes, stack segment
} __attribute__((packed));
typedef struct registers registers_t;


void halt_kernel(void);
void print_size_with_units(uint64_t size);

uint64_t read_rip();
uint64_t read_rsp();
uint64_t read_rflags();

void set_rsp(uint64_t rsp);
void set_rip(uint64_t rip);
void set_rflags(uint64_t flags);

