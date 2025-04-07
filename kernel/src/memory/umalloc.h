#pragma once

#include <stdint.h>

uint64_t umalloc(uint64_t sz);       // vanilla (normal).
uint64_t umalloc_a(uint64_t sz, int align);    // page aligned
uint64_t umalloc_p(uint64_t sz, uint64_t *phys);
uint64_t umalloc_ap(uint64_t sz, int align, uint64_t *phys);  // page aligned and returns a physical address..
void test_umalloc();
