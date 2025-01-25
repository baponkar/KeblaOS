
/*
The process control block (PCB) is a data structure in the operating 
system kernel containing the information needed to manage a particular process. 
The PCB is "the manifestation of a process in an operating system". 
The PCB is also known as the Task Control Block.

References :
https://wiki.osdev.org/Brendan%27s_Multi-tasking_Tutorial
https://wiki.osdev.org/Context_Switching
https://github.com/dreamportdev/Osdev-Notes/blob/master/05_Scheduling/README.md
https://web.archive.org/web/20160326122214/http://jamesmolloy.co.uk/tutorial_html/9.-Multitasking.html

*/


#include <stddef.h>

#include "../lib/string.h"
#include "../driver/vga.h"

#include "../x86_64/idt/idt.h"
#include "../x86_64/pit/pit_timer.h"

#include "../mmu/kheap.h"

#include "process.h"


#define STACK_SIZE 16*1024 // 16 KB

process_t *process_list = NULL;
process_t *current_process = NULL;

uint64_t next_pid = 1;

void add_process(process_t *process) {
    process->next = process_list;
    process_list = process;
}


void context_switch(registers_t *old_state, registers_t *new_state) {
    if (old_state) {
        // Save the current process's state
        *old_state = current_process->registers;
    }

    // Switch to the next process
    current_process = current_process->next ? current_process->next : process_list;

    if (new_state) {
        // Restore the next process's state
        *new_state = current_process->registers;
    }
}


void schedule(registers_t *regs) {
    disable_interrupts(); // Prevent interrupts during scheduling

    if (current_process != NULL) {
        current_process->registers = *regs; // Save the current process's state
        print("\nSaved state for PID: ");
        print_dec(current_process->pid);
        print("\n");
    }

    current_process = current_process->next ? current_process->next : process_list; // Switch to the next process

    if (current_process != NULL) {
        *regs = current_process->registers;
        print("Switched to PID: ");
        print_dec(current_process->pid);
        print("\n");
    }

    enable_interrupts(); // Re-enable interrupts after scheduling
}



process_t *create_process(void (*entry_point)(void)) {
    process_t *process = (process_t *)kheap_alloc(sizeof(process_t));
    process->pid = next_pid++;
    process->stack = (uint64_t *)kheap_alloc(STACK_SIZE);
    process->state = PROCESS_READY;
    process->next = NULL;

    // Initialize process registers
    memset(&process->registers, 0, sizeof(registers_t)); // Clear the registers
    process->registers.iret_rip = (uint64_t)entry_point;    // Set the instruction pointer to the entry point
    process->registers.iret_cs = 0x08; // Kernel code segment
    process->registers.iret_ss = 0x10; // Kernel stack segment
    process->registers.iret_rsp = (uint64_t)(process->stack + (STACK_SIZE / sizeof(uint64_t))); // Set the stack pointer to the end of the stack
    process->registers.iret_rflags = 0x202; // Interrupt flag enabled

    add_process(process);   // Add the process to the process list
    return process;
}


void terminate_process(process_t *process) {
    process_t *temp = process_list;
    process_t *prev = NULL;

    while (temp) {
        if (temp == process) {
            if (prev) {
                prev->next = temp->next;
            } else {
                process_list = temp->next;
            }

            kheap_free((void *)temp->stack, STACK_SIZE);
            kheap_free((void *)temp, sizeof(process_t));
            return;
        }

        prev = temp;
        temp = temp->next;
    }

    print("Successfully terminated process with PID: ");
    print_dec(process->pid);
    print("\n");
}



void process_1() {
    while (1) {
        print("Process 1 is running\n");
        delay(1000);
    }
}

void process_2() {
    while (1) {
        print("Process 2 is running\n");
        delay(1000);
    }
}



void init_processes() {
    create_process(&process_1);
    create_process(&process_2);

    // Debug: Print the process list after initialization
    print_process_list();
    
    // Initialize current_process to the first process in the list
    current_process = process_list; // Set the first process as current
}



void print_process_list() {
    process_t *temp = process_list;
    while (temp) {
        print("Process ID: ");
        print_dec(temp->pid);
        print(" | State: ");
        print_dec(temp->state);
        print("\n");
        temp = temp->next;
    }
}



