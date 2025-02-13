/*
    ACPI : Advanced Configuration and Power Interface
    https://github.com/dreamportdev/Osdev-Notes/blob/master/02_Architecture/06_ACPITables.md
    https://stackoverflow.com/questions/79406253/how-can-i-make-power-shutdown-by-acpi
*/


#include "../driver/io/ports.h"
#include "../limine/limine.h"

#include "../lib/string.h"
#include "../lib/stdio.h"

#include "fadt.h"
#include "madt.h"

#include "acpi.h"



__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(3);

// Get RSDP info
__attribute__((used, section(".requests")))
static volatile struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 3
};


void *fadt_addr;
void *madt_addr;

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
            madt_addr = (void *) entry;
            parse_madt();
        } else if (memcmp(entry->signature, "MCFG", 4) == 0) {
            parse_mcfg(entry);
        }else if (!memcmp(entry->signature, "FACP", 4)) {
            fadt_addr = (void *) entry;
        }
    }
}


void parse_mcfg(acpi_header_t *table) {
    mcfg_t *mcfg = (mcfg_t *)table;
    // Iterate through PCI devices and store configuration space
}



// Function to read ACPI enable status
int is_acpi_enabled() {
    fadt_t *fadt = (fadt_t *)fadt_addr;
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
    fadt_t *fadt = (fadt_t *) fadt_addr;
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





void init_acpi(){
    void *rsdp_addr = find_acpi_table();
    validate_acpi_table(rsdp_addr);
    parse_acpi_table(rsdp_addr);
}


