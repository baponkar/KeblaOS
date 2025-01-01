// ordered_array.h -- Interface for creating, inserting and deleting
// from ordered arrays.
// Written for JamesM's kernel development tutorials.

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "../lib/string.h"  // For memcpy
#include "../lib/assert.h"  // For assert
#include "kheap.h"          // For kmalloc, free


/**
  This array is insertion sorted - it always remains in a sorted state (between calls).
  It can store anything that can be cast to a void* -- so a uint64_t, or any pointer.
**/

// Define the type of the elements in the array.
typedef void* type_t;

// A predicate should return nonzero if the first argument is less than the second. Else it should return zero.
typedef int8_t (*lessthan_predicate_t)(type_t, type_t); // any function pointer type which will takes two arguments of type type_t and returns an int8_t.

// Define the structure of the ordered array.
struct ordered_array
{
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
ordered_array_t place_ordered_array(void *addr, uint64_t max_size, lessthan_predicate_t less_than);

// Destroy an ordered array.
void destroy_ordered_array(ordered_array_t *array);

// Add an item into the array.
void insert_ordered_array(type_t item, ordered_array_t *array);

// Lookup the item at index i.
type_t lookup_ordered_array(uint64_t i, ordered_array_t *array);

// Deletes the item at location i from the array.
void remove_ordered_array(uint64_t i, ordered_array_t *array);

