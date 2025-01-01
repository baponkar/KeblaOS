
/*
Kernel.c
Build Date  : 16/12/2024
Description :
Reference   : 
              https://github.com/limine-bootloader/limine/blob/v8.x/PROTOCOL.md#kernel-address-feature
              https://wiki.osdev.org/Limine
              https://github.com/limine-bootloader/limine-c-template
              https://wiki.osdev.org/Limine_Bare_Bones

*/

#include "kernel.h"



char *OS_NAME = "KeblaOS";
char *OS_VERSION = "0.11";
char *BUILD_DATE = "16/12/2024";



// Set the base revision to 3, this is recommended as this is the latest
// base revision described by the Limine boot protocol specification.
// See specification for further info.

__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(0);

__attribute__((used, section(".requests_start_marker")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".requests_end_marker")))
static volatile LIMINE_REQUESTS_END_MARKER;


void kmain(void){

    vga_init();

    get_bootloader_info();

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
































