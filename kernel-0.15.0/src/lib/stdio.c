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

// Outputs a decimal number to the screen.
void print_dec(uint64_t n){
    if (n == 0)
    {
        putchar('0');
        return;
    }

    if(n < 0){
        putchar('-');
        n *= -1;
        return;
    }

    char buffer[48]; // Enough for a 64-bit integer
    int i = 0;

    while (n > 0)
    {
        buffer[i++] = '0' + (n % 10); // get the last digit
        n /= 10;                      // remove the last digit
    }

    // Digits are in reverse order, so print them backwards
    for (int j = i - 1; j >= 0; j--)
    {
        putchar(buffer[j]);
    }
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

void print_bin(uint64_t value) {
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

void print_hex(uint64_t n) {
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


