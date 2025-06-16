/*
    Programmable Interval Timer (PIT) chip (Intel 8253/8254) 
    
    https://wiki.osdev.org/Programmable_Interval_Timer
    https://wiki.osdev.org/Programmable_Interval_Timer
*/

#include "../../process/process.h"
#include "../../process/thread.h"
#include "../../arch/interrupt/pic/pic.h"
#include "../../arch/interrupt/apic/apic.h"
#include "../../lib/stdio.h"
#include "../../driver/io/ports.h"
#include "../../arch/interrupt/apic/apic_interrupt.h"
#include "../../arch/interrupt/irq_manage.h"
#include "../../util/util.h"

#include "pit_timer.h"

#define PIT_TIMER_VECTOR  32    // 0x20 = 32, IRQ0 is the first interrupt vector in the PIC 
#define PIT_TIMER_IRQ 0         // 32-32 = 0

#define MAX_PIT_TICKS 0xFFFFFFFFFFFFFFFF

#define CHANEL0_DATA_PORT 0x40
#define CHANEL1_DATA_PORT 0x41
#define CHANEL2_DATA_PORT 0x42
#define COMMAND_PORT      0x43

static uint32_t PIT_FREQUENCY = 1193182;   // 1.193182 MHz = 1193182 Hz

extern void restore_cpu_state(registers_t* registers);
extern process_t *current_process;

volatile uint64_t pit_ticks = 0;

uint32_t frequency;

uint16_t read_pit_count(void) {
	uint16_t count = 0;
	
	// Disable interrupts
    asm volatile("cli");
	
	// al = channel in bits 6 and 7, remaining bits clear
	outb(COMMAND_PORT, 0b0000000);              // Read command: 0b00000000
	
	count = inb(CHANEL0_DATA_PORT);		        // Low byte
	count |= inb(CHANEL0_DATA_PORT) << 8;		// High byte
	
	return count;
}


void set_pit_count(unsigned count) {
	// Disable interrupts
    asm volatile("cli");
	
	// Set low byte
	outb(CHANEL0_DATA_PORT, count & 0xFF);		    // Low byte of divisor
	outb(CHANEL0_DATA_PORT, (count & 0xFF00) >> 8);	// High byte of divisor
    
	return;
}


void pit_timerHandler(registers_t *regs) {

    send_eoi(PIT_TIMER_IRQ);    // Send End of Interrupt (EOI) to the PIC
    
    if(pit_ticks >= MAX_PIT_TICKS) {
        pit_ticks = 0; // Reset pit_ticks value with zero
    }
    pit_ticks++;

    if (pit_ticks % 10 == 0){   // Prints in every in 100 ms = 0.1 sec interval
        printf("PIT Tick no : %d\n", pit_ticks);
    }

}

void enable_pit_timer(){
    irq_install(PIT_TIMER_IRQ, &pit_timerHandler); 
}

void disable_pit_timer(){
    irq_uninstall(PIT_TIMER_IRQ);
}

void init_pit_timer(uint32_t interval_ms) {
    asm volatile("cli");
    enable_pit_timer();

    // Compute frequency and divisor dynamically
    frequency = 1000 / interval_ms;  // Convert ms to Hz (interrupts per second)
    uint16_t divisor = (uint16_t)(PIT_FREQUENCY / frequency); // Calculate PIT divisor

    outb(COMMAND_PORT, 0x36);  // Command port: 0x36 for repeating square wave mode
    set_pit_count(divisor);    // Set PIT divisor

    asm volatile("sti");

    send_eoi(PIT_TIMER_IRQ);
    
    printf("[Info] PIT Timer initialized with %d ms interval (divisor: %d, frequency: %d Hz)\n", interval_ms, divisor, frequency);
}




// Delay function using busy-wait based on ticks
void pit_sleep(uint32_t millisec) {
    uint64_t endTicks = pit_ticks + ((frequency * millisec) / 1000);  // Convert milliseconds to ticks

    while (pit_ticks < endTicks){     // Busy wait for the timer to reach the desired tick count
        asm volatile ("hlt");
    } 
}





