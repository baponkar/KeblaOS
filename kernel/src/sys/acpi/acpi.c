
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


#include "../../driver/io/ports.h"
#include "../../../../ext_lib/limine-9.2.3/limine.h"
#include "../../lib/string.h"
#include "../../lib/stdio.h"
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
extern rsdt_t *rsdt;   // Defined in rsdt.c
extern xsdt_t *xsdt;   // Defined in rsdt.c
extern fadt_t *fadt;   // Defined in fadt.c
extern madt_t *madt;   // Defined in madt.c
extern mcfg_t *mcfg;   // Defined in mcfg.c
extern hpet_t *hpet;   // Defined in hpet.c



void find_acpi_table_pointer(){
    if(!rsdp_request.response || !rsdp_request.response->address){
        printf("[Error] ACPI is not available!\n");
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
                printf(" [-] ACPI %d.0 is signature and checksum validated\n", acpi_version);
            }else{
                printf("[Error] ACPI %d.0 is not checksum validated\n", acpi_version);
            }
        }else{
            printf(" [-] ACPI %d.0 is not signature validated\n", acpi_version);
        }
    }else{
        printf("[Error] ACPI Table not found\n");
    }
}


// Function to read ACPI enable status
int is_acpi_enabled() {
    fadt_t *fadt = (fadt_t *)fadt;
    if (!fadt) {
        printf("[Error] FADT not found! ACPI status unknown.\n");
        return -1;
    }

    uint32_t pm1a_control = (fadt->header.revision >= 2 && fadt->X_PM1aControlBlock.Address) ? \
                            (uint32_t)fadt->X_PM1aControlBlock.Address : fadt->PM1aControlBlock;

    // printf("FADT PM1a Control Block: %x\n", fadt->PM1aControlBlock);
    // printf("FADT X_PM1a Control Block: %x\n", fadt->X_PM1aControlBlock.Address);

    if (!pm1a_control) {
        printf("[Error] PM1a Control Block not found!\n");
        return -1;
    }

    uint16_t acpi_status = inw(pm1a_control); // Read PM1a Control Block register

    if (acpi_status & 1) { // Check SCI_EN (Bit 0)
        printf(" [-] ACPI is ENABLED.\n");
        return 1;
    }
    
    printf(" [-] ACPI is DISABLED.\n");
    return 0;
}


void acpi_enable() {
    fadt_t *fadt = (fadt_t *) fadt;
    if (!fadt) {
        printf(" [Error] FADT not found, ACPI cannot be enabled!\n");
        return;
    }

    // Check if ACPI mode needs to be enabled
    if (fadt->SMI_CommandPort && fadt->AcpiEnable) {
        outb(fadt->SMI_CommandPort, fadt->AcpiEnable);

        // Wait a bit for ACPI mode to activate
        for (volatile int i = 0; i < 100000; i++);
        printf(" [-] Succesfully ACPI Mode enable\n");
    }
}


void init_acpi(){
    
    find_acpi_table_pointer();
    
    validate_rsdp_table(rsdp);
    parse_rsdt_table(rsdp);

    if(!is_acpi_enabled()){
        acpi_enable();
    }else {
        printf("[ACPI] : ACPI is already enabled!\n");
    }
        
    // FADT
    parse_fadt(fadt);

    // HPET
    // parse_hpet(hpet);
    if(!hpet){
        printf("HPET is NULL!\n");
    }
    // hpet_init(hpet);
    // hpet_enable_periodic_irq(17, 1);

    // MADT
    parse_madt(madt);

    // MCFG
    // parse_mcfg(mcfg);    // PCIe Scan

    printf(" [-] Successfully ACPI Enabled\n");
}


