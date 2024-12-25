/*
Paging in x86_64

https://wiki.osdev.org/Paging
https://wiki.osdev.org/Identity_Paging
https://web.archive.org/web/20160326061042/http://jamesmolloy.co.uk/tutorial_html/6.-Paging.html
https://github.com/dreamportdev/Osdev-Notes/blob/master/04_Memory_Management/03_Paging.md
https://stackoverflow.com/questions/18431261/how-does-x86-paging-work
*/

#include "paging.h"

extern void enable_paging(uint64_t pml4_address);
extern void disable_paging();

extern uint64_t *frames;
extern uint64_t nframes; // Total number of frames

extern uint64_t placement_address;  // The value of it will set in kernel.c 
extern uint64_t mem_end_address;    // The value of it will set in kernel.c 

pml4_t *current_pml4;
pml4_t *user_pml4;
pml4_t *kernel_pml4;

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


uint64_t get_cr3_addr() {
    uint64_t cr3;
    // Read the CR3 register
    asm volatile("mov %%cr3, %0" : "=r"(cr3));

    return cr3;
}


void initialise_paging()
{  
    nframes = (uint64_t) mem_end_address / 0x1000; // Total numbers of frames
    frames = (uint64_t *) kmalloc(INDEX_FROM_BIT(nframes)); // container the used and unused frames data

    memset(frames, 0, INDEX_FROM_BIT(nframes));

    // Paging is enabled by Limine. Get the pml4 table pointer address that Limine set up
    current_pml4 = (pml4_t *) get_cr3_addr();

// Michael Petch is unsure what you were attempting to do here
// so he can't help.
#if 0
    // Identity-map the physical memory from `placement_address` up to mem_end_address
    uint64_t i = placement_address;

    while( i < mem_end_address) // increment 4kb i.e. 0x1000
    {
        // Kernel code is readable but not writeable from user space
        page_t *page = get_page(i, 1, kernel_pml4);
        alloc_frame(page, 0, 0);
        i += 0x1000;
    }

    // Update CR3 to flush the TLB
    asm("mov %0, %%cr3" :: "r"(kernel_pml4) : "memory");
#endif

    print("Successfully Paging have initialized!\n");
}



page_t *get_page(uint64_t address, int make, pml4_t *pml4) {
    // Note from Michael Petch - it may be by design but this
    // code doesn't support processing the PAGESIZE bit which
    // could become a problem??

    // Creating different index from virtual address
    uint64_t pml4_idx = PML4_INDEX(address);
    uint64_t pdpt_idx = PDPT_INDEX(address);
    uint64_t pd_idx = PD_INDEX(address);
    uint64_t pt_idx = PT_INDEX(address);

    // Resolve PML4 entry
    if (!(pml4->entry_t[pml4_idx].present)) { // Check if entry is present
        if (make) {
            pdpt_t *new_pdpt = (pdpt_t *) kmalloc_a(sizeof(pdpt_t), 1); // create a new pdpt with 4 kb aligned
            memset(new_pdpt, 0, sizeof(pdpt_t)); // Zero out memory for safety
            // pml4 will hold address of new_pdpt and make it present and writable
            pml4->entry_t[pml4_idx].present = 1; // present
            pml4->entry_t[pml4_idx].rw = 1;      // writable
            pml4->entry_t[pml4_idx].user = 0;    // superuser or kernel user
            pml4->entry_t[pml4_idx].base_addr = (uint64_t) new_pdpt;  // pdpt address
        } else {
            return NULL; // Entry not present and not creating
        }
    }

    // create  pdpt from pml4->entry_t[pml4_idx]
    pdpt_t *pdpt = (pdpt_t *) pml4->entry_t[pml4_idx].base_addr; 

    // Resolve PDPT entry
    if (!(pdpt->entry_t[pdpt_idx].present)) { // Check if entry is present
        if (make) {
            pd_t *new_pd = (pd_t *)kmalloc_a(sizeof(pd_t), 1); // making new pd with 4 kb aligned
            memset(new_pd, 0, sizeof(pd_t)); // Zero out memory for safety
            pdpt->entry_t[pdpt_idx].present = 1; // present
            pdpt->entry_t[pdpt_idx].rw = 1; // writable
            pdpt->entry_t[pdpt_idx].user = 1; // superuser or kernel user
            pdpt->entry_t[pdpt_idx].base_addr = (uint64_t) new_pd; // store new_pd address
        } else {
            return NULL;
        }
    }

    // Creating pd from pdpt->entry_t[pdpt_idx]
    pd_t *pd = (pd_t *) pdpt->entry_t[pdpt_idx].base_addr;

    // Resolve PD entry
    if (!(pd->entry_t[pd_idx].present)) {// Check if entry is present
        if (make) {
            pt_t *new_pt = (pt_t *)kmalloc_a(sizeof(pt_t), 1); // making new pt with 4 kb aligned
            memset(new_pt, 0, sizeof(pt_t)); // Zero out memory for safety
            pd->entry_t[pd_idx].present = 1; // present
            pd->entry_t[pd_idx].rw = 1; // writable
            pd->entry_t[pd_idx].user = 1; // superuser or kernel user
            pd->entry_t[pd_idx].base_addr = (uint64_t) new_pt; // store new_pt address
        } else {
            return NULL;
        }
    }

    // Resolve Page Table
    pt_t *pt = (pt_t *) pd->entry_t[pd_idx].base_addr; // Creating Page Table from pd->entry_t[pd_idx]

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

    print("Finish Paging Test\n");
}


