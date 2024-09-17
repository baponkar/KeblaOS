
#include "math.h"


static double sqrt_helper(double x, double guess) {
    double new_guess = (guess + x / guess) / 2;
    if (guess == new_guess) {
        return new_guess;
    } else {
        return sqrt_helper(x, new_guess);
    }
}

double sqrt(double x) {
    if (x < 0) {
        // Return NaN for negative inputs (not handling complex numbers)
        return 0.0 / 0.0; // NaN (Not a Number)
    }
    if (x == 0) {
        return 0;
    }
    return sqrt_helper(x, x);
}
