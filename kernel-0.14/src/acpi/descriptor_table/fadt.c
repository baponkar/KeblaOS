
#include "../../lib/stdio.h"
#include "../acpi.h"
#include "../../driver/io/ports.h"

#include "fadt.h"


#define PM1A_CNT_REG  0x604  // Default address of PM1a_CNT for many systems
#define SLP_EN (1 << 13)     // Bit 13: SLP_EN (Sleep Enable)
#define S5_SLEEP_TYPA (5 << 10)  // Sleep type S5 (5) in bits 10-12

extern fadt_t *fadt_addr; // defined in acpi.c

void qemu_poweroff() {
    // Write to the PM1a_CNT register
    outw(PM1A_CNT_REG, S5_SLEEP_TYPA | SLP_EN); // QEMU-specific ACPI shutdown port

    // If the system fails to power off, hang the CPU
    printf("[Info] ACPI Shutdown failed, halting system!\n");
    while (1) {
        asm volatile ("hlt");
    }
}


void qemu_reboot(){
    outb(0x64, 0xFE);   // Send reset command to the keyboard controller
}


void parse_fadt(){
    
}

void acpi_poweroff() {
    fadt_t *fadt = (fadt_t *) fadt_addr;
    if (!fadt) {
        printf("[Info] FADT not found, ACPI shutdown unavailable!\n");
        return;
    }

    // Enable ACPI first (if needed)
    if(!is_acpi_enabled()){
        acpi_enable();
    }

    uint32_t pm1a_control = (fadt->header.revision >= 2 && fadt->X_PM1aControlBlock.Address) ? (uint32_t)fadt->X_PM1aControlBlock.Address : fadt->PM1aControlBlock;
    uint32_t pm1b_control = (fadt->header.revision >= 2 && fadt->X_PM1bControlBlock.Address) ? (uint32_t)fadt->X_PM1bControlBlock.Address : fadt->PM1bControlBlock;

    if (!pm1a_control) {
        printf("[Info] PM1a Control Block not found!\n");
        return;
    }

    printf("[Info] Sending ACPI shutdown command: outw(%x, %x)\n", pm1a_control, S5_SLEEP_TYPA | SLP_EN);

    // Shutdown by setting SLP_EN (bit 13) with S5 sleep type (bits 10-12)
    outw(pm1a_control, S5_SLEEP_TYPA | SLP_EN);
    if(pm1b_control) outw(pm1b_control, S5_SLEEP_TYPA | SLP_EN);

    // If ACPI fails, use fallback methods
    printf("[Info] ACPI Shutdown failed, halting system!\n");
    while (1) {
        asm volatile ("hlt");
    }
}


void acpi_reboot(){
    fadt_t *fadt = (fadt_t *) fadt_addr;
    uint8_t reset_value = fadt->ResetValue;
    GenericAddressStructure_t reset_reg = fadt->ResetReg;

    // Check if the reset register is valid
    if (reset_reg.AddressSpace == 1) { // SystemIO space
        // Write the reset value to the reset register
        switch (reset_reg.AccessSize) {
            case 1: // Byte access
                outb((uint16_t)reset_reg.Address, reset_value);
                break;
            case 2: // Word access
                outw((uint16_t)reset_reg.Address, reset_value);
                break;
            case 3: // DWord access
                outl((uint16_t)reset_reg.Address, (uint32_t)reset_value);
                break;
            default:
                // Unsupported access size
                break;
        }
    } else if (reset_reg.AddressSpace == 0) { // SystemMemory space
        // Write the reset value to the reset register in memory
        volatile uint8_t* reset_reg_ptr = (volatile uint8_t*)(uintptr_t)reset_reg.Address;
        *reset_reg_ptr = reset_value;
    }
}











