
#include "vga.h"


extern unsigned char g_8x8_font[2048];  // 8x8 font data
extern unsigned char g_8x16_font[4096]; // 8x16 font data

extern uint32_t *fb_ptr;

extern size_t SCREEN_WIDTH;
extern size_t SCREEN_HEIGHT;


extern size_t MIN_LINE_NO;
extern size_t MAX_LINE_NO;

extern size_t MIN_COLUMN_NO;
extern size_t MAX_COLUMN_NO;

// pixel position defined by line_no * line_width + column_no * DEFAULT_FONT_WIDTH

uint32_t text_color = (uint32_t) DEFAULT_TEXT_COLOR;
uint32_t back_color = (uint32_t) DEFAULT_BACK_COLOR;

int line_height = DEFAULT_FONT_HEIGHT + DEFAULT_TEXT_LINE_GAP;

size_t font_width = DEFAULT_FONT_WIDTH;
size_t font_height = DEFAULT_FONT_SIZE;

size_t cur_pos_x = 0;
size_t cur_pos_y = 0;

size_t lin_col_to_pix(size_t line_no, size_t col_no){
    return (line_no * line_height + col_no * DEFAULT_FONT_WIDTH);
}

void cls(){
    for (size_t row = 0; row < SCREEN_HEIGHT; row++) {
        for (size_t col = 0; col < SCREEN_WIDTH; col++) {
            fb_ptr[row * (SCREEN_WIDTH) + col] = back_color;
        }
    }

    cur_pos_x = 0;
    cur_pos_y = 0;
}


void set_pixel(size_t x, size_t y, uint32_t color) {
    if (x <= SCREEN_WIDTH -1 ) { // Assuming 32-bit pixels
        fb_ptr[y * SCREEN_WIDTH + x] = color;
    }
}

// Printing a character at (x, y)
void print_a_char(size_t y, size_t x, unsigned char c){
    int font_index = (unsigned char) c; // ASCII value as index
    switch(DEFAULT_FONT_SIZE){
        case 8:{
            // Draw character using 8x8 font
            for (int row = 0; row < 8; row++) {
                uint8_t bitmap_row = g_8x8_font[font_index * 8 + row];
                for (int col = 0; col < 8; col++) {
                    if (bitmap_row & (1 << (7 - col))) { // Check if the bit is set
                        set_pixel( x*DEFAULT_FONT_WIDTH + col, y*line_height + row, text_color);
                    }
                }
            }
        }
        break;
        case 16:{
            // Draw character using 8x16 font
            for (int row = 0; row < 16; row++) {
                uint8_t bitmap_row = g_8x16_font[font_index * 16 + row];
                for (int col = 0; col < 8; col++) {
                    if (bitmap_row & (1 << (7 - col))) { // Check if the bit is set
                         set_pixel( x*DEFAULT_FONT_WIDTH + col, y*line_height + row, text_color);
                    }
                }
            }
        }
        break;
        default:
             // Draw character using 8x16 font
            for (int row = 0; row < 16; row++) {
                uint8_t bitmap_row = g_8x16_font[font_index * 16 + row];
                for (int col = 0; col < 8; col++) {
                    if (bitmap_row & (1 << (7 - col))) { // Check if the bit is set
                         set_pixel( x*DEFAULT_FONT_WIDTH + col, y*line_height + row, text_color);
                    }
                }
            }
            break;
    }
}


void create_newline(){
    cur_pos_x = 0;
    cur_pos_y++;
    update_cur_pos();
}

// Printing a single character in the VGA screen
void putchar(unsigned char c){
    if (c == '\t'){
        // Handle a tab by increasing the cursor's X, but only to a point
        // where it is divisible by 4*DEFAULT_FONT_WIDTH.
        cur_pos_x = (cur_pos_x + 4) & ~( 4  - 1);
    }else if( c == '\r'){
        // Handel carriage return
        cur_pos_x = 0;
    }else if(c == '\n'){
        // Handel newline
        create_newline();
    }else if(c == '\b'){
        backspace_manage();
    }else if(c == '\0'){
        // Do nothing for end of string otherwise it unnecessary increase column number
    }else{ 
        print_a_char(cur_pos_y, cur_pos_x, c);
        cur_pos_x += 1;
    }
    
    // Move the hardware cursor.
    update_cur_pos();
}


void print(const char* text) {
    while (*text) {
        char ch = *text++;
        putchar(ch);
    }
}

void scroll_up() {

    // Calculate the number of pixels to move up
    int move_up_pixels = line_height * SCREEN_WIDTH;

    // Move each pixel row up by line_height
    memmove(fb_ptr, fb_ptr + move_up_pixels, (SCREEN_HEIGHT - line_height) * SCREEN_WIDTH * sizeof(uint32_t));

    // Clear the last line
    int start_clear = (SCREEN_HEIGHT - line_height) * SCREEN_WIDTH;
    for (size_t i = start_clear; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        fb_ptr[i] = DEFAULT_BACK_COLOR;
    }
}



