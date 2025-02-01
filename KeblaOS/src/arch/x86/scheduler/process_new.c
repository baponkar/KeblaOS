
#include "../lib/string.h"
#include "../driver/vga.h"

#include "../lib/string.h"

#include "../x86_64/idt/idt.h"
#include "../x86_64/pit/pit_timer.h"

#include "../mmu/kheap.h"

#include "process.h"

#definne PROCESS_STACK_SIZE 16*1024 // 16 KB
#define THREAD_STACK_SIZE 4*1024 // 4 KB

process_t *current_process = NULL;
thread_t *current_thread = NULL;

size_t pid_next = 1;

thread_t *create_thread(process_t *process, void (*entry_point)(void), char *name, void* arg) {
    thread_t *thread = (thread_t *)kheap_alloc(sizeof(thread_t));
    thread->pid = next_pid++;
    strncpy(thread->name, name, NAME_MAX_LEN);
    thread->stack = (uint64_t *)kheap_alloc(STACK_SIZE);    // Allocate a stack for the thread
    thread->state = READY;
    thread->next = NULL;

    // Initialize process registers
    memset(&thread->registers, 0, sizeof(registers_t)); // Clear the registers
    thread->registers.iret_rip = (uint64_t)entry_point;    // Set the instruction pointer to the entry point
    thread->registers.iret_cs = 0x08; // Kernel code segment
    thread->registers.iret_ss = 0x10; // Kernel stack segment
    thread->registers.iret_rsp = (uint64_t)(process->stack + (STACK_SIZE / sizeof(uint64_t)) - 1); // Set the stack pointer to the end of the stack
    thread->registers.iret_rflags = 0x202; // Interrupt flag enabled
    thread->registers.rdi = (uint64_t)arg;  // Pass the argument to the thread
    thread->registers.rbp = 0;  // Clear the base pointer

    memset(process->stack, 0, STACK_SIZE); // Clear the stack

    // Add the thread to the process's thread list
    if (!process->threads) {    // If the process has no threads
        process->threads = thread;
    } else {    // If the process has threads
        thread_t *temp = process->threads;
        while (temp->next) temp = temp->next; // Find the last thread in the list
        temp->next = thread;    // Add the new thread to the end of the list
    }

    return thread;
}

void terminate_thread(process_t *process, thread_t *thread) {
    if (!process || !thread) return;

    thread_t *temp = process->threads;
    thread_t *prev = NULL;

    while (temp) {
        if (temp == thread) {
            if (prev) {
                prev->next = temp->next;
            } else {
                process->threads = temp->next;
            }

            kheap_free((void *)temp->stack, STACK_SIZE);    // Free the thread's stack
            kheap_free((void *)temp, sizeof(thread_t));    // Free the thread
            return;
        }

        prev = temp;
        temp = temp->next;
    }

    print("Successfully terminated process with PID: ");
    print_dec(process->pid);
    print("\n");
}


void context_switch(registers_t *regs) {
    static thread_t *current_thread = NULL;

    if (current_thread) {
        // Save the current thread's state
        current_thread->registers = *regs; // Save the current thread's registers with updated values
        current_thread->state = READY;
    }

    // Find the next thread to run
    process_t *process = process_list;
    while (process) {
        thread_t *thread = current_thread ? current_thread->next : process->threads;
        while (thread) {
            if (thread->state == READY) {
                current_thread = thread;
                break;
            }
            thread = thread->next;
        }
        if (current_thread) break;
        process = process->next;
    }

    if (current_thread) {
        current_thread->state = RUNNING;
        *regs = current_thread->registers;
    }
}
