
#include "../include/math.h"


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



// Define PI (more accurate version)
#define PI 3.14159265358979323846

// Helper: convert degrees to radians
double to_radians(double degrees) {
    return degrees * (PI / 180.0);
}

// Helper: factorial (for Taylor series)
double factorial(int n) {
    double result = 1.0;
    for (int i = 2; i <= n; ++i) {
        result *= i;
    }
    return result;
}

// Taylor series approximation for sin(x)
double sin(double x) {
    double result = 0.0;
    int terms = 10; // More terms = better precision

    for (int n = 0; n < terms; ++n) {
        double power = 2 * n + 1;
        double sign = (n % 2 == 0) ? 1.0 : -1.0;
        result += sign * (power_func(x, power) / factorial(power));
    }
    return result;
}

// Taylor series approximation for cos(x)
double cos(double x) {
    double result = 0.0;
    int terms = 10;

    for (int n = 0; n < terms; ++n) {
        double power = 2 * n;
        double sign = (n % 2 == 0) ? 1.0 : -1.0;
        result += sign * (power_func(x, power) / factorial(power));
    }
    return result;
}

// tan(x) = sin(x) / cos(x)
double tan(double x) {
    double cos_val = cos(x);
    if (cos_val == 0.0) {
        // Very large number to simulate infinity
        return 1e30;
    }
    return sin(x) / cos_val;
}

// Helper: power function (x^n)
double power_func(double base, double exponent) {
    double result = 1.0;
    for (int i = 0; i < (int)exponent; ++i) {
        result *= base;
    }
    return result;
}
