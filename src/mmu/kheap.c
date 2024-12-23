
#include "kheap.h"


extern uint64_t placement_address; // The value of it will set in kernel.c 

// Low level memory allocation by usin base as placement_address
uint64_t kmalloc(uint64_t sz)       // vanilla (normal).
{
    uint64_t tmp = (uint64_t) placement_address; // memory allocate in current placement address
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
    if (align == 1 && (placement_address & 0xFFF)) // If the address is not already page-aligned i.e. multiple of 0x1000
    {
        // Align it.
        placement_address &= 0xFFFFFFFFFFFFF000; // masking of most significant 20 bit which is used for address
        placement_address += 0x1000;    // increase  the placement address by 4 KB, Page Size
    }
    
    uint64_t tmp = placement_address; // asign tmp with increased placement_address
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
uint64_t kmalloc_ap(uint64_t sz, int align, uint64_t *phys)  // page aligned and returns a physical address.
{
    if (align == 1 && (placement_address & 0xFFF)) // If the address is not already page-aligned and want to make it page aligned
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


/*
* Dynamic Memory Allocation
* https://chatgpt.com/share/675fa60d-c044-8001-aef6-d23b3d62ab62
*/

#define KHEAP_START 0xFFFF800000000000 // Example kernel heap start address
#define KHEAP_SIZE  0x100000           // 1 MB heap size for example

void *heap_start = (void *) KHEAP_START;
void *heap_end = (void *) (KHEAP_START + KHEAP_SIZE);
heap_block_t *heap_head;

void init_kheap() {
    heap_head = (heap_block_t *)heap_start;
    heap_head->size = KHEAP_SIZE - HEAP_BLOCK_SIZE;
    heap_head->is_free = 1;
    heap_head->next = NULL;
}


void *kmalloc1(uint64_t size) {
    heap_block_t *current = heap_head;
    while (current) {
        if (current->is_free && current->size >= size) {
            // Split the block if there's enough space for a new block
            if (current->size > size + HEAP_BLOCK_SIZE) {
                heap_block_t *new_block = (heap_block_t *)((uint8_t *) current + HEAP_BLOCK_SIZE + size);
                new_block->size = current->size - size - HEAP_BLOCK_SIZE;
                new_block->is_free = 1;
                new_block->next = current->next;

                current->next = new_block;
                current->size = size;
            }

            current->is_free = 0;
            return (void *)((uint8_t *)current + HEAP_BLOCK_SIZE); // Return pointer to usable memory
        }
        current = current->next;
    }

    // No suitable block found
    return NULL;
}



void kfree1(void *ptr) {
    if (!ptr) return;

    // Retrieve the block metadata
    heap_block_t *block = (heap_block_t *)((uint8_t *)ptr - HEAP_BLOCK_SIZE);
    block->is_free = 1;

    // Merge adjacent free blocks
    heap_block_t *current = heap_head;
    while (current) {
        if (current->is_free && current->next && current->next->is_free) {
            current->size += current->next->size + HEAP_BLOCK_SIZE;
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
}



void *kmalloc1_a(uint64_t size, int align) {
    heap_block_t *current = heap_head;
    while (current) {
        if (current->is_free && current->size >= size) {
            // Calculate the aligned address
            uint64_t raw_address = (uint64_t)((uint8_t *)current + HEAP_BLOCK_SIZE);
            uint64_t aligned_address = raw_address;
            if (align == 1 && (raw_address & 0xFFF)) // If the address is not already page-aligned and want to make it page aligned
            {
                // align it
                aligned_address &= 0xFFFFFFFFFFFFF000; // masking of most significant 20 bit which is used for address
                aligned_address += 0x1000; // increase  the raw address by 4 KB, Page Size
            }

            // Split the block if there's enough space for a new block
            if (current->size > size + (aligned_address - raw_address) + HEAP_BLOCK_SIZE) {
                heap_block_t *new_block = (heap_block_t *)((uint8_t *) current + HEAP_BLOCK_SIZE + size);
                new_block->size = current->size - size - (aligned_address - raw_address) - HEAP_BLOCK_SIZE;
                new_block->is_free = 1;
                new_block->next = current->next;

                current->next = new_block;
                current->size = size + (aligned_address - raw_address);
            }

            current->is_free = 0;
            return (void *) aligned_address;
        }
        current = current->next;
    }

    // No suitable block found
    return NULL;
}


void *kmalloc1_p(uint64_t size, uint64_t *phys){
    heap_block_t *current = heap_head;
    while (current) {
        if (current->is_free && current->size >= size) {
            // Split the block if there's enough space for a new block
            if (current->size > size + HEAP_BLOCK_SIZE) {
                heap_block_t *new_block = (heap_block_t *)((uint8_t *) current + HEAP_BLOCK_SIZE + size);
                new_block->size = current->size - size - HEAP_BLOCK_SIZE;
                new_block->is_free = 1;
                new_block->next = current->next;

                current->next = new_block;
                current->size = size;
            }

            current->is_free = 0;
            *phys = (uint64_t)((uint8_t *)current + HEAP_BLOCK_SIZE);

        }
        current = current->next;
    }

}


void *kmalloc1_ap(uint64_t size, int align, uint64_t *phys){
    heap_block_t *current = heap_head;
    while (current) {
        if (current->is_free && current->size >= size) {
            // Calculate the aligned address
            uint64_t raw_address = (uint64_t)((uint8_t *)current + HEAP_BLOCK_SIZE);
            uint64_t aligned_address = raw_address;
            if (align == 1 && (raw_address & 0xFFF)) // If the address is not already page-aligned and want to make it page aligned
            {
                // align it
                aligned_address &= 0xFFFFFFFFFFFFF000; // masking of most significant 20 bit which is used for address
                aligned_address += 0x1000; // increase  the raw address by 4 KB, Page Size
            }

            // Split the block if there's enough space for a new block
            if (current->size > size + (aligned_address - raw_address) + HEAP_BLOCK_SIZE) {
                heap_block_t *new_block = (heap_block_t *)((uint8_t *) current + HEAP_BLOCK_SIZE + size);
                new_block->size = current->size - size - (aligned_address - raw_address) - HEAP_BLOCK_SIZE;
                new_block->is_free = 1;
                new_block->next = current->next;

                current->next = new_block;
                current->size = size + (aligned_address - raw_address);
            }

            current->is_free = 0;
            *phys = (uint64_t) aligned_address;
        }
        current = current->next;
    }
}


size_t get_allocated_size(void *ptr) {
    if (!ptr) return 0;

    // Retrieve the heap block metadata
    heap_block_t *block = (heap_block_t *)((uint8_t *)ptr - HEAP_BLOCK_SIZE);

    // Return the size of the allocated block
    return block->size;
}



void test_kheap() {
    print("Dynamic Memory allocation\n");
    init_kheap();
    void *ptr1 = kmalloc1(100);

    print("*ptr1 : ");
    print_dec((uint64_t)ptr1);
    print("\n");

    print("allocation sizeof ptr1 : ");
    print_dec(get_allocated_size(ptr1));
    print("\n");

    void *ptr2 = kmalloc1(200);

    print("*ptr2 : ");
    print_dec((uint64_t)ptr2);
    print("\n");

    kfree1(ptr1);
    print("kfree(ptr1) applied!\n");

    void *ptr3 = kmalloc1(50); // Should reuse the space from ptr1

    print("*ptr3 : ");
    print_dec((uint64_t)ptr3);
    print("\n");

    kfree1(ptr2);

    print("kfree(ptr2)\n");

    kfree1(ptr3);

    print("kfree(ptr3) applied!\n");

    void *ptr4 = kmalloc1_a(100, 1); // Page-aligned allocation

    print("*ptr4 : ");

    print_dec((uint64_t)ptr4);

    print("\n");
    
    print("allocation sizeof ptr4 : ");

    print_dec(get_allocated_size(ptr4));

    print("\n");

    void *ptr5 = kmalloc1_p(100, (uint64_t *)0x1000); // Physical address allocation

    print("*ptr5 : ");

    print_dec((uint64_t)ptr5);

    print("\n");

    void *ptr6 = kmalloc1_ap(100, 1, (uint64_t *)0x2000); // Page-aligned and physical address allocation

    print("*ptr6 : ");

    print_dec((uint64_t)ptr6);

    print("\n");

    print("allocation sizeof ptr6 : ");

    print_dec(get_allocated_size(ptr6));

    print("\n");

    print("Dynamic Memory allocation test completed!\n");
}















