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
                case 'd': {
                    int num = va_arg(args, int);
                    print_int(num);
                    break;
                }
                case 's': {
                    const char* str = va_arg(args, const char*);
                    print(str);
                    break;
                }
                case 'c': {
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
                case '%': {
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
