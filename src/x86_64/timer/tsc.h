#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

extern uint64_t cpu_frequency_hz;


static inline uint64_t rdmsr(uint32_t msr);
static inline uint64_t read_tsc();
void tsc_sleep(uint64_t microseconds);
uint64_t get_cpu_freq_msr();

void init_tsc();
