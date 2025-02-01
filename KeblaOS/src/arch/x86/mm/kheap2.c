
/*
Kernel Heap Allocation

https://github.com/dreamportdev/Osdev-Notes/blob/master/04_Memory_Management/05_Heap_Allocation.md
*/


#include "kheap.h"

#define KHEAP_START 0xFFFFFFFFC1D0B000  // Start address of kernel heap in higher half
#define KHEAP_INITIAL_SIZE (1 * 1024) // Initial 1 KB heap size
#define KHEAP_MAX_SIZE (128 * 1024 * 1024)   // Max size of kernel heap (128 MB)

#define USED 1
#define FREE 0

uint64_t *heap_start;
uint64_t *heap_end;

heap_node_t *start_node;
heap_node_t *end_node;

void init_kheap(){
  print("Start KHEAP initialization...\n");

  heap_start = (uint64_t *) KERNEL_MEM_START_ADDRESS; // a virtual address allocated from the VMM.
  heap_end = (uint64_t *) KERNEL_MEM_END_ADDRESS;

  start_node = (heap_node_t *) heap_start;

  start_node->size = KHEAP_INITIAL_SIZE;
  start_node->status = FREE;
  start_node->next = NULL;
  start_node->prev = NULL;

  end_node = start_node;
  
  print("Successfully KHEAP initialized.\n");
}


void *fourth_alloc(size_t size){
  heap_node_t *node = start_node;

  // find a suitable node
  while(node != NULL){
    //splitting if big enough
    if(node->size > (size + sizeof(heap_node_t)) && node->status ==  FREE ){
      heap_node_t *split_node1 = node;
      split_node1->status = USED;
      split_node1->size = size;

      heap_node_t *split_node = (heap_node_t*)((uint64_t)node + sizeof(heap_node_t) + size);
      split_node->status = FREE;
      split_node->size = (size_t) (node->size - (size + sizeof(heap_node_t)));
      split_node->prev = split_node1;
      split_node->next = node->next;

      split_node1->next = split_node;

      return (void*) ((uint64_t)split_node1 + sizeof(heap_node_t));
    }
    // simply use the free node
    if(node->size >= size && node->status ==  FREE ){
      node->status = USED;
      return (void*) ((uint64_t)node + sizeof(heap_node_t));
    }
    node = node->next;
  }

  // Allocate a new node if no suitable free block was found
  heap_node_t *new_node = (heap_node_t*)((uint64_t)end_node + sizeof(heap_node_t) + end_node->size);

  if((((uint64_t)new_node) + sizeof(heap_node_t) + size ) > (uint64_t) heap_end){
    print("Exhausted memory\n");
    return NULL; // exhausted memory 
  }
  new_node->size = size;
  new_node->status = USED;
  new_node->prev = end_node;
  new_node->next = NULL;

  end_node = new_node;
  return (void*)(((uint64_t)new_node) + sizeof(heap_node_t));
}


void fourth_free(uint64_t *ptr) {
    heap_node_t *node = (heap_node_t*)((uint64_t)ptr - sizeof(heap_node_t));
    if (node->status == USED) {
        node->status = FREE;

        // Attempt to merge with next node if free
        if (node->next != NULL && node->next->status == FREE) {
            node->size += sizeof(heap_node_t) + node->next->size;
            node->next = node->next->next;
            if (node->next != NULL) {
                node->next->prev = node;
            }else{
              end_node = node;
            }
        }

        // Attempt to merge with previous node if free
        if (node->prev != NULL && node->prev->status == FREE) {
            node->prev->size += sizeof(heap_node_t) + node->size;
            node->prev->next = node->next;
            if (node->next != NULL) {
                node->next->prev = node->prev;
            }else{
              end_node = node->prev;
            }
        }
    }
}




void test_heap() {
    printf("\nKheap test using fourth_alloc\n");


    // Allocate some blocks using fourth_alloc
    void *block1 = fourth_alloc(32);
    print("Allocated block1: ");
    print_hex((uint64_t)block1);
    print("\n");

    void *block2 = fourth_alloc(64);
    print("Allocated block2: ");
    print_hex((uint64_t)block2);
    print("\n");

    void *block3 = fourth_alloc(128);
    print("Allocated block3: ");
    print_hex((uint64_t)block3);
    print("\n");

    // Free a block and show merging
    fourth_free((uint64_t*)block3);
    print("Freed block3\n");

    // Allocate again to test reuse of freed block
    void *block4 = fourth_alloc(64);
    print("Allocated block4 (reusing block2 space): ");
    print_hex((uint64_t)block4);
    print("\n");

    // Free all blocks
    fourth_free((uint64_t*)block1);
    fourth_free((uint64_t*)block3);
    fourth_free((uint64_t*)block4);

    print("Heap test using fourth_alloc complete.\n");
}


