
#pragma once

#include <stdint.h>

#include "../util/util.h"


#define PAGE_SIZE    4096
#define PAGE_PRESENT 0x1
#define PAGE_WRITE   0x2
#define PAGE_USER    0x4

// Function to extract parts of a virtual address
#define PML4_INDEX(va)   (((va) >> 39) & 0x1FF)  // Bits 39-47
#define PDPT_INDEX(va)   (((va) >> 30) & 0x1FF)  // Bits 30-38
#define PD_INDEX(va)     (((va) >> 21) & 0x1FF)  // Bits 21-29
#define PT_INDEX(va)     (((va) >> 12) & 0x1FF)  // Bits 12-20
#define PAGE_OFFSET(va)  ((va) & 0xFFF)          // Bits 0-11

#define PAGE_ALIGN(addr) (((addr) + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1))

// pml4, pdpr and pd entry
struct dir_entry { // 64 bit
    uint64_t present      : 1;  // always 1
    uint64_t rw           : 1;  // 0 for read-only, 1 for read-write
    uint64_t user         : 1;  // 0 for kernel, 1 for user
    uint64_t pwt          : 1;  
    uint64_t pcd          : 1;
    uint64_t accessed     : 1;
    uint64_t reserved_1   : 3;  // all zeros
    uint64_t available_1  : 3;  // zero
    uint64_t base_addr    : 40; // Table base address
    // uint64_t reserved_2   : 12; // Reserved must be zero
    uint64_t available_2  : 11; // zero
    uint64_t xd           : 1;
} __attribute__((packed));
typedef struct dir_entry dir_entry_t;


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


extern pml4_t *current_pml4;

extern uint64_t V_KMEM_UP_BASE;
extern uint64_t V_KMEM_LOW_BASE;

void debug_page(page_t *page);
void alloc_frame(page_t *page, int is_kernel, int is_writeable);
void free_frame(page_t *page);
uint64_t get_cr3_addr();
void init_paging();
static page_t *alloc_page();
static pt_t* alloc_pt();
static pd_t* alloc_pd();
static pdpt_t* alloc_pdpt();
page_t* get_page(uint64_t va, int make, pml4_t* pml4);
void test_paging();
void flush_tlb(uint64_t address);
void flush_tlb_all();
void map_virtual_memory(void *phys_addr, size_t size, uint64_t flags);
uint64_t create_new_pml4();
bool is_user_page(uint64_t virtual_address);

