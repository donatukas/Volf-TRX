#ifndef SYSTEM_MENU_H
#define SYSTEM_MENU_H

#include "hardware.h"
#include "lcd_driver.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef LAY_160x128
#define ENUM_MAX_LENGTH 7
#else
#define ENUM_MAX_LENGTH 8
#endif

#define SYSMENU_TOUCHPAD_STYLE (LCD_WIDTH > 700 && HAS_TOUCHPAD)

typedef enum {
	SYSMENU_BOOLEAN,
	SYSMENU_RUN,
	SYSMENU_UINT8,
	SYSMENU_UINT16,
	SYSMENU_UINT32,
	SYSMENU_UINT32R,
	SYSMENU_UINT64,
	SYSMENU_INT8,
	SYSMENU_INT16,
	SYSMENU_INT32,
	SYSMENU_FLOAT32,
	SYSMENU_MENU,
	SYSMENU_INFOLINE,
	SYSMENU_FUNCBUTTON,
	SYSMENU_ENUM,
	SYSMENU_ENUMR,
	SYSMENU_B4,
	SYSMENU_ATU_I,
	SYSMENU_ATU_C,
	SYSMENU_TIMEZONE,
	SYSMENU_NAVBUTTON,
} SystemMenuType;

typedef char enumerate_item[ENUM_MAX_LENGTH];

struct sysmenu_item_handler {
	const char *title;
	const SystemMenuType type;
	bool (*checkVisibleHandler)(void);
	uint32_t *value;
	void (*menuHandler)(int8_t direction);
	const enumerate_item *enumerate;
};

struct sysmenu_menu_wrapper {
	const struct sysmenu_item_handler *menu_handler;
	uint8_t currentIndex;
};

extern bool SYSMENU_FT8_DECODER_opened;
extern bool sysmenu_ota_opened;
extern uint8_t sysmenu_ota_opened_state;
extern bool sysmenu_wifi_selectap1_menu_opened;
extern bool sysmenu_wifi_selectap2_menu_opened;
extern bool sysmenu_wifi_selectap3_menu_opened;
extern bool sysmenu_sat_selectsat_menu_opened;
extern uint8_t sysmenu_sat_selected_index;

extern void SYSMENU_drawSystemMenu(bool draw_background, bool only_infolines);
extern void SYSMENU_redrawCurrentItem(bool redrawAsUnselected);
extern void SYSMENU_eventRotateSystemMenu(int8_t direction);
extern void SYSMENU_eventSecEncoderClickSystemMenu(void);
extern void SYSMENU_eventSecRotateSystemMenu(int8_t direction);
extern void SYSMENU_eventCloseSystemMenu(void);
extern void SYSMENU_eventCloseAllSystemMenu(void);
extern bool SYSMENU_spectrum_opened;
extern bool SYSMENU_hiddenmenu_enabled;

extern void SYSMENU_TRX_STEP_HOTKEY(void);
extern void SYSMENU_CW_WPM_HOTKEY(void);
extern void SYSMENU_CW_KEYER_HOTKEY(void);
extern void SYSMENU_FILTER_BW_SSB_HOTKEY(void);
extern void SYSMENU_FILTER_BW_CW_HOTKEY(void);
extern void SYSMENU_FILTER_BW_AM_HOTKEY(void);
extern void SYSMENU_FILTER_BW_FM_HOTKEY(void);
extern void SYSMENU_FILTER_HPF_SSB_HOTKEY(void);
extern void SYSMENU_RX_IF_HOTKEY(void);
extern void SYSMENU_RX_SQUELCH_HOTKEY(void);
extern void SYSMENU_RX_DNR_HOTKEY(void);
extern void SYSMENU_RX_AGC_HOTKEY(void);
extern void SYSMENU_RX_AGC_MaxGain_HOTKEY(void);
extern void SYSMENU_RX_NB1_HOTKEY(void);
extern void SYSMENU_RX_NB2_HOTKEY(void);
extern void SYSMENU_TX_RFPOWER_HOTKEY(void);
extern void SYSMEUN_CALLSIGN_HOTKEY(void);
extern void SYSMEUN_CALLSIGN_INFO_HOTKEY(void);
extern void SYSMEUN_TIME_HOTKEY(void);
extern void SYSMEUN_WIFI_HOTKEY(void);
extern void SYSMENU_SERVICE_FT8_HOTKEY(void);
extern void SYSMEUN_SD_HOTKEY(void);
extern void SYSMENU_SD_FILEMANAGER_HOTKEY(void);
extern void SYSMENU_TX_CESSB_HOTKEY(void);

extern void SYSMENU_HANDL_Back(int8_t direction);
extern void SYSMENU_HANDL_BackTouch(uint32_t parameter);
extern void SYSMENU_HANDL_PrevPageTouch(uint32_t parameter);
extern void SYSMENU_HANDL_NextPageTouch(uint32_t parameter);
extern void SYSMENU_HANDL_SERVICESMENU(int8_t direction);
extern void SYSMENU_HANDL_FILTER_SSB_HPF_RX_pass(int8_t direction);
extern void SYSMENU_HANDL_FILTER_SSB_HPF_TX_pass(int8_t direction);
extern void SYSMENU_HANDL_FILTER_CW_LPF_pass(int8_t direction);
extern void SYSMENU_HANDL_FILTER_DIGI_LPF_pass(int8_t direction);
extern void SYSMENU_HANDL_FILTER_DIGI_HPF_pass(int8_t direction);
extern void SYSMENU_HANDL_FILTER_SSB_LPF_RX_pass(int8_t direction);
extern void SYSMENU_HANDL_FILTER_SSB_LPF_TX_pass(int8_t direction);
extern void SYSMENU_HANDL_FILTER_AM_LPF_RX_pass(int8_t direction);
extern void SYSMENU_HANDL_FILTER_AM_LPF_TX_pass(int8_t direction);
extern void SYSMENU_HANDL_FILTER_FM_LPF_RX_pass(int8_t direction);
extern void SYSMENU_HANDL_FILTER_FM_LPF_TX_pass(int8_t direction);
extern void SYSMENU_HANDL_FILTER_FM_HPF_RX_pass(int8_t direction);

extern void BUTTONHANDLER_CHOOSE_MENU_ELEMENT(uint32_t parameter);

extern bool SYSMENU_HANDL_CHECK_HAS_ATU(void);

#endif
