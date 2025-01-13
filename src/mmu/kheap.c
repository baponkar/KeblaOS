
#include "kheap.h"

#define VM_FLAG_NONE 0
#define VM_FLAG_WRITE (1 << 0)
#define VM_FLAG_EXEC (1 << 1)
#define VM_FLAG_USER (1 << 2)

#define PT_FLA_NX 0

#define KHEAP_START 0xFFFFFFFFC1D0B000  // Start address of kernel heap in higher half
#define KHEAP_INITIAL_SIZE (4 * 1024 * 1024) // Initial 4 MB heap size
#define KHEAP_MAX_SIZE (128 * 1024 * 1024)   // Max size of kernel heap (128 MB)

#define PAGE_ALIGN(addr) ((addr + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1))

// A basic block structure for managing allocations
struct kheap_block {
    size_t size;
    int free;
    struct kheap_block* next;
};
typedef struct kheap_block kheap_block_t;

static kheap_block_t* kheap_start;


void init_kheap() {
    kheap_start = (kheap_block_t*) KERNEL_MEM_START_ADDRESS;
    void* heap_base = vmm_alloc(KHEAP_INITIAL_SIZE, VM_FLAG_WRITE, NULL);

    print("heap_base(virtual) : ");
    print_hex((uint64_t)heap_base);
    print("\n");

    print("heap_base(physical) : ");
    print_hex((uint64_t)vir_to_phys((uint64_t)heap_base));
    print("\n");

    kheap_start = (kheap_block_t*) vir_to_phys((uint64_t)heap_base);
    kheap_start->size = (size_t) (KHEAP_INITIAL_SIZE - sizeof(kheap_block_t));
    kheap_start->free = 1;
    kheap_start->next = NULL;

    print("Kheap initialized!\n");
}



void* kheap_alloc(size_t size) {
    kheap_block_t* current = kheap_start;
    while (current) {
        if (current->free && current->size >= size) {
            current->free = 0;
            return (void*)(current + 1);  // Return the memory just after the block header
        }
        current = current->next;
    }
    
    // If no suitable block, expand the heap
    void* new_block = vmm_alloc(PAGE_SIZE, VM_FLAG_WRITE, NULL);
    current->next = (kheap_block_t*)new_block;
    current->next->size = PAGE_SIZE - sizeof(kheap_block_t);
    current->next->free = 0;
    current->next->next = NULL;
    
    return (void*)(current->next + 1);
}

void kheap_free(void* ptr) {
    if (!ptr) return;
    kheap_block_t* block = (kheap_block_t*)ptr - 1;
    block->free = 1;
}


void test_kheap_alloc_free() {
    printf("\nTesting kheap_alloc and kheap_free...\n");

    // Allocate a small block
    void* block1 = kheap_alloc(64);
    if (block1) {
        printf("Allocated 64 bytes at %x\n", block1);
    } else {
        printf("Allocation failed for block1!\n");
    }

    // Allocate a larger block
    void* block2 = kheap_alloc(2048);
    if (block2) {
        printf("Allocated 2048 bytes at %x\n", block2);
    } else {
        printf("Allocation failed for block2!\n");
    }

    // Free the first block
    kheap_free(block1);
    printf("Freed 64 bytes from %x\n", block1);

    // Allocate again to see if reuse works
    void* block3 = kheap_alloc(32);
    if (block3) {
        printf("Reallocated 32 bytes at %x\n", block3);
    } else {
        printf("Reallocation failed for block3!\n");
    }

    // Free the remaining blocks
    kheap_free(block2);
    kheap_free(block3);
    printf("Freed remaining blocks.\n");
}


void test_kheap_multiple_allocations() {
    printf("\nTesting multiple allocations...\n");

    void* blocks[10];
    for (int i = 0; i < 10; i++) {
        blocks[i] = kheap_alloc(128);
        printf("Allocated block %d at %x\n", i, blocks[i]);
    }

    // Free all blocks
    for (int i = 0; i < 10; i++) {
        kheap_free(blocks[i]);
        printf("Freed block %d from %x\n", i, blocks[i]);
    }
}


void test_kheap_exceed_initial_size() {
    printf("\nTesting heap expansion (exceeding initial size)...\n");

    void* large_block = kheap_alloc(KHEAP_INITIAL_SIZE + 1024);
    if (large_block) {
        printf("Allocated large block exceeding initial size at %x\n", large_block);
        kheap_free(large_block);
    } else {
        printf("Large allocation failed!\n");
    }
}


void heap_test() {
    test_kheap_alloc_free();
    test_kheap_multiple_allocations();
    test_kheap_exceed_initial_size();
    printf("\nAll tests completed.\n");
}


