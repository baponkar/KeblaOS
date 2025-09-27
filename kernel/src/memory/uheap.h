#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

void *uheap_alloc(size_t size, uint8_t type);
void uheap_free(void *ptr, size_t size);


