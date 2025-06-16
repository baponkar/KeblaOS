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


#include "../memory/detect_memory.h"
#include "kmalloc.h"                // kmalloc_a, kmalloc, kfree
#include  "../lib/string.h"         // memset, memcpy, memmove
#include  "../lib/stdio.h"          // printf
#include "../lib/assert.h"

#include "pmm.h"
#include "vmm.h"

#include "paging.h"


extern void enable_paging(uint64_t pml4_address);   // present in load_paging.asm
extern void disable_paging();                       // present in load_paging.asm

extern volatile uint64_t phys_mem_head;             // Physical memory head pointer, initialized in detect_memory.c
pml4_t *kernel_pml4;
uint64_t bsp_cr3;

// allocate a page with the free physical frame
void alloc_frame(page_t *page, int user, int is_writeable) {
    
    // idx is now the index of the first free frame.
    uint64_t bit_no = free_frame_bit_no(); 

    if (bit_no == (uint64_t)-1) {
        printf("[Error] Paging: No free frames!");
        halt_kernel();
    }

    set_frame(bit_no); // Mark the frame as used by passing the frame index

    page->present = 1;                      // Mark it as present.
    page->rw = is_writeable;                // Should the page be writeable?
    page->user = user;                      // Should the page be user-mode?
    phys_mem_head &= 0xFFFFFFFFFFFFF000;    // Align the physical memory head to 4KB boundary
    page->frame = (uint64_t) (phys_mem_head + (bit_no * FRAME_SIZE)) >> 12;     // Store physical base address

    phys_mem_head += FRAME_SIZE;            // Move the physical memory head to the next frame
}



