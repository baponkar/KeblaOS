
#include "../x86_64/interrupt/apic.h"

#include "trampoline.h"

void load_trampoline_for_ap(uint64_t core_id) {
    volatile uint64_t* core_id_ptr = (uint64_t*)0x9000;
    *core_id_ptr = core_id;
}


void wakeup_ap(uint64_t apic_id) {
    // Load the trampoline code at physical address 0x8000
    load_trampoline_for_ap(apic_id);

    // Send INIT IPI
    lapic_send_ipi(apic_id, 0x4500);
    for (volatile int i = 0; i < 100000; i++);

    // Send SIPI with the vector 0x08 (0x8000 / 4096)
    lapic_send_ipi(apic_id, 0x4608);
    for (volatile int i = 0; i < 100000; i++);

    // Send a second SIPI
    lapic_send_ipi(apic_id, 0x4608);
    for (volatile int i = 0; i < 100000; i++);
}
