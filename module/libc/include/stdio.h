#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>



void putc(char c);
void puts(const char* str);

void print_dec(uint64_t n);
void print_float(double num, int precision);
void print_bin(uint64_t value);
void print_hex(uint64_t n);;

void vprintf(const char* format, va_list args);
void printf(const char* format, ...);

void vsprintf(char* buf, const char* format, va_list args);
void snprintf(char* buf, size_t size, const char* format, ...);
void sprintf(char* buf, const char* format, ...);
void vsnprintf(char* buf, size_t size, const char* format, va_list args);
void snprintf(char* buf, size_t size, const char* format, ...);


