#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

extern uint64_t cpu_frequency_hz;

void tsc_sleep(uint64_t microseconds);

void init_tsc();
