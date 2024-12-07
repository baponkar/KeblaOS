/*
  Kebla OS
  Developed by : Bapon Kar
  License : GNU GPL v3.0
  Download Link : https://github.com/baponkar/KeblaOS
*/

#include "kernel.h"

void halt_cpu(void);
void check_system();

int main()
{
    vga_clear();          // Clear Screen with default color
    color_print(KEBLA_OS_LOGO, COLOR8_BLACK, COLOR8_CYAN); // Printing KeblaOS ASCI art

    init_gdt();     // initialization of Global Descriptor Table
    init_paging();  // Initialization of PAGING
    isr_install();  // initialization of Interrupt Descriptor Table with ISR
    irq_install();  // initialization of Interrupt Descriptor Table with IRQ
    initKeyboard(); // initialization of  Keyboard Driver
    initTimer();    // initialization of PIT timer

  
    check_system();

    halt_cpu();     // Halt the CPU if we return here

    return 0;
}


void halt_cpu(){
    for (;;) {
      asm volatile("hlt");
    }
}

void check_system(){
    // Testing Different sector
    // check_gdt();
    // test_interrupt();
    // test_paging();
    // trigger_page_fault_by_access();
}






