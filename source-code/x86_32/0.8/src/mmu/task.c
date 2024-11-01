#include "task.h"

process_t *create_process(void (*entry_point)()) {
    process_t *new_process = (process_t *) malloc(sizeof(process_t));  // Allocate PCB
    new_process->pid = next_pid++;  // Assign a unique process ID
    new_process->state = READY;

    // Set up the process's page directory
    new_process->page_dir = clone_page_directory(kernel_directory);  // Copy kernel page directory

    // Set up the process's stack and instruction pointer
    new_process->esp = allocate_stack();  // Allocate stack for the process
    new_process->eip = (uint32_t)entry_point;  // Set the entry point to the process function

    return new_process;
}


void context_switch(process_t *next_process) {
    // Save the current process state
    asm volatile ("mov %%esp, %0" : "=r"(current_process->esp));  // Save stack pointer
    asm volatile ("mov %%ebp, %0" : "=r"(current_process->ebp));  // Save base pointer
    current_process->eip = get_eip();  // Save the instruction pointer

    // Load the next process's state
    current_process = next_process;
    asm volatile ("mov %0, %%esp" :: "r"(next_process->esp));  // Restore stack pointer
    asm volatile ("mov %0, %%ebp" :: "r"(next_process->ebp));  // Restore base pointer
    set_eip(next_process->eip);  // Restore instruction pointer

    switch_page_directory(next_process->page_dir);  // Switch page directory (paging)
}


void switch_page_directory(page_directory_t *dir) {
    current_page_directory = dir;
    asm volatile("mov %0, %%cr3" :: "r"(dir->physical_addr));  // Load page directory into CR3
}


void terminate_process(process_t *process) {
    free_page_directory(process->page_dir);  // Free the page directory
    free(process);  // Free the PCB
}

void syscall_exit(process_t *current_process) {
    terminate_process(current_process);
    scheduler();  // Switch to the next process
}





