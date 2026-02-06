/*

Standard Input Output library

Last Updated : 26/07/2025
Author       : Baponkar

*/



#include "../driver/vga/vga_term.h"
#include "stdlib.h"

#include "stdio.h"

typedef struct {
    bool locked;
} spinlock_t;


spinlock_t serial_lock;


static void acquire(spinlock_t* lock) {
    while (true) {
        if (lock->locked == false) {
            lock->locked = true;
            return;
        }
    }
}

static void release(spinlock_t* lock) {
    lock->locked = false;
}

static void print_udec(uint64_t value) {
    char buf[32];
    int i = 0;

    if (value == 0) {
        putchar('0');
        return;
    }

    while (value > 0) {
        buf[i++] = '0' + (value % 10);
        value /= 10;
    }
    while (i > 0) {
        putchar(buf[--i]);
    }
}


// Outputs a decimal number to the screen.
static void print_dec(int64_t value) {
    if (value < 0) {
        putchar('-');
        value = -value;
    }
    print_udec((unsigned long long)value); // Reuse unsigned printer
}





// Helper function to print floating-point numbers
void print_float(double num, int precision) {
    if (num < 0) {
        putchar('-');
        num = -num;
    }

    uint64_t int_part = (uint64_t)num;
    double frac_part = num - (double)int_part;

    print_dec(int_part); // Print integer part
    putchar('.');        // Decimal point

    // Print fractional part with required precision
    for (int i = 0; i < precision; i++) {
        frac_part *= 10;
        int digit = (int)frac_part;
        putchar('0' + digit);
        frac_part -= digit;
    }
}


static void print_bin(uint64_t value) {
    if (value == 0) {
        print("0");
        return;
    }

    char buffer[65]; // Max for 64-bit binary + null terminator
    int pos = 0;
    bool started = false;

    for (int i = 63; i >= 0; i--) {
        if ((value >> i) & 1)
            started = true;

        if (started)
            buffer[pos++] = ((value >> i) & 1) ? '1' : '0';
    }

    buffer[pos] = '\0';
    print(buffer);
}

static void print_hex(uint64_t n) {
    char hex_chars[] = "0123456789ABCDEF";
    print("0x");

    int leading_zero = 1; // Flag to skip leading zeros

    for (int i = 60; i >= 0; i -= 4) { // Process each nibble (4 bits)
        char digit = hex_chars[(n >> i) & 0xF];
        if (digit != '0' || !leading_zero || i == 0) {
            putchar(digit);
            leading_zero = 0; // Once a non-zero digit is found, reset the flag
        }
    }
}

static void print_hex_padded(uint64_t value, int width, bool uppercase) {
    const char *hex_chars = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";
    char buf[32];
    int i = 0;

    do {
        buf[i++] = hex_chars[value & 0xF];
        value >>= 4;
    } while (value > 0 && i < (int)sizeof(buf));

    // Pad with zeros if needed
    while (i < width) {
        buf[i++] = '0';
    }

    // Print in reverse order (MSB first)
    for (int j = i - 1; j >= 0; j--) {
        putchar(buf[j]);
    }
}


// Print a Character 
void putc(char c) {
    putchar(c);
}


// Print a string 
void puts(const char* str) {
    print(str);
    putchar('\n');
}


