

/*
    The Schedular
    https://github.com/dreamportdev/Osdev-Notes/blob/master/05_Scheduling/02_Scheduler.md
*/


#include "ps.h"

#define NAME_MAX_LEN 64

#define KERNEL_SS 0x10  // Kernel Data Segment Selector
#define KERNEL_CS 0x08  // Kernel Code Segment Selector

process_t *processes_list = NULL;
process_t *current_process = NULL;
size_t next_free_pid = 0;

registers_t* schedule(registers_t* regs) {
    if (current_process != NULL) {
        current_process->regs = regs;
        current_process->status = READY;
    }

    while (true) {
        if (current_process == NULL || current_process->next == NULL) {
            current_process = processes_list; // Wrap around to the first process
        } else {
            current_process = current_process->next;
        }

        if (current_process != NULL && current_process->status != DEAD) {
            // We need to delete dead processes
            delete_process(current_process);
        }else {
            current_process->status = RUNNING;
            break;
        }
    }

    return current_process->regs;
}



uint64_t alloc_stack() {
    void* stack = fourth_alloc(16 * 4096); // Ensure this function returns valid memory
    if (!stack) {
        print("Stack allocation failed!\n");
        while(1); // Halt to debug the issue
    }
    return (uint64_t)stack + (16 * 4096); // Return the top of the stack (grows downward)
}


void add_process(process_t *process) {
    if (processes_list == NULL) {
        processes_list = process;
        process->next = NULL;
    } else {
        process->next = processes_list;
        processes_list = process;
    }
}

void delete_process(process_t *process) {
    if (processes_list == NULL || process == NULL) {
        return;
    }

    if (processes_list == process) {
        processes_list = process->next;
    } else {
        process_t *prev = processes_list;
        while (prev->next && prev->next != process) {
            prev = prev->next;
        }
        if (prev->next == process) {
            prev->next = process->next;
        }
    }

    fourth_free((void*)(process->regs->iret_rsp - (16 * 4096))); // Free the stack
    fourth_free((uint64_t *)process); // Free the process structure
}


process_t* create_process(const char* name, void(*function)(void*), void* arg) {
    process_t* process = (process_t*) fourth_alloc(sizeof(process_t));
    if (!process) {
        print("Failed to allocate process structure!\n");
        while (1);
    }

    strncpy(process->name, name, NAME_MAX_LEN - 1);
    process->name[NAME_MAX_LEN - 1] = '\0'; // Ensure null termination
    process->pid = next_free_pid++;
    process->status = READY;

    process->regs->iret_ss = KERNEL_SS;
    process->regs->iret_rsp = alloc_stack();
    process->regs->iret_rflags = 0x202;  // Interrupts enabled
    process->regs->iret_cs = KERNEL_CS;
    process->regs->iret_rip = (uint64_t)function;
    process->regs->rdi = (uint64_t)arg;
    process->regs->rbp = 0;

    add_process(process);

    return process;
}




void test_task1(void *arg){
    while (1) {
        print("Task 1 running!\n");
        for (volatile int i = 0; i < 1000000; i++);  // Delay to simulate task work
    }
}


void test_task2(void *arg){
    while (1) {
        print("Task 2 running!\n");
        for (volatile int i = 0; i < 1000000; i++);  // Delay to simulate task work
    }
}
