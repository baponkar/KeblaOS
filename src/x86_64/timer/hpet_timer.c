/*
HPET(High Precision Event Timer)
https://wiki.osdev.org/HPET
*/

#include "../../lib/stdio.h"
#include "../../lib/string.h"
#include "../../util/util.h"
#include "../interrupt/interrupt.h"


#include "hpet_timer.h"


extern void *hpet_addr;

#define HPET_GEN_CONFIG       0x10  // General Configuration Register
#define HPET_GEN_CONFIG_ENABLE (1 << 0)  // Bit 0 enables HPET
#define HPET_MAIN_COUNTER 0xF0  // Main counter register
#define HPET_TIMER0_CONFIG 0x100
#define HPET_TIMER0_COMPARATOR 0x108
#define HPET_TIMER0_INTERRUPT  34  // IOAPIC IRQ 34

#define IOAPIC_REG_SEL  0xFEC00000
#define IOAPIC_REG_WIN  (IOAPIC_REG_SEL + 0x10)



void ioapic_write(uint8_t reg, uint32_t value) {
    *(volatile uint32_t *)IOAPIC_REG_SEL = reg;
    *(volatile uint32_t *)IOAPIC_REG_WIN = value;
}

void hpet_irq_handler(registers_t *regs) {
    printf("HPET Timer Interrupt Triggered!\n");
}


void init_hpet(){

    printf("Starting HPET Timer\n");

    hpet_t *hpet = (hpet_t *)hpet_addr;
    printf("HPET Base Address: %x\n", hpet->base_address.Address);


    volatile uint64_t *hpet_mmio = (volatile uint64_t *) hpet->base_address.Address;

    printf("HPET Capabilities Register: %x\n", hpet_mmio[0]); // Read Capabilities
    printf("HPET Main Counter (Before Start): %x\n", hpet_mmio[HPET_MAIN_COUNTER / 8]);

    // Enable HPET Timer
    hpet_mmio[HPET_GEN_CONFIG / 8] |= HPET_GEN_CONFIG_ENABLE;


    uint64_t start = hpet_mmio[HPET_MAIN_COUNTER / 8];

    

    for (volatile int i = 0; i < 1000; i++);  // Small delay
    uint64_t end = hpet_mmio[HPET_MAIN_COUNTER / 8];

    if (end > start) {
        printf("HPET is running!\n");
    } else {
        printf("HPET failed to start!\n");
    }

    hpet_mmio[HPET_TIMER0_CONFIG / 8] |= (1 << 2); // Enable periodic mode
    // Set the comparator value (e.g., 1 ms interval)
    uint64_t clock_period_fs = hpet_mmio[0x04 / 8]; // HPET clock period in femtoseconds
    uint64_t frequency = 1000000000000000 / clock_period_fs; // HPET frequency in Hz
    uint64_t ticks_per_ms = frequency / 1000; // 1 ms interval

    hpet_mmio[HPET_TIMER0_COMPARATOR / 8] = ticks_per_ms;

    // Route HPET Timer 0 to an IOAPIC interrupt
    hpet_mmio[HPET_TIMER0_CONFIG / 8] |= (HPET_TIMER0_INTERRUPT << 9); // Set interrupt vector

    // Map HPET Timer 0 to APIC interrupt vector 48
    ioapic_write(0x10 + (HPET_TIMER0_INTERRUPT * 2), 48);

    interrupt_install_handler((48 - 32), &hpet_irq_handler);
    enable_interrupts();

    printf("Successfully HPET Timer Started\n");
}




