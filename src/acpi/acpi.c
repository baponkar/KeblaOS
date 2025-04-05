
/*
    ACPI : Advanced Configuration and Power Interface

    RSDP : Root System Descriptor Pointer
    RSDT : Root System Descriptor Table
    XSDT : Extended System Descriptor Table
    FADT : Fixed ACPI Description Table
    MADT : Multiple APIC Description Table

    https://wiki.osdev.org/ACPI
    https://github.com/dreamportdev/Osdev-Notes/blob/master/02_Architecture/06_ACPITables.md
    https://stackoverflow.com/questions/79406253/how-can-i-make-power-shutdown-by-acpi
*/


#include "../driver/io/ports.h"
#include "../limine/limine.h"
#include "../lib/string.h"
#include "../lib/stdio.h"
#include "./descriptor_table/fadt.h"
#include "./descriptor_table/madt.h"
#include "./descriptor_table/mcfg.h"
#include "./descriptor_table/rsdt.h"
#include "./descriptor_table/hpet.h"

#include "acpi.h"



__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(0);

// Get RSDP info
__attribute__((used, section(".requests")))
static volatile struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0
};


rsdp_t *rsdp;
rsdp_ext_t *rsdp_ext;

// Descriptor Table Address
rsdt_t *rsdt_addr;
xsdt_t *xsdt_addr;
fadt_t *fadt_addr;
madt_t *madt_addr;
mcfg_t *mcfg_addr;
hpet_t *hpet_addr;



void find_acpi_table_pointer(){
    if(!rsdp_request.response || !rsdp_request.response->address){
        printf("ACPI is not available!\n");
        return;
    }
    rsdp = (rsdp_t *) rsdp_request.response->address;
    if(rsdp->revision >= 2){
        rsdp_ext = (rsdp_ext_t *) rsdp;
    }
}

void validate_rsdp_table(rsdp_t *rsdp){
    if(rsdp){
        uint64_t acpi_version = (rsdp->revision >= 2) ? 2 : 1;
        if(!memcmp(rsdp->signature, "RSD PTR ", 8)){
            uint8_t sum = 0;
            uint8_t *ptr = (uint8_t *) rsdp;
            for (int i = 0; i < 20; i++) {
                sum += ptr[i];
            }
            if((sum % 256) == 0){
                printf("[Info] ACPI %d.0 is signature and checksum validated\n", acpi_version);
            }else{
                printf("[Info] ACPI %d.0 is not checksum validated\n", acpi_version);
            }
        }else{
            printf("[Info] ACPI %d.0 is not signature validated\n", acpi_version);
        }
    }else{
        printf("[Info] ACPI Table not found\n");
    }
}



// parsing RSDT and XSDT tables to get MADT, MCFG, FADT and HPET tables
void parse_rsdt_table(rsdp_t *rsdp){
    if(rsdp->revision >= 2){
        rsdp_ext_t *rsdp_ext = (rsdp_ext_t *) rsdp;
        xsdt_addr = (xsdt_t *)(uintptr_t) rsdp_ext->xsdt_address;

        acpi_header_t header = xsdt_addr->header;
        int entry_size = sizeof(uint64_t);
        int entry_count = (header.length - sizeof(acpi_header_t)) / entry_size;

        uint64_t *entries_64 = (uint64_t *)(uintptr_t) xsdt_addr->entries;
        void *entries = (void *) entries_64;

        for (int i = 0; i < entry_count; i++)
        {
            acpi_header_t *entry = (acpi_header_t *)(uintptr_t) ((uint64_t *)entries)[i];
            if(memcmp(entry->signature, "APIC", 4) == 0){
                madt_addr = (madt_t *)(uintptr_t) entry;
            }else if(memcmp(entry->signature, "MCFG", 4) == 0){
                mcfg_addr = (mcfg_t *)(uintptr_t) entry;
            }else if(memcmp(entry->signature, "FACP", 4) == 0){
                fadt_addr = (fadt_t *)(uintptr_t) entry;
            }else if(memcmp(entry->signature, "HPET", 4) == 0){
                hpet_addr = (hpet_t *)(uintptr_t) entry;
            }else{
                continue;
            }
        }
    }else{
        rsdt_addr = (rsdt_t *)(uintptr_t) rsdp->rsdt_address;
        acpi_header_t header = rsdt_addr->header;
        int entry_size = sizeof(uint32_t);
        int entry_count = (header.length - sizeof(acpi_header_t)) / entry_size;

        uint32_t *entries_32 = (uint32_t *)(uintptr_t) rsdt_addr->entries;
        void *entries = (void *) entries_32;

        for (int i = 0; i < entry_count; i++)
        {
            acpi_header_t *entry = (acpi_header_t *)(uintptr_t) ((uint32_t *)entries)[i];
            if(memcmp(entry->signature, "APIC", 4) == 0){
                madt_addr = (madt_t *)(uintptr_t) entry;
            }else if(memcmp(entry->signature, "MCFG", 4) == 0){
                mcfg_addr = (mcfg_t *)(uintptr_t) entry;
            }else if(memcmp(entry->signature, "FACP", 4) == 0){
                fadt_addr = (fadt_t *)(uintptr_t) entry;
            }else if(memcmp(entry->signature, "HPET", 4) == 0){
                hpet_addr = (hpet_t *)(uintptr_t) entry;
            }else{
                continue;
            }
        }
    }
}




// Function to read ACPI enable status
int is_acpi_enabled() {
    fadt_t *fadt = (fadt_t *)fadt_addr;
    if (!fadt) {
        printf("[Info] FADT not found! ACPI status unknown.\n");
        return -1;
    }

    uint32_t pm1a_control = (fadt->header.revision >= 2 && fadt->X_PM1aControlBlock.Address) ? \
                            (uint32_t)fadt->X_PM1aControlBlock.Address : fadt->PM1aControlBlock;

    // printf("FADT PM1a Control Block: %x\n", fadt->PM1aControlBlock);
    // printf("FADT X_PM1a Control Block: %x\n", fadt->X_PM1aControlBlock.Address);

    if (!pm1a_control) {
        printf("[Info] PM1a Control Block not found!\n");
        return -1;
    }

    uint16_t acpi_status = inw(pm1a_control); // Read PM1a Control Block register

    if (acpi_status & 1) { // Check SCI_EN (Bit 0)
        printf("[Info] ACPI is ENABLED.\n");
        return 1;
    } else {
        printf("[Info] ACPI is DISABLED.\n");
        return 0;
    }
}


void acpi_enable() {
    fadt_t *fadt = (fadt_t *) fadt_addr;
    if (!fadt) {
        printf("[Info] FADT not found, ACPI cannot be enabled!\n");
        return;
    }

    // Check if ACPI mode needs to be enabled
    if (fadt->SMI_CommandPort && fadt->AcpiEnable) {
        outb(fadt->SMI_CommandPort, fadt->AcpiEnable);

        // Wait a bit for ACPI mode to activate
        for (volatile int i = 0; i < 100000; i++);
        printf("[Info] Succesfully ACPI Mode enable\n");
    }
}


void init_acpi(){
    find_acpi_table_pointer();
    validate_rsdp_table(rsdp);
    parse_rsdt_table(rsdp);
    is_acpi_enabled();
    acpi_enable();
}


