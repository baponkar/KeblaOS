
#include "../driver/vga/vga_term.h"
#include "stdlib.h"

#include "string.h"

// Copy dat of src into dest
void *memcpy(void *dest, const void *src, size_t n) {
    uint64_t *pdest64 = (uint64_t *) dest;
    const uint64_t *psrc64 = (const uint64_t *) src;

    size_t i;
    for (i = 0; i < (n / sizeof(uint64_t)); i++) {
        pdest64[i] = psrc64[i];  // Copy 64-bit chunks
    }

    // Copy any remaining bytes
    uint8_t *pdest8 = (uint8_t *) (pdest64 + i);
    const uint8_t *psrc8 = (const uint8_t *) (psrc64 + i);
    for (i = 0; i < (n % sizeof(uint64_t)); i++) {
        pdest8[i] = psrc8[i];  // Copy remaining bytes
    }

    return dest;
}



void *memset(void *s, int c, size_t n) {
    // Checking This memory have Header or not
    
    uint8_t *p = (uint8_t *) s; // making uint8_t pointer from void pointer

    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t) c; // fill with c
    }

    return s;
}



// The memmove function copies n bytes from the memory area src to the memory area dest.
void *memmove(void *dest, const void *src, size_t n) {
    uint32_t *pdest = (uint32_t *) dest;
    const uint32_t *psrc = (const uint32_t *) src;

    // If src pointer location is located after dest
    if (src > dest) {
        for (size_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    // If src pointer location is located before dest
    } else if (src < dest) {
        for (size_t i = n; i > 0; i--) {
            pdest[i-1] = psrc[i-1];
        }
    }

    return dest;
}




// compare two memory location with size n
int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;

    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;
}


void *memchr(const void *s, int c, size_t n) {
    const unsigned char *p = (const unsigned char *)s;
    for (size_t i = 0; i < n; i++) {
        if (p[i] == (unsigned char)c) {
            return (void *)(p + i);
        }
    }
    return NULL;
}


