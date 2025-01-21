
/*
Kernel.c
Build Date  : 16-12-2024
Last Update : 15-01-2025
Description : KeblaOS is a x86 architecture based 64 bit Operating System. Currently it is using Limine Bootloader.
Reference   : https://wiki.osdev.org/Limine
              https://github.com/limine-bootloader/limine-c-template
              https://wiki.osdev.org/Limine_Bare_Bones

*/


#include "../limine/limine.h" // bootloader info
#include "../bootloader/boot.h" // bootloader info
#include "../lib/stdio.h" // printf
#include "../util/util.h" // registers_t , halt_kernel

#include "../driver/vga.h" // vga_init, print_bootloader_info, print_memory_map, display_image

#include "../x86_64/gdt/gdt.h" // init_gdt
#include "../x86_64/idt/idt.h" // init_idt, test_interrupt

#include "../mmu/pmm.h" // init_pmm, test_pmm
#include "../mmu/paging.h" // init_paging, test_paging
#include "../mmu/vmm.h" // test_vmm
#include "../mmu/kheap.h" // init_kheap, test_kheap
#include "../driver/keyboard.h" // initKeyboard
#include "../x86_64/pit/pit_timer.h" // init_timer

#include "../pcb/process.h" // init_processes

#include "kernel.h"





void kmain(){
    get_bootloader_info();
    vga_init();

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

    initKeyboard();

    init_timer(1);
    

    init_processes();



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





