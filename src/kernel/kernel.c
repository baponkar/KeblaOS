
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

    init_gdt();
    // check_gdt();

    init_idt();
    // test_interrupt();

    initialise_paging();
    // test_paging();

    init_timer();
    
    initKeyboard();

    print_memory_map();

    test_kheap();

    print("Hello World!\n");

    // uint64_t var_hex = 0x1234;
    // uint64_t var_bin = 0b100101;
    // int var_dec = 420;
    // print_dec(var_dec);

    // int64_t neg_no = -345;

    // print("var_hex = ");
    // print_hex(var_hex);
    // print("\n");

    // print("var_bin = ");
    // print_bin(var_bin);
    // print("\n");

    // print("var_dec = ");
    // print_dec(var_dec);
    // print("\n");

    // print("neg_no = ");
    // print_dec(neg_no);
    // print("\n");

    // print("\n");


    // printf("var_hex = %x\n", var_hex);  // 0x0000000000000010
    // printf("var_bin = %b\n", var_bin);  // 0b0000000000000000000000000000000000000000000000000000000000010000
    // printf("var_dec = %d\n", var_dec);  // -1
    // printf("neg_no = %d\n", neg_no);


    halt_kernel();
}
































