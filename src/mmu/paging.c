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

#include "../x86_64/interrupt/pic.h"
#include "../mmu/detect_memory.h"
#include "kmalloc.h"
#include  "../lib/string.h"
#include  "../lib/stdio.h"
#include "pmm.h"

#include "paging.h"


#define LOW_HALF_START 0x0000000000000000ULL
#define LOW_HALF_END   0x7FFFFFFFFFFFFFFFULL
// #define LOW_HALF_END    0x0000000000800000  // For testing purpose

#define UPPER_HALF_START 0xFFFF800000000000
#define UPPER_HALF_END   0xFFFFFFFFFFFFFFFF

extern void enable_paging(uint64_t pml4_address); // present in load_paging.asm
extern void disable_paging();   //  present in load_paging.asm

extern uint64_t KMEM_UP_BASE;
extern uint64_t KMEM_LOW_BASE;

extern uint64_t V_KMEM_UP_BASE;
extern uint64_t V_KMEM_LOW_BASE;

pml4_t *current_pml4;


void debug_page(page_t *page){
    printf("page pointer: %x\n", (uint64_t)page);
    printf("page->present: %d\n", page->present);
    printf("page->rw: %d\n", page->rw);
    printf("page->pwt: %x\n", page->pwt);
    printf("page->pcd: %x\n", page->pcd);
    printf("page->accessed: %x\n", page->accessed);
    printf("page->user: %d\n", page->user);
    printf("page->frame: %x\n", page->frame);
}


// allocate a page with the free physical frame
void alloc_frame(page_t *page, int is_kernel, int is_writeable) {
    // print("inside of alloc frame\n");
    if (page->frame != 0) {
        // print("Page was already allocated at ");
        // print_hex((uint64_t)page->frame);
        // print(" frame address.\n");
        return; // Frame was already allocated, return straight away.
    }

    uint64_t bit_no = free_frame_bit_no(); // idx is now the index of the first free frame.

    if (bit_no == (uint64_t)-1) {
        printf("No free frames!");
        halt_kernel();
    }

    set_frame(bit_no); // Mark the frame as used by passing the frame index

    page->present = 1; // Mark it as present.
    page->rw = (is_writeable) ? 1 : 0;  // Should the page be writeable?
    page->user = (is_kernel) ? 0 : 1;   // Should the page be user-mode?
    page->frame = (is_kernel) ? (KMEM_LOW_BASE + (bit_no * FRAME_SIZE)) >> 12 : (UMEM_LOW_BASE + (bit_no * FRAME_SIZE)) >> 12; // Store physical base address
    is_kernel ? (KMEM_LOW_BASE += FRAME_SIZE) : (UMEM_LOW_BASE += FRAME_SIZE);

}


// Function to deallocate a frame.
void free_frame(page_t *page)
{
    if (page->frame != NULL)
    {
        // print("Frame address: ");
        // print_hex((uint64_t)page->frame);
        // print("\n");

        uint64_t frame_idx = ADDR_TO_BIT_NO((uint64_t)page->frame);
        clear_frame(frame_idx); // Frame is now free again from bitmap.
        page->frame = 0; // Page now doesn't have a frame.
    }

    // print("No frame address found!\n");

    return;
}


// return current cr3 address i.e. root pml4 pointer address
uint64_t get_cr3_addr() {
    uint64_t cr3;
    asm volatile("mov %%cr3, %0" : "=r"(cr3)); // Read the CR3 register

    return cr3;
}




void init_paging()
{  

    // Lowest Virtual address 0xFFFFFFFF80322000 which have page and the page.frame address pointer have 0xFFFFFFFFFFFFFFFF value.
    // Paging is enabled by Limine. Get the pml4 table pointer address that Limine set up
    current_pml4 = (pml4_t *) get_cr3_addr();

    // Updating lower half pages
    // for (uint64_t addr = LOW_HALF_START; addr < LOW_HALF_END; addr += PAGE_SIZE) {
    //     page_t *page = get_page(addr, 1, current_pml4);
    //     if (!page) {
    //         // Handle error: Failed to get the page entry
    //         continue;
    //     }

    //     // Allocate a frame if not already allocated
    //     if (!page->frame) {
    //         alloc_frame(page, 0, 1); // Allocate a frame with user-level access
    //     }

    //     // Set the User flag (0x4 in x86_64) to allow user-level access
    //     page->present = 1;  // Ensure the page is present
    //     page->rw = 1;       // Allow read/write access
    //     page->user = 1;     // Set user-accessible bit
    // }

    // // Invalidate the TLB for the changes to take effect
    // // asm volatile("mov %%cr3, %%rax; mov %%rax, %%cr3" ::: "rax");
    // flush_tlb_all();
    
    printf("Successfully Paging initialized.\n");
}


