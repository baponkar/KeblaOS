/*
HPET(High Precision Event Timer)

    Reference: 
    https://wiki.osdev.org/HPET
*/


#include "../../../lib/stdio.h"
#include "../../../lib/string.h"
#include "../../../util/util.h"
#include "../acpi.h"

#include "hpet.h"




#define HPET_GENERAL_CAPS      0x000 // R - 64-bit
#define HPET_GENERAL_CONFIG    0x010 // RW - 32-bit
#define HPET_MAIN_COUNTER      0x0F0 // RW - 64-bit
#define HPET_TIMER0_CONFIG     0x100 // RW - 64-bit
#define HPET_TIMER0_COMPARATOR 0x108 // RW - 64-bit

static volatile uint64_t* hpet_base = NULL;
static uint32_t hpet_tick_period_fs = 0;
static uint64_t hpet_frequency_hz = 0;

hpet_t *hpet;

void hpet_init(hpet_t* hpet) {
    if (!hpet || hpet->base_address.AddressSpace != 0) {
        printf("[HPET] hpet: %x, hpet->base_address.AddressSpace\n", (uint64_t)hpet, (uint64_t)hpet->base_address.AddressSpace);
        return;
    }

    hpet_base = (volatile uint64_t*)(uintptr_t)(hpet->base_address.Address);

    // Disable HPET before configuration
    *(volatile uint32_t*)((uintptr_t)hpet_base + HPET_GENERAL_CONFIG) &= ~1;

    // Read tick period (in femtoseconds) from General Capabilities Register
    uint64_t capabilities = *(volatile uint64_t*)((uintptr_t)hpet_base + HPET_GENERAL_CAPS);
    hpet_tick_period_fs = (uint32_t)(capabilities >> 32);
    hpet_frequency_hz = 1000000000000000ULL / hpet_tick_period_fs;

    // Reset main counter to 0
    *(volatile uint64_t*)((uintptr_t)hpet_base + HPET_MAIN_COUNTER) = 0;

    // Enable HPET
    *(volatile uint32_t*)((uintptr_t)hpet_base + HPET_GENERAL_CONFIG) |= 1;
}


void hpet_sleep_us(uint64_t microseconds) {
    if (!hpet_base || hpet_tick_period_fs == 0) return;

    uint64_t start = *(volatile uint64_t*)((uintptr_t)hpet_base + HPET_MAIN_COUNTER);
    uint64_t ticks = (microseconds * 1000000000ULL) / hpet_tick_period_fs;
    uint64_t end = start + ticks;

    while (*(volatile uint64_t*)((uintptr_t)hpet_base + HPET_MAIN_COUNTER) < end);
}


#define HPET_GENERAL_CONFIG         0x010
#define HPET_MAIN_COUNTER           0x0F0

#define HPET_TIMER0_CONFIG          0x100
#define HPET_TIMER0_COMPARATOR      0x108
#define HPET_TIMER0_FSB             0x110

#define HPET_TN_INT_ENB_CNF         (1 << 2)
#define HPET_TN_TYPE_CNF            (1 << 3)
#define HPET_TN_VAL_SET_CNF         (1 << 6)
#define HPET_TN_32MODE_CNF          (1 << 8)

// IRQ route bits [9:13]
#define HPET_TN_IRQ_SHIFT           9
#define HPET_TN_IRQ_MASK            (0x1F << HPET_TN_IRQ_SHIFT)


extern void irq_install(int irq_no, void (*handler)(registers_t *r));

static void my_hpet_timer_handler(registers_t *regs) {
    printf("HPET Timer Interrupt\n");
}


void hpet_enable_periodic_irq(uint8_t irq_number, uint64_t period_fs) {
    if (!hpet_base || hpet_tick_period_fs == 0) return;

    uint64_t ticks = period_fs / hpet_tick_period_fs;

    // Disable overall HPET
    *(volatile uint32_t*)((uintptr_t)hpet_base + HPET_GENERAL_CONFIG) &= ~1;

    // Disable Timer 0
    *(volatile uint64_t*)((uintptr_t)hpet_base + HPET_TIMER0_CONFIG) = 0;

    // Step 1: Write initial comparator value
    *(volatile uint64_t*)((uintptr_t)hpet_base + HPET_TIMER0_COMPARATOR) = ticks;

    // Step 2: Set VAL_SET_CNF to update comparator for periodic
    *(volatile uint64_t*)((uintptr_t)hpet_base + HPET_TIMER0_CONFIG) =
        HPET_TN_VAL_SET_CNF;

    // Step 3: Write comparator value again (some implementations require this)
    *(volatile uint64_t*)((uintptr_t)hpet_base + HPET_TIMER0_COMPARATOR) = ticks;

    // Step 4: Final config - periodic, interrupt enabled, route to IRQ
    uint64_t config = HPET_TN_TYPE_CNF     // Periodic mode
                    | HPET_TN_INT_ENB_CNF  // Enable interrupt
                    | ((irq_number & 0x1F) << HPET_TN_IRQ_SHIFT);

    *(volatile uint64_t*)((uintptr_t)hpet_base + HPET_TIMER0_CONFIG) = config;

    // Step 5: Enable HPET main counter
    *(volatile uint32_t*)((uintptr_t)hpet_base + HPET_GENERAL_CONFIG) |= 1;

    // Install IRQ handler
    irq_install(irq_number, my_hpet_timer_handler);

    printf("[HPET] : Successfully Enabling Periodic IRQ\n");
}











