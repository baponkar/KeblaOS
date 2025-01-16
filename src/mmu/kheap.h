#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "../driver/vga.h"
#include "../bootloader/boot.h"
#include "vmm.h"

void *kheap_alloc(size_t size);
void kheap_free(void *ptr, size_t size);
void init_kheap();
