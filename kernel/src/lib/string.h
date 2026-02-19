#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


void *memcpy(void *dest, const void *src, size_t n);    // Copy memory from src to dest
void *memset(void *s, int c, size_t n);                 // Set first n bytes of memory s with value c 
void *memmove(void *dest, const void *src, size_t n);   // moving first n bytes from src to dest
int   memcmp(const void *s1, const void *s2, size_t n); // Compare first n bytes of memory s1 and s2
void *memchr(const void *s, int c, size_t n);           // Search c in first n bytes of pointer s


void int_to_ascii(int n, char str[]);   // Integer to ASCII Code
void reverse(char s[]);                 // String reverse
int strlen(char s[]);                   // Length of the string
void append(char s[], char c);          // append character c in string s
void backspace(char s[]);               // Remove last character from string

int strcmp(char s1[], char s2[]);           // Compare two string: return 0 if both are same otherwise -ve or +ve number
char *strcpy(char *dest, const char *src);  // Copy string from src to dest
char *strncpy(char *dest, const char *src, size_t n);   // copy n character from src into dest
int strncmp(const char* s1, const char* s2, unsigned int n);    // compare n character of s1 and s2
char* strcat(char* dest, const char* src);  //  is used in C to concatenate (i.e., append) one string (src) to the end of another string (dest).
char* strncat(char* dest, const char* src, size_t n);   //  function is a safer version of strcat() used to concatenate strings in C — it appends at most n characters from src to dest.
int strnlen(const char* str, size_t maxlen);            // Safer version of strlen this function return length upto maximum limit this function is usefull if striing do not have null termitter 
char* strchr(const char* str, int c);       // The function char* strchr(const char* str, int c); is used to find the first occurrence of a character c in the string str.

char *strtok(char *str, const char *delim); // is used in C to split a string into tokens (substrings) based on a set of delimiter characters.
char *strrchr(const char *str, int c);      // is used to find the last occurrence of a character in a C string

char *strdup(const char *str);              // Duplicate string 

void clear_buffer(char *buffer, int size);  // Clearing buffer
void int_to_str(int num, char* buffer);     // Converting integer to buffer

void int_to_base_str(unsigned int num, char* buffer, int base);

