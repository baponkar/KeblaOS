#include <stdint.h>

typedef enum
{
	FIS_TYPE_REG_H2D	= 0x27,	// Register FIS - host to device
	FIS_TYPE_REG_D2H	= 0x34,	// Register FIS - device to host
	FIS_TYPE_DMA_ACT	= 0x39,	// DMA activate FIS - device to host
	FIS_TYPE_DMA_SETUP	= 0x41,	// DMA setup FIS - bidirectional
	FIS_TYPE_DATA		= 0x46,	// Data FIS - bidirectional
	FIS_TYPE_BIST		= 0x58,	// BIST activate FIS - bidirectional
	FIS_TYPE_PIO_SETUP	= 0x5F,	// PIO setup FIS - device to host
	FIS_TYPE_DEV_BITS	= 0xA1,	// Set device bits FIS - device to host
} FIS_TYPE;

typedef struct tagFIS_REG_H2D
{
	// DWORD 0
	uint8_t  fis_type;	// FIS_TYPE_REG_H2D

	uint8_t  pmport:4;	// Port multiplier
	uint8_t  rsv0:3;		// Reserved
	uint8_t  c:1;		// 1: Command, 0: Control

	uint8_t  command;	// Command register
	uint8_t  featurel;	// Feature register, 7:0
	
	// DWORD 1
	uint8_t  lba0;		// LBA low register, 7:0
	uint8_t  lba1;		// LBA mid register, 15:8
	uint8_t  lba2;		// LBA high register, 23:16
	uint8_t  device;		// Device register

	// DWORD 2
	uint8_t  lba3;		// LBA register, 31:24
	uint8_t  lba4;		// LBA register, 39:32
	uint8_t  lba5;		// LBA register, 47:40
	uint8_t  featureh;	// Feature register, 15:8

	// DWORD 3
	uint8_t  countl;		// Count register, 7:0
	uint8_t  counth;		// Count register, 15:8
	uint8_t  icc;		// Isochronous command completion
	uint8_t  control;	// Control register

	// DWORD 4
	uint8_t  rsv1[4];	// Reserved
} FIS_REG_H2D;

typedef struct {
    uint32_t clb;  // Command List Base Address
    uint32_t clbu; // Command List Base Address Upper 32 Bits
    uint32_t fb;   // FIS Base Address
    uint32_t fbu;  // FIS Base Address Upper 32 Bits
    uint32_t is;   // Interrupt Status
    uint32_t ie;   // Interrupt Enable
    uint32_t cmd;  // Command and Status
    uint32_t rsv0;
    uint32_t tfd;  // Task File Data
    uint32_t sig;  // Signature
    uint32_t ssts; // SATA Status
    uint32_t sctl; // SATA Control
    uint32_t serr; // SATA Error
    uint32_t sact; // SATA Active
    uint32_t ci;   // Command Issue
    uint32_t sntf; // SATA Notification
    uint32_t fbs;  // FIS-Based Switching
} ahci_port_t;


uint32_t pci_read(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
void pci_write(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value);
void detect_ahci();
void init_ahci(uint32_t ahci_base);



