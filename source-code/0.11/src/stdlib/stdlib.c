#include "stdlib.h"



#define MEMORY_POOL_SIZE 4096 // Adjust this size as needed
static uint8_t memory_pool[MEMORY_POOL_SIZE];
static size_t allocated_size = 0;

// Block header for keeping track of allocated blocks
typedef struct Block {
    size_t size;
    struct Block* next;
} Block;

static Block* free_list = NULL;

// Initialize memory pool
void stdlib_init() {
    free_list = (Block*)memory_pool;
    free_list->size = MEMORY_POOL_SIZE - sizeof(Block);
    free_list->next = NULL;
}

// Memory allocation function
void* malloc(size_t size) {
    if (size == 0) {
        return NULL;
    }

    size += sizeof(Block); // Include header size
    Block* current = free_list;
    Block* previous = NULL;

    // Find a suitable block
    while (current != NULL) {
        if (current->size >= size) {
            // If the block is larger than required, split it
            if (current->size > size + sizeof(Block)) {
                Block* new_block = (Block*)((uint8_t*)current + size);
                new_block->size = current->size - size;
                new_block->next = current->next;
                current->next = new_block;
                current->size = size;
            }

            // Remove the block from the free list
            if (previous) {
                previous->next = current->next;
            } else {
                free_list = current->next; // Update free list head
            }

            allocated_size += current->size; // Update allocated size
            return (uint8_t*)current + sizeof(Block); // Return pointer to user space
        }
        previous = current;
        current = current->next;
    }

    return NULL; // No suitable block found
}

// Calloc function
void* calloc(size_t num, size_t size) {
    size_t total_size = num * size;
    void* ptr = malloc(total_size);
    if (ptr) {
        mem_set(ptr, 0, total_size);
    }
    return ptr;
}

// Realloc function
void* realloc(void* ptr, size_t size) {
    if (ptr == NULL) {
        return malloc(size);
    }
    if (size == 0) {
        free(ptr);
        return NULL;
    }

    Block* block = (Block*)((uint8_t*)ptr - sizeof(Block));
    if (block->size >= size + sizeof(Block)) {
        // If the block is large enough, just return the same pointer
        return ptr;
    }

    void* new_ptr = malloc(size);
    if (new_ptr) {
        mem_copy(new_ptr, ptr, block->size - sizeof(Block)); // Copy old data
        free(ptr);
    }
    return new_ptr;
}

// Free function
void free(void* ptr) {
    if (ptr == NULL) {
        return;
    }

    Block* block = (Block*)((uint8_t*)ptr - sizeof(Block));
    block->next = free_list; // Add block to the free list
    free_list = block; // Update free list
    allocated_size -= block->size; // Update allocated size
}

// Memory set function
void* mem_set(void* ptr, int value, size_t num) {
    unsigned char* p = ptr;
    while (num--) {
        *p++ = (unsigned char)value;
    }
    return ptr;
}

// Memory copy function
void* mem_copy(void* dest, const void* src, size_t n) {
    unsigned char* d = dest;
    const unsigned char* s = src;
    while (n--) {
        *d++ = *s++;
    }
    return dest;
}
