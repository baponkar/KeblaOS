/*
Paging in x86_64

Limine initially started 4 level paging and place kernel into higher half.
But limine started all pages as kernel space but I use init_paging() to
change lower half address's pages kernel space.

https://wiki.osdev.org/Paging
https://wiki.osdev.org/Identity_Paging
https://web.archive.org/web/20160326061042/http://jamesmolloy.co.uk/tutorial_html/6.-Paging.html
https://github.com/dreamportdev/Osdev-Notes/blob/master/04_Memory_Management/03_Paging.md
https://stackoverflow.com/questions/18431261/how-does-x86-paging-work

*/ 

#include "../arch/interrupt/pic/pic.h"
#include "../memory/detect_memory.h"
#include "kmalloc.h"
#include  "../lib/string.h"
#include  "../lib/stdio.h"
#include "../lib/assert.h"
#include "../sys/timer/tsc.h"
#include "pmm.h"
#include "vmm.h"

#include "paging.h"


extern void enable_paging(uint64_t pml4_address);   // present in load_paging.asm
extern void disable_paging();                       //  present in load_paging.asm

extern volatile uint64_t phys_mem_head;             // Physical memory head pointer, initialized in detect_memory.c

pml4_t *kernel_pml4;


uint64_t bsp_cr3;

// allocate a page with the free physical frame
void alloc_frame(page_t *page, int is_kernel, int is_writeable) {
    
    // idx is now the index of the first free frame.
    uint64_t bit_no = free_frame_bit_no(); 

    if (bit_no == (uint64_t)-1) {
        printf("[Error] Paging: No free frames!");
        halt_kernel();
    }

    set_frame(bit_no); // Mark the frame as used by passing the frame index

    page->present = 1;                      // Mark it as present.
    page->rw = (is_writeable) ? 1 : 0;      // Should the page be writeable?
    page->user = (is_kernel) ? 0 : 1;       // Should the page be user-mode?
    phys_mem_head &= 0xFFFFFFFFFFFFF000;    // Increment the usable memory pointer
    page->frame = (uint64_t) (phys_mem_head + (bit_no * FRAME_SIZE)) >> 12;     // Store physical base address

    phys_mem_head += FRAME_SIZE;
}



// Function to deallocate a frame.
void free_frame(page_t *page)
{
    if (page->frame != NULL)
    {
        uint64_t frame = (uint64_t) page->frame << 12;  // Get the physical address of the frame

        uint64_t bit_no = PHYS_ADDR_TO_BIT_NO(frame);   // Convert the frame address to a bit number
        
        clear_frame(bit_no);                            // Frame is now free again from bitmap.

        page->frame = 0;                                // Page now doesn't have a frame.
    }
}


// return current cr3 address i.e. root pml4 pointer address
uint64_t get_cr3_addr() {
    uint64_t cr3;
    asm volatile("mov %%cr3, %0" : "=r"(cr3)); // Read the CR3 register

    return cr3;
}

void set_cr3_addr(uint64_t cr3) {
    if (cr3 == 0) {
        printf("[Error] CR3 address is NULL\n");
        return;
    }
    // Set the CR3 register to the new PML4 address
    asm volatile("mov %0, %%cr3" : : "r"(cr3)); // Write the CR3 register
}

// Initialising Paging for bootstrap CPU core
void init_paging()
{  
    assert(phys_mem_head != 0); // Check if physical memory head is initialized

    bsp_cr3 = get_cr3_addr(); // Get the current value of CR3 (the base of the PML4 table)

    // Paging is enabled by Limine. Get the pml4 table pointer address that Limine set up
    kernel_pml4 = (pml4_t *) get_cr3_addr();
    if (!kernel_pml4) {
        printf("[Error] Paging: Kernel PML4 is NULL\n");
        halt_kernel(); // Halt the kernel if PML4 is not set
    }

    // Updating lower half pages first 10 MB
    for (uint64_t addr = 0x0; addr < 0x00A00000; addr += PAGE_SIZE) {
        page_t *page = get_page(addr, 1, kernel_pml4); // If not present, it will create a new page
        if (!page) {
            printf("[Error] Failed to get page entry for address: %x\n", addr);
            continue;
        }

        // Allocate a frame if not already allocated
        if (!page->frame) {
            alloc_frame(page, 0, 1); // page, is_kernel, rw
        }

        // Set the User flag (0x4 in x86_64) to allow user-level access
        page->present = 1;  // Ensure the page is present
        page->rw = 1;       // Allow read/write access
        page->user = 1;     // Set user-accessible bit
    }

    // Invalidate the TLB for the changes to take effect
    flush_tlb_all();
    
    printf(" [-] Successfully Paging initialized.\n");
}

