#include "kernel.h"


int main()
{
  //vga_clear();
  print(KEBLA_OS_LOGO);

  init_gdt();
  print("===> [Initialization of GDT]\n");
  check_gdt();

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

  // int *ptr;
  // int *ptr1 = memset(ptr, 0, 4);
  // print_hex(ptr1);
  // print_bin(ptr1);
  
  // print_at("-------------------------------------------------------------------------------",0,1);
  // print_at("-------------------------------------------------------------------------------",0,24);

  // for(int row=0;row<24;row++){
  //   print_at("|",0,row);
  //   print_at("|",79,row);
  // }

// Set VGA mode to 13h
    set_video_mode_13h();

    // Clear the back buffer to BLACK
    clear_back_buffer();

    // Draw a magenta rectangle
    draw_rect(25, 50, 75, 100, MAGENTA);

    // Draw a green circle
    draw_circle(160, 100, 50, GREEN);

    // Print a string
    print_string("Hello, VGA!", 50, 150, WHITE);

    // Swap buffers to display the graphics
    swap_buffers();



  // Halt the CPU if we return here
  for (;;) {
    asm("hlt");
  }

  return 0;
}