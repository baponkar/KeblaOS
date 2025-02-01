/*

Standard Input Output library

Last Updated : 19/12/2024

*/

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>
#include <limits.h>
#include <float.h>
#include <iso646.h>

#include "../driver/vga.h"

#include "stdio.h"


void putc(char c) {
    putchar(c);
}

void puts(const char* str) {
    print(str);
    putchar('\n');
}

void printf(const char* format, ...) {
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
}

