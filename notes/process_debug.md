# Output

```c
struct thread {                     // Allocated size 0x70
    size_t tid;                     // Thread ID
    status_t status;                // Thread status
    char name[THREAD_NAME_MAX_LEN]; // Thread name
    process_t* parent;              // Reference to parent process
    struct thread* next;            // Linked list for threads
    uint64_t cpu_time;              // Track CPU time per thread

    registers_t *registers;         // Thread registers
};
``

```bash
Created Thread: Thread0 (TID: 0) on (PID: 0) | rip : 0xFFFFFFFF8000A541 | rsp : 0xFFFFFFFF80329000| next: 0x0
Created Thread: Thread1 (TID: 1) on (PID: 0) | rip : 0xFFFFFFFF8000A576 | rsp : 0xFFFFFFFF8032F000| next: 0x0
Created Thread: Thread2 (TID: 2) on (PID: 0) | rip : 0xFFFFFFFF8000A5AB | rsp : 0xFFFFFFFF80335000| next: 0x0

Thread Name : Thread0 | *thread: 0xFFFFFFFF80323000 | rsp: 0xFFFFFFFF80329000 | thread_next: 0xFFFFFFFF80329000
Thread Name : Thread1 | *thread: 0xFFFFFFFF80329000 | rsp: 0xFFFFFFFF8032F000 | thread_next: 0xFFFFFFFF8032F000
Thread Name : Thread2 | *thread: 0xFFFFFFFF8032F000 | rsp: 0xFFFFFFFF80335000 | thread_next: 0x0
```

```bash
sizeof(thread_t) = 0x70
THREAD_STACK_SIZE 0x4000 // 16 KB
sizeof(registers_t) = 0xD0
```

```bash
Thread Name : Thread0 | *thread: 0xFFFFFFFF80323000
Created Thread: Thread0 (TID: 0) on (PID: 0) | rip : 0xFFFFFFFF8000A541 | rsp : 0xFFFFFFFF80329000| next: 0xFFFFFFFF80329000
Address of Thread0-registers:  0xFFFFFFFF80324000
Low address of thread0->stack: 0xFFFFFFFF80325000
Top address of thread0->stack: 0xFFFFFFFF80329000   => 0xFFFFFFFF80325000 + 0x4000

Thread Name : Thread1 | *thread: 0xFFFFFFFF80329000
Created Thread: Thread1 (TID: 1) on (PID: 0) | rip : 0xFFFFFFFF8000A576 | rsp : 0xFFFFFFFF8032F000| next: 0xFFFFFFFF8032F000
Address of Thread1-registers:  0xFFFFFFFF8032A000
Low address of thread1->stack: 0xFFFFFFFF8032B000
Top address of thread1->stack: 0xFFFFFFFF8032F000   => 0xFFFFFFFF8032B000 + 0x4000

Thread Name : Thread2 | *thread: 0xFFFFFFFF8032F000
Created Thread: Thread2 (TID: 2) on (PID: 0) | rip : 0xFFFFFFFF8000A5AB | rsp : 0xFFFFFFFF80335000| next: 0x0
Address of Thread2-registers:  0xFFFFFFFF80330000
Low address of thread2->stack: 0xFFFFFFFF80331000
Top address of thread1->stack: 0xFFFFFFFF80335000   => 0xFFFFFFFF80331000 + 0x4000

```

*thread1 - *thread0 = 0x6000
*thread2 - *thread1 = 0x6000

## Memory Calculation for thread0:

```bash
*Thread0 : 0xFFFFFFFF80323000

```