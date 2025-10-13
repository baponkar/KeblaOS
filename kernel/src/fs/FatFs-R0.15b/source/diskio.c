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

#include "../../../kernel/src/driver/disk/disk.h"

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */



// Following externel functions defined in  disk.c
extern bool kebla_disk_init(int disk_no);
extern bool kebla_disk_status(int disk_no);

extern bool kebla_disk_read(int disk_no, uint64_t lba, uint32_t count, void* buf);
extern bool kebla_disk_write(int disk_no, uint64_t lba, uint32_t count, void* buf);


/* Definitions of physical drive number for each drive */
#define DEV_RAM		0	/* Example: Map Ramdisk to physical drive 0 */
#define DEV_MMC		1	/* Example: Map MMC/SD card to physical drive 1 */
#define DEV_USB		2	/* Example: Map USB MSD to physical drive 2 */

#define LBA_OFFSET  2048	/* This offset is safe to use disk without overwritten MBR and GPT*/

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	return kebla_disk_status((int) pdrv) ? RES_OK : RES_ERROR;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS res =  (DSTATUS) kebla_disk_init((int)pdrv) ? RES_OK : RES_ERROR;

	return res;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{

	if(kebla_disk_read((int)pdrv, (uint64_t)(LBA_OFFSET + sector), count, buff)) {
		return RES_OK;
	} 
    return RES_ERROR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	// return RES_PARERR;
	#if FF_FS_READONLY
		return RES_WRPRT;
	#else
		if(kebla_disk_write((int)pdrv, (uint64_t)(LBA_OFFSET + sector), count, buff)) {
			return RES_OK;
		} 
    	return RES_ERROR;
	#endif
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	switch (cmd) {
        case CTRL_SYNC:
            return RES_OK;
        case GET_SECTOR_SIZE:
            *(WORD*)buff = 512;
            return RES_OK;
        case GET_BLOCK_SIZE:
            *(DWORD*)buff = 1;
            return RES_OK;
        case GET_SECTOR_COUNT:
            *(DWORD*)buff = 1024 * 1024; // Just a dummy value, ideally from identify data
            return RES_OK;
        default:
            return RES_PARERR;
    }
}






