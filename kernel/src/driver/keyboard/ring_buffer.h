
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>


typedef struct {
    uint8_t *buffer;    // The underlying buffer array
    size_t head;        // Write index
    size_t tail;        // Read index
    size_t max;         // Maximum number of elements
    bool full;          // Flag indicating if the buffer is full
} ring_buffer_t;


ring_buffer_t* ring_buffer_init(size_t capacity);           // Initialize the ring buffer with given capacity.
void ring_buffer_free(ring_buffer_t* rb, size_t capacity);  // Free the allocated ring buffer.

bool is_ring_buffer_empty(ring_buffer_t* rb);               // Check if the ring buffer is empty.
bool is_ring_buffer_full(ring_buffer_t* rb);                // Check if the ring buffer is full.

void ring_buffer_push(ring_buffer_t* rb, uint8_t data);     // Push an element into the ring buffer.
int ring_buffer_pop(ring_buffer_t* rb, uint8_t *data);      // Pop an element from the ring buffer. Returns 0 on success, -1 if empty.


void uses_of_ring_buffer();                                 // Testing of ring buffer.







