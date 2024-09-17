#include "stdlib.h"
#include "stddef.h"

// String to Integer Conversion
int atoi(const char *str) {
    int result = 0;
    int sign = 1;

    // Skip whitespace
    while (*str == ' ' || *str == '\t' || *str == '\n') {
        str++;
    }

    // Check for sign
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }

    // Convert digits to integer
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }

    return sign * result;
}

long strtol(const char *str, char **endptr, int base) {
    long result = 0;
    int sign = 1;

    // Skip whitespace
    while (*str == ' ' || *str == '\t' || *str == '\n') {
        str++;
    }

    // Check for sign
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }

    // Parse digits based on the base
    while ((*str >= '0' && *str <= '9') || (base == 16 && (*str >= 'a' && *str <= 'f')) || (base == 16 && (*str >= 'A' && *str <= 'F'))) {
        if (*str >= '0' && *str <= '9') {
            result = result * base + (*str - '0');
        } else if (*str >= 'a' && *str <= 'f') {
            result = result * base + (*str - 'a' + 10);
        } else if (*str >= 'A' && *str <= 'F') {
            result = result * base + (*str - 'A' + 10);
        }
        str++;
    }

    if (endptr) {
        *endptr = (char *)str;
    }

    return sign * result;
}

unsigned long strtoul(const char *str, char **endptr, int base) {
    unsigned long result = 0;

    // Skip whitespace
    while (*str == ' ' || *str == '\t' || *str == '\n') {
        str++;
    }

    // Parse digits based on the base
    while ((*str >= '0' && *str <= '9') || (base == 16 && (*str >= 'a' && *str <= 'f')) || (base == 16 && (*str >= 'A' && *str <= 'F'))) {
        if (*str >= '0' && *str <= '9') {
            result = result * base + (*str - '0');
        } else if (*str >= 'a' && *str <= 'f') {
            result = result * base + (*str - 'a' + 10);
        } else if (*str >= 'A' && *str <= 'F') {
            result = result * base + (*str - 'A' + 10);
        }
        str++;
    }

    if (endptr) {
        *endptr = (char *)str;
    }

    return result;
}

// Simple Memory Management (Dummy malloc and free implementations)

#define HEAP_SIZE 1024  // Define a simple heap size
static unsigned char heap[HEAP_SIZE];
static size_t heap_top = 0;

void *malloc(size_t size) {
    if (heap_top + size > HEAP_SIZE) {
        return NULL;  // Out of memory
    }
    void *ptr = &heap[heap_top];
    heap_top += size;
    return ptr;
}

void free(void *ptr) {
    // In a real implementation, this would mark memory as available, but we are keeping it simple here.
    // We will not implement actual memory deallocation for this dummy version.
}

void abort(void) {
    // This function would terminate the process in a real system.
    // For our purposes, we can trigger an infinite loop to simulate a halt.
    while (1) { }
}


// Integer to String Conversion
char *itoa(int value, char *str, int base) {
    char *ptr = str;
    char *ptr1 = str;
    char tmp_char;
    int tmp_value;

    // Handle base validity
    if (base < 2 || base > 36) {
        *str = '\0';
        return str;
    }

    // Handle negative numbers for base 10
    if (value < 0 && base == 10) {
        *ptr++ = '-';
        ptr1++;
        value = -value;
    }

    // Convert the integer to the string
    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "0123456789abcdefghijklmnopqrstuvwxyz"[tmp_value - value * base];
    } while (value);

    // Null-terminate the string
    *ptr = '\0';

    // Reverse the string (since digits were added in reverse order)
    while (ptr1 < --ptr) {
        tmp_char = *ptr;
        *ptr = *ptr1;
        *ptr1++ = tmp_char;
    }

    return str;
}