void vprintf(const char* format, va_list args) {
    for (const char* ptr = format; *ptr != '\0'; ptr++) {
        if (*ptr == '%') {
            ptr++;

            int width = 0;
            bool zero_pad = false;

            // Parse zero-padding flag
            if(*ptr == '0'){
                zero_pad = true;
                ptr++;
            }

            // Parse width number (e.g. "02" or "08")
            while(*ptr >= '0' && *ptr <= '9'){
                width = width * 10 + (*ptr - '0');
                ptr++;
            }

            if(*ptr == '%'){
                putchar('%');
                continue;
            }

            // Handle long/long long modifiers
            if (*ptr == 'l') {
                ptr++;
                if (*ptr == 'd') {
                    print_dec((long)va_arg(args, long));
                } 
                else if (*ptr == 'u') {
                    print_udec((unsigned long)va_arg(args, unsigned long));
                } 
                else if (*ptr == 'l') { // long long
                    ptr++;
                    if (*ptr == 'd') {
                        print_dec((long long)va_arg(args, long long));
                    } 
                    else if (*ptr == 'u') {
                        print_udec((unsigned long long)va_arg(args, unsigned long long));
                    } 
                    else {
                        putchar('%');
                        putchar('l');
                        putchar('l');
                        putchar(*ptr);
                    }
                } 
                else {
                    putchar('%');
                    putchar('l');
                    putchar(*ptr);
                }
            }
            else {
                // Handle single-character specifiers
                switch (*ptr) {
                    case 'd':{
                        print_dec((int)va_arg(args, int));
                        break;
                    }
                    case 'u':{
                        print_udec((unsigned int)va_arg(args, unsigned int));
                        break;
                    }
                    
                    case 'x': {
                        uint64_t val = va_arg(args, uint64_t);
                        if (width > 0 && zero_pad)
                            print_hex_padded(val, width, false);
                        else
                            print_hex(val);
                        break;
                    }

                    case 'X': {
                        uint64_t val = va_arg(args, uint64_t);
                        if (width > 0 && zero_pad)
                            print_hex_padded(val, width, true);
                        else
                            print_hex(val);
                        break;
                    }
                    case 'b':{
                        print_bin(va_arg(args, uint64_t));
                        break;
                    }
                    case 'c':{
                        putchar((char)va_arg(args, int));
                        break;
                    }
                    case 's':{
                        print(va_arg(args, const char*));
                        break;
                    }
                    case 'f':{
                        print_float(va_arg(args, double), 6); // Default precision: 6
                        break;
                    }
                    case 'p': {
                        void* ptr_val = va_arg(args, void*);
                        uintptr_t addr = (uintptr_t)ptr_val;
                        print_hex(addr);
                        break;
                    }
                    default:{
                        putchar('%');
                        putchar(*ptr);
                        break;
                    }
                }
            }
        } 
        else {
            putchar(*ptr);
        }
    }
}

// printing string ,character, numbers etc
void printf(const char* format, ...) {
    acquire(&serial_lock);
    va_list args;
    va_start(args, format);

    vprintf(format, args);

    va_end(args);
    release(&serial_lock);
}


static char* sprint_char(char* buffer, char c) {
    *buffer++ = c;
    return buffer;
}

static char* sprint_str(char* buffer, const char* s) {
    while (*s) {
        *buffer++ = *s++;
    }
    return buffer;
}

void vsprintf(char* buf, const char* format, va_list args) {
    char* out = buf;

    for (const char* ptr = format; *ptr != '\0'; ptr++) {
        if (*ptr == '%') {
            ptr++;

            if (*ptr == 'l') {
                ptr++;
                if (*ptr == 'd') {
                    long val = va_arg(args, long);
                    char tmp[32];
                    int i = 0;
                    if (val < 0) {
                        *out++ = '-';
                        val = -val;
                    }
                    do {
                        tmp[i++] = '0' + (val % 10);
                        val /= 10;
                    } while (val > 0);
                    while (i > 0) *out++ = tmp[--i];
                }
                else if (*ptr == 'u') {
                    unsigned long val = va_arg(args, unsigned long);
                    char tmp[32];
                    int i = 0;
                    do {
                        tmp[i++] = '0' + (val % 10);
                        val /= 10;
                    } while (val > 0);
                    while (i > 0) *out++ = tmp[--i];
                }
                else if (*ptr == 'l') { // long long
                    ptr++;
                    if (*ptr == 'd') {
                        long long val = va_arg(args, long long);
                        char tmp[32];
                        int i = 0;
                        if (val < 0) {
                            *out++ = '-';
                            val = -val;
                        }
                        do {
                            tmp[i++] = '0' + (val % 10);
                            val /= 10;
                        } while (val > 0);
                        while (i > 0) *out++ = tmp[--i];
                    }
                    else if (*ptr == 'u') {
                        unsigned long long val = va_arg(args, unsigned long long);
                        char tmp[32];
                        int i = 0;
                        do {
                            tmp[i++] = '0' + (val % 10);
                            val /= 10;
                        } while (val > 0);
                        while (i > 0) *out++ = tmp[--i];
                    }
                }
            }
            else {
                switch (*ptr) {
                    case 'd': {
                        int val = va_arg(args, int);
                        char tmp[32];
                        int i = 0;
                        if (val < 0) {
                            *out++ = '-';
                            val = -val;
                        }
                        do {
                            tmp[i++] = '0' + (val % 10);
                            val /= 10;
                        } while (val > 0);
                        while (i > 0) *out++ = tmp[--i];
                        break;
                    }
                    case 'u': {
                        unsigned int val = va_arg(args, unsigned int);
                        char tmp[32];
                        int i = 0;
                        do {
                            tmp[i++] = '0' + (val % 10);
                            val /= 10;
                        } while (val > 0);
                        while (i > 0) *out++ = tmp[--i];
                        break;
                    }
                    case 'c':
                        *out++ = (char)va_arg(args, int);
                        break;
                    case 's':
                        out = sprint_str(out, va_arg(args, const char*));
                        break;
                    default:
                        *out++ = '%';
                        *out++ = *ptr;
                        break;
                }
            }
        }
        else {
            *out++ = *ptr;
        }
    }

    *out = '\0'; // null terminate
}

