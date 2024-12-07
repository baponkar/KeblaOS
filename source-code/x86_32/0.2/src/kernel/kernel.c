// kernel.c -- Defines the C-code kernel entry point, calls initialisation routines.
// Made for JamesM's tutorials

#include "../driver/vga.h"

int main(struct multiboot *mboot_ptr)
{
  vga_clear();
  putchar('K');
  putchar('A');

  create_newline();
  print("Hello, World!");

  
  print_hex(0x25AF);
  create_newline();
  print_dec(29);



  putchar_at((char) 0, 40, 2); // Error printing
  print_at("Test",42, 5);

 
  
  // All our initialisation calls will go in here.
  return 0xDEADBABA;

  // // Halt the CPU if we return here
    for (;;) {
        asm("hlt");
    }
}