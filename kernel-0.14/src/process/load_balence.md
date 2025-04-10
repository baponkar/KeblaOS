To distribute processes and threads across multiple CPUs in your OS, follow these steps:

### 1. **Implement Per-CPU Scheduler Structures**
Add runqueue and current thread tracking to each CPU's data structure.

```c
// Updated cpu_data_t structure
typedef struct {
    gdt_entry_t gdt[GDT_ENTRIES_COUNT];
    tss_entry_t tss_entry;
    tss_t tss;
    uint64_t kernel_stack;
    thread_t* current_thread;    // Per-CPU current thread
    thread_t* runqueue;          // Per-CPU runqueue
    spinlock_t runqueue_lock;    // Lock for runqueue
} cpu_data_t;

cpu_data_t cpu_data[MAX_CORES];
```

### 2. **Thread Creation with Kernel Stacks**
Allocate kernel stacks for threads and assign them to CPU runqueues.

```c
thread_t* create_thread(process_t* parent, const char* name, 
                       void (*function)(void*), void* arg) {
    // ... existing thread setup ...

    // Allocate KERNEL stack (4KB)
    void* kstack = kmalloc_a(KERNEL_STACK_SIZE, true); 
    thread->kernel_stack = (uint64_t)kstack + KERNEL_STACK_SIZE;

    // Assign to a CPU's runqueue (round-robin)
    static atomic_int next_cpu = 0;
    int cpu_id = atomic_fetch_add(&next_cpu, 1) % num_cores;
    
    spin_lock(&cpu_data[cpu_id].runqueue_lock);
    thread->next = cpu_data[cpu_id].runqueue;
    cpu_data[cpu_id].runqueue = thread;
    spin_unlock(&cpu_data[cpu_id].runqueue_lock);
}
```

### 3. **Per-CPU Scheduler**
Modify the scheduler to use the current CPU's runqueue and update TSS.

```c
registers_t* schedule(registers_t* registers) {
    // Get current CPU ID (e.g., via APIC)
    int cpu_id = lapic_get_id() >> 24; 
    cpu_data_t* cpu = &cpu_data[cpu_id];

    // Save current thread state
    if (cpu->current_thread) {
        memcpy(&cpu->current_thread->registers, registers, sizeof(registers_t));
        cpu->current_thread->status = READY;
    }

    // Select next thread from runqueue
    spin_lock(&cpu->runqueue_lock);
    thread_t* next = cpu->runqueue;
    if (next) cpu->runqueue = next->next;
    spin_unlock(&cpu->runqueue_lock);

    if (!next) return NULL; // Idle thread

    // Update TSS to new thread's kernel stack
    cpu->tss.rsp0 = next->kernel_stack; 
    next->status = RUNNING;
    cpu->current_thread = next;

    return &next->registers;
}
```

### 4. **CPU Identification**
Add a way to get the current CPU's ID (e.g., via APIC).

```c
// Get current CPU's APIC ID
uint32_t lapic_get_id() {
    return *(volatile uint32_t*)(LAPIC_BASE + LAPIC_ID_REG);
}
```

### 5. **Load Balancing (Basic)**
Implement work stealing when a CPU's runqueue is empty.

```c
thread_t* steal_work(int cpu_id) {
    for (int i = 0; i < num_cores; i++) {
        if (i == cpu_id) continue;
        spin_lock(&cpu_data[i].runqueue_lock);
        if (cpu_data[i].runqueue) {
            thread_t* stolen = cpu_data[i].runqueue;
            cpu_data[i].runqueue = stolen->next;
            spin_unlock(&cpu_data[i].runqueue_lock);
            return stolen;
        }
        spin_unlock(&cpu_data[i].runqueue_lock);
    }
    return NULL;
}
```

### 6. **Core Initialization**
Initialize scheduler structures for each core.

```c
void core_init(int core) {
    // ... existing GDT/TSS setup ...

    // Initialize scheduler for this core
    cpu_data[core].runqueue = NULL;
    cpu_data[core].current_thread = NULL;
    spin_init(&cpu_data[core].runqueue_lock);
}
```

### 7. **Thread State Setup**
Ensure threads start in user mode with proper stacks.

```c
// In create_thread():
thread->registers.iret_cs = USER_CS;  // 0x18 (user code segment)
thread->registers.iret_ss = USER_SS;  // 0x20 (user data segment)
thread->registers.iret_rsp = user_stack; 
thread->registers.iret_rip = function;
```

### Key Changes Summary
| Component          | Modification                                  |
|--------------------|-----------------------------------------------|
| **CPU Data**       | Added runqueue, current thread, and lock      |
| **Thread Creation**| Assign threads to CPU runqueues round-robin   |
| **Scheduler**      | Per-CPU runqueues and TSS.rsp0 update         |
| **APIC**           | Use LAPIC ID for CPU identification          |

### Verification Steps
1. **Check Runqueue Population**  
   Print each CPU's runqueue after thread creation.
2. **Validate TSS.rsp0 Update**  
   Use debugger to verify `rsp0` changes during context switches.
3. **Test Multi-Core Execution**  
   Create CPU-bound threads and observe parallel execution via APIC timer logs.

This design allows threads to be distributed across CPUs while ensuring each core manages its own scheduling state.