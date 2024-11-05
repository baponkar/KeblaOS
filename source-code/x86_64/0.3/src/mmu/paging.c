#include "paging.h"

extern void load_cr3(uint64_t pml4_base_address);

// Enabaling first 4MB memory paging
// one single Table has 512 * 4Kb = 2048 Kb memory = 2MB memory
// So we need 2 page table(PT), one Page Directory(PD), one PDPR, one PML4 
// PML4 --> PDPR --> PD --> PT --> Pageframe
// PT has 512 entries of 4kb pages

//uint64_t kernel_phys_base = 0x100000; // 1MB : Physical start of kernel
//uint64_t kernel_virt_base = 0xFFFFFFFF80000000; // Virtual start of kernel 4096 TB

#define PML4_IDX(virt) (((virt) >> 39) & 0x1ff)
#define PDPT_IDX(virt) (((virt) >> 30) & 0x1ff)
#define PD_IDX(virt)   (((virt) >> 21) & 0x1ff)
#define PT_IDX(virt)   (((virt) >> 12) & 0x1ff)

pml4_t  pml4_table[512]__attribute__((aligned(4096)));
pdpt_t  pdpt_table[512]__attribute__((aligned(4096)));
pd_t    pd_table[512]__attribute__((aligned(4096)));
pt_t    pt_table[512]__attribute__((aligned(4096)));

void init_paging(){
    print("Initializing Paging...\n");

    // Setting up a single entry as an example for the higher-half kernel mapping.
    pml4_table[PML4_IDX(kernel_virt_base)].p = 1;  // Using last PML4 entry to map higher-half
    pml4_table[PML4_IDX(kernel_virt_base)].rw = 1;
    pml4_table[PML4_IDX(kernel_virt_base)].tab_addr = ((uint64_t)pdpt_table - kernel_offset) >> 12;

    pdpt_table[PDPT_IDX(kernel_virt_base)].p = 1;
    pdpt_table[PDPT_IDX(kernel_virt_base)].rw = 1;
    pdpt_table[PDPT_IDX(kernel_virt_base)].tab_addr = ((uint64_t)pd_table - kernel_offset) >> 12;

    pd_table[PD_IDX(kernel_virt_base)].p = 1;
    pd_table[PD_IDX(kernel_virt_base)].rw = 1;
    pd_table[PD_IDX(kernel_virt_base)].tab_addr = ((uint64_t)pt_table - kernel_offset) >> 12;

    // Map 512 4KiB kernel pages = 2MiB
    for (size_t i = 0; i < 512; i++){
        pt_table[PT_IDX(kernel_virt_base+i*4096)].p = 1;
        pt_table[PT_IDX(kernel_virt_base+i*4096)].rw = 1;
        pt_table[PT_IDX(kernel_virt_base+i*4096)].page_addr = (kernel_phys_base + i * 4096) >> 12; // Physical address of kernel
    }

    // Before we enable paging, we must register our page fault handler.
    disable_interrupts();
    interrupt_install_handler(14, &page_fault_handler);
    enable_interrupts();

    print("pml4_table address: ");
    print_hex( (uint64_t)pml4_table - kernel_offset);
    print("\n");
    load_cr3((uint64_t)pml4_table - kernel_offset);

    print("Paging Initialized.\n");
}



void page_fault_handler(registers_t *regs)
{
   // A page fault has occurred.
   // The faulting address is stored in the CR2 register.
   uint64_t faulting_address;
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
    print("Page fault! ( ");
    print("Error Code :");
    print_hex(id);
    print("\n");
    if (!present) {
        print("Thie page is present ");
    }else{
        print("The Page is not Prsent! ");
    }
    if (rw) {print(" read-only ");}
    if (us) {print(" user-mode ");}
    if (reserved) {print(" reserved ");}
    print_hex(faulting_address);
}

// Get Physicall address from virtual address
// Here We are using 4kb page transformation
// 48 bit