// Initializing Paging for other CPU cores
void init_core_paging(int core_id) {

    set_cr3_addr(bsp_cr3);  // Set the CR3 register to the PML4 address
    printf(" [-] CPU %d: Set CR3 to PML4 address: %x\n", core_id, bsp_cr3);

    pml4_t * pml4 = (pml4_t *) get_cr3_addr(); // Get the current value of CR3 (the base of the PML4 table)

    // Updating lower half pages first 10 MB
    for (uint64_t addr = 0x0; addr < 0x00A00000; addr += PAGE_SIZE) {
        page_t *page = get_page(addr, 1, pml4);
        if (!page) {
            // Handle error: Failed to get the page entry
            printf("[Error] Failed to get page entry for address: %x\n", addr);
            continue;
        }

        // Allocate a frame if not already allocated
        if (!page->frame) {
            alloc_frame(page, 0, 1); // page, is_kernel, rw
        }

        // Set the User flag (0x4 in x86_64) to allow user-level access
        page->present = 1;  // Ensure the page is present
        page->rw = 1;       // Allow read/write access
        page->user = 1;     // Set user-accessible bit

    }

    flush_tlb_all();        // Flush TLB for the current core

    printf(" [-] Enabling Paging first 1 MB Lower Half Memory address for core %d\n", core_id);
    printf(" [-] Successfully Paging initialized for core %d.\n", core_id);
    
}


static page_t *alloc_page(){
    page_t *pg = (page_t *) kmalloc_a(sizeof(page_t), 1);
    if(pg){
        memset(pg, 0, sizeof(pg));
    }else{
        printf("[Error] Paging: Failed to allocate page\n");
        return NULL; // Allocation failed
    }
    return pg;
}

// Function to allocate a new page table
static pt_t* alloc_pt() {
    pt_t* pt = (pt_t*)kmalloc_a(sizeof(pt_t), 1);
    if (pt) {
        memset(pt, 0, sizeof(pt_t)); // Zero out the page table
    }else {
        printf("[Error] Paging: Failed to allocate PT\n");
        return NULL; // Allocation failed
    }
    return pt;
}

// Function to allocate a new page directory
static pd_t* alloc_pd() {
    pd_t* pd = (pd_t*)kmalloc_a(sizeof(pd_t), 1);
    if (!pd){
        printf("[Error] Paging: Failed to allocate PD\n");
        return NULL; // Allocation failed
    }
    memset(pd, 0, sizeof(pd_t));    // Zero out the page directory
    return pd;
}

// Function to allocate a new page directory pointer table
static pdpt_t* alloc_pdpt() {
    pdpt_t* pdpt = (pdpt_t*)kmalloc_a(sizeof(pdpt_t), 1);

    if (!pdpt) {
        printf("[Error] Paging: Failed to allocate PDPT\n");
        return NULL; // Allocation failed
    }
    memset(pdpt, 0, sizeof(pdpt_t)); // Zero out the PDPT
    return pdpt;
}


page_t* get_page(uint64_t va, int make, pml4_t* pml4) {

    if (!pml4) {
        printf("[Error] Paging: get_page: pml4 is NULL\n");
        return NULL;
    }

    uint64_t pml4_index = PML4_INDEX(va);
    uint64_t pdpt_index = PDPT_INDEX(va);
    uint64_t pd_index   = PD_INDEX(va);
    uint64_t pt_index   = PT_INDEX(va);

    dir_entry_t* pml4_entry = &pml4->entry_t[pml4_index];

    if (!pml4_entry->present) {
        if (!make) return NULL;
        pdpt_t* pdpt = alloc_pdpt();
        if (!pdpt) return NULL;
        pml4_entry->present = 1;
        pml4_entry->rw = 1;
        pml4_entry->user = (va >= HIGHER_HALF_START_ADDR) ? 0 : 1;
        pml4_entry->base_addr = (uint64_t)pdpt >> 12;
    }

    pdpt_t* pdpt = (pdpt_t*)phys_to_vir(pml4_entry->base_addr << 12);
    dir_entry_t* pdpt_entry = &pdpt->entry_t[pdpt_index];

    if (!pdpt_entry->present) {
        if (!make) return NULL;
        pd_t* pd = alloc_pd();
        if (!pd) return NULL;
        pdpt_entry->present = 1;
        pdpt_entry->rw = 1;
        pdpt_entry->base_addr = (uint64_t)pd >> 12;
    }

    pd_t* pd = (pd_t*)phys_to_vir(pdpt_entry->base_addr << 12);
    dir_entry_t* pd_entry = &pd->entry_t[pd_index];

    if (!pd_entry->present) {
        if (!make) return NULL;
        pt_t* pt = alloc_pt();
        if (!pt) return NULL;
        pd_entry->present = 1;
        pd_entry->rw = 1;
        pd_entry->base_addr = (uint64_t)pt >> 12;
    }

    pt_t* pt = (pt_t*)phys_to_vir(pd_entry->base_addr << 12);
    page_t* page = &pt->pages[pt_index];

    if (!page->present) {
        page->present = 1;
        page->rw = 1;
        alloc_frame(page, 0, 1);
        if (!page->frame) return NULL;
        page->user = (va >= HIGHER_HALF_START_ADDR) ? 0 : 1;
    }

    flush_tlb(va);
    return page;
}




