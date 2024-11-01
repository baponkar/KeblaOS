#pragma once

#include "stdint.h"
#include "stddef.h"         // for size_t
#include "../util/util.h"   // for memset, memcpy


void* malloc(size_t size);
void* calloc(size_t num, size_t size);
void* realloc(void* ptr, size_t new_size);
void free(void* ptr);

