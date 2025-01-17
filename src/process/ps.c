
#include "ps.h"

static process_t *process_list = NULL; // Linked list of processes
static process_t *current_process = NULL; // Currently running process
static uint64_t next_pid = 1; // Next process ID

process_t *create_process(void (*entry_point)()) {
    // Allocate memory for the PCB
    process_t *new_process = (process_t *)kheap_alloc(sizeof(process_t));
    if (!new_process) return NULL;

    // Allocate a stack for the process
    void *stack = kheap_alloc(0x4000); // 16 KiB stack

    if (!stack) {
        kheap_free(new_process, sizeof(process_t));
        return NULL;
    }

    // Allocate a new page table for the process (dummy allocation in this example)
    uint64_t *page_table = (uint64_t *)kheap_alloc(0x1000);
    if (!page_table) {
        kheap_free(stack, 0x4000);
        kheap_free(new_process, sizeof(process_t));
        return NULL;
    }

    // Initialize the PCB
    new_process->pid = next_pid++;
    new_process->page_table = page_table;
    new_process->stack = (void *)((uint64_t)stack + 0x4000); // Stack grows downward
    new_process->state = PROCESS_READY;
    new_process->next = NULL;

    // Add the process to the scheduler's list
    if (!process_list) {
        process_list = new_process;
    } else {
        process_t *temp = process_list;
        while (temp->next) {
            temp = temp->next;
        }
        temp->next = new_process;
    }

    return new_process;
}


void terminate_process(process_t *process) {
    if (!process) return;

    // Free process resources
    kheap_free(process->stack, 0x4000);
    kheap_free(process->page_table, 0x1000);
    kheap_free(process, sizeof(process_t));

    // Update the process list
    process_t **indirect = &process_list;
    while (*indirect && (*indirect)->pid != process->pid) {
        indirect = &(*indirect)->next;
    }
    if (*indirect) {
        *indirect = (*indirect)->next;
    }
}

void load_page_table(uint64_t *page_table) {
    // Load the PML4 physical address into the CR3 register
    asm volatile("mov %0, %%cr3" :: "r"(page_table) : "memory");
}

void set_stack_pointer(void *stack) {
    // Load the stack pointer into the RSP register
    asm volatile("mov %0, %%rsp" :: "r"(stack) : "memory");
}


void switch_to_process(process_t *process) {
    if (!process) return;

    // Update page table (simplified)
    load_page_table(process->page_table);

    // Set stack pointer to the new process's stack
    set_stack_pointer(process->stack);

    // Update the current process
    current_process = process;
}

void scheduler_tick() {
    if (!current_process) {
        current_process = process_list;
    }

    process_t *start = current_process;
    do {
        current_process = current_process->next ? current_process->next : process_list;
        if (current_process->state == PROCESS_READY) {
            switch_to_process(current_process);
            return;
        }
    } while (current_process != start);

    // No ready process found
    print("No ready process to run.\n");
}

void test_process1() {
    while (1) {
        print("Process 1 running.\n");
    }
}

void test_process2() {
    while (1) {
        print("Process 2 running.\n");
    }
}

void init_scheduler() {
    create_process(test_process1);
    create_process(test_process2);

    print("Scheduler initialized with 2 processes.\n");
}
