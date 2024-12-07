#include "math.h"

// Function to calculate the absolute value of an integer
int abs(int x) {
    return (x < 0) ? -x : x;
}

// Function to calculate the absolute value of a floating-point number
double fabs(double x) {
    return (x < 0.0) ? -x : x;
}

// Function to find the minimum of two integers
int min_int(int a, int b) {
    return (a < b) ? a : b;
}

// Function to find the maximum of two integers
int max_int(int a, int b) {
    return (a > b) ? a : b;
}

// Function to calculate the square of an integer
int square_int(int x) {
    return x * x;
}

// Function to calculate the power of an integer (base^exp)
int power_int(int base, int exp) {
    if (exp < 0) {
        // Typically, negative exponents would return a floating-point number.
        // Since we're dealing with integers, return 0 or handle as needed.
        return 0;
    }
    
    int result = 1;
    for(int i = 0; i < exp; i++) {
        result *= base;
    }
    return result;
}


// Function to calculate the factorial of a non-negative integer
unsigned long factorial(unsigned int n) {
    if(n == 0 || n == 1)
        return 1;
    
    unsigned long result = 1;
    for(unsigned int i = 2; i <= n; i++) {
        result *= i;
    }
    return result;
}

// Function to calculate the nth Fibonacci number
unsigned long fibonacci(unsigned int n) {
    if(n == 0)
        return 0;
    if(n == 1)
        return 1;
    
    unsigned long a = 0;
    unsigned long b = 1;
    unsigned long temp;
    
    for(unsigned int i = 2; i <= n; i++) {
        temp = a + b;
        a = b;
        b = temp;
    }
    return b;
}

// Function to calculate the Greatest Common Divisor (GCD) of two integers
int gcd(int a, int b) {
    if (b == 0)
        return a;
    return gcd(b, a % b);
}


// Function to calculate the Least Common Multiple (LCM) of two integers
int lcm(int a, int b) {
    if(a == 0 || b == 0)
        return 0;
    return (abs(a) / gcd(a, b)) * abs(b);
}

// Approximation of sine function using Taylor series
double sin_double(double x) {
    // Normalize the angle to the range [-PI, PI]
    while (x > PI) x -= TWO_PI;
    while (x < -PI) x += TWO_PI;

    double term = x;  // First term
    double result = term;
    double x_squared = x * x;

    // Iterate through Taylor series expansion
    for (int n = 1; n <= 10; n++) {
        term *= -x_squared / ((2 * n) * (2 * n + 1));
        result += term;
    }
    return result;
}


// Approximation of cosine function using Taylor series
double cos_double(double x) {
    // Normalize the angle to the range [-PI, PI]
    while (x > PI) x -= TWO_PI;
    while (x < -PI) x += TWO_PI;

    double term = 1;  // First term
    double result = term;
    double x_squared = x * x;

    // Iterate through Taylor series expansion
    for (int n = 1; n <= 10; n++) {
        term *= -x_squared / ((2 * n - 1) * (2 * n));
        result += term;
    }
    return result;
}


// Tangent function calculated as sin(x) / cos(x)
double tan_double(double x) {
    return sin_double(x) / cos_double(x);
}


// Approximation of arctangent function
double atan_double(double x) {
    double result = x;
    double term = x;
    double x_squared = x * x;

    for (int n = 1; n <= 10; n++) {
        term *= -x_squared;
        result += term / (2 * n + 1);
    }
    return result;
}


// Approximation of exponential function using power series
double exp_double(double x) {
    double result = 1.0;
    double term = 1.0;

    for (int n = 1; n <= 20; n++) {
        term *= x / n;
        result += term;
    }
    return result;
}


// Approximation of natural logarithm using Newton's method
double log_double(double x) {
    if (x <= 0) return -1; // Undefined for x <= 0

    double result = 0;
    double term = (x - 1) / (x + 1);
    double term_squared = term * term;

    for (int n = 1; n <= 20; n += 2) {
        result += (1.0 / n) * term;
        term *= term_squared;
    }
    return 2 * result;
}

// Approximation of square root using Newton's method
double sqrt_double(double x) {
    if (x < 0) return -1; // Return -1 for negative numbers, as sqrt is undefined

    double guess = x / 2.0;
    double result;

    for (int i = 0; i < 20; i++) {
        result = 0.5 * (guess + (x / guess));
        if (fabs(result - guess) < EPSILON)
            break;
        guess = result;
    }
    return result;
}


