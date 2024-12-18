/*
https://github.com/dreamportdev/Osdev-Notes/blob/master/04_Memory_Management/04_Virtual_Memory_Manager.md
https://chatgpt.com/share/675fa60d-c044-8001-aef6-d23b3d62ab62
*/

#include "vmm.h"


extern pml4_t *user_pml4;
extern pml4_t *kernel_pml4;

void map_page(uint64_t virt_addr, uint64_t phys_addr, uint64_t flags, pml4_t *pml4) {
    uint64_t pml4_index = PML4_INDEX(virt_addr);
    uint64_t pdpt_index = PDPT_INDEX(virt_addr);
    uint64_t pd_index = PD_INDEX(virt_addr);
    uint64_t pt_index = PT_INDEX(virt_addr);

    dir_entry_t *pdpt, *pd, *pt;

    // Allocate or get the PDPT
    if (!(pml4->entry_t[pml4_index].present)) {
        pdpt = (dir_entry_t *)kmalloc_a(sizeof(pdpt_t), PAGE_SIZE);
        memset(pdpt, 0, sizeof(pdpt_t));
        pml4->entry_t[pml4_index].base_addr = ((uint64_t)pdpt) >> 12;
        pml4->entry_t[pml4_index].present = 1;
        pml4->entry_t[pml4_index].rw = 1;
    } else {
        pdpt = (dir_entry_t *)(pml4->entry_t[pml4_index].base_addr << 12);
    }

    // Allocate or get the PD
    if (!(pdpt[pdpt_index].present)) {
        pd = (dir_entry_t *)kmalloc_a(sizeof(pd_t), PAGE_SIZE);
        memset(pd, 0, sizeof(pd_t));
        pdpt[pdpt_index].base_addr = ((uint64_t)pd) >> 12;
        pdpt[pdpt_index].present = 1;
        pdpt[pdpt_index].rw = 1;
    } else {
        pd = (dir_entry_t *)(pdpt[pdpt_index].base_addr << 12);
    }

    // Allocate or get the PT
    if (!(pd[pd_index].present)) {
        pt = (dir_entry_t *)kmalloc_a(sizeof(pt_t), PAGE_SIZE);
        memset(pt, 0, sizeof(pt_t));
        pd[pd_index].base_addr = ((uint64_t)pt) >> 12;
        pd[pd_index].present = 1;
        pd[pd_index].rw = 1;
    } else {
        pt = (dir_entry_t *)(pd[pd_index].base_addr << 12);
    }

    // Map the physical address to the page table entry
    pt[pt_index].base_addr = phys_addr >> 12;
    pt[pt_index].present = (flags & PAGE_PRESENT) ? 1 : 0;
    pt[pt_index].rw = (flags & PAGE_WRITE) ? 1 : 0;
    pt[pt_index].user = (flags & PAGE_USER) ? 1 : 0;
}


void unmap_page(uint64_t virt_addr, pml4_t *pml4) {
    uint64_t pml4_index = PML4_INDEX(virt_addr);
    uint64_t pdpt_index = PDPT_INDEX(virt_addr);
    uint64_t pd_index = PD_INDEX(virt_addr);
    uint64_t pt_index = PT_INDEX(virt_addr);

    dir_entry_t *pdpt, *pd, *pt;

    // Walk through the hierarchy to the PT
    if (!pml4->entry_t[pml4_index].present) return;
    pdpt = (dir_entry_t *)(pml4->entry_t[pml4_index].base_addr << 12);

    if (!pdpt[pdpt_index].present) return;
    pd = (dir_entry_t *)(pdpt[pdpt_index].base_addr << 12);

    if (!pd[pd_index].present) return;
    pt = (dir_entry_t *)(pd[pd_index].base_addr << 12);

    if (!pt[pt_index].present) return;

    // Unmap the page
    memset(&pt[pt_index], 0, sizeof(dir_entry_t));

    // Flush TLB for the unmapped page
    asm volatile ("invlpg (%0)" : : "r" (virt_addr) : "memory");
}


uint64_t virtual_memory_start = 0xFFFF800000000000; // Kernel heap area

uint64_t vmm_alloc(uint64_t size, uint64_t flags) {
    uint64_t virt_addr = virtual_memory_start;
    virtual_memory_start += size;

    for (uint64_t i = 0; i < size; i += PAGE_SIZE) {
        uint64_t phys_addr = first_frame() * PAGE_SIZE; // Allocate a physical frame
        set_frame(phys_addr);                          // Mark the frame as used
        map_page(virt_addr + i, phys_addr, flags, kernel_pml4);
    }

    return virt_addr;
}

void vmm_free(uint64_t virt_addr, uint64_t size) {
    for (uint64_t i = 0; i < size; i += PAGE_SIZE) {
        unmap_page(virt_addr + i, kernel_pml4);
        free_frame((page_t *)((virt_addr + i) & ~(PAGE_SIZE - 1))); // Free physical frame
    }
}


void page_fault_handler1(registers_t *regs) {
    uint64_t faulting_address;
    asm volatile ("mov %%cr2, %0" : "=r" (faulting_address));

    uint64_t error_code = regs->err_code;

    if (!(error_code & PAGE_PRESENT)) {
        // Page not present - Allocate a new page
        map_page(faulting_address, first_frame() * PAGE_SIZE, PAGE_PRESENT | PAGE_WRITE, kernel_pml4);
        set_frame(faulting_address);
    } else {
        // Invalid access
        printf("Page fault at address: %p\n", faulting_address);
        printf("Error code: %x\n", error_code);
        // Terminate or handle appropriately
    }
}

void test_vmm() {
    uint64_t virt_addr1 = vmm_alloc(2 * PAGE_SIZE, PAGE_PRESENT | PAGE_WRITE);
    printf("Allocated virtual memory: %p\n", virt_addr1);

    vmm_free(virt_addr1, 2 * PAGE_SIZE);
    printf("Freed virtual memory.\n");
}


