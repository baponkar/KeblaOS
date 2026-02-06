

#include "../io/serial.h"

#include "data/fonts/eng/eng_8x8.h"
#include "data/fonts/eng/eng_8x16.h"
#include "color.h"
#include "framebuffer.h"
#include "vga.h"

#include "../../lib/string.h"
#include "../../lib/stdio.h"
#include "../../arch/interrupt/apic/apic.h"

#include "vga_term.h"


// Framebuffer info
extern uint32_t *fb0_address;

extern uint64_t fb0_width;
extern uint64_t fb0_height;
extern uint64_t fb0_pitch;
extern uint16_t fb0_bpp;        // Bits per pixel

int cur_x = 0;
int cur_y = 0;

// Color Settings
uint32_t text_color = (uint32_t) COLOR_WHITE;
uint32_t back_color = (uint32_t) COLOR_BLACK;

uint64_t font_width = 8;
uint64_t font_height = 16;

uint64_t font_size = 1;

// Font Data
extern unsigned char g_8x8_font[2048];
extern unsigned char g_8x16_font[4096];

// Emoji Data
extern unsigned char checkmark[16];





void clear_screen(){

    cls_color(back_color);

    cur_x = 0;
    cur_y = 0;
}


void scroll_up() {
    if (!fb0_address) return;

    size_t pitch_pixels = fb0_pitch / sizeof(uint32_t); // Convert pitch from bytes to pixels
    size_t line_height = font_height; // Number of pixel rows per text line

    // Move screen up by one text line
    for (size_t row = line_height; row < fb0_height; row++) {
        for (size_t col = 0; col < pitch_pixels; col++) {
            size_t dest_row = row - line_height;
            fb0_address[dest_row * pitch_pixels + col] = fb0_address[row * pitch_pixels + col];
        }
    }

    // Clear the last text line (fill with background color)
    uint32_t *last_line = fb0_address + (fb0_height - line_height) * pitch_pixels;
    for (size_t i = 0; i < line_height * pitch_pixels; i++) {
        last_line[i] = back_color;
    }
}

void update_cur_pos() {
    // Move cur to next position
    cur_x += (int)font_width / 2;

    // If cur reaches the end of the line, move to the next line
    if (cur_x >= (int)fb0_width) {
        cur_x = 0;
        cur_y += (int)font_height;
    }

    // If cur reaches the bottom of the screen, scroll up
    if (cur_y >= (int)fb0_height) {
        scroll_up();
        cur_y -= (int)font_height; // Keep cur within the screen after scrolling
    }
}


void draw_char_8x8(int x, int y, char c, uint32_t color) {
    uint8_t *glyph = &g_8x8_font[c * 8]; // Each character has 8 rows

    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if (glyph[row] & (1 << (7 - col))) { // Check if bit is set
                set_pixel(x + col, y + row, color);
            }
        }
    }
    update_cur_pos();
}

void draw_char_8x16(int x, int y, char c, uint32_t color) {
    uint8_t *glyph = &g_8x16_font[c * 16]; // Each character has 16 rows

    for (int row = 0; row < 16; row++) {
        for (int col = 0; col < 8; col++) {
            if (glyph[row] & (1 << (7 - col))) { // Check pixel bit
                set_pixel(x + col, y + row, color);
            }
        }
    }
    update_cur_pos();
}

void draw_char(int x, int y, char c, uint32_t color){
    switch(font_size * font_height){
        case 8:
            draw_char_8x8(x, y, c, color);
            break;
        case 16:
            draw_char_8x16(x, y, c, color);
            break;
        default:
            draw_char_8x16(x, y, c, color);
            break;
    }
}


void draw_string(int x, int y, const char* str, uint32_t color) {
    while (*str) {
        draw_char(x, y, *str++, color);
        x += font_width / 2; // Move right for next character
    }
}


void create_newline() {
    cur_x = 0;         // Reset to the beginning of the line
    cur_y += font_height; // Move to the next row

    // If we reach the bottom, scroll up
    if (cur_y >= fb0_height - font_height) {
        scroll_up();
        cur_y -= font_height; // Keep cur within the screen after scrolling
    }
}

void backspace_manage() {
    serial_clearchar();
    // If the cur is already at (0,0), do nothing
    if (cur_x == 0 && cur_y == 0) {
        return;
    }

    // If at the beginning of a line, move up one line
    if (cur_x == 0) {
        cur_x = fb0_width - font_width;  // Move to the last column
        cur_y -= font_height; // Move up a line
    } else {
        cur_x -= font_width; // Move back one character
    }

    // Erase the previous character by drawing a blank space
    for (uint64_t row = 0; row < font_height; row++) {
        for (uint64_t col = 0; col <= font_width; col++) {
            set_pixel(cur_x + col, cur_y + row, back_color); // Black color (background)
        }
    }
}



// Printing a single character in the VGA screen
void putchar(unsigned char c){
    serial_putchar(c);
    if (c == '\t'){
        // Handle a tab by increasing the cur's X, but only to a point
        // where it is divisible by 4*DEFAULT_FONT_WIDTH.
        cur_x = (cur_x + 4) & ~( 4  - 1);
        update_cur_pos();
    }else if( c == '\r'){
        // Handel carriage return
        cur_x = 0;
        update_cur_pos();
    }else if(c == '\n'){
        // Handel newline
        create_newline();
        update_cur_pos();
    }else if(c == '\b'){
        backspace_manage();
    }else if(c == '\0'){
        // Do nothing for end of string otherwise it unnecessary increase column number
    }else{ 
        draw_char(cur_x, cur_y, c, text_color);
        cur_x += 1;
        update_cur_pos();
    }
}


void print(const char* text) {
    while (*text) {
        char ch = *text++;
        putchar(ch);
    }
}







