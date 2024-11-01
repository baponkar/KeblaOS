
#include "kernel.h"


extern void switch_to_user_mode(uint32_t user_stack, uint32_t user_entry);
extern uint32_t test_frame(uint32_t frame_addr);

/* Check if the bit BIT in FLAGS is set. */
#define CHECK_FLAG(flags,bit)   ((flags) & (1 << (bit)))

extern uint32_t * _start;   // Defined in assembly and linker script which gives 0
extern uint32_t * _end;  // Defined in assembly and linker script which gives 1203528 bytes÷1024=1175.5 KB=1.15MB

multiboot_info_t *mbi;  // Multi Boot Info

uint32_t placement_address = 0x200000; // 2MB, Initialize to a reasonable starting address in memory.
uint32_t mem_end_page;

void kmain (uint32_t magic, uint32_t addr)
{
    mbi = (multiboot_info_t *) addr;    // Getting Info from Multibootloader
    mem_end_page = (unsigned) mbi->mem_upper * 1024;   // Seting upper address of memory

    vga_clear();    // Clear Screen with default color
    color_print(KEBLA_OS_LOGO, COLOR8_BLACK, COLOR8_CYAN); // Printing KeblaOS ASCI art
    init_system();

    // check_system();

    // print_linker_pointer_info();
    // print_mutiboot_info(magic, addr);

    // get_cpu_info();

   
    // syscall_check();
    // Switch to user mode and run user_program
    //print_execution_mode();
    //print("Switching to user mode...\n");
    //start_user_program();
    //print("Back to kernel mode (if user mode returns)...\n");


    // draw_diagonal_line(mbi);
    // multiboot_uint32_t white_color = 0xFFFFFF;
    // multiboot_uint32_t blue_color = 0x0000FF;

    // // Call the clear_screen function with mbi and the color
    // cls(mbi, white_color);

    // clear_screen(mbi, blue_color);
    // draw_diagonal_line(mbi);
    // for(int i=0;i<100;i++){
    //     for(int j=0;j<100;j++){
    //         drawPixel(i, j, white_color, mbi);
    //     }
    // }
    
    halt_cpu();     // Halt the CPU if we return here
}   


void print_execution_mode() {
    uint32_t cs;
    
    // Read the CS register to check the privilege level
    asm volatile ("mov %%cs, %0" : "=r"(cs));

    // Extract DPL (the last two bits of CS register)
    uint32_t dpl = (cs & 0x03); // Masking with 0x03 gives us the last 2 bits

    if (dpl == 0) {
        print("Current Mode: Kernel Mode\n");
    } else {
        print("Current Mode: User Mode\n");
    }
}



void start_user_program() {
    uint32_t user_stack = 0x500000;     // Example user-mode stack address
    uint32_t user_entry = 0x400000;     // Example user-mode entry address (your user program)

    check_address_access(user_entry);
    // get_dpl_from_address(user_entry);

    // check_address_access(user_entry);
    // get_dpl_from_address(user_stack);
    
    // Switch to user mode
    // switch_to_user_mode(user_stack, user_entry);
    // print_execution_mode();
    // sys_write("Hello from user mode!\n");
}



void user_program() {
    sys_write("Hello from user mode!\n");

    // Try accessing kernel memory (expect a page fault if access is restricted)
    volatile int *ptr = (int*) 0xC0000000;  // Kernel address
    *ptr = 42;  // Should trigger a page fault if memory protection is correctly set
}


void init_system()
{
    init_gdt(); // initialization of Global Descriptor Table
    init_paging();  // Initialization of PAGING
    isr_install();  // initialization of Interrupt Descriptor Table with ISR
    irq_install();  // initialization of Interrupt Descriptor Table with IRQ
    initKeyboard(); // initialization of  Keyboard Driver
    initTimer();    // initialization of PIT timer
}


void halt_cpu(){
    for (;;) {
      asm volatile("hlt");
    }
}


void check_system(){
    // Testing Different sector
    check_gdt();
    // test_interrupt();
    uint32_t test_frame_address = 0x90000;
    if(test_frame(test_frame_address)){
        printf("Frame at address %x is in use.\n", test_frame_address);
    }else{
        printf("Frame at address %x is free.\n", test_frame_address);
    }
    // trigger_page_fault_by_access();
}

void print_linker_pointer_info(){
    printf("_start address : %u Byte, _end address: %u Byte\n",(uint32_t) &_start, (uint32_t) &_end);
}

