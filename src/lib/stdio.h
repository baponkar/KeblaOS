#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>
#include <limits.h>
#include <float.h>
#include <iso646.h>

void printf(const char* format, ...);


typedef struct {
    bool locked;
} spinlock_t;

void acquire(spinlock_t* lock);
void release(spinlock_t* lock);

