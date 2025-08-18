#pragma once

// Number of bits in a char (always 8 in modern systems)
#define CHAR_BIT 8

// char limits
#define SCHAR_MIN (-128)
#define SCHAR_MAX 127
#define UCHAR_MAX 255

// Check if char is signed by default (you can adjust this based on your compiler)
#define CHAR_MIN SCHAR_MIN
#define CHAR_MAX SCHAR_MAX

// short limits (16-bit)
#define SHRT_MIN (-32768)
#define SHRT_MAX 32767
#define USHRT_MAX 65535

// int limits (32-bit)
#define INT_MIN (-2147483648)
#define INT_MAX 2147483647
#define UINT_MAX 4294967295U

// long limits (assuming 64-bit long in x86_64)
#define LONG_MIN (-9223372036854775807L - 1)
#define LONG_MAX 9223372036854775807L
#define ULONG_MAX 18446744073709551615UL

// long long limits (assuming 64-bit)
#define LLONG_MIN (-9223372036854775807LL - 1)
#define LLONG_MAX 9223372036854775807LL
#define ULLONG_MAX 18446744073709551615ULL

// Floating-point types are optional unless you use them in kernel
// If used, you can include something like:
#define FLT_MIN 1.17549435e-38F
#define FLT_MAX 3.40282347e+38F
#define DBL_MIN 2.2250738585072014e-308
#define DBL_MAX 1.7976931348623157e+308
#define LDBL_MIN 3.36210314311209350626e-4932L
#define LDBL_MAX 1.18973149535723176502e+4932L

#define FLT_DIG 6
#define DBL_DIG 15
#define LDBL_DIG 18
