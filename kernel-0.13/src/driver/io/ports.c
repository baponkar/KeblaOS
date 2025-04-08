/*
 ____________________________________________________________________________________________________
|----------------------------------------------------------------------------------------------------|
| Description : Getting input value and send output value through differents ports.                  |
| Developed By : Bapon Kar                                                                           |
| Credits :                                                                                          |
| 1. https://web.archive.org/web/20160412174753/http://www.jamesmolloy.co.uk/tutorial_html/index.html|
| 2. http://www.osdever.net/tutorials/view/brans-kernel-development-tutorial                         |
| 3. https://wiki.osdev.org/Port_IO                                                                  |
|____________________________________________________________________________________________________|

               
                          
Byte( 8 Bit)---> Word( 16 Byte)---> Double Word( 32 Bit)-----> Quad Word( 64 Bit)

*/

#include "ports.h"

// Write a byte (8-bit) to the specified port.
void outb(uint16_t port, uint8_t value) {
    asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

// Read a byte (8-bit) from the specified port.
uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Write a word (16-bit) to the specified port.
void outw(uint16_t port, uint16_t value) {
    asm volatile ("outw %0, %1" : : "a"(value), "Nd"(port));
}

// Read a word (16-bit) from the specified port.
uint16_t inw(uint16_t port) {
    uint16_t ret;
    asm volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Write a double word (32-bit) to the specified port.
void outl(uint16_t port, uint32_t value) {
    asm volatile("outl %0, %1" : : "a"(value), "Nd"(port));
}

// Read a double word (32-bit) from the specified port.
uint32_t inl(uint16_t port) {
    uint32_t value;
    asm volatile("inl %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

// Write a quad word (64-bit) to a pair of ports (if supported by hardware).
void outq(uint16_t port, uint64_t value) {
    outl(port, (uint32_t)(value & 0xFFFFFFFF));   // Lower 32 bits
    outl(port + 4, (uint32_t)(value >> 32));      // Upper 32 bits
}

// Read a quad word (64-bit) from a pair of ports (if supported by hardware).
uint64_t inq(uint16_t port) {
    uint32_t lower = inl(port);      // Read lower 32 bits
    uint32_t upper = inl(port + 4);  // Read upper 32 bits
    return ((uint64_t)upper << 32) | lower;
}

