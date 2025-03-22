
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
#include "../util/util.h"
#include "thread.h"
#include "types.h"
#include "process.h"


extern void restore_cpu_state(registers_t* registers);

size_t next_free_pid = 0;           // Available free process id
process_t *current_process;         // Current running process
process_t *processes_list = NULL;   // List of all processes


void add_process(process_t* proc) {
    if (!proc) return;

    proc->next = processes_list; // add the head of process list
    processes_list = proc;       // change into process list with new process
}


// Creating a new process with a null thread
process_t* create_process(const char* name) {
    
    process_t* proc = (process_t*) kheap_alloc(sizeof(process_t)); // Allocate memory for the process
    if (!proc){
        printf("Process Memory allocation Failed!\n");
        return NULL; // Return NULL if memory allocation fails
    } 
    
    // Assign the next available PID
    proc->pid = next_free_pid++;    // pick and assigne process id
    proc->status = READY;           // Changed the status into READY
    strncpy(proc->name, name, NAME_MAX_LEN - 1); // Copy name
    proc->name[NAME_MAX_LEN - 1] = '\0'; // Ensure null-termination
    proc->next = NULL;
    proc->threads = NULL; // Currents threads are null
    proc->current_thread = proc->threads;
    proc->cpu_time = 0;

    // Add the process to the global process list
    add_process(proc);

    printf("Created Process: %s (PID: %d)\n", proc->name, proc->pid);

    return proc;
}


void remove_process(process_t* proc) {
    if (!proc) return;

    // Remove the process from the global process list
    process_t* prev = NULL;
    process_t* current = processes_list;

    while (current) {
        if (current == proc) {
            if (prev) {
                prev->next = current->next;
            } else {
                processes_list = current->next;
            }
            break;
        }
        prev = current;
        current = current->next;
    }

}


void delete_process(process_t* proc) {
    if (!proc) return;

    // Remove the process from the global process list
    remove_process(proc);

    // Free the process and its threads
    while (true)
    {
        thread_t* thread = proc->threads;
        if (!thread) break;

        proc->threads = thread->next;
        delete_thread(thread);
    }

    kheap_free(proc, sizeof(process_t));
}


registers_t* schedule(registers_t* registers) {
    if (!current_process || !current_process->current_thread) return NULL;

    // Save current thread state
    memcpy(&current_process->current_thread->registers, registers, sizeof(registers_t));
    current_process->current_thread->status = READY;

    thread_t* current_thread = current_process->current_thread;
    thread_t* next_thread = current_thread->next;

    // Round-robin search for next READY thread
    while (next_thread != current_thread) {
        if (next_thread->status == READY) {
            break;
        }
        next_thread = next_thread->next;
    }

    // Update current thread and set status
    next_thread->status = RUNNING;
    current_process->current_thread = next_thread;
    
    return &next_thread->registers;
}







