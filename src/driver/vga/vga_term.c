

#include "../io/serial.h"

#include "fonts/eng/eng_8x8.h"
#include "fonts/eng/eng_8x16.h"
#include "color.h"
#include "framebuffer.h"
#include "../../lib/string.h"
#include "../../lib/stdio.h"
#include "../../x86_64/interrupt/apic.h"

#include "vga_term.h"

// Framebuffer info
extern uint32_t *fb_address;

extern uint64_t fb_width;
extern uint64_t fb_height;
extern uint64_t fb_pitch;
extern uint16_t fb_bpp; // Bits per pixel


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

void vga_init(){
    get_fb_info();

    cur_x = 0;
    cur_y = 0;
}



void clear_screen(){
    size_t pitch_pixels = fb_pitch / sizeof(uint32_t);
    for(size_t row = 0; row < (size_t) (fb_height - 1); row++){
        for(size_t col = 0; col < pitch_pixels - 1; col++){
            fb_address[row * pitch_pixels + col] = back_color;
        }
    }

    cur_x = 0;
    cur_y = 0;
}


void set_pixel(int x, int y, uint32_t color){
    if (x < 0 || x >= fb_width || y < 0 || y >= fb_height) return;
    uint32_t *pixel = (uint32_t*)((uintptr_t)fb_address + (y * fb_pitch) + (x * 4));
    *pixel = color;
}


void scroll_up() {
    if (!fb_address) return;

    size_t pitch_pixels = fb_pitch / sizeof(uint32_t); // Convert pitch from bytes to pixels
    size_t line_height = font_height; // Number of pixel rows per text line

    // Move screen up by one text line
    for (size_t row = line_height; row < fb_height; row++) {
        for (size_t col = 0; col < pitch_pixels; col++) {
            size_t dest_row = row - line_height;
            fb_address[dest_row * pitch_pixels + col] = fb_address[row * pitch_pixels + col];
        }
    }

    // Clear the last text line (fill with background color)
    uint32_t *last_line = fb_address + (fb_height - line_height) * pitch_pixels;
    for (size_t i = 0; i < line_height * pitch_pixels; i++) {
        last_line[i] = back_color;
    }
}




void update_cur_pos() {
    // Move cur to next position
    cur_x += (int)font_width / 2;

    // If cur reaches the end of the line, move to the next line
    if (cur_x >= (int)fb_width) {
        cur_x = 0;
        cur_y += (int)font_height;
    }

    // If cur reaches the bottom of the screen, scroll up
    if (cur_y >= (int)fb_height) {
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
    if (cur_y >= fb_height - font_height) {
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
        cur_x = fb_width - font_width;  // Move to the last column
        cur_y -= font_height; // Move up a line
    } else {
        cur_x -= font_width; // Move back one character
    }

    // Erase the previous character by drawing a blank space
    for (uint64_t row = 0; row < font_height; row++) {
        for (uint64_t col = 0; col < font_width; col++) {
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


void print_hex(uint64_t n) {
    char hex_chars[] = "0123456789ABCDEF";
    print("0x");

    int leading_zero = 1; // Flag to skip leading zeros

    for (int i = 60; i >= 0; i -= 4) { // Process each nibble (4 bits)
        char digit = hex_chars[(n >> i) & 0xF];
        if (digit != '0' || !leading_zero || i == 0) {
            putchar(digit);
            leading_zero = 0; // Once a non-zero digit is found, reset the flag
        }
    }
}





// Outputs a decimal number to the screen.
void print_dec(uint64_t n){
    if (n == 0)
    {
        putchar('0');
        return;
    }

    if(n < 0){
        putchar('-');
        n *= -1;
        return;
    }

    char buffer[48]; // Enough for a 64-bit integer
    int i = 0;

    while (n > 0)
    {
        buffer[i++] = '0' + (n % 10); // get the last digit
        n /= 10;                      // remove the last digit
    }

    // Digits are in reverse order, so print them backwards
    for (int j = i - 1; j >= 0; j--)
    {
        putchar(buffer[j]);
    }
}


// This will print binary numbers
void print_bin(uint64_t n) {
    print("0b");  // Print binary prefix
    // Loop through all 32 bits of the number, starting from the most significant bit (31)
    for (int i = 63; i >= 0; i--) {
        uint64_t bit = (n >> i) & 1;  // Extract the i-th bit (0 or 1)
        putchar(bit ? '1' : '0');     // Print '1' or '0'
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


