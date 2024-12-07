/*
 _____________________________________________________________________________________________________
|-----------------------------------------------------------------------------------------------------|
| Description : VGA Text Mode (80x25) driver, printing char and string, updating cursor etc.          |
| Developed By : Bapon Kar                                                                            |
| Credits :                                                                                           |
| 1. https://web.archive.org/web/20160412174753/http://www.jamesmolloy.co.uk/tutorial_html/index.html |
| 2. http://www.osdever.net/tutorials/view/brans-kernel-development-tutorial                          |
|_____________________________________________________________________________________________________|
*/




#include "../stdlib/stdint.h"
#include "ports.h"
#include "keyboard.h"
#include "vga.h"

const int WIDTH = 80;
const int HEIGHT = 25;

int cur_pos_col = 0; // Maximum Value is WIDTH - 1
int cur_pos_row = 0; // Maximum Value is HEIGHT - 1


uint16_t* const vgamem = (uint16_t* const) VIDEO_MEMORY_ADDRESS;

const uint16_t default_color = (COLOR8_BLACK << 12) | (COLOR8_WHITE << 8);   // White on Black
const uint16_t error_color = (COLOR8_BLUE << 12) | (COLOR8_RED << 8);       // Red on Blue

uint16_t current_color = default_color; 

// Function to enable the cursor and set its shape
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


void set_cursor_pos(int col, int row){
    int pos = row * WIDTH + col;

	outb(VGA_INDEX_REG_PORT, 0x0F);
	outb(VGA_DATA_REG_PORT, (uint8_t) (pos & 0xFF));

	outb(VGA_INDEX_REG_PORT, 0x0E);
	outb(VGA_DATA_REG_PORT, (uint8_t) ((pos >> 8) & 0xFF));
}


uint16_t get_cursor_position(void){
    /* This function willreturn 1 bit position where first 8 bit represent */
    uint16_t pos = 0;
    outb(VGA_INDEX_REG_PORT, 0x0F);
    pos |= inb(VGA_DATA_REG_PORT);

    outb(VGA_INDEX_REG_PORT, 0x0E);
    pos |= ((uint16_t)inb(VGA_DATA_REG_PORT)) << 8;
    return pos;
}

int get_cursor_position_col(){
    int col = get_cursor_position() % WIDTH;
    return col;
}

int get_cursor_position_row(){
    int row = (int) get_cursor_position()/WIDTH;
    return row;
}



// Functions to move the cursor

void move_cursor(){
    /* This Function will move the cursor on the basis of current value of cur_pos_col and cur_pos_row.*/
    uint16_t cur_pos = cur_pos_row * WIDTH + cur_pos_col;

	outb(VGA_INDEX_REG_PORT, 0x0F); // Set the low byte of the cursor position
	outb(VGA_DATA_REG_PORT, (uint8_t) (cur_pos & 0xFF));

	outb(VGA_INDEX_REG_PORT, 0x0E); // Set the high byte of the cursor position
	outb(VGA_DATA_REG_PORT, (uint8_t) ((cur_pos >> 8) & 0xFF));
}

void move_cursor_up() {
    if(cur_pos_row > 0){
        cur_pos_row--;
    }else if(cur_pos_row <= 0){
        // Fixed in the first row
        cur_pos_row = 0;
    }
    move_cursor();
}

void move_cursor_down() {
    if (cur_pos_row < HEIGHT - 1) {
        cur_pos_row++;
    }else{
        // Fixed in the last row
        cur_pos_row = HEIGHT;
    }
    move_cursor();
}

void move_cursor_left() {
    if (cur_pos_col > 2) {
        cur_pos_col--;
    }
    move_cursor();
}

void move_cursor_right(int max_right_pos) {
    if (cur_pos_col < max_right_pos) {
        cur_pos_col++;
    }
    move_cursor();
}

void cursor_offset(int row, int col){
    cur_pos_col += col;
    cur_pos_row += row;
    move_cursor();
}

void vga_clear(){
    cur_pos_col = 0;
    cur_pos_row = 0;
    // Block Cursor start 0x00, end 0x0F
    enable_cursor(0x0E, 0x0F);  // Underline Cursor
    set_cursor_pos(cur_pos_col, cur_pos_row);

    current_color = default_color;

    for(int row = 0; row < HEIGHT - 1; row++){
        for(int col = 0; col < WIDTH - 1; col++){
            vgamem[row * WIDTH + col] = current_color | ' '; // clearing each row and column
        }
    }
}


void create_newline(){
    if(cur_pos_row >= 24){
        scroll_up();
    }else{
        cur_pos_row++;    // Go to next line
    }
    cur_pos_col = 0;      // place cursor at the start of new line
}


