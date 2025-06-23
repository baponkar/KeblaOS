

#include "../lib/stdio.h"
#include "../bootloader/boot.h"
#include "../memory/detect_memory.h"
#include "vmm.h"

#include "kheap.h"

#define PAGE_SIZE 0x1000

static uint64_t h_va_head = HIGHER_HALF_START_ADDR;


void *kheap_alloc(size_t size, uint8_t type) {
    // Align size to page size (4 KiB)
    size = (size + 0xFFF) & ~0xFFF;

    // Check if we have enough space in the heap
    if ((h_va_head + size) > HIGHER_HALF_END_ADDR) {
        printf("Out of memory\n");
        return NULL; // Out of heap space
    }

    // Allocate virtual pages for the requested size
    uint64_t va = h_va_head;
    
    while (h_va_head < va + size) {
        vm_alloc(h_va_head, type);           // allocating by vm_alloc function
        h_va_head += PAGE_SIZE;        // Increment by page size (size)
    }

    // Add 4KB padding between allocations to prevent overlapping
    h_va_head += PAGE_SIZE;


    return (void *)va; // Return the start of the allocated region
}


void kheap_free(void *ptr, size_t size) {
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

void test_kheap(){

    // First Creating a virtual pointer and assigning a value to it
    uint64_t *vir_ptr = kheap_alloc(0x8000, ALLOCATE_DATA);    // Allocate 32 KB
    *vir_ptr = 0xDEADBEF;                       // Assign a value to the allocated memory
    printf("Allocated memory at: %x, Value: %x\n",(uint64_t) vir_ptr, *vir_ptr); // Allocated memory at: 0xFFFF800000001000, Value: 0xDEABEEF

    // Converting the virtual pointer to a physical pointer and assigning a value to it
    uint16_t *phy_ptr = (uint16_t *) vir_to_phys((uint64_t)vir_ptr);   // Converting Virtual to physical pointer
    *phy_ptr = 0xBEEF;                                                 // Assign a value at the physical pointer
    printf("Allocated memory at: %x, Value: %x\n", (uint64_t)phy_ptr, *phy_ptr);  // Allocated memory at: 0x1000, Value: 0xBEEF
    
    // Convert the physical pointer back to a virtual pointer and print the value
    void *vir_ptr_1 = (void *) phys_to_vir((uint64_t)phy_ptr);          // Convert to virtual address again
    printf("Converted to virtual address: %x\n", (uint64_t) vir_ptr_1); // Converted to virtual address: 0xFFFF800000001000

    // Free the allocated memory
    // kheap_free((void *)vir_ptr, 0x8000); // Page fault! ( not present ) at address 0x200210A78
    kheap_free(vir_ptr_1, 0x8000);       // Page fault! ( not present ) at address 0x200210A78

}





