#include "kheap.h"


extern uint64_t placement_address; // The value of it will set in kernel.c 

// Low level memory allocation by usin base as placement_address
uint64_t kmalloc(uint64_t sz)       // vanilla (normal).
{
    uint64_t tmp = placement_address; // memory allocate in current placement address
    placement_address += sz;    // increase the placement address for next memory allocation
    return tmp;
}


/*
Page Alignment: Page alignment ensures that memory addresses fall on boundaries that are 
multiples of the page size (typically 4 KB for 64-bit systems).
*/

uint64_t kmalloc_a(uint64_t sz, int align)    // page aligned.
{
    // 0xFFFFF000 = 1111 1111 1111 1111 1111 0000 0000 0000
    /*
     page directory and page table addresses need to be page-aligned: that is, the bottom 12 
     bits need to be zero (otherwise they would interfere with the read/write/protection/accessed bits).
    */
    if (align == 1 && (placement_address & 0xFFFFFFFFFFFFF000)) // If the address is not already page-aligned
    {
        // Align it.
        placement_address &= 0xFFFFFFFFFFFFF000; // masking of most significant 20 bit which is used for address
        placement_address += 0x1000;    // increase  the placement address by 4 KB, Page Size
    }
    uint64_t tmp = placement_address;
    placement_address += sz;    //increments placement_address by sz, the size of the requested allocation, 
                                // to update the placement address for the next allocation.
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
        *phys = placement_address;
    }
    uint64_t tmp = placement_address;
    placement_address += sz;
    return tmp;
}

/*
The kmalloc_ap function is used to allocate memory , returning the physical address,  
and also ensure page boundary alignment
*/
uint64_t kmalloc_ap(uint64_t sz,int align, uint64_t *phys)  // page aligned and returns a physical address.
{
    if (align == 1 && (placement_address & 0xFFFFFFFFFFFFF000)) // If the address is not already page-aligned
    {
        // Align it.
        placement_address &= 0xFFFFFFFFFFFFF000;
        placement_address += 0x1000;    // increase  the placement address by 4 KB, Page Size
    }
    if (phys)
    {
        *phys = placement_address;
    }
    uint64_t tmp = placement_address;
    placement_address += sz;
    return tmp;
}