void int_to_ascii(int n, char str[]) {
    int i, sign;
    if ((sign = n) < 0) n = -n;
    i = 0;
    do {
        str[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);

    if (sign < 0) str[i++] = '-';
    str[i] = '\0';

    reverse(str);
}


void reverse(char s[]) {
    int c, i, j;
    for (i = 0, j = strlen(s)-1; i < j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}


int strlen(char s[]) {
    int i = 0;
    while (s[i] != '\0') ++i;
    return i;
}

void append(char s[], char n) {
    int len = strlen(s);
    s[len] = n;
    s[len+1] = '\0';
}

void backspace(char s[]) {
    int len = strlen(s);
    s[len-1] = '\0';
}

/* Returns <0 if s1<s2, 0 if s1==s2, >0 if s1>s2 */
int strcmp(char s1[], char s2[]) {
    int i;
    for (i = 0; s1[i] == s2[i]; i++) {
        if (s1[i] == '\0') return 0;
    }
    return s1[i] - s2[i];
}

char *strcpy(char *dest, const char *src) {
    char *ret = dest;
    while ((*dest++ = *src++) != '\0');
    return ret;
}

// copy n elements from src to dest
char *strncpy(char *dest, const char *src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    for (; i < n; i++) {
        dest[i] = '\0';
    }
    return dest;
}

int strncmp(const char* s1, const char* s2, unsigned int n) {
    unsigned int i = 0;
    while (i < n) {
        if (s1[i] != s2[i]) {
            return (unsigned char)s1[i] - (unsigned char)s2[i];
        }
        if (s1[i] == '\0') {
            break;
        }
        i++;
    }
    return 0;
}

char* strcat(char* dest, const char* src) {
    char* original_dest = dest;

    // Move to the end of dest
    while (*dest != '\0') {
        dest++;
    }

    // Copy characters from src to dest
    while (*src != '\0') {
        *dest = *src;
        dest++;
        src++;
    }

    // Null-terminate the final string
    *dest = '\0';

    return original_dest;
}

char* strncat(char* dest, const char* src, size_t n) {
    char* original_dest = dest;

    // Move to the end of dest
    while (*dest != '\0') {
        dest++;
    }

    // Copy up to n characters from src to dest
    while (n-- && *src != '\0') {
        *dest = *src;
        dest++;
        src++;
    }

    // Null-terminate the final string
    *dest = '\0';

    return original_dest;
}

int strnlen(const char* str, size_t maxlen){
    size_t len = 0;
    while (len < maxlen && str[len] != '\0') {
        len++;
    }
    return len;
}



// Find the last occurrence of character `c` in string `str`
// Returns a pointer to the found character or NULL if not found
char *strrchr(const char *str, int c) {
    const char *last = NULL;

    while (*str) {
        if (*str == (char)c) {
            last = str; // Update last when a match is found
        }
        str++;
    }

    // Also check if the character to find is '\0'
    if (c == '\0') {
        return (char *)str;
    }

    return (char *)last;
}


// Duplicating the string
char *strdup(const char *str) {
    if (str == NULL) return NULL;
    
    size_t len = strlen(str) + 1;  // +1 for null terminator
    char *copy = malloc(len);
    
    if (copy != NULL) {
        memset(copy, 0, len);
        memcpy(copy, str, len);
    }
    
    return copy;
}

void clear_buffer(char *buffer, int size) {
    memset(buffer, '\0', size);  // Set entire buffer to '\0'
}




// Helper function to convert an integer to a string (base-10 for decimal)
void int_to_str(int num, char* buffer) {
    int i = 0, is_negative = 0;

    // Handle 0 explicitly
    if (num == 0) {
        buffer[i++] = '0';
        buffer[i] = '\0';
        return;
    }

    // Handle negative numbers
    if (num < 0) {
        is_negative = 1;
        num = -num;
    }

    // Process individual digits
    while (num != 0) {
        buffer[i++] = (num % 10) + '0';
        num /= 10;
    }

    // Add negative sign if necessary
    if (is_negative) {
        buffer[i++] = '-';
    }

    // Null-terminate the string
    buffer[i] = '\0';

    // Reverse the string
    for (int j = 0; j < i / 2; j++) {
        char temp = buffer[j];
        buffer[j] = buffer[i - j - 1];
        buffer[i - j - 1] = temp;
    }
}

// Helper function to convert an integer to a string in any base (binary, hex, etc.)
void int_to_base_str(unsigned int num, char* buffer, int base) {
    int i = 0;
    const char* digits = "0123456789ABCDEF";

    // Handle 0 explicitly
    if (num == 0) {
        buffer[i++] = '0';
        buffer[i] = '\0';
        return;
    }

    // Process individual digits based on the base
    while (num != 0) {
        buffer[i++] = digits[num % base];
        num /= base;
    }

    // Null-terminate the string
    buffer[i] = '\0';

    // Reverse the string
    for (int j = 0; j < i / 2; j++) {
        char temp = buffer[j];
        buffer[j] = buffer[i - j - 1];
        buffer[i - j - 1] = temp;
    }
}


char* strchr(const char* str, int c) {
    while (*str) {
        if (*str == (char)c) {
            return (char*)str;
        }
        str++;
    }

    // Check for null terminator match (some versions return pointer if c == '\0')
    if (c == '\0') {
        return (char*)str;
    }

    return 0;
}



static char *next_token = 0;

char *strtok(char *str, const char *delim) {
    if (str) {
        next_token = str;
    } else if (!next_token) {
        return 0;
    }

    char *start = next_token;

    // Skip leading delimiters
    while (*start && strchr(delim, *start)) {
        start++;
    }

    if (!*start) {
        next_token = 0;
        return 0;
    }

    char *end = start;

    while (*end && !strchr(delim, *end)) {
        end++;
    }

    if (*end) {
        *end = '\0';
        next_token = end + 1;
    } else {
        next_token = 0;
    }

    return start;
}













