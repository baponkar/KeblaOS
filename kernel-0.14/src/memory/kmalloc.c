/*
This file will manage static memory allocation
*/

#include "../lib/stdio.h"
#include "detect_memory.h"

#include "kmalloc.h"

#define FRAME_SIZE 4096




// Low level memory allocation by usin base as USABLE_END_PHYS_MEM
uint64_t kmalloc(uint64_t sz)       // vanilla (normal).
{
    if(USABLE_START_PHYS_MEM >= USABLE_END_PHYS_MEM) return 0;
    uint64_t tmp = (uint64_t) USABLE_START_PHYS_MEM; // memory allocate in current placement address
    USABLE_START_PHYS_MEM += sz;    // increase the placement address for next memory allocation

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
    if(USABLE_START_PHYS_MEM >= USABLE_END_PHYS_MEM) return 0;
    if (align == 1 && (USABLE_START_PHYS_MEM & 0xFFF)) // If the address is not already page-aligned i.e. multiple of PAGE_SIZE
    {
        // Align it.
        USABLE_START_PHYS_MEM &= 0xFFFFFFFFFFFFF000; // masking of most significant 20 bit which is used for address
        USABLE_START_PHYS_MEM += FRAME_SIZE;    // increase  the placement address by 4 KB, Page Size
    }
    
    uint64_t tmp = USABLE_START_PHYS_MEM;
    USABLE_START_PHYS_MEM += sz;

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
        *phys = USABLE_START_PHYS_MEM;
    }
    uint64_t tmp = USABLE_START_PHYS_MEM;
    USABLE_END_PHYS_MEM += sz;
    return tmp;
}

/*
The kmalloc_ap function is used to allocate memory , returning the physical address,  
and also ensure page boundary alignment
*/
uint64_t kmalloc_ap(uint64_t sz, int align, uint64_t *phys)  // page aligned and returns a physical address.
{
    if (align == 1 && (USABLE_START_PHYS_MEM & 0xFFF)) // If the address is not already page-aligned and want to make it page aligned
    {
        // Align it.
        USABLE_START_PHYS_MEM &= 0xFFFFFFFFFFFFF000;
        USABLE_START_PHYS_MEM += FRAME_SIZE;    // increase  the placement address by 4 KB, Page Size
    }
    if(USABLE_START_PHYS_MEM >= USABLE_END_PHYS_MEM) return 0;
    if (phys)
    {
        *phys = USABLE_START_PHYS_MEM;
    }
    uint64_t tmp = USABLE_START_PHYS_MEM;
    USABLE_START_PHYS_MEM += sz;
    return tmp;
}

bool check_mem_alloc(void *ptr, uint64_t size){

    uintptr_t addr = (uintptr_t) ptr;
    if (addr == NULL) {
        // printf("Null pointer\n");
        return 0;
    }

    if ((addr & (size - 1)) == 0) {
        // printf("%x pointer is %d-byte aligned.\n", (uint64_t) ptr, (int) size);
        return 1;
    } 
    // Buffer is not properly aligned.
    uint64_t alignment = 1; // Start with 1-byte alignment

    // Count trailing zeros
    while ((addr & 1) == 0) {
        alignment *= 2;
        addr >>= 1;
    }
    // printf("%x pointer is not %d-byte aligned.\n", (uint64_t) ptr, (int) size);
    // printf("Pointer %x is aligned to %d bytes.\n", (uint64_t) ptr, alignment);  
    return 0;  
}

uint64_t kmalloc_aligned(uint64_t sz, uint64_t alignment) {
    if (USABLE_START_PHYS_MEM >= USABLE_END_PHYS_MEM) return 0;

    // Align the USABLE_START_PHYS_MEM if necessary.
    if (USABLE_START_PHYS_MEM & (alignment - 1)) {
        USABLE_START_PHYS_MEM = (USABLE_START_PHYS_MEM + alignment - 1) & ~(alignment - 1);
    }
    
    uint64_t tmp = USABLE_START_PHYS_MEM;
    USABLE_START_PHYS_MEM += sz;
    
    return tmp;
}

void test_kmalloc(){
    printf("Test of kmalloc\n");

    uint64_t ptr1 = kmalloc(64);
    printf("ptr1 : %x\n", ptr1);

    uint64_t ptr2 = kmalloc_a(43, 1);
    printf("ptr2 : %x\n", ptr2);

    uint64_t ptr3 = kmalloc_p(26,&ptr2);
    printf("ptr3 : %x\n", (uint64_t)ptr3);

    uint64_t ptr4 = kmalloc_ap(23, 1, &ptr1);
    printf("ptr4 : %x\n", ptr4);
}

