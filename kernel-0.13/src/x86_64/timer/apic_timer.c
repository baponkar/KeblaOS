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
#include "../interrupt/irq_manage.h"
#include "../../lib/string.h"
#include "../../util/util.h"
#include "../../lib/stdio.h"
#include "tsc.h"
#include "pit_timer.h"
#include "../interrupt/apic.h"

#include "../../../../lvgl-9.2.2/lvgl.h"  // LVGL library

#include "apic_timer.h"


#define APIC_TIMER_VECTOR  48
#define APIC_IRQ 16     // 48 - 32

#define MAX_APIC_TICKS  0xFFFFFFFFFFFFFFFF

#define LAPIC_TPR                    0x80       // Task Priority Register Offset

#define APIC_REGISTER_TIMER_DIV      0x3E0      // Offset for Timer Divide Register
#define APIC_REGISTER_TIMER_INITCNT  0x380      // Offset for Initial Count Register
#define APIC_REGISTER_TIMER_CURRCNT  0x390      // Offset for Current Count Register
#define APIC_REGISTER_LVT_TIMER      0x320      // Offset for LVT Timer Register

#define APIC_LVT_TIMER_MODE_PERIODIC (1 << 17)  // Periodic mode bit
#define APIC_LVT_TIMER_MODE_ONESHOT  (0 << 0 )  // One shot mode
#define APIC_LVT_INT_MASKED          (1 << 16)  // Mask interrupt


extern void restore_cpu_state(registers_t* registers);
extern process_t *current_process; 

extern volatile uint64_t pit_ticks;
volatile uint64_t apic_ticks = 0;                 // Cached CPU frequency in Hz
volatile uint64_t apic_timer_ticks_per_ms = 0;



void apic_start_timer(uint32_t initial_count) {
    // Ensure the Local APIC is enabled
    apic_write(LAPIC_TPR, 0x0); // Accept all interrupts

    // Set APIC timer to use divider 16
    apic_write(APIC_REGISTER_TIMER_DIV, 0x3);
    
    // Set APIC initial count to max
    apic_write(APIC_REGISTER_TIMER_INITCNT, initial_count);

    // Set APIC Timer Mode (Periodic Mode = 0x20000, One-shot = 0x0)
    apic_write(APIC_REGISTER_LVT_TIMER, APIC_TIMER_VECTOR | APIC_LVT_TIMER_MODE_ONESHOT);  // Vector 48, one-shot mode
}



void calibrate_apic_timer_pit() {

    // Reset APIC timer to max count
    apic_write(APIC_REGISTER_TIMER_INITCNT, 0xFFFFFFFF);

    // Measuring apic_timer_ticks_per_ms by pit
    pit_sleep(100);    // Wait for 10 PIT ticks (100 ms delay)

    // Read remaining APIC timer count
    uint32_t end_count = apic_read(APIC_REGISTER_TIMER_CURRCNT); 

    // Calculate APIC Timer ticks per millisecond
    apic_timer_ticks_per_ms = ( 0xFFFFFFFF - end_count) / 100;
}



void calibrate_apic_timer_tsc() {

    // Reset APIC timer to max count
    apic_write(APIC_REGISTER_TIMER_INITCNT, 0xFFFFFFFF);

    // Measuring apic_timer_ticks_per_ms by pit
    tsc_sleep(100000);    // Wait for 100 ms

    // Read remaining APIC timer count
    uint32_t end_count = apic_read(APIC_REGISTER_TIMER_CURRCNT); 

    // Calculate elapsed APIC ticks
    uint32_t elapsed_apic_ticks = 0xFFFFFFFF - end_count;

    // APIC Timer Frequency (ticks per second)
    uint64_t apic_timer_frequency = (uint64_t) (elapsed_apic_ticks * 10);

    // Calculate APIC Timer ticks per millisecond
    apic_timer_ticks_per_ms = apic_timer_frequency / 1000;

    // printf("APIC Timer Frequency: %d ticks/ms\n", apic_timer_ticks_per_ms);
}



