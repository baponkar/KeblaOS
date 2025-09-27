/*
Multiple APIC Description Table (MADT)

Reference:
    https://wiki.osdev.org/MADT
*/


#include "../../../lib/stdio.h"

#include "madt.h"

extern bool debug_on;

madt_t *madt;       
extern uint32_t ioapic_addr;    // Defined in ioapic.c



void parse_madt(madt_t *madt) {
    
    if(madt == NULL) {
        printf("[Error] MADT is NULL\n");
        return;
    }

    uint8_t *ptr = (uint8_t *)(madt + 1);  // Start after MADT header
    uint8_t *end = (uint8_t *)madt + madt->header.length;

    while (ptr < end) {
        madt_entry_t *entry = (madt_entry_t *)ptr;
        
        switch (entry->type) {
            case 0x00: {  // Local APIC
                // if(debug_on) printf(" Found Local APIC entry\n");
                break;
            }
            
            case 0x01: {  // I/O APIC
                madt_ioapic_t *ioapic = (madt_ioapic_t *)ptr;
                ioapic_addr = ioapic->ioapic_addr;
                if(debug_on) printf(" Found I/O APIC: ID = %d, Address = %x, GSI Base = %d\n",
                       ioapic->ioapic_id, ioapic->ioapic_addr, ioapic->gsi_base);
                break;
            }
            
            case 0x02: {  // Interrupt Source Override
                madt_iso_t *iso = (madt_iso_t *)ptr;
                // if(debug_on) printf(" Found Interrupt Source Override: Bus Source = %d, IRQ Source = %d, GSI = %d, Flags = %x\n",
                //        iso->bus_source, iso->irq_source, iso->gsi, iso->flags);
                break;
            }
            
            case 0x05: {  // Local APIC Address Override
                madt_lapic_override_t *lapic_override = (madt_lapic_override_t *)ptr;
                // if(debug_on) printf(" [-] Found Local APIC Address Override: New LAPIC Address = %x\n",
                //        lapic_override->lapic_addr);
                break;
            }
            
            default:
                // if(debug_on) printf("[Error] Unknown MADT entry type: %x\n", entry->type);
                break;
        }
        
        ptr += entry->length;  // Move to next entry
    }
}






