#include "speaker.h"

// PIT control port and speaker port
#define PIT_CONTROL_PORT 0x43
#define PIT_CHANNEL_2_DATA_PORT 0x42
#define SPEAKER_CONTROL_PORT 0x61

// PIT frequency for channel 2
#define PIT_FREQUENCY 1193180

void play_sound(uint32_t frequency) {
    // Calculate the PIT divisor
    uint16_t divisor = (uint16_t)(PIT_FREQUENCY / frequency);

    // Set the PIT to square wave mode (Mode 3) for channel 2
    outb(PIT_CONTROL_PORT, 0xB6); // 10110110 (Channel 2, Access Mode LSB then MSB, Mode 3, Binary)

    // Send the divisor to channel 2's data port (low byte, then high byte)
    outb(PIT_CHANNEL_2_DATA_PORT, (uint8_t)(divisor & 0xFF)); // Send LSB
    outb(PIT_CHANNEL_2_DATA_PORT, (uint8_t)((divisor >> 8) & 0xFF)); // Send MSB

    // Enable the speaker by setting bit 2 of port 0x61
    uint8_t tmp = inb(SPEAKER_CONTROL_PORT);
    if (tmp != (tmp | 3)) {
        outb(SPEAKER_CONTROL_PORT, tmp | 3); // Enable both speaker and channel 2
    }
}

void stop_sound() {
    // Disable the speaker by clearing bit 0 and bit 1 of port 0x61
    uint8_t tmp = inb(SPEAKER_CONTROL_PORT);
    outb(SPEAKER_CONTROL_PORT, tmp & 0xFC); // Clear bits 0 and 1
}

void beep() {
    // Play a bell sound (usually around 1000 Hz)
    play_sound(1000);

    // Wait for a short duration (this delay is about 500ms)
    // for (int i = 0; i < 100000; i++) {
    //     asm volatile("nop");
    // }
    print("before\n");
    delay(1);
    print("after");
    // Stop the sound
    stop_sound();
}
