
#include "gdt.h"


// Lets us access our ASM functions from our C code.
extern void gdt_flush(uint32_t);

// Internal function prototypes.
void init_gdt();
void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);

gdt_entry_t gdt_entries[6]; // one null segment, two ring 0 segments, two ring 3 segments, TSS segment
// (ring 0 segments)
gdt_ptr_t   gdt_ptr;

extern void flush_tss(void);
void write_tss(gdt_entry_t *g);
extern void * jump_usermode(void);


void init_gdt()
{
   gdt_ptr.limit = (sizeof(gdt_entry_t) * 5) - 1;
   gdt_ptr.base  = (uint32_t) &gdt_entries;

   gdt_set_gate(0, 0, 0x00000000, 0x00, 0x00); // Null segment
   gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Kernel Code segment
   gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Kernel Data segment
   gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // User mode code segment
   gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // User mode data segment
   //gdt_set_gate(5, 0, 0xFFFFFFFF, 0, 0); // TSS

   write_tss(&gdt_entries[5]);

   gdt_flush((uint32_t) &gdt_ptr);
   flush_tss();
   jump_usermode();
}


// Set the value of one GDT entry. cf = 11001111
void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity) {
   gdt_entries[num].base_low = (base & 0xFFFFFF);        // Lower 24 bits of base
   gdt_entries[num].base_high = (base >> 24) & 0xFF;     // Upper 8 bits of base

   gdt_entries[num].limit_low = (limit & 0xFFFF);        // Lower 16 bits of limit
   gdt_entries[num].limit_high = (limit >> 16) & 0x0F;   // Upper 4 bits of limit

   gdt_entries[num].access = (access & 0x01);          // Accessed bit
   gdt_entries[num].read_write = (access >> 1) & 0x01;   // Read/write bit
   gdt_entries[num].conforming_expand_down = (access >> 2) & 0x01;
   gdt_entries[num].code = (access >> 3) & 0x01;         // Code bit
   gdt_entries[num].code_data_segment = (access >> 4) & 0x01;
   gdt_entries[num].DPL = (access >> 5) & 0x03;          // DPL bits
   gdt_entries[num].present = (access >> 7) & 0x01;      // Present bit

   gdt_entries[num].available = (granularity >> 4) & 0x01;
   gdt_entries[num].long_mode = (granularity >> 5) & 0x01;
   gdt_entries[num].big = (granularity >> 6) & 0x01;     // Big bit
   gdt_entries[num].gran = (granularity >> 7) & 0x01;    // Granularity bit
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
    print(" cs : ");
    print_hex(cs);
    print("\n");
    print(" ds : ");
    print_hex(ds);
    print("\n");
    print(" es : ");
    print_hex(es);
    print("\n");
    print(" fs : ");
    print_hex( fs);
    print("\n");
    print(" gs : ");
    print_hex(gs);
    print("\n");
    print(" ss : ");
    print_hex(ss);
    print("\n");
}


// Note: some of the GDT entry struct field names may not match perfectly to the TSS entries.
tss_entry_t tss_entry;



void write_tss(gdt_entry_t *g) {
	// Compute the base and limit of the TSS for use in the GDT entry.
	uint32_t base = (uint32_t) &tss_entry;
	uint32_t limit = sizeof tss_entry;

	// Add a TSS descriptor to the GDT.
	g->limit_low = limit;
	g->base_low = base;
	g->access   = 1; // With a system entry (`code_data_segment` = 0), 1 indicates TSS and 0 indicates LDT
	g->read_write = 0; // For a TSS, indicates busy (1) or not busy (0).
	g->conforming_expand_down = 0; // always 0 for TSS
	g->code = 1; // For a TSS, 1 indicates 32-bit (1) or 16-bit (0).
	g->code_data_segment=0; // indicates TSS/LDT (see also `accessed`)
	g->DPL = 0; // ring 0, see the comments below
	g->present = 1;
	g->limit_high = (limit & (0xf << 16)) >> 16; // isolate top nibble
	g->available = 0; // 0 for a TSS
	g->long_mode = 0;
	g->big = 0; // should leave zero according to manuals.
	g->gran = 0; // limit is in bytes, not pages
	g->base_high = (base & (0xff << 24)) >> 24; //isolate top byte

	// Ensure the TSS is initially zero'd.
	memset(&tss_entry, 0, sizeof tss_entry);

	tss_entry.ss0  = 0x10; //REPLACE_KERNEL_DATA_SEGMENT;  // Set the kernel stack segment.
   #define KERNEL_STACK_SIZE 4096*4
   uint8_t kernel_stack[KERNEL_STACK_SIZE];

   tss_entry.esp0 = (uint32_t)&kernel_stack + KERNEL_STACK_SIZE;  // Set esp0 to the top of the stack

	//tss_entry.esp0 = REPLACE_KERNEL_STACK_ADDRESS; // Set the kernel stack pointer.
	//note that CS is loaded from the IDT entry and should be the regular kernel code segment
}

extern void * jump_usermode(void);
void set_kernel_stack(uint32_t stack) { // Used when an interrupt occurs
	tss_entry.esp0 = stack;
}