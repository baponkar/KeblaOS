/*
    CPU details
    Symmetric Multi Processing(SMP)

    https://github.com/limine-bootloader/limine/blob/v9.x/PROTOCOL.md
    https://wiki.osdev.org/CPUID
*/

#include <cpuid.h>

#include "../limine/limine.h"
#include "../lib/stdio.h"
#include "../driver/keyboard/keyboard.h"
#include "../x86_64/interrupt/apic.h"
#include "../x86_64/interrupt/interrupt.h"
#include "../x86_64/gdt/gdt.h"
#include "../x86_64/timer/tsc.h"
#include "../x86_64/timer/apic_timer.h"
#include "../x86_64/interrupt/apic.h"
#include "../memory/kmalloc.h"
#include "../util/util.h"
#include "../acpi/acpi.h"
#include "../acpi/descriptor_table/madt.h"

#include "cpu.h"


__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(3);

__attribute__((used, section(".requests")))
static volatile struct limine_smp_request smp_request = {
    .id = LIMINE_SMP_REQUEST,
    .revision = 3
};



extern uint32_t is_cpuid_present(void);


// Getting CPU information using CPUID instruction

static inline void cpuid(uint32_t leaf, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx) {
    __asm__ volatile ("cpuid" : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx) : "a"(leaf));
}

int getLogicalProcessorCount() {
    uint32_t eax, ebx, ecx, edx;
    cpuid(0x1, &eax, &ebx, &ecx, &edx);

    return (ebx >> 16) & 0xFF;  // Bits 23:16 contain logical processor count
}

void get_cpu_vendor(char *vendor) {
    int eax, ebx, ecx, edx;

    // Input EAX = 0: Get Vendor ID
    eax = 0;
    __asm__ volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(eax) :);

    // EBX, EDX, ECX contain the vendor ID string
    ((int *)vendor)[0] = ebx;
    ((int *)vendor)[1] = edx;
    ((int *)vendor)[2] = ecx;
    vendor[12] = '\0'; // Null-terminate the string

    // Check specific feature bits
    if (edx & (1 << 25)) {
        printf("SSE supported\n");
    }
    if (ecx & (1 << 28)) {
        printf("AVX supported\n");
    }
}

void get_cpu_brand(char *brand) {
    int eax, ebx, ecx, edx;

    // Input EAX = 0x80000002: Get CPU Brand String (first part)
    eax = 0x80000002;
    __asm__ volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(eax):);
    ((int *)brand)[0] = eax;
    ((int *)brand)[1] = ebx;
    ((int *)brand)[2] = ecx;
    ((int *)brand)[3] = edx;

    // Input EAX = 0x80000003: Get CPU Brand String (second part)
    eax = 0x80000003;
    __asm__ volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(eax) :);
    ((int *)brand)[4] = eax;
    ((int *)brand)[5] = ebx;
    ((int *)brand)[6] = ecx;
    ((int *)brand)[7] = edx;

    // Input EAX = 0x80000004: Get CPU Brand String (third part)
    eax = 0x80000004;
    __asm__ volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(eax) :);
    ((int *)brand)[8] = eax;
    ((int *)brand)[9] = ebx;
    ((int *)brand)[10] = ecx;
    ((int *)brand)[11] = edx;

    brand[48] = '\0'; // Null-terminate the string
}


uint32_t get_cpu_base_frequency() {
    uint32_t max_cpuid_leaf, eax, ebx, ecx, edx;

    // Get the maximum supported CPUID leaf
    cpuid(0x0, &max_cpuid_leaf, &ebx, &ecx, &edx);

    // Check if 0x16 is supported
    if (max_cpuid_leaf < 0x16) {
        return 0;  // CPUID 0x16 is not available
    }

    // Call CPUID 0x16
    cpuid(0x16, &eax, &ebx, &ecx, &edx);

    if (eax == 0) {
        return 0;  // CPU does not report frequency
    }

    return eax * 1000000ULL;  // Convert MHz to Hz
}


uint32_t get_lapic_id_cpuid(void) {
    uint32_t eax, ebx, ecx, edx;
    __asm__ __volatile__(
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(1)
    );
    // Extract LAPIC ID from EBX bits 24-31
    return (ebx >> 24) & 0xFF;
}


bool has_fpu() {
    uint32_t eax, ebx, ecx, edx;
    cpuid(1,  &eax, &ebx, &ecx, &edx);
    return (edx & (1 << 0));  // FPU is bit 0 of EDX
}


void enable_fpu_and_sse() {
    asm volatile (
        "mov %%cr0, %%rax\n"      
        "and $-5, %%rax\n"        // Clear CR0.EM (bit 2) to enable FPU
        "or  $2, %%rax\n"         // Set CR0.MP (bit 1) for proper FPU operation
        "and $-9, %%rax\n"        // Clear CR0.TS (bit 3); -9 clears bit 3 (0xFFFFFFF7)
        "mov %%rax, %%cr0\n"
        :
        :
        : "rax"
    );

    asm volatile (
        "mov %%cr4, %%rax\n"
        "or  $0x600, %%rax\n"     // Set CR4.OSFXSR (bit 9) and CR4.OSXMMEXCPT (bit 10) to enable SSE
        "mov %%rax, %%cr4\n"
        :
        :
        : "rax"
    );

    asm volatile ("fninit");  // Initialize the FPU
}



// ---------------------------------------------- //

// Define stack size and storage
#define MAX_CORES 16
#define STACK_SIZE 4096 * 4  // 16 KB per core
static uint8_t *ap_stacks[MAX_CORES];



void init_cpu(){
    get_cpu_info();
    print_cpu_info();

    ap_stacks[0] = (uint8_t *) read_rsp();
}

