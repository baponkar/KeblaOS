/*
This file will manage static memory allocation
*/

#include "../lib/stdio.h"

#include "kmalloc.h"

#define FRAME_SIZE 4096

extern uint64_t KMEM_UP_BASE;
extern uint64_t KMEM_LOW_BASE;


// Low level memory allocation by usin base as KMEM_UP_BASE
uint64_t kmalloc(uint64_t sz)       // vanilla (normal).
{
    if(KMEM_LOW_BASE >= KMEM_UP_BASE) return 0;
    uint64_t tmp = (uint64_t) KMEM_LOW_BASE; // memory allocate in current placement address
    KMEM_LOW_BASE += sz;    // increase the placement address for next memory allocation
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
    if(KMEM_LOW_BASE >= KMEM_UP_BASE) return 0;
    if (align == 1 && (KMEM_LOW_BASE & 0xFFF)) // If the address is not already page-aligned i.e. multiple of PAGE_SIZE
    {
        // Align it.
        KMEM_LOW_BASE &= 0xFFFFFFFFFFFFF000; // masking of most significant 20 bit which is used for address
        KMEM_LOW_BASE += FRAME_SIZE;    // increase  the placement address by 4 KB, Page Size
    }
    
    uint64_t tmp = KMEM_LOW_BASE;
    KMEM_LOW_BASE += sz;

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
        *phys = KMEM_LOW_BASE;
    }
    uint64_t tmp = KMEM_LOW_BASE;
    KMEM_UP_BASE += sz;
    return tmp;
}

/*
The kmalloc_ap function is used to allocate memory , returning the physical address,  
and also ensure page boundary alignment
*/
uint64_t kmalloc_ap(uint64_t sz, int align, uint64_t *phys)  // page aligned and returns a physical address.
{
    if (align == 1 && (KMEM_LOW_BASE & 0xFFF)) // If the address is not already page-aligned and want to make it page aligned
    {
        // Align it.
        KMEM_LOW_BASE &= 0xFFFFFFFFFFFFF000;
        KMEM_LOW_BASE += FRAME_SIZE;    // increase  the placement address by 4 KB, Page Size
    }
    if(KMEM_LOW_BASE >= KMEM_UP_BASE) return 0;
    if (phys)
    {
        *phys = KMEM_LOW_BASE;
    }
    uint64_t tmp = KMEM_LOW_BASE;
    KMEM_LOW_BASE += sz;
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

