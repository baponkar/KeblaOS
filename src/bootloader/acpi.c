/*
    ACPI : Advanced Configuration and Power Interface
    https://github.com/dreamportdev/Osdev-Notes/blob/master/02_Architecture/06_ACPITables.md
*/

#include "../driver/vga.h"
#include "../driver/ports.h"
#include "../limine/limine.h"
#include "acpi.h"


__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(3);


// Get RSDP info
__attribute__((used, section(".requests")))
static volatile struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 3
};

rsdp_1_t *rsdp1;
rsdp_2_t *rsdp2;
xsdt_t *xsdt;

void init_acpi(){

    if (rsdp_request.response == NULL || rsdp_request.response->address == 0) {
        // ACPI is not available
        print("ACPI not available.\n");
        return;
    }

    // Retrieve the RSDP address
    void *rsdp_address = (void *) rsdp_request.response->address;
    rsdp1 = (rsdp_1_t *) rsdp_address;

    // Check if the RSDP is for ACPI 1.0 or ACPI 2.0+
    if(rsdp1->revision == 0){
        print("ACPI 1.0 found\n");
    }
    
    if(rsdp1->revision == 2){
        print("ACPI 2.0+ found\n");
        rsdp2 = (rsdp_2_t *) rsdp_address;
    }
    
    if(rsdp1->revision > 2){
        print("ACPI version > 2.0 found\n");
        rsdp_2_t *rsdp2 = (rsdp_2_t *) rsdp_address;
        xsdt = (xsdt_t *) rsdp2->xsdt_address;
    }


}




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