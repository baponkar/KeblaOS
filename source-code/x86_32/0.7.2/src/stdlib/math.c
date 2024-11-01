#include "math.h"


// Function to calculate the absolute value of an integer
int abs(int x) {
    return (x < 0) ? -x : x;
}

// Function to calculate the absolute value of a floating-point number
double fabs(double x) {
    return (x < 0.0) ? -x : x;
}