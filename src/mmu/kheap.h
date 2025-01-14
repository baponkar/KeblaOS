#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "../bootloader/boot.h"

#include "../lib/stdio.h"

#include "pmm.h"
#include "paging.h"
#include "vmm.h"

struct heap_node{
    size_t size;
    uint8_t status;
    struct heap_node *prev;
    struct heap_node *next;
};
typedef struct heap_node heap_node_t;


void init_kheap();
void *third_alloc(size_t size);
void third_free(uint64_t *ptr);
void test_heap();

