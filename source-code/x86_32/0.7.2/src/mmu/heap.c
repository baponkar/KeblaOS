
#include "heap.h"


// Define the block structure for memory management
typedef struct mem_block {
    size_t size;                // Size of the memory block
    int free;                   // 1 if the block is free, 0 if it's allocated
    struct mem_block *next;     // Pointer to the next memory block
} mem_block_t;

// Alignment for memory blocks (optional, depending on architecture)
#define ALIGN4(size) (((size) + 3) & ~3)

// Define the memory pool size (adjust as needed)
#define MEMORY_POOL_SIZE 1024 * 1024  // 1 MB

// Global memory pool
static char memory_pool[MEMORY_POOL_SIZE];

// Pointer to the start of the memory list
static mem_block_t *free_list = (void *)memory_pool;

// Initialize memory pool (called once during OS boot)
void init_memory() {
    free_list->size = MEMORY_POOL_SIZE - sizeof(mem_block_t);
    free_list->free = 1;
    free_list->next = NULL;
}

// Split a block into two smaller blocks
void split_block(mem_block_t *block, size_t size) {
    mem_block_t *new_block = (void *)((char *)block + size + sizeof(mem_block_t));
    new_block->size = block->size - size - sizeof(mem_block_t);
    new_block->free = 1;
    new_block->next = block->next;

    block->size = size;
    block->free = 0;
    block->next = new_block;
}

// Find a free block of memory
mem_block_t *find_free_block(size_t size) {
    mem_block_t *current = free_list;
    while (current != NULL) {
        if (current->free && current->size >= size) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// Malloc implementation
void *kmalloc(size_t size) {
    if (size == 0) {
        return NULL;
    }

    size = ALIGN4(size);  // Align size to 4 bytes

    mem_block_t *block = find_free_block(size);
    if (block != NULL) {
        if (block->size > size + sizeof(mem_block_t)) {
            split_block(block, size);
        }
        block->free = 0;
        return (void *)(block + 1);  // Return pointer to memory, after the header
    }
    return NULL;  // No suitable block found
}

// Free implementation
void kfree(void *ptr) {
    if (ptr == NULL) {
        return;
    }

    mem_block_t *block = (mem_block_t *)ptr - 1;
    block->free = 1;
    
    // Coalesce adjacent free blocks (optional, for efficiency)
    mem_block_t *current = free_list;
    while (current != NULL && current->next != NULL) {
        if (current->free && current->next->free) {
            current->size += current->next->size + sizeof(mem_block_t);
            current->next = current->next->next;
        }
        current = current->next;
    }
}

// Calloc implementation
void *kcalloc(size_t num, size_t size) {
    size_t total_size = num * size;
    void *ptr = kmalloc(total_size);
    if (ptr) {
        memset(ptr, 0, total_size);
    }
    return ptr;
}

// Realloc implementation
void *krealloc(void *ptr, size_t size) {
    if (ptr == NULL) {
        return kmalloc(size);  // realloc(NULL, size) is equivalent to malloc(size)
    }
    if (size == 0) {
        kfree(ptr);  // realloc(ptr, 0) is equivalent to free(ptr)
        return NULL;
    }

    mem_block_t *block = (mem_block_t *)ptr - 1;
    if (block->size >= size) {
        return ptr;  // If the current block is large enough, return the same pointer
    }

    // Allocate a new block and copy the data
    void *new_ptr = kmalloc(size);
    if (new_ptr) {
        memcpy(new_ptr, ptr, block->size);
        kfree(ptr);
    }
    return new_ptr;
}
