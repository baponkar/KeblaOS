/*
https://wiki.osdev.org/Paging
*/


#include "paging.h"

extern uint64_t *frames;
extern uint64_t nframes;

extern uint64_t placement_address; // The value of it will set in kernel.c 
extern uint64_t mem_end_address;    // The value of it will set in kernel.c 


pml4_t * current_pml4;
pml4_t * kernel_pml4;

// Function to allocate a frame.
void alloc_frame(page_t *page, int is_kernel, int is_writeable)
{
   if (page->frame != 0)
   {
       return; // Frame was already allocated, return straight away.
   }
   else
   {
       uint64_t idx = first_frame(); // idx is now the index of the first free frame.
       if (idx == (uint64_t)-1)
       {
           // PANIC is just a macro that prints a message to the screen then hits an infinite loop.
           print("No free frames!");
       }
       set_frame(idx*0x1000); // this frame is now ours!
       page->present = 1; // Mark it as present.
       page->rw = (is_writeable)?1:0; // Should the page be writeable?
       page->user = (is_kernel)?0:1; // Should the page be user-mode?
       page->frame = placement_address + idx*PAGE_SIZE;
       placement_address += PAGE_SIZE;
   }
}

// Function to deallocate a frame.
void free_frame(page_t *page)
{
   uint64_t frame;
   if (!(frame=page->frame))
   {
       return; // The given page didn't actually have an allocated frame!
   }
   else
   {
       clear_frame(frame); // Frame is now free again.
       page->frame = 0x0; // Page now doesn't have a frame.
   }
}

void print_cr3() {
    uint64_t cr3;
    // Read the CR3 register
    asm volatile("mov %%cr3, %0" : "=r"(cr3));
    
    // Print the value of CR3
    print("CR3 register value: ");
    print_hex(cr3);
    print("\n");
}


void initialise_paging()
{   /*
        placement_address = 0x100000
        mem_end_address   = 0x105000
        mem_range = 0x105000 - 0x100000 = 0x5000 = 20 KB
        total_no_of_pages = mem_range / PAGE_SIZE = 0x5000 / 0x1000 = 0x5
        required_pages = nframes(8B) + PML4 (4KB) + PDPT (4KB) + PD (4KB) + PT (4KB) + PAGES(4n KB) = 8B + 16 KB + 4n KB
        * for single page required memory = 20.1 KB
        * for double page required memory = 24.1 KB
        * for triple page required memory = 28.1 KB (*****)
        * for qudra page required memory = 32.1 KB
    */

    print("Paging Start\n");
    print("=>placement_address : ");
    print_hex(placement_address);
    print(", ");
    print("Memory End Address  : ");
    print_hex(mem_end_address);
    print("\n");

    nframes = (mem_end_address - placement_address) / 0x1000;
    frames = (uint64_t *) kmalloc(INDEX_FROM_BIT(nframes)); // less of 4 kb memory aligned
    memset(frames, 0, INDEX_FROM_BIT(nframes));
    // Allocate and zero-initialize the PML4 table
    kernel_pml4 = (pml4_t *) kmalloc_a(sizeof(pml4_t), 1); // 4 kb memory aligned
    memset(kernel_pml4, 0, sizeof(pml4_t));
    current_pml4 = kernel_pml4;

    print("Total number of frames : ");
    print_dec(nframes);
    print("\n");

    // Identity-map the physical memory up to `placement_address`
    uint64_t i = placement_address;

    while( i < mem_end_address) // increment 4kb i.e. 0x1000
    {
        // Kernel code is readable but not writeable from user space
        page_t *page = get_page(i, 1, kernel_pml4);
        alloc_frame(page, 0, 0);
        i += 0x1000;
    }
    
    print("placement_address : ");
    print_hex(placement_address);
    print("\n");

    switch_to_page_table(kernel_pml4);  // Enable paging
}


void switch_to_page_table(pml4_t *pml4) {
    // Update the global current PML4 pointer
    uint64_t physical_address = (uint64_t) pml4;

    // Load the physical address of the new PML4 table into CR3
    asm volatile("mov %0, %%cr3" :: "r"((uint64_t) physical_address) : "memory");

    // Enable paging by setting the paging (PG) bit in CR0
    uint64_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));  // Read CR0
    cr0 |= 0x80000000;                          // Set the PG bit (bit 31)
    asm volatile("mov %0, %%cr0" :: "r"(cr0));  // Write CR0
}

