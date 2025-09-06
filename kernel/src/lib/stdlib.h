#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>


int abs(int value) ;
double atof(const char* str);   // converting string to double
int atoi(const char* str);      // converting string to integer

// Converts a signed integer (int) into a null-terminated string (char*) representation, 
// in the given numerical base (e.g., binary, decimal, hex).
char* itoa(int value, char* str, int base); 

// Converts an unsigned integer (unsigned int) to a null-terminated string in the specified base.
char *utoa(unsigned int value, char* str, int base);

