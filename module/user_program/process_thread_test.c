

#include "../libc/include/syscall.h"
#include "../libc/include/stdio.h"
#include "../libc/include/string.h"


#include "process_thread_test.h"



int process_test() {
    // Create a process
    void *proc = syscall_create_process("my_process");
    if (!proc) {
        printf("Failed to create process!\n");
        return -1;
    }

    printf("Process created at %p\n", proc);

    // Get current process
    void *cur = syscall_get_current_process();
    printf("Current process: %p\n", cur);

    // Lookup by PID (example: assume PID=1 exists)
    void *proc2 = syscall_get_process_from_pid(1);
    printf("Process with PID=1 is at %p\n", proc2);

    // Delete process
    syscall_delete_process(proc);
    printf("Process deleted!\n");

    return 0;
}



void worker(void *arg) {
    printf("Worker thread running with arg=%s\n", (char*)arg);
    while (1) {
        // do work
    }
}

int thread_test() {
    // Create a process
    void *proc = syscall_create_process("thread_demo");
    printf("Created process at %p\n", proc);

    // Create a thread inside that process
    void *thread = syscall_create_thread(proc, "worker_thread", worker, "HelloArg");
    printf("Created thread at %p\n", thread);

    // Delete the thread later
    syscall_delete_thread(thread);
    printf("Thread deleted!\n");

    // Delete the process
    syscall_delete_process(proc);
    printf("Process deleted!\n");

    return 0;
}





