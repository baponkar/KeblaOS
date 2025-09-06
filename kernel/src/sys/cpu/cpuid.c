/*
CPUID

The CPUID instruction is used to determine the processor type and the presence of specific features. 
The CPUID instruction is a processor supplementary instruction (its opcode is 0Fh A2). 
When the CPUID instruction is executed, the processor returns the following information in the EAX, 
EBX, ECX, and EDX registers:
    EAX: The highest value of the CPUID function supported by the processor.
    EBX, EDX, ECX: Values that depend on the value in EAX.

    https://wiki.osdev.org/CPUID
*/


#include <cpuid.h>
#include "../../lib/stdio.h"

#include "cpuid.h"

// Vendor strings from CPUs.
#define CPUID_VENDOR_AMD           "AuthenticAMD"
#define CPUID_VENDOR_AMD_OLD       "AMDisbetter!" // Early engineering samples of AMD K5 processor
#define CPUID_VENDOR_INTEL         "GenuineIntel"
#define CPUID_VENDOR_VIA           "VIA VIA VIA "
#define CPUID_VENDOR_TRANSMETA     "GenuineTMx86"
#define CPUID_VENDOR_TRANSMETA_OLD "TransmetaCPU"
#define CPUID_VENDOR_CYRIX         "CyrixInstead"
#define CPUID_VENDOR_CENTAUR       "CentaurHauls"
#define CPUID_VENDOR_NEXGEN        "NexGenDriven"
#define CPUID_VENDOR_UMC           "UMC UMC UMC "
#define CPUID_VENDOR_SIS           "SiS SiS SiS "
#define CPUID_VENDOR_NSC           "Geode by NSC"
#define CPUID_VENDOR_RISE          "RiseRiseRise"
#define CPUID_VENDOR_VORTEX        "Vortex86 SoC"
#define CPUID_VENDOR_AO486         "MiSTer AO486"
#define CPUID_VENDOR_AO486_OLD     "GenuineAO486"
#define CPUID_VENDOR_ZHAOXIN       "  Shanghai  "
#define CPUID_VENDOR_HYGON         "HygonGenuine"
#define CPUID_VENDOR_ELBRUS        "E2K MACHINE "
 
// Vendor strings from hypervisors.
#define CPUID_VENDOR_QEMU          "TCGTCGTCGTCG"
#define CPUID_VENDOR_KVM           " KVMKVMKVM  "
#define CPUID_VENDOR_VMWARE        "VMwareVMware"
#define CPUID_VENDOR_VIRTUALBOX    "VBoxVBoxVBox"
#define CPUID_VENDOR_XEN           "XenVMMXenVMM"
#define CPUID_VENDOR_HYPERV        "Microsoft Hv"
#define CPUID_VENDOR_PARALLELS     " prl hyperv "
#define CPUID_VENDOR_PARALLELS_ALT " lrpepyh vr " // Sometimes Parallels incorrectly encodes "prl hyperv" as "lrpepyh vr" due to an endianness mismatch.
#define CPUID_VENDOR_BHYVE         "bhyve bhyve "
#define CPUID_VENDOR_QNX           " QNXQVMBSQG "

