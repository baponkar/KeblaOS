#pragma once

#include "../stdlib/stdint.h"
#include "../stdlib/stdlib.h"
#include "../util/util.h"
#include "../stdlib/stdlib.h"
#include "paging.h"
#include "scheduler.h"


typedef struct {
    uint32_t pid;               // Process ID
    uint32_t state;             // Process state (e.g., running, waiting, terminated)
    uint32_t esp;               // Stack pointer (saved when switching processes)
    uint32_t ebp;               // Base pointer
    uint32_t eip;               // Instruction pointer (points to the next instruction)
    page_directory_t *page_dir; // Pointer to the process's page directory
    uint32_t kernel_stack;      // Kernel stack for handling system calls
} process_t;


process_t *create_process(void (*entry_point)());
void context_switch(process_t *next_process);
void switch_page_directory(page_directory_t *dir);
void terminate_process(process_t *process);
void syscall_exit(process_t *current_process);
