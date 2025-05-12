#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>
#include <limits.h>
#include <float.h>
#include <iso646.h>


typedef struct {
    bool locked;
} spinlock_t;

void putc(char c);
void puts(const char* str);

void print_dec(uint64_t n);
void print_float(double num, int precision);
void print_bin(uint64_t value);
void print_hex(uint64_t n);

void printf(const char* format, ...);

void acquire(spinlock_t* lock);
void release(spinlock_t* lock);

