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
    if (regs == NULL) {
        print("Error: registers_t pointer is NULL\n");
        return;
    }

    // Print segment registers
    print("Segment Registers:\n");
    print("  DS: "); print_hex(regs->ds); print("\n");
    print("  ES: "); print_hex(regs->es); print("\n");
    print("  FS: "); print_hex(regs->fs); print("\n");
    print("  GS: "); print_hex(regs->gs); print("\n");

    // Print general-purpose registers
    print("General Purpose Registers:\n");
    print("  R15: "); print_hex(regs->r15); print("\n");
    print("  R14: "); print_hex(regs->r14); print("\n");
    print("  R13: "); print_hex(regs->r13); print("\n");
    print("  R12: "); print_hex(regs->r12); print("\n");
    print("  R11: "); print_hex(regs->r11); print("\n");
    print("  R10: "); print_hex(regs->r10); print("\n");
    print("  R9:  "); print_hex(regs->r9); print("\n");
    print("  R8:  "); print_hex(regs->r8); print("\n");
    print("  RDI: "); print_hex(regs->rdi); print("\n");
    print("  RSI: "); print_hex(regs->rsi); print("\n");
    print("  RBP: "); print_hex(regs->rbp); print("\n");
    print("  RDX: "); print_hex(regs->rdx); print("\n");
    print("  RCX: "); print_hex(regs->rcx); print("\n");
    print("  RBX: "); print_hex(regs->rbx); print("\n");
    print("  RAX: "); print_hex(regs->rax); print("\n");

    // Print interrupt information
    print("Interrupt Info:\n");
    print("  Interrupt Number: "); print_dec(regs->int_no); print("\n");
    print("  Error Code:       "); print_hex(regs->err_code); print("\n");

    // Print CPU state at interrupt return
    print("CPU State:\n");
    print("  RIP: "); print_hex(regs->iret_rip); print("\n");
    print("  CS:  "); print_hex(regs->iret_cs); print("\n");
    print("  RFLAGS: "); print_hex(regs->iret_rflags); print("\n");
    print("  RSP: "); print_hex(regs->iret_rsp); print("\n");
    print("  SS:  "); print_hex(regs->iret_ss); print("\n");
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

