#pragma once

#include "stddef.h"  // For size_t

// Function prototypes for string manipulation

// Sets the first 'count' bytes of the block of memory pointed by 'dest' to the value 'val'.
void *memset1(void *dest, int val, size_t count);

// Copies 'count' bytes from the memory area 'src' to memory area 'dest'.
void *memcpy(void *dest, const void *src, size_t count);

// Returns the length of the string (excluding the null terminator).
size_t strlen(const char *str);

// Compares two strings. Returns 0 if equal, <0 if str1 < str2, >0 if str1 > str2.
int strcmp(const char *str1, const char *str2);

// Copies the string pointed to by src (including the null terminator) to dest.
char *strcpy(char *dest, const char *src);

// Adding two string dest and src
char * strcat(char *dest, const char *src);


