
#include "../stdlib/stdint.h"



#define COM1_PORT 0x3F8             //COM1 (Serial Port)
#define COM2_PORT 0x2F8             //COM2 (Serial Port)
#define LPT1_PORT 0x278             //LPT1 PORT


#define ATA_HD_PORT 0x1F7           //Primary ATA/IDE Hard Drive Ports


#define PCI_CONFIG_ADD_PORT 0xCF8   //PCI Configuration Address Port
#define PCI_CONFIG_DATA_PORT 0xCFC  //PCI Configuration Data Port

#define SPEAKER_CONTROL_PORT 0x61




void outb(uint16_t port, uint8_t value);
uint8_t inb(uint16_t port);

void outw(uint16_t port, uint16_t value);
uint16_t inw(uint16_t port);








