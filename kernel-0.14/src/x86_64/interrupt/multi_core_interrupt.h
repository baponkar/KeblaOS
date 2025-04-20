

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "../../util/util.h" // for registers_t structure




void init_core_interrupt(uint64_t core_id);

void init_bootstrap_interrupt(int bootstrap_core_id);
void init_application_core_interrupt(int start_core_id, int end_core_id);


void test_interrupt();



