/*
https://wiki.osdev.org/PS/2_Keyboard
https://github.com/dreamportdev/Osdev-Notes/blob/master/02_Architecture/09_Add_Keyboard_Support.md
*/

#include "../../x86_64/interrupt/pic.h"
#include "../../x86_64/interrupt/apic.h"
#include "../../x86_64/interrupt/ioapic.h"
#include "../../x86_64/interrupt/interrupt.h"
#include "../../x86_64/interrupt/irq_manage.h"
#include "../../lib/stdlib.h"
#include "../../lib/string.h"
#include "../../lib/stdio.h"
#include "../../kshell/ring_buffer.h"
#include "../../driver/speaker/speaker.h"
#include "../../driver/io/ports.h"
#include "../../driver/vga/vga_term.h"

#include "keyboard.h"

#define KEYBOARD_BUF_SIZE 128   // Ring buffer capacity for keystrokes
#define KEYBOARD_INT_VECTOR 33
#define KEYBOARD_IRQ 1          // 33 - 32

ring_buffer_t* keyboard_buffer;


uint32_t scanCode;      // What key is pressed
bool press;             // Press down, or released

bool shift = false;     // Shift Key pressed or not
bool capsLock = false;  // Caps Lock Key pressed or not

int entered_keys = 0;


const uint32_t lowercase[128] = {
    UNKNOWN,ESC,'1','2','3','4','5','6','7','8',
    '9','0','-','=','\b','\t','q','w','e','r',
    't','y','u','i','o','p','[',']','\n',CTRL,
    'a','s','d','f','g','h','j','k','l',';',
    '\'','`',LSHIFT,'\\','z','x','c','v','b','n',
    'm',',','.','/',RSHIFT,'*',ALT,' ',CAPS_LOCK,F1,
    F2,F3,F4,F5,F6,F7,F8,F9,F10,NUM_LOCK,SCROLL_LOCK,
    HOME,UP,PAGE_UP,'-',LEFT,UNKNOWN,RIGHT,'+',END,
    DOWN,PAGE_DOWN,INSERT,DELETE,UNKNOWN,UNKNOWN,UNKNOWN,F11,F12,UNKNOWN,
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN
};
const uint32_t uppercase[128] = {
    UNKNOWN,ESC,'!','@','#','$','%','^','&','*',
    '(',')','_','+','\b','\t','Q','W','E','R',
    'T','Y','U','I','O','P','{','}','\n',CTRL,
    'A','S','D','F','G','H','J','K','L',':',
    '"','~',LSHIFT,'|','Z','X','C','V','B','N',
    'M','<','>','?',RSHIFT,'*',ALT,' ',CAPS_LOCK,F1,
    F2,F3,F4,F5,F6,F7,F8,F9,F10,NUM_LOCK,SCROLL_LOCK,
    HOME,UP,PAGE_UP,'-',LEFT,UNKNOWN,RIGHT,'+',END,
    DOWN,PAGE_DOWN,INSERT,DELETE,UNKNOWN,UNKNOWN,UNKNOWN,F11,F12,UNKNOWN,
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN
};


int getScanCode(){
    return inb(0x60) & 0x7F;
}


bool getKeyState(){
    char key_state = (char) inb(0x60) & 0x80;    // Press down return 0x0000000, or released return 0xffffff80

    // Key Released
    if(key_state == 0xFFFFFF80){
       return false;
    }
    // Key Pressed
    else{
        return true;
    }
}


char scanCodeToChar(uint32_t scanCode) {
    if(scanCode == 0xE0){
        scanCode = (scanCode | 0x100); // Example: Combine with a base to indicate extended
    } 

    if (scanCode >= NUM_1 && scanCode <= NUM_0) {
        // Handles NUM_0 to NUM_9, using the shift flag
        return shift ? uppercase[scanCode] : lowercase[scanCode];
    } else {
        // For other keys, check capsLock or other flags
        return capsLock ? uppercase[scanCode] : lowercase[scanCode];
    }
}


