
#include "../lib/stdio.h"
#include "acpi.h"

#include "madt.h"

extern void *madt_addr;

void parse_madt() {
    madt_t *madt = (madt_t *)madt_addr;

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
                // printf("Application Processor found!\n");
            }
        }

        entry_ptr += length;
    }
}