page_t *get_page(uint64_t address, int make, pml4_t *pml4) {
    uint64_t pml4_idx = PML4_INDEX(address);
    uint64_t pdpt_idx = PDPT_INDEX(address);
    uint64_t pd_idx = PD_INDEX(address);
    uint64_t pt_idx = PT_INDEX(address);

    print("-----------------------------------------------------------------------------------------------------------------------------\n");
    print("PML4 index : ");
    print_dec(pml4_idx);
    print(", ");
    print("PDPT index : ");
    print_dec(pdpt_idx);
    print(", ");
    print("PD index : ");
    print_dec(pd_idx);
    print(", ");
    print("PT index : ");
    print_dec(pt_idx);  
    print("\n");

    // Resolve PML4 entry
    if (!(pml4->entry_t[pml4_idx] & 0x1)) { // Check if entry is present
        if (make) {
            pdpt_t *new_pdpt = (pdpt_t *) kmalloc_a(sizeof(pdpt_t), 1); // create a new pdpt with 4 kb aligned
            memset(new_pdpt, 0, sizeof(pdpt_t)); // Zero out memory for safety
            pml4->entry_t[pml4_idx] = ((uint64_t)new_pdpt & ~0xFFF) << 12 | 0x3; // pml4 will hold address of new_pdpt and make it present and writable
        } else {
            return NULL; // Entry not present and not creating
        }
    }

    print("pml4 addr : ");
    print_hex((uint64_t) pml4);
    print(", ");

    print("content of pml4.entries[pml4_idx] : ");
    print_hex(pml4->entry_t[pml4_idx]);
    print("\n");

    pdpt_t *pdpt = (pdpt_t *)((pml4->entry_t[pml4_idx] >> 12 & ~0xFFF)); // create  pdpt from pml4->entry_t[pml4_idx]

    // Resolve PDPT entry
    if (!(pdpt->entry_t[pdpt_idx] & 0x1)) { // Check if entry is present
        if (make) {
            pd_t *new_pd = (pd_t *)kmalloc_a(sizeof(pd_t), 1); // making new pd with 4 kb aligned
            memset(new_pd, 0, sizeof(pd_t));
            pdpt->entry_t[pdpt_idx] = ((uint64_t)new_pd & ~0xFFF) << 12 | 0x3;
        } else {
            return NULL;
        }
    }

    print("pdpt addr : ");
    print_hex((uint64_t) pdpt);
    print(", ");

    print("content of pdpt.entries[pdpt_idx] : ");
    print_hex(pdpt->entry_t[pdpt_idx]);
    print("\n");

    pd_t *pd = (pd_t *)((pdpt->entry_t[pdpt_idx] >> 12 & ~0xFFF));

    // Resolve PD entry
    if (!(pd->entry_t[pd_idx] & 0x1)) {// Check if entry is present
        if (make) {
            pt_t *new_pt = (pt_t *)kmalloc_a(sizeof(pt_t), 1); // making new pt with 4 kb aligned
            memset(new_pt, 0, sizeof(pt_t));
            pd->entry_t[pd_idx] = ((uint64_t)new_pt & ~0xFFF) << 12 | 0x3;
        } else {
            return NULL;
        }
    }

    print("pd addr : ");
    print_hex((uint64_t) pd);
    print(", ");

    print("content of pd.entries[pd_idx] : ");
    print_hex(pd->entry_t[pd_idx]);
    print("\n");

    // Resolve Page Table
    pt_t *pt = (pt_t *)((pd->entry_t[pd_idx] >> 12 & ~0xFFF)); // Creating Page Table from pd->entry_t[pd_idx]

    print("pt addr : ");
    print_hex((uint64_t)pt);
    print(", ");

    print("content of pt->pages[pt_idx] : ");
    print_hex((uint64_t)&pt->pages[pt_idx]);
    print("\n");

    // Resolve Page
    page_t *page =(page_t *) &pt->pages[pt_idx];
    if(!(page->present)){ // Check if entry is present
         if (make) {
            page = (page_t *) kmalloc_a(sizeof(page_t), 1); // making new page  with 4 kb aligned
            memset(page, 0, sizeof(page_t));
            alloc_frame(page, 1, 1); // Allocate frame for the page
        }else{
            return NULL;
        }
    }

    print("page frame address : ");
    print_hex((uint64_t) page->frame);
    print("\n");

    return page;
}



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
    asm volatile("hlt");

}

void test_paging(){
    uint64_t* test_address = (uint64_t*)0xFFFFFFFF80000000;  // Higher-half virtual address
    *test_address = 0x12345678ABCDEF00;

    uint64_t* invalid_address = (uint64_t*)0xFFFFFFFF90000000;  // Unmapped address
    *invalid_address = 0x0;  // This should trigger a page fault
}


