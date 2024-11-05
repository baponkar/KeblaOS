#include "process.h"


void schedule() {
    // Save current process's state
    save_current_cpu_state();
    // Select the next process to run
    process_t* next_process = pick_next_process();
    // Load its page table
    load_cr3(next_process->page_table);
    // Restore CPU state
    restore_cpu_state(next_process->cpu_state);
}


void init_tasking() {
    // Set up initial process (e.g., init process or first user-space program)
    struct process_t *init_process = create_process("init_program");
    init_process->page_table = clone_kernel_page_table();
    map_user_space(init_process->page_table);
    load_cr3(init_process->page_table);  // Load first process
    current_process = init_process;      // Set as current process
}

void schedule() {
    save_current_process_state();
    process_t* next_process = pick_next_process();
    load_cr3(next_process->page_table);
    restore_cpu_state(next_process->cpu_state);
}

void timer_interrupt_handler() {
    schedule();
    // Send EOI to timer interrupt controller
}