static page_t *alloc_page(){
    page_t *pg = (page_t *) kmalloc_a(sizeof(page_t), 1);
    if(pg){
        memset(pg, 0, sizeof(pg));
    }
    return pg;
}


// Function to allocate a new page table
static pt_t* alloc_pt() {
    pt_t* pt = (pt_t*)kmalloc_a(sizeof(pt_t), 1);
    if (pt) {
        memset(pt, 0, sizeof(pt_t)); // Zero out the page table
    }
    return pt;
}


// Function to allocate a new page directory
static pd_t* alloc_pd() {
    pd_t* pd = (pd_t*)kmalloc_a(sizeof(pd_t), 1);
    if (pd) {
        memset(pd, 0, sizeof(pd_t)); // Zero out the page directory
    }
    return pd;
}


// Function to allocate a new page directory pointer table
static pdpt_t* alloc_pdpt() {
    pdpt_t* pdpt = (pdpt_t*)kmalloc_a(sizeof(pdpt_t), 1);
    if (pdpt) {
        memset(pdpt, 0, sizeof(pdpt_t)); // Zero out the PDPT
    }
    return pdpt;
}


// This function will return corresponding page pointer from virtual address
// The below function will not create a new pdpt, pd, pt and pages if already present for given virtual address
page_t* get_page(uint64_t va, int make, pml4_t* pml4) {

    uint64_t pml4_index = PML4_INDEX(va);
    uint64_t pdpt_index = PDPT_INDEX(va);
    uint64_t pd_index = PD_INDEX(va);
    uint64_t pt_index = PT_INDEX(va);
    uint64_t page_offset = PAGE_OFFSET(va);

    if(!pml4){
        return NULL;
    }

    // Get the PML4 entry from pml4_index which is found from va
    dir_entry_t* pml4_entry = &pml4->entry_t[pml4_index];

    // the below if block will create pdpt if not present then make a new pdpt by bool make = 1
    if (!pml4_entry->present) {
        if (!make) {
            return NULL; // Page table does not exist and we are not allowed to create it
        }
        // Allocate a new PDPT
        pdpt_t* pdpt = alloc_pdpt(); // Creating a new pdpt
        if (!pdpt) {
            return NULL; // Allocation failed
        }
        // Set up the PML4 entry
        pml4_entry->present = 1;
        pml4_entry->rw = 1; // Read/write
        pml4_entry->user = 0; // Kernel mode
        pml4_entry->base_addr = (uint64_t) pdpt >> 12; // Base address of PDPT
    }

    // Get the PDPT entry
    pdpt_t* pdpt = (pdpt_t*)(pml4_entry->base_addr << 12); // Converting base address into pdpt pointer
    dir_entry_t* pdpt_entry = &pdpt->entry_t[pdpt_index]; // 

    // the below if block will create pd if not present and make = 1
    if (!pdpt_entry->present) {
        if (!make) {
            return NULL; // Page directory does not exist and we are not allowed to create it
        }
        // Allocate a new PD
        pd_t* pd = alloc_pd();
        if (!pd) {
            return NULL; // Allocation failed
        }
        // Set up the PDPT entry
        pdpt_entry->present = 1;
        pdpt_entry->rw = 1; // Read/write
        pdpt_entry->user = 0; // Kernel mode
        pdpt_entry->base_addr = (uint64_t)pd >> 12; // Base address of PD
    }

    // Get the PD entry
    pd_t* pd = (pd_t*)(pdpt_entry->base_addr << 12);
    dir_entry_t* pd_entry = &pd->entry_t[pd_index];

    // the below if block will create pt if not present and make = 1 
    if (!pd_entry->present) {
        if (!make) {
            return NULL; // Page table does not exist and we are not allowed to create it
        }
        // Allocate a new PT
        pt_t* pt = alloc_pt();
        if (!pt) {
            return NULL; // Allocation failed
        }
        // Set up the PD entry
        pd_entry->present = 1;
        pd_entry->rw = 1; // Read/write
        pd_entry->user = 0; // Kernel mode
        pd_entry->base_addr = (uint64_t)pt >> 12; // Base address of PT
    }

    // Get the PT entry from pd_entry->base_addr
    pt_t* pt = (pt_t*)(pd_entry->base_addr << 12);

    page_t *page = (page_t *) &pt->pages[pt_index];

    if(page != NULL) page->present = 1;

    // return page pointer
    return page;
}


