
#include "kernel.h"

void kmain (uint32_t magic, struct multiboot_info *bootInfo);
void halt_cpu(void);
void check_system();

extern uint32_t * _start;
// _end = 0x000001F9 = 0.493 KB = 505 bytes
extern uint32_t * _end;  // Defined in assembly and linker script

uint32_t placement_address = 0x100000; // 1MB, Initialize to a reasonable starting address in memory.


void kmain (uint32_t magic, struct multiboot_info *bootInfo)
{
    vga_clear();    // Clear Screen with default color
    color_print(KEBLA_OS_LOGO, COLOR8_BLACK, COLOR8_CYAN); // Printing KeblaOS ASCI art

    init_gdt();     // initialization of Global Descriptor Table
    init_paging();  // Initialization of PAGING
    isr_install();  // initialization of Interrupt Descriptor Table with ISR
    irq_install();  // initialization of Interrupt Descriptor Table with IRQ
    initKeyboard(); // initialization of  Keyboard Driver
    initTimer();    // initialization of PIT timer
    check_system(); // Checking various implemented portion

    print("Memory Address: \n");
    print_hex((uint32_t) &_start);
    print("\n");
    print_hex((uint32_t) &_end);

    halt_cpu();     // Halt the CPU if we return here
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

void print_mutiboot_info(){
  
}
