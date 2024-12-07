// kernel.c -- Defines the C-code kernel entry point, calls initialisation routines.
// Made for JamesM's tutorials

#include "kernel.h"

int main()
{
  vga_clear();
  print(KEBLA_OS_LOGO);

  init_gdt();
  print("===> [Initialization of GDT]\n");
  check_gdt();

  setup_paging();
  print("===> [Initialization of PAGING]\n");
  test_paging();

  init_idt();
  print("===> [Initialization of IDT]\n");
  
  irq_install();
  print("===> [Initialization of IRQ]\n");

  initKeyboard();
  print("===> [Initialization of Keyboard]\n");
  
  // Testing Interrupts
  // asm volatile ("int $0x3");
  // asm volatile ("int $0x0");
  // asm volatile ("int $0x20");
  // print_dec( 104 / 0 );

  initTimer();
  print("===> [Initialization of Timer]\n");

  scheduler_init();  // Initialize the scheduler

  // Create processes
  process_t *p1 = create_process(process1);
  process_t *p2 = create_process(process2);

  // Add processes to the scheduler
  scheduler_add(p1);
  scheduler_add(p2);

  // Set up the timer for preemption
  init_timer(100);  // Set timer to 100Hz (100 ticks per second)

  // Start the first process
  scheduler();

  // process_t *p1 = create_process(simple_program);
  // scheduler_add(p1);  // Add the process to the scheduler
 
  // Halt the CPU if we return here
  for (;;) {
    asm("hlt");
  }

  return 0;
}

// void simple_program() {
//     for (int i = 0; i < 5; i++) {
//         print("Hello from the process!\n");
//     }
//     syscall_exit();  // Exit the process
// }

void process1() {
    while (1) {
        print("Process 1 running...\n");
        syscall_yield();  // Voluntarily yield the CPU
    }
}

void process2() {
    while (1) {
        print("Process 2 running...\n");
        syscall_yield();  // Voluntarily yield the CPU
    }
}




