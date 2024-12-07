
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

/* Example: Get CPU's model number */
int get_model(void)
{
    int ebx, unused;
    __cpuid(0, unused, ebx, unused, unused);
    return ebx;
}

/* Example: Check for builtin local APIC. */
int check_apic(void)
{
    unsigned int eax, unused, edx;
    __get_cpuid(1, &eax, &unused, &unused, &edx);
    return edx & CPUID_FEAT_EDX_APIC;
}



#define CPUID_FEAT_EDX_APIC (1 << 9) // APIC bit in EDX for CPUID function 0x1

void get_cpu_info() {
    unsigned int eax, ebx, ecx, edx;

    // Get Vendor ID (Function ID 0)
    __get_cpuid(0, &eax, &ebx, &ecx, &edx);
    
 
    // Convert registers to string
    char* vendor = (char*) registers_to_string(ebx, edx, ecx);

    // Print the result
    print( vendor);
    print("\n");

    // Get CPU model, family, and stepping (Function ID 1)
    /*if (__get_cpuid(1, &eax, &ebx, &ecx, &edx)) {
        unsigned int stepping = eax & 0xF;            // Bits 0-3
        unsigned int model = (eax >> 4) & 0xF;        // Bits 4-7
        unsigned int family = (eax >> 8) & 0xF;       // Bits 8-11
        unsigned int extended_model = (eax >> 16) & 0xF;  // Bits 16-19
        unsigned int extended_family = (eax >> 20) & 0xFF; // Bits 20-27

        // Adjust model and family based on Intel's extended information
        if (family == 0xF) {
            family += extended_family;
        }
        if (family == 0x6 || family == 0xF) {
            model += (extended_model << 4);
        }

        printf("CPU Family: %u, Model: %u, Stepping: %u\n", family, model, stepping);

        // Check for APIC support
        int apic_supported = (edx & CPUID_FEAT_EDX_APIC) != 0;
        printf("APIC Supported: %s\n", apic_supported ? "Yes" : "No");
    }*/
}




char* registers_to_string(unsigned int ebx, unsigned int edx, unsigned int ecx) {
    // Allocate space for a string of 12 characters + null terminator
    static char result[13];  // 12 characters + 1 for '\0'

    // Extract characters from the EBX register
    result[0] = (ebx & 0xFF);         // Lower byte of EBX ('u')
    result[1] = (ebx >> 8) & 0xFF;    // Second byte of EBX ('n')
    result[2] = (ebx >> 16) & 0xFF;   // Third byte of EBX ('e')
    result[3] = (ebx >> 24) & 0xFF;   // Fourth byte of EBX ('G')

    // Extract characters from the EDX register
    result[4] = (edx & 0xFF);         // Lower byte of EDX ('I')
    result[5] = (edx >> 8) & 0xFF;    // Second byte of EDX ('e')
    result[6] = (edx >> 16) & 0xFF;   // Third byte of EDX ('n')
    result[7] = (edx >> 24) & 0xFF;   // Fourth byte of EDX ('i')

    // Extract characters from the ECX register
    result[8] = (ecx & 0xFF);         // Lower byte of ECX ('l')
    result[9] = (ecx >> 8) & 0xFF;    // Second byte of ECX ('e')
    result[10] = (ecx >> 16) & 0xFF;  // Third byte of ECX ('t')
    result[11] = (ecx >> 24) & 0xFF;  // Fourth byte of ECX ('n')

    // Null-terminate the string
    result[12] = '\0';

    return result;
}