void init_ap_stacks(int start_id, int end_id) {
    for (int i = start_id; i < end_id; i++) {
        ap_stacks[i] = (uint8_t *) kmalloc_a(STACK_SIZE, 1);
        if (!ap_stacks[i]) {
            printf("Failed to allocate AP stack\n");
        }
    }
}


// Getting CPU information using Limine Bootloader

int get_cpu_count(){
    if(smp_request.response != NULL)  return (int) smp_request.response->cpu_count;
    return 1; // Defult one processor count
}


void get_cpu_info(){
    if(!smp_request.response) printf("No CPU info found!\n");

    uint64_t revision = smp_request.response->revision;
    uint32_t flags = smp_request.response->flags;
    uint32_t bsp_lapic_id = smp_request.response->bsp_lapic_id;
    uint64_t cpu_count = smp_request.response->cpu_count;
    struct limine_smp_info ** cpus = smp_request.response->cpus;

    for(size_t i=0; i<(size_t) cpu_count; i++){
        uint32_t processor_id = cpus[i]->processor_id;
        uint32_t lapic_id = cpus[i]->lapic_id;
        uint64_t reserved = cpus[i]->reserved;
        limine_goto_address goto_address = cpus[i]->goto_address;
        uint64_t extra_argument = cpus[i]->extra_argument;

        // printf("LAPIC ID: %d\n", lapic_id);
    }
}


void target_cpu_task(struct limine_smp_info *smp_info) {
    printf("Now running on CPU with LAPIC ID: %d\n", get_lapic_id());

    int core_id = (int) smp_info->lapic_id;

    uint8_t *stack_top = ap_stacks[core_id] + STACK_SIZE;

    // Set the stack pointer
    asm volatile (
        "mov %0, %%rsp\n"
        :
        : "r"(stack_top)
    );

    init_acpi();
    
    // Initialize GDT and TSS for this core
    init_core_gdt_tss(core_id);

    // Initialize interrupts for this core
    init_core_interrupt(core_id);

    // Do not need to initialize TSC for application core
    // init_tsc();

    // Initialize APIC for this 
    init_apic_interrupt();

    // Configure APIC timer for this core
    init_apic_timer(150); // 150 ms interval

    ioapic_route_irq(1, core_id, 0x21);
    initKeyboard();

    printf("CPU %d: APIC Timer initialized!\n", core_id);

    // Enable interrupts for this core
    asm volatile("sti");

    // Enter ap core loop or wait for tasks
    while (1) {
        asm volatile("hlt");
    }
}


void switch_to_core(uint32_t target_lapic_id) {
    if (smp_request.response == NULL) return;

    for (size_t i = 0; i < smp_request.response->cpu_count; i++) {
        struct limine_smp_info *smp_info = smp_request.response->cpus[i];
        if (smp_info->lapic_id == target_lapic_id) {
            // Send IPI to the target core
            lapic_send_ipi(target_lapic_id, 0x40); // 0x20 is the vector
            break;
        }
    }
    printf("Successfully Switched  into CPU %d\n", target_lapic_id);
}



void start_bootstrap_cpu_core() {
    uint32_t bsp_lapic_id = smp_request.response->bsp_lapic_id;
    asm volatile("cli");
    init_acpi();

    init_bootstrap_gdt_tss(bsp_lapic_id);
    init_bootstrap_interrupt(bsp_lapic_id);

    init_tsc();

    init_apic_interrupt();
    
    init_apic_timer(100);

    ioapic_route_irq(1, 0, 0x21); // Route IRQ 1 to LAPIC ID 0 with vector 0x20
    initKeyboard();

    asm volatile("sti");
    printf("Bootstrap CPU initialized...\n\n");
}



void start_secondary_cpu_cores(int start_id, int end_id) {
    for (int core = start_id; core < smp_request.response->cpu_count; core++) {
        struct limine_smp_info *smp_info = smp_request.response->cpus[core];

        // Set function to execute on the AP
        smp_info->goto_address = (limine_goto_address) target_cpu_task;

        // passing sm_info as argument which will accept as input by target_cpu_task
        smp_info->extra_argument = (uint64_t)smp_info;

        // Short delay to let APs start (only for debugging)
        for (volatile int i = 0; i < 1000000; i++);

        printf("Initializing CPU %d (LAPIC ID %d)...\n\n", core, smp_info->lapic_id);
    }
}



// Debugging
void print_cpu_vendor() {
    char vendor[13];
    get_cpu_vendor(vendor);
    printf("CPU Vendor : %s\n", vendor);
}

void print_cpu_brand() {
    char brand[49];
    get_cpu_brand(brand);
    printf("CPU Brand : %s\n", brand);
}


void print_cpu_info(){
    if(!smp_request.response) printf("No CPU info found!\n");
    
    uint64_t revision = smp_request.response->revision;
    uint32_t flags = smp_request.response->flags;
    uint32_t bsp_lapic_id = smp_request.response->bsp_lapic_id;
    uint64_t cpu_count = smp_request.response->cpu_count;
    struct limine_smp_info ** cpus = smp_request.response->cpus;

    printf("CPU info : \nRevision : %d, Flags : %d, BSP LAPIC ID : %d, CPU count : %d\n", revision, flags, bsp_lapic_id, cpu_count);

    for(size_t i=0; i<(size_t) cpu_count; i++){
        uint32_t processor_id = cpus[i]->processor_id;
        uint32_t lapic_id = cpus[i]->lapic_id;
        uint64_t reserved = cpus[i]->reserved;
        limine_goto_address goto_address = cpus[i]->goto_address;
        uint64_t extra_argument = cpus[i]->extra_argument;

        printf("Processor ID : %d, LAPIC ID : %d, Reserved : %x, Extra argument : %x\n", processor_id,lapic_id, reserved, extra_argument );
    }
}







