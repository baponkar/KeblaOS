/*

Standard Input Output library

Last Updated : 26/07/2025
Author       : Baponkar

*/

#include "../memory/kheap.h"
#include "../memory/vmm.h"

#include "stdio.h"
#include "string.h"
#include "assert.h"

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
    
    // Allocate space for header + user data
    size_t total_size = HEADER_SIZE + size;
    malloc_header_t *header = (malloc_header_t*)kheap_alloc(total_size, ALLOCATE_DATA);
    
    if (!header) return NULL;
    
    // Initialize header
    header->size = size;
    header->magic = MAGIC;
    // next/prev can be used for heap management if needed
    
    // Return pointer to user data (after header)
    return (void*)((uint8_t*)header + HEADER_SIZE);
}

void free(void *ptr) {
    if (ptr == NULL) return;
    
    // Get header from user pointer
    malloc_header_t *header = (malloc_header_t*)((uint8_t*)ptr - HEADER_SIZE);
    
    // Validate magic number to detect corruption
    if (header->magic != MAGIC) {
        printf("Memory Header magic number corrupted:%x/%x\n", header->magic, MAGIC);
        return;
    }
    
    // Free the entire block (header + user data)
    kheap_free(header, HEADER_SIZE + header->size);
}

void *calloc(size_t nmemb, size_t size) {
    size_t total = nmemb * size;
    void *ptr = malloc(total);
    if (ptr) {
        memset(ptr, 0, total);
    }
    return ptr;
}

void *realloc(void *ptr, size_t size) {
    if (ptr == NULL) return malloc(size);
    if (size == 0) {
        free(ptr);
        return NULL;
    }
    
    // Get original size from header
    malloc_header_t *old_header = (malloc_header_t*)((uint8_t*)ptr - HEADER_SIZE);
    
    if (old_header->magic != MAGIC) {
        printf("[Heap Error] Invalid realloc: corrupted header\n");
        return NULL;
    }
    
    size_t old_size = old_header->size;
    
    // If same size, return original pointer
    if (old_size == size) return ptr;
    
    // Allocate new block
    void *new_ptr = malloc(size);
    if (!new_ptr) return NULL;
    
    // Copy minimum of old and new size
    size_t copy_size = (old_size < size) ? old_size : size;
    memcpy(new_ptr, ptr, copy_size);
    
    // Free old block
    free(ptr);
    
    return new_ptr;
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

