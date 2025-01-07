
/*
Kernel.c
Build Date  : 16-12-2024
Last Update : 05-01-2025
Description : KeblaOS is a x86 architecture based 64 bit Operating System. Currently it is using Limine Bootloader.
Reference   : https://wiki.osdev.org/Limine
              https://github.com/limine-bootloader/limine-c-template
              https://wiki.osdev.org/Limine_Bare_Bones

*/

#include "kernel.h"

char *OS_NAME = "KeblaOS";
char *OS_VERSION = "0.11";
char *BUILD_DATE = "16/12/2024";

void kmain(void){

    vga_init();

    get_bootloader_info();

    print(OS_NAME);
    print("-");
    print(OS_VERSION);
    print("\n");

    init_gdt();
    // check_gdt();

    init_idt();
    // test_interrupt();

    initialise_paging();
    // test_paging();

    // init_timer();
    
    initKeyboard();

    init_mem();
    // print_memory_map();

    init_kheap();
    heap_test();

    halt_kernel();
}