void print_mutiboot_info(uint32_t magic, uint32_t addr){
    /* Am I booted by a Multiboot-compliant boot loader? */
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
    {
        printf("Invalid magic number: %x\n", (unsigned) magic);
        return;
    }

     /* Set MBI to the address of the Multiboot information structure. */
    mbi = (multiboot_info_t *) addr;
    /* Print out the flags. */
    printf ("flags = %x\n", (unsigned) mbi->flags);
     /* Are mem_* valid? */
    if (CHECK_FLAG (mbi->flags, 0))
        printf ("mem_lower = %u KB, mem_upper = %u KB\n",
                (unsigned) mbi->mem_lower, (unsigned) mbi->mem_upper);
    /* Is boot_device valid? */
    if (CHECK_FLAG (mbi->flags, 1))
        printf ("boot_device = 0x%x\n", (unsigned) mbi->boot_device);

    /* Is the command line passed? */
    if (CHECK_FLAG (mbi->flags, 2))
        printf ("cmdline = %s\n", (char *) mbi->cmdline);

    /* Are mods_* valid? */
    if (CHECK_FLAG (mbi->flags, 3))
        {
            multiboot_module_t *mod;
            int i;
        
            printf ("mods_count = %d, mods_addr = %x\n",
                (int) mbi->mods_count, (int) mbi->mods_addr);
            for (i = 0, mod = (multiboot_module_t *) mbi->mods_addr;
                i < mbi->mods_count;
                i++, mod++)
                printf ("mod_start = %x, mod_end = %x, cmdline = %s\n",
                    (unsigned) mod->mod_start,
                    (unsigned) mod->mod_end,
                    (char *) mod->cmdline);
        }
    
    /* Bits 4 and 5 are mutually exclusive! */
    if (CHECK_FLAG (mbi->flags, 4) && CHECK_FLAG (mbi->flags, 5))
        {
            printf ("Both bits 4 and 5 are set.\n");
            return;
        }

    /* Is the symbol table of a.out valid? */
    if (CHECK_FLAG (mbi->flags, 4))
        {
            multiboot_aout_symbol_table_t *multiboot_aout_sym = &(mbi->u.aout_sym);
        
            printf ("multiboot_aout_symbol_table: tabsize = %0x, "
                "strsize = %x, addr = %x\n",
                (unsigned) multiboot_aout_sym->tabsize,
                (unsigned) multiboot_aout_sym->strsize,
                (unsigned) multiboot_aout_sym->addr);
        }

    /* Is the section header table of ELF valid? */
    if (CHECK_FLAG (mbi->flags, 5))
        {
            multiboot_elf_section_header_table_t *multiboot_elf_sec = &(mbi->u.elf_sec);

            printf ("multiboot_elf_sec: num=%u, size=%x,"
                " addr=%x, shndx=%x\n",
                (unsigned) multiboot_elf_sec->num, (unsigned) multiboot_elf_sec->size,
                (unsigned) multiboot_elf_sec->addr, (unsigned) multiboot_elf_sec->shndx);
        }
        
    /* Are mmap_* valid? */
    if (CHECK_FLAG (mbi->flags, 6)){
            multiboot_memory_map_t *mmap;
        
            printf ("mmap_addr = %x, mmap_length = %x\n",(unsigned) mbi->mmap_addr, (unsigned) mbi->mmap_length);

            for (mmap = (multiboot_memory_map_t *) mbi->mmap_addr; (unsigned long) mmap < mbi->mmap_addr + mbi->mmap_length; mmap = (multiboot_memory_map_t *) ((unsigned long) mmap + mmap->size + sizeof (mmap->size))){
                printf ("size=%x, base_addr=%x,"
                        "length=%x, type=%x\n",
                        (unsigned) mmap->size,
                        (unsigned) (mmap->addr >> 32),
                        (unsigned) (mmap->addr & 0xffffffff),
                        (unsigned) (mmap->len >> 32),
                        (unsigned) (mmap->len & 0xffffffff),
                        (unsigned) mmap->type);
            }
    }

    printf("Multiboot Info Flags=%d\n", mbi->flags);

    if (CHECK_FLAG(mbi->flags, 12)) {
        void *fb = (void *) (unsigned long) mbi->framebuffer_addr;
        printf("Frame Buffer Address = %x\n", fb);
        printf("Width = %d\n", mbi->framebuffer_width);
        printf("Height = %d\n", mbi->framebuffer_height);
        printf("Type = %d\n", mbi->framebuffer_type);
    }
}


void get_cpu_info() {
    uint32_t eax, ebx, ecx, edx;
    char vendor[13];

    // Get vendor ID
    __asm__ __volatile__("cpuid"
                         : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                         : "a"(0));
    *(uint32_t*)(vendor) = ebx;
    *(uint32_t*)(vendor + 4) = edx;
    *(uint32_t*)(vendor + 8) = ecx;
    vendor[12] = '\0';
    printf("CPU Vendor: %s\n", vendor);

    // Get core and cache info if supported
    // Basic CPUID Function 1 for feature info
    __asm__ __volatile__("cpuid"
                         : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                         : "a"(1));
    
    uint32_t logical_processors = (ebx >> 16) & 0xFF;  // EBX[23:16] contains logical processor count
    printf("Logical processors (per socket): %d\n", logical_processors);

    // For Intel CPUs, use CPUID 0x4 for cache information
    if (vendor[0] == 'G' && vendor[1] == 'e' && vendor[2] == 'n' && vendor[3] == 'u' && vendor[4] == 'i' && vendor[5] == 'n' && vendor[6] == 'e' && vendor[7] == 'I' && vendor[8] == 'n' && vendor[9] == 't' && vendor[10] == 'e' && vendor[11] == 'l') {
        for (int i = 0; i < 4; i++) {  // Iterate for each cache level
            __asm__ __volatile__("cpuid"
                                 : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                                 : "a"(4), "c"(i));

            uint32_t cache_type = eax & 0x1F;
            if (cache_type == 0) break;  // No more caches if cache type is 0

            uint32_t cache_level = (eax >> 5) & 0x7;
            uint32_t ways_of_associativity = ((ebx >> 22) & 0x3FF) + 1;
            uint32_t physical_line_partitions = ((ebx >> 12) & 0x3FF) + 1;
            uint32_t line_size = (ebx & 0xFFF) + 1;
            uint32_t sets = ecx + 1;
            uint32_t cache_size = ways_of_associativity * physical_line_partitions * line_size * sets;

            printf("L%d Cache: %d KB\n", cache_level, cache_size / 1024);
        }
    }
}

void get_display_info(){

}

