/*
    ACPI : Advanced Configuration and Power Interface
    https://github.com/dreamportdev/Osdev-Notes/blob/master/02_Architecture/06_ACPITables.md
*/


#include "../driver/ports.h"
#include "../limine/limine.h"

#include "../lib/string.h"
#include "../lib/stdio.h"

#include "acpi.h"

#define PM1A_CNT_REG  0x604  // Default address of PM1a_CNT for many systems
#define SLP_EN        (1 << 13)  // Bit 13: SLP_EN (Sleep Enable)
#define S5_SLEEP_TYPA (5 << 10)  // Sleep type S5 (5) in bits 10-12

void qemu_poweroff() {

    // Write to the PM1a_CNT register
    outw(PM1A_CNT_REG, S5_SLEEP_TYPA | SLP_EN); // QEMU-specific ACPI shutdown port

    // If the system fails to power off, hang the CPU
    printf("ACPI Shutdown failed, halting system!\n");
    while (1) {
        __asm__ volatile ("hlt");
    }
}


void qemu_reboot(){
    outb(0x64, 0xFE);   // Send reset command to the keyboard controller
}


__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(3);

// Get RSDP info
__attribute__((used, section(".requests")))
static volatile struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 3
};


fadt_t *fadt;

void *find_acpi_table() {

    if (!rsdp_request.response || !rsdp_request.response->address) {
        printf("ACPI is not available\n");
        return NULL; // ACPI is not available
    }

    rsdp_t *rsdp = (rsdp_t *) rsdp_request.response->address;
    
    if (rsdp->revision >= 2) {
        rsdp_ext_t *rsdp_ext = (rsdp_ext_t *)rsdp;
        return (void *)(uintptr_t)rsdp_ext; // Use XSDT for 64-bit systems
    }

    return (void *)(uintptr_t)rsdp; // Use RSDT for ACPI 1.0
}

void validate_acpi_table(void *table_addr){
    rsdp_t *rsdp = (rsdp_t *) table_addr;
    if(rsdp){
        uint64_t acpi_version = (rsdp->revision >= 2) ? 2 : 1;
        if(!memcmp(rsdp->signature, "RSD PTR ", 8)){
            uint8_t sum = 0;
            uint8_t *ptr = (uint8_t *) rsdp;
            for (int i = 0; i < 20; i++) {
                sum += ptr[i];
            }
            if((sum % 256) == 0){
                printf("ACPI %d.0 is signature and checksum validated\n", acpi_version);
            }else{
                printf("ACPI %d.0 is not checksum  validated\n", acpi_version);
            }
        }else{
            printf("ACPI %d.0 is not signature  validated\n", acpi_version);
        }

    }else{
        printf("ACPI Table not found\n");
    }
}


void parse_acpi_table(void *table_addr) {
    rsdp_t *rsdp = (rsdp_t *) table_addr;
    rsdt_t *rsdt = (rsdt_t *) rsdp->rsdt_address;

    rsdp_ext_t *rsdp_ext = (rsdp->revision >= 2) ? (rsdp_ext_t *) table_addr : 0;;
    xsdt_t *xsdt = (rsdp->revision >= 2) ? (xsdt_t *)rsdp_ext->xsdt_address : 0;

    acpi_header_t header = (rsdp->revision >= 2) ? xsdt->header : rsdt->header;

    int entry_size = (rsdp->revision >= 2) ? sizeof(uint64_t) : sizeof(uint32_t);
    int entry_count = (header.length - sizeof(acpi_header_t)) / entry_size;

    uint32_t *entries_32 = (uint32_t *) rsdt->entries;
    uint64_t *entries_64 = (uint64_t *) xsdt->entries;
    void *entries = (rsdp->revision >= 2) ? (void *)entries_64 : (void *)entries_32;

    for (int i = 0; i < entry_count; i++) {
        acpi_header_t *entry = (acpi_header_t *)(uintptr_t)((rsdp->revision >= 2) ? ((uint64_t *)entries)[i] : ((uint32_t *)entries)[i]);
        if (memcmp(entry->signature, "APIC", 4) == 0) {
            parse_madt(entry);
        } else if (memcmp(entry->signature, "MCFG", 4) == 0) {
            parse_mcfg(entry);
        }else if (!memcmp(entry->signature, "FACP", 4)) {
            fadt = (fadt_t *) entry;
            parse_fadt(entry); // Found FADT
        }
    }

    
}



