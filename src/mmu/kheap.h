

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "../bootloader/boot.h"

#include "pmm.h"
#include "../driver/vga.h"
#include "../lib/string.h"  // For memcpy
#include "../lib/assert.h"  // For assert

#define PAGE_ALIGN(addr) ((addr + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1))

// To implement memory allocation by using placement_address
void init_kheap();
uint64_t kmalloc(uint64_t sz); // vanilla (normal).
uint64_t kmalloc_a(uint64_t sz, int align);  // page aligned.
uint64_t kmalloc_p(uint64_t sz, uint64_t *phys); // placed at physical address.
uint64_t kmalloc_ap(uint64_t sz, int align, uint64_t *phys); // page aligned and returns a physical address.


 
// To implementing dynamic memory allocation

typedef void* type_t; // This can hold any type of pointer.

// A predicate should return nonzero if the first argument is less than the second. Else it should return zero.
typedef int8_t (*lessthan_predicate_t)(type_t, type_t); // any function pointer type which will takes two arguments of type type_t and returns an int8_t.

// Define the structure of the ordered array.
struct ordered_array
{
  // This structure will hold many other structures of type type_t.
  type_t *array; // Stores the type_t pointer
  uint64_t size; // Stores the size of the array
  uint64_t max_size; // Stores the maximum size of the array
  lessthan_predicate_t less_than; // function pointer
};
typedef struct ordered_array ordered_array_t;

// A standard less than predicate function.
int8_t standard_lessthan_predicate(type_t a, type_t b);

// Create an ordered array.
ordered_array_t create_ordered_array(uint64_t max_size, lessthan_predicate_t less_than);

// Create an ordered array and place it at a specific address.
ordered_array_t place_ordered_array(void *addr, uint64_t max_size, lessthan_predicate_t less_than);

// Destroy an ordered array.
void destroy_ordered_array(ordered_array_t *array);

// Add an item into the array.
void insert_ordered_array(type_t item, ordered_array_t *array);

// Lookup the item at index i.
type_t lookup_ordered_array(uint64_t i, ordered_array_t *array);

// Deletes the item at location i from the array.
void remove_ordered_array(uint64_t i, ordered_array_t *array);


// Kheap implementation
struct header{
  uint64_t magic;     // Magic number, used for error checking and identification
  uint8_t is_hole;    // 1 if this is a hole. 0 if this is a block.
  uint64_t size;      // size of the block, including the end footer.
}__attribute__((packed));
typedef struct header header_t;


struct footer{
  uint64_t magic;     // Magic number, same as in header_t
  header_t *header;   // Pointer to the block header.
}__attribute__((packed));
typedef struct footer footer_t;


struct heap{
  ordered_array_t ordered_array;  
  uint64_t start_address; // The start address of our allocated space.
  uint64_t end_address;   // The end address of our allocated space. May be expanded up to max_address.
  uint64_t max_address;   // The maximum address the heap can be expanded to.
  uint8_t supervisor;     // Should extra pages requested by us be mapped as supervisor-only?
  uint8_t readonly;       // Should extra pages requested by us be mapped as read-only?
};
typedef struct heap heap_t;


// Function to find the smallest hole that will fit the given size.
static int8_t header_t_less_than(void*a, void *b);

// Function to expand the size of the heap.
static int64_t find_smallest_hole(uint64_t size, uint8_t page_align, heap_t *heap);

// Function to expand the size of the heap.
heap_t *create_heap(uint64_t start, uint64_t end, uint64_t max, uint8_t supervisor, uint8_t readonly);

// Function to allocate a contiguous region of memory 'size' in size. If page_align==1, it creates that block starting on a page boundary.
static void expand(uint64_t new_size, heap_t *heap);

// Function to contract the size of the heap.
static uint64_t contract(uint64_t new_size, heap_t *heap);

//Allocates a contiguous region of memory 'size' in size. If page_align==1, it creates that block starting on a page boundary.
void *alloc(uint64_t size, uint8_t page_align, heap_t *heap);

// Releases a block allocated with 'alloc'.
void kfree(void *p, heap_t *heap);



void heap_test();





