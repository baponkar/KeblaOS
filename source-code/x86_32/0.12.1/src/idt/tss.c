#include "tss.h"

struct tss_entry {
    uint32_t prev_tss;   // Previous TSS - unused now, set to 0
    uint32_t esp0;       // Stack pointer to load when changing to ring 0
    uint32_t ss0;        // Stack segment to load when changing to ring 0
    uint32_t esp1;       // Stack pointer to load when changing to ring 1 (unused)
    uint32_t ss1;        // Stack segment to load when changing to ring 1 (unused)
    uint32_t esp2;       // Stack pointer to load when changing to ring 2 (unused)
    uint32_t ss2;        // Stack segment to load when changing to ring 2 (unused)
    uint32_t cr3;        // Page directory base register
    uint32_t eip;        // Instruction pointer
    uint32_t eflags;     // Flags register
    uint32_t eax, ecx, edx, ebx, esp, ebp, esi, edi;  // General purpose registers
    uint32_t es, cs, ss, ds, fs, gs;  // Segment selectors
    uint32_t ldt;        // Local Descriptor Table segment selector
    uint16_t trap;       // Trap on task switch
    uint16_t iomap_base; // I/O map base address
} __attribute__((packed));


struct tss_entry tss;

void write_tss(int32_t num, uint16_t ss0, uint32_t esp0) {
    uint32_t base = (uint32_t)&tss;
    uint32_t limit = base + sizeof(tss);

    // Add a TSS descriptor to the GDT
    gdt_set_gate(num, base, limit, 0xE9, 0x00);

    // Initialize the TSS entry
    memset(&tss, 0, sizeof(tss));
    tss.ss0  = ss0;    // Kernel data segment
    tss.esp0 = esp0;   // Kernel stack pointer
    tss.cs   = 0x0B;   // Code segment (user mode)
    tss.ss   = 0x13;   // Stack segment (user mode)
    tss.ds   = 0x13;   // Data segment (user mode)
    tss.es   = 0x13;
    tss.fs   = 0x13;
    tss.gs   = 0x13;

    // Load the TSS descriptor into the TR register
    asm volatile("ltr %0" : : "r"((uint16_t)(num << 3)));
}


void init_tss() {
    write_tss(5, 0x10, 0x0);  // TSS entry number in GDT (assume it's the 5th entry)
}


void switch_to_user_mode() {
    // Set up a stack for user mode
    asm volatile (
        "cli;"                       // Disable interrupts
        "mov $0x23, %%ax;"           // User data segment (GDT entry 4)
        "mov %%ax, %%ds;"            // Load user mode segments
        "mov %%ax, %%es;"
        "mov %%ax, %%fs;"
        "mov %%ax, %%gs;"

        "mov %%esp, %%eax;"          // Save the current ESP (kernel stack)
        "push $0x23;"                // Push user data segment selector (SS = 0x23)
        "push %%eax;"                // Push user mode stack pointer
        "pushf;"                     // Push EFLAGS
        "push $0x1B;"                // Push user code segment selector (CS = 0x1B)
        "push $1f;"                  // Push the instruction pointer (IP) to jump to
        "iret;"                      // Return from interrupt, switch to user mode
        "1:;"
        "mov $0x23, %%ax;"           // Reload the user data segments
        "mov %%ax, %%ds;"
        "mov %%ax, %%es;"
        "mov %%ax, %%fs;"
        "mov %%ax, %%gs;"
        "sti;"                       // Re-enable interrupts
        :
        :
        : "eax"
    );
}
