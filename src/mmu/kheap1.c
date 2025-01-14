#include "kheap1.h"

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

  while(cur_pointer < cur_heap_position){ // loop from minimum position to last updated heap position
    size_t cur_size = *cur_pointer; // get the size in cur_pointer
    size_t status = *(cur_pointer + 1); // get the status of cur_pointer+1

    if(cur_size >= size && status == FREE){ // check is it sufficient size and status is free
      status = USED; 
      return cur_pointer + 2; // return found a previously allocated pointer address
    }
    cur_pointer = cur_pointer + (size + 2); // not found move next allocation
  }

  // if not found proper free previously allocated size
  // Now expand the current heap
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