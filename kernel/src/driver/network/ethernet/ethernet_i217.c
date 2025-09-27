/*
Description: I'm writing this driver to control QEmu supplied PCI scan found device
             Vendor ID: 0x8086, Device ID: 0x10D3, Class: 0x2, Subclass: 0x0 (82574L Gigabit Network Connection)
Developer: Bapon Kar
Last Update: 16.09.2025
Reference: 
            1. https://pcilookup.com
            2. https://wiki.osdev.org/Intel_Ethernet_i217
            3. https://www.alldatasheet.com/html-pdf/522393/INTEL/82574L/149/1/82574L.html

*/

#include "../../../memory/kmalloc.h"
#include "../../../memory/kheap.h"
#include "../../../sys/timer/apic_timer.h"

#include "../../../lib/stdio.h"
#include "../../../lib/string.h"

#include "utility.h"

#include  "ethernet_i217.h"

extern pci_device_t *network_controllers;   // Array to store detected network controllers
extern size_t network_controller_count;     // Counter for network controllers



uint8_t bar_type;     // Type of BAR0
uint16_t io_base;     // IO Base Address
uint64_t  mem_base;   // MMIO Base Address
bool eerprom_exists;  // A flag indicating if eeprom exists
uint8_t mac [6];      // A buffer for storing the mack address

struct e1000_rx_desc *rx_descs[E1000_NUM_RX_DESC]; // Receive Descriptor Buffers
struct e1000_tx_desc *tx_descs[E1000_NUM_TX_DESC]; // Transmit Descriptor Buffers

uint16_t rx_cur;      // Current Receive Descriptor Buffer
uint16_t tx_cur;      // Current Transmit Descriptor Buffer

void writeCommand( uint16_t p_address, uint32_t p_value)
{
    if ( bar_type == 0 )
    {
         write32(mem_base+p_address,p_value);
    }
    else
    {
        outl(io_base, p_address);
        outl(io_base + 4, p_value);
    }
}

uint32_t readCommand( uint16_t p_address)
{
    if ( bar_type == 0 )
    {
        return  read32(mem_base+p_address);
    }
    else
    {
        outl(io_base, p_address);
        return inl(io_base + 4);
    }
}



bool detectEEProm()
{
    uint32_t val = 0;
    writeCommand(REG_EEPROM, 0x1); 

    for(int i = 0; i < 1000 && ! eerprom_exists; i++)
    {
        val = readCommand( REG_EEPROM);
        if(val & 0x10){
            eerprom_exists = true;
        }else{
            eerprom_exists = false;
        }
    }
    return eerprom_exists;
}

uint32_t eepromRead( uint8_t addr)
{
	uint16_t data = 0;
	uint32_t tmp = 0;
        if ( eerprom_exists)
        {
            writeCommand( REG_EEPROM, (1) | ((uint32_t)(addr) << 8) );
        	while( !((tmp = readCommand(REG_EEPROM)) & (1 << 4)) );
        }
        else
        {
            writeCommand( REG_EEPROM, (1) | ((uint32_t)(addr) << 2) );
            while( !((tmp = readCommand(REG_EEPROM)) & (1 << 1)) );
        }
	data = (uint16_t)((tmp >> 16) & 0xFFFF);
	return data;
}

bool readMACAddress()
{
    if ( eerprom_exists)
    {
        uint32_t temp;
        temp = eepromRead( 0);
        mac[0] = temp &0xff;
        mac[1] = temp >> 8;
        temp = eepromRead( 1);
        mac[2] = temp &0xff;
        mac[3] = temp >> 8;
        temp = eepromRead( 2);
        mac[4] = temp &0xff;
        mac[5] = temp >> 8;
    }
    else
    {
        uint8_t *mem_base_mac_8 = (uint8_t *) (mem_base+0x5400);
        uint32_t *mem_base_mac_32 = (uint32_t *) (mem_base+0x5400);
        if ( mem_base_mac_32[0] != 0 )
        {
            for(int i = 0; i < 6; i++)
            {
                mac[i] = mem_base_mac_8[i];
            }
        }
        else return false;
    }
    return true;
}


