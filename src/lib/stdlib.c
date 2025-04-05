#include "stdlib.h"



int abs(int value) {
    return (value < 0) ? -value : value;
}


// converting string to double
double atof(const char* str) {
    double result = 0.0;
    double fraction = 0.0;
    int divisor = 1;
    int sign = 1;

    // Skip whitespaces
    while (*str == ' ' || *str == '\t') {
        str++;
    }

    // Handle sign
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }

    // Parse integer part
    while (*str >= '0' && *str <= '9') {
        result = result * 10.0 + (*str - '0');
        str++;
    }

    // Parse fractional part
    if (*str == '.') {
        str++;
        while (*str >= '0' && *str <= '9') {
            fraction = fraction * 10.0 + (*str - '0');
            divisor *= 10;
            str++;
        }
        result += fraction / divisor;
    }

    return sign * result;
}

// converting string to integer
int atoi(const char* str) {
    int result = 0;
    int sign = 1;

    // Skip whitespace
    while (*str == ' ' || *str == '\t' || *str == '\n') {
        str++;
    }

    // Handle optional '+' or '-' sign
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }

    // Process digits
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }

    return result * sign;
}
