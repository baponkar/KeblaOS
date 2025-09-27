#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

int abs(int value);
long labs(long value);
long long llabs(long long value);
double atof(const char* str);
int atoi(const char* str);
long atol(const char *str);
void *malloc(size_t size);
void free(void *ptr);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);
int rand(void);
void srand(unsigned int seed);
void qsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *));
void *bsearch(const void *key, const void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *));





