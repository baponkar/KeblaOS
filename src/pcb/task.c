
#include "../lib/string.h"  // for strncpy
#include "../lib/stdio.h"   // for printf
#include "../mmu/kheap.h"   // for kheap_alloc and kheap_free
#include "../x86_64/interrupt/interrupt.h"  // for enable_interrupt and disable_interrupt
#include "../x86_64/timer/apic_timer.h"     // for apic_delay

#include "task.h"

#define STACK_SIZE 0x4000   // 16 KB
#define KERNEL_SS  0x10     // Kernel Data Segment from GDT
#define KERNEL_CS  0x08     // Kernel Code Segment from GDT

size_t next_free_tid = 0;

task_t *current_task;
task_t *tasks_list = NULL;

task_t* create_task(char* name, void(*function)(void*), void* arg) {
    // Allocate memory for the task
    task_t* task = (task_t*) kheap_alloc(sizeof(task_t));

    // Return NULL if memory allocation fails
    if (!task) return NULL;

    // Initialize the task
    strncpy(task->name, name, NAME_MAX_LEN);
    task->tid = next_free_tid++; // Increment the task ID
    task->status = READY;
    // Allocate memory for the registers
    task->registers = (registers_t*) kheap_alloc(sizeof(registers_t));
    if (!task->registers) { // Return NULL if memory allocation fails
        kheap_free(task, sizeof(task_t)); // Free the task memory
        return NULL;
    }
    
    // Initialize the registers
    task->registers->iret_ss = KERNEL_SS;   // Set the stack segment
    task->registers->iret_rsp = ((uint64_t) kheap_alloc(STACK_SIZE) + STACK_SIZE) & ~0xF; // Make stack pointer on top and 16 byte aligned
    task->registers->iret_rflags = 0x202; // Set the RFLAGS register
    task->registers->iret_cs = KERNEL_CS; // Set the code segment
    task->registers->iret_rip = (uint64_t)function; // Set the instruction pointer
    task->registers->rdi = (uint64_t)arg;   // Set the first argument
    task->registers->rbp = 0;   // Set the base pointer
    
    if(!tasks_list) { // If the tasks list is empty
        tasks_list = task; // Set the task as the first task
    }else{
        tasks_list->next = task; // Add the task to the end of the list
    }

    // Print the task details
    printf("Created Task: %s (TID: %d)  | rsp : %x | rip : %x | rdi : %x\n", 
        task->name, task->tid, 
        task->registers->iret_rsp, 
        task->registers->iret_rip, 
        task->registers->rdi);

    return task;
}


void delete_task(task_t *prev_task, task_t *current_task){
    if (!current_task) return;
    
    // Remove the task from the list
    if (prev_task) {
        prev_task->next = current_task->next;
    } else {
        tasks_list = current_task->next;
    }
    
    printf("Deleting Task: %s (TID: %d)\n", current_task->name, current_task->tid);
    kheap_free(current_task->registers, sizeof(registers_t));
    kheap_free(current_task, sizeof(task_t));
}


extern void switch_to_task(task_t *next_task); // defines in task_switch.asm


void schedule(registers_t *regs) {

    if (current_task->status == RUNNING){
        current_task->status = READY;   // Set the current task status to READY
        current_task->registers = regs; // Save the current task's registers
    }
        
    current_task = current_task->next;  // Move to the next task

    switch_to_task(current_task); // Switch to the next task
}

void task1(void* arg) {
    while (1) {
        printf("Task 1 is Running\n");
        apic_delay(1000);  // Delay for 1000ms (1 second)
    }
}


void task2(void* arg) {
    while (1) {
        printf("Task 2 is Running\n");
        apic_delay(1000);  // Delay for 1000ms (1 second)
    }
}



void init_tasking() {
    disable_interrupts();
    // Create the initial started task
    task_t *init_task = create_task("Init Task", NULL, NULL); // Create the initial task and set it as the current task

    // Create the first task
    task_t *first_task = create_task("Task1", task1, NULL);
    task_t *second_task = create_task("Task2", task2, NULL);


    second_task->next = init_task;  // link last task to the init task
    current_task = init_task;       // Set the current task to the init task

    enable_interrupts();
}



