#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "../util/util.h"
#include "../idt/idt.h"

#include "../../limine/limine.h"

#include "../stdlib/stdio.h"

#include  "../driver/vga/vga.h"

#include "pmm.h"
#include "kheap.h"
#include "../kernel/kernel.h"


// Page table entry for x86_64
typedef struct page
{
    uint64_t present : 1;   // Page present in memory
    uint64_t rw : 1;        // Read/write permission
    uint64_t user : 1;      // User/supervisor mode
    uint64_t pwt : 1;       // Page-level write-through
    uint64_t pcd : 1;       // Page-level cache disable
    uint64_t accessed : 1;  // Accessed flag
    uint64_t dirty : 1;     // Dirty flag
    uint64_t pat : 1;       // Page Attribute Table
    uint64_t global : 1;    // Global page
    uint64_t ignored : 3;   // Ignored bits
    uint64_t frame : 40;    // Frame address (aligned to 4 KB)
    uint64_t reserved : 11; // Reserved bits
    uint64_t nx : 1;        // No-execute flag
} __attribute__((packed)) page_t;

// A page table (512 entries)
typedef struct page_table
{
    page_t pages[512];
} page_table_t;

// A page directory (higher-level table like PML4, PDPT, etc.)
typedef struct page_directory
{
    page_table_t *tables[512]; // Pointers to lower-level tables
    uint64_t physicalAddr;     // Physical address of this directory
} page_directory_t;

/**
  Sets up the environment, page directories, etc., and enables paging.
**/
void initialise_paging();

/**
  Causes the specified page directory to be loaded into the
  CR3 register.
**/
void switch_page_directory(page_directory_t *new);

/**
  Retrieves a pointer to the page required.
  If make == 1, if the page-table in which this page should
  reside isn't created, create it!
**/
page_t *get_page(uint64_t address, int make, page_directory_t *dir);

/**
  Handler for page faults.
**/
void page_fault_handler(registers_t *regs);

void test_paging();


void alloc_frame(page_t *page, int is_kernel, int is_writeable);
void free_frame(page_t *page);

