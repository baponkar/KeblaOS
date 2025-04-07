
#include "../lib/string.h"
#include "../lib/stdio.h"
#include "../memory/kheap.h"
#include "process.h"
#include "types.h"
#include "../x86_64/timer/apic_timer.h"

#include "thread.h"


#define THREAD_STACK_SIZE 0x4000 // 16 KB

#define KERNEL_CS  0x08
#define KERNEL_SS  0x10

// Update these to include RPL=3 for user mode
#define USER_CS    0x18 | 3
#define USER_SS    0x20 | 3
  
#define FLAGS      0x202



size_t next_free_tid = 0;



void add_thread(thread_t* thread) {
    if (!thread || !thread->parent) return;

    process_t* parent = thread->parent;

    if (parent->threads == NULL) {
        // First thread in process
        parent->threads = thread;
        parent->current_thread = thread;
    } else {
        // Detect last thread
        thread_t* last = parent->threads;
        while (last) {
            if(!last->next){
                break;
            }
            last = last->next;
        }
        last->next = thread;
    }
    parent->current_thread = thread;
}



thread_t* create_thread(process_t* parent, const char* name, void (*function)(void*), void* arg) {

    thread_t* thread = (thread_t*) kheap_alloc(sizeof(thread_t)); // Allocate memory for the thread

    if (!thread) return NULL;
    memset((void*)thread, 0, sizeof(thread_t)); // Initialize the thread to 0

    // Assign the next available TID
    thread->tid = next_free_tid++;
    thread->status = READY;
    strncpy(thread->name, name, THREAD_NAME_MAX_LEN - 1);
    thread->name[THREAD_NAME_MAX_LEN - 1] = '\0'; // Ensure null-termination

    thread->parent = parent;
    thread->next = 0;
    thread->cpu_time = 0;

    // Allocate a stack for the thread
    void* stack = kheap_alloc(THREAD_STACK_SIZE);

    if (!stack) { // If stack allocation fails, free the thread
        kheap_free((void*)thread, sizeof(thread_t));
        next_free_tid--; // Revert the TID counter if stack allocation fails
        return NULL;
    }

    // Set up the thread's stack and registers to execute the provided function
    thread->registers.iret_ss = KERNEL_SS;
    thread->registers.iret_rsp = ((uint64_t)stack + THREAD_STACK_SIZE) & ~0xF; // Align stack , Stack grows downward
    thread->registers.iret_rflags = FLAGS;              // Enable interrupts (default flags)
    thread->registers.iret_cs = KERNEL_CS;             // Assume a default code segment selector
    thread->registers.iret_rip = (uint64_t) function;  // Instruction pointer = function address
    thread->registers.rdi = (uint64_t) arg;            // First argument (rdi) = arg
    thread->registers.rbp = 0;                         // Base pointer = 0

    add_thread(thread);                                 // Add the thread to the parent process's thread list

    // printf("Created Thread: %s (TID: %d) at %x | rip : %x | rsp : %x\n", 
    //     thread->name, 
    //     thread->tid, 
    //     (uint64_t)thread, 
    //     thread->registers.iret_rip, 
    //     thread->registers.iret_rsp);

    return thread;
}



// Remove the thread from the process's thread list
void remove_thread(thread_t* thread) {
    if (!thread) return;

    process_t* parent = thread->parent;
    if (!parent) return;

    while (true) {
        if (parent->threads == thread) {
            parent->threads = thread->next; // Remove from head of list
            break;
        } else {
            thread_t* current = parent->threads;
            while (current && current->next) {
                if (current->next == thread) {
                    current->next = thread->next; // Remove from middle or end of list
                    break;
                }
                current = current->next;
            }
        }
    }
    thread->next = NULL; // Clear the next pointer of the removed thread
    thread->parent = NULL; // Clear the parent pointer   
}



void delete_thread(thread_t* thread) {
    if (!thread) return;
    printf("Deleting Thread: %s (TID: %d)\n", thread->name, thread->tid);
    remove_thread(thread); // Remove the thread from the process's thread list

    // Free the thread's stack and registers
    kheap_free((void*)(thread->registers.iret_rsp - THREAD_STACK_SIZE), THREAD_STACK_SIZE ); // Free the stack
    kheap_free(thread, sizeof(thread_t)); // Free the thread
    printf("Thread Deleted: %s (TID: %d)\n", thread->name, thread->tid);
}

