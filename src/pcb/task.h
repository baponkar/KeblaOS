#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "../util/util.h"   // registers_t



#define NAME_MAX_LEN 64


typedef enum {
    READY,
    RUNNING,
    DEAD
} status_t;


struct task{
    size_t tid;
    status_t status;
    void *stack_top;
    registers_t* registers;
    char name[NAME_MAX_LEN];
    struct task* next;
};
typedef struct task task_t;

extern task_t *current_task;
extern task_t *tasks_list;

task_t* create_task(char* name, void(*function)(void*), void* arg); // Create a new task
void delete_task(task_t *prev_task, task_t *current_task); // Delete a task
extern void switch_to_task(task_t *next_task);  // Switch to the next task
void schedule(registers_t *regs);            // Schedule the next task
void init_tasking(); // Initialize the tasking system