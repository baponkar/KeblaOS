
#include "ps.h"


#define PROCESS_READY    0
#define PROCESS_RUNNING  1
#define PROCESS_BLOCKED  2

#define STACK_SIZE 16384 // 16 KB

extern void switch_to_process(process_t *current, process_t *next);

process_t *current_process = NULL;
process_t *process_list = NULL; // Linked list of all processes


process_t *create_process(void (*entry_point)()) {
    process_t *new_proc = (process_t *)kheap_alloc(sizeof(process_t));
    new_proc->stack = (uint64_t *)kheap_alloc(STACK_SIZE);

    // Set up stack and registers
    uint64_t *stack_top = new_proc->stack + (STACK_SIZE / sizeof(uint64_t));
    new_proc->cr3 = create_new_pml4(); // Create a new page table
    new_proc->regs = (registers_t *)(stack_top - 1); // Reserve space for saved registers

    print("test\n");
    new_proc->regs->iret_rsp = (uint64_t)stack_top;  // Stack pointer
    new_proc->regs->iret_rip = (uint64_t)entry_point; // Entry point

    new_proc->state = PROCESS_READY;
    new_proc->next = NULL;

    // Add to process list
    if (process_list == NULL) {
        process_list = new_proc;
    } else {
        process_t *temp = process_list;
        while (temp->next) temp = temp->next;
        temp->next = new_proc;
    }

    return new_proc;
}



void scheduler_tick() {
    if (current_process == NULL  || current_process->state != PROCESS_RUNNING) {
        // Start with the first process in the list
        current_process = process_list;
    } else {
        // Move to the next process in the list
        process_t *previous_process = current_process;
        current_process = current_process->next;

        if (current_process == NULL) {
            current_process = process_list; // Wrap around if at the end
        }

        // Perform the context switch
        switch_to_process(previous_process, current_process);
    }
}


void process1() {
    while (true) {
        print("Process 1 running...\n");
        halt_kernel(); // Simulate some waiting
    }
}

void process2() {
    while (true) {
        print("Process 2 running...\n");
        halt_kernel();
    }
}


void init_multitasking() {
    print("Start of multitasking\n");

    create_process(process1);
    create_process(process2);

    // Start the scheduler
    current_process = process_list;

    // Enable interrupts and let the scheduler run
    enable_interrupts();
    print("End of multitasking\n");
}
