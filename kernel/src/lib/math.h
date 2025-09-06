
#pragma once

// Mathematical constants
#define M_E        2.71828182845904523536   // e
#define M_LOG2E    1.44269504088896340736   // log2(e)
#define M_LOG10E   0.43429448190325182765   // log10(e)
#define M_LN2      0.69314718055994530942   // ln(2)
#define M_LN10     2.30258509299404568402   // ln(10)
#define M_PI       3.14159265358979323846   // pi
#define M_PI_2     1.57079632679489661923   // pi/2
#define M_PI_4     0.78539816339744830962   // pi/4
#define M_1_PI     0.31830988618379067154   // 1/pi
#define M_2_PI     0.63661977236758134308   // 2/pi
#define M_2_SQRTPI 1.12837916709551257390   // 2/sqrt(pi)
#define M_SQRT2    1.41421356237309504880   // sqrt(2)
#define M_SQRT1_2  0.70710678118654752440   // 1/sqrt(2)

// Macros for common math operations
#define ABS(x)     ((x) < 0 ? -(x) : (x))
#define MIN(a, b)  ((a) < (b) ? (a) : (b))
#define MAX(a, b)  ((a) > (b) ? (a) : (b))
#define CLAMP(x, min, max) (MAX((min), MIN((x), (max))))
#define SIGN(x)    ((x) > 0 ? 1 : ((x) < 0 ? -1 : 0))
#define SQUARE(x)  ((x) * (x))

// Degree-radian conversions
#define DEG2RAD(d) ((d) * (M_PI / 180.0))
#define RAD2DEG(r) ((r) * (180.0 / M_PI))


double pow(double base, int exp);
double sqrt(double x);
int gcd(int a, int b);
double fmod(double x, double y);

double to_radians(double degrees);
double factorial(int n);
double sin(double x);
double cos(double x);
double tan(double x);
double power_func(double base, double exponent);