void update_cur_pos(){
    if(cur_pos_x > MAX_COLUMN_NO){
        cur_pos_y++;
        cur_pos_x = MIN_COLUMN_NO;
    }

    if(cur_pos_y > MAX_LINE_NO){
        scroll_up();
        cur_pos_y = MAX_LINE_NO;
        cur_pos_x = MIN_COLUMN_NO;
    }
}

size_t get_cursor_pos_x(){
    return cur_pos_x;
}

size_t get_cursor_pos_y(){
    return cur_pos_y;
}

void set_cursor_pos_x(size_t _pos_x){
    cur_pos_x = _pos_x;
}

void set_cursor_pos_y(size_t _pos_y){
    cur_pos_y = _pos_y;
}

void move_cur_up(){
    cur_pos_y -= line_height;
}

void move_cur_down(){
    cur_pos_y += line_height;
}

void move_cur_left(){
    cur_pos_x -= DEFAULT_FONT_WIDTH;
}

void move_cur_right(){
    cur_pos_x += DEFAULT_FONT_WIDTH;
}

void print_hex(uint32_t n)
{
    char hex_chars[] = "0123456789ABCDEF";
    print("0x");

    for (int i = 28; i >= 0; i -= 4) // process each nibble (4 bits)
    {
        putchar(hex_chars[(n >> i) & 0xF]);
    }
}

// Outputs a decimal number to the screen.
void print_dec(uint32_t n)
{
    if (n == 0)
    {
        putchar('0');
        return;
    }

    char buffer[12]; // Enough for a 32-bit integer
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
void print_bin(uint32_t n) {
    print("0b");  // Print binary prefix

    // Loop through all 32 bits of the number, starting from the most significant bit (31)
    for (int i = 31; i >= 0; i--) {
        uint32_t bit = (n >> i) & 1;  // Extract the i-th bit (0 or 1)
        putchar(bit ? '1' : '0');     // Print '1' or '0'
    }
}

void  backspace_manage(){
    // Current cursor position previous character remove
    for(int row = cur_pos_y; row < cur_pos_y + line_height; row++){
        for(int col = cur_pos_x - DEFAULT_FONT_WIDTH; col < cur_pos_x ; col++){
            set_pixel(col, row, DEFAULT_BACK_COLOR);
        }
    }

    // Update Cursor Position
    if(cur_pos_x > 0 & cur_pos_y >= 0){
        cur_pos_x -= DEFAULT_FONT_WIDTH;
    }else if(cur_pos_x == 0 & cur_pos_y > 0){
        cur_pos_y -= line_height;
        cur_pos_x = SCREEN_WIDTH - 1 - DEFAULT_FONT_WIDTH;
    }
}


void draw_line( int x1, int y1, int x2, int y2, uint32_t color) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        set_pixel( x1, y1, color);  // Plot the current point

        // Check if we have reached the end point
        if (x1 == x2 && y1 == y2) {
            break;
        }

        int e2 = 2 * err;

        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }

        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}


void draw_vertical_line( int x, int y, int length, uint32_t color){
    // Drawing vertical line
    for(int i=y; i<y+length; i++){
        fb_ptr[i * SCREEN_WIDTH + x] = color;
    }
}

void draw_horizontal_line( int x, int y, int length, uint32_t color){
    // Drawing horizontal line
    for(int i=x; i<x+length; i++){
        fb_ptr[y * SCREEN_WIDTH + i] = color;
    }
}

void draw_rectangle( int x, int y, int width, int height, uint32_t color){
    draw_horizontal_line( x, y, width, color);
    draw_horizontal_line( x, y+height, width, color);

    draw_vertical_line( x, y, height, color);
    draw_vertical_line( x+width, y, height, color);
}

void draw_circle( int center_x, int center_y, int radius, uint32_t color) {
    int x = radius;
    int y = 0;
    int decision = 1 - x;  // Initial decision parameter

    while (x >= y) {
        // Plot the eight symmetrical points
        set_pixel( center_x + x, center_y + y, color);
        set_pixel( center_x - x, center_y + y, color);
        set_pixel( center_x + x, center_y - y, color);
        set_pixel( center_x - x, center_y - y, color);
        set_pixel( center_x + y, center_y + x, color);
        set_pixel( center_x - y, center_y + x, color);
        set_pixel( center_x + y, center_y - x, color);
        set_pixel( center_x - y, center_y - x, color);

        y++;

        if (decision <= 0) {
            decision += 2 * y + 1;
        } else {
            x--;
            decision += 2 * (y - x) + 1;
        }
    }
}

