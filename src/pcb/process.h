
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "../util/util.h"

#define NAME_MAX_LEN 64

typedef enum {
    READY,
    RUNNING,
    DEAD
} status1_t;


typedef struct process {
    size_t pid;
    status1_t status;
    registers_t* registers;
    char name[NAME_MAX_LEN];
    struct process* next;
} process_t;

extern process_t *current_process;
extern process_t *processes_list;
extern void restore_cpu_state(registers_t *regs);

process_t* create_process(char* name, void(*function)(void*), void* arg);
void delete_process(process_t *prev_process, process_t *current_process);
registers_t* schedule(registers_t *regs);
void init_processes();
