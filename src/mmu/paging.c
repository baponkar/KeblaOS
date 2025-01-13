/*
Paging in x86_64

https://wiki.osdev.org/Paging
https://wiki.osdev.org/Identity_Paging
https://web.archive.org/web/20160326061042/http://jamesmolloy.co.uk/tutorial_html/6.-Paging.html
https://github.com/dreamportdev/Osdev-Notes/blob/master/04_Memory_Management/03_Paging.md
https://stackoverflow.com/questions/18431261/how-does-x86-paging-work

*/

#include "paging.h"

extern void enable_paging(uint64_t pml4_address); // present in load_paging.asm
extern void disable_paging();   //  present in load_paging.asm


pml4_t *current_pml4;
pml4_t *user_pml4;
pml4_t *kernel_pml4;


// allocate a page with the free physical frame
void alloc_frame(page_t *page, int is_kernel, int is_writeable) {
    if (page->frame != 0) {
        print("Frame was already allocated!\n");
        return; // Frame was already allocated, return straight away.
    }

    uint64_t bit_no = free_frame_bit_no(); // idx is now the index of the first free frame.

    if (bit_no == (uint64_t)-1) {
        print("No free frames!");
        halt_kernel();
    }

    set_frame(bit_no); // Mark the frame as used by passing the frame index

    page->present = 1;       // Mark it as present.
    page->rw = (is_writeable) ? 1 : 0; // Should the page be writeable?
    page->user = (is_kernel) ? 0 : 1; // Should the page be user-mode?
    page->frame = (KERNEL_MEM_START_ADDRESS + (bit_no * FRAME_SIZE)) >> 12; // Store physical base address
    KERNEL_MEM_START_ADDRESS += FRAME_SIZE; // Update the new KERNEL_MEM_START_ADDRESS
}


// Function to deallocate a frame.
void free_frame(page_t *page)
{
   uint64_t frame_addr = page->frame;
   if (!(frame_addr))
   {
       return; // The given page didn't actually have an allocated frame!
   }
   else
   {
        uint64_t frame_idx = ADDR_TO_BIT_NO(frame_addr);
        clear_frame(frame_idx); // Frame is now free again.
        page->frame = 0; // Page now doesn't have a frame.
   }
}


// return current cr3 address i.e. root pml4 pointer address
uint64_t get_cr3_addr() {
    uint64_t cr3;
    asm volatile("mov %%cr3, %0" : "=r"(cr3)); // Read the CR3 register

    return cr3;
}


void initialise_paging()
{  
    // Paging is enabled by Limine. Get the pml4 table pointer address that Limine set up
    current_pml4 = (pml4_t *) get_cr3_addr();

    print("Successfully Paging have initialized!\n");
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

    // Get the PML4 entry from pml4_index which is found from va
    dir_entry_t* pml4_entry = &pml4->entry_t[pml4_index];

    // the below if block will create pdpt if not present and make = 1
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
        pml4_entry->base_addr = (uint64_t)pdpt >> 12; // Base address of PDPT
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

    // return page pointer
    return &pt->pages[pt_index];
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
    print("Page fault! ( ");
    if (present) print("not present ");
    if (rw) print("write ");
    if (us) print("user-mode ");
    if (reserved) print("reserved ");
    if (id) print("instruction fetch ");
    print(") at address ");
    print_hex(faulting_address);
    print("\n");


    // Additional action to handle the page fault could be added here,
    // such as invoking a page allocator or terminating a faulty process.

    // Halt the system to prevent further errors (for now).
    print("Halting the system due to page fault.\n");
    halt_kernel();

}



void test_paging(){
    print("Start Paging Test...\n");
    uint64_t* test_address = (uint64_t*) 0xFFFFFFFF80000000;  // Higher-half virtual address
    *test_address = 0x12345678ABCDEF00;

    print("The value at pointer *0xFFFFFFFF80000000 : "); 
    print_hex(*test_address);
    print("\n");

    // uint64_t* invalid_address = (uint64_t*)0xFFFFFFFF90000000;  // Unmapped address
    // *invalid_address = 0x0;  // This should trigger a page fault

    // Check if a virtual address is already mapped (no allocation)
    page_t* page = get_page(0x400000, 0, current_pml4);
    if (page == NULL) {
        print("Page not mapped yet!\n");
    }

    // Allocate a page if it does not exist
    page_t* page2 = get_page(0x400000, 1, current_pml4);
    if (page2 != NULL) {
        print("Page successfully allocated!\n");
    }


    print("Finish Paging Test\n");
}


