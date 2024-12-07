
#include "keyboard.h"

#define BUFFER_SIZE 128 // Max Size of Command Buffer
char COMMAND_BUFFER[BUFFER_SIZE];   // Command string Container Buffer
int BUFFER_INDEX = 0;   // Current Length of Command String Buffer

uint32_t scanCode;      // What key is pressed
bool press;             // Press down, or released

bool shift = false;     // Shift Key pressed or not
bool capsLock = false;  // Caps Lock Key pressed or not

// Scan Code values
const uint32_t UNKNOWN  = 0xFFFFFFFF;

const uint32_t ESC       = 0x00000001;
const uint32_t NUM_1     = 0x00000002;
const uint32_t NUM_2     = 0x00000003;
const uint32_t NUM_3     = 0x00000004;
const uint32_t NUM_4     = 0x00000005;
const uint32_t NUM_5     = 0x00000006;
const uint32_t NUM_6     = 0x00000007;
const uint32_t NUM_7     = 0x00000008;
const uint32_t NUM_8     = 0x00000009;
const uint32_t NUM_9     = 0x0000000A;
const uint32_t NUM_0     = 0x0000000B;
const uint32_t MINUS     = 0x0000000C; // '-'
const uint32_t EQUAL     = 0x0000000D; // '='
const uint32_t BACKSPACE = 0x0000000E;
const uint32_t TAB       = 0x0000000F;
const uint32_t Q         = 0x00000010;
const uint32_t W         = 0x00000011;
const uint32_t E         = 0x00000012;
const uint32_t R         = 0x00000013;
const uint32_t T         = 0x00000014;
const uint32_t Y         = 0x00000015;
const uint32_t U         = 0x00000016;
const uint32_t I         = 0x00000017;
const uint32_t O         = 0x00000018;
const uint32_t P         = 0x00000019;
const uint32_t LBRACKET  = 0x0000001A; // '['
const uint32_t RBRACKET  = 0x0000001B; // ']'
const uint32_t ENTER     = 0x0000001C;
const uint32_t CTRL      = 0x0000001D;
const uint32_t A         = 0x0000001E;
const uint32_t S         = 0x0000001F;
const uint32_t D         = 0x00000020;
const uint32_t F         = 0x00000021;
const uint32_t G         = 0x00000022;
const uint32_t H         = 0x00000023;
const uint32_t J         = 0x00000024;
const uint32_t K         = 0x00000025;
const uint32_t L         = 0x00000026;
const uint32_t SEMICOLON = 0x00000027; // ';'
const uint32_t APOSTROPHE= 0x00000028; // '\''
const uint32_t BACKTICK  = 0x00000029; // '`'
const uint32_t LSHIFT    = 0x0000002A;
const uint32_t BACKSLASH = 0x0000002B; // '\'
const uint32_t Z         = 0x0000002C;
const uint32_t X         = 0x0000002D;
const uint32_t C         = 0x0000002E;
const uint32_t V         = 0x0000002F;
const uint32_t B         = 0x00000030;
const uint32_t N         = 0x00000031;
const uint32_t M         = 0x00000032;
const uint32_t COMMA     = 0x00000033; // ','
const uint32_t PERIOD    = 0x00000034; // '.'
const uint32_t SLASH     = 0x00000035; // '/'
const uint32_t RSHIFT    = 0x00000036;
const uint32_t NUMPAD_ASTERISK = 0x00000037; // '*'
const uint32_t ALT      = 0x00000038;
const uint32_t SPACE     = 0x00000039;
const uint32_t CAPS_LOCK = 0x0000003A;

const uint32_t F1        = 0x0000003B;
const uint32_t F2        = 0x0000003C;
const uint32_t F3        = 0x0000003D;
const uint32_t F4        = 0x0000003E;
const uint32_t F5        = 0x0000003F;
const uint32_t F6        = 0x00000040;
const uint32_t F7        = 0x00000041;
const uint32_t F8        = 0x00000042;
const uint32_t F9        = 0x00000043;
const uint32_t F10       = 0x00000044;
const uint32_t NUM_LOCK  = 0x00000045;
const uint32_t SCROLL_LOCK = 0x00000046;

