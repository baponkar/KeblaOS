
/*
https://github.com/dreamportdev/Osdev-Notes/blob/master/04_Memory_Management/04_Virtual_Memory_Manager.md
*/

#include "paging.h"
#include "../driver/vga.h"

#include "vmm.h"



extern pml4_t *user_pml4;
extern pml4_t *kernel_pml4;
extern pml4_t *current_pml4;

extern uint64_t V_KMEM_LOW_BASE;
extern uint64_t V_KMEM_UP_BASE;



// Allocate a virtual page at the specified virtual address
void vm_alloc(uint64_t va) {
    // print("inside of vm_alloc\n");
    // Get the page for the virtual address, creating necessary structures
    page_t *page = get_page(va, 1, current_pml4);
    if (!page) {
        // Handle error if page creation fails
        print("Page creation failed!\n");
        return;
    }

    // print("page->present : ");
    // print_dec(page->present);
    // print("\n");
  
    if (page->present){
        // Allocate a physical frame for the page
        alloc_frame(page, 1, 1); // Kernel-mode, writable by default
        page->present = 1;
        page->rw = 1; // Writable
        page->user = 1; // User-accessible (optional, depends on use case)
    }
    
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


void test_vmm() {
    print("Testing Virtual Memory Manager (VMM)...\n");

    // Define a virtual address to test
    uint64_t test_va = 0x100000; // 1 MB (example virtual address)

    // Step 1: Allocate a virtual page
    print("Allocating virtual page at ");
    print_hex(test_va);
    print("\n");
    vm_alloc(test_va);

    // Step 2: Write to the allocated page
    print("Writing to the allocated page...\n");
    uint64_t *ptr = (uint64_t *)test_va;
    *ptr = 0xDEADBEEF; // Example value to write

    // Step 3: Verify the write
    print("Verifying the write...\n");
    if (*ptr == 0xDEADBEEF) {
        print("Write successful: ");
        print_hex(*ptr);
        print("\n");
    } else {
        print("Write verification failed!\n");
    }

    // Step 4: Free the virtual page
    print("Freeing the virtual page...\n");
    vm_free((uint64_t *)test_va);

    // Step 5: Verify that the page is no longer accessible
    print("Verifying page is no longer accessible...\n");
    if (*ptr != 0xDEADBEEF) {
        print("Page successfully freed and inaccessible.\n");
    } else {
        print("Page is still accessible after being freed!\n");
    }

    print("VMM test complete.\n");
}
