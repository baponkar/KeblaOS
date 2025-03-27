
/*
This file will test process-thread working
*/

#include "../x86_64/timer/apic_timer.h"     // for apic_delay
#include "../x86_64/timer/pit_timer.h"      // for delay
#include "../x86_64/timer/tsc.h"            // for tsc_sleep
#include "../x86_64/timer/hpet_timer.h"     // for hpet_sleep
#include "../lib/stdio.h"                   // for printf function

#include "test_process.h"


/*

V_UMEM_LOW_BASE : 0x1000

sizeof(process_t): 0x380, sizeof(thread_t): 0x9C0

uheap_base = 0x1000

after process creation, uheap_base = 0x1000 + 0x380 = 0x1380

after thread0 creation, uheap_base = 0x1380 + 0x9C0 = 0x1D40
after thread0 stack creation, uheap_base = 0x1D40 + 0x4000 = 0x5D40
thread0.registers.rsp = 0x5D40 + 0x4000 = 0x9D40 ; rsp = 0x9000

after thread1 creation, uheap_base = 0x5D40 + 0x9C0 = 0x6700
after thread1 stack creation, uheap_base = 0x6700 + 0x4000 = 0xA700
thread1.registers.rsp = 0xA700 + 0x4000 = 0xE700 ; rsp = 0x10000

V_UMEM_LOW_BASE : 0x11000

*/

/*

size of process_t = 0x380
size of thread_t  = 0x9C0
size of status_t  = 0x20

+======================+===========+
| V_UMEM_LOW_BASE      | 0x1000    |
|======================+===========|
| process0 ptr         | 0x1000    |------->>--------+
|----------------------|-----------|                 |
| thread0 ptr          | 0x3000    |<--- + 0x2000 ---+
|----------------------|-----------|                 |
| thread0 stack below  | 0x5000    |<--- + 0x2000 ---+
|----------------------|-----------|                 |
| thread0 stack top    | 0x9000    |<--- + 0x4000 ---+
|----------------------|-----------|                 |
| thread1 ptr          | 0xA000    |<--- +0x1000 ----+
|----------------------|-----------|                 |
| thread1 stack below  | 0xC000    |<--- +0x2000 ----+
|----------------------|-----------|                 |
| thread1 stack top    | 0x10000   |<--- +0x4000 ----+
+----------------------+-----------+                 |
| V_UMEM_LOW_BASE      | 0x11000   |<--- +0x1000 ----+
+----------------------+-----------+

*/


extern volatile uint64_t apic_ticks;

void thread0_func(void *arg) {
    int *var = (int*) arg;
    while(true){
        printf("==> Thread 0 is Running...\n");
        apic_delay(100); // delay 100 milli seconds
    }
}


void thread1_func(void* arg) {
    while(true) {
        printf("### Thread 1 is Running...\n");
        apic_delay(100); // delay 100 milli seconds
    }
}

void thread2_func(void* arg) {
    while(true) {
        printf("### Thread 2 is Running...\n");
        apic_delay(100); // delay 100 milli seconds
    }
}

void thread10_func(void *arg) {
    int *var = (int*) arg;
    while(true){
        printf("===> Thread 10 is Running...\n");
        apic_delay(100); // delay 100 milli seconds
    }
}


void thread11_func(void* arg) {
    while(true) {
        printf("#### Thread 11 is Running...\n");
        apic_delay(100); // delay 100 milli seconds
    }
}

void thread12_func(void* arg) {
    while(true) {
        printf("#### Thread 12 is Running...\n");
        apic_delay(100); // delay 100 milli seconds
    }
}


void print_all_threads_name(process_t *p){
    thread_t * t = p->threads;
    for(uint64_t tid = 0; tid < next_free_tid; tid++){
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
    process_t *process = create_process("process0");
    if (!process) {
        printf("Failed to create init process\n");
        return;
    }

    process_t *process1 = create_process("process1");
    if (!process) {
        printf("Failed to create init process\n");
        return;
    }

    process->status = READY;
    process1->status = READY;

    thread_t* thread0 = create_thread(process, "Thread0", (void *) &thread0_func, NULL);
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
    thread_t* thread2 = create_thread(process, "Thread2", (void *) &thread2_func, NULL);
    if (!thread1) {
        printf("Failed to create Thread3\n");
        return;
    }

    thread_t* thread10 = create_thread(process1, "Thread10", (void *) &thread10_func, NULL);
    if (!thread0) {
        printf("Failed to get init thread\n");
        return;
    }

    // Create a thread with an argument
    thread_t* thread11 = create_thread(process1, "Thread11", (void *) &thread11_func, NULL);
    if (!thread1) {
        printf("Failed to create Thread1\n");
        return;
    }

    // Create a thread with an argument
    thread_t* thread12 = create_thread(process1, "Thread12", (void *) &thread12_func, NULL);
    if (!thread1) {
        printf("Failed to create Thread3\n");
        return;
    }

    // Set the current process
    current_process = process;
    processes_list = process;

    current_process->next = process1;
    processes_list = process1;

    thread0->next = thread1;
    thread1->next = thread2;
    thread2->next = thread0;

    thread10->next = thread11;
    thread11->next = thread12;
    thread12->next = thread10;
}