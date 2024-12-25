
#include "vga.h"

__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(0);

__attribute__((used, section(".requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};



extern unsigned char g_8x8_font[2048];  // 8x8 font data
extern unsigned char g_8x16_font[4096]; // 8x16 font data

uint32_t *FRAMEBUFFER_PTR; // Note: we assume the framebuffer model is RGB with 32-bit pixels.
size_t FRAMEBUFFER_WIDTH;
size_t FRAMEBUFFER_HEIGHT;

size_t FONT_SIZE;
size_t FONT_WIDTH;
size_t FONT_HEIGHT;

size_t MAX_LINE;
size_t MIN_LINE;
size_t LINE_GAP;
size_t LINE_HEIGHT;
size_t MAX_COLUMN;
size_t MIN_COLUMN;

uint64_t TEXT_COLOR;
uint64_t BACKGROUND_COLOR;

size_t cur_pos_x;
size_t cur_pos_y;






void vga_init(){

     // Ensure the bootloader actually understands our base revision (see spec).
    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        halt_kernel();
    }

    // Ensure the framebuffer is initialized
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        halt_kernel();
    }

        // Fetch the first framebuffer
    struct limine_framebuffer* framebuffer = framebuffer_request.response->framebuffers[0];
    FRAMEBUFFER_PTR = (uint32_t*) framebuffer->address;

    FRAMEBUFFER_WIDTH = framebuffer->width;
    FRAMEBUFFER_HEIGHT = framebuffer->height;

    FONT_SIZE = 16;
    FONT_WIDTH = 8;
    FONT_HEIGHT = 16;

    MIN_LINE = 0;
    MAX_LINE = (FRAMEBUFFER_HEIGHT / (FONT_HEIGHT + LINE_GAP)) - 1; // 47

    LINE_HEIGHT = FONT_HEIGHT + LINE_GAP;

    MIN_COLUMN = 0;
    MAX_COLUMN = (FRAMEBUFFER_WIDTH / FONT_WIDTH) - 1; // 127

    TEXT_COLOR = COLOR_WHITE;
    BACKGROUND_COLOR = COLOR_BLUE;

    cur_pos_x = 0;
    cur_pos_y = 0;

    clear_screen();
}

void print_framebuffer_info(){
    print("Framebuffer Info\n");
    print("Framebuffer Address : ");
    print_hex((uint64_t) FRAMEBUFFER_PTR);
    print("\n");

    print("RGB Mode : ");
    print_dec(32);
    print("\n");

    print("Framebuffer Resolution : ");
    print_dec(FRAMEBUFFER_WIDTH);
    print("x");
    print_dec(FRAMEBUFFER_HEIGHT);
    print("\n");

    print("Font Size : ");
    print_dec(FONT_SIZE);
    print("\n");

    print("Font Width : ");
    print_dec(FONT_WIDTH);
    print("\n");

    print("Font Height : ");
    print_dec(FONT_HEIGHT);
    print("\n");

    print("Line Height : ");
    print_dec(LINE_HEIGHT);
    print("\n");

}

void clear_screen(){
        for (size_t row = 0; row < FRAMEBUFFER_HEIGHT; row++) {
        for (size_t col = 0; col < FRAMEBUFFER_WIDTH; col++) {
            FRAMEBUFFER_PTR[row * (FRAMEBUFFER_WIDTH) + col] = BACKGROUND_COLOR;
        }
    }
    cur_pos_x = 0;
    cur_pos_y = 0;
}

void set_pixel(size_t x, size_t y, uint64_t color) {
    if ((x <= FRAMEBUFFER_WIDTH -1) & (y < FRAMEBUFFER_HEIGHT)) {
        FRAMEBUFFER_PTR[y * FRAMEBUFFER_WIDTH + x] = color;
    }
}


// Printing a character at (x, y)
void print_char_at(size_t y, size_t x, unsigned char c){
    int font_index = (unsigned char) c; // ASCII value as index
    switch(FONT_SIZE){
        case 8:{
            // Draw character using 8x8 font
            for (int row = 0; row < 8; row++) {
                uint8_t bitmap_row = g_8x8_font[font_index * 8 + row];
                for (int col = 0; col < 8; col++) {
                    if (bitmap_row & (1 << (7 - col))) { // Check if the bit is set
                        set_pixel( x * FONT_WIDTH + col, y*LINE_HEIGHT + row, TEXT_COLOR);
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
                         set_pixel( x*DEFAULT_FONT_WIDTH + col, y*LINE_HEIGHT + row, TEXT_COLOR);
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
                         set_pixel( x*FONT_WIDTH + col, y*LINE_HEIGHT + row, TEXT_COLOR);
                    }
                }
            }
            break;
    }

    update_cur_pos();
}

void update_cur_pos(){
    if(cur_pos_x > (int) MAX_COLUMN){
        cur_pos_y++;
        cur_pos_x = MIN_COLUMN;
    }

    if(cur_pos_y > MAX_LINE){
        scroll_up();
        cur_pos_y = MAX_LINE;
        cur_pos_x = MIN_COLUMN;
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
        update_cur_pos();
    }else if( c == '\r'){
        // Handel carriage return
        cur_pos_x = 0;
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
        print_char_at(cur_pos_y, cur_pos_x, c);
        cur_pos_x += 1;
        update_cur_pos();
    }
}

void print(const char* text) {
    while (*text) {
        char ch = *text++;
        putchar(ch);
    }
}

void scroll_up() {

    // Calculate the number of pixels to move up
    int move_up_pixels = LINE_HEIGHT * FRAMEBUFFER_WIDTH;

    // Move each pixel row up by line_height
    memmove(FRAMEBUFFER_PTR, FRAMEBUFFER_PTR + move_up_pixels, (FRAMEBUFFER_HEIGHT - LINE_HEIGHT) * FRAMEBUFFER_WIDTH * sizeof(uint64_t));

    // Clear the last line
    int start_clear = (FRAMEBUFFER_HEIGHT - LINE_HEIGHT) * FRAMEBUFFER_WIDTH;
    for (size_t i = start_clear; i < FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT; i++) {
        FRAMEBUFFER_PTR[i] = BACKGROUND_COLOR;
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
    cur_pos_y--;
}

void move_cur_down(){
    cur_pos_y++;
}

void move_cur_left(){
    cur_pos_x--;
}

void move_cur_right(){
    cur_pos_x++;
}



void print_hex(uint64_t n) {
    char hex_chars[] = "0123456789ABCDEF";
    printf("0x");

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
void print_dec(uint64_t n)
{
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



void  backspace_manage(){
    // Current cursor position previous character remove
    for(int row = cur_pos_y*LINE_HEIGHT; row < (cur_pos_y+1)*LINE_HEIGHT; row++){
        for(int col = (cur_pos_x-1)*FONT_WIDTH; col < cur_pos_x * FONT_WIDTH ; col++){
            set_pixel(col, row, BACKGROUND_COLOR);
        }
    }
    
    // Update Cursor Position
    if(cur_pos_x > 0){
        cur_pos_x--;
    }
    if((cur_pos_x == 0) && (cur_pos_y+1 > 0)){
        cur_pos_y--;
        cur_pos_x = MAX_COLUMN;
    }
}


void draw_line( int x1, int y1, int x2, int y2, uint64_t color) {
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


void draw_vertical_line( int x, int y, int length, uint64_t color){
    // Drawing vertical line
    for(int i=y; i<y+length; i++){
        FRAMEBUFFER_PTR[i * FRAMEBUFFER_WIDTH + x] = color;
    }
}


void draw_horizontal_line( int x, int y, int length, uint64_t color){
    // Drawing horizontal line
    for(int i=x; i<x+length; i++){
        FRAMEBUFFER_PTR[y * FRAMEBUFFER_WIDTH + i] = color;
    }
}


void draw_rectangle( int x, int y, int width, int height, uint64_t color){
    draw_horizontal_line( x, y, width, color);
    draw_horizontal_line( x, y+height, width, color);

    draw_vertical_line( x, y, height, color);
    draw_vertical_line( x+width, y, height, color);
}


void draw_circle( int center_x, int center_y, int radius, uint64_t color) {
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


void cls_color( uint64_t color) {
    for (size_t y = 0; y < FRAMEBUFFER_HEIGHT; y++) {
        for (size_t x = 0; x < FRAMEBUFFER_WIDTH; x++) {
            FRAMEBUFFER_PTR[y * FRAMEBUFFER_WIDTH + x] = color;
        }
    }
}


void fill_rectangle( int x, int y, int width, int height, uint64_t color) {
    for (int i = y; i < y + height; i++) {
        for (int j = x; j < x + width; j++) {
            set_pixel( j, i, color);
        }
    }
}


void fill_circle( int center_x, int center_y, int radius, uint64_t color) {
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x * x + y * y <= radius * radius) {
                set_pixel( center_x + x, center_y + y, color);
            }
        }
    }
}


void draw_triangle( int x1, int y1, int x2, int y2, int x3, int y3, uint64_t color) {
    draw_line( x1, y1, x2, y2, color);
    draw_line( x2, y2, x3, y3, color);
    draw_line( x3, y3, x1, y1, color);
}


void fill_flat_bottom_triangle( int x1, int y1, int x2, int y2, int x3, int y3, uint64_t color) {
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


void fill_triangle( int x1, int y1, int x2, int y2, int x3, int y3, uint64_t color) {
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


void draw_gradient( uint64_t start_color, uint64_t end_color) {
    for (size_t y = 0; y < FRAMEBUFFER_HEIGHT; y++) {
        uint64_t color = start_color + (y * ((end_color - start_color) / FRAMEBUFFER_HEIGHT));
        for (size_t x = 0; x < FRAMEBUFFER_WIDTH; x++) {
            set_pixel( x, y, color);
        }
    }
}


// Function to draw a simple colorful image
void draw_colorful_image() {
    // Draw gradient background
    draw_gradient( COLOR_BLUE, COLOR_CYAN);

    // Draw a central circle
    fill_circle( FRAMEBUFFER_WIDTH / 2, FRAMEBUFFER_HEIGHT / 2, 100, COLOR_YELLOW);

    // Draw a border rectangle around the circle
    fill_rectangle( FRAMEBUFFER_WIDTH / 2 - 120, FRAMEBUFFER_HEIGHT / 2 - 120, 240, 20, COLOR_MAGENTA);
    fill_rectangle( FRAMEBUFFER_WIDTH / 2 - 120, FRAMEBUFFER_HEIGHT / 2 + 100, 240, 20, COLOR_MAGENTA);
    fill_rectangle( FRAMEBUFFER_WIDTH / 2 - 120, FRAMEBUFFER_HEIGHT / 2 - 100, 20, 200, COLOR_MAGENTA);
    fill_rectangle( FRAMEBUFFER_WIDTH / 2 + 100, FRAMEBUFFER_HEIGHT / 2 - 100, 20, 200, COLOR_MAGENTA);

    // Draw some additional colorful circles
    fill_circle( FRAMEBUFFER_WIDTH / 4, FRAMEBUFFER_HEIGHT / 4, 50, COLOR_GREEN);
    fill_circle( 3 * FRAMEBUFFER_WIDTH / 4, FRAMEBUFFER_HEIGHT / 4, 50, COLOR_RED);
    fill_circle( FRAMEBUFFER_WIDTH / 4, 3 * FRAMEBUFFER_HEIGHT / 4, 50, COLOR_WHITE);
    fill_circle( 3 * FRAMEBUFFER_WIDTH / 4, 3 * FRAMEBUFFER_HEIGHT / 4, 50, COLOR_BLUE);
}


// Display an image at position (x, y) in the framebuffer
void display_image( int x, int y, const uint64_t* image_data, int img_width, int img_height) {
    for (int i = 0; i < img_height; i++) {
        for (int j = 0; j < img_width; j++) {
            uint64_t color = image_data[i * img_width + j];
            set_pixel(x + j, y + i, color);
        }
    }
}


