#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "../limine/limine.h"
#include "../util/util.h"   
#include "../x86_64/idt.h"
#include  "../driver/vga.h"
#include "../limine/limine.h"
#include "pmm.h"
#include "kheap.h"
#include "../kernel/kernel.h"




#define PAGE_SIZE 4096
#define PAGE_PRESENT 0x1
#define PAGE_WRITE   0x2
#define PAGE_USER    0x4

// Function to extract parts of a virtual address
#define PML4_INDEX(va)   (((va) >> 39) & 0x1FF)  // Bits 39-47
#define PDPT_INDEX(va)   (((va) >> 30) & 0x1FF)  // Bits 30-38
#define PD_INDEX(va)     (((va) >> 21) & 0x1FF)  // Bits 21-29
#define PT_INDEX(va)     (((va) >> 12) & 0x1FF)  // Bits 12-20
#define PAGE_OFFSET(va)  ((va) & 0xFFF)          // Bits 0-11

// pml4, pdpr and pd entry
typedef struct dir_entry { // 64 bit
    uint64_t present      : 1;
    uint64_t rw           : 1;
    uint64_t user         : 1;
    uint64_t pwt          : 1;
    uint64_t pcd          : 1;
    uint64_t accessed     : 1;
    uint64_t reserved_1   : 3;  // all zeros
    uint64_t available_1  : 3;  // zero
    uint64_t base_addr    : 28; // Table base address
    uint64_t reserved_2   : 12; // Reserved must be zero
    uint64_t available_2  : 11; // zero
    uint64_t xd           : 1;
} __attribute__((packed)) dir_entry_t;

typedef struct page { // 64 bit
    uint64_t present   : 1;
    uint64_t rw        : 1;
    uint64_t user      : 1;
    uint64_t pwt       : 1;
    uint64_t pcd       : 1;
    uint64_t accessed  : 1;
    uint64_t dirty     : 1;
    uint64_t pat       : 1;
    uint64_t global    : 1;
    uint64_t ignored   : 3;
    uint64_t frame     : 40;
    uint64_t reserved  : 11;
    uint64_t nx        : 1;
} __attribute__((packed)) page_t;

typedef struct pt { // page table structure is containing 512 page entries
    page_t pages[512];
} __attribute__((aligned(PAGE_SIZE))) pt_t;

typedef struct pd { // page directory structure is containg 512 page table entries
    dir_entry_t entry_t[512]; // Each entry have Physical addresses of PTs
} __attribute__((aligned(PAGE_SIZE))) pd_t;

typedef struct pdpt { // pdpt structure is containing 512 page directory entries
    dir_entry_t entry_t[512]; // Each entry have Physical addresses of PDs
} __attribute__((aligned(PAGE_SIZE))) pdpt_t;

typedef struct pml4 { // pml4 structure is containing 512 pdpt directory entries
    dir_entry_t entry_t[512]; // Each entry have Physical addresses of PDPTs
} __attribute__((aligned(PAGE_SIZE))) pml4_t;


void alloc_frame(page_t *page, int is_kernel, int is_writeable);
void free_frame(page_t *page);
void print_cr3();
void initialise_paging();
void switch_to_page_table(pml4_t *pml4);
page_t *get_page(uint64_t address, int make, pml4_t *pml4);
void page_fault_handler(registers_t *regs);
void test_paging();

