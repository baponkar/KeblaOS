
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


size_t next_free_pid = 1;
process_t *current_process;
process_t *processes_list = NULL;




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
    
    process->registers->iret_ss = KERNEL_SS;
    process->registers->iret_rsp = ((uint64_t) kheap_alloc(STACK_SIZE) + STACK_SIZE) & ~0xF; // Make stack pointer on top and 16 byte aligned
    process->registers->iret_rflags = 0x202;
    process->registers->iret_cs = KERNEL_CS;
    process->registers->iret_rip = (uint64_t)function;
    process->registers->rdi = (uint64_t)arg;
    process->registers->rbp = 0;
    
    process->next = processes_list;
    processes_list = process;

    printf("Created Process: %s (PID: %d)  | rsp : %x | rip : %x | rdi : %x\n", 
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



uint64_t read_rflags() {
    uint64_t flags;
    __asm__ volatile ("pushfq; pop %0" : "=r"(flags));
    return flags;
}


registers_t* schedule(registers_t *regs) {

    if (!current_process || !processes_list) 
        return regs;

    printf("Current Process PID : %d | rsp : %x | rip : %x | rdi : %x\n", 
        current_process->pid, 
        current_process->registers->iret_rsp, 
        current_process->registers->iret_rip, 
        current_process->registers->rdi);
    
    // Move to the next process
    current_process = (current_process->next) ? current_process->next : processes_list;
    
    printf("Switching Process PID : %d\n", current_process->pid);

    return current_process->registers;
}



void process1(void* arg) {
    while (1) {
        printf("Process 1 is Running\n");
        apic_delay(1000);  // Delay for 1000ms (1 second)
    }
}


void process2(void* arg) {
    while(1) {
        printf("Process 2 is Running\n");
        apic_delay(1000);
    }
}


void process3(void* arg) {
    while (1) {
        printf("Process 3 is Running\n");
        apic_delay(1000);
    }
}


void init_processes() {
    create_process("Process1", process1, NULL);
    create_process("Process2", process2, NULL);
    create_process("Process3", process3, NULL);

    current_process = processes_list;  // Set the first process
}





