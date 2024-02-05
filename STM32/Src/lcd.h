#ifndef LCD_h
#define LCD_h

#include "hardware.h"
#include "lcd_driver.h"
#include "screen_layout.h"
#include "system_menu.h"
#include "touchpad.h"
#include "trx_manager.h"

#define VHF_S_METER_FREQ_START 30000000

typedef struct {
	bool Background;
	bool TopButtons;
	bool TopButtonsRedraw;
	bool BottomButtons;
	bool BottomButtonsRedraw;
	bool FreqInfo;
	bool FreqInfoRedraw;
	bool StatusInfoGUI;
	bool StatusInfoGUIRedraw;
	bool StatusInfoBar;
	bool StatusInfoBarRedraw;
	bool SystemMenu;
	bool SystemMenuRedraw;
	bool SystemMenuCurrent;
	bool SystemMenuInfolines;
	bool TextBar;
	bool Tooltip;
} DEF_LCD_UpdateQuery;

typedef struct {
	uint16_t x1;
	uint16_t y1;
	uint16_t x2;
	uint16_t y2;
	uint32_t parameter;
	void (*clickHandler)(uint32_t parameter);
	void (*holdHandler)(uint32_t parameter);
} TouchpadButton_handler;

typedef struct {
	uint16_t x;
	uint16_t y;
	uint16_t w;
	uint16_t h;
	bool opened;
	TouchpadButton_handler buttons[64];
	uint16_t buttons_count;
} WindowType;

extern void LCD_Init(void);
extern bool LCD_doEvents(void);
extern void LCD_showError(char text[], bool redraw);
extern void LCD_showErrorColored(char text[], bool redraw, uint16_t bg_color);
extern void LCD_showInfo(char text[], bool autohide);
extern void LCD_redraw(bool do_now);
extern void LCD_processTouch(uint16_t x, uint16_t y);
extern void LCD_processHoldTouch(uint16_t x, uint16_t y);
extern bool LCD_processSwipeTouch(uint16_t x, uint16_t y, int16_t dx, int16_t dy);
extern bool LCD_processSwipeTwoFingersTouch(uint16_t x, uint16_t y, int16_t dx, int16_t dy);
extern void LCD_showTooltip(char text[]);
extern void LCD_openWindow(uint16_t w, uint16_t h);
extern void LCD_closeWindow(void);
extern void LCD_showRFPowerWindow(void);
extern void LCD_showManualFreqWindow(uint32_t parameter);
extern void LCD_printKeyboard(void (*keyboardHandler)(char *string, uint32_t max_size, char entered), char *string, uint32_t max_size, bool lowcase);
extern void LCD_hideKeyboard(void);
#if HAS_TOUCHPAD
extern void LCD_cleanTouchpadButtons(void);
extern void LCD_ShowMemoryChannelsButtonHandler(uint32_t parameter);
#endif

#if (defined(LAY_800x480))
void printSystemMenuButton(uint16_t x, uint16_t y, uint16_t width, uint16_t height, char *text, char *value, bool selected, bool active, uint32_t parameter,
                           void (*clickHandler)(uint32_t parameter), void (*holdHandler)(uint32_t parameter), uint16_t active_color, uint16_t inactive_color, uint16_t border_color,
                           uint16_t back_color, SystemMenuType type);
#endif

volatile extern DEF_LCD_UpdateQuery LCD_UpdateQuery;
volatile extern bool LCD_busy;
volatile extern bool LCD_systemMenuOpened;
extern uint16_t LCD_bw_trapez_stripe_pos;
extern WindowType LCD_window;
extern STRUCT_COLOR_THEME *COLOR;
extern STRUCT_LAYOUT_THEME *LAYOUT;
extern bool TRX_X1_VLT_CUR_Mode; // false - VLT, true - CUR

#endif
