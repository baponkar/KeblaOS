
#include <stdint.h>


struct ahci_controller {
    uint8_t bus;
    uint8_t device;
    uint8_t function;
    uint64_t abar;
    bool initialized;
};
typedef struct ahci_controller ahci_controller_t;


struct hba_port {
	uint32_t clb;    // Command list base (low)
	uint32_t clbu;   // Command list base (high)
	uint32_t fb;     // FIS base (low)
	uint32_t fbu;    // FIS base (high)
	uint32_t is;     // Interrupt status
	uint32_t ie;     // Interrupt enable
	uint32_t cmd;    // Command and status
	uint32_t rsvd1;
	uint32_t tfd;    // Task file data
	uint32_t sig;    // Signature
	uint32_t ssts;   // SATA status (SCR0: SStatus)
	uint32_t sctl;   // SATA control (SCR2: SControl)
	uint32_t serr;   // SATA error (SCR1: SError)
	uint32_t sact;   // SATA active (SCR3: SActive)
	uint32_t ci;     // Command issue
	uint32_t sntf;   // SATA notification (SCR4: SNotification)
	uint32_t fbs;    // FIS-based switching control
	uint32_t rsvd2[11];
	uint32_t vendor[4]; // Vendor specific
};
typedef struct hba_port hba_port_t;

typedef volatile struct hba_mem {
    uint32_t cap;        // Host capabilities
    uint32_t ghc;        // Global host control
    uint32_t is;         // Interrupt status
    uint32_t pi;         // Ports implemented
    uint32_t vsn;        // Version
    uint32_t ccc_ctl;    // Command completion coalescing control
    uint32_t ccc_pts;    // Command completion coalescing ports
    uint32_t em_loc;     // Enclosure management location
    uint32_t em_ctl;     // Enclosure management control
    uint32_t cap2;       // Host capabilities extended
    uint32_t bohc;       // BIOS/OS handoff control
    uint8_t rsvd[0xA0-0x2C]; // Reserved
    uint8_t vendor[0x100-0xA0]; // Vendor specific registers

    hba_port_t ports[32];
} hba_mem;



typedef struct {
    uint8_t fis_type;    // FIS_TYPE_H2D
    uint8_t pm_port;     // Port multiplier
    uint8_t command;
    uint8_t featurel;
    uint8_t lba0;
    uint8_t lba1;
    uint8_t lba2;
    uint8_t device;
    uint8_t lba3;
    uint8_t lba4;
    uint8_t lba5;
    uint8_t featureh;
    uint8_t countl;
    uint8_t counth;
    uint8_t icc;
    uint8_t control;
} h2d_fis;


typedef struct {
    h2d_fis cfis;       // Command FIS
    uint8_t atapi[16];  // ATAPI (unused)
    uint8_t reserved[48];
    uint32_t prdt[4];   // PRD Table (simplified)
} hba_cmd_table;

typedef struct {
    uint16_t cfl:5;   // Command FIS length in DWORDS, typically 5 (20 bytes)
    uint16_t a:1;     // ATAPI
    uint16_t w:1;     // Write (1: write, 0: read)
    uint16_t p:1;     // Prefetchable
    uint16_t r:1;     // Reset
    uint16_t b:1;     // BIST
    uint16_t c:1;     // Clear busy upon R_OK
    uint16_t rsvd0:1; // Reserved
    uint16_t pmp:4;   // Port multiplier port
    uint16_t prdtl;   // Physical region descriptor table length
    uint32_t prdbc;   // Physical region descriptor byte count transferred
    uint32_t ctba;    // Command table descriptor base address (low)
    uint32_t ctbau;   // Command table descriptor base address (high)
    uint32_t rsvd1[4]; // Reserved
} hba_cmd_header;


int ahci_read(uint64_t lba, uint32_t count, void *buffer);
int ahci_write(uint64_t lba, uint32_t count, void *buffer);

void ahci_port_init(struct hba_port *port);

void ahci_init();






