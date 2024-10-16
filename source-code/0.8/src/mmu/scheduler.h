#pragma once

#include "../stdlib/stdint.h"
#include "task.h"

#define MAX_PROCESSES 256

typedef struct {
    process_t *processes[MAX_PROCESSES];  // List of processes
    int num_processes;                    // Number of processes in the queue
    int current_index;                    // Index of the currently running process
} scheduler_t;

void scheduler_add(process_t *process);
void scheduler_init();
void context_switch(process_t *next_process);
void scheduler();
void timer_interrupt_handler();
void init_timer(uint32_t frequency);
void syscall_yield();
