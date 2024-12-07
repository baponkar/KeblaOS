#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "../kernel/kernel.h"


uint64_t kmalloc(uint64_t sz); // vanilla (normal).
uint64_t kmalloc_a(uint64_t sz, int align);  // page aligned.
uint64_t kmalloc_p(uint64_t sz, uint64_t *phys); // returns a physical address.
uint64_t kmalloc_ap(uint64_t sz, int align, uint64_t *phys); // page aligned and returns a physical address.

