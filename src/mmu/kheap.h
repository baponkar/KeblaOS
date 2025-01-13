#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "../bootloader/boot.h"

#include "../lib/stdio.h"

#include "pmm.h"
#include "vmm.h"



void init_kheap();
void* kheap_alloc(size_t size);
void kheap_free(void* ptr);

void test_kheap_alloc_free();
void test_kheap_multiple_allocations();
void test_kheap_exceed_initial_size();
void heap_test();