void sprintf(char* buf, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsprintf(buf, format, args);
    va_end(args);
}


static char* sprint_char_limit(char* buffer, char c, size_t* remaining) {
    if (*remaining > 1) { // leave space for null terminator
        *buffer++ = c;
        (*remaining)--;
    }
    return buffer;
}

static char* sprint_str_limit(char* buffer, const char* s, size_t* remaining) {
    while (*s && *remaining > 1) {
        *buffer++ = *s++;
        (*remaining)--;
    }
    return buffer;
}

void vsnprintf(char* buf, size_t size, const char* format, va_list args) {
    char* out = buf;
    size_t remaining = size;

    for (const char* ptr = format; *ptr != '\0'; ptr++) {
        if (*ptr == '%') {
            ptr++;

            if (*ptr == 'l') {
                ptr++;
                if (*ptr == 'd') {
                    long val = va_arg(args, long);
                    char tmp[32];
                    int i = 0;
                    if (val < 0) {
                        out = sprint_char_limit(out, '-', &remaining);
                        val = -val;
                    }
                    do {
                        tmp[i++] = '0' + (val % 10);
                        val /= 10;
                    } while (val > 0);
                    while (i > 0) out = sprint_char_limit(out, tmp[--i], &remaining);
                }
                else if (*ptr == 'u') {
                    unsigned long val = va_arg(args, unsigned long);
                    char tmp[32];
                    int i = 0;
                    do {
                        tmp[i++] = '0' + (val % 10);
                        val /= 10;
                    } while (val > 0);
                    while (i > 0) out = sprint_char_limit(out, tmp[--i], &remaining);
                }
                else if (*ptr == 'l') { // long long
                    ptr++;
                    if (*ptr == 'd') {
                        long long val = va_arg(args, long long);
                        char tmp[32];
                        int i = 0;
                        if (val < 0) {
                            out = sprint_char_limit(out, '-', &remaining);
                            val = -val;
                        }
                        do {
                            tmp[i++] = '0' + (val % 10);
                            val /= 10;
                        } while (val > 0);
                        while (i > 0) out = sprint_char_limit(out, tmp[--i], &remaining);
                    }
                    else if (*ptr == 'u') {
                        unsigned long long val = va_arg(args, unsigned long long);
                        char tmp[32];
                        int i = 0;
                        do {
                            tmp[i++] = '0' + (val % 10);
                            val /= 10;
                        } while (val > 0);
                        while (i > 0) out = sprint_char_limit(out, tmp[--i], &remaining);
                    }
                }
            }
            else {
                switch (*ptr) {
                    case 'd': {
                        int val = va_arg(args, int);
                        char tmp[32];
                        int i = 0;
                        if (val < 0) {
                            out = sprint_char_limit(out, '-', &remaining);
                            val = -val;
                        }
                        do {
                            tmp[i++] = '0' + (val % 10);
                            val /= 10;
                        } while (val > 0);
                        while (i > 0) out = sprint_char_limit(out, tmp[--i], &remaining);
                        break;
                    }
                    case 'u': {
                        unsigned int val = va_arg(args, unsigned int);
                        char tmp[32];
                        int i = 0;
                        do {
                            tmp[i++] = '0' + (val % 10);
                            val /= 10;
                        } while (val > 0);
                        while (i > 0) out = sprint_char_limit(out, tmp[--i], &remaining);
                        break;
                    }
                    case 'c':
                        out = sprint_char_limit(out, (char)va_arg(args, int), &remaining);
                        break;
                    case 's':
                        out = sprint_str_limit(out, va_arg(args, const char*), &remaining);
                        break;
                    default:
                        out = sprint_char_limit(out, '%', &remaining);
                        out = sprint_char_limit(out, *ptr, &remaining);
                        break;
                }
            }
        }
        else {
            out = sprint_char_limit(out, *ptr, &remaining);
        }
    }

    if (remaining > 0) {
        *out = '\0';
    } else if (size > 0) {
        buf[size - 1] = '\0';
    }
}

void snprintf(char* buf, size_t size, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(buf, size, format, args);
    va_end(args);
}




