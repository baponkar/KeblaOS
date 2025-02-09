
#include "../interrupt/pic.h"
#include "../interrupt/apic.h"
#include "../../util/util.h"
#include "../../lib/stdio.h"

#include "apic_timer.h"

#define LAPIC_BASE       0xFEE00000
#define LAPIC_TIMER      (LAPIC_BASE + 0x320)  // Timer register
#define LAPIC_TICR       (LAPIC_BASE + 0x380)  // Initial Count
#define LAPIC_TCCR       (LAPIC_BASE + 0x390)  // Current Count
#define LAPIC_TDCR       (LAPIC_BASE + 0x3E0)  // Divide Configuration
#define LAPIC_EOI        (LAPIC_BASE + 0xB0)   // End of Interrupt
#define LAPIC_SVR        (LAPIC_BASE + 0xF0)   // Spurious Interrupt Vector Register

#define APIC_TIMER_VECTOR  0x20  // Interrupt vector for timer

static inline void lapic_write(uint32_t reg, uint32_t value) {
    *((volatile uint32_t*)(reg)) = value;
}

static inline uint32_t lapic_read(uint32_t reg) {
    return *((volatile uint32_t*)(reg));
}










void setup_apic_timer(uint32_t tick_count) {
    lapic_write(LAPIC_TDCR, 0b1011); // Divide by 1 (0000, 1011 = 1)
    lapic_write(LAPIC_TIMER, APIC_TIMER_VECTOR | 0x20000); // Mode = Periodic (bit 17)
    lapic_write(LAPIC_TICR, tick_count); // Set initial count
}

void apic_timer_handler() {
    printf("APIC Timer Interrupt!\n"); // Print message on each interrupt
    lapic_write(LAPIC_EOI, 0); // Send End of Interrupt (EOI)
}

void init_apic_timer(){
    interrupt_install_handler(0, &apic_timer_handler);
}






