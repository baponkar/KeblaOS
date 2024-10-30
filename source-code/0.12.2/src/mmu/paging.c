
#include "paging.h"

#include "../stdlib/stdio.h"
// A bitset of frames - used or free.
uint32_t *frames;
uint32_t nframes;   // this hold all the frames info


extern uint32_t placement_address; // Defined in heap.c
extern uint32_t mem_end_page;      // Defined in Kernel.c

extern gdt_entry_t gdt_entries[5];
extern gdt_ptr_t   gdt_ptr;

// Macros used in the bitset algorithms.
#define INDEX_FROM_BIT(a) (a/(8*4)) // Each index can hold 32 bit
#define OFFSET_FROM_BIT(a) (a%(8*4))

// Declare the current directory globally
page_directory_t *current_directory;
page_directory_t *kernel_directory;



// Static function to set a bit in the frames bitset
// A bitset is typically used to keep track of allocated and free memory frames in a paging system.
static void set_frame(uint32_t frame_addr)
{
   uint32_t frame = frame_addr/0x1000;      // dividing by 4kb
   uint32_t idx = INDEX_FROM_BIT(frame);    // get the index in the frames array
   uint32_t off = OFFSET_FROM_BIT(frame);   // get the offset (bit position) within that index
   frames[idx] |= (0x1 << off);     // set the bit at the calculated position to 1
}



// Static function to clear a bit in the frames bitset
static void clear_frame(uint32_t frame_addr)
{
   uint32_t frame = frame_addr/0x1000;
   uint32_t idx = INDEX_FROM_BIT(frame);
   uint32_t off = OFFSET_FROM_BIT(frame);
   frames[idx] &= ~(0x1 << off);
}



// Static function to test if a bit is set.
uint32_t test_frame(uint32_t frame_addr)
{
   uint32_t frame = frame_addr/0x1000;
   uint32_t idx = INDEX_FROM_BIT(frame);
   uint32_t off = OFFSET_FROM_BIT(frame);
   return (frames[idx] & (0x1 << off));
}



// Static function to find the first free frame.
static uint32_t first_frame()
{
   uint32_t i, j;
   for (i = 0; i < INDEX_FROM_BIT(nframes); i++)
   {
       if (frames[i] != 0xFFFFFFFF) // nothing free, exit early.
       {
           // at least one bit is free here.
           for (j = 0; j < 32; j++)
           {
               uint32_t toTest = 0x1 << j;
               if ( !(frames[i]&toTest) )
               {
                   return i*4*8+j;
               }
           }
       }
   }
}


// Function to allocate a frame.
void alloc_frame(page_t *page, int is_kernel, int is_writeable)
{
   if (page->frame != 0)
   {
       return; // Frame was already allocated, return straight away.
   }
   else
   {
       uint32_t idx = first_frame(); // idx is now the index of the first free frame.
       if (idx == (uint32_t) -1)
       {
           // PANIC is just a macro that prints a message to the screen then hits an infinite loop.
           PANIC("No free frames!");
       }
       set_frame(idx*0x1000); // this frame is now ours!
       page->present = 1; // Mark it as present.
       page->rw = (is_writeable)?1:0; // Should the page be writeable? writable=1, readable=0
       page->user = (is_kernel)?0:1; // Should the page be user-mode? kernel=0, user=1
       page->frame = idx;
   }
}


