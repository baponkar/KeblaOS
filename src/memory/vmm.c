
/*
    https://github.com/dreamportdev/Osdev-Notes/blob/master/04_Memory_Management/04_Virtual_Memory_Manager.md
*/

#include "detect_memory.h"
#include "../lib/stdio.h"
#include "paging.h"

#include "vmm.h"

#define LOW_HALF_START 0x0000000000000000ULL
#define LOW_HALF_END   0x7FFFFFFFFFFFFFFFULL

#define UPPER_HALF_START 0xFFFF800000000000ULL
#define UPPER_HALF_END   0xFFFFFFFFFFFFFFFFULL

extern uint64_t V_KMEM_LOW_BASE;
extern uint64_t V_KMEM_UP_BASE;

extern uint64_t V_UMEM_LOW_BASE;
extern uint64_t V_UMEM_UP_BASE;

extern uint64_t PHYSICAL_TO_VIRTUAL_OFFSET;

extern pml4_t *current_pml4;


// Allocate a virtual page at the specified virtual address
void vm_alloc(uint64_t va) {    
    page_t *page = get_page(va, 1, current_pml4); // if not present then create the page

    if (!page) {
        // Handle error if page creation fails
        printf("Page creation failed!\n");
        return;
    }
  
    if (page->present){
        if(va > V_UMEM_UP_BASE && va >= V_KMEM_LOW_BASE){ // For kernel page
            // Allocate a physical frame for the page
            alloc_frame(page, 1, 1); // Kernel-mode, writable by default
            page->present = 1;
            page->rw = 1;   // Writable
            page->user = 0; // User non-accessible
        }else{ // For user page
            // Allocate a physical frame for the page
            alloc_frame(page, 0, 1); // User-mode, writable by default
            page->present = 1;
            page->rw = 1;   // Writable
            page->user = 1; // User accessible
        }
    }
    
    // Invalidate the TLB for this address
    asm volatile("invlpg (%0)" ::"r"(va) : "memory");
}


// Free a virtual page at the specified virtual address
void vm_free(uint64_t *ptr) {
    if (!ptr) {
        return; // Invalid pointer
    }
    
    uint64_t va = (uint64_t)ptr;

    // Get the page for the virtual address
    page_t *page = get_page(va, 0, current_pml4);
    if (!page || !page->present) {
        // Page not allocated or invalid
        return;
    }

    // Free the physical frame
    free_frame(page);

    // Clear the page table entry
    page->present = 0;
    page->rw = 0;
    page->user = 0;
    page->frame = 0;

    // Invalidate the TLB for this address
    asm volatile("invlpg (%0)" ::"r"(va) : "memory");
}

// converting physical to virtual address
uint64_t phys_to_vir(uint64_t phys){
    return phys + PHYSICAL_TO_VIRTUAL_OFFSET;
}

// converting virtual to physical address
uint64_t vir_to_phys(uint64_t va){
    return va - PHYSICAL_TO_VIRTUAL_OFFSET;
}



void test_vmm() {
    printf("Testing Virtual Memory Manager (VMM)...\n");

    // Define a virtual address to test
    uint64_t test_va = 0x100000; // 1 MB (example virtual address)

    printf("Allocating virtual page at %x\n");
    vm_alloc(test_va);

    printf("Writing to the allocated page...\n");
    uint64_t *ptr = (uint64_t *)test_va;
    *ptr = 0xDEADBEEF; // Example value to write


    printf("Verifying the write...\n");
    if (*ptr == 0xDEADBEEF) {
        printf("Write successful: %x\n", *ptr);
    } else {
        printf("Write verification failed!\n");
    }

    printf("Freeing the virtual page...\n");
    vm_free((uint64_t *)test_va);

    // Step 5: Verify that the page is no longer accessible
    printf("Verifying page is no longer accessible...\n");
    if (*ptr != 0xDEADBEEF) {
        printf("Page successfully freed and inaccessible.\n");
    } else {
        printf("Page is still accessible after being freed!\n");
    }

    printf("VMM test complete.\n");
}


