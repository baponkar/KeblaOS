#ifndef SPEAKER_H
#define SPEAKER_H

#include <stdint.h>
#include "ports.h"
#include "../idt/timer.h"

void play_sound(uint32_t frequency);
void stop_sound(void);
void beep(void);

#endif