void cls_color( uint32_t color) {
    for (size_t y = 0; y < SCREEN_HEIGHT; y++) {
        for (size_t x = 0; x < SCREEN_WIDTH; x++) {
            fb_ptr[y * SCREEN_WIDTH + x] = color;
        }
    }
}

void fill_rectangle( int x, int y, int width, int height, uint32_t color) {
    for (int i = y; i < y + height; i++) {
        for (int j = x; j < x + width; j++) {
            set_pixel( j, i, color);
        }
    }
}

void fill_circle( int center_x, int center_y, int radius, uint32_t color) {
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x * x + y * y <= radius * radius) {
                set_pixel( center_x + x, center_y + y, color);
            }
        }
    }
}

void draw_triangle( int x1, int y1, int x2, int y2, int x3, int y3, uint32_t color) {
    draw_line( x1, y1, x2, y2, color);
    draw_line( x2, y2, x3, y3, color);
    draw_line( x3, y3, x1, y1, color);
}

void fill_flat_bottom_triangle( int x1, int y1, int x2, int y2, int x3, int y3, uint32_t color) {
    int inv_slope1 = (x2 - x1) / (y2 - y1);
    int inv_slope2 = (x3 - x1) / (y3 - y1);

    int curx1 = x1;
    int curx2 = x1;

    for (int scanlineY = y1; scanlineY <= y2; scanlineY++) {
        draw_horizontal_line( (int)curx1, scanlineY, (int)(curx2 - curx1), color);
        curx1 += inv_slope1;
        curx2 += inv_slope2;
    }
}

void fill_triangle( int x1, int y1, int x2, int y2, int x3, int y3, uint32_t color) {
    if (y2 < y1) { int temp_x = x1; x1 = x2; x2 = temp_x; int temp_y = y1; y1 = y2; y2 = temp_y; }
    if (y3 < y1) { int temp_x = x1; x1 = x3; x3 = temp_x; int temp_y = y1; y1 = y3; y3 = temp_y; }
    if (y3 < y2) { int temp_x = x2; x2 = x3; x3 = temp_x; int temp_y = y2; y2 = y3; y3 = temp_y; }

    if (y2 == y3) {
        fill_flat_bottom_triangle( x1, y1, x2, y2, x3, y3, color);
    } else if (y1 == y2) {
        fill_flat_bottom_triangle( x3, y3, x1, y1, x2, y2, color);
    } else {
        int x4 = x1 + (int)(y2 - y1) / (y3 - y1) * (x3 - x1);
        int y4 = y2;
        fill_flat_bottom_triangle( x1, y1, x2, y2, x4, y4, color);
        fill_flat_bottom_triangle( x3, y3, x2, y2, x4, y4, color);
    }
}


void draw_gradient( uint32_t start_color, uint32_t end_color) {
    for (size_t y = 0; y < SCREEN_HEIGHT; y++) {
        uint32_t color = start_color + (y * ((end_color - start_color) / SCREEN_HEIGHT));
        for (size_t x = 0; x < SCREEN_WIDTH; x++) {
            set_pixel( x, y, color);
        }
    }
}


// Function to draw a simple colorful image
void draw_colorful_image() {
    // Draw gradient background
    draw_gradient( COLOR_BLUE, COLOR_CYAN);

    // Draw a central circle
    fill_circle( SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 100, COLOR_YELLOW);

    // Draw a border rectangle around the circle
    fill_rectangle( SCREEN_WIDTH / 2 - 120, SCREEN_HEIGHT / 2 - 120, 240, 20, COLOR_MAGENTA);
    fill_rectangle( SCREEN_WIDTH / 2 - 120, SCREEN_HEIGHT / 2 + 100, 240, 20, COLOR_MAGENTA);
    fill_rectangle( SCREEN_WIDTH / 2 - 120, SCREEN_HEIGHT / 2 - 100, 20, 200, COLOR_MAGENTA);
    fill_rectangle( SCREEN_WIDTH / 2 + 100, SCREEN_HEIGHT / 2 - 100, 20, 200, COLOR_MAGENTA);

    // Draw some additional colorful circles
    fill_circle( SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4, 50, COLOR_GREEN);
    fill_circle( 3 * SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4, 50, COLOR_RED);
    fill_circle( SCREEN_WIDTH / 4, 3 * SCREEN_HEIGHT / 4, 50, COLOR_WHITE);
    fill_circle( 3 * SCREEN_WIDTH / 4, 3 * SCREEN_HEIGHT / 4, 50, COLOR_BLUE);
}

// Display an image at position (x, y) in the framebuffer
void display_image( int x, int y, const uint32_t* image_data, int img_width, int img_height) {
    for (int i = 0; i < img_height; i++) {
        for (int j = 0; j < img_width; j++) {
            uint32_t color = image_data[i * img_width + j];

            set_pixel(x + j, y + i, color);
        }
    }
}


