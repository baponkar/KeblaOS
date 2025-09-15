/*
APIC TIMER:

https://wiki.osdev.org/APIC_Timer
https://github.com/dreamportdev/Osdev-Notes/blob/master/02_Architecture/08_Timers.md

*/

#include "../../memory/kmalloc.h"

#include "../../process/types.h"
#include "../../process/thread.h"
#include "../../process/process.h"

#include "../../arch/interrupt/pic/pic.h"
#include "../../arch/interrupt/apic/apic.h"
#include "../../arch/interrupt/apic/apic_interrupt.h"
#include "../../arch/interrupt/irq_manage.h"    // for irq_install

#include "../../lib/string.h"
#include "../../lib/stdio.h"

#include "../../driver/io/serial.h"

#include "../../util/util.h"

#include "tsc.h"
#include "pit_timer.h"

#include "apic_timer.h"


extern bool debug_on;

#define APIC_TIMER_VECTOR  48                   // APIC Timer Interrupt Vector 0x30
#define APIC_IRQ           16                   // APIC Timer Interrupt Request 48 - 32 = 16 (0x10)

#define MAX_APIC_TICKS 0xFFFFFFFFFFFFFFFF       // Maximum ticks for APIC timer

#define LAPIC_TPR_REGISTER           0x80       // Task Priority Register Offset
#define APIC_TIMER_DIV_REGISTER      0x3E0      // Offset for Timer Divide Register
#define APIC_TIMER_INITCNT_REGISTER  0x380      // Offset for Initial Count Register
#define APIC_TIMER_CURRCNT_REGISTER  0x390      // Offset for Current Count Register
#define APIC_LVT_TIMER_REGISTER      0x320      // Offset for LVT Timer Register

#define APIC_LVT_TIMER_MODE_PERIODIC (1 << 17)  // Periodic mode bit
#define APIC_LVT_TIMER_MODE_ONESHOT  (0 << 0 )  // One shot mode bit
#define APIC_LVT_INT_MASKED          (1 << 16)  // Mask interrupt

// Divider values
#define DIV_BY_2    0b000   // 0x0
#define DIV_BY_4    0b001   // 0x1
#define DIV_BY_8    0b010   // 0x2
#define DIV_BY_16   0b011   // 0x3
#define DIV_BY_32   0b100   // 0x4
#define DIV_BY_64   0b101   // 0x5
#define DIV_BY_128  0b110   // 0x6
#define DIV_BY_1    0b111   // 0x7
 
#define MAX_CPUS 256
volatile uint64_t apic_ticks[MAX_CPUS] = {0};

volatile bool apic_calibrated = false;
volatile uint64_t apic_timer_ticks_per_ms = 0;

uint8_t get_core_id() { return get_lapic_id(); }


void calibrate_apic_timer_tsc() {

    // Ensure the Local APIC is enabled
    apic_write(LAPIC_TPR_REGISTER, 0x0); // Accept all interrupts

    // 
    apic_write(APIC_TIMER_DIV_REGISTER, DIV_BY_16);

    // Reset APIC timer to max count
    apic_write(APIC_TIMER_INITCNT_REGISTER, 0xFFFFFFFF);

    tsc_sleep(100000);                  // Wait for 100 ms

    // Read remaining APIC timer count
    uint32_t end_count = apic_read(APIC_TIMER_CURRCNT_REGISTER); 

    // Calculate elapsed APIC ticks
    uint32_t elapsed_apic_ticks = 0xFFFFFFFF - end_count;

    // APIC Timer Frequency (ticks per second)
    uint64_t apic_timer_frequency = (uint64_t) (elapsed_apic_ticks * 10);

    // Calculate APIC Timer ticks per millisecond
    apic_timer_ticks_per_ms = apic_timer_frequency / 1000;

    apic_calibrated = true;

    if(debug_on) printf(" APIC Timer Frequency: %d ticks/ms\n", apic_timer_ticks_per_ms);
}


