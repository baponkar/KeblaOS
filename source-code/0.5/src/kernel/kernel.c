// kernel.c -- Defines the C-code kernel entry point, calls initialisation routines.
// Made for JamesM's tutorials

#include "../driver/vga.h"
#include "../gdt/gdt.h"
#include "../idt/idt.h"
#include "../idt/timer.h"
#include "osinfo.h"

int main(struct multiboot *mboot_ptr)
{
  vga_clear();
  print(KEBLA_OS_LOGO);

  init_gdt();
  print("===> [ Initialization of GDT ]\n");
  check_gdt();

  init_idt();
  print("===> [ Initialization of IDT ]\n");
  
  irq_install();
  print("===> [ Initialization of IRQ ]\n");
  
  // Testing Interrupts
  // asm volatile ("int $0x3");
  // asm volatile ("int $0x0");
  // asm volatile ("int $0x20");
  // print_dec( 104 / 0 );

  initTimer();
  print("===> [ Initialization of Timer ]\n");

  // Halt the CPU if we return here
  for (;;) {
    asm("hlt");
  }

  return 0xDEADBABA;
}


