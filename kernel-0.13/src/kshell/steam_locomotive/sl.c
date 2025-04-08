#include <stddef.h>

#include "../../driver/vga/vga_term.h" // vga_init, print, create_newline, clear_screen
#include "../../driver/vga/color.h"   // COLOR_WHITE
#include "../../x86_64/timer/apic_timer.h" // apic_delay

#include "sl.h"

// ASCII art for the locomotive stored as an array of strings.
const char* locomotive[] = {
    "            (  )   (   )  )",
    "             ) (   )  (  (",
    "             ( )  (    ) )",
    "              _________",
    "           ___//_||_\\\\____",
    "          /   -       -   \\",
    "         (   )         (   )",
    "          \\_/___________\\_/"
};

#define NUM_LINES (sizeof(locomotive) / sizeof(locomotive[0]))
#define SCREEN_WIDTH 80      // Adjust to your screen width (in characters)
#define LOCO_WIDTH   40      // Approximate width of the locomotive
#define DELAY_US     10     // Delay between frames in milliseconds

// The animation function.
void sl_animation() {
    int offset = SCREEN_WIDTH; // Start off-screen on the right

    while (offset > 0) {  // Infinite loop for continuous animation
        clear_screen();

        // For each line in the locomotive ASCII art:
        for (int i = 0; i < NUM_LINES; i++) {
            // Print 'offset' spaces to move the locomotive horizontally.
            for (int j = 0; j < offset; j++) {
                print(" ");
            }
            // Print the current line of the locomotive.
            print(locomotive[i]);
            create_newline();
        }

        apic_delay(DELAY_US); // Delay between frames

        offset -= 2;     // Move locomotive left by 2 characters per frame

        // If the locomotive has completely moved off the left, reset to the right.
        if (offset < -LOCO_WIDTH) {
            offset = SCREEN_WIDTH;
        }
    }
}
