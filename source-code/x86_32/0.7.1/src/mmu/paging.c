#include "paging.h"

void setup_paging() {
    // Allocate page directory and a page table
    page_directory_t *page_directory = (page_directory_t *)0x9C000; // For example, some free memory location
    page_table_t *page_table = (page_table_t *)0x9D000; // Another free memory location

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
    page_directory->entries[0].address = ((uint32_t)page_table) >> 12; // Address of page table, aligned

    // Load the page directory into CR3
    __asm__ volatile("mov %0, %%cr3" :: "r"(page_directory));

    // Enable paging by setting the PG bit (bit 31) in CR0
    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;  // Enable paging
    __asm__ volatile("mov %0, %%cr0" :: "r"(cr0));
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