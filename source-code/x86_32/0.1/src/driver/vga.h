// monitor.h -- Defines the interface for monitor.h
// From JamesM's kernel development tutorials.

#ifndef VGA_H
#define VGA_H

#include "../stdlib/stdint.h"



// Write a single character out to the screen.
void monitor_put(char c);

// Clear the screen to all black.
void monitor_clear();

// Output a null-terminated ASCII string to the monitor.
void monitor_write(char *c);

// Write a Hex number to the screen
void monitor_write_hex(u32int n);

// Write a Decimal number to the screen
void monitor_write_dec(u32int n);

#endif // VGA_H