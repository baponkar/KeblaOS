
/*
Sector 0: Protective MBR
- 446 bytes: Bootstrap code
- 64 bytes: 4 Partition entries (16 bytes each)
- 2 bytes: MBR Signature (0x55AA)
*/

#include "../include/diskio.h"

#include "../../../lib/string.h"
#include "../../../lib/stdlib.h"
#include "../../../lib/stdio.h"
#include "../../../lib/ctype.h"


#include "../include/mbr.h"


// Create Protective MBR
void create_protective_mbr(ProtectiveMBR *mbr, uint64_t total_sectors) {
    memset(mbr, 0, sizeof(ProtectiveMBR));
    
    // Create a protective partition entry
    mbr->partition[0].type = 0xEE;  // Protective MBR type
    mbr->partition[0].lba_start = 1;
    
    // Set partition size (limited to 32-bit in MBR)
    uint32_t mbr_sectors = (total_sectors > 0xFFFFFFFFULL) ? 0xFFFFFFFF : (uint32_t)(total_sectors - 1);
    mbr->partition[0].sector_count = mbr_sectors;   // Fake Partition of whole disk
    
    // MBR signature
    mbr->signature = 0xAA55;
}


