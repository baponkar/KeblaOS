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
    print ("    _  __      _      _           ____    _____ \n");
    print ("   | |/ /     | |    | |         / __ \\  / ____|\n");
    print ("   | ' /  ___ | |__  | |  __ _  | |  | || (___  \n"); 
    print ("   |  <  / _ \\| '_ \\ | | / _` | | |  | | \\___ \\ \n");
    print ("   | . \\|  __/| |_) || || (_| | | |__| | ____) |\n");
    print ("   |_|\\_|\\___||_.__/ |_| \\__,_|  \\____/ |_____/ \n");
    print ("                                   Version - 0.4\n");
    print("\n\n");                                                    

    print(" |------------------------------|\n");
    print(" | VGA Driver is used to display|\n");
    initGdt();
    print(" | GDT is done!                 |\n");
    initIdt();
    print(" | Interrupts is done!          |\n");
    initTimer();
    print(" | Timer is done!               |\n");
    initKeyboard();
    print(" | Keboard Driver is initiated! |\n");

    uint32_t mod1 = *(uint32_t*)(bootInfo->mods_addr + 4);
    uint32_t physicalAllocStart = (mod1 + 0xFFF) & ~0xFFF;

    initMemory(bootInfo->mem_upper * 1024, physicalAllocStart);
    kmallocInit(0x1000);
    print(" | Memory allocation done!      |\n");
    print(" |------------------------------|\n");

    print("~:$");

    for(;;);
}
