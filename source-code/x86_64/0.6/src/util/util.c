#include "util.h"



// Function to disable interrupts
void disable_interrupts() {
    asm volatile("cli"); // Clear the interrupt flag
}

// Function to enable interrupts
void enable_interrupts() {
    asm volatile("sti"); // Set the interrupt flag
}