enum cpu_feature{
    CPUID_FEAT_ECX_SSE3         = 1 << 0,
    CPUID_FEAT_ECX_PCLMUL       = 1 << 1,
    CPUID_FEAT_ECX_DTES64       = 1 << 2,
    CPUID_FEAT_ECX_MONITOR      = 1 << 3,
    CPUID_FEAT_ECX_DS_CPL       = 1 << 4,
    CPUID_FEAT_ECX_VMX          = 1 << 5,
    CPUID_FEAT_ECX_SMX          = 1 << 6,
    CPUID_FEAT_ECX_EST          = 1 << 7,
    CPUID_FEAT_ECX_TM2          = 1 << 8,
    CPUID_FEAT_ECX_SSSE3        = 1 << 9,
    CPUID_FEAT_ECX_CID          = 1 << 10,
    CPUID_FEAT_ECX_SDBG         = 1 << 11,
    CPUID_FEAT_ECX_FMA          = 1 << 12,
    CPUID_FEAT_ECX_CX16         = 1 << 13,
    CPUID_FEAT_ECX_XTPR         = 1 << 14,
    CPUID_FEAT_ECX_PDCM         = 1 << 15,
    CPUID_FEAT_ECX_PCID         = 1 << 17,
    CPUID_FEAT_ECX_DCA          = 1 << 18,
    CPUID_FEAT_ECX_SSE4_1       = 1 << 19,
    CPUID_FEAT_ECX_SSE4_2       = 1 << 20,
    CPUID_FEAT_ECX_X2APIC       = 1 << 21,
    CPUID_FEAT_ECX_MOVBE        = 1 << 22,
    CPUID_FEAT_ECX_POPCNT       = 1 << 23,
    CPUID_FEAT_ECX_TSC          = 1 << 24,
    CPUID_FEAT_ECX_AES          = 1 << 25,
    CPUID_FEAT_ECX_XSAVE        = 1 << 26,
    CPUID_FEAT_ECX_OSXSAVE      = 1 << 27,
    CPUID_FEAT_ECX_AVX          = 1 << 28,
    CPUID_FEAT_ECX_F16C         = 1 << 29,
    CPUID_FEAT_ECX_RDRAND       = 1 << 30,
    CPUID_FEAT_ECX_HYPERVISOR   = 1 << 31,

    CPUID_FEAT_EDX_FPU          = 1 << 0,
    CPUID_FEAT_EDX_VME          = 1 << 1,
    CPUID_FEAT_EDX_DE           = 1 << 2,
    CPUID_FEAT_EDX_PSE          = 1 << 3,
    CPUID_FEAT_EDX_TSC          = 1 << 4,
    CPUID_FEAT_EDX_MSR          = 1 << 5,
    CPUID_FEAT_EDX_PAE          = 1 << 6,
    CPUID_FEAT_EDX_MCE          = 1 << 7,
    CPUID_FEAT_EDX_CX8          = 1 << 8,
    CPUID_FEAT_EDX_APIC         = 1 << 9,
    CPUID_FEAT_EDX_SEP          = 1 << 11,
    CPUID_FEAT_EDX_MTRR         = 1 << 12,
    CPUID_FEAT_EDX_PGE          = 1 << 13,
    CPUID_FEAT_EDX_MCA          = 1 << 14,
    CPUID_FEAT_EDX_CMOV         = 1 << 15,
    CPUID_FEAT_EDX_PAT          = 1 << 16,
    CPUID_FEAT_EDX_PSE36        = 1 << 17,
    CPUID_FEAT_EDX_PSN          = 1 << 18,
    CPUID_FEAT_EDX_CLFLUSH      = 1 << 19,
    CPUID_FEAT_EDX_DS           = 1 << 21,
    CPUID_FEAT_EDX_ACPI         = 1 << 22,
    CPUID_FEAT_EDX_MMX          = 1 << 23,
    CPUID_FEAT_EDX_FXSR         = 1 << 24,
    CPUID_FEAT_EDX_SSE          = 1 << 25,
    CPUID_FEAT_EDX_SSE2         = 1 << 26,
    CPUID_FEAT_EDX_SS           = 1 << 27,
    CPUID_FEAT_EDX_HTT          = 1 << 28,
    CPUID_FEAT_EDX_TM           = 1 << 29,
    CPUID_FEAT_EDX_IA64         = 1 << 30,
    CPUID_FEAT_EDX_PBE          = 1 << 31
};


extern uint32_t is_cpuid_present(void); // defined in cpuid.asm to check if CPUID instruction is present

static inline void cpuid(uint32_t leaf, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx) {
    asm volatile ("cpuid" : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx) : "a"(leaf));
}


// Getting Logical Processor Count by using CPUID instruction
int getLogicalProcessorCount() {
    uint32_t eax, ebx, ecx, edx;
    cpuid(0x1, &eax, &ebx, &ecx, &edx);

    return (ebx >> 16) & 0xFF;  // Bits 23:16 contain logical processor count
}

