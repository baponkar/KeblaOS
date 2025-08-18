
#include "../include/syscall.h"

#include "../include/stdarg.h"
#include "../include/stdio.h"

void putc(char c) {
    char str[2];
    str[0] = c;
    str[1] = '\0';
    syscall_print(str);  // ✅ Safe: null-terminated string
}


void puts(const char* str) {
    if (str) {
        syscall_print(str);
    }
}

static void print_udec(uint64_t value) {
    char buf[32];
    int i = 0;

    if (value == 0) {
        putc('0');
        return;
    }

    while (value > 0) {
        buf[i++] = '0' + (value % 10);
        value /= 10;
    }
    while (i > 0) {
        putc(buf[--i]);
    }
}

void print_dec(uint64_t n) {
    char buffer[21]; // Enough for 64-bit integer
    int i = 0;
    if (n == 0) {
        putc('0');
        return;
    }
    while (n > 0) {
        buffer[i++] = '0' + (n % 10);
        n /= 10;
    }
    while (i > 0) {
        putc(buffer[--i]);
    }
}

void print_float(double num, int precision) {
    if (precision < 0) precision = 0; // Ensure non-negative precision
    if (precision > 20) precision = 20; // Limit precision to avoid overflow

    // Handle negative numbers
    if (num < 0) {
        putc('-');
        num = -num;
    }

    // Split into integer and fractional parts
    uint64_t integer_part = (uint64_t)num;
    double fractional_part = num - integer_part;

    // Print integer part
    print_dec(integer_part);

    if (precision > 0) {
        putc('.'); // Decimal point
        for (int i = 0; i < precision; i++) {
            fractional_part *= 10;
            int digit = (int)fractional_part;
            putc('0' + digit);
            fractional_part -= digit;
        }
    }
}

void print_bin(uint64_t value) {
    if (value == 0) {
        putc('0');
        return;
    }
    char buffer[65]; // Enough for 64-bit integer
    int i = 0;
    while (value > 0) {
        buffer[i++] = (value & 1) ? '1' : '0';
        value >>= 1;
    }
    while (i > 0) {
        putc(buffer[--i]);
    }
}

void print_hex(uint64_t n) {
    if (n == 0) {
        putc('0');
        return;
    }
    char buffer[17]; // Enough for 64-bit hex
    int i = 0;
    while (n > 0) {
        int digit = n & 0xF;
        buffer[i++] = (digit < 10) ? ('0' + digit) : ('A' + digit - 10);
        n >>= 4;
    }
    while (i > 0) {
        putc(buffer[--i]);
    }
}

void vprintf(const char* format, va_list args) {
    for (const char* ptr = format; *ptr != '\0'; ptr++) {
        if (*ptr == '%') {
            ptr++;

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
                        putc('%');
                        putc('l');
                        putc('l');
                        putc(*ptr);
                    }
                } 
                else {
                    putc('%');
                    putc('l');
                    putc(*ptr);
                }
            }
            else {
                // Handle single-character specifiers
                switch (*ptr) {
                    case 'd':
                        print_dec((int)va_arg(args, int));
                        break;
                    case 'u':
                        print_udec((unsigned int)va_arg(args, unsigned int));
                        break;
                    case 'x':
                        print_hex(va_arg(args, uint64_t));
                        break;
                    case 'b':
                        print_bin(va_arg(args, uint64_t));
                        break;
                    case 'c':
                        putc((char)va_arg(args, int));
                        break;
                    case 's':
                        puts(va_arg(args, const char*));
                        break;
                    case 'f':
                        print_float(va_arg(args, double), 6); // Default precision: 6
                        break;
                    default:
                        putc('%');
                        putc(*ptr);
                        break;
                }
            }
        } 
        else {
            putc(*ptr);
        }
    }
}

// printing string ,character, numbers etc
void printf(const char* format, ...) {
    va_list args;
    va_start(args, format);

    vprintf(format, args);

    va_end(args);
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








