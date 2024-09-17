#include "string.h"
#include "stddef.h"

void *memset1(void *dest, int val, size_t count) {
    unsigned char *temp = (unsigned char *)dest;
    for (size_t i = 0; i < count; i++) {
        temp[i] = (unsigned char)val;
    }
    return dest;
}

void *memcpy(void *dest, const void *src, size_t count) {
    const unsigned char *src_ptr = (const unsigned char *)src;
    unsigned char *dest_ptr = (unsigned char *)dest;
    for (size_t i = 0; i < count; i++) {
        dest_ptr[i] = src_ptr[i];
    }
    return dest;
}

size_t strlen(const char *str) {
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

int strcmp(const char *str1, const char *str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return *(unsigned char *)str1 - *(unsigned char *)str2;
}

char *strcpy(char *dest, const char *src) {
    char *ret = dest;
    while ((*dest++ = *src++) != '\0');
    return ret;
}

char *strcat(char *dest, const char *src) {
    char *original_dest = dest;

    // Move `dest` pointer to the end of the existing string
    while (*dest) {
        dest++;
    }

    // Append characters from `src` to `dest`
    while (*src) {
        *dest++ = *src++;
    }

    // Null-terminate the resulting string
    *dest = '\0';

    // Return the pointer to the beginning of the destination string
    return original_dest;
}
