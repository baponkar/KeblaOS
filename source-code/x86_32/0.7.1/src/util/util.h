#pragma once

#include "../stdlib/stdint.h"
#include "../stdlib/stddef.h"


void *memset(void *dest, int val, size_t count);
void *memcpy(void *dest, const void *src, size_t count);

typedef struct registers
{
   uint32_t cr2;
   uint32_t ds;                  // Data segment selector
   uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha.
   uint32_t int_no, err_code;    // Interrupt number and error code (if applicable)
   uint32_t eip, cs, eflags, useresp, ss; // Pushed by the processor automatically.
} registers_t;

typedef struct block {
    size_t size;           // Size of the block (excluding the metadata)
    int free;              // Whether the block is free or not
    struct block *next;    // Pointer to the next block in the free list
} block_t;

#define BLOCK_SIZE sizeof(block_t)