void scroll_up(){
    // Scrolling up each line
    for(int row = 1; row < 26; row++){
        for(int col = 0; col < (WIDTH - 1); col++){
            vgamem[(row - 1) * WIDTH + col] = vgamem[row * WIDTH + col]; 
        }
    }
    
    // Go to last line
    cur_pos_row = 24;
    cur_pos_col = 0;

    // Creating a blank line in last row
    for( int col = 0; col < (WIDTH - 1); col++){
        vgamem[cur_pos_row * WIDTH + col] = current_color | ' ';
    }
}

void backspace_manage(){
    // Handel backspace
    if(cur_pos_row <= 0 && cur_pos_col <= 0){ // Cursor never going beyond its start position
        cur_pos_col = 0;
        cur_pos_row = 0;
        move_cursor();
    }else{
        cur_pos_col--;
        vgamem[cur_pos_row * WIDTH + cur_pos_col] = current_color |  ' '; // Clearing former column
    }
    move_cursor();
}

void del_manage() {
    // Check if the current position is valid
    if (cur_pos_col >= WIDTH - 1 && cur_pos_row >= HEIGHT - 1) {
        return; // Cursor at the last cell, nothing to delete
    }

    // Calculate the current position offset
    int current_offset = cur_pos_row * WIDTH + cur_pos_col;

    // Shift all characters on the current line to the left
    for (int col = cur_pos_col; col < WIDTH - 1; col++) {
        // Get the next character in the line
        vgamem[cur_pos_row * WIDTH + col] = vgamem[cur_pos_row * WIDTH + col + 1];
    }

    // Place a space at the last column of the current row to clear the last shifted character
    vgamem[cur_pos_row * WIDTH + (WIDTH - 1)] = current_color | ' ';

    // Move the cursor position after the deletion
    move_cursor();
}



// Printing a single character in the VGA screen
// unsigned char => uint8_t
void putchar(unsigned char c){
    /* I am converting character into scancode and then compare the values  exception some cases like \b, \t*/

    current_color = default_color;

    if (c == '\t'){
        // Handle a tab by increasing the cursor's X, but only to a point
        // where it is divisible by 8.
       cur_pos_col = (cur_pos_col + 8) & ~( 8 - 1);
    }else if( c == '\r'){
        // Handel carriage return
        cur_pos_col = 0;
    }else if(c == '\n'){
        // Handel newline
        create_newline();
    }else if(c == '\b'){
        backspace_manage();
    }else if(c == '\0'){
        // Do nothing for end of string otherwise it unnecessary increase column number
    }else{ 
        // Handel any other printable character
        vgamem[cur_pos_row * WIDTH + cur_pos_col] = current_color | c;
        cur_pos_col++;
    }


    // Manage Cursor position
    // Check if we need to insert a new line because we have reached the end
    // of the screen.
    if (cur_pos_col >= WIDTH - 1)
    {
        cur_pos_row++;
        cur_pos_col = 0;
    }

    if(cur_pos_row >= 25 ){
        // Scroll the screen if needed.
        scroll_up();
    }
    

    // Move the hardware cursor.
    move_cursor();
}


void putchar_at(char c, int col, int row){
    cur_pos_col = col;
    cur_pos_row = row;
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


void print_at(char* s, int col, int row){
    cur_pos_col = col;
    cur_pos_row = row;
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


void print_hex_at(uint32_t n, int col, int row){
    char hex_chars[] = "0123456789ABCDEF";
    print_at("0x", col, row);
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


void print_dec_at(uint32_t n, int col, int row){
    if (n == 0)
    {
        putchar_at('0', col, row);
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

    cur_pos_col -= get_cursor_position_col() - col;
    cur_pos_row -= get_cursor_position_row() - row;
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



// Print a binary number in a specific position
void print_bin_at(uint32_t n, int col, int row){
    print_at("0b", col, row);

     // Loop through all 32 bits of the number, starting from the most significant bit (31)
    for (int i = 31; i >= 0; i--) {
        uint32_t bit = (n >> i) & 1;  // Extract the i-th bit (0 or 1)
        putchar(bit ? '1' : '0');     // Print '1' or '0'
    }
}

void print_all_cell(char c){
    for(int row = 0; row < HEIGHT - 1; row++){
        for(int col = 0; col < WIDTH - 1; col++){
            putchar_at(c, col, row);
        }
    }
}


vga_cell_t get_vga_cell(int row, int col) {
    vga_cell_t cell;
    
    // Calculate the offset in the VGA buffer
    int offset = row * WIDTH + col;

    // Get the 16-bit value at the calculated offset
    uint16_t value = vgamem[offset];

    // The first byte is the ASCII character
    cell.character = (char)(value & 0x00FF);

    // The second byte is the color attribute
    cell.color = (uint8_t)((value >> 8) & 0xFF);

    return cell;
}