// Function to deallocate a frame.
void free_frame(page_t *page)
{
   uint32_t frame;
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


void init_paging()
{
    // The size of physical memory. For the moment we
    // assume it is 2MB big.
    // uint32_t mem_end_page = 0x400000;   // Consider System has 4MB of memory
    // uint32_t mem_end_page = 0x100000000 - 0x1;   // Consider System has 4GB of memory

    printf("Memory End address : %x\n", mem_end_page);
    nframes = mem_end_page / 0x4000;
    frames = (uint32_t*) kmalloc(INDEX_FROM_BIT(nframes));
    if (frames == NULL) {
        printf("Failed to allocate frame bitmap\n");
        return;
    }
    memset(frames, 0, INDEX_FROM_BIT(nframes));

    // Let's make a page directory.
    kernel_directory = (page_directory_t*) kmalloc_a(sizeof(page_directory_t), 1);
    memset(kernel_directory, 0, sizeof(page_directory_t));
    current_directory = kernel_directory;
        
    // We need to identity map (phys addr = virt addr) from
    // 0x0 to the end of used memory, so we can access this
    // transparently, as if paging wasn't enabled.
    // NOTE that we use a while loop here deliberately.
    // inside the loop body we actually change placement_address
    // by calling kmalloc(). A while loop causes this to be
    // computed on-the-fly rather than once at the start.
    int i = 0;
    // The below steps allocate 2 Mb memory
    // Identity Mapping for Used Memory
    while (i < placement_address)
    {
        // Kernel code is readable but not writeable from userspace.
        alloc_frame( get_page(i, 1, kernel_directory), 0, 0);
        i += 0x1000; // Move to the next page.
    }
    

    // Map additional user-accessible pages for user-mode programs
    // This example maps the virtual range from 0x400000 to 0x800000 as user-accessible
    int j = 0;
    uint32_t user_mem_start = 0x400000;
    uint32_t user_mem_end = 0x800000;
    while (j < user_mem_end) {
        alloc_frame(get_page(i, 1, kernel_directory), 1, 1);  // User, read-write
        j += 0x1000;
    }

    print("Inside of Paging!\n");

    disable_interrupts();
    // Before we enable paging, we must register our page fault handler.
    interrupt_install_handler(14, &page_fault);
    enable_interrupts();

    // Now, enable paging!
    switch_page_directory(kernel_directory);
    print("Inside of Paging!\n");
}



void switch_page_directory(page_directory_t *dir)
{
   current_directory = dir;
   asm volatile("mov %0, %%cr3":: "r"(&dir->tablesPhysical));
   uint32_t cr0;
   asm volatile("mov %%cr0, %0": "=r"(cr0));
   cr0 |= 0x80000000; // Enable paging!
   asm volatile("mov %0, %%cr0":: "r"(cr0));
}


page_t *get_page(uint32_t address, int make, page_directory_t *dir)
{
   // Turn the address into an index.
   address /= 0x1000;
   // Find the page table containing this address.
   uint32_t table_idx = address / 1024;
   if (dir->tables[table_idx]) // If this table is already assigned
   {
       return &dir->tables[table_idx]->pages[address%1024];
   }
   else if(make)
   {
       uint32_t tmp;
       dir->tables[table_idx] = (page_table_t*) kmalloc_ap(sizeof(page_table_t),1, &tmp);
       memset(dir->tables[table_idx], 0, 0x1000);
       dir->tablesPhysical[table_idx] = tmp | 0x7; // PRESENT, RW, US.
       return &dir->tables[table_idx]->pages[address%1024];
   }
   else
   {
       return 0;
   }
}



void page_fault(registers_t * regs)
{
   // A page fault has occurred.
   // The faulting address is stored in the CR2 register.
   uint32_t faulting_address;
   asm volatile("mov %%cr2, %0" : "=r" (faulting_address));

   // The error code gives us details of what happened.
   int present   = !(regs->err_code & 0x1); // Page not present // When set, the page fault was caused by a page-protection violation. When not set, it was caused by a non-present page.
   // err_code = 1 page protection violation
   // err_code = 0 non present page
   int rw = regs->err_code & 0x2;           // Write operation?
   int us = regs->err_code & 0x4;           // Processor was in user-mode?
   int reserved = regs->err_code & 0x8;     // Overwritten CPU-reserved bits of page entry?
   int id = regs->err_code & 0x10;          // Caused by an instruction fetch?

   // Output an error message.
    errprint("Page fault! ( ");
    if (!present) {
        print("Thie page is present ");
    }else{
        errprint("The Page is not Prsent! ");
    }
    if (rw) {errprint(" read-only ");}
    if (us) {errprint(" user-mode ");}
    if (reserved) {print(" reserved ");}
    print_hex(faulting_address);
    errprint(" )");
    errprint("\n");
    PANIC("Page fault");
}


// A function that will attempt to access an unmapped address and cause a page fault
void trigger_page_fault_by_access() {
    uint32_t *invalid_address = (uint32_t*)0xDEADBEEF;  // 3.478 GB ,Choose an address that is not allocated
    
    // Attempt to read from the invalid address, causing a page fault
    uint32_t value = *invalid_address;

    // This line will likely never be reached since a page fault should occur above
    // and your page fault handler should handle it.
    print("Value at invalid address: ");
    print_hex(value);  // If no page fault occurred (which it should), print the value.
}


void check_paging_enabled() {
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    if (cr0 & 0x80000000) {
        print("Paging enabled!\n");
    } else {
        print("Paging NOT enabled!\n");
    }
}



// Function to get the DPL of a segment descriptor
uint8_t get_dpl_from_address(uint32_t address) {
    uint32_t descriptor_index = address >> 22; // Assuming 4KB pages (12 bits offset)
    
    // The actual descriptor index might depend on your GDT structure.
    // This example is simplified for demonstration purposes.
    gdt_entry_t *descriptor = &gdt_entries[descriptor_index];

    // Extract the DPL (bits 5-6 of the access byte)
    uint8_t dpl = (descriptor->access >> 5) & 0x03; // Bits 5-6 give us DPL
    return dpl;
}

uint8_t get_current_privilege_level() {
    uint16_t cs;
    
    asm volatile (
        "push %%cs;"         // Push CS onto the stack
        "pop %0;"           // Pop it into a general-purpose register
        : "=r"(cs)          // Output operand
        :                    // No input operands
        :                    // No clobbered registers
    );
    
    // The privilege level is in the lower 2 bits of CS
    return cs & 0x03;       // Extracting the RPL (Request Privilege Level)
}


// Function to check if an address is accessible from the current privilege level
void check_address_access(uint32_t address) {
    uint8_t dpl = get_dpl_from_address(address);
    uint8_t current_privilege_level = get_current_privilege_level(); // Get the privilege level from CS

    if (dpl <= current_privilege_level) {
        // Access is allowed
        print("Address access allowed\n");
    } else {
        // Access is denied
        print("Address access denied\n");
    }
}

