/*
    ACPI : Advanced Configuration and Power Interface
    https://github.com/dreamportdev/Osdev-Notes/blob/master/02_Architecture/06_ACPITables.md
*/

#include "acpi.h"


#define PM1A_CNT_REG  0x604  // Default address of PM1a_CNT for many systems
#define SLP_EN        (1 << 13)  // Bit 13: SLP_EN (Sleep Enable)
#define S5_SLEEP_TYPE (5 << 10)  // Sleep type S5 (5) in bits 10-12




void qemu_poweroff() {
    // Combine S5 Sleep Type and Sleep Enable into a single command
    uint16_t poweroff_cmd = S5_SLEEP_TYPE | SLP_EN; // 0x1400 + 0x2000 = 0x3400

    // Write to the PM1a_CNT register
    outw(PM1A_CNT_REG, poweroff_cmd); // QEMU-specific ACPI shutdown port

    // If the system fails to power off, hang the CPU
    while (1) {
        __asm__ volatile ("hlt");
    }
}




void qemu_reboot(){
    outb(0x64, 0xFE);   // Send reset command to the keyboard controller
}