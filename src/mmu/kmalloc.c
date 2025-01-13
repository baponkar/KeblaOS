/*
This file will manage static memory allocation
*/

#include "kmalloc.h"

extern uint64_t PHYSICAL_TO_VIRTUAL_OFFSET;
extern uint64_t KERNEL_MEM_START_ADDRESS;



// Low level memory allocation by usin base as KERNEL_MEM_START_ADDRESS
uint64_t kmalloc(uint64_t sz)       // vanilla (normal).
{
    uint64_t tmp = (uint64_t) KERNEL_MEM_START_ADDRESS; // memory allocate in current placement address
    KERNEL_MEM_START_ADDRESS += sz;    // increase the placement address for next memory allocation
    return tmp;
}

/*
    Page Alignment: Page alignment ensures that memory addresses fall on boundaries that are 
    multiples of the page size (typically 4 KB for 64-bit systems).
*/

uint64_t kmalloc_a(uint64_t sz, int align)    // page aligned.
{
    /*
    page directory and page table addresses need to be page-aligned: that is, the bottom 12 
    bits need to be zero (otherwise they would interfere with the read/write/protection/accessed bits).
    */
    if (align == 1 && (KERNEL_MEM_START_ADDRESS & 0xFFF)) // If the address is not already page-aligned i.e. multiple of PAGE_SIZE
    {
        // Align it.
        KERNEL_MEM_START_ADDRESS &= 0xFFFFFFFFFFFFF000; // masking of most significant 20 bit which is used for address
        KERNEL_MEM_START_ADDRESS += FRAME_SIZE;    // increase  the placement address by 4 KB, Page Size
    }
    
    uint64_t tmp = KERNEL_MEM_START_ADDRESS;
    KERNEL_MEM_START_ADDRESS += sz;

    return tmp;
}

/*
The kmalloc_p function is used to allocate memory while also optionally returning the physical address 
of the allocated memory.
*/
uint64_t kmalloc_p(uint64_t sz, uint64_t *phys){
    if (phys)
    {   // phys (parameter): This is a pointer to a uint64_t variable where 
        // the physical address of the allocated memory will be stored.
        *phys = KERNEL_MEM_START_ADDRESS;
    }
    uint64_t tmp = KERNEL_MEM_START_ADDRESS;
    KERNEL_MEM_START_ADDRESS += sz;
    return tmp;
}

/*
The kmalloc_ap function is used to allocate memory , returning the physical address,  
and also ensure page boundary alignment
*/
uint64_t kmalloc_ap(uint64_t sz, int align, uint64_t *phys)  // page aligned and returns a physical address.
{
    if (align == 1 && (KERNEL_MEM_START_ADDRESS & 0xFFF)) // If the address is not already page-aligned and want to make it page aligned
    {
        // Align it.
        KERNEL_MEM_START_ADDRESS &= 0xFFFFFFFFFFFFF000;
        KERNEL_MEM_START_ADDRESS += PAGE_SIZE;    // increase  the placement address by 4 KB, Page Size
    }
    if (phys)
    {
        *phys = KERNEL_MEM_START_ADDRESS;
    }
    uint64_t tmp = KERNEL_MEM_START_ADDRESS;
    KERNEL_MEM_START_ADDRESS += sz;
    return tmp;
}

