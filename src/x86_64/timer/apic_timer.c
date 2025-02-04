
#include "../interrupt/pic.h"
#include "../interrupt/apic.h"
#include "../../util/util.h"
#include "../../lib/stdio.h"

#include "apic_timer.h"


#define LAPIC_TIMER     0x320  // Timer Interrupt Register
#define LAPIC_SVR       0xF0   // Spurious Interrupt Vector Register
#define LAPIC_TICR      0x380  // Timer Initial Count Register
#define LAPIC_TCCR      0x390  // Timer Current Count Register
#define LAPIC_TDCR      0x3E0  // Timer Divide Configuration Register
#define TIMER_VECTOR    0x20   // IRQ 0 (Programmable Timer Interrupt)

extern uint64_t LAPIC_BASE;
uint64_t apic_ticks = 0;


void apic_remap_timer(uint32_t initial_count) {

    // Set APIC Timer to periodic mode and assign IRQ vector 0x20
    mmio_write((LAPIC_BASE + LAPIC_TIMER), 0x20 | (1 << 17)); // Periodic mode, vector 0x20

    // Set Timer Divide Configuration (Divide by 16)
    mmio_write((LAPIC_BASE + LAPIC_TDCR), 0x3);

    // Set Initial Count (determines timer frequency)
    mmio_write((LAPIC_BASE + LAPIC_TICR), initial_count);  // Arbitrary value, calibrate for actual frequency
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
    printf("\nLAPIC_SVR before enable: %x\n", mmio_read(LAPIC_BASE + LAPIC_SVR));
    mmio_write((LAPIC_BASE + LAPIC_SVR), mmio_read(LAPIC_BASE + LAPIC_SVR) | 0x100);
    printf("LAPIC_SVR after enable: %x\n", mmio_read(LAPIC_BASE + LAPIC_SVR));

    // Set timer divide configuration (Divide by 16)
    mmio_write((LAPIC_BASE + LAPIC_TDCR), 0x3);
    printf("LAPIC_TDCR: %x\n", mmio_read(LAPIC_BASE + LAPIC_TDCR));

    // Set initial count for calibration (large value)
    mmio_write((LAPIC_BASE + LAPIC_TICR), 0xFFFFFFFF);
    printf("LAPIC_TICR after set: %x\n", mmio_read(LAPIC_BASE + LAPIC_TICR));

    // Wait a bit (using PIT or another timer for calibration)
    for (volatile int i = 0; i < 1000000; i++);

    // Measure elapsed ticks (TSC or another timer)
    uint32_t elapsed = 0xFFFFFFFF - mmio_read(LAPIC_BASE + LAPIC_TCCR);
    printf("Elapsed ticks: %x\n", elapsed);
    
    // Calculate the required APIC timer count for the desired frequency
    uint32_t ticks_per_ms = elapsed / 10; // Assuming 10ms elapsed
    uint32_t initial_count = ticks_per_ms * (1000 / frequency);
    
    // Set the APIC Timer to periodic mode
    mmio_write((LAPIC_BASE + LAPIC_TIMER), TIMER_VECTOR | 0x20000);
    printf("LAPIC_TIMER: %x\n", mmio_read(LAPIC_BASE + LAPIC_TIMER));

    printf("LAPIC_TCCR before start: %x\n", mmio_read(LAPIC_BASE + LAPIC_TCCR));
    mmio_write((LAPIC_BASE + LAPIC_TICR), initial_count);  // Periodic mode, vector 0x20
    printf("LAPIC_TCCR after start: %x\n", mmio_read(LAPIC_BASE + LAPIC_TCCR));

    // install apic timer interrupt handler apic_timerHandler
    interrupt_install_handler(TIMER_VECTOR, &apic_timerHandler); 

    apic_remap_timer(initial_count);

    enable_interrupts();

    printf("APIC Timer initialized for periodic mode.\n");
}



#define APIC_APICID	0x20
#define APIC_APICVER 0x30
#define APIC_TASKPRIOR	0x80
#define APIC_EOI 0x0B0
#define APIC_LDR 0x0D0
#define APIC_DFR 0x0E0
#define APIC_SPURIOUS 0x0F0
#define APIC_ESR 0x280
#define APIC_ICRL 0x300
#define APIC_ICRH 0x310
#define APIC_LVT_TMR 0x320
#define APIC_LVT_PERF 0x340
#define APIC_LVT_LINT0 0x350
#define APIC_LVT_LINT1 0x360
#define APIC_LVT_ERR 0x370
#define APIC_TMRINITCNT 0x380
#define APIC_TMRCURRCNT 0x390
#define APIC_TMRDIV 0x3E0
#define APIC_LAST 0x38F
#define APIC_DISABLE 0x10000
#define APIC_SW_ENABLE 0x100
#define APIC_CPUFOCUS 0x200
#define APIC_NMI (4<<8)
#define TMR_PERIODIC 0x20000
#define TMR_BASEDIV	(1<<20)

#define APIC_REGISTER_TIMER_DIV      (LAPIC_BASE + 0x3E0)  // Timer Divide Configuration Register
#define APIC_REGISTER_TIMER_INITCNT  (LAPIC_BASE + 0x380)  // Timer Initial Count Register
#define APIC_REGISTER_TIMER_CURRCNT  (LAPIC_BASE + 0x390)  // Timer Current Count Register
#define APIC_REGISTER_LVT_TIMER      (LAPIC_BASE + 0x320)  // Local APIC Timer Register

#define APIC_LVT_INT_MASKED         (1 << 16)  // Mask timer interrupt
#define APIC_LVT_TIMER_MODE_PERIODIC (1 << 17) // Set timer to periodic mode
#define APIC_LVT_TIMER_MODE_ONESHOT  (0 << 17) // One-shot mode (default)
#define APIC_LVT_TIMER_MODE_TSCDEADL (2 << 17) // TSC Deadline mode (if supported)



void apic_start_timer() {
        disable_interrupts();
        // Tell APIC timer to use divider 16
        mmio_write(APIC_REGISTER_TIMER_DIV, 0x3);
 
        // Prepare the PIT to sleep for 10ms (10000µs)
        for (volatile int i = 0; i < 1000000; i++);
 
        // Set APIC init counter to -1
        mmio_write(APIC_REGISTER_TIMER_INITCNT, 0xFFFFFFFF);
 
        // Perform PIT-supported sleep
        for (volatile int i = 0; i < 1000000; i++);
 
        // Stop the APIC timer
        mmio_write(APIC_REGISTER_LVT_TIMER, APIC_LVT_INT_MASKED);
 
        // Now we know how often the APIC timer has ticked in 10ms
        uint32_t ticksIn10ms = 0xFFFFFFFF - mmio_read(APIC_REGISTER_TIMER_CURRCNT);
 
        // Start timer as periodic on IRQ 0, divider 16, with the number of ticks we counted
        mmio_write(APIC_REGISTER_LVT_TIMER, 32 | APIC_LVT_TIMER_MODE_PERIODIC);
        mmio_write(APIC_REGISTER_TIMER_DIV, 0x3);
        mmio_write(APIC_REGISTER_TIMER_INITCNT, ticksIn10ms);

        // install apic timer interrupt handler apic_timerHandler
        interrupt_install_handler(0, &apic_timerHandler); 

        apic_remap_timer(ticksIn10ms);

        enable_interrupts();
}

