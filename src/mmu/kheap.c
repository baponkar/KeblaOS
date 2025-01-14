
/*
Kernel Heap Allocation

https://github.com/dreamportdev/Osdev-Notes/blob/master/04_Memory_Management/05_Heap_Allocation.md
*/


#include "kheap.h"

#define KHEAP_START 0xFFFFFFFFC1D0B000  // Start address of kernel heap in higher half
#define KHEAP_INITIAL_SIZE (4 * 1024 * 1024) // Initial 4 MB heap size
#define KHEAP_MAX_SIZE (128 * 1024 * 1024)   // Max size of kernel heap (128 MB)

#define USED 1
#define FREE 0

uint64_t *heap_start;

uint64_t *cur_heap_position; //a virtual address allocated from the VMM.

uint64_t *cur_pointer;

void init_kheap(){
    print("Start KHEAP initialization...\n");

    heap_start = (uint64_t *) KERNEL_MEM_START_ADDRESS; // a virtual address allocated from the VMM.

    cur_heap_position = heap_start;

    print("Successfully KHEAP initialized.\n");
}


void *third_alloc(size_t size) {

  uint64_t *cur_pointer = heap_start; // Set minimum position

  while(cur_pointer < cur_heap_position){
    size_t cur_size = *cur_pointer;
    size_t status = *(cur_pointer + 1);

    if(cur_size >= size && status == FREE){
      status = USED;
      return cur_pointer + 2;
    }
    cur_pointer = cur_pointer + (size + 2);
  }

  *cur_heap_position=size;
  cur_heap_position = cur_heap_position + 1;
  *cur_heap_position = USED;
  cur_heap_position = cur_heap_position + 1;
  uint64_t *addr_to_return = cur_heap_position;
  cur_heap_position+=size;
  return (void*) addr_to_return;
}


void third_free(uint64_t *ptr) {
  if( *(ptr - 1) == USED ) {
    *(ptr - 1) = FREE;
  }
}


void merge_left(){
  heap_node_t *prev_node = cur_node->prev //cur_pointer is the node we want to check if can be merged
  if (prev_node != NULL && prev_node->status == FREE) {
      // The prev node is free, and cur node is going to be freed so we can merge them
      heap_node_t next_node = cur_pointer->next;
      prev_node->size = prev_node->size + cur_node->size + sizeof(heap_node_t);
      prev_node->next = cur_pointer->next;
      if (next_node != NULL) {
          next_node->prev = prev_node;
      }
  }
}



void test_heap(){
  print("\nKheap test\n");
  uint64_t *ptr1 = (uint64_t *) third_alloc(4096);
  print("ptr1 : ");
  print_dec(*(ptr1 - 1));
  print("\n");

  third_free(ptr1);
  print("ptr1 : ");
  print_dec(*(ptr1-1));
  print("\n");

  uint64_t *a = third_alloc(6);
  uint64_t *b = third_alloc(6);
  uint64_t *c = third_alloc(6);
  third_free(c);
  third_free(b);
  third_free(a);
}

