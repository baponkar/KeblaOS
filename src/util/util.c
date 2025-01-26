/*
    The common function is present in this file.
*/

#include "../driver/vga.h"

#include "util.h"



// Halt and catch fire function.
void halt_kernel(void) {
    for (;;) {
        asm ("hlt");
    }
}

void print_regs_content(registers_t *regs) {
    // Print segment registers
    print("  gs: "); print_hex(regs->gs); print("\n");
    print("  fs: "); print_hex(regs->fs); print("\n");
    print("  es: "); print_hex(regs->es); print("\n");
    print("  ds: "); print_hex(regs->ds); print("\n");

    // Print general-purpose registers
    print("  rax: "); print_hex(regs->rax); print("\n");
    print("  rbx: "); print_hex(regs->rbx); print("\n");
    print("  rcx: "); print_hex(regs->rcx); print("\n");
    print("  rdx: "); print_hex(regs->rdx); print("\n");
    print("  rbp: "); print_hex(regs->rbp); print("\n");
    print("  rdi: "); print_hex(regs->rdi); print("\n");
    print("  rsi: "); print_hex(regs->rsi); print("\n");
    // print("  r8:  "); print_hex(regs->r8); print("\n");
    // print("  r9:  "); print_hex(regs->r9); print("\n");
    // print("  r10: "); print_hex(regs->r10); print("\n");
    // print("  r11: "); print_hex(regs->r11); print("\n");
    // print("  r12: "); print_hex(regs->r12); print("\n");
    // print("  r13: "); print_hex(regs->r13); print("\n");
    // print("  r14: "); print_hex(regs->r14); print("\n");
    // print("  r15: "); print_hex(regs->r15); print("\n");

    // Print interrupt number and error code
    // print("  int_no:   "); print_dec(regs->int_no); print("\n");
    // print("  err_code: "); print_dec(regs->err_code); print("\n");

    // Print CPU state (iret context)
    print("  iret_ss:    "); print_hex(regs->iret_ss); print("\n");
    print("  iret_rsp:   "); print_hex(regs->iret_rsp); print("\n");
    print("  iret_rflags:"); print_hex(regs->iret_rflags); print("\n");
    print("  iret_cs:    "); print_hex(regs->iret_cs); print("\n");
    print("  iret_rip:   "); print_hex(regs->iret_rip); print("\n");
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

