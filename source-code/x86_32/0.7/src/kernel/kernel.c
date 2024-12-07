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

  
 
  // Halt the CPU if we return here
  for (;;) {
    asm("hlt");
  }

  return 0;
}






