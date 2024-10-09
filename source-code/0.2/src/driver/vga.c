/*
 ____________________________________________________________________________________________________
|----------------------------------------------------------------------------------------------------|
| Description : VGA Text Mode (80x25) driver, printing char and string, updating cursor etc.         |
| Developed By : Bapon Kar                                                                           |
| Credits :                                                                                          |
| 1. https://web.archive.org/web/20160412174753/http://www.jamesmolloy.co.uk/tutorial_html/index.html|
| 2. http://www.osdever.net/tutorials/view/brans-kernel-development-tutorial                         |
|____________________________________________________________________________________________________|
*/


#include "../stdlib/stdint.h"
#include "../driver/ports.h"
#include "vga.h"

int cur_pos_x = 0;
int cur_pos_y = 0;

int MIN_ROW = 0;
int MAX_ROW = HEIGHT - 1;

int MIN_COLUMN = 0;
int MAX_COLUMN = WIDTH - 1;

uint16_t* const vgamem = (uint16_t* const) VIDEO_MEMORY_ADDRESS;

const uint16_t default_color = (COLOR8_BLUE << 12) | (COLOR8_WHITE << 8);
const uint16_t error_color = (COLOR8_BLUE << 12) | (COLOR8_RED << 8);

uint16_t current_color = default_color;



void enable_cursor(uint8_t cursor_start, uint8_t cursor_end){
    // Select the Cursor Start Register (Register 0x0A)
    outb(VGA_INDEX_REG_PORT, 0x0A);
    // Read the current value of the Cursor Start Register, preserve the upper two bits,
    // and set the lower six bits to `cursor_start`
    outb(VGA_DATA_REG_PORT, (inb(VGA_DATA_REG_PORT) & 0xC0) | cursor_start);

    // Select the Cursor End Register (Register 0x0B)
    outb(VGA_INDEX_REG_PORT, 0x0B);
    // Read the current value of the Cursor End Register, preserve the upper three bits,
    // and set the lower five bits to `cursor_end`
    outb(VGA_DATA_REG_PORT, (inb(VGA_DATA_REG_PORT) & 0xE0) | cursor_end);
}


void disable_cursor(){
    outb(VGA_INDEX_REG_PORT, 0x0A);
	outb(VGA_DATA_REG_PORT, 0x20);
}

void set_cursor_pos(int x, int y){
    uint16_t pos = y * WIDTH + x;

	outb(VGA_INDEX_REG_PORT, 0x0F);
	outb(VGA_DATA_REG_PORT, (uint8_t) (pos & 0xFF));

	outb(VGA_INDEX_REG_PORT, 0x0E);
	outb(VGA_DATA_REG_PORT, (uint8_t) ((pos >> 8) & 0xFF));
}


void move_cursor(){
    uint16_t cur_pos = (cur_pos_y) * WIDTH + cur_pos_x;

	outb(VGA_INDEX_REG_PORT, 0x0F);
	outb(VGA_DATA_REG_PORT, (uint8_t) (cur_pos & 0xFF));

	outb(VGA_INDEX_REG_PORT, 0x0E);
	outb(VGA_DATA_REG_PORT, (uint8_t) ((cur_pos >> 8) & 0xFF));
}

uint16_t get_cursor_position(void)
{
    uint16_t pos = 0;
    outb(VGA_INDEX_REG_PORT, 0x0F);
    pos |= inb(VGA_DATA_REG_PORT);

    outb(VGA_INDEX_REG_PORT, 0x0E);
    pos |= ((uint16_t)inb(VGA_DATA_REG_PORT)) << 8;
    return pos;
}

int get_cursor_position_x(){
    int x = get_cursor_position() % WIDTH;
    return x;
}

int get_cursor_position_y(){
    int y = (int) get_cursor_position()/WIDTH;
    return y;
}


void vga_clear(){
    cur_pos_x = 0;
    cur_pos_y = 0;
    enable_cursor(0x00, 0x01);
    move_cursor();

    current_color = default_color;

    for(int row = MIN_ROW; row < MAX_ROW; row++){
        for(int col = MIN_COLUMN; col < MAX_COLUMN; col++){
            vgamem[row * WIDTH + col] = current_color | ' '; // clearing each row and column
        }
    }

    
}


void create_newline(){
    cur_pos_x = 0;
    cur_pos_y++;
    vgamem[cur_pos_y * WIDTH + cur_pos_x] = current_color | ' ';
}

void scroll_up(){
    // Scrolling up each line
    for(int row=1; row<MAX_ROW - 1; row++){
        for(int col=0; col<MAX_COLUMN - 1; col++){
            vgamem[(row-1)*WIDTH + col] = vgamem[row*WIDTH + col]; 
        }
    }

    // Creating a blank line in last row
    for( int col=0; col<MAX_COLUMN - 1; col++){
        vgamem[MAX_ROW*WIDTH + col] = current_color | ' ';
    }

    cur_pos_x = 0;
    cur_pos_y = MAX_ROW;
    move_cursor();
}

// Printing a single character in the VGA screen
// unsigned char => uint8_t
void putchar(unsigned char c){
    current_color = default_color;
    // // Handel backspace
    if( c == 0x08 & cur_pos_x | c == '\b'){
        cur_pos_x--;
        vgamem[cur_pos_y * WIDTH + cur_pos_x] = current_color |  ' '; // Clearing former column

    // Handle a tab by increasing the cursor's X, but only to a point
    // where it is divisible by 8.
    }else if (c == 0x09 | c == '\t'){
       cur_pos_x = (cur_pos_x+8) & ~(8-1);

    // Handel carriage return
    }else if( c == '\r'){
        cur_pos_x--;

    // Handel newline
    }else if(c == '\n'){
        cur_pos_x = 0;
        cur_pos_y++;

    // Handel any other printable character
    }else if(c >= ' '){
        vgamem[cur_pos_y * WIDTH + cur_pos_x] = current_color | c;
        cur_pos_x++;
    }else{
        current_color = error_color;
        vgamem[cur_pos_y * WIDTH + cur_pos_x] = current_color | 'X';
        cur_pos_x++;
    }


    // Manage Cursor position
    // Check if we need to insert a new line because we have reached the end
    // of the screen.
    if (cur_pos_x >= MAX_COLUMN)
    {
        cur_pos_x = 0;
        cur_pos_y ++;
    }

    // Scroll the screen if needed.
    if(cur_pos_y >= MAX_ROW){
        scroll_up();
    }
    
    // Move the hardware cursor.
    move_cursor();
}

void putchar_at(char c, int x, int y){
    cur_pos_x = x;
    cur_pos_y = y;
    move_cursor();
    putchar(c);
}

void print(char* s){
    int i = 0;
    while (s[i])
    {
        putchar(s[i++]);
    }
}

void print_at(char* s, int x, int y){
    cur_pos_x = x;
    cur_pos_y = y;
    move_cursor();
    print(s);
}

// Outputs a hexadecimal number to the screen.
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







