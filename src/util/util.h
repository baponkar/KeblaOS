#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


// Saved State in register
typedef struct registers
{
    uint64_t ds, es, fs, gs;                       // Segment registers 
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8; // General-purpose registers
    uint64_t rdi, rsi, rbp, rdx, rcx, rbx, rax;    // General-purpose registers

    uint64_t int_no, err_code;                     // Interrupt number and error code

    uint64_t iret_rip, iret_cs, iret_rflags, iret_rsp, iret_ss; // CPU state
} registers_t; 


#define BLOCK_SIZE sizeof(block_t)

typedef struct block {
    size_t size;           // Size of the block (excluding the metadata)
    int free;              // Whether the block is free or not
    struct block *next;    // Pointer to the next block in the free list
} block_t;


void disable_interrupts();
void enable_interrupts();
void halt_kernel(void);
