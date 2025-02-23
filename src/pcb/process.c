
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

#include "process.h"



#define STACK_SIZE 0x4000 // 16 KB
#define KERNEL_SS  0x10
#define KERNEL_CS  0x08


size_t next_free_pid = 0;
process_t *current_process;
process_t *processes_list = NULL;



static inline uint64_t read_rip() {
    uint64_t rip;
    __asm__ volatile (
        "lea (%%rip), %0" 
        : "=r"(rip)
    );
    return rip;
}

static inline uint64_t read_rsp() {
    uint64_t rsp;
    __asm__ volatile ("movq %%rsp, %0" : "=r"(rsp));
    return rsp;
}

static inline uint64_t read_rflags() {
    uint64_t flags;
    __asm__ volatile ("pushfq; pop %0" : "=r"(flags));
    return flags;
}



process_t *create_init_process(char* name, void(*function)(void*), void* arg) {
    process_t *init_process = (process_t *) kheap_alloc(sizeof(process_t));
    if (!init_process) return NULL;

    strncpy(init_process->name, name, NAME_MAX_LEN);
    init_process->pid = next_free_pid++;
    init_process->status = READY;
    init_process->registers = (registers_t *) kheap_alloc(sizeof(registers_t));
    if (!init_process->registers) {
        kheap_free(init_process, sizeof(process_t));
        return NULL;
    }

    init_process->registers->iret_ss = KERNEL_SS;
    init_process->registers->iret_rsp = read_rsp(); // Set stack pointer to current stack top
    init_process->registers->iret_rflags = 0x202;
    init_process->registers->iret_cs = KERNEL_CS;
    init_process->registers->iret_rip = (uint64_t) function; // Set instruction pointer to current instruction
    init_process->registers->rdi = (uint64_t) arg;      // No arguments
    init_process->registers->rbp = 0;
    
    
    processes_list = init_process;

    printf("Created Process: %s (PID: %d) | rsp : %x | rip : %x | rdi : %x\n", 
        init_process->name, init_process->pid, 
        init_process->registers->iret_rsp, 
        init_process->registers->iret_rip, 
        init_process->registers->rdi);

    return init_process;
}



process_t* create_process(char* name, void(*function)(void*), void* arg) {
    process_t* process = (process_t*) kheap_alloc(sizeof(process_t));
    if (!process) return NULL;

    strncpy(process->name, name, NAME_MAX_LEN);
    process->pid = next_free_pid++;
    process->status = READY;
    process->registers = (registers_t*) kheap_alloc(sizeof(registers_t));
    if (!process->registers) {
        kheap_free(process, sizeof(process_t));
        return NULL;
    }

    // Allocate stack and align it properly
    void* stack_base = kheap_alloc(STACK_SIZE);
    if (!stack_base) {
        kheap_free(process->registers, sizeof(registers_t));
        kheap_free(process, sizeof(process_t));
        return NULL;
    }

    process->registers->iret_ss = KERNEL_SS;
    process->registers->iret_rsp = ((uint64_t) stack_base + STACK_SIZE) & ~0xF;  // Ensure 16-byte alignment
    process->registers->iret_rflags = 0x202;
    process->registers->iret_cs = KERNEL_CS;
    process->registers->iret_rip = (uint64_t) function;
    process->registers->rdi = (uint64_t) arg;
    process->registers->rbp = 0;
    process->next = NULL;  // Ensure it's properly terminated

    // Append process to the end of the linked list
    if (!processes_list) {
        processes_list = process;
    } else {
        process_t* temp = processes_list;
        while (temp->next) {
            temp = temp->next;
        }
        temp->next = process;
    }

    printf("Created Process: %s (PID: %d) | rsp: %x | rip: %x | rdi: %x\n", 
        process->name, process->pid, 
        process->registers->iret_rsp, 
        process->registers->iret_rip, 
        process->registers->rdi);

    return process;
}



void delete_process(process_t *prev_process, process_t *current_process) {
    if (!current_process) return;
    
    if (prev_process) {
        prev_process->next = current_process->next;
    } else {
        processes_list = current_process->next;
    }
    
    printf("Deleting Process: %s (PID: %d)\n", current_process->name, current_process->pid);
    kheap_free(current_process->registers, sizeof(registers_t));
    kheap_free(current_process, sizeof(process_t));
}




registers_t* schedule(registers_t *regs) {
    if(current_process != NULL) {
        current_process->registers = regs;
        current_process->status = READY;
    }

    while(true) {
        process_t *prev_process = current_process;
        process_t *next_process = current_process->next;
        if(next_process != NULL) {
            current_process = next_process;
        } else {
            current_process = processes_list;
        }

        if(current_process != NULL && current_process->status == DEAD) {
            delete_process(prev_process, current_process);
        }else if(current_process->status == READY) {
            // printf("Switching to %s (PID: %d) | rsp : %x | cs : %x | rflags : %x | rip : %x | ss : %x\n",
            //     current_process->name,
            //     current_process->pid,
            //     current_process->registers->iret_rsp,
            //     current_process->registers->iret_cs,
            //     current_process->registers->iret_rflags,
            //     current_process->registers->iret_rip,
            //     current_process->registers->iret_ss);
            current_process->status = RUNNING;
            return current_process->registers;
            break;
        }
    }
    return current_process->registers;
}

void process0(void *arg) {
    while(1) {
        printf("Process init is Running\n");
        apic_delay(100);  // Delay for 1000ms (1 second)
    }
}


void process1(void* arg) {
    while (1) {
        printf("Process 1 is Running\n");
        apic_delay(100);  // Delay for 1000ms (1 second)
    }
}


void process2(void* arg) {
    while(1) {
        printf("Process 2 is Running\n");
        apic_delay(100);
    }
}


void process3(void* arg) {
    while (1) {
        printf("Process 3 is Running\n");
        apic_delay(100);
    }
}




void init_processes() {
    create_init_process("Init Process", process0, NULL);
    create_process("Process1", process1, NULL);
    create_process("Process2", process2, NULL);
    create_process("Process3", process3, NULL);

    current_process = processes_list;  // Set the first process
}





