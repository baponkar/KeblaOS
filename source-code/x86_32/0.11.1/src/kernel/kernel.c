// kernel.c -- Defines the C-code kernel entry point, calls initialisation routines.
// Made for JamesM's tutorials

#include "kernel.h"

extern uint32_t end;  // Defined in linker.ld
uint32_t placement_address = (uint32_t) &end;

int main()
{
  vga_clear();          // Clear Screen with default color
  print(KEBLA_OS_LOGO); // Printing KeblaOS ASCI art

  init_gdt();     // initialization of Global Descriptor Table
  setup_paging(); // initialization of Paging
  init_idt();     // initialization of Interrupt Descriptor Table with ISR
  irq_install();  // initialization of Interrupt Descriptor Table with IRQ
  initKeyboard(); // initialization of  Keyboard Driver
  initTimer();    // initialization of PIT timer

  check_system();
  // Halt the CPU if we return here
  for (;;) {
    asm("hlt");
  }

  return 0;
}


void check_system(){
    // Testing Different sector
    // check_gdt();
    // test_interrupt();
    // test_paging();
    // trigger_page_fault_by_access();
    test_valid_paging();
    // test_invalid_paging();
}





