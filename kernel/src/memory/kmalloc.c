/*
This file will manage static memory allocation
*/

#include "../lib/stdio.h"
#include "detect_memory.h"

#include "kmalloc.h"

#define FRAME_SIZE 4096

extern volatile uint64_t phys_mem_head;


// Low level memory allocation by usin base as phys_mem_head
uint64_t kmalloc(uint64_t sz)       // vanilla (normal).
{
    if(phys_mem_head >= USABLE_END_PHYS_MEM) return 0;
    uint64_t ptr = (uint64_t) phys_mem_head;    // memory allocate in current placement address
    phys_mem_head += sz;                        // increase the placement address for next memory allocation

    return ptr;
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


    if(phys_mem_head >= USABLE_END_PHYS_MEM) {
        printf("kmalloc_a: Out of memory\n");
        return 0;
    }

    if (align == 1 && (phys_mem_head & 0xFFF)) // If the address is not already page-aligned i.e. multiple of PAGE_SIZE
    {
        // Align it.
        phys_mem_head &= 0xFFFFFFFFFFFFF000; // masking of most significant 20 bit which is used for address
        phys_mem_head += FRAME_SIZE;    // increase  the placement address by 4 KB, Page Size
    }
    
    uint64_t ptr = phys_mem_head;
    phys_mem_head += sz;

    return ptr;
}

/*
The kmalloc_p function is used to allocate memory while also optionally returning the physical address 
of the allocated memory.
*/
uint64_t kmalloc_p(uint64_t sz, uint64_t *phys){
    if (phys)
    {   // phys (parameter): This is a pointer to a uint64_t variable where 
        // the physical address of the allocated memory will be stored.
        *phys = phys_mem_head;
    }
    uint64_t ptr = phys_mem_head;
    ptr += sz;
    return ptr;
}

/*
The kmalloc_ap function is used to allocate memory , returning the physical address,  
and also ensure page boundary alignment
*/
uint64_t kmalloc_ap(uint64_t sz, int align, uint64_t *phys)  // page aligned and returns a physical address.
{
    if (align == 1 && (phys_mem_head & 0xFFF)) // If the address is not already page-aligned and want to make it page aligned
    {
        // Align it.
        phys_mem_head &= 0xFFFFFFFFFFFFF000;
        phys_mem_head += FRAME_SIZE;    // increase  the placement address by 4 KB, Page Size
    }

    if(phys_mem_head >= USABLE_END_PHYS_MEM) return 0;

    if (phys)
    {
        *phys = phys_mem_head;
    }
    uint64_t tmp = phys_mem_head;
    phys_mem_head += sz;
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
    if (phys_mem_head >= USABLE_END_PHYS_MEM){
         return 0;
    }

    // Align the phys_mem_head if necessary.
    if (phys_mem_head & (alignment - 1)) {
        phys_mem_head = (phys_mem_head + alignment - 1) & ~(alignment - 1);
    }
    
    uint64_t ptr = phys_mem_head;
    phys_mem_head += sz;
    
    return ptr;
}

void test_kmalloc(){
    printf("Test of kmalloc\n");

    uint64_t *ptr1 = (uint64_t *) kmalloc(64);
    printf("ptr1 : %x\n", ptr1);

    *ptr1 = 0x1234;
    printf("ptr1:%x, *ptr1 = %x\n", (uint64_t)ptr1, (uint64_t)*ptr1);

    uint64_t *ptr2 = (uint64_t *) kmalloc_a(43, 1);
    printf("ptr2 : %x\n", ptr2);

    uint64_t *ptr3 = (uint64_t *) kmalloc_p(26, (void *) ptr2);
    printf("ptr3 : %x\n", (uint64_t)ptr3);

    uint64_t *ptr4 = (uint64_t *) kmalloc_ap(23, 1, (void *)ptr1);
    printf("ptr4 : %x\n", ptr4);
}

