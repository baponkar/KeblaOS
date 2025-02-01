#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "math.h"

// Absolute value of an integer
int abs(int x) {
    return (x < 0) ? -x : x;
}

// Power function (integer exponentiation)
double pow(double base, int exp) {
    double result = 1.0;
    if (exp < 0) {
        base = 1.0 / base;
        exp = -exp;
    }
    for (int i = 0; i < exp; i++) {
        result *= base;
    }
    return result;
}

// Square root approximation using Newton's method
double sqrt(double x) {
    if (x < 0) return -1.0; // Error: negative input
    double guess = x;
    double epsilon = 0.00001;
    while ((guess * guess - x) > epsilon || (x - guess * guess) > epsilon) {
        guess = (guess + x / guess) / 2.0;
    }
    return guess;
}

// Integer greatest common divisor (GCD) using Euclidean algorithm
int gcd(int a, int b) {
    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

// Floating-point modulo
double fmod(double x, double y) {
    return x - ((int)(x / y) * y);
}

