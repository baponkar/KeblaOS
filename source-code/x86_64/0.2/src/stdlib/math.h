#pragma once

#include "stdint.h"

#define PI 3.141592653589793
#define HALF_PI (PI / 2)
#define TWO_PI (2 * PI)
#define EPSILON 1e-9 // Precision for approximations

// Function to calculate the absolute value of an integer
int abs(int x);

// Function to calculate the absolute value of a floating-point number
double fabs(double x);

// Function to find the minimum of two integers
int min_int(int a, int b);

// Function to find the maximum of two integers
int max_int(int a, int b);

// Function to calculate the square of an integer
int square_int(int x);

// Function to calculate the power of an integer (base^exp)
int power_int(int base, int exp);

// Function to calculate the factorial of a non-negative integer
unsigned long factorial(unsigned int n);

// Function to calculate the nth Fibonacci number
unsigned long fibonacci(unsigned int n);

// Function to calculate the Greatest Common Divisor (GCD) of two integers
int gcd(int a, int b);

// Function to calculate the Least Common Multiple (LCM) of two integers
int lcm(int a, int b);

// Advanced math functions
double sin_double(double x);
double cos_double(double x);
double tan_double(double x);
double exp_double(double x);
double log_double(double x);
double sqrt_double(double x);
double pow_double(double base, double exp);
double atan_double(double x);



