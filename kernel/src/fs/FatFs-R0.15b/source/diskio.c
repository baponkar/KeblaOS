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

#include "../../../memory/vmm.h"
#include "../../../driver/disk/disk.h"

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */


extern block_device_t *g_block_devices[MAX_DISKS];
extern int g_block_device_count;

extern uint64_t FAT32_PARTITION_LBA;  // start of partition in sectors

/* Definitions of physical drive number for each drive */
#define DEV_RAM		0	/* Example: Map Ramdisk to physical drive 0 */
#define DEV_MMC		1	/* Example: Map MMC/SD card to physical drive 1 */
#define DEV_USB		2	/* Example: Map USB MSD to physical drive 2 */


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	// DSTATUS stat;
	// int result;

	// switch (pdrv) {
	// case DEV_RAM :
	// 	result = RAM_disk_status();

	// 	// translate the reslut code here

	// 	return stat;

	// case DEV_MMC :
	// 	result = MMC_disk_status();

	// 	// translate the reslut code here

	// 	return stat;

	// case DEV_USB :
	// 	result = USB_disk_status();

	// 	// translate the reslut code here

	// 	return stat;
	// }
	// return STA_NOINIT;

	return disk_status_1((int)pdrv) ? RES_OK : RES_ERROR;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	// DSTATUS stat;
	// int result;

	// switch (pdrv) {
	// case DEV_RAM :
	// 	result = RAM_disk_initialize();

	// 	// translate the reslut code here

	// 	return stat;

	// case DEV_MMC :
	// 	result = MMC_disk_initialize();

	// 	// translate the reslut code here

	// 	return stat;

	// case DEV_USB :
	// 	result = USB_disk_initialize();

	// 	// translate the reslut code here

	// 	return stat;
	// }
	// return STA_NOINIT;

	return (DSTATUS) init_disks((int)pdrv) ? RES_OK : RES_ERROR;

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
	// DRESULT res;
	// int result;

	// switch (pdrv) {
	// case DEV_RAM :
	// 	// translate the arguments here

	// 	result = RAM_disk_read(buff, sector, count);

	// 	// translate the reslut code here

	// 	return res;

	// case DEV_MMC :
	// 	// translate the arguments here

	// 	result = MMC_disk_read(buff, sector, count);

	// 	// translate the reslut code here

	// 	return res;

	// case DEV_USB :
	// 	// translate the arguments here

	// 	result = USB_disk_read(buff, sector, count);

	// 	// translate the reslut code here

	// 	return res;
	// }

	// return RES_PARERR;


	if(disk_read_1(g_block_devices[(int)pdrv], (uint32_t)( FAT32_PARTITION_LBA + sector), count, buff)) {
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
	// DRESULT res;
	// int result;

	// switch (pdrv) {
	// case DEV_RAM :
	// 	// translate the arguments here

	// 	result = RAM_disk_write(buff, sector, count);

	// 	// translate the reslut code here

	// 	return res;

	// case DEV_MMC :
	// 	// translate the arguments here

	// 	result = MMC_disk_write(buff, sector, count);

	// 	// translate the reslut code here

	// 	return res;

	// case DEV_USB :
	// 	// translate the arguments here

	// 	result = USB_disk_write(buff, sector, count);

	// 	// translate the reslut code here

	// 	return res;
	// }

	// return RES_PARERR;
	#if FF_FS_READONLY
		return RES_WRPRT;
	#else
		if(disk_write_1(g_block_devices[(int)pdrv], (uint32_t)( FAT32_PARTITION_LBA + sector), count, buff)) {
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
	// DRESULT res;
	// int result;

	// switch (pdrv) {
	// case DEV_RAM :

	// 	// Process of the command for the RAM drive

	// 	return res;

	// case DEV_MMC :

	// 	// Process of the command for the MMC/SD card

	// 	return res;

	// case DEV_USB :

	// 	// Process of the command the USB drive

	// 	return res;
	// }

	// return RES_PARERR;

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






