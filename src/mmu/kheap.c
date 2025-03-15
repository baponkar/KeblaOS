

#include "../lib/stdio.h"
#include "../bootloader/boot.h"
#include "../bootloader/memory.h"
#include "vmm.h"

#include "kheap.h"

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
        V_KMEM_LOW_BASE += 0x1000; // Increment by page size (4 KiB)
    }

    // Add 4KB padding between allocations to prevent overlapping
    V_KMEM_LOW_BASE += 0x1000;

    // Update the maximum allocated address
    if (V_KMEM_LOW_BASE > V_KMEM_UP_BASE) {
        V_KMEM_UP_BASE = V_KMEM_LOW_BASE;
    }

    return (void *)va; // Return the start of the allocated region
}


void kheap_free(void *ptr, size_t size) {
    if (!ptr || size == 0) {
        return;
    }

    // Align size to page size (4 KiB)
    size = (size + 0xFFF) & ~0xFFF;

    uint64_t va = (uint64_t)ptr;

    // Free the pages corresponding to the memory region
    while (size > 0) {
        vm_free((void *)va); // Free the virtual page
        va += 0x1000;        // Move to the next page
        size -= 0x1000;      // Reduce the remaining size
    }
}

void test_kheap(){
    uint64_t *ptr = (uint64_t *) kheap_alloc(25);
    *ptr = 0xDEADBEF;
    printf("Successfully tested kheap at address : %x\n", *ptr);
}
