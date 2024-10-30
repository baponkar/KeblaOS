#pragma once

#include <stdint.h>
#include <stddef.h>



typedef struct registers
{
    uint64_t cr2;                // Control Register 2 (page fault address)
    uint64_t ds;                 // Data segment selector (not generally used in 64-bit)
    
    uint64_t rdi, rsi, rbp, rsp, rbx, rdx, rcx, rax; // General-purpose registers pushed manually
    
    uint64_t int_no, err_code;   // Interrupt number and error code (if applicable)
    
    uint64_t rip, cs, rflags, user_rsp, ss; // Pushed by the processor automatically

} registers_t;

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