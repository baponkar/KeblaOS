#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


#include "../../pci/pci.h"


// PCI Class/Subclass for Ethernet controllers
#define PCI_CLASS_NETWORK        0x02
#define PCI_SUBCLASS_ETHERNET    0x00

// Intel Vendor ID
#define INTEL_VENDOR_ID          0x8086

// Intel Device IDs for e1000 family
#define E1000_DEVICE_82540EM    0x100E
#define E1000_DEVICE_82545EM    0x100F
#define E1000_DEVICE_82573E     0x108B
#define E1000_DEVICE_82574L     0x10D3

// Register offsets
#define E1000_REG_CTRL          0x0000
#define E1000_REG_STATUS        0x0008
#define E1000_REG_EEPROM        0x0014
#define E1000_REG_CTRL_EXT      0x0018
#define E1000_REG_IMC           0x00D0
#define E1000_REG_IMS           0x00D4
#define E1000_REG_ICR           0x00C0
#define E1000_REG_RCTL          0x0100
#define E1000_REG_TCTL          0x0400
#define E1000_REG_RDBAL         0x2800
#define E1000_REG_RDBAH         0x2804
#define E1000_REG_RDLEN         0x2808
#define E1000_REG_RDH           0x2810
#define E1000_REG_RDT           0x2818
#define E1000_REG_TDBAL         0x3800
#define E1000_REG_TDBAH         0x3804
#define E1000_REG_TDLEN         0x3808
#define E1000_REG_TDH           0x3810
#define E1000_REG_TDT           0x3818
#define E1000_REG_MAC_LOW       0x5400
#define E1000_REG_MAC_HIGH      0x5404

// Control register bits
#define E1000_CTRL_RST          (1 << 26)  // Software reset
#define E1000_CTRL_ASDE         (1 << 5)   // Auto-speed detection
#define E1000_CTRL_SLU          (1 << 4)   // Set link up
#define E1000_CTRL_FRCSPD       (1 << 3)   // Force speed
#define E1000_CTRL_FRCDPLX      (1 << 2)   // Force duplex

// Receive control register bits
#define E1000_RCTL_EN           (1 << 1)   // Receive enable
#define E1000_RCTL_SBP          (1 << 2)   // Store bad packets
#define E1000_RCTL_UPE          (1 << 3)   // Unicast promiscuous enable
#define E1000_RCTL_MPE          (1 << 4)   // Multicast promiscuous enable
#define E1000_RCTL_LPE          (1 << 5)   // Long packet enable
#define E1000_RCTL_BAM          (1 << 15)  // Broadcast accept mode
#define E1000_RCTL_SECRC        (1 << 26)  // Strip Ethernet CRC

// Transmit control register bits
#define E1000_TCTL_EN           (1 << 1)   // Transmit enable
#define E1000_TCTL_PSP          (1 << 3)   // Pad short packets
#define E1000_TCTL_CT           (0x10 << 4) // Collision threshold
#define E1000_TCTL_COLD         (0x40 << 12) // Collision distance

// Descriptor flags
#define E1000_TXD_CMD_EOP       (1 << 0)   // End of packet
#define E1000_TXD_CMD_RS        (1 << 3)   // Report status
#define E1000_RXD_STAT_DD       (1 << 0)   // Descriptor done
#define E1000_RXD_STAT_EOP      (1 << 1)   // End of packet

// Driver constants
#define E1000_NUM_RX_DESC      64
#define E1000_NUM_TX_DESC      64
#define E1000_MTU              1500
#define E1000_RX_BUFFER_SIZE   2048
#define E1000_TX_BUFFER_SIZE   2048

// Descriptor structures
typedef struct e1000_rx_desc {
    uint64_t buffer_addr;
    uint16_t length;
    uint16_t checksum;
    uint8_t status;
    uint8_t errors;
    uint16_t special;
} __attribute__((packed)) e1000_rx_desc_t;

typedef struct e1000_tx_desc {
    uint64_t buffer_addr;
    uint16_t length;
    uint8_t cso;
    uint8_t cmd;
    uint8_t status;
    uint8_t css;
    uint16_t special;
} __attribute__((packed)) e1000_tx_desc_t;

// Driver structure
typedef struct e1000_driver {
    uintptr_t io_base;
    uint8_t mac_addr[6];
    e1000_rx_desc_t *rx_descs;
    e1000_tx_desc_t *tx_descs;
    void **rx_buffers;
    void **tx_buffers;
    uint16_t rx_cur;
    uint16_t tx_cur;
    bool initialized;
} e1000_driver_t;

// Function prototypes
bool e1000_init(pci_device_t *dev);
void e1000_send_packet(void *data, size_t len);
bool e1000_receive_packet(void *buffer, size_t *len);
void e1000_handle_interrupt(void);
void e1000_get_mac_address(uint8_t *mac);
bool e1000_is_initialized(void);


void e100_test();
void test_e1000_driver();