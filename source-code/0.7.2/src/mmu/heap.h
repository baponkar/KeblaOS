#pragma once

#include "../stdlib/stdint.h"
#include "../stdlib/stddef.h"
#include "../util/util.h"

void *kmalloc(size_t size);
void *kcalloc(size_t num, size_t size);
void *krealloc(void *ptr, size_t size);
void kfree(void *ptr);
void init_memory();  // Initializes the memory system
