#include "stdio.h"

void print_int(int n);
void vprintf(const char* format, va_list args);

void print_int(int n) {
    if (n == 0) {
        putchar('0');
        return;
    }

    char buffer[11];
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
                    uint32_t num = va_arg(args, uint32_t);
                    print_hex(num);  // Use your custom print_hex function
                    break;
                }
                case 'b': {  // Integer in binary
                    uint32_t num = va_arg(args, uint32_t);
                    print_bin(num);  // Use your custom print_bin function
                    break;
                }
                case 'u':{ // for unsignen int i.e. uint32_t variable
                    uint32_t num = va_arg(args, uint32_t);
                    print_dec(num);  // Use your custom print_hex function 
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
