#pragma once

#include "stdint.h"
#include "../driver/vga.h"
#include "stdio.h"

void panic(const char *message);

#define ASSERT(condition)                \
    do {                                 \
        if (!(condition)) {              \
            panic("Assertion failed: " #condition); \
        }                                \
    } while (0)

void panic(const char *message) {
    // Handle the error (e.g., print the message and halt the system)
    errprint(message);
    // Implement your panic handling logic here
    while (1); // Infinite loop to halt execution
}
