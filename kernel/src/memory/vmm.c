
/*
Virtual Memory Manager (VMM) Implementation
This code is part of a simple virtual memory manager for an operating system kernel. 
It provides functions to allocate and free virtual memory pages, convert between physical 
and virtual addresses, and check the type of address (physical or virtual). The implementation 
is designed to work with a paging system and includes error handling for various scenarios.
    https://github.com/dreamportdev/Osdev-Notes/blob/master/04_Memory_Management/04_Virtual_Memory_Manager.md
*/

#include "detect_memory.h"
#include "../lib/stdio.h"
#include "paging.h"

#include "vmm.h"


// Allocate a virtual page at the specified virtual address
void vm_alloc(uint64_t va, uint8_t type) {  

    pml4_t *current_pml4 = (pml4_t *) get_cr3_addr(); // Get the current PML4 table
    
    if(current_pml4 == NULL){
        printf("[Error] VMM: current_pml4 is NULL\n");
        return;
    }

    page_t *page = get_page(va, 1, current_pml4);     // if not present then create the page

    if (!page) {
        // Handle error if page creation fails
        printf("[Error] VMM: Page creation failed!\n");
        return;
    }

    if(page->present && page->frame << 12 >= USABLE_START_PHYS_MEM){
        // Page is already present and valid
        // printf("[Error] page is present with higher physical frame address asigned by limine!\n");
    }
    
    alloc_frame(page, va >= HIGHER_HALF_START_ADDR ? 1 : 0, 1);

    switch (type) {
        case ALLOCATE_CODE:
            page->rw = 0; // Read-only for code
            page->nx = 1; // No execute for code
            break;
        case ALLOCATE_DATA:
            page->rw = 1; // Read-write for data
            page->nx = 1; // No execute for data
            break;
        case ALLOCATE_STACK:
            page->rw = 1; // Read-write for stack
            page->nx = 1; // No execute for stack
            break;
        default:
            printf("[Error] VMM: Invalid allocation type!\n");
            return;
    }


    flush_tlb(va); // Flush the TLB for the new page
}


// Free a virtual page at the specified virtual address
void vm_free(uint64_t *ptr) {

    if (!ptr) {
        printf("Inside of vm_free: invalid ptr\n");
        return; // Invalid pointer
    }
    
    uint64_t va = (uint64_t)ptr;

    pml4_t *current_pml4 = (pml4_t *) get_cr3_addr(); // Get the current PML4 table
    
    if(current_pml4 == NULL){
        printf("[Error] VMM: current_pml4 is NULL\n");
        return;
    }

    // Get the page for the virtual address
    page_t *page = get_page(va, 0, current_pml4);   // If not present do not create
    if (!page || !page->present) {
        printf("[Error] VMM: Page not present or null!\n");
        return; // Page not present or invalid
    }

    // Free the physical frame
    free_frame(page);

    // Clear the page table entry
    page->present = 0;
    page->rw = 0;
    page->user = 0;
    page->frame = 0;

    flush_tlb(va);
}

bool is_user_virt_addr(uint64_t va){
    if(va >= LOWER_HALF_START_ADDR && va < HIGHER_HALF_START_ADDR){
        return true;
    }
    return false;
}

bool is_kernel_virt_addr(uint64_t va){
    if(va >= HIGHER_HALF_START_ADDR){
        return true;
    }
    return false;
}

// converting physical to virtual address
uint64_t phys_to_vir(uint64_t pa){
    return pa + HHDM_OFFSET;
}

// converting virtual to physical address
uint64_t vir_to_phys(uint64_t va){
    // printf("[VIR_TO_PHYS] Called with virt=%p\n", (void*)va);
    if(va == 0){
        printf("[Error] VMM: Address is NULL!\n");
        return 0;
    }
    if(va <= HHDM_OFFSET){
        printf("[Error] VMM: Address is not a higher half virtual address!\n");
        return 0;
    }
    uint64_t phys = va - HHDM_OFFSET;
    // printf("[VIR_TO_PHYS] Converted %x -> %x\n", (void*)va, (void*)phys);
    return phys;
}

bool is_phys_addr(uint64_t addr){
    if(addr < HHDM_OFFSET){
        return true;
    }
    return false;
}

bool is_virt_addr(uint64_t addr){
    if(addr < HHDM_OFFSET){
        return false;
    }
    return true;
}



void test_vmm() {
    printf("Testing Virtual Memory Manager (VMM)...\n");

    // Define a virtual address to test
    uint64_t test_va = 0x100000; // 1 MB (example virtual address)

    printf("Allocating virtual page at %x\n");
    vm_alloc(test_va, ALLOCATE_DATA);

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

void test_vmm_1(){
    // Test Higher Half Memory Address
    uint64_t* high_test_addr = (uint64_t*) (HIGHER_HALF_START_ADDR + 0x400000);                     // 4 MB virtual address
    page_t* low_page = get_page((uint64_t)high_test_addr, 1, kernel_pml4);   // If not present, it will create a new page

    // Check test virtual address before writing a value
    printf("Test Address: %x, Value: %d\n", high_test_addr, *high_test_addr);
    printf("Page: present=%d, rw=%d, user=%d, frame=%x, nx=%d\n", low_page->present, low_page->rw, low_page->user, low_page->frame << 12, low_page->nx);

    // Check test virtual address after writing a value
    *high_test_addr = 12345; // Write a test value to the address
    printf("Test Address: %x, Value: %d\n", high_test_addr, *high_test_addr);
    printf("Page: present=%d, rw=%d, user=%d, frame=%x, nx=%d\n", low_page->present, low_page->rw, low_page->user, low_page->frame << 12, low_page->nx);

    // Test Lowr Half Memory Address
    // 0x200000,0x400000,0x600000,0x800000,0xA00000
    // Test virtual address 0x400000 which is start address in user_linker_x86_64.ld
    uint64_t* low_test_addr = (uint64_t*) (LOWER_HALF_START_ADDR + 0x400000);                     // 4 MB virtual address
    page_t* page = get_page((uint64_t)low_test_addr, 1, kernel_pml4);   // If not present, it will create a new page

    // Check test virtual address before writing a value
    printf("Test Address: %x, Value: %d\n", low_test_addr, *low_test_addr);
    printf("Page: present=%d, rw=%d, user=%d, frame=%x, nx=%d\n", page->present, page->rw, page->user, page->frame << 12, page->nx);

    // Check test virtual address after writing a value
    *low_test_addr = 12345; // Write a test value to the address
    printf("Test Address: %x, Value: %d\n", low_test_addr, *low_test_addr);
    printf("Page: present=%d, rw=%d, user=%d, frame=%x, nx=%d\n", page->present, page->rw, page->user, page->frame << 12, page->nx);
}
