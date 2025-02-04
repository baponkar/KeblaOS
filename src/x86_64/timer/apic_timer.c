
#include "../interrupt/pic.h"
#include "../interrupt/apic.h"
#include "../../util/util.h"
#include "../../lib/stdio.h"

#include "apic_timer.h"

#define LAPIC_TIMER     0x320
#define LAPIC_TICR      0x380  // Timer Initial Count Register
#define LAPIC_TCCR      0x390  // Timer Current Count Register
#define LAPIC_TDCR      0x3E0  // Timer Divide Configuration Register
#define LAPIC_EOI       0xB0   // End Of Interrupt Register
#define LAPIC_SVR       0xF0   // Spurious Interrupt Vector Register
#define TIMER_VECTOR    0x20   // IRQ 0 (Programmable Timer Interrupt)

extern uint64_t LAPIC_BASE;
uint64_t apic_ticks = 0;

// void apic_remap_timer() {
//     mmio_write(APIC_TIMER, 0x20);  // Set APIC Timer IRQ vector to 0x20
//     mmio_write(APIC_LVT, 0x20);    // Set LVT entry to 0x20
// }

void apic_remap_timer() {
    // Set APIC Timer to periodic mode and assign IRQ vector 0x20
    mmio_write(LAPIC_BASE + LAPIC_TIMER, 0x20 | (1 << 17)); // Periodic mode, vector 0x20

    // Set Timer Divide Configuration (Divide by 16)
    mmio_write(LAPIC_BASE + LAPIC_TDCR, 0x3);

    // Set Initial Count (determines timer frequency)
    mmio_write(LAPIC_BASE + LAPIC_TICR, 10000000);  // Arbitrary value, calibrate for actual frequency
}

void apic_timerHandler(registers_t *regs) {
    (void)regs; // Suppress unused parameter warning if not used.

    apic_ticks++;
    printf("Tick: %d\n", apic_ticks);

    // Send EOI to the APIC
    apic_send_eoi();
}


void apic_timer_init(uint32_t frequency) {

    disable_interrupts();

    // Ensure APIC is enabled (set Spurious Interrupt Vector Register)
    mmio_write(LAPIC_BASE + LAPIC_SVR, mmio_read(LAPIC_BASE + LAPIC_SVR) | 0x100);

    // Set timer divide configuration (Divide by 16)
    mmio_write(LAPIC_BASE + LAPIC_TDCR, 0x3);

    // Set initial count for calibration (large value)
    mmio_write(LAPIC_BASE + LAPIC_TICR, 0xFFFFFFFF);

    // Wait a bit (using PIT or another timer for calibration)
    for (volatile int i = 0; i < 1000000; i++);

    // Measure elapsed ticks (TSC or another timer)
    uint32_t elapsed = 0xFFFFFFFF - mmio_read(LAPIC_BASE + LAPIC_TCCR);
    
    // Calculate the required APIC timer count for the desired frequency
    uint32_t ticks_per_ms = elapsed / 10; // Assuming 10ms elapsed
    uint32_t initial_count = ticks_per_ms * (1000 / frequency);
    
    // Set the APIC Timer to periodic mode
    mmio_write(LAPIC_BASE + LAPIC_TIMER, TIMER_VECTOR | 0x20000);
    mmio_write(LAPIC_BASE + LAPIC_TICR, initial_count);

    // install apic timer interrupt handler apic_timerHandler
    interrupt_install_handler(TIMER_VECTOR, &apic_timerHandler); 
    
    apic_remap_timer();

    enable_interrupts();

    printf("APIC Timer initialized for periodic mode.\n");
}







