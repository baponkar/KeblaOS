

#pragma once
#include <stdint.h>
#include "../util/util.h"

#define NAME_MAX_LEN 64


typedef enum {
    PROCESS_READY,
    PROCESS_RUNNING,
    PROCESS_WAITING,
    PROCESS_TERMINATED
} process_status_t;



typedef struct process {
    uint64_t pid;                   // Process ID
    char name[NAME_MAX_LEN];        // Process name
    uint64_t *stack;                // Kernel stack for the process
    registers_t registers;          // Saved CPU state
    process_status_t state;                 // Process state
    struct process *next;           // Pointer to the next process in the queue
} process_t;







extern process_t *current_process;


void add_process(process_t *process);
void context_switch(registers_t *regs);
void schedule(registers_t *regs);
process_t *create_process(void (*entry_point)(void), char *name, void* arg);
void terminate_process(process_t *process);

void init_processes();

void print_process_list();
void look_into_process(process_t *process);
void print_stack_contents(process_t *process);
