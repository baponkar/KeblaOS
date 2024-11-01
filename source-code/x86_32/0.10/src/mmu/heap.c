#include "heap.h"

extern uint32_t placement_address;
uint32_t placement_address = 0x1000000; // 16MB, Initialize to a reasonable starting address in memory.


uint32_t kmalloc(uint32_t sz)       // vanilla (normal).
{
    uint32_t tmp = placement_address; // memory allocate in current placement address
    placement_address += sz;    // increase the placement address for next memory allocation
    return tmp;
}

/*
Page Alignment: Page alignment ensures that memory addresses fall on boundaries that are 
multiples of the page size (typically 4 KB for 32-bit systems).
*/

uint32_t kmalloc_a(uint32_t sz, int align)    // page aligned.
{
    if (align == 1 && (placement_address & 0xFFFFF000)) // If the address is not already page-aligned
    {
        // Align it.
        placement_address &= 0xFFFFF000; // masking of most significant 20 bit which is used for address
        placement_address += 0x1000;    // increase  the placement address by 4 KB, Page Size
    }
    uint32_t tmp = placement_address;
    placement_address += sz;    //increments placement_address by sz, the size of the requested allocation, 
                                // to update the placement address for the next allocation.
    return tmp;
}

/*
The kmalloc_p function is used to allocate memory while also optionally returning the physical address 
of the allocated memory.
*/
uint32_t kmalloc_p(uint32_t sz, uint32_t *phys){
    if (phys)
    {   // phys (parameter): This is a pointer to a uint32_t variable where 
        // the physical address of the allocated memory will be stored.
        *phys = placement_address;
    }
    uint32_t tmp = placement_address;
    placement_address += sz;
    print("Physical address: ");
    print_hex(*phys);
    print("\n");
    return tmp;
}

/*
The kmalloc_ap function is used to allocate memory , returning the physical address,  
and also ensure page boundary alignment
*/
uint32_t kmalloc_ap(uint32_t sz,int align, uint32_t *phys)  // page aligned and returns a physical address.
{
    if (align == 1 && (placement_address & 0xFFFFF000)) // If the address is not already page-aligned
    {
        // Align it.
        placement_address &= 0xFFFFF000;
        placement_address += 0x1000;    // increase  the placement address by 4 KB, Page Size
    }
    if (phys)
    {
        *phys = placement_address;
    }
    uint32_t tmp = placement_address;
    placement_address += sz;
    return tmp;
}

