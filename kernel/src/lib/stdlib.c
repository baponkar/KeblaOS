/*

Standard Input Output library

Last Updated : 26/07/2025
Author       : Baponkar

*/

#include "../memory/kheap.h"
#include "../memory/vmm.h"

#include "stdlib.h"






/* ---------- Basic absolute values ---------- */
int abs(int value) {
    return (value < 0) ? -value : value;
}

long labs(long value) {
    return (value < 0) ? -value : value;
}

long long llabs(long long value) {
    return (value < 0) ? -value : value;
}

/* ---------- String to number conversions ---------- */
// converting string to double
double atof(const char* str) {
    double result = 0.0;
    double fraction = 0.0;
    int divisor = 1;
    int sign = 1;

    // Skip whitespaces
    while (*str == ' ' || *str == '\t') {
        str++;
    }

    // Handle sign
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }

    // Parse integer part
    while (*str >= '0' && *str <= '9') {
        result = result * 10.0 + (*str - '0');
        str++;
    }

    // Parse fractional part
    if (*str == '.') {
        str++;
        while (*str >= '0' && *str <= '9') {
            fraction = fraction * 10.0 + (*str - '0');
            divisor *= 10;
            str++;
        }
        result += fraction / divisor;
    }

    return sign * result;
}

// converting string to integer
int atoi(const char* str) {
    int result = 0;
    int sign = 1;

    // Skip whitespace
    while (*str == ' ' || *str == '\t' || *str == '\n') {
        str++;
    }

    // Handle optional '+' or '-' sign
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }

    // Process digits
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }

    return result * sign;
}

/* More general: string to long */
long atol(const char *str) {
    long result = 0;
    int sign = 1;

    // Skip whitespace
    while (*str == ' ' || *str == '\t' || *str == '\n')
        str++;

    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }

    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }

    return result * sign;
}

/* ---------- Memory management ---------- */
void *malloc(size_t size) {
    if (size == 0) return NULL;
    return (void*)kheap_alloc(size, ALLOCATE_DATA);
}

void free(void *ptr, size_t size) {
    if (ptr == NULL) return;
    // In your syscall, size may be required; store metadata or ignore
    kheap_free(ptr, size);
}

void *calloc(size_t nmemb, size_t size) {
    size_t total = nmemb * size;
    void *ptr = malloc(total);
    if (ptr) {
        unsigned char *p = (unsigned char*)ptr;
        for (size_t i = 0; i < total; i++) {
            p[i] = 0;
        }
    }
    return ptr;
}

void *realloc(void *ptr, size_t size) {
    if (ptr == NULL) {
        return malloc(size);
    }
    if (size == 0) {
        free(ptr, size);
        return NULL;
    }

    // Simple strategy: allocate new, copy old, free old
    // For real impl: store size in metadata
    void *newptr = malloc(size);
    if (!newptr) return NULL;

    // copy min(old_size, size) — here we don't know old size, so assume size
    // Better: implement heap metadata in your kernel syscall layer
    // For now: blind copy
    unsigned char *dst = newptr;
    unsigned char *src = ptr;
    for (size_t i = 0; i < size; i++) {
        dst[i] = src[i];
    }

    free(ptr, size); // In real impl, pass old size
    return newptr;
}

/* ---------- Random numbers ---------- */
static unsigned long rand_next = 1;

int rand(void) {
    rand_next = rand_next * 1103515245 + 12345;
    return (int)((rand_next / 65536) % 32768);
}

void srand(unsigned int seed) {
    rand_next = seed;
}

/* ---------- Searching and sorting ---------- */
void qsort(void *base, size_t nmemb, size_t size,
           int (*compar)(const void *, const void *)) {
    // Simple bubble sort for now
    unsigned char *arr = (unsigned char *)base;
    for (size_t i = 0; i < nmemb; i++) {
        for (size_t j = 0; j < nmemb - 1; j++) {
            void *a = arr + j * size;
            void *b = arr + (j + 1) * size;
            if (compar(a, b) > 0) {
                // swap
                for (size_t k = 0; k < size; k++) {
                    unsigned char tmp = ((unsigned char*)a)[k];
                    ((unsigned char*)a)[k] = ((unsigned char*)b)[k];
                    ((unsigned char*)b)[k] = tmp;
                }
            }
        }
    }
}



void *bsearch(const void *key, const void *base,
              size_t nmemb, size_t size,
              int (*compar)(const void *, const void *)) {
    size_t low = 0;
    size_t high = nmemb;
    while (low < high) {
        size_t mid = (low + high) / 2;
        const void *elem = (const void *)((const unsigned char*)base + mid * size);
        int cmp = compar(key, elem);
        if (cmp < 0) {
            high = mid;
        } else if (cmp > 0) {
            low = mid + 1;
        } else {
            return (void*)elem;
        }
    }
    return NULL;
}

