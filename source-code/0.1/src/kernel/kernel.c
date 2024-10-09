// kernel.c -- Defines the C-code kernel entry point, calls initialisation routines.
// Made for JamesM's tutorials

#include "../driver/vga.h"

int main(struct multiboot *mboot_ptr)
{
  monitor_clear();
  monitor_write("Hello, world!");
  monitor_put('\n');
  monitor_write_hex(0xf);
  monitor_put('\n');
  monitor_write_dec(25);
  // All our initialisation calls will go in here.
  return 0xDEADBABA;

  // Halt the CPU if we return here
    for (;;) {
        asm("hlt");
    }
}