/*

Standard Input Output library

Last Updated : 19/12/2024

*/

#include "../driver/vga.h"

#include "stdio.h"

typedef char* va_list;  // va_list is just a pointer to the argument list

// #define VA_SIZE(type) ((sizeof(type) + sizeof(int) - 1) & ~(sizeof(int) - 1))  // Ensure proper alignment for x86_32
// Ensure proper alignment for 64-bit (align to 8 bytes, size of void*)
// 8 bytes or 64 bit
#define VA_SIZE(type)((sizeof(type) + sizeof(void*) - 1) & ~(sizeof(void*) - 1))  // Ensure proper alignment it will return 8 bytes

// va_start: Initializes the va_list to point to the first argument after the 'last' fixed argument
#define va_start(ap, last)(ap = (va_list) &last + VA_SIZE(last))

// va_arg: Retrieve the next argument in the list
#define va_arg(ap, type)(*(type *)((ap += VA_SIZE(type)) - VA_SIZE(type)))

// va_end: Cleans up (does nothing here, but can be used for portability)
#define va_end(ap)(ap = (va_list)0)



void print_int(int n) {
    if (n == 0) {
        putchar('0');
        return;
    }

    char buffer[24];
    int i = 0;

    if (n < 0) {
        putchar('-');
        n = -n;
    }

    while (n > 0) {
        buffer[i++] = '0' + (n % 10);
        n /= 10;
    }

    // Print digits in reverse order
    while (i > 0) {
        putchar(buffer[--i]);
    }
}



void vprintf(const char* format, va_list args) {
    for (const char* p = format; *p != '\0'; p++) {
        if (*p == '%') {
            p++;  // Get next character after '%'

            switch (*p) {
                case 'd': {// integer
                    int num = va_arg(args, int);
                    print_int(num);
                    break;
                }
                case 's': { // string
                    const char* str = va_arg(args, const char*);
                    print(str);
                    break;
                }
                case 'c': { // character
                    char ch = (char) va_arg(args, int);
                    putchar(ch);
                    break;
                }
                case 'x': {  // Integer in hexadecimal
                    uint64_t num = va_arg(args, uint64_t);
                    print_hex(num);  // Use your custom print_hex function
                    break;
                }
                case 'b': {  // Integer in binary
                    uint64_t num = va_arg(args, uint64_t);
                    print_bin(num);  // Use your custom print_bin function
                    break;
                }
                case 'u':{ // for unsignen int i.e. uint32_t variable
                    uint32_t num = va_arg(args, uint32_t);
                    print_int(num);  // Use your custom print_hex function 
                    break;
                }
                case 'f': {  // Handle floating-point numbers
                    double num = va_arg(args, double);  // Floats are promoted to double in variadic arguments
                    int whole_part = (int)num;  // Get the integer part
                    int fraction_part = (int)((num - whole_part) * 100);  // Get 2 decimal places
                    
                    print_int(whole_part);  // Print the integer part
                    putchar('.');  // Print the decimal point
                    if (fraction_part < 0) {
                        fraction_part = -fraction_part;  // Handle negative fraction
                    }
                    print_int(fraction_part);  // Print the fractional part
                    break;
                }
                case '%': { // raw %
                    putchar('%');
                    break;
                }
                default:
                    putchar('%');
                    putchar(*p);
                    break;
            }
        } else {
            putchar(*p);
        }
    }
}



void printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

