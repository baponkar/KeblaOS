

#include "../lib/stdio.h"
#include "../bootloader/boot.h"
#include "../memory/detect_memory.h"
#include "vmm.h"

#include "kheap.h"

#define PAGE_SIZE 0x1000
extern uint64_t HIGHER_HALF_START_ADDR;
extern uint64_t HIGHER_HALF_END_ADDR;


void *kheap_alloc(size_t size) {
    // Align size to page size (4 KiB)
    size = (size + 0xFFF) & ~0xFFF;

    // Check if we have enough space in the heap
    if ((HIGHER_HALF_START_ADDR + size) > HIGHER_HALF_END_ADDR) {
        printf("Out of memory\n");
        return NULL; // Out of heap space
    }

    // Allocate virtual pages for the requested size
    uint64_t va = HIGHER_HALF_START_ADDR;
    while (HIGHER_HALF_START_ADDR < va + size) {
        vm_alloc(HIGHER_HALF_START_ADDR); // Use the previous vm_alloc
        HIGHER_HALF_START_ADDR += PAGE_SIZE; // Increment by page size (size)
    }

    // Add 4KB padding between allocations to prevent overlapping
    // HIGHER_HALF_START_ADDR += PAGE_SIZE;

    // Update the maximum allocated address
    if (HIGHER_HALF_START_ADDR > HIGHER_HALF_END_ADDR) {
        HIGHER_HALF_END_ADDR = HIGHER_HALF_START_ADDR;
    }

    return (void *)va; // Return the start of the allocated region
}


void kheap_free(void *ptr, size_t size) {
    if (!ptr || size == 0) {
        printf("ptr | size == 0\n");
        return;
    }

    // Align size to page size (4 KiB)
    size = (size + 0xFFF) & ~0xFFF;

    uint64_t va = (uint64_t)ptr;

    // Free the pages corresponding to the memory region
    while (size > 0) {          // If size greater than zero
        // printf("\nInside kheap_free: va: %x, size:%x\n", va, size);
        vm_free((void *)va);    // Free the virtual page
        va += PAGE_SIZE;        // Increase Virtual Address by 4KB
        size -= PAGE_SIZE;      // Decrease the size variable by 4KB
    }

    // printf("Successfully cleared memory from kheap_free\n");
}

void test_kheap(){
    uint64_t *ptr = (uint64_t *) kheap_alloc(25);
    *ptr = 0xDEADBEF;
    uint64_t *ptr1 = (uint64_t *) kheap_alloc(40);
    *ptr1 = 0xBABA;
    
    printf("*%x = %x, *%x = %x\n", (uint64_t)ptr, *ptr, (uint64_t)ptr1, *ptr1);
}
