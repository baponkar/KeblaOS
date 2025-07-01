#include "../include/syscall.h"

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

void printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    const char* p = format;
    while (*p) {
        if (*p == '%') {
            p++;
            switch (*p) {
                case 'd': {
                    int num = va_arg(args, int);
                    print_dec(num < 0 ? -num : num);
                    if (num < 0) putc('-');
                    break;
                }
                case 'f': {
                    double fnum = va_arg(args, double);
                    print_float(fnum, 2); // Default precision of 2
                    break;
                }
                case 'b': {
                    uint64_t bnum = va_arg(args, uint64_t);
                    print_bin(bnum);
                    break;
                }
                case 'x': {
                    uint64_t xnum = va_arg(args, uint64_t);
                    print_hex(xnum);
                    break;
                }
                case 's': {
                    const char* str = va_arg(args, const char*);
                    puts(str);
                    break;
                }
                default:
                    putc(*p); // Print unknown format specifier as is
            }
        } else {
            putc(*p); // Print regular character
        }
        p++;
    }

    va_end(args);
}

void acquire(spinlock_t* lock) {
    while (__atomic_test_and_set(&lock->locked, __ATOMIC_ACQUIRE)) {
        // Spin until the lock is acquired
    }
}

void release(spinlock_t* lock) {
    __atomic_clear(&lock->locked, __ATOMIC_RELEASE);
}



