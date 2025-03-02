/*
    CPU details
    Symmetric Multiprocessing
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

        printf("CPU info : \nRevision : %d\nFlags : %d\nBSP LAPIC ID : %x\nCPU count : %d\n", revision, flags, bsp_lapic_id, cpu_count);

        for(size_t i=0; i<(size_t) cpu_count; i++){
           uint32_t processor_id = cpus[i]->processor_id;
           uint32_t lapic_id = cpus[i]->lapic_id;
           uint64_t reserved = cpus[i]->reserved;
           limine_goto_address goto_address = cpus[i]->goto_address;
           uint64_t extra_argument = cpus[i]->extra_argument;

           printf("Processor ID : %d\nLAPIC ID : %x\nReserved : %x\nExtra argument : %x\n", processor_id,lapic_id, reserved, extra_argument );
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
    unsigned int eax, ebx, ecx, edx;

    // Check CPUID support (simplified)
    __asm__ volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(0));

    // Get extended CPUID support
    __asm__ volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(0x80000000));

    if (eax >= 0x80000008) {
        // Get logical processor count
        __asm__ volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(0x80000008));
        return (ebx & 0xFF); // Extract logical core count from lower 8 bits.
    }

    return 1; // Default to 1 processor if CPUID fails.
}