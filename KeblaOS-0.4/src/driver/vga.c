
#include "vga.h"
#include "../stdlib/stdint.h"
#include "../stdlib/string.h"
#include "../kernel/util.h"
#include "../interrupts/timer.h"

uint16_t column = 0;
uint16_t line = 0;
uint16_t* const vga = (uint16_t* const) 0xC00B8000;
const uint16_t defaultColor = (COLOR8_RED << 8) | (COLOR8_BLACK << 12);
uint16_t currentColor = defaultColor;

void Reset(){
    line = 0;
    column = 0;
    currentColor = defaultColor;

    for (uint16_t y = 0; y < height; y++){
        for (uint16_t x = 0; x < width; x++){
            vga[y * width + x] = ' ' | defaultColor;
        }
    }
}

void newLine(){
    if (line < height - 1){
        line++;
        column = 0;
    }else{
        scrollUp();
        column = 0;
    }
}

void scrollUp(){
    for (uint16_t y = 0; y < height; y++){
        for (uint16_t x = 0; x < width; x++){
            vga[(y-1) * width + x] = vga[y*width+x];
        }
    }

    for (uint16_t x = 0; x < width; x++){
        vga[(height-1) * width + x] = ' ' | currentColor;
    }
}

void print(const char* s){
    while(*s){
        switch(*s){
            case '\n':
                newLine();
                break;
            case '\r':
                column = 0;
                break;
            case '\b':
                if (column == 0 && line != 0){
                    line--;
                    column = width;
                }
                vga[line * width + (--column)] = ' ' | currentColor;
                break;
            case '\t':
                if (column == width){
                    newLine();
                }
                uint16_t tabLen = 4 - (column % 4);
                while (tabLen != 0){
                    vga[line * width + (column++)] = ' ' | currentColor;
                    tabLen--;
                }
                break;
            default:
                if (column == width){
                    newLine();
                }

                vga[line * width + (column++)] = *s | currentColor;
                break;
        }
        s++;
    }
     update_cursor(column, line+1);
}

void enable_cursor(uint8_t cursor_start, uint8_t cursor_end)
{
	outPortB(0x3D4, 0x0A);
	outPortB(0x3D5, (inPortB(0x3D5) & 0xC0) | cursor_start);

	outPortB(0x3D4, 0x0B);
	outPortB(0x3D5, (inPortB(0x3D5) & 0xE0) | cursor_end);
}

void disable_cursor()
{
	outPortB(0x3D4, 0x0A);
	outPortB(0x3D5, 0x20);
}

void update_cursor(int x, int y)
{
	uint16_t pos = y * width + x;

	outPortB(0x3D4, 0x0F);
	outPortB(0x3D5, (uint8_t) (pos & 0xFF));
	outPortB(0x3D4, 0x0E);
	outPortB(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}

uint16_t get_cursor_position(void)
{
    uint16_t pos = 0;
    outPortB(0x3D4, 0x0F);
    pos |= inPortB(0x3D5);
    outPortB(0x3D4, 0x0E);
    pos |= ((uint16_t)inPortB(0x3D5)) << 8;
    return pos;
}



void delay_print(const char* s) {
    while (*s) {
        switch (*s) {
            case '\n':
                newLine();
                break;
            case '\r':
                column = 0;
                break;
            case '\b':
                if (column == 0 && line != 0) {
                    line--;
                    column = width;
                }
                vga[line * width + (--column)] = ' ' | currentColor;
                break;
            case '\t': {
                if (column == width) {
                    newLine();
                }
                uint16_t tabLen = 4 - (column % 4);
                while (tabLen != 0) {
                    vga[line * width + (column++)] = ' ' | currentColor;
                    tabLen--;
                }
                break;
            }
            default:
                if (column == width) {
                    newLine();
                }
                vga[line * width + (column++)] = *s | currentColor;
                break;
        }
        s++;
        update_cursor(column, line);
        delay(100);  // Introduce delay (100 ms, adjust as needed)
    }
     update_cursor(column, line+1);
}
