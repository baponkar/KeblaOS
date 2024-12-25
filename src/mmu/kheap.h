

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "../kernel/kernel.h"
#include "../driver/vga.h"


uint64_t kmalloc(uint64_t sz); // vanilla (normal).
uint64_t kmalloc_a(uint64_t sz, int align);  // page aligned.
uint64_t kmalloc_p(uint64_t sz, uint64_t *phys); // placed at physical address.
uint64_t kmalloc_ap(uint64_t sz, int align, uint64_t *phys); // page aligned and returns a physical address.

// To implementing dynamic memory allocation
typedef struct heap_block { // 2*(64 + 32) = 192
    uint64_t size;        // Size of the block (excluding metadata)
    int is_free;          // 1 if free, 0 if allocated
    struct heap_block *next; // Pointer to the next block
} heap_block_t;

#define HEAP_BLOCK_SIZE (sizeof(heap_block_t)) // Metadata size

void init_kheap();
void *kmalloc1(uint64_t size);
void kfree1(void *ptr);
void *kmalloc1_a(uint64_t size, int align);
void test_kheap();
size_t get_allocated_size(void *ptr);


