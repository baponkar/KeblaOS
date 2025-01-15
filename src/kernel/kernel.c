
/*
Kernel.c
Build Date  : 16-12-2024
Last Update : 15-01-2025
Description : KeblaOS is a x86 architecture based 64 bit Operating System. Currently it is using Limine Bootloader.
Reference   : https://wiki.osdev.org/Limine
              https://github.com/limine-bootloader/limine-c-template
              https://wiki.osdev.org/Limine_Bare_Bones

*/

#include "kernel.h"

char *OS_NAME = "KeblaOS";
char *OS_VERSION = "0.11";
char *BUILD_DATE = "16/12/2024";

extern process_t *processes_list;
extern process_t *current_process;


void kmain(){

    vga_init();
    print(OS_NAME);
    print("--");
    print(OS_VERSION);
    print("\n");

    display_image((FRAMEBUFFER_WIDTH - KEBLAOS_ICON_320X200X32_WIDTH)/2 , (FRAMEBUFFER_HEIGHT - KEBLAOS_ICON_320X200X32_WIDTH)/2, KeblaOS_icon_320x200x32, KEBLAOS_ICON_320X200X32_WIDTH, KEBLAOS_ICON_320X200X32_HEIGHT);

    get_bootloader_info();
    // print_bootloader_info();

    init_gdt();
    // check_gdt();

    init_idt();
    // test_interrupt();

    init_pmm();
    // test_pmm();

    initialise_paging();
    // test_paging();

    init_vmm();
    // test_vmm();

    init_kheap();
    // test_heap();

    init_timer();
    
    initKeyboard();

    halt_kernel();
}
