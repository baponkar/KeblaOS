
#include "../stdlib/stdint.h"


// Defining some ports with port number for future use

//I/O Ports (Input/Output Ports)
#define KEYBOARD_COMMAND_PORT 0x64  // Keyboard Command Port
#define KEYBOARD_DATA_PORT 0x60     // Keyboard Data Port

#define COM1_PORT 0x3F8             //COM1 (Serial Port)
#define COM2_PORT 0x2F8             //COM2 (Serial Port)
#define LPT1_PORT 0x278             //LPT1 PORT

#define PIC1_COMMAND_PORT 0x20      //Primary PIC(programmable interrupt controller) Command Port:
#define PIC1_DATA_PORT 0x21         //Primary PIC Data Port
#define PIC2_COMMAND_PORT 0xA0      //Secondary PIC Command Port:
#define PIC2_DATA_PORT 0xA1         //Secondary PIC Data Port

#define RTC_COMMAND_PORT 0x70       //Real-Time Clock (RTC) Command Ports
#define RTC_DATA_PORT 0x71          //Real-Time Clock (RTC) Data Ports

#define ATA_HD_PORT 0x1F7           //Primary ATA/IDE Hard Drive Ports

#define VGA_INDEX_REG_PORT 0x3D4    //VGA CRT Controller Index Register
#define VGA_DATA_REG_PORT 0x3D5     //0x3D5: VGA CRT Controller Data Register

#define PCI_CONFIG_ADD_PORT 0xCF8   //PCI Configuration Address Port
#define PCI_CONFIG_DATA_PORT 0xCFC  //PCI Configuration Data Port

#define SPEAKER_CONTROL_PORT 0x61
#define PIT_CHANNEL_0 0x40          // Programmable Interval Timer
#define PIT_CHANNEL_1 0x41
#define PIT_CHANNEL_2 0x42          // Generate Sound
#define PIT_COMMAND_PORT 0x43



void outb(uint16_t port, uint8_t value);
uint8_t inb(uint16_t port);

void outw(uint16_t port, uint16_t value);
uint16_t inw(uint16_t port);








