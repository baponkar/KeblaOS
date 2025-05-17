

#include "../../../lib/stdio.h"
#include "../../../lib/string.h"
#include "../acpi.h"
#include "fadt.h"
#include "hpet.h"
#include "madt.h"
#include "mcfg.h"

#include "rsdt.h"

rsdt_t *rsdt; 
xsdt_t *xsdt;

extern rsdp_t *rsdp;
extern rsdp_ext_t *rsdp_ext;

// Descriptor Table Address
extern fadt_t *fadt;   // Defined in fadt.c
extern madt_t *madt;   // Defined in madt.c
extern mcfg_t *mcfg;   // Defined in mcfg.c
extern hpet_t *hpet;   // Defined in hpet.c

// parsing RSDT and XSDT tables to get MADT, MCFG, FADT and HPET tables
void parse_rsdt_table(rsdp_t *rsdp){
    if(rsdp->revision >= 2){
        rsdp_ext_t *rsdp_ext = (rsdp_ext_t *) rsdp;
        xsdt = (xsdt_t *)(uintptr_t) rsdp_ext->xsdt_address;

        acpi_header_t header = xsdt->header;
        int entry_size = sizeof(uint64_t);
        int entry_count = (header.length - sizeof(acpi_header_t)) / entry_size;

        uint64_t *entries_64 = (uint64_t *)(uintptr_t) xsdt->entries;
        void *entries = (void *) entries_64;

        for (int i = 0; i < entry_count; i++)
        {
            acpi_header_t *entry = (acpi_header_t *)(uintptr_t) ((uint64_t *)entries)[i];
            if(memcmp(entry->signature, "APIC", 4) == 0){
                madt = (madt_t *)(uintptr_t) entry;
            }else if(memcmp(entry->signature, "MCFG", 4) == 0){
                mcfg = (mcfg_t *)(uintptr_t) entry;
            }else if(memcmp(entry->signature, "FACP", 4) == 0){
                fadt = (fadt_t *)(uintptr_t) entry;
            }else if(memcmp(entry->signature, "HPET", 4) == 0){
                hpet = (hpet_t *)(uintptr_t) entry;
            }else{
                continue;
            }
        }
    }else{
        rsdt = (rsdt_t *)(uintptr_t) rsdp->rsdt_address;
        acpi_header_t header = rsdt->header;
        int entry_size = sizeof(uint32_t);
        int entry_count = (header.length - sizeof(acpi_header_t)) / entry_size;

        uint32_t *entries_32 = (uint32_t *)(uintptr_t) rsdt->entries;
        void *entries = (void *) entries_32;

        for (int i = 0; i < entry_count; i++)
        {
            acpi_header_t *entry = (acpi_header_t *)(uintptr_t) ((uint32_t *)entries)[i];
            if(memcmp(entry->signature, "APIC", 4) == 0){
                madt = (madt_t *)(uintptr_t) entry;
            }else if(memcmp(entry->signature, "MCFG", 4) == 0){
                mcfg = (mcfg_t *)(uintptr_t) entry;
            }else if(memcmp(entry->signature, "FACP", 4) == 0){
                fadt = (fadt_t *)(uintptr_t) entry;
            }else if(memcmp(entry->signature, "HPET", 4) == 0){
                hpet = (hpet_t *)(uintptr_t) entry;
            }else{
                continue;
            }
        }
    }
}

