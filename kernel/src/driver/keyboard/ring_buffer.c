/*
Circular Buffer or Ring Buffer to store and read user shell command
buffer is well-suited as a FIFO (first in, first out) buffer while a standard, 
non-circular buffer is well suited as a LIFO (last in, first out) buffer.

https://en.wikipedia.org/wiki/Circular_buffer
https://embedjournal.com/implementing-circular-buffer-embedded-c/
*/

#include "../../memory/kheap.h"
#include "../../memory/vmm.h"
#include "../../lib/stdio.h"

#include "ring_buffer.h"

extern ring_buffer_t* keyboard_buffer;

// Initialize the ring buffer with given capacity.
ring_buffer_t* ring_buffer_init(size_t capacity) {

    keyboard_buffer = (ring_buffer_t *) kheap_alloc(sizeof(ring_buffer_t), ALLOCATE_DATA);
    if (!keyboard_buffer) return NULL;

    keyboard_buffer->buffer = kheap_alloc(capacity * sizeof(uint8_t), ALLOCATE_DATA);
    if (!keyboard_buffer->buffer) {
        kheap_free((void *)keyboard_buffer, sizeof(ring_buffer_t));
        return NULL;
    }

    keyboard_buffer->max = capacity;
    keyboard_buffer->head = 0;
    keyboard_buffer->tail = 0;
    keyboard_buffer->full = false;
    return keyboard_buffer;
}

// Free the allocated ring buffer.
void ring_buffer_free(ring_buffer_t* rb, size_t capacity) {
    if (rb) {
        kheap_free(rb->buffer, capacity * sizeof(uint8_t));
        kheap_free((void *)rb, sizeof(ring_buffer_t));
    }
}

// Check if the ring buffer is empty.
bool is_ring_buffer_empty(ring_buffer_t* rb) {
    return (!rb->full && (rb->head == rb->tail));
}

// Check if the ring buffer is full.
bool is_ring_buffer_full(ring_buffer_t* rb) {
    return rb->full;
}

// Advance the pointer, wrapping around if necessary.
static void advance_pointer(ring_buffer_t* rb) {
    if (rb->full) {
        // When full, move tail to next position to make room.
        rb->tail = (rb->tail + 1) % rb->max;
    }
    rb->head = (rb->head + 1) % rb->max;
    rb->full = (rb->head == rb->tail);
}

// Retreat the tail pointer after reading data.
static void retreat_pointer(ring_buffer_t* rb) {
    rb->full = false;
    rb->tail = (rb->tail + 1) % rb->max;
}

// Push an element into the ring buffer.
void ring_buffer_push(ring_buffer_t* rb, uint8_t data) {
    if (!rb || !rb->buffer) {
        printf("ring_buffer_push: NULL access detected!\n");
        return;
    }
    rb->buffer[rb->head] = data;
    advance_pointer(rb);
}

// Pop an element from the ring buffer. Returns 0 on success, -1 if empty.
int ring_buffer_pop(ring_buffer_t* rb, uint8_t *data) {

    if (!rb || !rb->buffer || !data) {
        printf("ring_buffer_pop: NULL access detected!\n");
        return -1;
    }

    if (is_ring_buffer_empty(rb)) {
        return -1;
    }

    *data = rb->buffer[rb->tail];
    retreat_pointer(rb);
    return 0;
}


// Example usage.
void uses_of_ring_buffer() {
    const size_t capacity = 8;
    ring_buffer_t* rb = ring_buffer_init(capacity);
    if (!rb) {
        printf( "Failed to initialize ring buffer\n");
    }
    
    // Push data into the ring buffer.
    for (uint8_t i = 0; i < 10; i++) {
        printf("Pushing: %d\n", i);
        ring_buffer_push(rb, i);
    }
    
    // Pop data from the ring buffer.
    uint8_t value;
    while (ring_buffer_pop(rb, &value) == 0) {
        printf("Popped: %d\n", value);
    }
    
    ring_buffer_free((void *)rb, capacity);
}



