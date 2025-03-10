/*
    https://wiki.osdev.org/Programmable_Interval_Timer
*/

#include "../../process/process.h"
#include "../../process/thread.h"
#include "../interrupt/pic.h"
#include "../interrupt/apic.h"
#include "tsc.h"
#include "../../lib/stdio.h"
#include "../../driver/io/ports.h"
#include "../interrupt/interrupt.h"
#include "../../util/util.h"

#include "pit_timer.h"

#define MAX_PIT_TICKS 0xFFFFFFFFFFFFFFFF

#define PIT_TIMER_VECTOR 0x20   // 32

#define CHANEL0_DATA_PORT 0x40
#define CHANEL1_DATA_PORT 0x41
#define CHANEL2_DATA_PORT 0x42
#define COMMAND_PORT      0x43
#define PIT_FREQUENCY     1193180   // 1.193182 MHz = 1193182 Hz

extern void restore_cpu_state(registers_t* registers);
extern process_t *current_process;

uint64_t pit_ticks = 0;
uint32_t frequency = 100;           // 100 Hz -> 10 ms per tick



void pit_timerHandler(registers_t *regs) {

    // if(pit_ticks >= MAX_PIT_TICKS) pit_ticks = 0; // Reset pit_ticks value with zero
    pit_ticks++;

    // if (ticks % 100 == 0) printf("PIT Tick no : %d\n", ticks);  // Prints every 1 sec

    if (pit_ticks % 10 == 0){   // Prints in every 0.1 sec interval
        printf("(Inside of pit_timerHandler) PIT Tick no : %d\n", pit_ticks);
        // Saving the current thread state and selecting the next thread
        // registers_t* new_regs = schedule(regs); 

        // if(new_regs){
        //     printf("Switching Thread: %d | rip: %x | rsp: %x\n", 
        //         current_process->current_thread->tid,
        //         current_process->current_thread->registers.iret_rip,
        //         current_process->current_thread->registers.iret_rsp);
        //     restore_cpu_state(new_regs);        // Restoring the next thread's state
        // }
    }

    outb(0x20, 0x20); // Send End of Interrupt (EOI) to the PIC
}


void init_pit_timer() {

    interrupt_install_handler((PIT_TIMER_VECTOR -  32), &pit_timerHandler);            // IRQ0 for PIT timer

    uint16_t divisor = (uint16_t) PIT_FREQUENCY / frequency;    // Divisor max value 65535

    // Command port: 0x36 for repeating square wave mode
    outb(COMMAND_PORT, 0x36);  
    // Send the frequency divisor (LSB first, then MSB)
    outb(CHANEL0_DATA_PORT, (uint8_t) (divisor & 0xFF) );         // Low byte of divisor
    outb(CHANEL0_DATA_PORT, (uint8_t) ((divisor & 0xFF00) >> 8));  // High byte of divisor
    
    printf("Successfully PIT timer initialized.\n");
}



// Delay function using busy-wait based on ticks
void pit_sleep(uint32_t millisec) {
    uint64_t endTicks = pit_ticks + ((frequency * millisec) / 1000);  // Convert milliseconds to ticks

    while (pit_ticks < endTicks){             // Busy wait for the timer to reach the desired tick count
        // asm volatile ("sti; hlt; cli");
        printf("Inside of pit_sleep: pit_ticks: %d\n", pit_ticks);
    } 
}

