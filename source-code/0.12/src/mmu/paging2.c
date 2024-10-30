/*
Page Directory: Contains 1024 entries. Each entry points to a page table.
Page Table: Contains 1024 entries. Each entry points to a 4 KB page in physical memory.
CR3 Register: Holds the address of the page directory.
CR0 Register: The control register where paging is enabled by setting bit 31 (PG bit).

Lowest Physical Memory Address : 0x00000000
Highest Physical Memory Address : 0xFFFFFFFF 4GB

0x00000000 – 0x0009FFFF: Reserved for low memory, BIOS data, and interrupt vector tables.
0x000A0000 – 0x000FFFFF: Video memory and additional system structures.
0x00100000 – 0x001FFFFF: Kernel code and data (commonly used for the kernel in a 32-bit OS).
0x00200000 – Upwards: Free memory for the Page Directory and Page Tables, as well as additional memory for heap, stack, and user processes.
So, a typical setup might look like this:

Page Directory: Start at address 0x00200000 (2 MB).
Page Table 1: Start at address 0x00201000 (2 MB + 4 KB).
Page Table 2: Start at address 0x00202000 (2 MB + 8 KB).
Continue this pattern as needed for additional page tables.
*/

#include "paging2.h"


void setup_paging() {

    // Allocate page directory and a page table
    page_directory_t *page_directory = (page_directory_t *) 0x9C000; // Some free memory location
    page_table_t *page_table = (page_table_t *) 0x9D000; // Another free memory location

    // Identity map the first 4 MB of memory using one page table
    for (int i = 0; i < 1024; i++) {
        page_table->entries[i].present = 1;
        page_table->entries[i].rw = 1;  // Writable
        page_table->entries[i].user = 0; // Supervisor mode
        page_table->entries[i].address = i;  // Map the page directly (identity mapping)
    }

    // Fill page directory with the page table
    page_directory->entries[0].present = 1;
    page_directory->entries[0].rw = 1; // Writable
    page_directory->entries[0].user = 0; // Supervisor mode
    page_directory->entries[0].address = ((uint32_t) page_table) >> 12; // Address of page table, aligned

    // Load the page directory into CR3
    __asm__ volatile("mov %0, %%cr3" :: "r"(page_directory));

    // Enable paging by setting the PG bit (bit 31) in CR0
    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;  // Enable paging
    __asm__ volatile("mov %0, %%cr0" :: "r"(cr0));

    disable_interrupts();
    install_page_fault_handler();
    enable_interrupts();
    
}


// Function to check if paging is enabled
bool is_paging_enabled() {
    uint32_t cr0;
    
    // Read the value of CR0 register
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));

    // Check if the PG (paging) bit is set (bit 31)
    return (cr0 & 0x80000000) != 0;
}



// Function to test if paging is working by accessing a known mapped address
void test_paging() {
    // Check if paging is enabled
    if (is_paging_enabled()) {
        // Try to access a known mapped virtual address
        // For example, if you identity-mapped the first 4 MB of memory:
        volatile uint32_t *test_address = (uint32_t *)0x1000; // Any known mapped address
        uint32_t value = *test_address; // Try to read from it
        *test_address = value + 1;      // Try to write to it
        
        // If you reach here, the page was successfully accessed.
        print("Paging is enabled and a test address 0x1000 is accessible!\n");
    } else {
        print("Paging is not enabled.\n");
    }
}

// Function to free a page directory and its associated page tables
void free_page_directory(page_directory_t *page_directory) {
    // Loop over all the entries in the page directory
    for (int i = 0; i < 1024; i++) {
        // If a page table is present
        if (page_directory->entries[i].present) {
            // Get the address of the page table
            page_table_t *page_table = (page_table_t *)(page_directory->entries[i].address << 12);
            
            // Free the page table memory
            kfree(page_table); // Assuming you have a free() function
        }
    }
    
    // Free the page directory memory itself
    kfree(page_directory); // Assuming you have a free() function
}


void test_page_fault() {
    // Intentionally access an unmapped address to cause a page fault
    volatile uint32_t *unmapped_address = (uint32_t *) 0xDEADBEEF;
    uint32_t value = *unmapped_address;  // This should trigger a page fault
}



// The page fault handler function
void page_fault_handler(registers_t *regs) {
    // The faulting address is stored in the CR2 register
    uint32_t faulting_address;
    __asm__ volatile("mov %%cr2, %0" : "=r" (faulting_address));

    // Get the error code that tells us what caused the page fault
    uint32_t error_code = regs->err_code;

    // Determine the cause of the fault
    int present = !(error_code & 0x1); // Page not present
    int rw = error_code & 0x2;         // Write operation?
    int us = error_code & 0x4;         // Processor was in user-mode?
    int reserved = error_code & 0x8;   // Overwritten CPU-reserved bits of page entry?
    int id = error_code & 0x10;        // Instruction fetch?

    // Display the faulting address and error code for debugging
    print("Page fault at address: ");
    print_hex(faulting_address);
    print("\nError code: ");
    print_hex(error_code);
    print("\n");

    // If it's a page not present error, handle loading the page
    if (present) {
        print("Page not present in memory.\n");
    } else {
        print("Page fault due to access violation.\n");
    }

    // Perform specific actions depending on the cause of the fault
    if (rw) {
        print("Attempted write operation.\n");
    }
    if (us) {
        print("Occurred in user mode.\n");
    }
    if (reserved) {
        print("Reserved bits overwritten.\n");
    }
    if (id) {
        print("Page fault during instruction fetch.\n");
    }

    // For now, we'll halt the CPU for unhandled page faults
    print("Halting system due to page fault.\n");
    __asm__ volatile("hlt");
}

// Function to install the page fault handler into the IDT
void install_page_fault_handler() {
    interrupt_install_handler(14, &page_fault_handler);
}

// void handle_page_fault(uint32_t faulting_address) {
//     // Allocate a new page and map it
//     uint32_t new_page = find_free_page();
//     map_page(faulting_address, new_page);
// }