void get_cpu_vendor(char *vendor) {
    int eax, ebx, ecx, edx;

    cpuid(0x0, &eax, &ebx, &ecx, &edx);

    // EBX, EDX, ECX contain the vendor ID string
    ((int *)vendor)[0] = ebx;
    ((int *)vendor)[1] = edx;
    ((int *)vendor)[2] = ecx;
    ((int *)vendor)[12] = '\0'; // Null-terminate the string
}

void get_cpu_brand(char *brand) {
    int eax, ebx, ecx, edx;

    // Input EAX = 0x80000002: Get CPU Brand String (first part)
    eax = 0x80000002;
    cpuid(eax, &eax, &ebx, &ecx, &edx);
    ((int *)brand)[0] = eax;
    ((int *)brand)[1] = ebx;
    ((int *)brand)[2] = ecx;
    ((int *)brand)[3] = edx;

    // Input EAX = 0x80000003: Get CPU Brand String (second part)
    eax = 0x80000003;
    cpuid(eax, &eax, &ebx, &ecx, &edx);
    ((int *)brand)[4] = eax;
    ((int *)brand)[5] = ebx;
    ((int *)brand)[6] = ecx;
    ((int *)brand)[7] = edx;

    // Input EAX = 0x80000004: Get CPU Brand String (third part)
    eax = 0x80000004;
    cpuid(eax, &eax, &ebx, &ecx, &edx);
    ((int *)brand)[8] = eax;
    ((int *)brand)[9] = ebx;
    ((int *)brand)[10] = ecx;
    ((int *)brand)[11] = edx;

    brand[48] = '\0'; // Null-terminate the string
}



uint64_t get_cpu_base_frequency() {
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

uint32_t get_lapic_id_by_cpuid() {
    uint32_t eax, ebx, ecx, edx;
    cpuid(1, &eax, &ebx, &ecx, &edx);
    // Extract LAPIC ID from EBX bits 24-31
    return (ebx >> 24) & 0xFF;
}

bool has_apic() {
    uint32_t eax, ebx, ecx, edx;
    cpuid(1, &eax, &ebx, &ecx, &edx);
    return (edx & (1 << 9));  // APIC is bit 9 of EDX
}

bool has_sse() {
    uint32_t eax, ebx, ecx, edx;
    cpuid(1, &eax, &ebx, &ecx, &edx);
    return (edx & (1 << 25));  // SSE is bit 25 of EDX
}

bool has_sse2() {
    uint32_t eax, ebx, ecx, edx;
    cpuid(1, &eax, &ebx, &ecx, &edx);
    return (edx & (1 << 26));  // SSE2 is bit 26 of EDX
}

bool has_avs() {
    uint32_t eax, ebx, ecx, edx;
    cpuid(1, &eax, &ebx, &ecx, &edx);
    return (ecx & (1 << 28));  // AVX is bit 28 of ECX
}

// Check if the CPU has an FPU
bool has_fpu() {
    uint32_t eax, ebx, ecx, edx;
    cpuid(1,  &eax, &ebx, &ecx, &edx);
    return (edx & (1 << 0));  // FPU is bit 0 of EDX
}

// Enable FPU and SSE
void enable_fpu_and_sse() {

    if(!has_fpu() == false) {
        printf("[Error] FPU not present!\n");
        return;
    }

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

    asm volatile ("fninit");    // Initialize the FPU

    printf(" [-] FPU and SSE enabled\n");
}


// Debugging
void print_cpu_vendor() {
    char vendor[13];
    get_cpu_vendor(vendor);
    printf(" [*] CPUID: CPU Vendor : %s\n", vendor);
}

void print_cpu_brand() {
    char brand[49];
    get_cpu_brand(brand);
    printf(" [*] CPUID: CPU Brand : %s\n", brand);
}

void print_cpu_base_frequency(){
    uint32_t cpu_base_frequency = get_cpu_base_frequency();

    printf(" [*] CPUID: CPU Base Frequency: %d Hz\n", cpu_base_frequency);
}



