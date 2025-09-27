#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>


#include "../ahci/ahci.h"


bool init_nvme();

/*

0x00-0x07	CAP	Controller capabilities.
0x08-0x0B	VS	Version.
0x0C-0x0F	INTMS	Interrupt mask set.
0x10-0x13	INTMC	Interrupt mask clear.
0x14-0x17	CC	Controller configuration.
0x1C-0x1F	CSTS	Controller status.
0x24-0x27	AQA	Admin queue attributes.
0x28-0x2F	ASQ	Admin submission queue.
0x30-0x37	ACQ	Admin completion queue.
0x1000+(2X)*Y	SQxTDBL	Submission queue X tail doorbell.
0x1000+(2X+1)*Y	CQxHDBL	Completion queue X head doorbell.

*/

typedef struct {
    uint64_t cap;    // 0x0
    uint32_t vs;     // 0x8
    uint32_t intms;  // 0xC
    uint32_t intmc;  // 0x10
    uint32_t cc;     // 0x14
    uint32_t reserved1;
    uint32_t csts;   // 0x1C
    // ... many more, but these are enough for init
} nvme_controller_regs_t;


typedef struct {
    uint64_t address; // Physical address of the queue
    uint16_t size;    // Number of entries in the queue
} nvme_queue;

typedef struct {
    uint8_t opcode;
    uint8_t flags;
    uint16_t command_id;
    uint32_t nsid;
    uint64_t reserved1;
    uint64_t mptr;
    uint64_t prp1;
    uint64_t prp2;
    uint32_t command_specific[6];
} nvme_command_entry;

typedef struct {
    uint32_t cdw0;
    uint32_t reserved1;
    uint16_t sq_head;
    uint16_t sq_id;
    uint16_t command_id;
    uint16_t status; // includes phase tag
} nvme_completion;

typedef struct {
    uint16_t vid;            // PCI Vendor ID
    uint16_t ssvid;          // PCI Subsystem Vendor ID
    char serial_number[20];  // Serial Number (20 ASCII characters)
    char model_number[40];   // Model Number (40 ASCII characters)
    char firmware_rev[8];    // Firmware Revision (8 ASCII characters)
    uint8_t rab;             // Recommended Arbitration Burst
    uint8_t ieee[3];         // IEEE OUI Identifier
    uint8_t cmic;           // Controller Multi-Path I/O and Namespace Sharing Capabilities
    uint8_t mdts;           // Maximum Data Transfer Size
    uint16_t cntlid;        // Controller ID
    uint32_t ver;           // Version
    uint32_t rtd3r;         // RTD3 Resume Latency
    uint32_t rtd3e;         // RTD3 Entry Latency
    uint32_t oaes;          // Optional Asynchronous Events Supported
    uint32_t ctratt;        // Controller Attributes
    uint8_t reserved1[156];
    uint16_t oacs;          // Optional Admin Command Support
    uint8_t acl;            // Abort Command Limit
    uint8_t aerl;           // Asynchronous Event Request Limit
    uint8_t frmw;           // Firmware Updates
    uint8_t lpa;            // Log Page Attributes
    uint8_t elpe;           // Error Log Page Entries
    uint8_t npss;           // Number of Power States Support
    uint8_t avscc;          // Admin Vendor Specific Command Configuration
    uint8_t apsta;          // Autonomous Power State Transition Attributes
    uint16_t wctemp;        // Warning Composite Temperature Threshold
    uint16_t cctemp;        // Critical Composite Temperature Threshold
    uint16_t mtfa;          // Maximum Time for Firmware Activation
    uint32_t hmpre;         // Host Memory Buffer Preferred Size
    uint32_t hmmin;         // Host Memory Buffer Minimum Size
    uint64_t tnvmcap[2];    // Total NVM Capacity
    uint64_t unvmcap[2];    // Unallocated NVM Capacity
    uint32_t rpmbs;         // Replay Protected Memory Block Support
    uint16_t edstt;         // Extended Device Self-test Time
    uint8_t dsto;           // Device Self-test Options
    uint8_t fwug;           // Firmware Update Granularity
    uint16_t kas;           // Keep Alive Support
    uint16_t hctma;         // Host Controlled Thermal Management Attributes
    uint16_t mntmt;         // Minimum Thermal Management Temperature
    uint16_t mxntmt;        // Maximum Thermal Management Temperature
    uint8_t reserved2[246];
    uint8_t subnqn[256];    // NVM Subsystem NVMe Qualified Name
    uint8_t reserved3[768];
    uint8_t vendor_specific[1024]; // Vendor Specific
} nvme_identify_controller_t;

