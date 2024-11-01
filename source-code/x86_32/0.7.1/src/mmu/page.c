/*
Page Directory: Contains 1024 entries. Each entry points to a page table.
Page Table: Contains 1024 entries. Each entry points to a 4 KB page in physical memory.
CR3 Register: Holds the address of the page directory.
CR0 Register: The control register where paging is enabled by setting bit 31 (PG bit).

Lowest Physical Memory Address : 0x00000000
Highest Physical Memory Address : 0xFFFFFFFF
*/

#include "page.h"

#define PAGE_SIZE 4096  // 4 KB pages
#define PAGE_PRESENT 0x1
#define PAGE_WRITE   0x2


// Page Directory (4 KB aligned, 1024 entries)
uint32_t page_directory[1024] __attribute__((aligned(PAGE_SIZE)));

// Page Table (4 KB aligned, 1024 entries)
uint32_t page_table[1024] __attribute__((aligned(PAGE_SIZE)));


// Function to set up paging
void setup_paging() {
    // Fill the page directory with the address of the page tables
    for (int i = 0; i < 1024; i++) {
        // Each entry in the page directory points to a page table.
        // We are mapping the first 4 MB of physical memory here.
        page_directory[i] = ((uint32_t) page_table) | PAGE_PRESENT | PAGE_WRITE;
    }

    // Identity map the first 4 MB of memory using the first page table
    for (int i = 0; i < 1024; i++) {
        // Map each 4 KB page in the page table
        page_table[i] = (i * PAGE_SIZE) | PAGE_PRESENT | PAGE_WRITE;
    }

    // Load the page directory into CR3 (Page Directory Base Register)
    __asm__ volatile("mov %0, %%cr3" :: "r"(page_directory));

    // Enable paging by setting the PG (paging) bit in CR0
    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));  // Read CR0
    cr0 |= 0x80000000;  // Set the paging bit (bit 31) in CR0
    __asm__ volatile("mov %0, %%cr0" :: "r"(cr0));  // Write back to CR0
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
        print("Paging is enabled and address 0x1000 is accessible!\n");
    } else {
        print("Paging is not enabled.\n");
    }
}

