

#include "../lib/stdio.h"
#include "../bootloader/boot.h"
#include "../memory/detect_memory.h"
#include "vmm.h"

#include "kheap.h"

#define PAGE_SIZE 0x1000
extern uint64_t V_KMEM_LOW_BASE;
extern uint64_t V_KMEM_UP_BASE;


void *kheap_alloc(size_t size) {
    // Align size to page size (4 KiB)
    size = (size + 0xFFF) & ~0xFFF;

    // Check if we have enough space in the heap
    if ((V_KMEM_LOW_BASE + size) > V_KMEM_UP_BASE) {
        printf("Out of memory\n");
        return NULL; // Out of heap space
    }

    // Allocate virtual pages for the requested size
    uint64_t va = V_KMEM_LOW_BASE;
    while (V_KMEM_LOW_BASE < va + size) {
        vm_alloc(V_KMEM_LOW_BASE); // Use the previous vm_alloc
        V_KMEM_LOW_BASE += PAGE_SIZE; // Increment by page size (size)
    }

    // Add 4KB padding between allocations to prevent overlapping
    // V_KMEM_LOW_BASE += PAGE_SIZE;

    // Update the maximum allocated address
    if (V_KMEM_LOW_BASE > V_KMEM_UP_BASE) {
        V_KMEM_UP_BASE = V_KMEM_LOW_BASE;
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
