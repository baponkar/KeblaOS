#include "keyboard.h"

int getScanCode(){
    int scancode = inb(0x60) & 0x7F;
    print_dec(scancode);
    return scancode;
}

bool getKeyState(){
    char key_state = (char) inb(0x60) & 0x80;    // Press down return 0x0000000, or released return 0xfffff80

    // Key Released
    if(key_state == 0xFFFFFF80){
       return false;
    }
    // Key Pressed
    else{
        return true;
    }
}

void keyboard_handler(registers_t *reg){
    int scancode = getScanCode();
    bool keystate = getKeyState();
    print("Scan code: ");
    print_dec(scancode);
    print("\n");
    print("Keystate: ");
    print_dec(keystate);
    print("\n");
}

void init_keyboard(){
    disable_interrupts();
    interrupt_install_handler(1, &keyboard_handler);
    enable_interrupts();
}



