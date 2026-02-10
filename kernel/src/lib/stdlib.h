#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>


int abs(int value);                 // Absolute value of an integer
long labs(long value);              // Absolute value of a long integer
long long llabs(long long value);   // Absolute value of a long long integer

double atof(const char* str);       // Convert string to double
int atoi(const char* str);          // Convert string to integer
long atol(const char *str);         // Convert string to long integer

// Metadata header structure
typedef struct malloc_header {
    size_t size;
    struct malloc_header *next;
    struct malloc_header *prev;
    unsigned char magic;  // For integrity checking
} malloc_header_t;

#define MAGIC 0xAB
#define HEADER_SIZE sizeof(malloc_header_t)
#define TAILER_SIZE sizeof(malloc_header_t)


void *malloc(size_t size);                  // Allocate memory
void free(void *ptr);                       // Free allocated memory
void *calloc(size_t nmemb, size_t size);    // Allocate and zero-initialize array
void *realloc(void *ptr, size_t size);      // Resize allocated memory block

int rand(void);                      // Generate a random number
void srand(unsigned int seed);       // Seed the random number generator

void qsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *));   // Sort an array
void *bsearch(const void *key, const void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *)); // Binary search in a sorted array









