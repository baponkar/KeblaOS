
/*
The process control block (PCB) is a data structure in the operating 
system kernel containing the information needed to manage a particular process. 
The PCB is "the manifestation of a process in an operating system". 
The PCB is also known as the Task Control Block.

References :
https://wiki.osdev.org/Brendan%27s_Multi-tasking_Tutorial
https://wiki.osdev.org/Context_Switching
https://github.com/dreamportdev/Osdev-Notes/blob/master/05_Scheduling/README.md
https://web.archive.org/web/20160326122214/http://jamesmolloy.co.uk/tutorial_html/9.-Multitasking.html
https://wiki.osdev.org/Brendan%27s_Multi-tasking_Tutorial
*/


#include "../lib/string.h"
#include "../lib/stdio.h"
#include "../lib/string.h"
#include "../memory/kheap.h"
#include "../memory/vmm.h"
#include "../util/util.h"
#include "thread.h"
#include "types.h"
#include "process.h"


extern void restore_cpu_state(registers_t* registers);

size_t next_free_pid = 0;           // Available free process id
process_t *current_process;         // Current running process
process_t *processes_list = NULL;   // List of all processes

// Adding Process into process_list
void add_process(process_t* proc) {
    if (!proc) return;

    proc->next = processes_list; // add the head of process list
    processes_list = proc;       // change into process list with new process
}


// Creating a new process with a null thread
process_t* create_process(const char* name) {
    
    process_t* proc = (process_t*) kheap_alloc(sizeof(process_t), ALLOCATE_CODE); // Allocate memory for the process
    if (!proc){
        printf("Process Memory allocation Failed!\n");
        return NULL; // Return NULL if memory allocation fails
    } 
    
    // Assign the next available PID
    proc->pid = next_free_pid++;    // pick and assigne process id
    proc->status = READY;           // Changed the status into READY
    strncpy(proc->name, name, NAME_MAX_LEN - 1); // Copy name
    proc->name[NAME_MAX_LEN - 1] = '\0'; // Ensure null-termination
    proc->next = NULL;      // The next process of this is Null
    proc->threads = NULL;   // Currents threads are null
    proc->current_thread = proc->threads;
    proc->cpu_time = 0;

    // Add the process to the global process list
    add_process(proc);

    printf("Created Process: %s (PID: %d)\n", proc->name, proc->pid);

    return proc;
}


// Removing process from process_list
void remove_process(process_t* proc) {
    if (!proc) return;

    // Remove the process from the global process list
    process_t* prev = NULL;
    process_t* current = processes_list;

    while (current) { // If current is not null
        if (current == proc) { // If given proc is equal to current
            if (prev) {     // If the previous process of current is not null
                prev->next = current->next; // Removing current process from linked list
            } else {        // If the current is head of process_list
                processes_list = current->next; // Removing First Process from linked list
            }
            break;
        }
        prev = current;          // Store the previous process before shifting next process
        current = current->next; // going from head to tail
    }
}


// Delete process from memory
void delete_process(process_t* proc) {
    printf("Start Deleting Process PID: %d\n", proc->pid);
    if (!proc) return;

    // Remove the process from the global process list
    remove_process(proc);

    // Free the process and its threads
    while (true)
    {
        thread_t* thread = proc->threads; // First Thread of the thread linked list
        if (!thread) break; // No more threads to delete
        proc->threads = thread->next;
        delete_thread(thread);
    }

    printf("Deleting Process: %s (PID: %d)\n", proc->name, proc->pid);
    kheap_free(proc, sizeof(process_t));
}


registers_t* schedule(registers_t* registers) {
    void *prev_registers = (void *)(uintptr_t)registers;
    if (!current_process || !current_process->current_thread) return NULL;

    __asm__ volatile("cli");   // Clearing Interrupt
    // printf("Starting mem copy\n");
    memcpy((void *)&current_process->current_thread->registers, (void *)prev_registers, sizeof(registers_t)); // Save current thread state
    // printf("mem copy Ended\n");
    // __asm__ volatile("sti");   // Starting Interrupt
    

    current_process->current_thread->status = READY;

    thread_t* current_thread = current_process->current_thread;
    thread_t* next_thread = current_thread->next;

    if(!next_thread) next_thread = current_process->threads;

    // Round-robin search for next READY thread
    // while (next_thread != current_thread) {
    //     if (next_thread->status == READY){
    //         break;
    //     }
    //     next_thread = next_thread->next;
    // }
  
    // printf("Next:%s\n", next_thread->name);

    // Update current thread and set status
    next_thread->status = RUNNING;
    current_process->current_thread = next_thread;

    // __asm__ volatile("sti");   // Starting Interrupt
    return (registers_t *)(uintptr_t) &current_process->current_thread->registers;
}



void print_process_list() {
    process_t* current = processes_list;
    printf("Current Running Process:\n");
    while (current) {
        printf("PID: %d, Name: %s, Status: %d\n", current->pid, current->name, current->status);
        current = current->next;
    }
}


process_t* get_process_by_pid(size_t pid) {
    process_t* current = processes_list;
    while (current) {
        if (current->pid == pid) {
            return current;
        }
        current = current->next;
    }
    return NULL; // Not found
}

process_t * get_current_process() {
    return processes_list; // Return the current process
}




