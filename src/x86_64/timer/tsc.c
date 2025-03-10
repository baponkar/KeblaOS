/*
Time Stamp Counter(TSC):
The TSC (Time Stamp Counter) is a 64-bit register in x86 processors that 
counts the number of cycles since the processor was powered on. It is read 
using the RDTSC (Read Time-Stamp Counter) instruction.

https://wiki.osdev.org/TSC
*/

#include "../../util/util.h" // for enable_interrupts()
#include "../interrupt/interrupt.h"
#include "../../lib/stdio.h"
#include "apic_timer.h"
#include "pit_timer.h"

#include "tsc.h"




uint64_t cpu_frequency_hz1 = 0;  // Cached CPU frequency in Hz


static inline uint64_t rdmsr(uint32_t msr) {
    uint32_t low, high;
    __asm__ volatile ("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
    return ((uint64_t)high << 32) | low;
}


static inline uint64_t read_tsc() {
    uint32_t low, high;
    asm volatile ("rdtsc" : "=a"(low), "=d"(high)); // Read TSC
    return ((uint64_t)high << 32) | low;
}


void tsc_sleep(uint64_t microseconds) {
    enable_interrupts();
    uint64_t start = read_tsc();
    
    // freq cycles in 1 s; 1 cycle = 1/freq s; x µs = x/1000000 s; no. of cycle in x µs = x*freq/1000000
    uint64_t cycles_to_wait = (microseconds * cpu_frequency_hz1) / 1000000; // Adjust based on CPU frequency

    while ((read_tsc() - start) < cycles_to_wait); // Wait in loop to perform all loops
}


uint64_t get_cpu_freq_msr() {
    uint64_t start_tsc, end_tsc;

    // Read TSC at the start
    start_tsc = read_tsc();

    // Sleep for 1s
    tsc_sleep(1000000000);

    // Read TSC at the end
    end_tsc = read_tsc();

    // Calculate elapsed TSC cycles
    uint64_t tsc_diff = end_tsc - start_tsc;

    // Calculate CPU frequency in Hz
    uint64_t cpu_frequency = (uint64_t) tsc_diff;

    return cpu_frequency;
}


void init_tsc(){
    cpu_frequency_hz1 = get_cpu_freq_msr();
}






