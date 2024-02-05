/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    user_diskio.c
 * @brief   This file includes a diskio driver skeleton to be completed by the user.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under Ultimate Liberty license
 * SLA0044, the "License"; You may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 *                             www.st.com/SLA0044
 *
 ******************************************************************************
 */
/* USER CODE END Header */

#ifdef USE_OBSOLETE_USER_CODE_SECTION_0
/*
 * Warning: the user section 0 is no more in use (starting from CubeMx version 4.16.0)
 * To be suppressed in the future.
 * Kept to ensure backward compatibility with previous CubeMx versions when
 * migrating projects.
 * User code previously added there should be copied in the new user sections before
 * the section contents can be deleted.
 */
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */
#endif

/* USER CODE BEGIN DECL */

/* Includes ------------------------------------------------------------------*/
#include "user_diskio.h"
#include "functions.h"
#include "sd.h"
#include <string.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
/* Disk status */
static volatile DSTATUS Stat = STA_NOINIT;

#define SD_RETRY_COUNT 10

/* USER CODE END DECL */

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Initializes a Drive
 * @param  pdrv: Physical drive number (0..)
 * @retval DSTATUS: Operation status
 */
DSTATUS disk_initialize(BYTE pdrv /* Physical drive nmuber to identify the drive */
) {
	/* USER CODE BEGIN INIT */

	if (sd_ini() == 0) {
		Stat &= ~STA_NOINIT;
	} else {
		Stat |= STA_NOINIT;
	}

	return Stat;
	/* USER CODE END INIT */
}

/**
 * @brief  Gets Disk Status
 * @param  pdrv: Physical drive number (0..)
 * @retval DSTATUS: Operation status
 */
DSTATUS disk_status(BYTE pdrv /* Physical drive number to identify the drive */
) {
	/* USER CODE BEGIN STATUS */
	if (pdrv) {
		return STA_NOINIT;
	}
	return Stat;
	/* USER CODE END STATUS */
}

/**
 * @brief  Reads Sector(s)
 * @param  pdrv: Physical drive number (0..)
 * @param  *buff: Data buffer to store read data
 * @param  sector: Sector address (LBA)
 * @param  count: Number of sectors to read (1..128)
 * @retval DRESULT: Operation result
 */
DRESULT disk_read(BYTE pdrv,    /* Physical drive nmuber to identify the drive */
                  BYTE *buff,   /* Data buffer to store read data */
                  LBA_t sector, /* Sector address in LBA */
                  UINT count    /* Number of sectors to read */
) {
	/* USER CODE BEGIN READ */
	if (pdrv || !count) {
		return RES_PARERR;
	}
	if (Stat & STA_NOINIT) {
		return RES_NOTRDY;
	}
	if (!(sdinfo.type & CT_BLOCK)) { /* Convert to byte address if needed */
		sector *= sdinfo.BLOCK_SIZE;
	}

	uint8_t try_n = 0;
	UINT _count = count;
	while (try_n < SD_RETRY_COUNT && _count > 0) {
		try_n++;
		_count = count;
		BYTE *_buff = buff;
		if (_count == 1) /* Single block read */
		{
			if ((SD_cmd(CMD17, sector) == 0) && SD_Read_Block(_buff, sdinfo.BLOCK_SIZE)) /* READ_SINGLE_BLOCK */
			{
				_count = 0;
			}
		} else /* Multiple block read */
		{
			if (SD_cmd(CMD18, sector) == 0) { /* READ_MULTIPLE_BLOCK */
				do {
					if (!SD_Read_Block(_buff, sdinfo.BLOCK_SIZE)) {
						break;
					}
					_buff += sdinfo.BLOCK_SIZE;
				} while (--_count);
				SD_cmd(CMD12, 0); /* STOP_TRANSMISSION */
			}
		}
		SPI_Release();
	}
	return _count ? RES_ERROR : RES_OK;
	/* USER CODE END READ */
}

/**
 * @brief  Writes Sector(s)
 * @param  pdrv: Physical drive number (0..)
 * @param  *buff: Data to be written
 * @param  sector: Sector address (LBA)
 * @param  count: Number of sectors to write (1..128)
 * @retval DRESULT: Operation result
 */
DRESULT disk_write(BYTE pdrv,        /* Physical drive nmuber to identify the drive */
                   const BYTE *buff, /* Data to be written */
                   LBA_t sector,     /* Sector address in LBA */
                   UINT count        /* Number of sectors to write */
) {
	/* USER CODE BEGIN WRITE */
	/* USER CODE HERE */
	if (pdrv || !count) {
		return RES_PARERR;
	}
	if (Stat & STA_NOINIT) {
		return RES_NOTRDY;
	}
	if (Stat & STA_PROTECT) {
		return RES_WRPRT;
	}
	if (!(sdinfo.type & CT_BLOCK)) {
		sector *= sdinfo.BLOCK_SIZE; /* Convert to byte address if needed */
	}

	uint8_t try_n = 0;
	UINT _count = count;
	while (try_n < SD_RETRY_COUNT && _count > 0) {
		try_n++;
		_count = count;
		BYTE *_buff = (BYTE *)buff;

		if (_count == 1) /* Single block write */
		{
			if ((SD_cmd(CMD24, sector) == 0) && SD_Write_Block(_buff, 0xFE, true)) { /* WRITE_BLOCK */
				_count = 0;
			}
		} else /* Multiple block write */
		{
			if (sdinfo.type & CT_SDC) {
				SD_cmd(ACMD23, _count); /* Predefine number of sectors */
			}

			if (SD_cmd(CMD25, sector) == 0) { /* WRITE_MULTIPLE_BLOCK */
				do {
					if (!SD_Write_Block(_buff, 0xFC, true)) {
						break;
					}
					_buff += sdinfo.BLOCK_SIZE;
				} while (--_count);
				if (!SD_Write_Block(0, 0xFD, true)) /* STOP_TRAN token */
				{
					_count = 1;
				}
				SPI_wait_ready();
			}
		}
		SPI_Release();
	}
	return _count ? RES_ERROR : RES_OK;
	/* USER CODE END WRITE */
}

/**
 * @brief  I/O control operation
 * @param  pdrv: Physical drive number (0..)
 * @param  cmd: Control code
 * @param  *buff: Buffer to send/receive control data
 * @retval DRESULT: Operation result
 */
DRESULT disk_ioctl(BYTE pdrv, /* Physical drive nmuber (0..) */
                   BYTE cmd,  /* Control code */
                   void *buff /* Buffer to send/receive control data */
) {
	/* USER CODE BEGIN IOCTL */
	DRESULT res;
	if (pdrv) {
		return RES_PARERR;
	}
	if (Stat & STA_NOINIT) {
		return RES_NOTRDY;
	}
	res = RES_ERROR;

	switch (cmd) {
	case CTRL_SYNC: /* Flush dirty buffer if present */
		HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_RESET);
		if (SPI_wait_ready() == 0xFF) {
			res = RES_OK;
		}
		break;
	case GET_SECTOR_COUNT:
		*(DWORD *)buff = sdinfo.SECTOR_COUNT;
		res = RES_OK;
		break;
	case GET_SECTOR_SIZE: /* Get sectors on the disk (WORD) */
		*(WORD *)buff = sdinfo.BLOCK_SIZE;
		res = RES_OK;
		break;
	case GET_BLOCK_SIZE:
		*(DWORD *)buff = sdinfo.BLOCK_SIZE;
		res = RES_OK;
		break;
	default:
		res = RES_PARERR;
	}

	SPI_Release();
	return res;
	/* USER CODE END IOCTL */
}
