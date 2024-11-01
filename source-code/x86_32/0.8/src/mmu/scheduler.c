#include "scheduler.h"
#include "task.h"

scheduler_t scheduler;  // Global scheduler instance

void scheduler_add(process_t *process) {
    if (scheduler.num_processes < MAX_PROCESSES) {
        scheduler.processes[scheduler.num_processes++] = process;
    } else {
        print("Scheduler process limit reached.\n");
    }
}


void scheduler_init() {
    scheduler.num_processes = 0;
    scheduler.current_index = -1;  // No process running initially
}

void context_switch(process_t *next_process) {
    // Save the current process's state
    asm volatile ("mov %%esp, %0" : "=r"(current_process->esp));  // Save stack pointer
    asm volatile ("mov %%ebp, %0" : "=r"(current_process->ebp));  // Save base pointer
    current_process->eip = get_eip();  // Save instruction pointer

    // Switch to the next process
    current_process = next_process;
    asm volatile ("mov %0, %%esp" :: "r"(next_process->esp));  // Restore stack pointer
    asm volatile ("mov %0, %%ebp" :: "r"(next_process->ebp));  // Restore base pointer
    set_eip(next_process->eip);  // Restore instruction pointer

    // Switch page directory (paging)
    switch_page_directory(next_process->page_dir);
}

void scheduler() {
    if (scheduler.num_processes == 0) {
        print("No processes to schedule.\n");
        return;
    }

    // Move to the next process in the queue
    scheduler.current_index = (scheduler.current_index + 1) % scheduler.num_processes;
    process_t *next_process = scheduler.processes[scheduler.current_index];

    // Perform the context switch to the next process
    context_switch(next_process);
}

void timer_interrupt_handler() {
    scheduler();  // Call the scheduler to switch processes
    send_eoi();   // End of interrupt signal to the PIC (if using the 8259 PIC)
}

void init_timer(uint32_t frequency) {
    uint32_t divisor = 1193180 / frequency;
    outb(0x43, 0x36);                // Command byte for PIT
    outb(0x40, divisor & 0xFF);      // Low byte of divisor
    outb(0x40, (divisor >> 8) & 0xFF);  // High byte of divisor
}

void syscall_yield() {
    scheduler();  // Call the scheduler to switch processes
}