// Function to flush TLB for a specific address
void flush_tlb(uint64_t va) {
    // page_t *page = get_page(va, 0, (pml4_t *)get_cr3_addr());
    // Use the invlpg instruction to invalidate the TLB entry for a specific address
    // if(page->present) asm volatile("invlpg (%0)" : : "r"(va) : "memory");
    asm volatile("invlpg (%0)" : : "r"(va) : "memory");
}

// Function to flush the entire TLB (by writing to cr3)
void flush_tlb_all() {
    uint64_t cr3;
    // Get the current value of CR3 (the base of the PML4 table)
    asm volatile("mov %%cr3, %0" : "=r"(cr3));

    // Write the value of CR3 back to itself, which will flush the TLB
    asm volatile("mov %0, %%cr3" : : "r"(cr3) : "memory");
}

void map_virtual_memory(void *phys_addr, size_t size, uint64_t flags) {
    uint64_t pml4_index, pdpt_index, pd_index, pt_index;
    uint64_t *pml4, *pdpt, *pd, *pt;

    // Cast the physical address to a usable form
    uint64_t phys = (uint64_t)phys_addr;
    uint64_t virt = phys_to_vir(phys);  // Mapping physical to virtual 1:1 for simplicity.

    // Assume we already have the base PML4 loaded in CR3
    pml4 = (uint64_t *)get_cr3_addr();  

    // Iterate through the address range to map
    for (size_t offset = 0; offset < size; offset += 0x1000) {
        uint64_t current_phys = phys + offset;
        uint64_t current_virt = virt + offset;

        // Calculate indices in the paging hierarchy
        pml4_index = PML4_INDEX(current_virt);
        pdpt_index = PDPT_INDEX(current_virt);
        pd_index   = PD_INDEX(current_virt);
        pt_index   = PT_INDEX(current_virt);

        // Get or create PDPT
        if (!(pml4[pml4_index] & PAGE_PRESENT)) {
            pdpt = (uint64_t *)kmalloc_a(0x1000, 1);
            memset(pdpt, 0, 0x1000);
            pml4[pml4_index] = ((uint64_t)pdpt | flags);
        } else {
            pdpt = (uint64_t *)(pml4[pml4_index] & ~0xFFF);
        }

        // Get or create PD
        if (!(pdpt[pdpt_index] & PAGE_PRESENT)) {
            pd = (uint64_t *)kmalloc_a(0x1000,1);
            memset(pd, 0, 0x1000);
            pdpt[pdpt_index] = ((uint64_t)pd | flags);
        } else {
            pd = (uint64_t *)(pdpt[pdpt_index] & ~0xFFF);
        }

        // Get or create PT
        if (!(pd[pd_index] & PAGE_PRESENT)) {
            pt = (uint64_t *)kmalloc_a(0x1000,1);
            memset(pt, 0, 0x1000);
            pd[pd_index] = ((uint64_t)pt | flags);
        } else {
            pt = (uint64_t *)(pd[pd_index] & ~0xFFF);
        }

        // Map the physical address to the virtual address
        pt[pt_index] = (current_phys | flags);
    }

    // Ensure changes to page tables are reflected in the CPU
    flush_tlb(virt);
}



uint64_t create_new_pml4() {
    uint64_t pml4_ptr_phys = (uint64_t) kmalloc_a(sizeof(pml4_t), 1);
    memset((void*)pml4_ptr_phys, 0, sizeof(pml4_t)); // Clear PML4 table

    // Map the PML4 into the page tables
    map_virtual_memory((void*)pml4_ptr_phys, sizeof(pml4_t), PAGE_WRITE | PAGE_PRESENT);
    return pml4_ptr_phys;
}

bool is_user_page(uint64_t virtual_address) {
    uint64_t cr3 = get_cr3_addr(); // Get PML4 base address

    uint64_t *pml4 = (uint64_t *)(cr3 & ~0xFFF); // Mask to get page-aligned base
    uint64_t pml4e = pml4[PML4_INDEX(virtual_address)];
    if (!(pml4e & 1)) return false; // Not present

    uint64_t *pdpt = (uint64_t *)(pml4e & ~0xFFF);
    uint64_t pdpte = pdpt[PDPT_INDEX(virtual_address)];
    if (!(pdpte & 1)) return false;

    uint64_t *pd = (uint64_t *)(pdpte & ~0xFFF);
    uint64_t pde = pd[PD_INDEX(virtual_address)];
    if (!(pde & 1)) return false;

    uint64_t *pt = (uint64_t *)(pde & ~0xFFF);
    uint64_t pte = pt[PT_INDEX(virtual_address)];
    if (!(pte & 1)) return false;

    // Check the user bit (bit 2)
    return (pte & (1 << 2)) != 0;
}




