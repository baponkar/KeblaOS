
#include "vga.h"


extern unsigned char g_8x8_font[2048];  // 8x8 font data
extern unsigned char g_8x16_font[4096]; // 8x16 font data

uint32_t text_color = (uint32_t) DEFAULT_TEXT_COLOR;
uint32_t back_color = (uint32_t) DEFAULT_BACK_COLOR;

extern uint32_t *fb_ptr;

extern size_t SCREEN_WIDTH, SCREEN_HEIGHT;
extern size_t MIN_LINE_NO, MAX_LINE_NO;     // MIN_LINE_NO = 0, MAX_LINE_NO = 47
extern size_t MIN_COLUMN_NO, MAX_COLUMN_NO; // MIN_COLUMN_NO = 0, MAX_COLUMN_NO = 127 

int line_height = DEFAULT_FONT_HEIGHT + DEFAULT_TEXT_LINE_GAP;

// pixel position defined by line_no * line_width + column_no * DEFAULT_FONT_WIDTH
size_t line_no = 0;
size_t column_no = 0;

void cls(){
    for (size_t row = 0; row < SCREEN_HEIGHT; row++) {
        for (size_t col = 0; col < SCREEN_WIDTH; col++) {
            fb_ptr[row * (SCREEN_WIDTH) + col] = back_color;
        }
    }

    line_no = 0;
    column_no = 0;
}

void set_pixel(size_t x, size_t y, uint32_t color){
    fb_ptr[y * SCREEN_WIDTH + x] = color;
}

void print_char_at(size_t line_no, size_t column_no, unsigned char c){
    size_t start_x = column_no * DEFAULT_FONT_WIDTH;
    size_t start_y = line_no * line_height;
    int font_index = (unsigned char) c; // ASCII value as index
    
    switch(DEFAULT_FONT_SIZE){
        case 8:{
            // Draw character using 8x8 font
            for (int row = 0; row < 8; row++) {
                uint8_t bitmap_row = g_8x8_font[font_index * 8 + row];
                for (int col = 0; col < 8; col++) {
                    if (bitmap_row & (1 << (7 - col))) { // Check if the bit is set
                        set_pixel( start_x + col, start_y + row, text_color);
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
                         set_pixel( start_x + col, start_y + row, text_color);
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
                         set_pixel( start_x + col, start_y + row, text_color);
                    }
                }
            }
            break;
    }
}

void update_line_col_no(){
    // If line higher than Max Line No
    if(line_no > MAX_LINE_NO ){
        scroll_up();
        line_no = MAX_LINE_NO;
        column_no = MIN_COLUMN_NO;
    }

    // If column higher than Max column number
    if(column_no > MAX_COLUMN_NO){
        line_no++;
        column_no = MIN_COLUMN_NO;
    }
}


void create_newline(){
    line_no++;
    column_no = MIN_COLUMN_NO;
    update_line_col_no();
}


void putchar(unsigned char c){
    if(c == '\n'){
        create_newline();
    }else if(c == '\t'){
        column_no += 2;
    }else if(c == '\b'){
        column_no--;
    }else if(c == '\0'){
        // Do nothing for end of string
    }else{
        print_char_at(line_no, column_no, c);
        column_no++;
    }
    update_line_col_no();
}

void print(const char* text) {
    while (*text) {
        char ch = *text++;
        putchar(ch);
    }
}

void print_hex(uint64_t n){
    char hex_char[] = "0123456789ABCDEF";
    print("0x");
    for(int i = 60; i > 0; i -= 4){
        putchar(hex_char[(n >> i) & 0xF]); // Get 4 bit binary to hex value of ith position
    }
}

void print_bin(uint64_t n){
    print("0b");
    for(int i = 31; i > 0; i--){
        uint64_t bit = (n >> i) & 1; // get ith bit value either zero or one
        putchar(bit ? '1' : '0');
    }
}

void  print_dec(uint64_t n){
    if(n == 0){
        putchar('0');
        return;
    }

    char buffer[24]; // Considering it can hold 64 bit decimal values
    int i = 0;
    while(n > 0){
        buffer[i++] = '0' + (n % 10); // get the last digit
        n /= 10;    //removing last digit from n
    }

    for(int j = i -1; j>= 0; j--){
        putchar(buffer[j]);
    }
}


void scroll_up() {
    // scroll up each row by one
    for(int row = 1; row < SCREEN_HEIGHT - 1; row++){
        for(int col = 0; col < SCREEN_WIDTH - 1; col++){
            fb_ptr[(row - 1) * SCREEN_WIDTH + col] =  fb_ptr[row * SCREEN_WIDTH + col];
        }
    }

    // Clearing last line
    for(int row = SCREEN_HEIGHT - 1 - line_height; row < SCREEN_HEIGHT - 1; row++){
        for(int col = 0; col < SCREEN_WIDTH - 1; col++){
            fb_ptr[row * SCREEN_WIDTH + col] = DEFAULT_BACK_COLOR;
        }
    }
}


