#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

void *kheap_alloc(size_t size);
void kheap_free(void *ptr, size_t size);
void test_kheap();
