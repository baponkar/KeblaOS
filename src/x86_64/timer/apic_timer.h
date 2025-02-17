
#include <stdint.h>
#include "../../util/util.h"

void tsc_sleep(uint64_t microseconds);
void apic_delay(uint32_t milliseconds);
void init_apic_timer();
