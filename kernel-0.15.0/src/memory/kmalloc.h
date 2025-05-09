#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>



uint64_t kmalloc(uint64_t sz); // vanilla (normal).
uint64_t kmalloc_a(uint64_t sz, int align);  // page aligned.
// uint64_t kmalloc_p(uint64_t sz, uint64_t *phys); // placed at physical address.
// uint64_t kmalloc_ap(uint64_t sz, int align, uint64_t *phys); // page aligned and returns a physical address.
// uint64_t kmalloc_aligned(uint64_t sz, uint64_t alignment);
// bool check_mem_alloc(void *ptr, uint64_t size);

void test_kmalloc();


