
#pragma once

// The following header files are present in gcc even in -ffreestanding mode
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>
#include <limits.h>
#include <float.h>
#include <iso646.h>



void putc(char c);
void puts(const char* str);

void printf(const char* format, ...);

void sprintf(char* buf, const char* format, ...);
void snprintf(char* buf, size_t size, const char* format, ...);

