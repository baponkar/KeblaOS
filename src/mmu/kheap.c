#include "kheap.h"


#define KHEAP_START 0xFFFFFFFF80000000 // Start of the kernel heap
#define KHEAP_END   0xFFFFFFFFFFFFFFFF // End of the kernel heap (e.g., 16 MiB)


static uint64_t kheap_current = KHEAP_START;
static uint64_t kheap_max = KHEAP_END; // Tracks the current maximum allocated address


void *kheap_alloc(size_t size) {
    // Align size to page size (4 KiB)
    size = (size + 0xFFF) & ~0xFFF;

    // Check if we have enough space in the heap
    if ((kheap_current + size) > KHEAP_END) {
        print("Out of memory\n");
        return NULL; // Out of heap space
    }

    // Allocate virtual pages for the requested size
    uint64_t va = kheap_current;
    while (kheap_current < va + size) {
        vm_alloc(kheap_current); // Use the previous vm_alloc
        print("kheap_current :");
        print_hex(kheap_current);
        print("\n");
        kheap_current += 0x1000; // Increment by page size (4 KiB)
    }

    // Update the maximum allocated address
    if (kheap_current > kheap_max) {
        kheap_max = kheap_current;
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

void init_kheap() {
    kheap_current = KHEAP_START;
    kheap_max = KHEAP_START;

    print("Successfully VMM initialized.\n");
}