size_t get_cursor_pos_x(){
    return cur_x;
}

size_t get_cursor_pos_y(){
    return cur_y;
}

void set_cursor_pos_x(size_t _pos_x){
    cur_x = _pos_x;
}

void set_cursor_pos_y(size_t _pos_y){
    cur_y = _pos_y;
}

void move_cur_up(){
    cur_y--;
}

void move_cur_down(){
    cur_y++;
}

void move_cur_left(){
    cur_x--;
}

void move_cur_right(){
    cur_x++;
}


void draw_checkmark(int x, int y, uint32_t color) {
    for (int row = 0; row < 16; row++) {
        for (int col = 0; col < 8; col++) {
            if (checkmark[row] & (1 << (7 - col))) { // Check if bit is set
                set_pixel(x + col, y + row, color);
            }
        }
    }
}




void color_putchar(unsigned char c, uint32_t color){
    serial_putchar(c);
    if (c == '\t'){
        // Handle a tab by increasing the cur's X, but only to a point
        // where it is divisible by 4*DEFAULT_FONT_WIDTH.
        cur_x = (cur_x + 4) & ~( 4  - 1);
        update_cur_pos();
    }else if( c == '\r'){
        // Handel carriage return
        cur_x = 0;
        update_cur_pos();
    }else if(c == '\n'){
        // Handel newline
        create_newline();
        update_cur_pos();
    }else if(c == '\b'){
        backspace_manage();
    }else if(c == '\0'){
        // Do nothing for end of string otherwise it unnecessary increase column number
    }else{ 
        draw_char(cur_x, cur_y, c, color);
        cur_x += 1;
        update_cur_pos();
    }
}


void color_print(const char* text, uint32_t color) {
    while (*text) {
        char ch = *text++;
        color_putchar(ch, color);
    }
}

// Global variables to track cursor state and previous position.
volatile bool cursor_visible = true;
int prev_cursor_x = -1;
int prev_cursor_y = -1;

// Dummy fill_rect function; replace with your actual implementation.
void fill_rect(int x, int y, int width, int height, uint32_t color) {
    // Draw a rectangle at (x, y) with given width and height.
    // This should set all pixels in that area to 'color'.
    for (int row = y; row < y + height && row < fb0_height; row++) {
        for (int col = x; col < x + width && col < fb0_width; col++) {
            set_pixel(col, row, color);
        }
    }
}

void draw_cursor() {
    // Before drawing the new cursor, clear the previous cursor area.
    if (prev_cursor_x != -1 && prev_cursor_y != -1) {
        // Clear the previous cursor area by redrawing the background.
        fill_rect(prev_cursor_x, prev_cursor_y, font_width / 2, font_height, back_color);
    }
    
    if (cursor_visible) {
        // Draw the cursor as a vertical bar at the current cursor position.
        fill_rect(cur_x, cur_y, font_width / 2, font_height, text_color);
    }
    
    // Save the current cursor position for next time.
    prev_cursor_x = cur_x;
    prev_cursor_y = cur_y;
}

// This function is called periodically (e.g., via a timer interrupt) to toggle the cursor.
void toggle_cursor() {
    // Toggle the visibility flag.
    cursor_visible = !cursor_visible;
    // Update the cursor drawing.
    draw_cursor();
}

// Function to draw a progress bar at current cursor position
void draw_progress_bar(int progress, int total, int width) {
    
    if (width <= 0) {
        width = fb0_width - cur_x; // Use remaining screen width by default
    }
    
    // Save current cursor position
    int saved_x = cur_x;
    int saved_y = cur_y;
    
    // Move cursor to the desired position (current y + 1, x = 0)
    int bar_y = saved_y + font_height;
    int bar_x = 0;
    
    // Ensure we don't go beyond screen height
    if (bar_y >= fb0_height - font_height) {
        scroll_up();
        bar_y -= font_height;
        saved_y -= font_height; // Adjust saved position too
    }
    
    // Draw progress bar border
    uint32_t border_color = COLOR_GRAY;
    uint32_t fill_color = COLOR_CYAN;
    uint32_t text_color = COLOR_WHITE;
    
    // Draw top border
    for (int i = 0; i < width; i++) {
        set_pixel(bar_x + i, bar_y, border_color);
    }
    
    // Draw bottom border
    for (int i = 0; i < width; i++) {
        set_pixel(bar_x + i, bar_y + font_height - 1, border_color);
    }
    
    // Draw left border
    for (int i = 0; i < font_height; i++) {
        set_pixel(bar_x, bar_y + i, border_color);
    }
    
    // Draw right border
    for (int i = 0; i < font_height; i++) {
        set_pixel(bar_x + width - 1, bar_y + i, border_color);
    }
    
    // Calculate fill width
    float percentage = (float)progress / total;
    int fill_width = (int)((width - 2) * percentage); // -2 for borders
    
    // Fill the progress bar
    for (int row = 1; row < font_height - 1; row++) {
        for (int col = 1; col < width - 1; col++) {
            if (col <= fill_width) {
                set_pixel(bar_x + col, bar_y + row, fill_color);
            } else {
                set_pixel(bar_x + col, bar_y + row, back_color);
            }
        }
    }
    
    // Draw percentage text in the middle of the bar
    char percentage_str[16];
    int percent = (int)(percentage * 100);
    snprintf(percentage_str, sizeof(percentage_str), "%d %%", percent);
    
    // Calculate text position (centered)
    int text_len = strlen(percentage_str);
    int text_x = bar_x + (width - text_len * (font_width / 2)) / 2;
    int text_y = bar_y + (font_height - font_height) / 2;
    
    // Draw the percentage text
    draw_string(text_x, text_y, percentage_str, text_color);
    
    // Restore original cursor position
    cur_x = saved_x;
    cur_y = saved_y;
}


