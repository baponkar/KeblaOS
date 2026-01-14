

/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/



#include <stdint.h>
#include <stdbool.h>

#include "../../../driver/disk/disk.h"
#include "../../../lib/stdio.h"

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */



#define LBA_OFFSET 0


DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	if(!kebla_disk_status((int) pdrv)) return STA_NOINIT;
	return 0;
}

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	if(!kebla_disk_init((int)pdrv)) return STA_NOINIT;
	return 0;
}

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{

	if(kebla_disk_read((int)pdrv, (uint64_t)((LBA_t)LBA_OFFSET + sector), (uint64_t)count, buff)) return RES_OK;
 
    return RES_ERROR;
}


#if FF_FS_READONLY == 0
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	// Check if 'sector' looks like a pointer (has high bits set)
    if(sector > 0xFFFF800000000000ULL) {
        printf("[CRITICAL] Sector parameter looks like a pointer: %x\n", sector);
        return RES_PARERR;
    }

	// return RES_PARERR;
	#if FF_FS_READONLY
		return RES_WRPRT;
	#else
		if(kebla_disk_write((int)pdrv, (uint64_t)((LBA_t)LBA_OFFSET + sector), count, buff)){
			return RES_OK;
		} 
    	return RES_ERROR;
	#endif
}
#endif


DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff)
{
    switch (cmd)
    {
        case CTRL_SYNC:
            return RES_OK;
        case GET_SECTOR_SIZE:
            *(WORD*)buff = 512;
            return RES_OK;
        case GET_BLOCK_SIZE:
            *(DWORD*)buff = 1;
            return RES_OK;
        case GET_SECTOR_COUNT:
            *(DWORD*)buff = disks[(int)pdrv].total_sectors;
            return RES_OK;
        default:
            return RES_PARERR;
    }
}






