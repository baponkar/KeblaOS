
#include "util.h"

/*
* 'void *' Represents a pointer to a memory location of unspecified type. 
* It's a generic pointer that can point to any data type.
*/

// This function is used to set a block of memory with a particular value.
// This function return uint8_t pointer
// int * ptr;
// Example : memset(ptr, 0, 2) creates a pointer of int ptr[2]
void *memset(void *dest, int val, size_t count) {
    uint8_t *temp = (uint8_t *) dest;
    for (size_t i = 0; i < count; i++) {
        temp[i] = (uint8_t) val;
    }
    return dest;
}


// This function copies a block of memory from a source location to a destination location.
void *memcpy(void *dest, const void *src, size_t count) {
    const uint8_t *src_ptr = (const uint8_t *) src;
    uint8_t *dest_ptr = (uint8_t *) dest;
    for (size_t i = 0; i < count; i++) {
        dest_ptr[i] = src_ptr[i];
    }
    return dest;
}