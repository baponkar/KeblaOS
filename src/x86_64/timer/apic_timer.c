/*
APIC TIMER:

https://wiki.osdev.org/APIC_Timer
https://github.com/dreamportdev/Osdev-Notes/blob/master/02_Architecture/08_Timers.md

*/

#include "../../process/types.h"
#include "../../process/thread.h"
#include "../../process/process.h"
#include "../interrupt/pic.h"
#include "../interrupt/apic.h"
#include "../interrupt/interrupt.h"
#include "../../lib/string.h"
#include "../../util/util.h"
#include "../../lib/stdio.h"
#include "tsc.h"

#include "apic_timer.h"



#define LAPIC_BASE 0xFEE00000
#define APIC_TIMER_VECTOR  0x30  // Interrupt vector for timer

#define LAPIC_TPR                    (LAPIC_BASE + 0x80)   // Task Priority Register Offset
#define APIC_REGISTER_TIMER_DIV      (LAPIC_BASE + 0x3E0)  // APIC Timer Divide Configuration Register
#define APIC_REGISTER_TIMER_INITCNT  (LAPIC_BASE + 0x380)  // APIC Timer Initial Count Register
#define APIC_REGISTER_TIMER_CURRCNT  (LAPIC_BASE + 0x390)  // APIC Timer Current Count Register
#define APIC_REGISTER_LVT_TIMER      (LAPIC_BASE + 0x320)  // APIC Local Vector Table (LVT) Timer Register

#define APIC_LVT_TIMER_MODE_PERIODIC (1 << 17)             // Periodic mode bit
#define APIC_LVT_INT_MASKED          (1 << 16)             // Mask interrupt

extern uint64_t cpu_frequency_hz;  // Cached CPU frequency in Hz
// uint64_t cpu_frequency_hz = 1600000000;  // 1.60 GHz

int max_ticks = 4294967295;
int apic_ticks = 0;


void apic_start_timer() {

    // Ensure the Local APIC is enabled
    mmio_write(LAPIC_TPR, 0x0); // Accept all interrupts

    // Set APIC timer to use divider 16
    mmio_write(APIC_REGISTER_TIMER_DIV, 0x3);
    
    // Set APIC initial count to max
    mmio_write(APIC_REGISTER_TIMER_INITCNT, 0xFFFFFFFF);
    
    tsc_sleep(10000);
    
    // Stop APIC timer
    mmio_write(APIC_REGISTER_LVT_TIMER, APIC_LVT_INT_MASKED);
    
    // Calculate ticks in 10ms
    uint32_t ticksIn10ms = 0xFFFFFFFF - mmio_read(APIC_REGISTER_TIMER_CURRCNT);
    printf("ticksIn10ms : %d\n", ticksIn10ms);
    
    // Configure APIC timer in periodic mode with calculated ticks
    mmio_write(APIC_REGISTER_LVT_TIMER, APIC_TIMER_VECTOR | APIC_LVT_TIMER_MODE_PERIODIC);
    mmio_write(APIC_REGISTER_TIMER_INITCNT, ticksIn10ms);
    printf("[APIC] Timer set in periodic mode with ticks: %d\n", ticksIn10ms);
}


void apic_delay(uint32_t milliseconds) {
    uint32_t ticks_per_ms = cpu_frequency_hz / 1000;
    uint32_t ticks_to_wait = ticks_per_ms * milliseconds;

    // Configure APIC timer in one-shot mode
    mmio_write(APIC_REGISTER_LVT_TIMER, APIC_TIMER_VECTOR);
    mmio_write(APIC_REGISTER_TIMER_INITCNT, ticks_to_wait);

    // Wait for the timer to expire
    while (mmio_read(APIC_REGISTER_TIMER_CURRCNT) > 0);
}




// void apic_delay(uint32_t milliseconds) {
//     uint32_t target_ticks = apic_ticks + (milliseconds / 10);  // Convert ms to 10ms ticks
//     while (apic_ticks < target_ticks);
// }



extern size_t    next_free_pid;     //Available free process id
extern process_t *current_process;  // Current running process
extern process_t *processes_list;   // List of all processes

extern void restore_cpu_state(registers_t* registers);


void apic_timer_handler(registers_t *regs) {

    if((max_ticks-apic_ticks) >= 0) apic_ticks = 0;
    apic_ticks++;
    apic_send_eoi();

    // printf("Tick: %d\n", apic_ticks);

    // if (!current_process || !current_process->current_thread) {
        // printf("\nInitial regs: rip: %x | rsp: %x\n", regs->iret_rip, regs->iret_rsp);
    //     return;
    // }

    // Saving the current thread state and selecting the next thread
    // registers_t* new_regs = schedule(regs); 

    // if(new_regs){
    //     // printf("Switching Thread: %d | rip: %x | rsp: %x\n", 
    //     //     current_process->current_thread->tid,
    //     //     current_process->current_thread->registers.iret_rip,
    //     //     current_process->current_thread->registers.iret_rsp);
    //     restore_cpu_state(new_regs);        // Restoring the next thread's state
    // }

}


void init_apic_timer(){
    enable_apic();
    init_tsc();
    printf("CPU Frequency: %d Hz (%d GHz)\n", cpu_frequency_hz, cpu_frequency_hz/1000000000);
    interrupt_install_handler((APIC_TIMER_VECTOR - 32), &apic_timer_handler);
    apic_start_timer();
    printf("Successfully APIC Timer enabled\n");
}

