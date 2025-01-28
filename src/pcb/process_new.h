#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define NAME_MAX_LEN 64

typedef enum {
    READY,
    RUNNING,
    WAITING,
    TERMINATED
} status_t;

typedef struct thread {
    uint64_t tid;                   // Thread ID
    char name[NAME_MAX_LEN];        // Thread name
    uint64_t *stack;                // Kernel stack for the thread
    uint64_t *rsp;                  // Saved stack pointer
    status_t state;                 // Thread state (READY, RUNNING, etc.)
    struct thread *next;            // Pointer to the next thread in the process
} thread_t;

typedef struct process {
    uint64_t pid;                   // Process ID
    char name[NAME_MAX_LEN];        // Process name
    thread_t *threads;              // List of threads in the process
    status_t state;                 // Process state (READY, RUNNING, etc.)
    struct process *next;           // Pointer to the next process
} process_t;