// Function to deallocate a frame.
void free_frame(page_t *page)
{
    if (page->frame != NULL)
    {
        uint64_t frame = (uint64_t) page->frame << 12;  // Get the physical address of the frame

        uint64_t bit_no = PHYS_ADDR_TO_BIT_NO(frame);   // Convert the physical frame address into a bit number
        
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
void init_bs_paging()
{  
    printf(" [-] Initializing Paging for Bootstrap CPU Core\n");
    assert(phys_mem_head != 0); // Check if physical memory head is initialized

    bsp_cr3 = get_cr3_addr();   // Get the current value of CR3 (the base of the PML4 table)

    // Paging is enabled by Limine. Get the pml4 table pointer address that Limine set up
    kernel_pml4 = (pml4_t *) phys_to_vir((uint64_t)get_cr3_addr());

    if (!kernel_pml4) {
        printf("[Error] Paging: Kernel PML4 is NULL\n");
        halt_kernel();          // Halt the kernel if PML4 is not set
    }

    // Updating lower half pages first 10 MB
    for (uint64_t addr = 0x0; addr < 0x00A00000; addr += PAGE_SIZE) {
        page_t *page = get_page(addr, 1, kernel_pml4); // If not present, it will create a new page
        if (!page) {
            printf("[Error] Failed to get page entry for address: %x\n", addr);
            continue;
        }

        // If the page is not present and frame address is null, allocate a frame for it
        // if(!page->present || !page->frame) {
        //     alloc_frame(page, 0, 1); // page, user, rw
        // }

        alloc_frame(page, 1, 1); // page, user, rw

        // printf("[Debug] Page at address %x: present=%d, rw=%d, user=%d, frame=%x\n", 
            // addr, page->present, page->rw, page->user, page->frame << 12);
    }

    // Invalidate the TLB for the changes to take effect
    flush_tlb_all();
    
    printf(" [-] Successfully Paging initialized.\n");
}


// Initializing Paging for other CPU cores
void init_ap_paging(int core_id) {
    printf(" [-] Initializing Paging for CPU %d\n", core_id);

    set_cr3_addr(bsp_cr3);  // Set the CR3 register to the PML4 address
    printf(" [-] CPU %d: Set CR3 to PML4 address: %x\n", core_id, bsp_cr3);

    pml4_t * pml4 = (pml4_t *) phys_to_vir((uint64_t)get_cr3_addr()); // Get the current value of CR3 (the base of the PML4 table)

    // Updating lower half pages first 10 MB
    for (uint64_t addr = 0x0; addr < 0x00A00000; addr += PAGE_SIZE) {
        page_t *page = get_page(addr, 1, pml4);
        if (!page) {
            printf("[Error] Failed to get page entry for address: %x\n", addr);
            continue;
        }

        if(!page->present || !page->frame) {
            alloc_frame(page, 1, 1); // page, is_kernel, rw
        }
    }

    flush_tlb_all();        // Flush TLB for the current core

    printf(" [-] Enabling Paging first 1 MB Lower Half Memory address for core %d\n", core_id);
    printf(" [-] Successfully Paging initialized for core %d.\n", core_id);
}

// Function to allocate a new page
static page_t *alloc_page(){
    page_t *pg = (page_t *) kmalloc_a(sizeof(page_t), 1);
        
    if(!pg){
        printf("[Error] Paging: Failed to allocate page\n");
        return NULL;            // Allocation failed
    }
    memset(pg, 0, sizeof(pg));  // Zero out the page structure
    return pg;
}

// Function to allocate a new page table
static pt_t* alloc_pt() {
    pt_t* pt = (pt_t*)kmalloc_a(sizeof(pt_t), 1);
    if(!pt) {
        printf("[Error] Paging: Failed to allocate PT\n");
        return NULL;            // Allocation failed
    }
    memset(pt, 0, sizeof(pt_t)); // Zero out the page table
    return pt;
}

// Function to allocate a new page directory
static pd_t* alloc_pd() {
    pd_t* pd = (pd_t*)kmalloc_a(sizeof(pd_t), 1);
    if (!pd){
        printf("[Error] Paging: Failed to allocate PD\n");
        return NULL;            // Allocation failed
    }
    memset(pd, 0, sizeof(pd_t)); // Zero out the page directory
    return pd;
}

// Function to allocate a new page directory pointer table
static pdpt_t* alloc_pdpt() {
    pdpt_t* pdpt = (pdpt_t*)kmalloc_a(sizeof(pdpt_t), 1);

    if (!pdpt) {
        printf("[Error] Paging: Failed to allocate PDPT\n");
        return NULL;                    // Allocation failed
    }
    memset(pdpt, 0, sizeof(pdpt_t));    // Zero out the PDPT
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

    int user = (va >= HIGHER_HALF_START_ADDR) ? 0 : 1; // User mode if the address is in lower half

    dir_entry_t* pml4_entry = (dir_entry_t*) ((uint64_t) &pml4->entries[pml4_index]);

    if (!pml4_entry->present) {
        if (!make) return NULL;
        pdpt_t* pdpt = alloc_pdpt(); 
        if (!pdpt) return NULL;
        pml4_entry->present = 1;
        pml4_entry->rw = 1;
        pml4_entry->user = user;;
        pml4_entry->base_addr = (uint64_t)pdpt >> 12;
    }

    pdpt_t* pdpt = (pdpt_t*)(pml4_entry->base_addr << 12);
    dir_entry_t* pdpt_entry = (dir_entry_t*)phys_to_vir((uint64_t) &pdpt->entries[pdpt_index]);

    if (!pdpt_entry->present) {
        if (!make) return NULL;
        pd_t* pd = alloc_pd();
        if (!pd) return NULL;
        pdpt_entry->present = 1;
        pdpt_entry->rw = 1;
        pdpt_entry->user = user;
        pdpt_entry->base_addr = (uint64_t)pd >> 12;
    }

    pd_t* pd = (pd_t*)(pdpt_entry->base_addr << 12);
    dir_entry_t* pd_entry = (dir_entry_t*) phys_to_vir((uint64_t)&pd->entries[pd_index]);

    if (!pd_entry->present) {
        if (!make) return NULL;
        pt_t* pt = alloc_pt();
        if (!pt) return NULL;
        pd_entry->present = 1;
        pd_entry->rw = 1;
        pd_entry->user = user;
        pd_entry->base_addr = (uint64_t)pt >> 12;
    }

    pt_t* pt = (pt_t*)(pd_entry->base_addr << 12);
    page_t* page = (page_t *)phys_to_vir((uint64_t)&pt->pages[pt_index]);

    if (!page->present) {
        alloc_frame(page, user, 1);            // kernel space, read-write
        if (!page->frame) return NULL;
        page->user = user;
    }

    flush_tlb(va);
    
    return page;
}


// Function to flush TLB for a specific address
void flush_tlb(uint64_t va) {
    // page_t *page = get_page(va, 0, (pml4_t *)get_cr3_addr());
    // if(!page) {
    //     printf("[Error] Paging: flush_tlb: Page not found for address %x\n", va);
    //     return;
    // }
    // // Use the invlpg instruction to invalidate the TLB entry for a specific address
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
    uint64_t virt = phys_to_vir(phys);   // HHDM mapping

    // Assume we already have the base PML4 loaded in CR3
    pml4 = (uint64_t *)get_cr3_addr();  

    // Iterate through the address range to map
    for (size_t offset = 0; offset < size; offset += PAGE_SIZE) {
        uint64_t current_phys = phys + offset;
        uint64_t current_virt = virt + offset;

        // Calculate indices in the paging hierarchy
        pml4_index = PML4_INDEX(current_virt);
        pdpt_index = PDPT_INDEX(current_virt);
        pd_index   = PD_INDEX(current_virt);
        pt_index   = PT_INDEX(current_virt);

        // Get or create PDPT
        if (!(pml4[pml4_index] & PAGE_PRESENT)) {
            pdpt = (uint64_t *)kmalloc_a(PAGE_SIZE, 1);
            memset(pdpt, 0, PAGE_SIZE);
            pml4[pml4_index] = ((uint64_t)vir_to_phys((uint64_t)pdpt) | flags);
        } else {
            pdpt = (uint64_t *)phys_to_vir((uint64_t)pml4[pml4_index] & ~0xFFF);
        }

        // Get or create PD
        if (!(pdpt[pdpt_index] & PAGE_PRESENT)) {
            pd = (uint64_t *)kmalloc_a(PAGE_SIZE, 1);
            memset(pd, 0, PAGE_SIZE);
            pdpt[pdpt_index] = ((uint64_t)vir_to_phys((uint64_t)pd) | flags);
        } else {
            pd = (uint64_t *)phys_to_vir((uint64_t)pdpt[pdpt_index] & ~0xFFF);
        }

        // Get or create PT
        if (!(pd[pd_index] & PAGE_PRESENT)) {
            pt = (uint64_t *)kmalloc_a(PAGE_SIZE, 1);
            memset(pt, 0, PAGE_SIZE);
            pd[pd_index] = ((uint64_t)vir_to_phys((uint64_t)pt) | flags);
        } else {
            pt = (uint64_t *)phys_to_vir((uint64_t)pd[pd_index] & ~0xFFF);
        }

        // Final mapping of the page
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