void calibrate_apic_timer_pit() {

    // Ensure the Local APIC is enabled
    apic_write(LAPIC_TPR_REGISTER, 0x0); // Accept all interrupts

    // Set Divisor 16
    apic_write(APIC_TIMER_DIV_REGISTER, DIV_BY_16);

    // Reset APIC timer to max count
    apic_write(APIC_TIMER_INITCNT_REGISTER, 0xFFFFFFFF);

    pit_sleep(1000);    // Wait for 1000 ms i.e 1 s

    // Read remaining APIC timer count
    uint32_t end_count = apic_read(APIC_TIMER_CURRCNT_REGISTER); 

    // Calculate elapsed APIC ticks
    uint32_t elapsed_apic_ticks = 0xFFFFFFFF - end_count;

    // APIC Timer Frequency (ticks per second)
    uint64_t apic_timer_frequency = (uint64_t)elapsed_apic_ticks;

    // Calculate APIC Timer ticks per millisecond
    apic_timer_ticks_per_ms = apic_timer_frequency / 1000;

    apic_calibrated = true;

    if(debug_on) printf(" APIC Timer Frequency: %d ticks/ms\n", apic_timer_ticks_per_ms);
}


void apic_start_oneshot_timer(uint32_t initial_count) {
    // Ensure the Local APIC is enabled
    apic_write(LAPIC_TPR_REGISTER, 0x0); // Accept all interrupts

    // Set APIC timer to use divider 16
    apic_write(APIC_TIMER_DIV_REGISTER, DIV_BY_16);
    
    // Set APIC initial count to max
    apic_write(APIC_TIMER_INITCNT_REGISTER, initial_count);

    // Set APIC Timer Mode (Periodic Mode = 0x20000, One-shot = 0x0)
    apic_write(APIC_LVT_TIMER_REGISTER, APIC_TIMER_VECTOR | APIC_LVT_TIMER_MODE_ONESHOT);  // Vector 48, one-shot mode
}



void apic_timer_handler(registers_t *regs) {

    uint32_t cpu_id = get_core_id(); // Implement this using APIC ID or CPU-local ID

    if(apic_ticks[cpu_id] >= MAX_APIC_TICKS) {
        apic_ticks[cpu_id] = 0;     // Starts Recount
    }

    apic_ticks[cpu_id] += apic_timer_ticks_per_ms;

    if(apic_ticks[cpu_id] % 100 == 0){
        // printf("apic_ticks[%d]: %d\n", cpu_id, apic_ticks[cpu_id]);
        // serial_printf("apic_ticks[%d]: %d\n", cpu_id, apic_ticks[cpu_id]);
    }

    apic_send_eoi();
}



void init_apic_timer(uint32_t interval_ms) {// Start APIC timer with a large count

    if(is_apic_enabled() == false) {
        printf(" APIC is not enabled. Cannot initialize APIC timer.\n");
        return;
    }

    uint64_t cpu_id = get_core_id();        // Get the current CPU core ID

    apic_ticks[cpu_id] = 0;                 // Initialize APIC ticks for this CPU core

    asm volatile("cli");

    irq_install(APIC_IRQ, (void *)&apic_timer_handler);

    apic_start_oneshot_timer(0xFFFFFFFF);   // Set APIC timer to max count
    
    if(apic_timer_ticks_per_ms == 0) {
        // calibrate_apic_timer_tsc();      // Calibrate APIC timer using TSC
        calibrate_apic_timer_pit();         // Calibrate APIC timer using PIT

        while (!apic_calibrated);           // Wait for calibration to complete
        printf(" APIC Timer calibrated with %d ticks/ms\n", apic_timer_ticks_per_ms);
    }

    uint32_t apic_count = apic_timer_ticks_per_ms * interval_ms;
    
    // Set APIC Timer for periodic interrupts
    apic_write(APIC_LVT_TIMER_REGISTER, APIC_TIMER_VECTOR | APIC_LVT_TIMER_MODE_PERIODIC);  // Vector 48, Periodic Mode
    apic_write(APIC_TIMER_DIV_REGISTER , DIV_BY_16);                                        // Set divisor by 16
    apic_write(APIC_TIMER_INITCNT_REGISTER, apic_timer_ticks_per_ms * interval_ms);         // Set initial count for timer

    asm volatile("sti");

    if(debug_on) printf(" APIC Timer initialized with %d ms interval in CPU: %d.\n", interval_ms, get_lapic_id());
}



void apic_delay(uint32_t milliseconds) {  
    uint32_t cpu_id = get_core_id(); 

    uint64_t start_ticks = apic_ticks[cpu_id];
    uint64_t target_ticks = start_ticks +  milliseconds * apic_timer_ticks_per_ms; 

    while (apic_ticks[cpu_id] < target_ticks) {
        asm volatile("hlt");
        printf("Inside of apic_delay: ticks = %d, start_ticks: %d, target_ticks: %d\n",
        apic_ticks[cpu_id], start_ticks, target_ticks);
        printf("apic_timer_ticks_per_ms: %d\n", apic_timer_ticks_per_ms);
    }
}











