#pragma once

#include "../stdlib/stdint.h"
#include "../driver/vga.h"


#define PAGE_PRESENT  0x1    // Page is present in memory
#define PAGE_WRITE    0x2    // Page is writable
#define PAGE_USER     0x4    // Page is accessible from user mode
#define PAGE_SIZE_4MB 0x80   // Page size (0 = 4 KB, 1 = 4 MB)
#define PAGE_CACHE_DISABLE 0x10  // Cache disable
#define PAGE_SIZE 4096  // 4 KB pages

// Structure for a page directory entry
typedef struct {
    uint32_t present    : 1;  // Page present in memory
    uint32_t rw         : 1;  // Read/write access (0 = read-only, 1 = read/write)
    uint32_t user       : 1;  // User-mode accessible (0 = supervisor, 1 = user)
    uint32_t write_through : 1; // Write-through caching (0 = write-back, 1 = write-through)
    uint32_t cache_disable : 1; // Cache disabled
    uint32_t accessed   : 1;  // Has the page been accessed (set by the CPU)
    uint32_t reserved   : 1;  // Reserved
    uint32_t page_size  : 1;  // Page size (0 = 4 KB, 1 = 4 MB)
    uint32_t ignored    : 1;  // Ignored by the CPU
    uint32_t available  : 3;  // Available for OS use
    uint32_t address    : 20; // Address of the page table (if 4 KB pages) or the page (if 4 MB pages)
} __attribute__((packed)) page_directory_entry_t;

// Structure for a page table entry
typedef struct {
    uint32_t present    : 1;  // Page present in memory
    uint32_t rw         : 1;  // Read/write access (0 = read-only, 1 = read/write)
    uint32_t user       : 1;  // User-mode accessible (0 = supervisor, 1 = user)
    uint32_t write_through : 1; // Write-through caching (0 = write-back, 1 = write-through)
    uint32_t cache_disable : 1; // Cache disabled
    uint32_t accessed   : 1;  // Has the page been accessed (set by the CPU)
    uint32_t dirty      : 1;  // Has the page been written to (set by the CPU)
    uint32_t reserved   : 1;  // Reserved
    uint32_t global     : 1;  // Global page (0 = not global, 1 = global page)
    uint32_t available  : 3;  // Available for OS use
    uint32_t address    : 20; // Address of the physical page
} __attribute__((packed)) page_table_entry_t;

// Structure for a page directory (1024 entries)
typedef struct {
    page_directory_entry_t entries[1024];
} page_directory_t;

// Structure for a page table (1024 entries)
typedef struct {
    page_table_entry_t entries[1024];
} page_table_t;



void setup_paging();
bool is_paging_enabled();
void test_paging();
void free_page_directory(page_directory_t *page_directory);