// The below function will print some debug message for page fault 
void page_fault_handler(registers_t *regs)
{
    // A page fault has occurred.
    // Retrieve the faulting address from the CR2 register.
    uint64_t faulting_address;
    asm volatile("mov %%cr2, %0" : "=r"(faulting_address));

    // Decode the error code to determine the cause of the page fault.
    int present = !(regs->err_code & 0x1); // Page not present
    int rw = regs->err_code & 0x2;         // Write operation?
    int us = regs->err_code & 0x4;         // Processor was in user mode?
    int reserved = regs->err_code & 0x8;   // Overwritten CPU-reserved bits of page entry?
    int id = regs->err_code & 0x10;        // Caused by an instruction fetch?

    // Output an error message with details about the page fault.
    printf("Page fault! ( ");
    if (present) printf("not present ");
    if (rw) printf("write ");
    if (us) printf("user-mode ");
    if (reserved) printf("reserved ");
    if (id) printf("instruction fetch ");
    printf(") at address %x\n", faulting_address);


    // Additional action to handle the page fault could be added here,
    // such as invoking a page allocator or terminating a faulty process.

    // Halt the system to prevent further errors (for now).
    printf("Halting the system due to page fault.\n");
    halt_kernel();
}

void test_paging() {

    printf("\nTest of Paging\n");

    // uint64_t va1 = (uint64_t) PAGE_ALIGN(0xFFFFFFFF80322000 + 40*PAGE_SIZE);

    // test following address
    uint64_t va1 = VIRTUAL_BASE;
    // uint64_t va2 = PAGE_ALIGN(KMEM_LOW_BASE) + PHYSICAL_TO_VIRTUAL_OFFSET + PAGE_SIZE; 
 
    // Virtual Pointer on based higher half
    uint64_t *v_ptr1 = (uint64_t *) va1; 
    // uint64_t *v_ptr2 = (uint64_t *) va2; 

    printf("Previous content of v_ptr1: %x\n", *v_ptr1);

    // Get Page pointer from above virtual pointer address
    page_t *page1 = (page_t *) get_page((uint64_t)v_ptr1, 1, current_pml4);
    // page_t *page2 = get_page((uint64_t)v_ptr2, 1, current_pml4);

    debug_page(page1);
    // debug_page(page2);
    
    // allocate a physical frame address in above page with kernel level and writable
    alloc_frame(page1, 1, 1); 
    // alloc_frame(page2, 1, 1); 

    debug_page(page1);
    // debug_page(page2);  

    // Store a value at the virtual pointer
    *v_ptr1 = 0x567; 
    // *v_ptr2 = 0xABC50;  

    printf("Content of v_ptr1: %x\n", *v_ptr1);


    // print("Content of v_ptr2: ");
    // print_hex(*v_ptr2);
    // print("\n");

    printf("Finish Paging Test\n");

    uint64_t val = *frames;

    printf("%x\n", val);
}



// Function to flush TLB for a specific address
void flush_tlb(uint64_t address) {
    // Use the invlpg instruction to invalidate the TLB entry for a specific address
    asm volatile("invlpg (%0)" : : "r"(address) : "memory");
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
    uint64_t virt = phys;  // Mapping physical to virtual 1:1 for simplicity.

    // Assume we already have the base PML4 loaded in CR3
    pml4 = (uint64_t *)get_cr3_addr();

    // Iterate through the address range to map
    for (size_t offset = 0; offset < size; offset += 0x1000) {
        uint64_t current_phys = phys + offset;
        uint64_t current_virt = virt + offset;

        // Calculate indices in the paging hierarchy
        pml4_index = (current_virt >> 39) & 0x1FF;
        pdpt_index = (current_virt >> 30) & 0x1FF;
        pd_index   = (current_virt >> 21) & 0x1FF;
        pt_index   = (current_virt >> 12) & 0x1FF;

        // Get or create PDPT
        if (!(pml4[pml4_index] & PAGE_PRESENT)) {
            pdpt = (uint64_t *)kmalloc_a(0x1000,1);
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