const uint32_t NUMPAD_7  = 0x00000047;
const uint32_t NUMPAD_8  = 0x00000048;
const uint32_t NUMPAD_9  = 0x00000049;
const uint32_t NUMPAD_MINUS = 0x0000004A;
const uint32_t NUMPAD_4  = 0x0000004B;
const uint32_t NUMPAD_5  = 0x0000004C;
const uint32_t NUMPAD_6  = 0x0000004D;
const uint32_t NUMPAD_PLUS = 0x0000004E;
const uint32_t NUMPAD_1  = 0x0000004F;
const uint32_t NUMPAD_2  = 0x00000050;
const uint32_t NUMPAD_3  = 0x00000051;
const uint32_t NUMPAD_0  = 0x00000052;
const uint32_t NUMPAD_PERIOD = 0x00000053; // '.'

const uint32_t F11       = 0x00000057;
const uint32_t F12       = 0x00000058;

const uint32_t HOME      = 0x00000047; // Same as NUMPAD_7 without Num Lock
const uint32_t UP        = 0x00000048; // Same as NUMPAD_8 without Num Lock
const uint32_t PAGE_UP   = 0x00000049; // Same as NUMPAD_9 without Num Lock
const uint32_t LEFT      = 0x0000004B; // Same as NUMPAD_4 without Num Lock
const uint32_t RIGHT     = 0x0000004D; // Same as NUMPAD_6 without Num Lock
const uint32_t END       = 0x0000004F; // Same as NUMPAD_1 without Num Lock
const uint32_t DOWN      = 0x00000050; // Same as NUMPAD_2 without Num Lock
const uint32_t PAGE_DOWN = 0x00000051; // Same as NUMPAD_3 without Num Lock
const uint32_t INSERT    = 0x00000052; // Same as NUMPAD_0 without Num Lock
const uint32_t DELETE    = 0x00000053; // Same as NUMPAD_PERIOD without Num Lock


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
            beep();
            break;
        case ENTER:  // Enter Key Manage
            handel_enter_key(keyPress);
            break;
        case CTRL:      // CTRL
        case ALT:       // ALT
        case F1:        // F1
        case F2:        // F2
        case F3:        // F3
        case F4:        // F4
        case F5:        // F5
        case F6:        // F6
        case F7:        // F7
        case F8:        // F8
        case F9:        // F9
        case F10:       // F10
        case DELETE:    // DELETEete key
            handel_del_key(keyPress);
        case F11:       // F11
        case F12:       // F12
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
                move_cursor_left();
            }
            break;
        case RIGHT:
            if(keyPress == true){
                move_cursor_right(BUFFER_INDEX + 2);
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
                putchar(scanCodeToChar(scanCode));
                COMMAND_BUFFER[BUFFER_INDEX] = scanCodeToChar(scanCode);
                BUFFER_INDEX++;
            }
            break;   
    }
}

void handel_enter_key(bool keyPressed){
    if(keyPressed == false){
        create_newline();
        execute_command(COMMAND_BUFFER);
        clear_buffer(COMMAND_BUFFER, BUFFER_SIZE);
        BUFFER_INDEX = 0;
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
    if(keyPressed == true){
        backspace_manage();
        BUFFER_INDEX--;
        COMMAND_BUFFER[BUFFER_INDEX] = '\0';
    }
}

void handel_del_key(bool keyPressed){
    int cur_pos_col;
    if(keyPressed == true){
        del_manage();   // updating screen
        cur_pos_col = get_cursor_position_col();
        for(int i=cur_pos_col;i<BUFFER_INDEX;i++){
            COMMAND_BUFFER[i] = COMMAND_BUFFER[i+1]; // Shifting left 
        }
        BUFFER_INDEX--;
    }
}

void keyboardHandler(registers_t *regs){
    scanCode =  getScanCode();  // What key is pressed
    press = getKeyState();      // Manage Key Pressed or Released by changing bool variable press
    key_ctrl(scanCode,  press);
}




void initKeyboard(){
    disable_interrupts();
    interrupt_install_handler(1, &keyboardHandler);
    enable_interrupts();
}

void disableKeyboard(){
    interrupt_uninstall_handler(1);
}




void read_command(char* input) {
    BUFFER_INDEX = 0;  // Start fresh
    while (true) {
        // The keyboard handler should populate COMMAND_BUFFER based on key presses
        if (COMMAND_BUFFER[BUFFER_INDEX] == '\n') {
            // When Enter is pressed, break the loop
            input[BUFFER_INDEX] = '\0';  // Null-terminate the input
            break;
        }
    }
}