void rxinit()
{
    uint8_t * ptr;
    struct e1000_rx_desc *descs;

    // Allocate buffer for receive descriptors. For simplicity, in my case khmalloc returns a virtual address that is identical to it physical mapped address.
    // In your case you should handle virtual and physical addresses as the addresses passed to the NIC should be physical ones
 
    ptr = (uint8_t *)(kmalloc(sizeof(struct e1000_rx_desc)*E1000_NUM_RX_DESC + 16));

    descs = (struct e1000_rx_desc *)ptr;
    for(int i = 0; i < E1000_NUM_RX_DESC; i++)
    {
        rx_descs[i] = (struct e1000_rx_desc *)((uint8_t *)descs + i*16);
        rx_descs[i]->addr = (uint64_t)(uint8_t *)(kmalloc(8192 + 16));
        rx_descs[i]->status = 0;
    }

    writeCommand(REG_TXDESCLO, (uint32_t)((uint64_t)ptr >> 32) );
    writeCommand(REG_TXDESCHI, (uint32_t)((uint64_t)ptr & 0xFFFFFFFF));

    writeCommand(REG_RXDESCLO, (uint64_t)ptr);
    writeCommand(REG_RXDESCHI, 0);

    writeCommand(REG_RXDESCLEN, E1000_NUM_RX_DESC * 16);

    writeCommand(REG_RXDESCHEAD, 0);
    writeCommand(REG_RXDESCTAIL, E1000_NUM_RX_DESC-1);
    rx_cur = 0;
    writeCommand(REG_RCTRL, RCTL_EN| RCTL_SBP| RCTL_UPE | RCTL_MPE | RCTL_LBM_NONE | RTCL_RDMTS_HALF | RCTL_BAM | RCTL_SECRC  | RCTL_BSIZE_8192);
    
}


void txinit()
{    
    uint8_t *  ptr;
    struct e1000_tx_desc *descs;
    // Allocate buffer for receive descriptors. For simplicity, in my case khmalloc returns a virtual address that is identical to it physical mapped address.
    // In your case you should handle virtual and physical addresses as the addresses passed to the NIC should be physical ones
    ptr = (uint8_t *)(kmalloc(sizeof(struct e1000_tx_desc)*E1000_NUM_TX_DESC + 16));

    descs = (struct e1000_tx_desc *)ptr;
    for(int i = 0; i < E1000_NUM_TX_DESC; i++)
    {
        tx_descs[i] = (struct e1000_tx_desc *)((uint8_t*)descs + i*16);
        tx_descs[i]->addr = 0;
        tx_descs[i]->cmd = 0;
        tx_descs[i]->status = TSTA_DD;
    }

    writeCommand(REG_TXDESCHI, (uint32_t)((uint64_t)ptr >> 32) );
    writeCommand(REG_TXDESCLO, (uint32_t)((uint64_t)ptr & 0xFFFFFFFF));


    //now setup total length of descriptors
    writeCommand(REG_TXDESCLEN, E1000_NUM_TX_DESC * 16);


    //setup numbers
    writeCommand( REG_TXDESCHEAD, 0);
    writeCommand( REG_TXDESCTAIL, 0);
    tx_cur = 0;
    writeCommand(REG_TCTRL,  TCTL_EN | TCTL_PSP | (15 << TCTL_CT_SHIFT) | (64 << TCTL_COLD_SHIFT) | TCTL_RTLC);

    // This line of code overrides the one before it but I left both to highlight that the previous one works with e1000 cards, but for the e1000e cards 
    // you should set the TCTRL register as follows. For detailed description of each bit, please refer to the Intel Manual.
    // In the case of I217 and 82577LM packets will not be sent if the TCTRL is not configured using the following bits.
    writeCommand(REG_TCTRL,  0b0110000000000111111000011111010);
    writeCommand(REG_TIPG,  0x0060200A);

}

void enableInterrupt()
{
    writeCommand(REG_IMASK ,0x1F6DC);
    writeCommand(REG_IMASK ,0xff & ~4);
    readCommand(0xc0);

}

void E1000(pci_device_t device)
{
    // Get BAR0 type, io_base address and MMIO base address
    uint32_t bar0 = device.base_address_registers[0];

    if (bar0 & 0x1) {
        // I/O space
        printf("BAR0 is IO space\n");
    } else {
        // Memory space
        printf("BAR0 is MMIO space\n");
    }

    bar_type = bar0 & 0x1;
    io_base = bar0 & ~0x3;  // mask lower 2 bits
    mem_base = bar0 & ~0xF;  // mask lower 4 bits

    eerprom_exists = false;
}

void printMac(){
    printf("MAC: %x:%x:%x:%x:%x:%x\n",
        mac[0],
        mac[1],
        mac[2],
        mac[3],
        mac[4],
        mac[5]
    );
}

