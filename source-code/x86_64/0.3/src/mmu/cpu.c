#include "cpu.h"

// Getting CPU information by using CPUID

extern uint64_t check_cpuid(void);

void is_cpuid_present(){
    uint64_t check_result = check_cpuid();
    if (check_result != 0) {
        print("CPUID is supported on this CPU.\n");
    } else {
        print("CPUID is not supported on this CPU.\n");
    }
}

void cpuid(uint32_t code, uint32_t* a, uint32_t* b, uint32_t* c, uint32_t* d) {
    asm volatile("cpuid" : "=a"(*a), "=b"(*b), "=c"(*c), "=d"(*d)  : "a"(code));
}

void get_cpu_vendor(char* vendor) {
    uint32_t eax, ebx, ecx, edx;
    cpuid(0x0, &eax, &ebx, &ecx, &edx);

    ((uint32_t*)vendor)[0] = ebx;  // Copy EBX to vendor string
    ((uint32_t*)vendor)[1] = edx;  // Copy EDX to vendor string
    ((uint32_t*)vendor)[2] = ecx;  // Copy ECX to vendor string
    vendor[12] = '\0';              // Null-terminate the string
}

void get_cpu_brand(char* brand) {
    uint32_t regs[4];
    for (uint32_t i = 0; i < 3; i++) {
        cpuid(0x80000002 + i, &regs[0], &regs[1], &regs[2], &regs[3]);
        memcpy(brand + i * 16, regs, 16);
    }
    brand[48] = '\0';  // Null-terminate the brand string
}

void get_cpu_features(uint32_t* family, uint32_t* model, uint32_t* features) {
    uint32_t eax, ebx, ecx, edx;
    cpuid(0x1, &eax, &ebx, &ecx, &edx);

    *family = ((eax >> 8) & 0xF) + ((eax >> 20) & 0xFF);  // Family ID
    *model = ((eax >> 4) & 0xF) + ((eax >> 16) & 0xF0);   // Model ID
    *features = edx;                                       // Feature flags
}


