#ifndef FILEMANAGER_h
#define FILEMANAGER_h

#include "hardware.h"
#include "system_menu.h"
#include <stdbool.h>

#define FILEMANAGER_LISTING_MAX_FILES 26
#define FILEMANAGER_LISTING_MAX_FILELEN 40
#define FILEMANAGER_LISTING_ITEMS_ON_PAGE (LAYOUT->SYSMENU_MAX_ITEMS_ON_PAGE - 4)

#define FILEMANAGER_BYTES_PER_PIXEL 2
#define FILEMANAGER_FILE_HEADER_SIZE 14
#define FILEMANAGER_INFO_HEADER_SIZE 40

typedef enum {
	FILMAN_ACT_CANCEL,
	FILMAN_ACT_DELETE,
	FILMAN_ACT_PLAY_WAV,
	FILMAN_ACT_TRANSMIT_WAV,
	FILMAN_ACT_REC_CQ_WAV,
	FILMAN_ACT_FLASHBIN,
	FILMAN_ACT_FLASHJIC,
} FILEMANAGER_ACTION;

extern char FILEMANAGER_CurrentPath[128];
extern char FILEMANAGER_LISTING[FILEMANAGER_LISTING_MAX_FILES][FILEMANAGER_LISTING_MAX_FILELEN + 1];
extern uint16_t FILEMANAGER_files_startindex;
extern uint16_t FILEMANAGER_files_count;

extern void FILEMANAGER_Draw(bool redraw);
extern void FILEMANAGER_EventRotate(int8_t direction);
extern void FILEMANAGER_Closing(void);
extern void FILEMANAGER_EventSecondaryRotate(int8_t direction);
extern void FILEMANAGER_OTAUpdate_handler(void);
extern void FILEMANAGER_OTAUpdate_reset(void);
extern void FILEMANAGER_StartRecCQWav(void);
extern void FILEMANAGER_SCREENSHOT_handler(void);

#endif
