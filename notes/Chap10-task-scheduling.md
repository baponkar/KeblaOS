# Task or Process or Thread and Scheduler

Here I am using "One Kernel stack per task" . The Task or Process or Job or Thread can be defined by Task Control Block (TCB) or Process Control Block(PCB) which have initial informaton of the task like, task id, task name, separate stack pointer address, linked list next task address etc. Suppose I am defining a TCB by

```c
// The below enum will help to assign the state of a task
typedef enum {
    READY,
    RUNNING,
    DEAD
} status_t;

typedef struct task{
    size_t id;  // Some numbers like 0,1,2,... to identify the task
    char name[NAME_MAX_LEN]; // NAME_MAX_LENGTH ' s value 64 is good 
    status_t status;    // current status of this task
    registers_t regs; // hold cpu state
    struct task *prev;
    struct task *next; // linked to next task pointer
} task_t;

```

* Creating a new task by assigning its all elements

```c
task_t *create_task(char* name, void(*function)(void*), void* arg, task_t *prev_task){
    task_t *new_task = (task_t *) (kheap_alloc(sizeof(task_t))); // This will arrange some space for new_task structure

    if(!new_taask) return NULL; // If we faild to assiging the space

    strncpy(new_task->name, name, NAME_MAX_LEN); // assiging task name
    new_task->id = next_free_id++;
    new_task->status = READY;
    new_task->registers = (registers_t *) kheap_alloc(sizeof(registers_t)); // assiging some space and set its pointer address
    if(!new_task->registers) {
        kheap_free(new_task, sizeof(task_t));
        return NULL;
    }
    new_task->prev = prev_task;
    new_task->next = NULL;

    printf("Successfully created the Task\n");

    return new_task;
}
```

Suppose I am creating two fake non stopping task

```c
void task1(void* arg) {
    while (1) {
        printf("Task 1 is Running\n");
        apic_delay(1000);  // Delay for 1000ms (1 second)
    }
}


void task2(void* arg) {
    while(1) {
        printf("Task 2 is Running\n");
        apic_delay(1000);
    }
}

void init_task(){
    // creating a task which is running from the start of the kernel or first task
    task_t *task0 = create_task("kernel", NULL, NULL, NULL);

    // Creating two manual task
    task_t *task1 = create_task("task1", task1, NULL, task0);
    task_t *task2 = create_task("task2", task1, NULL, task1);

}
```

There are many situation to change task like end of a task then we need to change task.



## References :

1. [osdev-notes](https://github.com/dreamportdev/Osdev-Notes/blob/master/05_Scheduling/README.md)

2. [Processes and Threads in wiki.osdev.io](https://wiki.osdev.org/Processes_and_Threads)

3. [Brendan's Multi-tasking Tutorial](https://wiki.osdev.org/Brendan%27s_Multi-tasking_Tutorial)

4. [osdever.net](http://www.osdever.net/tutorials/view/software-task-switching)