void parse_madt(acpi_header_t *table) {
    madt_t *madt = (madt_t *)table;

    uint8_t *entry_ptr = (uint8_t *)(madt + 1);
    uint8_t *end_ptr = (uint8_t *)madt + madt->header.length;

    while (entry_ptr < end_ptr) {
        uint8_t type = entry_ptr[0];
        uint8_t length = entry_ptr[1];

        if (type == 0) { // Local APIC Entry
            uint8_t apic_id = entry_ptr[2];
            uint8_t cpu_flags = entry_ptr[3];

            if (cpu_flags & 1) {
                // Store detected APs for SMP
                printf("Application Processor found!\n");
            }
        }

        entry_ptr += length;
    }
}



void parse_mcfg(acpi_header_t *table) {
    mcfg_t *mcfg = (mcfg_t *)table;
    // Iterate through PCI devices and store configuration space
}


void parse_fadt(acpi_header_t *table){
    fadt_t *fadt = (fadt_t *) table;
}



// Function to read ACPI enable status
int is_acpi_enabled() {
    if (!fadt) {
        // printf("FADT not found! ACPI status unknown.\n");
        return -1;
    }

    uint32_t pm1a_control = (fadt->header.revision >= 2 && fadt->X_PM1aControlBlock.Address) ? \
                                (uint32_t)fadt->X_PM1aControlBlock.Address : fadt->PM1aControlBlock;

    printf("FADT PM1a Control Block: %x\n", fadt->PM1aControlBlock);
    printf("FADT X_PM1a Control Block: %x\n", fadt->X_PM1aControlBlock.Address);

    if (!pm1a_control) {
        // printf("PM1a Control Block not found!\n");
        return -1;
    }

    uint16_t acpi_status = inw(pm1a_control); // Read PM1a Control Block register

    if (acpi_status & 1) { // Check SCI_EN (Bit 0)
        // printf("ACPI is ENABLED.\n");
        return 1;
    } else {
        // printf("ACPI is DISABLED.\n");
        return 0;
    }
}


void acpi_enable() {
    if (!fadt) {
        // printf("FADT not found, ACPI cannot be enabled!\n");
        return;
    }

    // Check if ACPI mode needs to be enabled
    if (fadt->SMI_CommandPort && fadt->AcpiEnable) {
        // printf("Enabling ACPI Mode...\n");
        outb(fadt->SMI_CommandPort, fadt->AcpiEnable);

        // Wait a bit for ACPI mode to activate
        for (volatile int i = 0; i < 100000; i++);
        // printf("Succesfully ACPI Mode enable\n");
    }
}


void acpi_poweroff() {
    if (!fadt) {
        printf("FADT not found, ACPI shutdown unavailable!\n");
        return;
    }

    // Enable ACPI first (if needed)
    if(!is_acpi_enabled()){
        acpi_enable();
    }

    uint32_t pm1a_control = 0;

    pm1a_control = (fadt->header.revision >= 2 && fadt->X_PM1aControlBlock.Address) ? (uint32_t)fadt->X_PM1aControlBlock.Address : fadt->PM1aControlBlock;

    uint32_t pm1b_control = fadt->PM1bControlBlock;

    // printf("PreferredPowerManagementProfile : %d\n", fadt->PreferredPowerManagementProfile);
    // uint16_t slp_typa = (fadt->PreferredPowerManagementProfile == 5) ? (1 << 10) : (1 << 13); // SLP_TYPa for S5


    if (!pm1a_control) {
        printf("PM1a Control Block not found!\n");
        return;
    }

    printf("Sending ACPI shutdown command: outw(%x, %x)\n", pm1a_control, S5_SLEEP_TYPA | SLP_EN);

    // Shutdown by setting SLP_EN (bit 13) with S5 sleep type (bits 10-12)
    outw(pm1a_control, S5_SLEEP_TYPA | SLP_EN);
    if(pm1b_control) outw(pm1b_control, S5_SLEEP_TYPA | SLP_EN);

    // If ACPI fails, use fallback methods
    printf("ACPI Shutdown failed, halting system!\n");
    while (1) {
        __asm__ volatile ("hlt");
    }
}


void acpi_reboot(){
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


void init_acpi(){
    void *rsdp_addr = find_acpi_table();
    validate_acpi_table(rsdp_addr);
    parse_acpi_table(rsdp_addr);
}


