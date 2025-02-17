
#include "../driver/vga/vga_term.h"  // for print, print_dec, print_hex functions
#include "../mmu/kheap.h"   // for kheap_alloc, kfree functions
#include "../x86_64/timer/apic_timer.h" // for apic_delay function
#include "../x86_64/interrupt/pic.h" // for enable_interrupts and disable_interrupts functions
#include "../lib/string.h"  // for strncpy function

#include "process.h"

#define STACK_SIZE (16 * 1024) // 16 KB stack size


uint64_t next_pid = 1;
process_t *process_list = NULL;


thread_t *create_thread(process_t *process, void (*entry_point)(void), char *name, void *arg) {
    if (!process) return NULL;

    thread_t *thread = (thread_t *)kheap_alloc(sizeof(thread_t));
    thread->tid = next_pid++; // Use `next_pid` for unique thread IDs
    strncpy(thread->name, name, NAME_MAX_LEN);
    thread->stack = (uint64_t *)kheap_alloc(STACK_SIZE);
    thread->state = THREAD_READY;
    thread->next = NULL;

    // Initialize thread registers
    memset(&thread->registers, 0, sizeof(registers_t));
    thread->registers.iret_rip = (uint64_t)entry_point; // Instruction pointer
    thread->registers.iret_cs = 0x08;                  // Kernel code segment
    thread->registers.iret_ss = 0x10;                  // Kernel stack segment
    thread->registers.iret_rsp = (uint64_t)(thread->stack + (STACK_SIZE / sizeof(uint64_t)) - 1); // Top of the stack
    thread->registers.iret_rflags = 0x202;             // Enable interrupts
    thread->registers.rdi = (uint64_t)arg;             // First argument

    // Add thread to the process's thread list
    if (!process->threads) {
        process->threads = thread;
    } else {
        thread_t *temp = process->threads;
        while (temp->next) temp = temp->next;
        temp->next = thread;
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

            kheap_free((void *)temp->stack, STACK_SIZE);
            kheap_free((void *)temp, sizeof(thread_t));
            return;
        }

        prev = temp;
        temp = temp->next;
    }
}


void context_switch(registers_t *regs) {
    static thread_t *current_thread = NULL;

    if (current_thread) {
        // Save the current thread's state
        current_thread->registers = *regs;
        current_thread->state = THREAD_READY;
    }

    // Find the next thread to run
    process_t *process = process_list;
    while (process) {
        thread_t *thread = current_thread ? current_thread->next : process->threads;
        while (thread) {
            if (thread->state == THREAD_READY) {
                current_thread = thread;
                break;
            }
            thread = thread->next;
        }
        if (current_thread) break;
        process = process->next;
    }

    if (current_thread) {
        current_thread->state = THREAD_RUNNING;
        *regs = current_thread->registers;
    }
}


process_t *create_process(char *name) {
    process_t *process = (process_t *)kheap_alloc(sizeof(process_t));
    process->pid = next_pid++;
    process->threads = NULL;
    process->next = NULL;

    // Add the process to the process list
    process->next = process_list;
    process_list = process;

    return process;
}


void terminate_process(process_t *process) {
    if (!process) return;

    // Terminate all threads
    thread_t *thread = process->threads;
    while (thread) {
        thread_t *next_thread = thread->next;
        terminate_thread(process, thread);
        thread = next_thread;
    }

    // Remove the process from the process list
    process_t *temp = process_list;
    process_t *prev = NULL;

    while (temp) {
        if (temp == process) {
            if (prev) {
                prev->next = temp->next;
            } else {
                process_list = temp->next;
            }

            kheap_free((void *)process, sizeof(process_t));
            return;
        }

        prev = temp;
        temp = temp->next;
    }
}


void schedule(registers_t *regs) {
    disable_interrupts();
    context_switch(regs);
    enable_interrupts();
}


void thread_1() {
    while (1) {
        print("Thread 1 is running\n");
        apic_delay(10000);
    }
}

void thread_2() {
    while (1) {
        print("Thread 2 is running\n");
        apic_delay(10000);
    }
}


void init_processes() {
    process_t *process_1 = create_process("Process 1");
    create_thread(process_1, thread_1, "Thread 1", NULL);
    create_thread(process_1, thread_2, "Thread 2", NULL);

    // Debug: Print process and thread list
    print_process_list();
}


void print_process_list() {
    process_t *process = process_list;
    while (process) {
        print("Process ID: ");
        print_dec(process->pid);
        print(" | Threads: ");
        thread_t *thread = process->threads;
        while (thread) {
            print("[Thread ID: ");
            print_dec(thread->tid);
            print(", Name: ");
            print(thread->name);
            print("] ");
            thread = thread->next;
        }
        print("\n");
        process = process->next;
    }
}