void startLink() {
    uint32_t ctrl = readCommand(REG_CTRL);

    // Enable Auto speed detection, Set Link Up, Full Duplex
    ctrl |= CTRL_SLU | CTRL_ASDE | CTRL_FD;
    writeCommand(REG_CTRL, ctrl);

    // Enable receiver
    uint32_t rctl = readCommand(REG_RCTL);
    rctl |= RCTL_EN;
    writeCommand(REG_RCTL, rctl);

    // Enable transmitter
    uint32_t tctl = readCommand(REG_TCTL);
    tctl |= TCTL_EN;
    writeCommand(REG_TCTL, tctl);

    // (Optional) Wait for link status
    uint32_t status = readCommand(REG_STATUS);
    if (status & (1 << 1)) { // STATUS.LU = Link Up
        printf("E1000: Link is up!\n");
    } else {
        printf("E1000: Link is down...\n");
    }
}


bool start ()
{
    E1000(network_controllers[0]);
    detectEEProm ();
    if (! readMACAddress()) return false;
    printMac();
    startLink();
    
    for(int i = 0; i < 0x80; i++){
        writeCommand(0x5200 + i*4, 0);
    }
        
    
    enableInterrupt();
    rxinit();
    txinit();        
    printf("E1000 card started\n");

    return true;

}


void handleReceive()
{
    uint16_t old_cur;
    bool got_packet = false;
 
    while((rx_descs[rx_cur]->status & 0x1))
    {
        got_packet = true;
        uint8_t *buf = (uint8_t *)rx_descs[rx_cur]->addr;
        uint16_t len = rx_descs[rx_cur]->length;

        // Here you should inject the received packet into your network stack


        rx_descs[rx_cur]->status = 0;
        old_cur = rx_cur;
        rx_cur = (rx_cur + 1) % E1000_NUM_RX_DESC;
        writeCommand(REG_RXDESCTAIL, old_cur );
    }    
}

void fire ()
{
    /* This might be needed here if your handler doesn't clear interrupts from each device and must be done before EOI if using the PIC.
        Without this, the card will spam interrupts as the int-line will stay high. */
    writeCommand(REG_IMASK, 0x1);
    
    uint32_t status = readCommand(0xc0);
    if(status & 0x04)
    {
        startLink();
    }
    else if(status & 0x10)
    {
        // good threshold
    }
    else if(status & 0x80)
    {
        handleReceive();
    }
    
}



int sendPacket(const void * p_data, uint16_t p_len)
{    
    tx_descs[tx_cur]->addr = (uint64_t)p_data;
    tx_descs[tx_cur]->length = p_len;
    tx_descs[tx_cur]->cmd = CMD_EOP | CMD_IFCS | CMD_RS;
    tx_descs[tx_cur]->status = 0;
    uint8_t old_cur = tx_cur;
    tx_cur = (tx_cur + 1) % E1000_NUM_TX_DESC;
    writeCommand(REG_TXDESCTAIL, tx_cur);   
    while(!(tx_descs[old_cur]->status & 0xff));    
    return 0;
}


void test_e1000()
{
    printf("=== E1000 Test Start ===\n");

    if (!start()) {
        printf("E1000: init failed!\n");
        return;
    }

    printf("E1000: initialized successfully.\n");

    // Build a simple broadcast Ethernet frame (destination FF:FF:FF:FF:FF:FF)
    uint8_t packet[64]; // min Ethernet frame size is 64 bytes
    memset(packet, 0, sizeof(packet));

    // Destination MAC = broadcast
    for (int i = 0; i < 6; i++) packet[i] = 0xFF;

    // Source MAC = our NIC's MAC
    for (int i = 0; i < 6; i++) packet[6 + i] = mac[i];

    // EtherType = 0x0800 (IPv4) just as a placeholder
    packet[12] = 0x08;
    packet[13] = 0x00;

    // Payload = "HelloE1000" (fits inside)
    const char *msg = "HelloE1000";
    memcpy(&packet[14], msg, strlen(msg));

    printf("E1000: Sending test packet...\n");
    sendPacket(packet, sizeof(packet));

    printf("E1000: Packet sent. Waiting for incoming packets...\n");

    // Poll for received packets for a while
    for (int i = 0; i < 100000; i++) {
        handleReceive();
    }

    printf("=== E1000 Test End ===\n");
}
