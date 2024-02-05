#ifndef SD_H_
#define SD_H_

#include "hardware.h"
#if HRDW_HAS_SD

//--------------------------------------------------
#include "fatfs.h"
#include "main.h"
#include "settings.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
//--------------------------------------------------
#define SD_CARD_SCAN_INTERVAL 1000
#define SD_CQ_MESSAGE_FILE "!cq_message.wav"
//--------------------------------------------------
/* Card type flags (CardType) */
#define CT_MMC 0x01              /* MMC ver 3 */
#define CT_SD1 0x02              /* SD ver 1 */
#define CT_SD2 0x04              /* SD ver 2 */
#define CT_SDC (CT_SD1 | CT_SD2) /* SD */
#define CT_BLOCK 0x08            /* Block addressing */
// Definitions for MMC/SDC command
#define CMD0 (0x40 + 0)    // 64 GO_IDLE_STATE
#define CMD1 (0x40 + 1)    // 65 SEND_OP_COND (MMC)
#define CMD8 (0x40 + 8)    // 72 SEND_IF_COND
#define CMD9 (0x40 + 9)    // 73 SEND_CSD
#define CMD12 (0x40 + 12)  // 76 STOP_TRANSMISSION
#define CMD16 (0x40 + 16)  // 80 SET_BLOCKLEN
#define CMD17 (0x40 + 17)  // 81 READ_SINGLE_BLOCK
#define CMD18 (0x40 + 18)  // 82 READ_MULTIPLE_BLOCK
#define CMD24 (0x40 + 24)  // 88 WRITE_BLOCK
#define CMD25 (0x40 + 25)  // 89 WRITE_MULTIPLE_BLOCK
#define CMD55 (0x40 + 55)  // 119 APP_CMD
#define CMD58 (0x40 + 58)  // 122 READ_OCR
#define ACMD23 (0xC0 + 23) // 215 SET_WR_BLK_ERASE_COUNT (SDC)
#define ACMD41 (0xC0 + 41) // 233 SEND_OP_COND (SDC)
#define ACMD42 (0xC0 + 42) // 234 SET_CLR_CARD_DETECT
//--------------------------------------------------

#define SD_MAXBLOCK_SIZE 512
#define SD_DEBUG false

typedef struct sd_info {
	uint64_t CAPACITY;
	uint32_t SECTOR_COUNT;
	uint16_t BLOCK_SIZE;
	volatile uint8_t type;
} sd_info_ptr;

typedef enum {
	SDCOMM_IDLE,
	SDCOMM_CHECK_SD,
	SDCOMM_LIST_ROOT,
	SDCOMM_FORMAT,
	SDCOMM_EXPORT_SETTINGS,
	SDCOMM_IMPORT_SETTINGS,
	SDCOMM_START_RECORD,
	SDCOMM_PROCESS_RECORD,
	SDCOMM_CREATE_CQ_MESSAGE_FILE,
	SDCOMM_LIST_DIRECTORY,
	SDCOMM_DELETE_FILE,
	SDCOMM_START_PLAY,
	SDCOMM_PROCESS_PLAY,
	SDCOMM_FLASH_BIN,
	SDCOMM_FLASH_JIC,
	SDCOMM_WRITE_TO_FILE,
	SDCOMM_GET_LINES_COUNT,
	SDCOMM_EXPORT_CALIBRATIONS,
	SDCOMM_IMPORT_TLE_SATNAMES,
	SDCOMM_IMPORT_TLE_INFO,
} SD_COMMAND;

extern FIL File;
extern sd_info_ptr sdinfo;
extern FATFS SDFatFs;
extern bool SD_RecordInProcess;
extern bool SD_RecordingCQmessage;
extern TRX_MODE rec_cqmessage_old_mode;
extern bool SD_PlayInProcess;
extern bool SD_PlayCQMessageInProcess;
extern bool SD_CommandInProcess;
extern bool SD_underrun;
extern bool SD_NeedStopRecord;
extern bool SD_NeedStopPlay;
extern bool SD_Present;
extern bool SD_BusyByUSB;
extern bool SD_USBCardReader;
extern uint32_t SD_Present_tryTime;
extern bool SD_Mounted;
extern uint32_t SD_RecordBufferIndex;
extern BYTE SD_workbuffer_A[FF_MAX_SS];
extern BYTE SD_workbuffer_B[FF_MAX_SS];
extern BYTE SD_workbuffer_current;
extern bool SD_Play_Buffer_Ready;
extern uint32_t SD_Play_Buffer_Size;
extern uint32_t SDCOMM_WRITE_TO_FILE_partsize;
extern void (*SDCOMM_WRITE_TO_FILE_callback)(void);
extern void (*SDCOMM_GET_LINES_COUNT_callback)(uint32_t count);

//--------------------------------------------------
extern void SD_PowerOn(void);
extern uint8_t sd_ini(void);
extern void SPI_Release(void);
extern uint8_t SD_Read_Block(uint8_t *buff, uint32_t btr);
extern uint8_t SD_Write_Block(uint8_t *buff, uint8_t token, bool dma);
extern uint8_t SPI_wait_ready(void);
extern uint8_t SD_cmd(uint8_t cmd, uint32_t arg);
extern void SD_Process(void);
extern bool SD_isIdle(void);
extern bool SD_doCommand(SD_COMMAND command, bool force);
extern void SDCOMM_FLASH_JIC_handler(bool restart);
extern void SDCOMM_FLASH_BIN_handler(void);
//--------------------------------------------------
#endif
#endif /* SD_H_ */