uint64_t get_phys_addr(uint64_t vir_addr, pml4_t* pml4_base){

    // Step 1 : Getting the indices of each level
    uint64_t pml4_index  = (uint64_t) (vir_addr >> 39) & 0x1FF; // 9 bit
    uint64_t pdpt_index  = (uint64_t) (vir_addr >> 30) & 0x1FF;
    uint64_t pd_index    = (uint64_t) (vir_addr >> 21) & 0x1FF;
    uint64_t pt_index    = (uint64_t) (vir_addr >> 12) & 0x1FF;
    uint64_t page_offset = (uint64_t) vir_addr & 0xFFF; // last 12 bit

    // Step 2 : Traverse PML4
    pml4_t pml4_entry = pml4_base[pml4_index];
    if(!(pml4_entry.p)){ // Check if pml4 entry is present
        return 0; // page is not present
    }

    // Step 3 : Traverse PDPR
    pdpt_t * pdpr_base = (pdpt_t *) ((uint64_t)pml4_entry.tab_addr << 12);  // To get the base address
    pdpt_t pdpt_entry = pdpr_base[pdpt_index];
    if (!(pdpt_entry.p)) {  // Check if the PDPT entry is present
        return 0;             // Page not present
    }

    // Step 4 : Traverse PD
    pd_t* pd_base = (pd_t*)((uint64_t)pdpt_entry.tab_addr << 12);
    pd_t pd_entry = pd_base[pd_index];
    if (!(pd_entry.p)) {  // Check if the PD entry is present
        return 0;           // Page not present
    }


    // Step 5: Traverse PT
    pt_t* pt_base = (pt_t*)((uint64_t)pd_entry.tab_addr << 12);
    pt_t pt_entry = (pt_t) pt_base[pt_index];
    if (!(pt_entry.p)) {  // Check if the PT entry is present
        return 0;           // Page not present
    }

    // Step 6: Get the physical address from the PT entry
    uint64_t physical_address = (pt_entry.page_addr << 12) | page_offset;
    return physical_address;
}


#define PAGE_MASK 0xFFFFFFFFFFFFF000

#define PAGE_SIZE 4096               // 4 KB pages
#define ENTRY_COUNT 512              // 512 entries per table
#define ENTRY_MASK 0x1FF             // Mask for each entry index (9 bits)

uint64_t phys_to_virt(uint64_t phys_addr, pml4_t* pml4_base) {
    // Iterate through PML4 entries
    for (uint64_t pml4_index = 0; pml4_index < ENTRY_COUNT; pml4_index++) {
        pml4_t pml4_entry = pml4_base[pml4_index];
        if (!pml4_entry.p) continue;  // Entry not present, skip

        pdpt_t* pdpt_base = (pdpt_t*)((uint64_t)pml4_entry.tab_addr << 12);

        // Iterate through PDPR entries
        for (uint64_t pdpt_index = 0; pdpt_index < ENTRY_COUNT; pdpt_index++) {
            pdpt_t pdpt_entry = pdpt_base[pdpt_index];
            if (!pdpt_entry.p) continue;

            pd_t* pd_base = (pd_t*)((uint64_t)pdpt_entry.tab_addr << 12);

            // Iterate through PD entries
            for (uint64_t pd_index = 0; pd_index < ENTRY_COUNT; pd_index++) {
                pd_t pd_entry = pd_base[pd_index];
                if (!pd_entry.p) continue;

                pt_t* pt_base = (pt_t*)((uint64_t)pd_entry.tab_addr << 12);

                // Iterate through PT entries
                for (uint64_t pt_index = 0; pt_index < ENTRY_COUNT; pt_index++) {
                    pt_t pt_entry = pt_base[pt_index];
                    if (!pt_entry.p) continue;

                    // Calculate physical frame address
                    uint64_t page_base_addr = pt_entry.page_addr << 12;
                    if ((phys_addr & PAGE_MASK) == page_base_addr) {
                        // Calculate virtual address
                        uint64_t page_offset = phys_addr & (PAGE_SIZE - 1);
                        uint64_t virt_addr = 
                            (pml4_index << 39) | 
                            (pdpt_index << 30) | 
                            (pd_index << 21) | 
                            (pt_index << 12) | 
                            page_offset;
                        return virt_addr;
                    }
                }
            }
        }
    }
    // Return 0 if the physical address is not mapped to any virtual address
    return 0;
}



void test_paging(){
    uint64_t* test_address = (uint64_t*)0xFFFFFFFF80000000;  // Higher-half virtual address
    *test_address = 0x12345678ABCDEF00;

    uint64_t* invalid_address = (uint64_t*)0xFFFFFFFF90000000;  // Unmapped address
    *invalid_address = 0x0;  // This should trigger a page fault
}

