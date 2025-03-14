/*
    CPU details
    Symmetric Multi Processing(SMP)
*/


#include "../limine/limine.h"
#include "../lib/stdio.h"

#include "cpu.h"

__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(3);

__attribute__((used, section(".requests")))
static volatile struct limine_smp_request smp_request = {
    .id = LIMINE_SMP_REQUEST,
    .revision = 3
};

int get_cpu_count(){
    if(smp_request.response != NULL)
        return (int) smp_request.response->cpu_count;
    return 1;
}


void get_cpu_info(){
    if(smp_request.response != NULL){
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
        }
    }else{
        printf("No CPU info found!\n");
    }
}


void print_cpu_info(){
    if(smp_request.response != NULL){
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
    }else{
        printf("No CPU info found!\n");
    }
}





void get_cpu_vendor(char *vendor) {
    int eax, ebx, ecx, edx;

    // Input EAX = 0: Get Vendor ID
    eax = 0;
    __asm__ volatile("cpuid"
                     : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                     : "a"(eax)
                     :);

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


void print_cpu_vendor() {
    char vendor[13];
    get_cpu_vendor(vendor);
    printf("CPU Vendor : %s\n", vendor);
    
}


void get_cpu_brand(char *brand) {
    int eax, ebx, ecx, edx;

    // Input EAX = 0x80000002: Get CPU Brand String (first part)
    eax = 0x80000002;
    __asm__ volatile("cpuid"
                     : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                     : "a"(eax)
                     :);
    ((int *)brand)[0] = eax;
    ((int *)brand)[1] = ebx;
    ((int *)brand)[2] = ecx;
    ((int *)brand)[3] = edx;

    // Input EAX = 0x80000003: Get CPU Brand String (second part)
    eax = 0x80000003;
    __asm__ volatile("cpuid"
                     : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                     : "a"(eax)
                     :);
    ((int *)brand)[4] = eax;
    ((int *)brand)[5] = ebx;
    ((int *)brand)[6] = ecx;
    ((int *)brand)[7] = edx;

    // Input EAX = 0x80000004: Get CPU Brand String (third part)
    eax = 0x80000004;
    __asm__ volatile("cpuid"
                     : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                     : "a"(eax)
                     :);
    ((int *)brand)[8] = eax;
    ((int *)brand)[9] = ebx;
    ((int *)brand)[10] = ecx;
    ((int *)brand)[11] = edx;

    brand[48] = '\0'; // Null-terminate the string
}



void print_cpu_brand() {
    char brand[49];
    get_cpu_brand(brand);
    printf("CPU Brand : %s\n", brand);
}



int getLogicalProcessorCount() {
    uint32_t eax, ebx, ecx, edx;
    cpuid(0x1, &eax, &ebx, &ecx, &edx);

    return (ebx >> 16) & 0xFF;  // Bits 23:16 contain logical processor count
}





static inline void cpuid(uint32_t leaf, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx) {
    __asm__ volatile ("cpuid"
                      : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
                      : "a"(leaf));
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


void enable_fpu() {
    asm volatile (
        "mov %%cr0, %%rax\n"         
        "and $~(1 << 2), %%rax\n"     // Clear CR0.EM (bit 2) to enable FPU  
        "or  $ (1 << 1), %%rax\n"     // Set CR0.MP (bit 1) for FPU emulation  
        "mov %%rax, %%cr0\n"          
        :
        :
        : "rax"
    );

    asm volatile ("fninit");  // Initialize the FPU
}


bool has_fpu() {
    uint32_t eax, ebx, ecx, edx;
    cpuid(1,  &eax, &ebx, &ecx, &edx);
    return (edx & (1 << 0));  // FPU is bit 0 of EDX
}

