

#include "../lib/stdio.h"
#include "../bootloader/boot.h"
#include "../memory/detect_memory.h"
#include "vmm.h"

#include "kheap.h"

extern uint64_t LOWER_HALF_START_ADDR;
extern uint64_t LOWER_HALF_END_ADDR;


void *uheap_alloc(size_t size) {
    // Align size to page size (4 KiB)
    size = (size + 0xFFF) & ~0xFFF;

    // Check if we have enough space in the heap
    if ((LOWER_HALF_START_ADDR + size) > LOWER_HALF_END_ADDR) {
        printf("Out of memory\n");
        return NULL; // Out of heap space
    }

    // Allocate virtual pages for the requested size
    uint64_t va = LOWER_HALF_START_ADDR;
    while (LOWER_HALF_START_ADDR < va + size) {
        vm_alloc(LOWER_HALF_START_ADDR); // Use the vm_alloc from vmm.c
        LOWER_HALF_START_ADDR += 0x1000; // Increment by page size (4 KiB)
    }

    // Add 4KB padding between allocations to prevent overlapping
    LOWER_HALF_START_ADDR += 0x1000;

    // Update the maximum allocated address
    if (LOWER_HALF_START_ADDR > LOWER_HALF_END_ADDR) {
        LOWER_HALF_END_ADDR = LOWER_HALF_START_ADDR;
    }

    
    return (void *)va; // Return the start of the allocated region
}


void uheap_free(void *ptr, size_t size) {
    if (!ptr || size == 0) {
        return;
    }

    // Align size to page size (4 KiB)
    size = (size + 0xFFF) & ~0xFFF;

    uint64_t va = (uint64_t)ptr;

    // Free the pages corresponding to the memory region
    while (size > 0) {
        vm_free((void *)va); // Free the virtual page
        va += 0x1000;        // Move to the prev page
        size -= 0x1000;      // Reduce the remaining size
    }
}


void test_uheap(){
    uint64_t *ptr = (uint64_t *) uheap_alloc(25);
    *ptr = 0xDEADBEFF;
    printf("Successfully tested uheap at %x\n", *ptr);
}