void apic_timer_handler(registers_t *regs) {
    if(apic_ticks >= MAX_APIC_TICKS) apic_ticks = 0;

    apic_ticks++;

    apic_send_eoi();

    if(apic_ticks % 100 == 0){
        // toggle_cursor();
        // draw_cursor();
    }

    // if(apic_ticks % 100 != 0 && apic_ticks % 120 != 0 && apic_ticks % 130 != 0 && apic_ticks % 140 != 0)
        // printf("CPU %d : APIC Tick: %d\n", get_lapic_id(), apic_ticks);

    // if(get_lapic_id() == 0 && apic_ticks % 100 == 0){
    //     printf("CPU %d : APIC Tick: %d\n", get_lapic_id(), apic_ticks);
    // }else if(get_lapic_id() == 1 && apic_ticks % 120 == 0){
    //     printf("CPU %d : APIC Tick: %d\n", get_lapic_id(), apic_ticks);
    // }else if(get_lapic_id() == 2 && apic_ticks % 130 == 0){
    //     printf("CPU %d : APIC Tick: %d\n", get_lapic_id(), apic_ticks);
    // }else if(get_lapic_id() == 3 && apic_ticks % 140 == 0){
    //     printf("CPU %d : APIC Tick: %d\n", get_lapic_id(), apic_ticks);
    // }

    // if(get_lapic_id() == 0 && current_process && current_process->current_thread){
    //     registers_t *new_regs = schedule(regs);
        
    //     if(new_regs){
    //         printf("=>current thread: %s, rip: %x, rsp: %x\n", 
    //             current_process->current_thread->name,  
    //             current_process->current_thread->registers.iret_rip,
    //             current_process->current_thread->registers.iret_rsp);
    //         restore_cpu_state(new_regs);
    //     }
    // }

    // if(apic_ticks % 2 && get_lapic_id() == 1 && current_process->next && current_process->next->current_thread){
    //     // printf("=>current thread: %s, rip: %x, rsp: %x\n", 
    //     //     current_process->current_thread->name,  
    //     //     current_process->current_thread->registers.iret_rip,
    //     //     current_process->current_thread->registers.iret_rsp);
    //     registers_t *new_regs = schedule(regs);
    //     if(new_regs){
    //         // printf("=>current thread: %s, rip: %x, rsp: %x\n", 
    //         //     current_process->current_thread->name,  
    //         //     current_process->current_thread->registers.iret_rip,
    //         //     current_process->current_thread->registers.iret_rsp);
    //         restore_cpu_state(new_regs);
    //     }
    // }
}




void init_apic_timer(uint32_t interval_ms) {// Start APIC timer with a large count

    apic_start_timer(0xFFFFFFFF);
    calibrate_apic_timer_tsc();
    // calibrate_apic_timer_pit();

    uint32_t apic_count = apic_timer_ticks_per_ms * interval_ms;
    
    asm volatile("cli");
    // Set APIC Timer for periodic interrupts
    apic_write(APIC_REGISTER_LVT_TIMER, APIC_TIMER_VECTOR | APIC_LVT_TIMER_MODE_PERIODIC);   // Vector 48, Periodic Mode
    apic_write(APIC_REGISTER_TIMER_DIV , 0x3);                          // Set divisor
    apic_write(APIC_REGISTER_TIMER_INITCNT, apic_count);

    irq_install(APIC_IRQ, &apic_timer_handler);
    asm volatile("sti");

    printf("[Info] APIC Timer initialized with %d ms interval in CPU: %d.\n", interval_ms, get_lapic_id());
}



void apic_delay(uint32_t milliseconds) {    
    uint64_t start_ticks = apic_ticks;
    uint64_t target_ticks = start_ticks + ((uint64_t) milliseconds * apic_timer_ticks_per_ms) / 1000; 

    while (apic_ticks < target_ticks) {
        asm volatile ("hlt");
    }
}