void key_ctrl(uint32_t scanCode, bool keyPress){
    switch(scanCode){
        case 0x00000060: // Extra scan code returning
            break;
        case 0x00000608: // Extra scan code returning
            break;
        case UNKNOWN:
        case ESC:     // ESC Key
            // beep();
            break;
        case ENTER:  // Enter Key Manage
            // handel_enter_key(keyPress);
            entered_keys = 0; 
            break;
        case CTRL:      // CTRL
            break;
        case ALT:       // ALT
            break;
        case F1:        // F1
            break;
        case F2:        // F2
            break;
        case F3:        // F3
            break;
        case F4:        // F4
            break;
        case F5:        // F5
            break;
        case F6:        // F6
            break;
        case F7:        // F7
            break;
        case F8:        // F8
            break;
        case F9:        // F9
            break;
        case F10:       // F10
            break;
        case DELETE:    // DELETEete key
            handel_del_key(keyPress);
        case F11:       // F11
            break;
        case F12:       // F12
            break;
        case LSHIFT:    // Left shift key
            handel_shift_key(keyPress);
            break;
        case RSHIFT:    // Right shift key
            handel_shift_key(keyPress);
            break;
        case CAPS_LOCK:    // Caps Lock Key
            handel_caps_lock_key(keyPress);
            break;
        case UP:
            // if(press == true){
            //     move_cursor_up();
            // }
            break;
        case LEFT:
            if(keyPress == true){
                move_cur_left();
            }
            break;
        case RIGHT:
            if(keyPress == true){
                move_cur_right(get_cursor_pos_x() + 2);
            }
            break;
        case DOWN:
            // if(keyPress == true){
            //     move_cursor_down();
            // }
            break;
        case BACKSPACE:
            handel_backspace_key(keyPress);
            break;
        default:
            if(keyPress == true){
                entered_keys++; // to track how many keys pressed
                putchar(scanCodeToChar(scanCode));
            }
            break;   
    }
    apic_send_eoi();
}


void handel_enter_key(bool keyPressed){
    if(keyPressed == false){
        create_newline();
    }
}

void handel_shift_key(bool keyPressed){
    shift = keyPressed;
    capsLock = !capsLock;
}

void handel_caps_lock_key(bool keyPressed){
    if(keyPressed == true){
        capsLock = !capsLock;
    }
}

void handel_backspace_key(bool keyPressed){
    if(keyPressed == true && entered_keys > 0){
        backspace_manage();
        entered_keys--;
    }
}

void handel_del_key(bool keyPressed){
    int cur_pos_col;
    if(keyPressed == true){
        //del_manage();   // updating screen
        cur_pos_col = get_cursor_pos_x() - 2; // 2 for cursor size
    }
}


void keyboardHandler(registers_t *regs){
    scanCode =  getScanCode();  // What key is pressed
    press = getKeyState();      // Manage Key Pressed or Released by changing bool variable press
    key_ctrl(scanCode,  press);
    
    if(press && keyboard_buffer){
        ring_buffer_push(keyboard_buffer, scanCodeToChar(scanCode));    // Storing character into keyboard_buffer
    }
}


void initKeyboard(){
    asm volatile("cli");
    
    // uint32_t flag = IOAPIC_EDGE_TRIG | IOAPIC_HIGH_ACTIVE | IOAPIC_FIXED | IOAPIC_UNMASKED;
    // ioapic_route_irq(KEYBOARD_IRQ, 0, KEYBOARD_INT_VECTOR, flag );

    enableKeyboard();
    asm volatile("sti");
    printf(" [-] Successfully KEYBOARD initialized.\n");
}

void enableKeyboard(){
    irq_install(KEYBOARD_IRQ, (void *) &keyboardHandler);
}

void disableKeyboard(){
    irq_uninstall(KEYBOARD_IRQ);
}







