/*

Standard Input Output library

Last Updated : 26/07/2025
Author       : Baponkar

*/


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


char* itoa(int value, char* str, int base) {
    char *ptr = str, *ptr1 = str, tmp_char;
    int tmp_value;
    int is_negative = 0;

    if (value == 0) {
        *ptr++ = '0';
        *ptr = '\0';
        return str;
    }

    if (value < 0 && base == 10) {
        is_negative = 1;
        value = -value;
    }

    while (value != 0) {
        tmp_value = value % base;
        *ptr++ = (tmp_value < 10) ? (tmp_value + '0') : (tmp_value - 10 + 'a');
        value /= base;
    }

    if (is_negative)
        *ptr++ = '-';

    *ptr-- = '\0';

    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }

    return str;
}

char *utoa(unsigned int value, char* str, int base) {
    char *ptr = str, *ptr1 = str, tmp_char;
    unsigned int tmp_value;

    if (value == 0) {
        *ptr++ = '0';
        *ptr = '\0';
        return str;
    }

    while (value != 0) {
        tmp_value = value % base;
        *ptr++ = (tmp_value < 10) ? (tmp_value + '0') : (tmp_value - 10 + 'a');
        value /= base;
    }

    *ptr-- = '\0';

    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }

    return str;
}
