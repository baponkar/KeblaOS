

#include "../lib/stdio.h"
#include "../bootloader/boot.h"
#include "../memory/detect_memory.h"
#include "vmm.h"

#include "uheap.h"

#define PAGE_SIZE 0x1000

static volatile uint64_t l_va_head = 0x500000 + LOWER_HALF_START_ADDR;


void *uheap_alloc(size_t size, uint8_t type) {
    // Align size to page size (4 KiB)
    size = (size + 0xFFF) & ~0xFFF;

    // Check if we have enough space in the heap
    if ((l_va_head + size) > LOWER_HALF_END_ADDR) {
        printf("Out of memory\n");
        return NULL;                    // Out of heap space
    }

    // Allocate virtual pages for the requested size
    uint64_t va = l_va_head;
    
    while (l_va_head < va + size) {
        vm_alloc(l_va_head, type);            // allocating by vm_alloc function
        l_va_head+= PAGE_SIZE;          // Increment by page size (size)
    }

    // Add 4KB padding between allocations to prevent overlapping
    l_va_head += PAGE_SIZE;

    return (void *)va; // Return the start of the allocated region
}


void uheap_free(void *ptr, size_t size) {
    if (!ptr || size == 0) {
        printf("ptr | size == 0\n");
        return;
    }

    // Align size to page size (4 KiB)
    size = (size + 0xFFF) & ~0xFFF;

    uint64_t va = (uint64_t)ptr;    // Get the virtual address of the pointer

    // Free the pages corresponding to the memory region
    while (size > 0) {              // If size greater than zero
        vm_free((uint64_t *)va);    // Free the virtual page
        va += PAGE_SIZE;            // Increase Virtual Address by 4KB
        size -= PAGE_SIZE;          // Decrease the size variable by 4KB
    }
}







