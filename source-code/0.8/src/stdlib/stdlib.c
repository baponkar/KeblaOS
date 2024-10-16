#include "stdlib.h" 

// Define the size of the heap (for example, 1MB)
#define HEAP_SIZE 0x100000

static uint8_t heap[HEAP_SIZE];  // The heap space
static size_t heap_end = 0;      // Tracks the current end of the heap

// malloc: Allocate memory
void* malloc(size_t size) {
    if (heap_end + size > HEAP_SIZE) {
        return NULL;  // Out of memory
    }
    void* ptr = &heap[heap_end];
    heap_end += size;
    return ptr;
}

// calloc: Allocate zero-initialized memory
void* calloc(size_t num, size_t size) {
    size_t total_size = num * size;
    void* ptr = malloc(total_size);
    if (ptr) {
        memset(ptr, 0, total_size);  // Initialize allocated memory to 0
    }
    return ptr;
}

// realloc: Reallocate memory
void* realloc(void* ptr, size_t new_size) {
    if (!ptr) {
        // If the pointer is NULL, just use malloc
        return malloc(new_size);
    }

    if (new_size == 0) {
        // If new_size is 0, just free the memory
        free(ptr);
        return NULL;
    }

    // Allocate new memory
    void* new_ptr = malloc(new_size);
    if (!new_ptr) {
        return NULL;  // Out of memory
    }

    // Copy old data to new memory block (using memcpy)
    memcpy(new_ptr, ptr, new_size);
    
    // Free the old memory
    free(ptr);

    return new_ptr;
}

// free: Free allocated memory (simple version - does nothing)
void free(void* ptr) {
    // In this simple implementation, we do not maintain a free list
    // or reclaim memory. The memory is essentially "lost" until the
    // program terminates.
}
