

#include "../../process/types.h"
#include "../../process/thread.h"
#include "../../process/process.h"
#include "../interrupt/pic.h"
#include "../interrupt/apic.h"
#include "../interrupt/interrupt.h"
#include "../../lib/string.h"
#include "../../util/util.h"
#include "../../lib/stdio.h"

#include "apic_timer.h"



#define LAPIC_BASE 0xFEE00000
#define APIC_TIMER_VECTOR  0x20  // Interrupt vector for timer

#define LAPIC_TPR                    (LAPIC_BASE + 0x80)   // Task Priority Register Offset
#define APIC_REGISTER_TIMER_DIV      (LAPIC_BASE + 0x3E0)  // APIC Timer Divide Configuration Register
#define APIC_REGISTER_TIMER_INITCNT  (LAPIC_BASE + 0x380)  // APIC Timer Initial Count Register
#define APIC_REGISTER_TIMER_CURRCNT  (LAPIC_BASE + 0x390)  // APIC Timer Current Count Register
#define APIC_REGISTER_LVT_TIMER      (LAPIC_BASE + 0x320)  // APIC Local Vector Table (LVT) Timer Register

#define APIC_LVT_TIMER_MODE_PERIODIC (1 << 17)             // Periodic mode bit
#define APIC_LVT_INT_MASKED          (1 << 16)             // Mask interrupt

static uint64_t cpu_frequency_hz = 0;  // Cached CPU frequency in Hz

int ticks1 = 0;

static inline uint64_t read_tsc() {
    uint32_t low, high;
    asm volatile ("rdtsc" : "=a"(low), "=d"(high)); // Read TSC
    return ((uint64_t)high << 32) | low;
}


void tsc_sleep(uint64_t microseconds) {
    uint64_t start = read_tsc();
    
    // Assuming a 3 GHz CPU, 1 cycle = 1 / 3 GHz = 0.333 ns
    // 1 ms = 3,000,000 cycles
    uint64_t cycles_to_wait = (microseconds * 3000); // Adjust based on CPU frequency

    enable_interrupts();  // ✅ Ensure interrupts are enabled

    while ((read_tsc() - start) < cycles_to_wait);
}


void apic_start_timer() {

    // Ensure the Local APIC is enabled
    mmio_write(LAPIC_TPR, 0x00); // Accept all interrupts

    // Set APIC timer to use divider 16
    mmio_write(APIC_REGISTER_TIMER_DIV, 0x3);
    
    // Set APIC initial count to max
    mmio_write(APIC_REGISTER_TIMER_INITCNT, 0xFFFFFFFF);
    
    // Sleep for 10,000 ms using TSC
    tsc_sleep(10000); // Sleep for 1 second (1000ms)
    
    // Stop APIC timer
    mmio_write(APIC_REGISTER_LVT_TIMER, APIC_LVT_INT_MASKED);
    
    // Calculate ticks in 10ms
    uint32_t ticksIn10ms = 0xFFFFFFFF - mmio_read(APIC_REGISTER_TIMER_CURRCNT);

    
    // Configure APIC timer in periodic mode with calculated ticks
    mmio_write(APIC_REGISTER_LVT_TIMER, APIC_TIMER_VECTOR | APIC_LVT_TIMER_MODE_PERIODIC);
    mmio_write(APIC_REGISTER_TIMER_DIV, 0x3);
    mmio_write(APIC_REGISTER_TIMER_INITCNT, ticksIn10ms);
    printf("[APIC] Timer set in periodic mode with ticks: %d\n", ticksIn10ms);

}


// void apic_delay(uint32_t milliseconds) {
    
//     // Calculate ticks for the given delay based on 10ms calibration
//     uint32_t ticks_per_ms = mmio_read(APIC_REGISTER_TIMER_CURRCNT) / 10;
//     uint32_t ticks_to_wait = ticks_per_ms * milliseconds;

//     // Configure APIC timer in one-shot mode
//     mmio_write(APIC_REGISTER_LVT_TIMER, APIC_LVT_INT_MASKED);
//     mmio_write(APIC_REGISTER_TIMER_INITCNT, ticks_to_wait);

//     // Wait for timer to reach zero
//     while (mmio_read(APIC_REGISTER_TIMER_CURRCNT) > 0);

// }


void apic_delay(uint32_t milliseconds) {
    uint32_t target_ticks = ticks1 + (milliseconds / 10);  // Convert ms to 10ms ticks
    while (ticks1 < target_ticks);
}



extern size_t    next_free_pid;     //Available free process id
extern process_t *current_process;  // Current running process
extern process_t *processes_list;   // List of all processes

extern registers_t* schedule(registers_t* regs);
extern void restore_cpu_state(registers_t* regs);


void apic_timer_handler(registers_t *regs) {
    ticks1++;
    apic_send_eoi();

    if (!current_process || !current_process->current_thread) {
        return;
    }

    // Save the current thread's register state
    current_process->current_thread->registers = regs;
    memcpy(current_process->current_thread->registers, regs, sizeof(registers_t));
    if (memcmp(current_process->current_thread->registers, regs, sizeof(registers_t)) != 0) {
        printf("registers assignment failed!\n");
        return;
    }

    registers_t* new_regs = schedule(regs); // Saving the current thread state and selecting the next thread

    if(new_regs){
        printf("[ Switching TID: %d | rip: %x | rsp: %x ]\n", current_process->current_thread->tid, new_regs->iret_rip, new_regs->iret_rsp);
        restore_cpu_state(new_regs);        // Restoring the next thread's state
    }
}


void init_apic_timer(){
    interrupt_install_handler(0, &apic_timer_handler);
    apic_start_timer();
    printf("Successfully APIC Timer enabled\n");
}






