/*

Standard Input Output library

Last Updated : 19/12/2024

*/



#include "../driver/vga/vga_term.h"

#include "stdio.h"


spinlock_t serial_lock;

void putc(char c) {
    putchar(c);
}

void puts(const char* str) {
    print(str);
    putchar('\n');
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


void printf(const char* format, ...) {
    acquire(&serial_lock);
    va_list args;
    va_start(args, format);

    for (const char* ptr = format; *ptr != '\0'; ptr++) {
        if (*ptr == '%') {
            ptr++;
            switch (*ptr) {
                case 'd':
                    print_dec(va_arg(args, uint64_t));
                    break;
                case 'x':
                    print_hex(va_arg(args, uint64_t));
                    break;
                case 'b':
                    print_bin(va_arg(args, uint64_t));
                    break;
                case 'c':
                    putchar((char)va_arg(args, int));
                    break;
                case 's':
                    print(va_arg(args, const char*));
                    break;
                case 'f':
                    print_float(va_arg(args, double), 6); // Default precision: 6
                    break;
                default:
                    putchar('%');
                    putchar(*ptr);
                    break;
            }
        } else {
            putchar(*ptr);
        }
    }
    
    va_end(args);
    release(&serial_lock);
}



void acquire(spinlock_t* lock) {
    while (true) {
        if (lock->locked == false) {
            lock->locked = true;
            return;
        }
    }
}

void release(spinlock_t* lock) {
    lock->locked = false;
}


