
#include "../include/ctype.h"

// This file implements the ctype functions for character classification
// and conversion as per the C standard library.

// The functions in this file are used to determine the type of a character
int isalpha(int c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

// The isdigit function checks if the character is a digit (0-9)
int isdigit(int c) {
    return (c >= '0' && c <= '9');
}

// The isalnum function checks if the character is either an alphabetic
int isalnum(int c) {
    return isalpha(c) || isdigit(c);
}

// The isspace function checks if the character is a whitespace character
int isspace(int c) {
    return c == ' '  || c == '\t' || c == '\n' ||
           c == '\v' || c == '\f' || c == '\r';
}

// The isupper function checks if the character is an uppercase letter
int isupper(int c) {
    return (c >= 'A' && c <= 'Z');
}

// The islower function checks if the character is a lowercase letter
int islower(int c) {
    return (c >= 'a' && c <= 'z');
}

// The tolower function converts an uppercase letter to lowercase
int tolower(int c) {
    if (c >= 'A' && c <= 'Z')
        return c + ('a' - 'A');
    return c;
}

// The toupper function converts a lowercase letter to uppercase
int toupper(int c) {
    if (c >= 'a' && c <= 'z')
        return c - ('a' - 'A');
    return c;
}

