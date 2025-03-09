#include "../../util/util.h"
#include <stdint.h>

extern int apic_ticks;




void apic_delay(uint32_t milliseconds);
void apic_timer_handler(registers_t *regs);

void init_apic_timer();



