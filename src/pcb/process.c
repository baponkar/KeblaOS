
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
#include "../lib/stdio.h"
#include "../x86_64/interrupt/pic.h" // for enable_interrupt and disable_interrupt
#include "../x86_64/interrupt/interrupt.h"
#include "../lib/string.h"
#include "../x86_64/timer/apic_timer.h" // for apic_delay
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



void context_switch(registers_t *regs) {
     if (current_process != NULL) {
        // current_process->registers = *regs; // Save the current process's state
        current_process->registers.iret_rflags = 0x202; // Enable interrupts

        printf("\nSaved state for PID: %d\n", current_process->pid);
        printf("Saved Process name: %s\n", current_process->name);
    }

    current_process = current_process->next ? current_process->next : process_list; // Switch to the next process

    if (current_process != NULL) {
        *regs = current_process->registers;  // Restore the next process's state
        printf("Switched to PID: %d\n", current_process->pid);
        // look_into_process(current_process);
        // print_regs_content(&current_process->registers);
        // print_stack_contents(current_process);
    }
}




void schedule(registers_t *regs) {
    disable_interrupts(); // Prevent interrupts during scheduling
    context_switch(regs); // Perform the context switch
    enable_interrupts(); // Re-enable interrupts after scheduling
}



process_t *create_process(void (*entry_point)(void), char *name, void* arg) {
    process_t *process = (process_t *)kheap_alloc(sizeof(process_t));
    process->pid = next_pid++;
    strncpy(process->name, name, NAME_MAX_LEN);
    process->stack = (uint64_t *)kheap_alloc(STACK_SIZE);
    process->state = PROCESS_READY;
    process->next = NULL;

    // Initialize process registers
    memset(&process->registers, 0, sizeof(registers_t)); // Clear the registers
    process->registers.iret_rip = (uint64_t)entry_point;    // Set the instruction pointer to the entry point
    process->registers.iret_cs = 0x08; // Kernel code segment
    process->registers.iret_ss = 0x10; // Kernel stack segment
    process->registers.iret_rsp = (uint64_t)(process->stack + (STACK_SIZE / sizeof(uint64_t)) - 1); // Set the stack pointer to the end of the stack
    process->registers.iret_rflags = 0x202; // Interrupt flag enabled
    process->registers.rdi = (uint64_t)arg;
    process->registers.rbp = 0;

    memset(process->stack, 0, STACK_SIZE); // Clear the stack
    // process->stack[STACK_SIZE / sizeof(uint64_t) - 1] = 0xDEADBEEF;

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

    printf("Successfully terminated process with PID: %d\n", process->pid);
}



void process_1() {
    while (1) {
        printf("Process 1 is running\n");
        apic_delay(10000);
    }
}


void process_2() {
    while (1) {
        printf("Process 2 is running\n");
        apic_delay(10000);
    }
}



void init_processes() {
    create_process(&process_1, "Process 1", NULL);
    create_process(&process_2, "Process 2", NULL);

    // Debug: Print the process list after initialization
    print_process_list();
    
    // Initialize current_process to the first process in the list
    current_process = process_list; // Set the first process as current
}



void print_process_list() {
    process_t *temp = process_list;
    while (temp) {
        printf("Process ID: %d | State: %d\n", temp->pid, temp->state);
        temp = temp->next;
    }
}


void look_into_process(process_t *process) {
    printf("Current process: %d\n", process->pid);
    printf("Stack pointer: %x\n", process->registers.iret_rsp);
    printf("process->registers.iret_rip: %x\n", process->registers.iret_rip);
    printf("process->registers.iret_cs: %x\n",process->registers.iret_cs);
    printf("process->registers.iret_rflags: %x\n", process->registers.iret_rflags);
    printf("process->registers.iret_ss: %x\n", process->registers.iret_ss);
    printf("process->registers.iret_rsp: %x\n", process->registers.iret_rsp);
    printf("process->registers.rax: %x\n", process->registers.rax);
    printf("process->stack[STACK_SIZE / sizeof(uint64_t) - 1] : %x\n", process->registers.iret_rsp);
}


void print_stack_contents(process_t *process) {
    if (!process || !process->stack) {
        printf("Invalid process or stack pointer\n");
        return;
    }

    printf("Stack contents for process with PID: %d\n", process->pid);

    // Calculate the number of uint64_t elements in the stack
    size_t stack_elements = STACK_SIZE / sizeof(uint64_t);


    // Iterate through the stack and print each element
    for (size_t i = stack_elements - 1; i > stack_elements - 25; i--) {
        printf("Stack[%d] @ %x = %x\n", i, (uint64_t)&process->stack[i], process->stack[i]);
    }
}


