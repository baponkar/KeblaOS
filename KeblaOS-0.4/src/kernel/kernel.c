#include "../driver/vga.h"
#include "../stdlib/stdint.h"
#include "../gdt/gdt.h"
#include "../interrupts/idt.h"
#include "../interrupts/timer.h"
#include "../mmu/kmalloc.h"
#include "../stdlib/stdio.h"
#include "../driver/keyboard.h"
#include "../bootloader/multiboot.h"
#include "../mmu/memory.h"
#include "util.h"

void kmain(uint32_t magic, struct multiboot_info* bootInfo);

void kmain(uint32_t magic, struct multiboot_info* bootInfo){
    initGdt();
    initIdt();
    initTimer();
    initKeyboard();
    uint32_t mod1 = *(uint32_t*)(bootInfo->mods_addr + 4);
    uint32_t physicalAllocStart = (mod1 + 0xFFF) & ~0xFFF;
    initMemory(bootInfo->mem_upper * 1024, physicalAllocStart);
    kmallocInit(0x1000);

    Reset();
    enable_cursor(0, 1);
    delay_print ("    _  __      _      _           ____    _____ \n");
    delay_print ("   | |/ /     | |    | |         / __ \\  / ____|\n");
    delay_print ("   | ' /  ___ | |__  | |  __ _  | |  | || (___  \n"); 
    delay_print ("   |  <  / _ \\| '_ \\ | | / _` | | |  | | \\___ \\ \n");
    delay_print ("   | . \\|  __/| |_) || || (_| | | |__| | ____) |\n");
    delay_print ("   |_|\\_|\\___||_.__/ |_| \\__,_|  \\____/ |_____/ \n");
    delay_print ("                                   Version - 0.4\n");
    delay_print("\n\n");                                                    

    print(" |------------------------------|\n");
    print(" | VGA Driver is used to display|\n");
    
    print(" | GDT is done!                 |\n");
    
    print(" | Interrupts is done!          |\n");
    
    print(" | Timer is done!               |\n");
   
    print(" | Keboard Driver is initiated! |\n");

    

    
    print(" | Memory allocation done!      |\n");
    print(" |------------------------------|\n");

    delay_print("Hello World!");

    //uint16_t cursor_pos = get_cursor_position();
    //printf("%d\n", cursor_pos);     //0

    // Set cursor to position (row 10, column 5)
    //update_cursor(0 , 10); // Assuming VGA width is 80 characters
    //cursor_pos = get_cursor_position()/8;
    //printf("%d\n", cursor_pos);     //65478 65487 ffcf

    for(;;);
}
