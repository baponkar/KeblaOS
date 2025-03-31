

#include "../driver/vga/vga_term.h"  // Assuming you have a VGA text driver.
#include "../driver/io/ports.h"
#include "../lib/string.h"    // Assuming you have string manipulation functions.
#include "../lib/stdio.h"

#include "../driver/keyboard/keyboard.h"  // Assuming you have keyboard input handling.
#include "../x86_64/gdt/gdt.h"
#include "../util/util.h"

#include "../x86_64/interrupt/pic.h"
#include "../x86_64/interrupt/apic.h"
#include "../x86_64/interrupt/interrupt.h"
#include "../kernel/kernel.h"

#include "../x86_64/timer/rtc.h"

#include "../driver/image_data.h"

#include "../acpi/acpi.h"
#include "../acpi/descriptor_table/fadt.h"

#include "../bootloader/boot.h"
#include "../memory/detect_memory.h"
#include "../bootloader/firmware.h"



#include "shell.h"



bool shell_running = false;  // Global flag to manage shell state

void shell_prompt() {
    print("KeblaOS>> ");
}



void execute_command(char* command) {
    
    if (strcmp(command, "help") == 0) {
        print("Available commands: [ help, clear, reboot, poweroff, bootinfo, time, uptime, regvalue, checkgdt, features, testint, logo, image, memmap,  exit, kermod, rsdp, firmware, stack, limine, pagingmod, phystoviroff, smp, hhdm ]\n");
    } else if (strcmp(command, "clear") == 0) {
        clear_screen();  // Clear the screen using your VGA driver
    } else if (strcmp(command, "reboot") == 0) {
        print("Rebooting...\n");
        // qemu_reboot();
        acpi_reboot();
    } else if (strcmp(command, "poweroff") == 0){
        print("Shutting Down!\n");
        //qemu_poweroff();
        acpi_poweroff();
    } else if(strcmp(command, "bootinfo") == 0){
        print_bootloader_info();
    } else if (strcmp(command, "time") == 0){
        print_current_time();
        // printing current time
    } else if (strcmp(command, "uptime") == 0){
        print("Up time : \n");
        // printing the time run this os
    }else if (strcmp(command, "regvalue") == 0){
        // print_registers();
    }else if (strcmp(command, "features") == 0){
        print_features();
    }else if (strcmp(command, "testint") == 0){
        // print("Test Interrupts\n");
        test_interrupt();
    }else if (strcmp(command, "exit") == 0) {
        print("Exiting shell...\n");
        shell_running = false;
    }else if(strcmp(command, "logo") == 0){
        // clear_screen();
        // display_image(100, 300, (const uint64_t*) KeblaOS_icon_320x200x32, KEBLAOS_ICON_320X200X32_WIDTH,KEBLAOS_ICON_320X200X32_HEIGHT);
    }else if(strcmp(command, "image") == 0){
        clear_screen();
        // display_image(0, 0, (const uint64_t*) girl_6352783_640, GIRL_6352783_640_WIDTH, GIRL_6352783_640_HEIGHT);
    }else if(strcmp(command, "") == 0){
        print("type 'help'\n");
    }else if(strcmp(command, "memmap") == 0){
        print_memory_map();
    }else if(strcmp(command, "kermod") == 0){
        print_kernel_modules_info();
    }else if(strcmp(command, "firmware") == 0){
        print_firmware_info();
    }else if(strcmp(command, "stack") == 0){
        print_stack_info();
    }else if(strcmp(command, "limine") == 0){
        print_limine_info();
    }else if(strcmp(command, "pagingmod") == 0){
        print_paging_mode_info();
    }else if(strcmp(command, "phystoviroff") == 0){
        print_kernel_to_virtual_offset();
    }else if(strcmp(command, "hhdm") == 0){
        print_hhdm_info();
    }else{
        print("!Unknown command: ");
        print(command);
        print("\n");
        print("type 'help'\n");
    }
}


void run_shell(bool is_shell_running) {
    char input[BUFFER_SIZE];

    while (is_shell_running) {
        shell_prompt();  // Display shell prompt
        read_command(input);  // Function to read user input (can be implemented based on your keyboard handler)
        execute_command(input);  // Process the input command
    }
}



void shell_main() {
    shell_running = true;
    run_shell(shell_running);
    
    // If shell exits, we should halt or restart it.
    while (1) {
        asm volatile("hlt");
    }
}




void print_features(){
    print("KeblaOS-0.11\n");
    print("Architecture : x86_64.\n");
    print("1. Limine Bootloading.\n");
    print("2. GDT initialization.\n");
    print("3. VGA Graphics Driver.\n");
    print("4. IDT initialization.\n");
    print("5. Keyboard driver initialization.\n");
    print("6. PIT Timer initialization.\n");
    print("7. Basic User Shell\n");
    print("8. Memory Management Unit(Kheap, PMM, 4 Level Paging)\n");
    print("7. Standard Libraries : math.h, stddef.h, stdint.h, stdio.h, stdlib.h, string.h\n");
}


