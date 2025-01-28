#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>





typedef char* va_list;  // va_list is just a pointer to the argument list

// #define VA_SIZE(type) ((sizeof(type) + sizeof(int) - 1) & ~(sizeof(int) - 1))  // Ensure proper alignment for x86_32
// Ensure proper alignment for 64-bit (align to 8 bytes, size of void*)
// 8 bytes or 64 bit
#define VA_SIZE(type)((sizeof(type) + sizeof(void*) - 1) & ~(sizeof(void*) - 1))  // Ensure proper alignment it will return 8 bytes

// va_start: Initializes the va_list to point to the first argument after the 'last' fixed argument
#define va_start(ap, last)(ap = (va_list) &last + VA_SIZE(last))

// va_arg: Retrieve the next argument in the list
#define va_arg(ap, type)(*(type *)((ap += VA_SIZE(type)) - VA_SIZE(type)))

// va_end: Cleans up (does nothing here, but can be used for portability)
#define va_end(ap)(ap = (va_list)0)


void printf(const char* format, ...);


