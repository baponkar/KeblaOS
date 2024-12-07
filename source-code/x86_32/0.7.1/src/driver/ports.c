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