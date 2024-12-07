#pragma once

#include <stdint.h>
#include <stddef.h>


// Saved State in register

typedef struct registers
{
 
    // General purpose registers
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rbp;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    uint64_t rax;

    uint64_t int_no;
    uint64_t err_code;
    uint64_t rip, cs, rflags, rsp, ss; // CPU state : Pushed by the processor automatically.
} registers_t;

// typedef struct registers
// {
//    uint64_t cr2;
//    uint64_t ds;                  // Data segment selector
//    uint64_t rdi, rsi, rbp, rsp, rbx, rdx, rcx, rax; // Pushed by pusha.
//    uint64_t int_no, err_code;    // Interrupt number and error code (if applicable)
//    uint64_t rip, cs, rflags, userrsp, ss; // Pushed by the processor automatically.
// } registers_t;


#define BLOCK_SIZE sizeof(block_t)

typedef struct block {
    size_t size;           // Size of the block (excluding the metadata)
    int free;              // Whether the block is free or not
    struct block *next;    // Pointer to the next block in the free list
} block_t;


void disable_interrupts();
void enable_interrupts();

void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
void *memmove(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);


