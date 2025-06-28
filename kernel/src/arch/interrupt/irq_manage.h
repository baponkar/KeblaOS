#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "../../util/util.h"

// Defined in irq
extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();    // FPU / Floating-Point Unit (Coprocessor), IRQ13
extern void irq14();    // Primary ATA Hard Disk Controller, IRQ14
extern void irq15();    // Secondary ATA Hard Disk Controller, IRQ15

extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();    // FPU / Floating-Point Unit (Coprocessor), IRQ13
extern void irq14();    // Primary ATA Hard Disk Controller, IRQ14
extern void irq15();    // Secondary ATA Hard Disk Controller, IRQ15

extern void irq16();    // APIC Timer
extern void irq17();    // HPET Timer
extern void irq18(); 

extern void irq19();    // IPI (Inter-Processor Interrupt)
extern void irq20();    // Open System Call
extern void irq21();    // Close System Call
extern void irq22();    // Read System Call
extern void irq23();    // Write System Call
extern void irq24();    // Lseek System Call
extern void irq25();    // Truncate System Call
extern void irq26();    // Sync System Call
extern void irq27();    // Forward System Call
extern void irq28();    // Expand System Call
extern void irq29();    // Gets System Call
extern void irq30();    // Puts System Call
extern void irq31();    // Error System Call
extern void irq32();    // FatFs Error System Call
extern void irq33();    // FatFs Open System Call
extern void irq34();    // FatFs Close System Call
extern void irq35();    // FatFs Read System Call
extern void irq36();    // FatFs Write System Call
extern void irq37();    // FatFs Lseek System Call
extern void irq38();    // FatFs Truncate System Call
extern void irq39();    // FatFs Sync System Call
extern void irq40();    // FatFs Forward System Call
extern void irq41();    // FatFs Expand System Call
extern void irq42();    // FatFs Gets System Call
extern void irq43();    // FatFs Puts System Call
extern void irq44();    // FatFs Error System Call  
extern void irq45();    // FatFs Mount System Call
extern void irq46();    // FatFs Unmount System Call
extern void irq47();    // FatFs Format System Call
extern void irq48();    // FatFs List System Call
extern void irq49();    // FatFs Create System Call
extern void irq50();    // FatFs Delete System Call
extern void irq51();    // FatFs Rename System Call
extern void irq52();    // FatFs Copy System Call
extern void irq53();    // FatFs Move System Call
extern void irq54();    // FatFs Get Info System Call
extern void irq55();    // FatFs Set Info System Call
extern void irq56();    // FatFs Get Attributes System Call

extern void irq57();   // Print System Call
extern void irq58();   // Read System Call
extern void irq59();   // Exit System Call
extern void irq60();   // Printing rax content system call



void irq_handler(registers_t *regs);
void irq_install(int irq_no, void (*handler)(registers_t *r));
void irq_uninstall(int irq_no);

