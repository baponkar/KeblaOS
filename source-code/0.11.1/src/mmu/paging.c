
#include "paging.h"

#define PAGE_SIZE 4096
#define PAGE_TABLE_ENTRIES 1024
#define KERNEL_VIRTUAL_BASE 0xC0100000 // 3GB + 1MB
#define KERNEL_PHYSICAL_BASE 0x00100000 // 1MB


// Enable paging
void enable_paging(uint32_t* page_directory) {
    asm volatile("mov %0, %%cr3" : : "r" (page_directory)); // Load page directory
    asm volatile("mov %cr0, %eax");
    asm volatile("or $0x80000000, %eax"); // Set the paging bit in CR0
    asm volatile("mov %eax, %cr0");
}



uint32_t* page_directory;    // Page directory with 1024 entries
uint32_t* first_page_table;  // Page table for identity mapping
uint32_t* higher_half_page_table; // Page table for higher half

// Allocate memory for page tables and page directory
void setup_paging() {
    // Allocate aligned memory for the page directory
    page_directory = (uint32_t*) kmalloc_a(PAGE_SIZE, 1);

    // Allocate aligned memory for the page tables
    first_page_table = (uint32_t*) kmalloc_a(PAGE_SIZE, 1);
    higher_half_page_table = (uint32_t*) kmalloc_a(PAGE_SIZE, 1);
    
    // Zero out page directory and page tables
    memset(page_directory, 0, PAGE_SIZE);
    memset(first_page_table, 0, PAGE_SIZE);
    memset(higher_half_page_table, 0, PAGE_SIZE);

    // Set up identity mapping for the first 4MB of memory (for boot)
    for (int i = 0; i < PAGE_TABLE_ENTRIES; i++) {
        first_page_table[i] = create_page_table_entry(i * PAGE_SIZE);  // Identity mapping
    }

    // Map virtual 0xC0100000 (3GB+1MB) to physical 0x00100000 (1MB)
    higher_half_page_table[0] = create_page_table_entry(KERNEL_PHYSICAL_BASE);

    // Set up page directory entries
    page_directory[0] = create_page_directory_entry(first_page_table);  // Identity map
    page_directory[KERNEL_VIRTUAL_BASE >> 22] = create_page_directory_entry(higher_half_page_table); // Higher half map

    // Enable paging
    enable_paging(page_directory);
}



#define PAGE_PRESENT 0x1    // Page is present in memory
#define PAGE_RW      0x2    // Page is writable
#define PAGE_USER    0x4    // User-mode access allowed

// Function to create a page directory entry (PDE)
uint32_t create_page_directory_entry(uint32_t* page_table) {
    return ((uint32_t)page_table & 0xFFFFF000) | PAGE_PRESENT | PAGE_RW;
}

// Function to create a page table entry (PTE)
uint32_t create_page_table_entry(uint32_t physical_address) {
    return (physical_address & 0xFFFFF000) | PAGE_PRESENT | PAGE_RW;
}


// Set page directory entry
void set_page_directory_entry(uint32_t *page_directory, uint32_t index, uint32_t *page_table) {
    page_directory[index] = ((uint32_t)page_table & 0xFFFFF000) | PAGE_PRESENT | PAGE_RW;
}

// Set page table entry
void set_page_table_entry(uint32_t *page_table, uint32_t index, uint32_t physical_address) {
    page_table[index] = (physical_address & 0xFFFFF000) | PAGE_PRESENT | PAGE_RW;
}

void page_fault_handler(registers_t * regs)
{
   // A page fault has occurred.
   // The faulting address is stored in the CR2 register.
   uint32_t faulting_address;
   asm volatile("mov %%cr2, %0" : "=r" (faulting_address));

   // The error code gives us details of what happened.
   int present   = !(regs->err_code & 0x1); // Page not present
   int rw = regs->err_code & 0x2;           // Write operation?
   int us = regs->err_code & 0x4;           // Processor was in user-mode?
   int reserved = regs->err_code & 0x8;     // Overwritten CPU-reserved bits of page entry?
   int id = regs->err_code & 0x10;          // Caused by an instruction fetch?

   // Output an error message.
   errprint("Page fault! ( ");
   if (present) {errprint("present ");}
   if (rw) {errprint("read-only ");}
   if (us) {errprint("user-mode ");}
   if (reserved) {print("reserved ");}
   print_hex(faulting_address);
   errprint(")");
   errprint("\n");
   PANIC("Page fault");
}


void test_valid_paging() {
    // Accessing the mapped virtual address
    uint32_t *test_addr = (uint32_t *)0xC0100000; // Virtual address
    *test_addr = 0xDEADBEEF; // Write to the address

    // Read back the value
    if (*test_addr == 0xDEADBEEF) {
        // Success, paging is working for this address
        print("Paging works for valid address!\n");
    } else {
        // Failure, something is wrong
        print("Paging failed for valid address!\n");
    }
}


void test_invalid_paging() {
    // Attempt to access an unmapped virtual address
    volatile uint32_t *invalid_addr = (uint32_t *)0xC0200000; // Unmapped address
    uint32_t value = *invalid_addr; // This should cause a page fault

    // If this line is reached, paging is not working properly
    print("Invalid access did not cause a page fault!\n");
}

