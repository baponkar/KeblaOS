
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
    get_bootloader_info();
    vga_init();
    print(OS_NAME);
    print("--");
    print(OS_VERSION);
    print("\n");

    // display_image((FRAMEBUFFER_WIDTH - KEBLAOS_ICON_320X200X32_WIDTH)/2 , (FRAMEBUFFER_HEIGHT - KEBLAOS_ICON_320X200X32_WIDTH)/2, KeblaOS_icon_320x200x32, KEBLAOS_ICON_320X200X32_WIDTH, KEBLAOS_ICON_320X200X32_HEIGHT);

    
    // print_bootloader_info();
    // print_memory_map();
    // mem_info();

    init_gdt();
    // check_gdt();

    init_idt();
    // test_interrupt();

    // test_kmalloc();

    init_pmm();
    // test_pmm();


    init_paging();
    // test_paging();

    // test_vmm();

    init_kheap();
    // test_kheap();

    init_timer();
    
    initKeyboard();

    init_scheduler();      // Initialize the scheduler

    while (1) {
        scheduler_tick();  // Run the scheduler
    }

    // // Create a few windows
    // Window* win1 = create_window(10, 10, 200, 100, 0xAAAAAA, "Window 1");
    // Window* win2 = create_window(30, 30, 200, 100, 0xBBBBBB, "Window 2");
    
    // add_window(win1);
    // add_window(win2);
    
    // // Draw all windows
    // render_all_windows();
    
    // // Create a button on window 1
    // draw_button(win1, 50, 50, 500, 400, "Click Me");

    // int mouse_x = 0;
    // int mouse_y = 0;
    // bool mouse_clicked = false;
    // // Main loop
    // while (1) {
    //     // Get input and update mouse position
    //     // Assume you have some function for this
    //     update_mouse_position(mouse_x, mouse_y, mouse_clicked);

    //     // Handle mouse input
    //     handle_mouse_input();

    //     // Render windows periodically
    //     render_all_windows();
    // }

    halt_kernel();
}

void mem_info(){

    print("\nKernel physical base (upper) : ");
    print_hex(KMEM_UP_BASE);
    print("\n");

    print("Kernel physical base (lower) : ");
    print_hex(KMEM_LOW_BASE);
    print("\n");

    print("Kernel physical LENGTH (upper - low) : ");
    print_hex(KMEM_UP_BASE - KMEM_LOW_BASE);
    print("\n");

    print("\nKernel virtual base (upper) : ");
    print_hex(V_KMEM_UP_BASE);
    print("\n");

    print("Kernel virtual base (lower) : ");
    print_hex(V_KMEM_LOW_BASE);
    print("\n");

    print("Kernel virtual LENGTH (upper - low) : ");
    print_hex(V_KMEM_UP_BASE - V_KMEM_LOW_BASE);
    print("\n");
}
