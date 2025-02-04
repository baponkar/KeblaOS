

#include <stdint.h>
#include "../../util/util.h"

void apic_remap_timer();
void apic_timer_handler(registers_t *regs);
void apic_timer_init(uint32_t frequency);


