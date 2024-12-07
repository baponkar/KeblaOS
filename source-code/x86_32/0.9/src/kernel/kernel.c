// kernel.c -- Defines the C-code kernel entry point, calls initialisation routines.
// Made for JamesM's tutorials

#include "kernel.h"

int main()
{
  vga_clear();          // Clear Screen with default color
  print(KEBLA_OS_LOGO); // Printing KeblaOS ASCI art

  init_gdt();     // initialization of Global Descriptor Table
  init_idt();     // initialization of Interrupt Descriptor Table with ISR
  irq_install();  // initialization of Interrupt Descriptor Table with IRQ
  initKeyboard(); // initialization of  Keyboard Driver
  initTimer();    // initialization of PIT timer

  

  // Halt the CPU if we return here
  for (;;) {
    asm("hlt");
  }

  return 0;
}






