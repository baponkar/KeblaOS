/*
    The common function is present in this file.
*/

#include "util.h"



// Halt and catch fire function.
void halt_kernel(void) {
    for (;;) {
        asm ("hlt");
    }
}


void print_size_with_units(uint64_t size) {
    const char *units[] = {"Bytes", "KB", "MB", "GB", "TB"};
    int unit_index = 0;

    // Determine the appropriate unit
    while (size >= 1024 && unit_index < 4) {
        size /= 1024;
        unit_index++;
    }

    // Print the size with the unit
    print_dec((uint64_t)size); // Print the integer part
    print(" ");
    print(units[unit_index]);
}

