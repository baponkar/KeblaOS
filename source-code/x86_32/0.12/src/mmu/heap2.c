

#include "heap2.h"



// extern void *heap_start;  // The starting address of your heap
// extern size_t heap_size;  // The size of the heap
// extern void *free_list;   // A linked list of free memory blocks (if you're implementing a basic memory allocator)

// External variables representing the heap and free list
void *heap_start = (void *)0x100000;  // Example heap starting address (e.g., 1 MB)
size_t heap_size = 0x100000;          // Example heap size (e.g., 1 MB)
void *free_list = NULL;               // Initially, no free blocks

typedef struct free_block {
    size_t size;
    struct free_block *next;
} free_block_t;

void *heap_end = NULL;  // Pointer to the end of the heap (for managing allocations)

void kfree(void *ptr) {
    if (ptr == NULL) {
        return;  // Can't free a NULL pointer
    }

    // Convert the pointer to a free block and insert it back into the free list
    free_block_t *block = (free_block_t *) ptr;
    block->size = sizeof(free_block_t);  // Set size for the block (you could enhance this with better tracking)
    block->next = (free_block_t *) free_list;
    
    // Update the free list head to point to the newly freed block
    free_list = block;
}



