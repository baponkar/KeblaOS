/*
 ____________________________________________________________________________________________________
|----------------------------------------------------------------------------------------------------|
| Description : Getting input value and send output value through differents ports.                  |
| Developed By : Bapon Kar                                                                           |
| Credits :                                                                                          |
| 1. https://web.archive.org/web/20160412174753/http://www.jamesmolloy.co.uk/tutorial_html/index.html|
| 2. http://www.osdever.net/tutorials/view/brans-kernel-development-tutorial                         |
|____________________________________________________________________________________________________|
*/

#include "ports.h"

// Write a byte out to the specified port.
void outb(uint16_t port, uint8_t value)
{
    asm volatile ("out %%al, %%dx" : : "a" (value), "d" (port));
}

// Getting a byte value through a port
uint8_t inb(uint16_t port)
{
   uint8_t ret;
   asm volatile ("in %%dx, %%ax" : "=a" (ret) : "d" (port));
   return ret;
}




// Write a word(2 byte) out to the specified port.
void outw(uint16_t port, uint16_t value)
{
    asm volatile ("out %%al, %%dx" : : "a" (value), "d" (port));
}

// Getting a word(2 byte) value through a port
uint16_t inw(uint16_t port)
{
   uint16_t ret;
   asm volatile ("in %%dx, %%ax" : "=a" (ret) : "d" (port));
   return ret;
}





// Write 4 byte(dword) value through a port
void outl(uint16_t port, uint32_t value) {
    asm volatile("outl %0, %1" : : "a"(value), "Nd"(port));
}

// Getting a 4 byte(dword) value through a port
uint32_t inl(uint16_t port) {
    uint32_t value;
    asm volatile("inl %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}




// Write a 8 byte(quad word) value through a port
void outq(uint16_t port, uint64_t value) {
    // Write the lower 32 bits
    asm volatile("outl %0, %1" : : "a"((uint32_t)(value & 0xFFFFFFFF)), "Nd"(port));
    // Write the upper 32 bits
    asm volatile("outl %0, %1" : : "a"((uint32_t)(value >> 32)), "Nd"(port + 4));
}


// Getting a 8 byte(quad word) value through a port
uint64_t inq(uint16_t port) {
    uint32_t lower, upper;
    asm volatile("inl %1, %0" : "=a"(lower) : "Nd"(port));        // Read lower 32 bits
    asm volatile("inl %1, %0" : "=a"(upper) : "Nd"(port));        // Read upper 32 bits
    return ((uint64_t)upper << 32) | lower;  // Combine the upper and lower 32 bits
}

