#ifndef STDLIB_H
#define STDLIB_H

#include "stddef.h"  // For size_t

// String to integer conversion
int atoi(const char *str);
long strtol(const char *str, char **endptr, int base);
unsigned long strtoul(const char *str, char **endptr, int base);

// Integer to string conversion
char *itoa(int value, char *str, int base);

// Memory management functions (simple implementations)
void *malloc(size_t size);
void free(void *ptr);

// Memory allocation error function
void abort(void);

#endif // STDLIB_H
