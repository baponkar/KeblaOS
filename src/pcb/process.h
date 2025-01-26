

#pragma once
#include <stdint.h>
#include "../util/util.h"

typedef enum {
    PROCESS_READY,
    PROCESS_RUNNING,
    PROCESS_WAITING,
    PROCESS_TERMINATED
} process_state_t;



typedef struct process {
    uint64_t pid;                   // Process ID
    uint64_t *stack;                // Kernel stack for the process
    registers_t registers;          // Saved CPU state
    process_state_t state;          // Process state
    struct process *next;           // Pointer to the next process in the queue
} process_t;


extern process_t *current_process;


void add_process(process_t *process);
void context_switch(registers_t *regs);
void schedule(registers_t *regs);
process_t *create_process(void (*entry_point)(void));
void terminate_process(process_t *process);

void init_processes();

void print_process_list();
void look_into_process(process_t *process);
void print_stack_contents(process_t *process);
