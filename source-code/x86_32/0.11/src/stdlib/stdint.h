/*
 ____________________________________________________________________________________________________
|----------------------------------------------------------------------------------------------------|
| Description : Standard Integers                                                                    |
| Developed By : Bapon Kar                                                                           |
| Credits :                                                                                          |
| 1. https://web.archive.org/web/20160412174753/http://www.jamesmolloy.co.uk/tutorial_html/index.html|
| 2. http://www.osdever.net/tutorials/view/brans-kernel-development-tutorial                         |
|____________________________________________________________________________________________________|
*/

#pragma once

// Some nice typedefs, to standardise sizes across platforms.
// These typedefs are written for 32-bit X86.

// db equivalent
typedef signed char int8_t;
typedef unsigned char uint8_t;


// dw equivalent
typedef signed short int16_t;
typedef unsigned short uint16_t;


// dd equivalent
typedef signed long int int32_t;
typedef unsigned long int uint32_t;


// dq equivalent
typedef signed long long int int64_t;
typedef unsigned long long int uint64_t;


// Boolean variable
typedef uint8_t bool;
#define true 1
#define false 0

