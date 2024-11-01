#include "ps.h"



process_t *process_list = NULL; // Head of the process linked list
uint32_t next_pid = 1;           // PID counter

extern uint32_t placement_address;

process_t* create_process() {
    // Allocate memory for the new process
    process_t *new_process = (process_t*) kmalloc(sizeof(process_t));
    if (!new_process) {
        PANIC("Failed to allocate memory for new process.");
    }

   
    // Allocate a new page directory for the process
    new_process->page_directory = (page_directory_t*) kmalloc_a(sizeof(page_directory_t), 1);
    memset(new_process->page_directory, 0, sizeof(page_directory_t));
 
    // Set up the process fields
    new_process->pid = next_pid++;
    new_process->next = NULL;

    // Initialize the process's page directory (identity map, etc.)
    init_process_paging(new_process->page_directory);

    // Add the new process to the process list
    if (!process_list) {
        process_list = new_process; // First process
    } else {
        process_t *current = process_list;
        while (current->next) {
            current = current->next; // Traverse to the end
        }
        current->next = new_process; // Append the new process
    }

    return new_process;
}

void init_process_paging(page_directory_t *dir) {
    // Identity map the process memory space, similar to init_paging
    for (uint32_t i = 0; i < placement_address; i += 0x1000) {
        printf("Page Directory : %d\n", i);
        alloc_frame(get_page(i, 1, dir), 0, 1); // Allocate frames in the new directory
    }
}


void switch_process(process_t *process) {
    if (process) {
        switch_page_directory(process->page_directory); // Switch to the new process's page directory
        // Additional context switch operations can go here
    }
}


void scheduler() {
    static process_t *current_process = NULL;

    // If no process is currently running, start with the first one
    if (!current_process) {
        current_process = process_list;
    } else {
        current_process = current_process->next ? current_process->next : process_list; // Move to the next process
    }

    switch_process(current_process); // Switch to the new process
}


void timer_interrupt_handler(registers_t *regs) {
    scheduler(); // Call the scheduler on each timer interrupt
    // Additional interrupt handling code...
}
