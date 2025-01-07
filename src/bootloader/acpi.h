#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "../driver/ports.h"
#include "boot.h"

void qemu_poweroff();
void qemu_reboot();
