
#include "gdt.h"

// Each define here is for a specific flag in the descriptor.
// Refer to the intel documentation for a description of what each one does.
#define SEG_DESCTYPE(x)  ((x) << 0x04) // Descriptor type (0 for system, 1 for code/data)
#define SEG_PRES(x)      ((x) << 0x07) // Present
#define SEG_SAVL(x)      ((x) << 0x0C) // Available for system use
#define SEG_LONG(x)      ((x) << 0x0D) // Long mode
#define SEG_SIZE(x)      ((x) << 0x0E) // Size (0 for 16-bit, 1 for 32)
#define SEG_GRAN(x)      ((x) << 0x0F) // Granularity (0 for 1B - 1MB, 1 for 4KB - 4GB)
#define SEG_PRIV(x)     (((x) &  0x03) << 0x05)   // Set privilege level (0 - 3)
 
#define SEG_DATA_RD        0x00 // Read-Only
#define SEG_DATA_RDA       0x01 // Read-Only, accessed
#define SEG_DATA_RDWR      0x02 // Read/Write
#define SEG_DATA_RDWRA     0x03 // Read/Write, accessed
#define SEG_DATA_RDEXPD    0x04 // Read-Only, expand-down
#define SEG_DATA_RDEXPDA   0x05 // Read-Only, expand-down, accessed
#define SEG_DATA_RDWREXPD  0x06 // Read/Write, expand-down
#define SEG_DATA_RDWREXPDA 0x07 // Read/Write, expand-down, accessed
#define SEG_CODE_EX        0x08 // Execute-Only
#define SEG_CODE_EXA       0x09 // Execute-Only, accessed
#define SEG_CODE_EXRD      0x0A // Execute/Read
#define SEG_CODE_EXRDA     0x0B // Execute/Read, accessed
#define SEG_CODE_EXC       0x0C // Execute-Only, conforming
#define SEG_CODE_EXCA      0x0D // Execute-Only, conforming, accessed
#define SEG_CODE_EXRDC     0x0E // Execute/Read, conforming
#define SEG_CODE_EXRDCA    0x0F // Execute/Read, conforming, accessed
 
#define GDT_CODE_PL0 SEG_DESCTYPE(1) | SEG_PRES(1) | SEG_SAVL(0) | \
                     SEG_LONG(0)     | SEG_SIZE(1) | SEG_GRAN(1) | \
                     SEG_PRIV(0)     | SEG_CODE_EXRD
 
#define GDT_DATA_PL0 SEG_DESCTYPE(1) | SEG_PRES(1) | SEG_SAVL(0) | \
                     SEG_LONG(0)     | SEG_SIZE(1) | SEG_GRAN(1) | \
                     SEG_PRIV(0)     | SEG_DATA_RDWR
 
#define GDT_CODE_PL3 SEG_DESCTYPE(1) | SEG_PRES(1) | SEG_SAVL(0) | \
                     SEG_LONG(0)     | SEG_SIZE(1) | SEG_GRAN(1) | \
                     SEG_PRIV(3)     | SEG_CODE_EXRD
 
#define GDT_DATA_PL3 SEG_DESCTYPE(1) | SEG_PRES(1) | SEG_SAVL(0) | \
                     SEG_LONG(0)     | SEG_SIZE(1) | SEG_GRAN(1) | \
                     SEG_PRIV(3)     | SEG_DATA_RDWR


extern void setGdt(uint16_t, uint64_t);
extern void reloadSegments();

gdt_entry_t gdt_entries[5];



void gdt_set_gate(int8_t index, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags){
    gdt_entries[index].limit_low = limit & 0xFFFF;   // set lower 16 bit
    gdt_entries[index].limit_up = (limit >> 16) & 0xF;   // set upper 4 bit
    
    gdt_entries[index].base_low = base & 0xFFFF;     // set lower 16 bit
    gdt_entries[index].base_mid = (base >> 16) & 0xFF;    // set middle 8 bit
    gdt_entries[index].base_up = (base >> 24) & 0xFF;     // set upper 8 bit

    gdt_entries[index].access = access;      // set 8 bit                                    G DB L R                      G DB L R
    gdt_entries[index].flags = flags;        // set flag =G DB L 0 => 0 00 If flags = 0xA  = 1 0  1 0 = 64 BIT CODE, 0XC = 1 1  0 0
}


void init_gdt(){
    gdt_set_gate(0,0,0x00000,0x00,0x0); // null descriptor selector : 0x0

    gdt_set_gate(1,0,0xFFFFF, 0x9A, 0xA); // kernel mode code segment, selector : 0x8
    gdt_set_gate(2,0,0xFFFFF,0x92,0xC); // kernel mode data segment, selector : 0x10

    gdt_set_gate(3,0,0xFFFFF,0xFA,0xA); // user mode code segment, selector : 0x18
    gdt_set_gate(4,0,0xFFFFF,0xF2,0xC); // user mode data segment

    // Calculate the GDT limit and base address
    uint16_t limit = (uint16_t) (sizeof(gdt_entry_t) * 5 - 1);
    uint64_t base = (uint64_t) &gdt_entries;
    
    setGdt(limit, base);
    reloadSegments();
}


void check_gdt() {
    uint16_t cs, ds, es, fs, gs, ss;

    // Inline assembly to get the values of segment registers
    asm volatile("mov %%cs, %0" : "=r"(cs));    
    asm volatile("mov %%ds, %0" : "=r"(ds));
    asm volatile("mov %%es, %0" : "=r"(es));
    asm volatile("mov %%fs, %0" : "=r"(fs));
    asm volatile("mov %%gs, %0" : "=r"(gs));
    asm volatile("mov %%ss, %0" : "=r"(ss));

    // Print the segment register values
    print("GDT Set\n");
    print("Segment Registers Values :\n");

    print("CS: ");
    print_hex(cs);
    print("\n");

    print("ES: ");
    print_hex(es);
    print("\n");

    print("FS: ");
    print_hex(fs);
    print("\n");

    print("GS: ");
    print_hex(gs);
    print("\n");

    print("SS: ");
    print_hex(ss);
    print("\n");
}

