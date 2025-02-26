
#include <stdint.h>


void tsc_sleep(uint64_t microseconds);
void apic_delay(uint32_t milliseconds);
void init_apic_timer();

void apic_timer_handler(registers_t *regs);
