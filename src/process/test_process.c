
/*
This file will test process-thread working
*/

#include "../x86_64/timer/apic_timer.h"     // for apic_delay
#include "../x86_64/timer/pit_timer.h"      // for delay
#include "../x86_64/timer/tsc.h"
#include "../lib/stdio.h"                   // for printf function

#include "test_process.h"


extern volatile uint64_t apic_ticks;

void thread0_func(void *arg) {
    while(true){
        printf("==> Thread0 is Running... [Ticks: %d] \n", apic_ticks);
        apic_delay(10); // delay 10 milli seconds
    }
}


void thread1_func(void* arg) {
    while(true) {
        printf("==> Thread1 is Running... [Ticks: %d] \n", apic_ticks);
        apic_delay(10); // delay 10 milli seconds
    }
}


void thread2_func(void* arg) {
    while(true) {
        printf("==> Thread2 is Running... [Ticks: %d] \n", apic_ticks);
        apic_delay(10); // delay 10 milli seconds
    }
}




void print_all_threads_name(process_t *p){
    thread_t * t = p->threads;
    for(int tid = 0; tid < next_free_tid; tid++){
        printf("Thread Name : %s | *thread: %x | rsp: %x | thread_next: %x\n", 
            t->name,
            (uint64_t)t, 
            t->registers.iret_rsp, 
            (uint64_t)t->next
        );
        t = t->next;
    }
}



void init_processes() {
    
    // Create the init process
    process_t *process = create_process("process0", (void *)&thread0_func, NULL);
    if (!process) {
        printf("Failed to create init process\n");
        return;
    }

    process->status = READY;

    thread_t* thread0 = process->threads; // Get the main thread of the init process
    if (!thread0) {
        printf("Failed to get init thread\n");
        return;
    }

    // Create a thread with an argument
    thread_t* thread1 = create_thread(process, "Thread1", (void *)&thread1_func, NULL);
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

    printf("thread0: %x\n", current_process->threads);
}