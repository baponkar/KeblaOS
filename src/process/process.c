
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
#include "../mmu/kheap.h"
#include "../x86_64/interrupt/interrupt.h"  // for enable_interrupt and disable_interrupt
#include "../x86_64/timer/apic_timer.h"     // for apic_delay
#include "../util/util.h"
#include "thread.h"
#include "types.h"
#include "process.h"


extern void restore_cpu_state(registers_t* registers);
size_t next_free_pid = 0;           // Available free process id
process_t *current_process;         // Current running process
process_t *processes_list = NULL;   // List of all processes


process_t* create_process(const char* name, void (*function)(void*), void* arg) {
    
    process_t* proc = (process_t*) kheap_alloc(sizeof(process_t)); // Allocate memory for the process
    if (!proc) return NULL; // Return NULL if memory allocation fails

    // Assign the next available PID
    proc->pid = next_free_pid++;
    proc->status = READY;
    strncpy(proc->name, name, NAME_MAX_LEN - 1);
    proc->name[NAME_MAX_LEN - 1] = '\0'; // Ensure null-termination
    proc->next = NULL;
    proc->threads = NULL;
    proc->current_thread = NULL;
    proc->cpu_time = 0;

    // Create the main thread for the process
    thread_t* init_thread = create_thread(proc, "init_thread", function, arg);
    if (!init_thread) {
        kheap_free(proc, sizeof(process_t)); // Free the process if thread creation fails
        next_free_pid--; // Revert the PID counter if thread creation fails
        return NULL;
    }

    // Add the main thread to the process's thread list
    proc->current_thread = init_thread;
    proc->threads = init_thread;

    // Add the process to the global process list
    add_process(proc);

    // printf("Created Process: %s (PID: %d)\n", proc->name, proc->pid);

    return proc;
}


void add_process(process_t* proc) {
    if (!proc) return;

    // Add to the head of the global process list
    proc->next = processes_list;
    processes_list = proc;
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
    
    current_process->current_thread->status = READY;

    // Save the current thread's register state
    memcpy(current_process->current_thread->registers, registers, sizeof(registers_t));
    if (memcmp(current_process->current_thread->registers, registers, sizeof(registers_t)) != 0) {
        printf("registers assignment failed!\n");
        return NULL;
    }
    
    thread_t* start_thread = current_process->current_thread;
    thread_t* next_thread = current_process->current_thread->next;
    
    // Look for the next READY thread in a round-robin manner
    while (next_thread && next_thread->status != READY) {
        next_thread = next_thread->next;
    }
    
    // If no READY thread found, start from the first thread
    if (!next_thread) {
        next_thread = current_process->threads;
        while (next_thread && next_thread->status != READY) {
            next_thread = next_thread->next;
        }
    }
    
    // If still no READY thread, keep running the same thread
    if (!next_thread) {
        current_process->current_thread->status = RUNNING;
        return current_process->current_thread->registers;
    }
    
    // Set the next thread as RUNNING
    current_process->current_thread = next_thread;
    current_process->current_thread->status = RUNNING;
    
    return current_process->current_thread->registers;
}



void thread0_func(void *arg) {
    while(true){
        printf("Init Thread is Running...\n");
        apic_delay(1000);  // Delay for 1000ms (1 second)
    }
}


void thread1_func(void* arg) {
    while(true) {
        printf("Thread1 is Running...\n");
        apic_delay(1000);  // Delay for 1000ms (1 second)
    }
}


void thread2_func(void* arg) {
    while(true) {
        printf("Thread2 is Running...\n");
        apic_delay(1000);  // Delay for 1000ms (1 second)
    }
}




void init_processes() {
    // Create the init process
    process_t *process = create_process("process0", (void *) &thread0_func, NULL);
    if (!process) {
        printf("Failed to create init process\n");
        return;
    }

    thread_t* thread0 = process->threads; // Get the main thread of the init process
    if (!thread0) {
        printf("Failed to get init thread\n");
        return;
    }

    // Create a thread with an argument
    thread_t* thread1 = create_thread(process, "Thread1", (void *) &thread1_func, NULL);
    if (!thread1) {
        printf("Failed to create Thread1\n");
        return;
    }

    // Create a thread with an argument
    thread_t* thread2 = create_thread(process, "Thread2", (void *)&thread2_func, NULL);
    if (!thread2) {
        printf("Failed to create Thread2\n");
        return;
    }
   

    // Set the current process
    current_process = process;
    processes_list = process;

}

