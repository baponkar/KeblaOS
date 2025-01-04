/*

Reference: https://github.com/dreamportdev/Osdev-Notes/blob/master/02_Architecture/06_ACPITables.md

*/

#include "acpi.h"


bool validate_RSDP(char *byte_array, size_t size) {
    uint32_t sum = 0;
    for(int i = 0; i < size; i++) {
        sum += byte_array[i];
    }
    return (sum & 0xFF) == 0;
}


#define PM1A_CNT_REG  0x604  // Default address of PM1a_CNT for many systems
#define SLP_EN        (1 << 13)  // Bit 13: SLP_EN (Sleep Enable)
#define S5_SLEEP_TYPE (5 << 10)  // Sleep type S5 (5) in bits 10-12



void poweroff() {
    // Combine S5 Sleep Type and Sleep Enable into a single command
    uint16_t poweroff_cmd = S5_SLEEP_TYPE | SLP_EN;

    // Write to the PM1a_CNT register
    outw(PM1A_CNT_REG, poweroff_cmd);

    // If the system fails to power off, hang the CPU
    while (1) {
        __asm__ volatile ("hlt");
    }
}

void qemu_poweroff() {
    outw(0x604, 0x2000);  // QEMU-specific ACPI shutdown port
}



void reboot(){
    outb(0x64, 0xFE);   // Send reset command to the keyboard controller
}



