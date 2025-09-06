/*
    The common function is present in this file.
*/

#include "../lib/stdio.h"

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
    printf("%d %s", (uint64_t)size, units[unit_index]); // Print the integer part
}


uint64_t read_rip() {
    uint64_t rip;
    __asm__ volatile ("lea (%%rip), %0" : "=r"(rip));
    return rip;
}

uint64_t read_rsp() {
    uint64_t rsp;
    __asm__ volatile ("movq %%rsp, %0" : "=r"(rsp));
    return rsp;
}

uint64_t read_rflags() {
    uint64_t flags;
    __asm__ volatile ("pushfq; pop %0" : "=r"(flags));
    return flags;
}

void set_rip(uint64_t rip) {
    __asm__ volatile ("jmp *%0" : : "r"(rip));
}

void set_rsp(uint64_t rsp) {
    __asm__ volatile ("movq %0, %%rsp" : : "r"(rsp));
}

void set_rflags(uint64_t flags) {
    __asm__ volatile ("push %0; popfq" : : "r"(flags));
}
