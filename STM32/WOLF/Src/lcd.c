#include "lcd.h"
#include "FT8/FT8_GUI.h"
#include "FT8/FT8_main.h"
#include "FT8/decode_ft8.h"
#include "INA226_PWR_monitor.h"
#include "agc.h"
#include "arm_math.h"
#include "audio_filters.h"
#include "codec.h"
#include "cw_decoder.h"
#include "fft.h"
#include "fonts.h"
#include "front_unit.h"
#include "functions.h"
#include "images.h"
#include "main.h"
#include "noise_reduction.h"
#include "rds_decoder.h"
#include "rf_unit.h"
#include "rtty_decoder.h"
#include "screen_layout.h"
#include "sd.h"
#include "settings.h"
#include "system_menu.h"
#include "usbd_ua3reo.h"
#include "vad.h"
#include "wifi.h"

volatile bool LCD_busy = false;
volatile DEF_LCD_UpdateQuery LCD_UpdateQuery = {false};
volatile bool LCD_systemMenuOpened = false;
uint16_t LCD_bw_trapez_stripe_pos = 0;
IRAM2 WindowType LCD_window = {0};
STRUCT_COLOR_THEME *COLOR = &COLOR_THEMES[0];
STRUCT_LAYOUT_THEME *LAYOUT = &LAYOUT_THEMES[0];

static char LCD_freq_string_Hz[6] = {0};
static char LCD_freq_string_kHz[6] = {0};
static char LCD_freq_string_MHz[6] = {0};
static uint64_t LCD_last_showed_freq = 0;
static uint16_t LCD_last_showed_freq_MHz = 9999;
static uint16_t LCD_last_showed_freq_kHz = 9999;
static uint16_t LCD_last_showed_freq_Hz = 9999;
#if (defined(LAY_800x480))
static char LCD_freq_string_Hz_B[6] = {0};
static char LCD_freq_string_kHz_B[6] = {0};
static char LCD_freq_string_MHz_B[6] = {0};
static uint64_t LCD_last_showed_freq_B = 0;
static uint16_t LCD_last_showed_freq_MHz_B = 9999;
static uint16_t LCD_last_showed_freq_kHz_B = 9999;
static uint16_t LCD_last_showed_freq_Hz_B = 9999;
#endif
static char manualFreqEnterString[32] = "";
static bool LCD_screenKeyboardOpened = false;

static bool LCD_inited = false;
static float32_t LCD_last_s_meter = 1.0f;
static float32_t LCD_smeter_peak_x = 0;
static uint32_t Time;
static uint8_t Hours;
static uint8_t Last_showed_Hours = 255;
static uint8_t Minutes;
static uint8_t Last_showed_Minutes = 255;
static uint8_t Seconds;
static uint8_t Last_showed_Seconds = 255;
static uint8_t Day;
static uint8_t Last_showed_Day = 255;
static uint8_t Month;
static uint8_t Last_showed_Month = 255;

static uint32_t Tooltip_DiplayStartTime = 0;
static bool Tooltip_first_draw = true;
static char Tooltip_string[64] = {0};
static bool LCD_showInfo_opened = false;

static TouchpadButton_handler TouchpadButton_handlers[64] = {0};
static uint8_t TouchpadButton_handlers_count = 0;

static void (*LCD_keyboardHandlerCurrent)(char *string, uint32_t max_size, char entered) = NULL;
static bool LCD_keyboards_current_lowcase_status = false;
static char *LCD_keyboards_current_string;
static uint32_t LCD_keyboards_current_max_string_size = 0;

static void printInfoSmall(uint16_t x, uint16_t y, uint16_t width, uint16_t height, char *text, uint16_t back_color, uint16_t text_color, uint16_t in_active_color, bool active);
static void printInfo(uint16_t x, uint16_t y, uint16_t width, uint16_t height, char *text, uint16_t back_color, uint16_t text_color, uint16_t in_active_color, bool active);
static void LCD_displayFreqInfo(bool redraw);
static void LCD_displayTopButtons(bool redraw);
static void LCD_displayStatusInfoBar(bool redraw);
static void LCD_displayStatusInfoGUI(bool redraw);
static void LCD_displayTextBar(void);
static void LCD_printTooltip(void);
static void LCD_showBandWindow(bool secondary_vfo);
static void LCD_showModeWindow(bool secondary_vfo);
static void LCD_showBWWindow(void);
static void LCD_showATTWindow(uint32_t parameter);
static void LCD_ManualFreqButtonHandler(uint32_t parameter);
#if (defined(LAY_800x480))
static void printButton(uint16_t x, uint16_t y, uint16_t width, uint16_t height, char *text, bool active, bool show_lighter, bool in_window, uint32_t parameter,
                        void (*clickHandler)(uint32_t parameter), void (*holdHandler)(uint32_t parameter), uint16_t active_color, uint16_t inactive_color, uint16_t border_color,
                        uint16_t background_color, uint8_t round, const GFXfont *font);
#endif

#define CN_Theme ((TRX.LayoutThemeId == 6) || (TRX.LayoutThemeId == 7))

void LCD_Init(void) {
	COLOR = &COLOR_THEMES[TRX.ColorThemeId];
	LAYOUT = &LAYOUT_THEMES[TRX.LayoutThemeId];

	// DMA2D LCD addr
	WRITE_REG(hdma2d.Instance->OMAR, LCD_FSMC_DATA_ADDR);
	// DMA2D FILL MODE
	MODIFY_REG(hdma2d.Instance->CR, DMA2D_CR_MODE | DMA2D_CR_LOM, DMA2D_R2M | DMA2D_LOM_PIXELS);
	// DMA2D PIXEL FORMAT
	MODIFY_REG(hdma2d.Instance->OPFCCR, DMA2D_OPFCCR_CM | DMA2D_OPFCCR_SB, DMA2D_OUTPUT_ARGB8888 | DMA2D_BYTES_REGULAR);

	LCDDriver_Init();
	if (CALIBRATE.LCD_Rotate) {
		LCDDriver_setRotation(2);
	} else {
		LCDDriver_setRotation(4);
	}

#ifdef HAS_TOUCHPAD
	TOUCHPAD_Init();
#endif
	LCDDriver_Fill(BG_COLOR);

#ifdef HAS_BRIGHTNESS_CONTROL
	LCDDriver_setBrightness(TRX.LCD_Brightness);
#endif

	LCD_inited = true;
}

static void LCD_displayTopButtons(bool redraw) { // display the top buttons
	if (LCD_systemMenuOpened || LCD_window.opened) {
		return;
	}
	if (LCD_busy) {
		LCD_UpdateQuery.TopButtons = true;
		return;
	}
	LCD_busy = true;
	if (redraw) {
		LCDDriver_Fill_RectWH(LAYOUT->TOPBUTTONS_X1, LAYOUT->TOPBUTTONS_Y1, LAYOUT->TOPBUTTONS_X2, LAYOUT->TOPBUTTONS_Y2, BG_COLOR);
	}

	TouchpadButton_handlers_count = 0;

// display information about the operation of the transceiver
#if (defined(LAY_800x480))
	if (CN_Theme) {
		if (TRX.LayoutThemeId == 6) {
			LCDDriver_printImage_RLECompressed(-150, LAYOUT->TOPBUTTONS_Y1 - 3, &GR_LINE, 0, 0);
			LCDDriver_printImage_RLECompressed(100, LAYOUT->TOPBUTTONS_Y1 + 33, &GR_LINE, 0, 0);
			LCDDriver_printImage_RLECompressed(0, LAYOUT->FFT_FFTWTF_BOTTOM + 3, &GR_LINE, 0, 0);
		} else {
			LCDDriver_printImage_RLECompressed(-150, LAYOUT->TOPBUTTONS_Y1 - 3, &GR_LINE, 0, 0);
			LCDDriver_printImage_RLECompressed(100, LAYOUT->TOPBUTTONS_Y1 + 63, &GR_LINE, 0, 0);
			LCDDriver_printImage_RLECompressed(0, LAYOUT->FFT_FFTWTF_BOTTOM, &GR_LINE, 0, 0);
			int8_t curband = getBandFromFreq(TRX.VFO_A.Freq, true);
			if (TRX.selected_vfo) {
				curband = getBandFromFreq(TRX.VFO_B.Freq, true);
			}

			// selectable bands first
			uint8_t xi = 0;
			for (uint8_t bindx = 0; bindx < BANDS_COUNT; bindx++) {
				if (!BAND_SELECTABLE[bindx] || BANDS[bindx].broadcast || BANDS[bindx].name == (char *)BANDS[BANDID_CB].name) {
					continue;
				}

				if (!TRX.selected_vfo) {
					printButton(xi * LAYOUT->TOPBUTTONS_WIDTH, 129, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, (char *)BANDS[bindx].name, (curband == bindx), true, 0, bindx,
					            BUTTONHANDLER_SET_VFOA_BAND, LCD_showManualFreqWindow, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT, COLOR->BUTTON_BORDER, COLOR->BUTTON_SWITCH_BACKGROUND,
					            LAYOUT->TOPBUTTONS_ROUND, LAYOUT->TOPBUTTONS_FONT);
				} else {
					printButton(xi * LAYOUT->TOPBUTTONS_WIDTH, 129, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, (char *)BANDS[bindx].name, (curband == bindx), true, 0, bindx,
					            BUTTONHANDLER_SET_VFOB_BAND, LCD_showManualFreqWindow, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT, COLOR->BUTTON_BORDER, COLOR->BUTTON_SWITCH_BACKGROUND,
					            LAYOUT->TOPBUTTONS_ROUND, LAYOUT->TOPBUTTONS_FONT);
				}

				xi++;
				if (xi >= 10) {
					break;
				}
			}
			// memory bank
			printButton(LCD_WIDTH - LAYOUT->TOPBUTTONS_WIDTH, 129, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "MEM", false, 0, false, 0, LCD_ShowMemoryChannelsButtonHandler,
			            LCD_ShowMemoryChannelsButtonHandler, COLOR->BUTTON_TEXT, COLOR_YELLOW, COLOR->BUTTON_BORDER, COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND,
			            LAYOUT->TOPBUTTONS_FONT);
		}
	}
	printButton(LAYOUT->TOPBUTTONS_PRE_X, LAYOUT->TOPBUTTONS_PRE_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "PRE", TRX.LNA, true, false, 0, BUTTONHANDLER_PRE, NULL,
	            COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT, COLOR->BUTTON_BORDER, COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND, LAYOUT->TOPBUTTONS_FONT);
	char buff[64] = {0};
	if (TRX.ATT_DB >= 1.0f) {
		sprintf(buff, "ATT%d", (uint8_t)TRX.ATT_DB);
	} else {
		sprintf(buff, "ATT");
	}
	printButton(LAYOUT->TOPBUTTONS_ATT_X, LAYOUT->TOPBUTTONS_ATT_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, buff, (TRX.ATT && TRX.ATT_DB >= 1.0f), true, false, 0,
	            BUTTONHANDLER_ATT, LCD_showATTWindow, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT, COLOR->BUTTON_BORDER, COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND,
	            LAYOUT->TOPBUTTONS_FONT);
	printButton(LAYOUT->TOPBUTTONS_PGA_X, LAYOUT->TOPBUTTONS_PGA_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "PGA", TRX.ADC_PGA, true, false, 0, BUTTONHANDLER_PGA_ONLY, NULL,
	            COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT, COLOR->BUTTON_BORDER, COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND, LAYOUT->TOPBUTTONS_FONT);
	printButton(LAYOUT->TOPBUTTONS_DRV_X, LAYOUT->TOPBUTTONS_DRV_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "DRV", TRX.ADC_Driver, true, false, 0, BUTTONHANDLER_DRV_ONLY, NULL,
	            COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT, COLOR->BUTTON_BORDER, COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND, LAYOUT->TOPBUTTONS_FONT);
	printButton(LAYOUT->TOPBUTTONS_FAST_X, LAYOUT->TOPBUTTONS_FAST_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "FAST", TRX.Fast, true, false, 0, BUTTONHANDLER_FAST, NULL,
	            COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT, COLOR->BUTTON_BORDER, COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND, LAYOUT->TOPBUTTONS_FONT);
	printButton(LAYOUT->TOPBUTTONS_AGC_X, LAYOUT->TOPBUTTONS_AGC_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "AGC", CurrentVFO->AGC, true, false, 0, BUTTONHANDLER_AGC,
	            BUTTONHANDLER_AGC_SPEED, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT, COLOR->BUTTON_BORDER, COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND,
	            LAYOUT->TOPBUTTONS_FONT);
	if (CurrentVFO->DNR_Type == 1) {
		printButton(LAYOUT->TOPBUTTONS_DNR_X, LAYOUT->TOPBUTTONS_DNR_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "DNR1", true, true, false, 0, BUTTONHANDLER_DNR,
		            BUTTONHANDLER_DNR_HOLD, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT, COLOR->BUTTON_BORDER, COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND,
		            LAYOUT->TOPBUTTONS_FONT);
	} else if (CurrentVFO->DNR_Type == 2) {
		printButton(LAYOUT->TOPBUTTONS_DNR_X, LAYOUT->TOPBUTTONS_DNR_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "DNR2", true, true, false, 0, BUTTONHANDLER_DNR,
		            BUTTONHANDLER_DNR_HOLD, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT, COLOR->BUTTON_BORDER, COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND,
		            LAYOUT->TOPBUTTONS_FONT);
	} else {
		printButton(LAYOUT->TOPBUTTONS_DNR_X, LAYOUT->TOPBUTTONS_DNR_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "DNR", false, true, false, 0, BUTTONHANDLER_DNR,
		            BUTTONHANDLER_DNR_HOLD, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT, COLOR->BUTTON_BORDER, COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND,
		            LAYOUT->TOPBUTTONS_FONT);
	}
	if (TRX.NOISE_BLANKER1 == false && TRX.NOISE_BLANKER2 == false) {
		printButton(LAYOUT->TOPBUTTONS_NB_X, LAYOUT->TOPBUTTONS_NB_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "NB", false, true, false, 0, BUTTONHANDLER_NB, BUTTONHANDLER_NB_HOLD,
		            COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT, COLOR->BUTTON_BORDER, COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND, LAYOUT->TOPBUTTONS_FONT);
	}
	if (TRX.NOISE_BLANKER1 == true && TRX.NOISE_BLANKER2 == false) {
		printButton(LAYOUT->TOPBUTTONS_NB_X, LAYOUT->TOPBUTTONS_NB_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "NB1", true, true, false, 0, BUTTONHANDLER_NB, BUTTONHANDLER_NB_HOLD,
		            COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT, COLOR->BUTTON_BORDER, COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND, LAYOUT->TOPBUTTONS_FONT);
	}
	if (TRX.NOISE_BLANKER1 == false && TRX.NOISE_BLANKER2 == true) {
		printButton(LAYOUT->TOPBUTTONS_NB_X, LAYOUT->TOPBUTTONS_NB_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "NB2", true, true, false, 0, BUTTONHANDLER_NB, BUTTONHANDLER_NB_HOLD,
		            COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT, COLOR->BUTTON_BORDER, COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND, LAYOUT->TOPBUTTONS_FONT);
	}
	if (TRX.NOISE_BLANKER1 == true && TRX.NOISE_BLANKER2 == true) {
		printButton(LAYOUT->TOPBUTTONS_NB_X, LAYOUT->TOPBUTTONS_NB_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "NB1+2", true, true, false, 0, BUTTONHANDLER_NB, BUTTONHANDLER_NB_HOLD,
		            COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT, COLOR->BUTTON_BORDER, COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND, LAYOUT->TOPBUTTONS_FONT);
	}
	printButton(LAYOUT->TOPBUTTONS_NOTCH_X, LAYOUT->TOPBUTTONS_NOTCH_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "NOTCH",
	            (CurrentVFO->AutoNotchFilter || CurrentVFO->ManualNotchFilter), true, false, 0, BUTTONHANDLER_NOTCH, BUTTONHANDLER_NOTCH_MANUAL, COLOR->BUTTON_TEXT,
	            COLOR->BUTTON_INACTIVE_TEXT, COLOR->BUTTON_BORDER, COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND, LAYOUT->TOPBUTTONS_FONT);
	printButton(LAYOUT->TOPBUTTONS_SQL_X, LAYOUT->TOPBUTTONS_SQL_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "SQL", CurrentVFO->SQL, true, false, 0, BUTTONHANDLER_SQL,
	            BUTTONHANDLER_SQUELCH, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT, COLOR->BUTTON_BORDER, COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND,
	            LAYOUT->TOPBUTTONS_FONT);
	if (TRX.AFAmp_Mute) {
#if defined(FRONTPANEL_WOLF_2)
		printButton(LAYOUT->TOPBUTTONS_MUTE_X, LAYOUT->TOPBUTTONS_MUTE_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "BT", true, true, false, 0, BUTTONHANDLER_MUTE,
		            BUTTONHANDLER_MUTE_AFAMP, COLOR_RED, COLOR->BUTTON_INACTIVE_TEXT, COLOR->BUTTON_BORDER, COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND, LAYOUT->TOPBUTTONS_FONT);
#else
		printButton(LAYOUT->TOPBUTTONS_MUTE_X, LAYOUT->TOPBUTTONS_MUTE_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "MAMP", true, true, false, 0, BUTTONHANDLER_MUTE,
		            BUTTONHANDLER_MUTE_AFAMP, COLOR_RED, COLOR->BUTTON_INACTIVE_TEXT, COLOR->BUTTON_BORDER, COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND, LAYOUT->TOPBUTTONS_FONT);
#endif
	} else {
		printButton(LAYOUT->TOPBUTTONS_MUTE_X, LAYOUT->TOPBUTTONS_MUTE_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "MUTE", TRX.Mute, true, false, 0, BUTTONHANDLER_MUTE,
		            BUTTONHANDLER_MUTE_AFAMP, COLOR_RED, COLOR->BUTTON_INACTIVE_TEXT, COLOR->BUTTON_BORDER, COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND, LAYOUT->TOPBUTTONS_FONT);
	}
#else
	printInfo(LAYOUT->TOPBUTTONS_PRE_X, LAYOUT->TOPBUTTONS_PRE_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "PRE", COLOR->BUTTON_STATUS_BACKGROUND, COLOR->BUTTON_TEXT,
	          COLOR->BUTTON_INACTIVE_TEXT, TRX.LNA);
	char buff[64] = {0};
	if (TRX.ATT_DB >= 1.0f) {
		sprintf(buff, "ATT%d", (uint8_t)TRX.ATT_DB);
	} else {
		sprintf(buff, "ATT");
	}
	printInfo(LAYOUT->TOPBUTTONS_ATT_X, LAYOUT->TOPBUTTONS_ATT_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, buff, COLOR->BUTTON_STATUS_BACKGROUND, COLOR->BUTTON_TEXT,
	          COLOR->BUTTON_INACTIVE_TEXT, (TRX.ATT && TRX.ATT_DB >= 1.0f));
	printInfo(LAYOUT->TOPBUTTONS_PGA_X, LAYOUT->TOPBUTTONS_PGA_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "PGA", COLOR->BUTTON_STATUS_BACKGROUND, COLOR->BUTTON_TEXT,
	          COLOR->BUTTON_INACTIVE_TEXT, TRX.ADC_PGA);
	printInfo(LAYOUT->TOPBUTTONS_DRV_X, LAYOUT->TOPBUTTONS_DRV_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "DRV", COLOR->BUTTON_STATUS_BACKGROUND, COLOR->BUTTON_TEXT,
	          COLOR->BUTTON_INACTIVE_TEXT, TRX.ADC_Driver);
	printInfo(LAYOUT->TOPBUTTONS_FAST_X, LAYOUT->TOPBUTTONS_FAST_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "FAST", COLOR->BUTTON_STATUS_BACKGROUND, COLOR->BUTTON_TEXT,
	          COLOR->BUTTON_INACTIVE_TEXT, TRX.Fast);
	printInfo(LAYOUT->TOPBUTTONS_AGC_X, LAYOUT->TOPBUTTONS_AGC_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "AGC", COLOR->BUTTON_STATUS_BACKGROUND, COLOR->BUTTON_TEXT,
	          COLOR->BUTTON_INACTIVE_TEXT, CurrentVFO->AGC);
	if (CurrentVFO->DNR_Type == 1) {
		printInfo(LAYOUT->TOPBUTTONS_DNR_X, LAYOUT->TOPBUTTONS_DNR_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "DNR1", COLOR->BUTTON_STATUS_BACKGROUND, COLOR->BUTTON_TEXT,
		          COLOR->BUTTON_INACTIVE_TEXT, true);
	} else if (CurrentVFO->DNR_Type == 2) {
		printInfo(LAYOUT->TOPBUTTONS_DNR_X, LAYOUT->TOPBUTTONS_DNR_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "DNR2", COLOR->BUTTON_STATUS_BACKGROUND, COLOR->BUTTON_TEXT,
		          COLOR->BUTTON_INACTIVE_TEXT, true);
	} else {
		printInfo(LAYOUT->TOPBUTTONS_DNR_X, LAYOUT->TOPBUTTONS_DNR_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "DNR", COLOR->BUTTON_STATUS_BACKGROUND, COLOR->BUTTON_TEXT,
		          COLOR->BUTTON_INACTIVE_TEXT, false);
	}
	if (TRX.NOISE_BLANKER1 == false && TRX.NOISE_BLANKER2 == false) {
		printInfo(LAYOUT->TOPBUTTONS_NB_X, LAYOUT->TOPBUTTONS_NB_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "NB", COLOR->BUTTON_STATUS_BACKGROUND, COLOR->BUTTON_TEXT,
		          COLOR->BUTTON_INACTIVE_TEXT, false);
	}
	if (TRX.NOISE_BLANKER1 == true && TRX.NOISE_BLANKER2 == false) {
		printInfo(LAYOUT->TOPBUTTONS_NB_X, LAYOUT->TOPBUTTONS_NB_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "NB1", COLOR->BUTTON_STATUS_BACKGROUND, COLOR->BUTTON_TEXT,
		          COLOR->BUTTON_INACTIVE_TEXT, true);
	}
	if (TRX.NOISE_BLANKER1 == false && TRX.NOISE_BLANKER2 == true) {
		printInfo(LAYOUT->TOPBUTTONS_NB_X, LAYOUT->TOPBUTTONS_NB_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "NB2", COLOR->BUTTON_STATUS_BACKGROUND, COLOR->BUTTON_TEXT,
		          COLOR->BUTTON_INACTIVE_TEXT, true);
	}
	if (TRX.NOISE_BLANKER1 == true && TRX.NOISE_BLANKER2 == true) {
		printInfo(LAYOUT->TOPBUTTONS_NB_X, LAYOUT->TOPBUTTONS_NB_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "NB12", COLOR->BUTTON_STATUS_BACKGROUND, COLOR->BUTTON_TEXT,
		          COLOR->BUTTON_INACTIVE_TEXT, true);
	}
	printInfo(LAYOUT->TOPBUTTONS_MUTE_X, LAYOUT->TOPBUTTONS_MUTE_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "MUTE", COLOR->BUTTON_STATUS_BACKGROUND, COLOR_RED,
	          COLOR->BUTTON_INACTIVE_TEXT, TRX.Mute);
	printInfo(LAYOUT->TOPBUTTONS_LOCK_X, LAYOUT->TOPBUTTONS_LOCK_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "LOCK", COLOR->BUTTON_STATUS_BACKGROUND, COLOR_RED,
	          COLOR->BUTTON_INACTIVE_TEXT, TRX.Locked);
#endif

	LCD_UpdateQuery.TopButtons = false;
	if (redraw) {
		LCD_UpdateQuery.TopButtonsRedraw = false;
	}

	// redraw bottom after top
	LCD_UpdateQuery.BottomButtons = true;

	LCD_busy = false;
}

static void LCD_displayBottomButtons(bool redraw) {
	// display the bottom buttons
	if (LCD_systemMenuOpened || LCD_window.opened) {
		return;
	}
	if (LCD_busy) {
		LCD_UpdateQuery.BottomButtons = true;
		return;
	}
	LCD_busy = true;

#if (defined(LAY_800x480))
	if (redraw) {
		LCDDriver_Fill_RectWH(0, LAYOUT->BOTTOM_BUTTONS_BLOCK_TOP, LCD_WIDTH, LAYOUT->BOTTOM_BUTTONS_BLOCK_HEIGHT, BG_COLOR);
	}

	const uint16_t bottomNavigationButtonsWidth = 48;
	uint16_t curr_x = 0;

	for (uint8_t i = 0; i < FUNCBUTTONS_ON_PAGE; i++) {
		uint16_t menuPosition = TRX.FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + i;

		bool enabled = false;
		if (PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[menuPosition]].checkBool != NULL) {
			if ((uint8_t)*PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[menuPosition]].checkBool == 1) {
				enabled = true;
			}
		}

		bool dummyButton = false;
		if (PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[menuPosition]].clickHandler == NULL && PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[menuPosition]].holdHandler == NULL) {
			dummyButton = true;
		}

		uint16_t width = LAYOUT->BOTTOM_BUTTONS_ONE_WIDTH;

		if (TRX.EnableBottomNavigationButtons) {
			width = floorf((float32_t)(LCD_WIDTH - bottomNavigationButtonsWidth * 2) / FUNCBUTTONS_ON_PAGE);
		} else if (FUNCBUTTONS_ON_PAGE == 9 && i < 8) {
			width += 1;
		}

		bool printLeftPutton = TRX.EnableBottomNavigationButtons && ((FUNCBUTTONS_ON_PAGE == 9 && i == 4) || (FUNCBUTTONS_ON_PAGE == 8 && i == 3));
		if (printLeftPutton) {
			printButton(curr_x, LAYOUT->BOTTOM_BUTTONS_BLOCK_TOP, bottomNavigationButtonsWidth, LAYOUT->BOTTOM_BUTTONS_BLOCK_HEIGHT, "<", false, false, false, 0, BUTTONHANDLER_LEFT_ARR,
			            BUTTONHANDLER_LEFT_ARR, COLOR->BUTTON_PAGER_TEXT, COLOR->BUTTON_PAGER_TEXT, COLOR->BUTTON_BORDER, COLOR->BUTTON_PAGER_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND,
			            LAYOUT->TOPBUTTONS_FONT);
			curr_x += bottomNavigationButtonsWidth;
		}

		if (!dummyButton) {
			printButton(curr_x, LAYOUT->BOTTOM_BUTTONS_BLOCK_TOP, width, LAYOUT->BOTTOM_BUTTONS_BLOCK_HEIGHT, (char *)PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[menuPosition]].name,
			            enabled, false, false, PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[menuPosition]].parameter,
			            PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[menuPosition]].clickHandler, PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[menuPosition]].holdHandler,
			            COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT, COLOR->BUTTON_BORDER, COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND, LAYOUT->TOPBUTTONS_FONT);
		} else {
			printButton(curr_x, LAYOUT->BOTTOM_BUTTONS_BLOCK_TOP, width, LAYOUT->BOTTOM_BUTTONS_BLOCK_HEIGHT, " ", false, false, false, 0, NULL, NULL, COLOR->BUTTON_TEXT,
			            COLOR->BUTTON_INACTIVE_TEXT, COLOR->BUTTON_BORDER, COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND, LAYOUT->TOPBUTTONS_FONT);
		}
		curr_x += width;

		bool printRightPutton = TRX.EnableBottomNavigationButtons && ((FUNCBUTTONS_ON_PAGE == 9 && i == 4) || (FUNCBUTTONS_ON_PAGE == 8 && i == 4));
		if (printRightPutton) {
			printButton(curr_x, LAYOUT->BOTTOM_BUTTONS_BLOCK_TOP, bottomNavigationButtonsWidth, LAYOUT->BOTTOM_BUTTONS_BLOCK_HEIGHT, ">", false, false, false, 0, BUTTONHANDLER_RIGHT_ARR,
			            BUTTONHANDLER_RIGHT_ARR, COLOR->BUTTON_PAGER_TEXT, COLOR->BUTTON_PAGER_TEXT, COLOR->BUTTON_BORDER, COLOR->BUTTON_PAGER_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND,
			            LAYOUT->TOPBUTTONS_FONT);
			curr_x += bottomNavigationButtonsWidth;
		}
	}
#endif

	LCD_UpdateQuery.BottomButtons = false;
	if (redraw) {
		LCD_UpdateQuery.BottomButtonsRedraw = false;
	}
	LCD_busy = false;
}

static void LCD_displayFreqInfo(bool redraw) { // display the frequency on the screen
	if (LCD_systemMenuOpened || LCD_window.opened) {
		return;
	}

	uint64_t display_freq_a = CurrentVFO->Freq;
#if (defined(LAY_800x480))
	display_freq_a = TRX.VFO_A.Freq;
#endif
	if (TRX.Custom_Transverter_Enabled) {
		display_freq_a += (uint64_t)CALIBRATE.Transverter_Custom_Offset_MHz * 1000 * 1000;
	}
	if (TRX_on_TX && !TRX.selected_vfo) {
		display_freq_a += TRX_XIT;
	}

#if (defined(LAY_800x480))
	uint64_t display_freq_b = TRX.VFO_B.Freq;
	if (TRX.Custom_Transverter_Enabled) {
		display_freq_b += (uint64_t)CALIBRATE.Transverter_Custom_Offset_MHz * 1000 * 1000;
	}
	if (TRX_on_TX && TRX.selected_vfo) {
		display_freq_b += TRX_XIT;
	}
#endif

	if (!redraw && (LCD_last_showed_freq == display_freq_a)
#if (defined(LAY_800x480))
	    && (LCD_last_showed_freq_B == display_freq_b)
#endif
	) {
		return;
	}

	if (LCD_busy) {
		LCD_UpdateQuery.FreqInfo = true;
		if (redraw) {
			LCD_UpdateQuery.FreqInfoRedraw = true;
		}
		return;
	}
	LCD_busy = true;

	LCD_last_showed_freq = display_freq_a;
	uint16_t Hz = (display_freq_a % 1000);
	uint16_t kHz = ((display_freq_a / 1000) % 1000);
	uint16_t MHz = ((display_freq_a / 1000000) % 1000);
	uint16_t GHz = ((display_freq_a / 1000000000) % 1000);
	if (display_freq_a >= 1000000000) //>= 1GHz
	{
		Hz = kHz;
		kHz = MHz;
		MHz = GHz;
	}

	uint16_t MHz_x_offset = 0;
	if (MHz >= 100) {
		MHz_x_offset = LAYOUT->FREQ_X_OFFSET_100;
	} else if (MHz >= 10) {
		MHz_x_offset = LAYOUT->FREQ_X_OFFSET_10;
	} else {
		MHz_x_offset = LAYOUT->FREQ_X_OFFSET_1;
	}

	if (redraw) {
		LCDDriver_Fill_RectWH(LAYOUT->FREQ_LEFT_MARGIN, LAYOUT->FREQ_Y_TOP, LCD_WIDTH - LAYOUT->FREQ_LEFT_MARGIN - LAYOUT->FREQ_RIGHT_MARGIN, LAYOUT->FREQ_BLOCK_HEIGHT, BG_COLOR);
	}

	if ((MHz_x_offset - LAYOUT->FREQ_LEFT_MARGIN) > 0) {
		LCDDriver_Fill_RectWH(LAYOUT->FREQ_LEFT_MARGIN, LAYOUT->FREQ_Y_TOP, MHz_x_offset - LAYOUT->FREQ_LEFT_MARGIN, LAYOUT->FREQ_BLOCK_HEIGHT, BG_COLOR);
	}

	// add spaces to output the frequency
	sprintf(LCD_freq_string_Hz, "%d", Hz);
	sprintf(LCD_freq_string_kHz, "%d", kHz);
	sprintf(LCD_freq_string_MHz, "%d", MHz);

	if (redraw || (LCD_last_showed_freq_MHz != MHz)) {
		LCDDriver_printTextFont(LCD_freq_string_MHz, MHz_x_offset, LAYOUT->FREQ_Y_BASELINE, !TRX.selected_vfo ? COLOR->FREQ_MHZ : COLOR->FREQ_A_INACTIVE, BG_COLOR, LAYOUT->FREQ_FONT);
		LCD_last_showed_freq_MHz = MHz;
	}

	char buff[50] = "";
	if (redraw || (LCD_last_showed_freq_kHz != kHz)) {
		addSymbols(buff, LCD_freq_string_kHz, 3, "0", false);
		LCDDriver_printTextFont(buff, LAYOUT->FREQ_X_OFFSET_KHZ, LAYOUT->FREQ_Y_BASELINE, !TRX.selected_vfo ? COLOR->FREQ_KHZ : COLOR->FREQ_A_INACTIVE, BG_COLOR, LAYOUT->FREQ_FONT);
		LCD_last_showed_freq_kHz = kHz;
	}
	if (redraw || (LCD_last_showed_freq_Hz != Hz) || TRX.ChannelMode) {
		addSymbols(buff, LCD_freq_string_Hz, 3, "0", false);
		int_fast8_t band = -1;
		int_fast16_t channel = -1;
		if (TRX.ChannelMode) {
			band = getBandFromFreq(display_freq_a, false);
			if (!TRX.selected_vfo) {
				channel = getChannelbyFreq(display_freq_a, false);
			} else {
				channel = getChannelbyFreq(display_freq_a, true);
			}
		}
		if (TRX.ChannelMode && band >= 0 && BANDS[band].channelsCount > 0) {
			if (channel != -1 && strlen((char *)BANDS[band].channels[channel].subname) > 0) {
				sprintf(buff, "%s", (char *)BANDS[band].channels[channel].subname);
			} else {
				sprintf(buff, "CH");
			}
			addSymbols(buff, buff, 2, " ", false);
			LCDDriver_printText(buff, LAYOUT->FREQ_X_OFFSET_HZ + 2, LAYOUT->FREQ_Y_BASELINE_SMALL - RASTR_FONT_H * 2, !TRX.selected_vfo ? COLOR->FREQ_HZ : COLOR->FREQ_A_INACTIVE, BG_COLOR, 2);

			if (channel != -1) {
				sprintf(buff, "%d", BANDS[band].channels[channel].number);
			} else {
				sprintf(buff, "-");
			}
			addSymbols(buff, buff, 2, " ", true);
			LCDDriver_printTextFont(buff, LAYOUT->FREQ_X_OFFSET_HZ + 2 + RASTR_FONT_W * 2 * 2, LAYOUT->FREQ_Y_BASELINE_SMALL, !TRX.selected_vfo ? COLOR->STATUS_MODE : COLOR->FREQ_A_INACTIVE,
			                        BG_COLOR, LAYOUT->FREQ_CH_FONT);
		} else {
			LCDDriver_printTextFont(buff, LAYOUT->FREQ_X_OFFSET_HZ, LAYOUT->FREQ_Y_BASELINE_SMALL, !TRX.selected_vfo ? COLOR->FREQ_HZ : COLOR->FREQ_A_INACTIVE, BG_COLOR, LAYOUT->FREQ_SMALL_FONT);
		}
		LCD_last_showed_freq_Hz = Hz;
	}

#if (defined(LAY_800x480))
	LCD_last_showed_freq_B = display_freq_b;
	uint16_t Hz_B = (LCD_last_showed_freq_B % 1000);
	uint16_t kHz_B = ((LCD_last_showed_freq_B / 1000) % 1000);
	uint16_t MHz_B = ((LCD_last_showed_freq_B / 1000000) % 1000);
	uint16_t GHz_B = ((LCD_last_showed_freq_B / 1000000000) % 1000);
	if (display_freq_b >= 1000000000) //>= 1GHz
	{
		Hz_B = kHz_B;
		kHz_B = MHz_B;
		MHz_B = GHz_B;
	}

	uint16_t MHz_x_offset_B = 0;
	if (MHz_B >= 100) {
		MHz_x_offset_B = LAYOUT->FREQ_B_X_OFFSET_100;
	} else if (MHz_B >= 10) {
		MHz_x_offset_B = LAYOUT->FREQ_B_X_OFFSET_10;
	} else {
		MHz_x_offset_B = LAYOUT->FREQ_B_X_OFFSET_1;
	}

	if (redraw) {
		LCDDriver_Fill_RectWH(LAYOUT->FREQ_B_LEFT_MARGIN, LAYOUT->FREQ_B_Y_TOP, LCD_WIDTH - LAYOUT->FREQ_B_LEFT_MARGIN - LAYOUT->FREQ_B_RIGHT_MARGIN, LAYOUT->FREQ_B_BLOCK_HEIGHT, BG_COLOR);
	}

	if ((MHz_x_offset_B - LAYOUT->FREQ_B_LEFT_MARGIN) > 0) {
		LCDDriver_Fill_RectWH(LAYOUT->FREQ_B_LEFT_MARGIN, LAYOUT->FREQ_B_Y_TOP, MHz_x_offset_B - LAYOUT->FREQ_B_LEFT_MARGIN, LAYOUT->FREQ_B_BLOCK_HEIGHT, BG_COLOR);
	}

	// add spaces to output the frequency
	sprintf(LCD_freq_string_Hz_B, "%d", Hz_B);
	sprintf(LCD_freq_string_kHz_B, "%d", kHz_B);
	sprintf(LCD_freq_string_MHz_B, "%d", MHz_B);

	if (redraw || (LCD_last_showed_freq_MHz_B != MHz_B)) {
		LCDDriver_printTextFont(LCD_freq_string_MHz_B, MHz_x_offset_B, LAYOUT->FREQ_B_Y_BASELINE, TRX.selected_vfo ? COLOR->FREQ_B_MHZ : COLOR->FREQ_B_INACTIVE, BG_COLOR, LAYOUT->FREQ_B_FONT);
		LCD_last_showed_freq_MHz_B = MHz_B;
	}

	if (redraw || (LCD_last_showed_freq_kHz_B != kHz_B)) {
		addSymbols(buff, LCD_freq_string_kHz_B, 3, "0", false);
		LCDDriver_printTextFont(buff, LAYOUT->FREQ_B_X_OFFSET_KHZ, LAYOUT->FREQ_B_Y_BASELINE, TRX.selected_vfo ? COLOR->FREQ_B_KHZ : COLOR->FREQ_B_INACTIVE, BG_COLOR, LAYOUT->FREQ_B_FONT);
		LCD_last_showed_freq_kHz_B = kHz_B;
	}
	if (redraw || (LCD_last_showed_freq_Hz_B != Hz_B) || TRX.ChannelMode) {
		addSymbols(buff, LCD_freq_string_Hz_B, 3, "0", false);
		int_fast8_t band = -1;
		int_fast8_t channel = -1;
		if (TRX.ChannelMode) {
			band = getBandFromFreq(LCD_last_showed_freq_B, false);
			if (TRX.selected_vfo) {
				channel = getChannelbyFreq(LCD_last_showed_freq_B, false);
			} else {
				channel = getChannelbyFreq(LCD_last_showed_freq_B, true);
			}
		}
		if (TRX.ChannelMode && band >= 0 && BANDS[band].channelsCount > 0) {
			if (channel != -1 && strlen((char *)BANDS[band].channels[channel].subname) > 0) {
				sprintf(buff, "%.2s", (char *)BANDS[band].channels[channel].subname);
			} else {
				sprintf(buff, "CH");
			}
			addSymbols(buff, buff, 2, " ", false);
			LCDDriver_printText(buff, LAYOUT->FREQ_B_X_OFFSET_HZ, LAYOUT->FREQ_B_Y_BASELINE_SMALL - RASTR_FONT_H - 3, TRX.selected_vfo ? COLOR->FREQ_B_KHZ : COLOR->FREQ_B_INACTIVE, BG_COLOR, 1);

			if (channel != -1) {
				sprintf(buff, "%d", BANDS[band].channels[channel].number);
			} else {
				sprintf(buff, "-");
			}
			addSymbols(buff, buff, 2, " ", true);
			LCDDriver_printTextFont(buff, LAYOUT->FREQ_B_X_OFFSET_HZ + 2 + RASTR_FONT_W * 1 * 2, LAYOUT->FREQ_B_Y_BASELINE_SMALL, TRX.selected_vfo ? COLOR->STATUS_MODE : COLOR->FREQ_B_INACTIVE,
			                        BG_COLOR, LAYOUT->FREQ_CH_B_FONT);
		} else {
			LCDDriver_printTextFont(buff, LAYOUT->FREQ_B_X_OFFSET_HZ, LAYOUT->FREQ_B_Y_BASELINE_SMALL, TRX.selected_vfo ? COLOR->FREQ_B_HZ : COLOR->FREQ_B_INACTIVE, BG_COLOR,
			                        LAYOUT->FREQ_B_SMALL_FONT);
		}
		LCD_last_showed_freq_Hz_B = Hz_B;
	}
#endif

	if (redraw) {
		// Frequency delimiters
		LCDDriver_printTextFont(".", LAYOUT->FREQ_DELIMITER_X1_OFFSET, LAYOUT->FREQ_Y_BASELINE + LAYOUT->FREQ_DELIMITER_Y_OFFSET, !TRX.selected_vfo ? COLOR->FREQ_KHZ : COLOR->FREQ_A_INACTIVE,
		                        BG_COLOR, LAYOUT->FREQ_FONT);
		LCDDriver_printTextFont(".", LAYOUT->FREQ_DELIMITER_X2_OFFSET, LAYOUT->FREQ_Y_BASELINE + LAYOUT->FREQ_DELIMITER_Y_OFFSET, !TRX.selected_vfo ? COLOR->FREQ_HZ : COLOR->FREQ_A_INACTIVE,
		                        BG_COLOR, LAYOUT->FREQ_FONT);
#if (defined(LAY_800x480))
		LCDDriver_printTextFont(".", LAYOUT->FREQ_B_DELIMITER_X1_OFFSET, LAYOUT->FREQ_B_Y_BASELINE + LAYOUT->FREQ_B_DELIMITER_Y_OFFSET,
		                        TRX.selected_vfo ? COLOR->FREQ_B_KHZ : COLOR->FREQ_B_INACTIVE, BG_COLOR, LAYOUT->FREQ_B_FONT);
		LCDDriver_printTextFont(".", LAYOUT->FREQ_B_DELIMITER_X2_OFFSET, LAYOUT->FREQ_B_Y_BASELINE + LAYOUT->FREQ_B_DELIMITER_Y_OFFSET,
		                        TRX.selected_vfo ? COLOR->FREQ_B_HZ : COLOR->FREQ_B_INACTIVE, BG_COLOR, LAYOUT->FREQ_B_FONT);
#endif
	}

	NeedSaveSettings = true;

	LCD_UpdateQuery.FreqInfo = false;
	if (redraw) {
		LCD_UpdateQuery.FreqInfoRedraw = false;

		Last_showed_Hours = 255;
		Last_showed_Minutes = 255;
		Last_showed_Seconds = 255;
	}

	LCD_busy = false;
}

static void LCD_drawSMeter(void) {
	// analog version
	if (LAYOUT->STATUS_SMETER_ANALOG) {
		if (CN_Theme) {
			LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_BAR_X_OFFSET - 2, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET, &image_data_meter2, COLOR_BLACK, BG_COLOR);
		} else {
			LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_BAR_X_OFFSET - 2, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET, &image_data_meter, COLOR_BLACK, BG_COLOR);
		}
		return;
	}

	// Labels on the scale
	const float32_t step = LAYOUT->STATUS_SMETER_WIDTH / 15.0f;
	LCDDriver_printText("S", LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 0.0f) - 2, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y,
	                    COLOR->STATUS_BAR_LABELS, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	LCDDriver_printText("1", LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 1.0f) - 2, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y,
	                    COLOR->STATUS_BAR_LABELS, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	LCDDriver_printText("3", LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 3.0f) - 2, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y,
	                    COLOR->STATUS_BAR_LABELS, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	LCDDriver_printText("5", LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 5.0f) - 2, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y,
	                    COLOR->STATUS_BAR_LABELS, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	LCDDriver_printText("7", LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 7.0f) - 2, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y,
	                    COLOR->STATUS_BAR_LABELS, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	LCDDriver_printText("9", LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 9.0f) - 2, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y,
	                    COLOR->STATUS_BAR_LABELS, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	LCDDriver_printText("+20", LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 11.0f) - 10, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y,
	                    COLOR->STATUS_BAR_LABELS, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	LCDDriver_printText("+40", LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 13.0f) - 10, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y,
	                    COLOR->STATUS_BAR_LABELS, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	LCDDriver_printText("+60", LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 15.0f) - 10, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y,
	                    COLOR->STATUS_BAR_LABELS, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	for (uint8_t i = 0; i <= 15; i++) {
		uint16_t color = COLOR->STATUS_BAR_LEFT;
		if (i >= 9) {
			color = COLOR->STATUS_BAR_RIGHT;
		}
		if ((i % 2) != 0 || i == 0) {
			LCDDriver_drawFastVLine(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * i) - 1, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET, -4, color);
			LCDDriver_drawFastVLine(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * i), LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET, -6, color);
			LCDDriver_drawFastVLine(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * i) + 1, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET, -4, color);
		} else {
			LCDDriver_drawFastVLine(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * i), LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET, -3, color);
		}
	}

	// S-meter frame
	LCDDriver_drawRectXY(LAYOUT->STATUS_BAR_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET,
	                     LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 9.0f) - 2, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 1,
	                     COLOR->STATUS_BAR_LEFT);
	LCDDriver_drawRectXY(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 9.0f) - 1, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET,
	                     LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_SMETER_WIDTH, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 1,
	                     COLOR->STATUS_BAR_RIGHT);
	LCDDriver_drawRectXY(LAYOUT->STATUS_BAR_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + LAYOUT->STATUS_BAR_HEIGHT,
	                     LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 9.0f) - 2,
	                     LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + LAYOUT->STATUS_BAR_HEIGHT + 1, COLOR->STATUS_BAR_LEFT);
	LCDDriver_drawRectXY(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 9.0f) - 1,
	                     LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + LAYOUT->STATUS_BAR_HEIGHT,
	                     LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_SMETER_WIDTH,
	                     LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + LAYOUT->STATUS_BAR_HEIGHT + 1, COLOR->STATUS_BAR_RIGHT);
}

static void LCD_displayStatusInfoGUI(bool redraw) {
	// display RX / TX and s-meter
	if (LCD_systemMenuOpened || LCD_window.opened) {
		return;
	}
	if (LCD_busy) {
		if (redraw) {
			LCD_UpdateQuery.StatusInfoGUIRedraw = true;
		} else {
			LCD_UpdateQuery.StatusInfoGUI = true;
		}
		return;
	}
	LCD_busy = true;

	if (redraw) {
		LCDDriver_Fill_RectWH(0, LAYOUT->STATUS_Y_OFFSET, LCD_WIDTH, LAYOUT->STATUS_HEIGHT, BG_COLOR);
		if (LAYOUT->STATUS_SMETER_ANALOG) {
			LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET - 2, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET, LAYOUT->STATUS_SMETER_WIDTH + 12,
			                      LAYOUT->STATUS_SMETER_ANALOG_HEIGHT, BG_COLOR);
		}
	}

	if (TRX_on_TX) {
		if (TRX_Tune) {
			if (CN_Theme) {
				LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_TXRX_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_TXRX_Y_OFFSET), &TU_ICO, COLOR_BLACK, BG_COLOR);
			} else {
				LCDDriver_printTextFont("TU", LAYOUT->STATUS_TXRX_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_TXRX_Y_OFFSET), COLOR->STATUS_TU, BG_COLOR, LAYOUT->STATUS_TXRX_FONT);
			}
		} else {
			if (CN_Theme) {
				LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_TXRX_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_TXRX_Y_OFFSET), &TX_ICO, COLOR_BLACK, BG_COLOR);
			} else {
				LCDDriver_printTextFont("TX", LAYOUT->STATUS_TXRX_X_OFFSET + 1, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_TXRX_Y_OFFSET), COLOR->STATUS_TX, BG_COLOR, LAYOUT->STATUS_TXRX_FONT);
			}
		}

		// frame of the SWR meter
		const float32_t step = LAYOUT->STATUS_PMETER_WIDTH / 16.0f;
		LCDDriver_drawRectXY(LAYOUT->STATUS_BAR_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET,
		                     LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 9.0f), LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 1,
		                     COLOR->STATUS_BAR_LEFT);
		LCDDriver_drawRectXY(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 9.0f), LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET,
		                     LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_PMETER_WIDTH, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 1,
		                     COLOR->STATUS_BAR_RIGHT);
		LCDDriver_drawRectXY(LAYOUT->STATUS_BAR_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + LAYOUT->STATUS_BAR_HEIGHT,
		                     LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 9.0f),
		                     LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + LAYOUT->STATUS_BAR_HEIGHT + 1, COLOR->STATUS_BAR_LEFT);
		LCDDriver_drawRectXY(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 9.0f),
		                     LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + LAYOUT->STATUS_BAR_HEIGHT,
		                     LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_PMETER_WIDTH,
		                     LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + LAYOUT->STATUS_BAR_HEIGHT + 1, COLOR->STATUS_BAR_RIGHT);

		for (uint8_t i = 0; i <= 16; i++) {
			uint16_t color = COLOR->STATUS_BAR_LEFT;
			if (i > 9) {
				color = COLOR->STATUS_BAR_RIGHT;
			}
			if ((i % 2) == 0) {
				LCDDriver_drawFastVLine(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * i), LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET, -10, color);
			} else {
				LCDDriver_drawFastVLine(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * i), LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET, -5, color);
			}
		}

		LCDDriver_printText("SWR:", LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_TX_LABELS_OFFSET_X, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y,
		                    COLOR->STATUS_LABELS_TX, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
		LCDDriver_printText("FWD:", LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_TX_LABELS_OFFSET_X + LAYOUT->STATUS_TX_LABELS_MARGIN_X,
		                    LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, COLOR->STATUS_LABELS_TX, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
		LCDDriver_printText("REF:", LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_TX_LABELS_OFFSET_X + LAYOUT->STATUS_TX_LABELS_MARGIN_X * 2,
		                    LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, COLOR->STATUS_LABELS_TX, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);

		// frame of the ALC meter
		LCDDriver_drawRectXY(LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_PMETER_WIDTH + LAYOUT->STATUS_ALC_BAR_X_OFFSET,
		                     LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET,
		                     LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_PMETER_WIDTH + LAYOUT->STATUS_ALC_BAR_X_OFFSET + LAYOUT->STATUS_AMETER_WIDTH,
		                     LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 1, COLOR->STATUS_BAR_LEFT);
		LCDDriver_drawRectXY(LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_PMETER_WIDTH + LAYOUT->STATUS_ALC_BAR_X_OFFSET,
		                     LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + LAYOUT->STATUS_BAR_HEIGHT,
		                     LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_PMETER_WIDTH + LAYOUT->STATUS_ALC_BAR_X_OFFSET + LAYOUT->STATUS_AMETER_WIDTH,
		                     LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + LAYOUT->STATUS_BAR_HEIGHT + 1, COLOR->STATUS_BAR_LEFT);
		LCDDriver_printText("ALC:", LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_PMETER_WIDTH + LAYOUT->STATUS_ALC_BAR_X_OFFSET + LAYOUT->STATUS_TX_LABELS_OFFSET_X,
		                    LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, COLOR->STATUS_LABELS_TX, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	} else {
		LCD_UpdateQuery.StatusInfoBar = true;
		if (CN_Theme) {
			LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_TXRX_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_TXRX_Y_OFFSET), &RX_ICO, COLOR_BLACK, BG_COLOR);
		} else {
			LCDDriver_printTextFont("RX", LAYOUT->STATUS_TXRX_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_TXRX_Y_OFFSET), COLOR->STATUS_RX, BG_COLOR, LAYOUT->STATUS_TXRX_FONT);
		}
	}

	// VFO indicator
	if (!TRX.selected_vfo) // VFO-A
	{
		if (!TRX.Dual_RX) {
			if (CN_Theme) {
				LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_TXRX_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_VFO_Y_OFFSET), &VFO_A_ICO, COLOR_BLACK, BG_COLOR);
			} else {
				printInfo(LAYOUT->STATUS_VFO_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_VFO_Y_OFFSET), LAYOUT->STATUS_VFO_BLOCK_WIDTH, LAYOUT->STATUS_VFO_BLOCK_HEIGHT, "A",
				          COLOR->STATUS_VFO_BG, COLOR->STATUS_VFO, COLOR->STATUS_VFO, true);
			}
		} else if (TRX.Dual_RX_Type == VFO_A_AND_B) {
			if (CN_Theme) {
				LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_TXRX_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_VFO_Y_OFFSET), &VFO_A_B_ICO, COLOR_BLACK, BG_COLOR);
			} else {
				printInfo(LAYOUT->STATUS_VFO_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_VFO_Y_OFFSET), LAYOUT->STATUS_VFO_BLOCK_WIDTH, LAYOUT->STATUS_VFO_BLOCK_HEIGHT, "A&B",
				          COLOR->STATUS_VFO_BG, COLOR->STATUS_VFO, COLOR->STATUS_VFO, true);
			}
		} else {
			if (CN_Theme) {
				LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_TXRX_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_VFO_Y_OFFSET), &VFO_AB_ICO, COLOR_BLACK, BG_COLOR);
			} else {
				printInfo(LAYOUT->STATUS_VFO_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_VFO_Y_OFFSET), LAYOUT->STATUS_VFO_BLOCK_WIDTH, LAYOUT->STATUS_VFO_BLOCK_HEIGHT, "A+B",
				          COLOR->STATUS_VFO_BG, COLOR->STATUS_VFO, COLOR->STATUS_VFO, true);
			}
		}
	} else // VFO-B
	{
		if (!TRX.Dual_RX) {
			if (CN_Theme) {
				LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_TXRX_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_VFO_Y_OFFSET), &VFO_B_ICO, COLOR_BLACK, BG_COLOR);
			} else {
				printInfo(LAYOUT->STATUS_VFO_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_VFO_Y_OFFSET), LAYOUT->STATUS_VFO_BLOCK_WIDTH, LAYOUT->STATUS_VFO_BLOCK_HEIGHT, "B",
				          COLOR->STATUS_VFO_BG, COLOR->STATUS_VFO, COLOR->STATUS_VFO, true);
			}
		} else if (TRX.Dual_RX_Type == VFO_A_AND_B) {
			if (CN_Theme) {
				LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_TXRX_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_VFO_Y_OFFSET), &VFO_B_A_ICO, COLOR_BLACK, BG_COLOR);
			} else {
				printInfo(LAYOUT->STATUS_VFO_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_VFO_Y_OFFSET), LAYOUT->STATUS_VFO_BLOCK_WIDTH, LAYOUT->STATUS_VFO_BLOCK_HEIGHT, "B&A",
				          COLOR->STATUS_VFO_BG, COLOR->STATUS_VFO, COLOR->STATUS_VFO, true);
			}
		} else {
			if (CN_Theme) {
				LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_TXRX_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_VFO_Y_OFFSET), &VFO_BA_ICO, COLOR_BLACK, BG_COLOR);
			} else {
				printInfo(LAYOUT->STATUS_VFO_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_VFO_Y_OFFSET), LAYOUT->STATUS_VFO_BLOCK_WIDTH, LAYOUT->STATUS_VFO_BLOCK_HEIGHT, "B+A",
				          COLOR->STATUS_VFO_BG, COLOR->STATUS_VFO, COLOR->STATUS_VFO, true);
			}
		}
	}

	// Mode indicator
#if (defined(LAY_800x480))
	if (CN_Theme) {
		LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_MODE_ALL_X, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_MODE_Y_OFFSET), &MODE_ALL, COLOR_BLACK, BG_COLOR);

		if (TRX.VFO_A.Mode == TRX_MODE_AM) {
			LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_MODE_AM_X, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_MODE_Y_OFFSET), &MODE_ICO_AM, COLOR_BLACK, BG_COLOR);
		}
		if (TRX.VFO_A.Mode == TRX_MODE_SAM_STEREO) {
			LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_MODE_SAM_X, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_MODE_Y_OFFSET), &MODE_ICO_SAM, COLOR_BLACK, BG_COLOR);
		}
		if (TRX.VFO_A.Mode == TRX_MODE_CW) {
			LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_MODE_CW_X, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_MODE_Y_OFFSET), &MODE_ICO_CW, COLOR_BLACK, BG_COLOR);
		}
		if (TRX.VFO_A.Mode == TRX_MODE_DIGI_L) {
			LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_MODE_DIGL_X, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_MODE_Y_OFFSET), &MODE_ICO_DIGL, COLOR_BLACK, BG_COLOR);
		}
		if (TRX.VFO_A.Mode == TRX_MODE_DIGI_U) {
			LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_MODE_DIGU_X, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_MODE_Y_OFFSET), &MODE_ICO_DIGU, COLOR_BLACK, BG_COLOR);
		}
		if (TRX.VFO_A.Mode == TRX_MODE_NFM) {
			LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_MODE_NFM_X, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_MODE_Y_OFFSET), &MODE_ICO_NFM, COLOR_BLACK, BG_COLOR);
		}
		if (TRX.VFO_A.Mode == TRX_MODE_WFM) {
			LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_MODE_WFM_X, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_MODE_Y_OFFSET), &MODE_ICO_WFM, COLOR_BLACK, BG_COLOR);
		}
		if (TRX.VFO_A.Mode == TRX_MODE_LSB) {
			LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_MODE_LSB_X, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_MODE_Y_OFFSET), &MODE_ICO_LSB, COLOR_BLACK, BG_COLOR);
		}
		if (TRX.VFO_A.Mode == TRX_MODE_USB) {
			LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_MODE_USB_X, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_MODE_Y_OFFSET), &MODE_ICO_USB, COLOR_BLACK, BG_COLOR);
		}
		// if (TRX.VFO_A.Mode == TRX_MODE_IQ){
		//  LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_MODE_IQ_X, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_MODE_Y_OFFSET), &MODE_ICO_IQ,
		//  COLOR_BLACK, BG_COLOR);
		// }
		// if (TRX.VFO_A.Mode == TRX_MODE_LOOPBACK) {
		//	LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_MODE_LOOP_X, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_MODE_Y_OFFSET), &MODE_ICO_LOOP, COLOR_BLACK, BG_COLOR);
		//}
		if (TRX.VFO_A.Mode == TRX_MODE_RTTY) {
			LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_MODE_RTTY_X, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_MODE_Y_OFFSET), &MODE_ICO_RTTY, COLOR_BLACK, BG_COLOR);
		}
		LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_MODE_SELECT_X, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_MODE_Y_OFFSET), &MODE_ICO_SELECT, COLOR_BLACK, BG_COLOR);

		// VFO B
		LCDDriver_Fill_RectWH(LAYOUT->STATUS_MODE_B_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_MODE_B_Y_OFFSET), MODE_ICO_DIGL.width, MODE_ICO_DIGL.height, BG_COLOR);

		if (TRX.VFO_B.Mode == TRX_MODE_AM) {
			LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_MODE_B_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_MODE_B_Y_OFFSET), &MODE_ICO_AM, COLOR_BLACK, BG_COLOR);
		}
		if (TRX.VFO_B.Mode == TRX_MODE_SAM_STEREO || TRX.VFO_B.Mode == TRX_MODE_SAM_LSB || TRX.VFO_B.Mode == TRX_MODE_SAM_USB) {
			LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_MODE_B_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_MODE_B_Y_OFFSET), &MODE_ICO_SAM, COLOR_BLACK, BG_COLOR);
		}
		if (TRX.VFO_B.Mode == TRX_MODE_CW) {
			LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_MODE_B_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_MODE_B_Y_OFFSET), &MODE_ICO_CW, COLOR_BLACK, BG_COLOR);
		}
		if (TRX.VFO_B.Mode == TRX_MODE_DIGI_L) {
			LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_MODE_B_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_MODE_B_Y_OFFSET), &MODE_ICO_DIGL, COLOR_BLACK, BG_COLOR);
		}
		if (TRX.VFO_B.Mode == TRX_MODE_DIGI_U) {
			LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_MODE_B_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_MODE_B_Y_OFFSET), &MODE_ICO_DIGU, COLOR_BLACK, BG_COLOR);
		}
		if (TRX.VFO_B.Mode == TRX_MODE_WFM) {
			LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_MODE_B_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_MODE_B_Y_OFFSET), &MODE_ICO_WFM, COLOR_BLACK, BG_COLOR);
		}
		if (TRX.VFO_B.Mode == TRX_MODE_NFM) {
			LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_MODE_B_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_MODE_B_Y_OFFSET), &MODE_ICO_NFM, COLOR_BLACK, BG_COLOR);
		}
		// if (TRX.VFO_B.Mode == TRX_MODE_IQ){
		//  LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_MODE_B_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_MODE_B_Y_OFFSET),
		//  &MODE_ICO_IQ, COLOR_BLACK, BG_COLOR);
		// }
		if (TRX.VFO_B.Mode == TRX_MODE_LOOPBACK) {
			LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_MODE_B_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_MODE_B_Y_OFFSET), &MODE_ICO_LOOP, COLOR_BLACK, BG_COLOR);
		}
		if (TRX.VFO_B.Mode == TRX_MODE_LSB) {
			LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_MODE_B_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_MODE_B_Y_OFFSET), &MODE_ICO_LSB, COLOR_BLACK, BG_COLOR);
		}
		if (TRX.VFO_B.Mode == TRX_MODE_USB) {
			LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_MODE_B_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_MODE_B_Y_OFFSET), &MODE_ICO_USB, COLOR_BLACK, BG_COLOR);
		}
		if (TRX.VFO_B.Mode == TRX_MODE_RTTY) {
			LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_MODE_B_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_MODE_B_Y_OFFSET), &MODE_ICO_RTTY, COLOR_BLACK, BG_COLOR);
		}
	} else {
		printInfo(LAYOUT->STATUS_MODE_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_MODE_Y_OFFSET), LAYOUT->STATUS_MODE_BLOCK_WIDTH, LAYOUT->STATUS_MODE_BLOCK_HEIGHT,
		          (char *)MODE_DESCR[TRX.VFO_A.Mode], BG_COLOR, COLOR->STATUS_MODE, COLOR->STATUS_MODE, true);
		printInfoSmall(LAYOUT->STATUS_MODE_B_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_MODE_B_Y_OFFSET), LAYOUT->STATUS_MODE_BLOCK_WIDTH, LAYOUT->STATUS_MODE_BLOCK_HEIGHT,
		               (char *)MODE_DESCR[TRX.VFO_B.Mode], BG_COLOR, COLOR->STATUS_MODE, COLOR->STATUS_MODE, true);
	}
#else
	printInfo(LAYOUT->STATUS_MODE_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_MODE_Y_OFFSET), LAYOUT->STATUS_MODE_BLOCK_WIDTH, LAYOUT->STATUS_MODE_BLOCK_HEIGHT,
	          (char *)MODE_DESCR[CurrentVFO->Mode], BG_COLOR, COLOR->STATUS_MODE, COLOR->STATUS_MODE, true);
#endif
	// Redraw TextBar
	if (NeedProcessDecoder) {
		LCDDriver_Fill_RectWH(0, LAYOUT->FFT_FFTWTF_BOTTOM - LAYOUT->FFT_CWDECODER_OFFSET, LAYOUT->FFT_PRINT_SIZE, LAYOUT->FFT_CWDECODER_OFFSET, BG_COLOR);
		LCD_UpdateQuery.TextBar = true;
	}
	// ANT indicator
	uint8_t currentAnt = TRX_on_TX ? TRX.ANT_TX : TRX.ANT_RX;
	if (CN_Theme) {
		if (TRX.ANT_RX != TRX.ANT_TX && !TRX_on_TX) {
			LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_ANT_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_ANT_Y_OFFSET), &ANT1_2_ICO, COLOR_BLACK, BG_COLOR);
		} else {
			const tIMAGE *icon = &ANT1_ICO;
			if (currentAnt == TRX_ANT_2) {
				icon = &ANT2_ICO;
			}
			if (currentAnt == TRX_ANT_3) {
				icon = &ANT3_ICO;
			}
			if (currentAnt == TRX_ANT_4) {
				icon = &ANT4_ICO;
			}
			LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_ANT_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_ANT_Y_OFFSET), icon, COLOR_BLACK, BG_COLOR);
		}
	} else {
		char ctmp[10];
		if (TRX.ANT_RX != TRX.ANT_TX && !TRX_on_TX) {
			sprintf(ctmp, "%d+%d", TRX.ANT_RX + 1, TRX.ANT_TX + 1);
			printInfoSmall(LAYOUT->STATUS_ANT_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_ANT_Y_OFFSET), LAYOUT->STATUS_ANT_BLOCK_WIDTH, LAYOUT->STATUS_ANT_BLOCK_HEIGHT, ctmp, BG_COLOR,
			               COLOR->STATUS_RX, COLOR->STATUS_RX, true);
		} else {
			sprintf(ctmp, "ANT%d", currentAnt + 1);
			printInfoSmall(LAYOUT->STATUS_ANT_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_ANT_Y_OFFSET), LAYOUT->STATUS_ANT_BLOCK_WIDTH, LAYOUT->STATUS_ANT_BLOCK_HEIGHT, ctmp, BG_COLOR,
			               COLOR->STATUS_RX, COLOR->STATUS_RX, true);
		}
	}

	// WIFI indicator
	if (WIFI_connected) {
		LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_WIFI_ICON_X, LAYOUT->STATUS_WIFI_ICON_Y, &IMAGES_wifi_active, COLOR_BLACK, BG_COLOR);
	} else {
		LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_WIFI_ICON_X, LAYOUT->STATUS_WIFI_ICON_Y, &IMAGES_wifi_inactive, COLOR_BLACK, BG_COLOR);
	}

	if (CN_Theme) {
		// SD indicator
		if (SD_Present) {
			LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_SD_ICON_X, LAYOUT->STATUS_SD_ICON_Y, &IMAGES_sd_active, COLOR_BLACK, BG_COLOR);
		} else {
			LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_SD_ICON_X, LAYOUT->STATUS_SD_ICON_Y, &IMAGES_sd_inactive, COLOR_BLACK, BG_COLOR);
		}
		if (FAN_Active) {
			LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_FAN_ICON_X, LAYOUT->STATUS_FAN_ICON_Y, &IMAGES_fan2, COLOR_BLACK, BG_COLOR);
		}
		if (!FAN_Active) {
			printInfoSmall(LAYOUT->STATUS_FAN_ICON_X, LAYOUT->STATUS_FAN_ICON_Y, 22, 22, "   ", BG_COLOR, COLOR->STATUS_RX, COLOR->STATUS_RX, true);
		}
	} else {
		if (FAN_Active) {
			LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_FAN_ICON_X, LAYOUT->STATUS_FAN_ICON_Y, &IMAGES_fan, COLOR_BLACK, BG_COLOR);
		} else {
			// SD indicator
			if (SD_Present) {
				LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_SD_ICON_X, LAYOUT->STATUS_SD_ICON_Y, &IMAGES_sd_active, COLOR_BLACK, BG_COLOR);
			} else {
				LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_SD_ICON_X, LAYOUT->STATUS_SD_ICON_Y, &IMAGES_sd_inactive, COLOR_BLACK, BG_COLOR);
			}
		}
	}

	/////BW trapezoid
	LCDDriver_Fill_RectWH(LAYOUT->BW_TRAPEZ_POS_X, LAYOUT->BW_TRAPEZ_POS_Y, LAYOUT->BW_TRAPEZ_WIDTH, LAYOUT->BW_TRAPEZ_HEIGHT,
	                      BG_COLOR); // clear back
#define bw_trapez_margin 10
	uint16_t bw_trapez_top_line_width = (uint16_t)(LAYOUT->BW_TRAPEZ_WIDTH * 0.9f - bw_trapez_margin * 2);
	// border
	LCDDriver_drawFastHLine(LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2 - bw_trapez_top_line_width / 2, LAYOUT->BW_TRAPEZ_POS_Y, bw_trapez_top_line_width,
	                        COLOR->BW_TRAPEZ_BORDER); // top
	LCDDriver_drawFastHLine(LAYOUT->BW_TRAPEZ_POS_X, LAYOUT->BW_TRAPEZ_POS_Y + LAYOUT->BW_TRAPEZ_HEIGHT, LAYOUT->BW_TRAPEZ_WIDTH,
	                        COLOR->BW_TRAPEZ_BORDER); // bottom
	LCDDriver_drawLine(LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2 - bw_trapez_top_line_width / 2 - bw_trapez_margin, LAYOUT->BW_TRAPEZ_POS_Y + LAYOUT->BW_TRAPEZ_HEIGHT,
	                   LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2 - bw_trapez_top_line_width / 2, LAYOUT->BW_TRAPEZ_POS_Y,
	                   COLOR->BW_TRAPEZ_BORDER); // left
	LCDDriver_drawLine(LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2 + bw_trapez_top_line_width / 2 + bw_trapez_margin, LAYOUT->BW_TRAPEZ_POS_Y + LAYOUT->BW_TRAPEZ_HEIGHT,
	                   LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2 + bw_trapez_top_line_width / 2, LAYOUT->BW_TRAPEZ_POS_Y,
	                   COLOR->BW_TRAPEZ_BORDER); // right
	// bw fill
	float32_t bw_trapez_bw_left_width = 1.0f;
	float32_t bw_trapez_bw_right_width = 1.0f;
	float32_t bw_trapez_bw_hpf_margin = 0.0f;
	switch (CurrentVFO->Mode) {
	case TRX_MODE_DIGI_L:
	case TRX_MODE_LSB:
		bw_trapez_bw_hpf_margin = 1.0f / (float32_t)MAX_LPF_WIDTH_SSB * (TRX_on_TX ? CurrentVFO->HPF_TX_Filter_Width : CurrentVFO->HPF_RX_Filter_Width);
		bw_trapez_bw_left_width = 1.0f / (float32_t)MAX_LPF_WIDTH_SSB * (TRX_on_TX ? CurrentVFO->LPF_TX_Filter_Width : CurrentVFO->LPF_RX_Filter_Width);
		bw_trapez_bw_right_width = 0.0f;
		break;
	case TRX_MODE_RTTY:
	case TRX_MODE_DIGI_U:
	case TRX_MODE_USB:
		bw_trapez_bw_left_width = 0.0f;
		bw_trapez_bw_hpf_margin = 1.0f / (float32_t)MAX_LPF_WIDTH_SSB * (TRX_on_TX ? CurrentVFO->HPF_TX_Filter_Width : CurrentVFO->HPF_RX_Filter_Width);
		bw_trapez_bw_right_width = 1.0f / (float32_t)MAX_LPF_WIDTH_SSB * (TRX_on_TX ? CurrentVFO->LPF_TX_Filter_Width : CurrentVFO->LPF_RX_Filter_Width);
		break;
	case TRX_MODE_CW:
		bw_trapez_bw_left_width = 1.0f / (float32_t)MAX_LPF_WIDTH_CW * CurrentVFO->LPF_RX_Filter_Width;
		bw_trapez_bw_right_width = bw_trapez_bw_left_width;
		break;
	case TRX_MODE_NFM:
		bw_trapez_bw_left_width = 1.0f / (float32_t)MAX_LPF_WIDTH_NFM * (TRX_on_TX ? CurrentVFO->LPF_TX_Filter_Width : CurrentVFO->LPF_RX_Filter_Width);
		bw_trapez_bw_right_width = bw_trapez_bw_left_width;
		break;
	case TRX_MODE_AM:
	case TRX_MODE_SAM_STEREO:
	case TRX_MODE_SAM_LSB:
	case TRX_MODE_SAM_USB:
		bw_trapez_bw_left_width = 1.0f / (float32_t)MAX_LPF_WIDTH_AM * (TRX_on_TX ? CurrentVFO->LPF_TX_Filter_Width : CurrentVFO->LPF_RX_Filter_Width);
		bw_trapez_bw_right_width = bw_trapez_bw_left_width;
		break;
	case TRX_MODE_IQ:
	case TRX_MODE_WFM:
	case TRX_MODE_LOOPBACK:
		bw_trapez_bw_left_width = 1.0f;
		bw_trapez_bw_right_width = 1.0f;
		break;
	}
	uint16_t bw_trapez_left_width = (uint16_t)(bw_trapez_top_line_width / 2 * bw_trapez_bw_left_width);
	uint16_t bw_trapez_right_width = (uint16_t)(bw_trapez_top_line_width / 2 * bw_trapez_bw_right_width);
	uint16_t bw_trapez_bw_hpf_margin_width = (uint16_t)(bw_trapez_top_line_width / 2 * bw_trapez_bw_hpf_margin);
	uint16_t bw_trapez_bw_hpf_margin_width_offset = 0;
	if (bw_trapez_bw_hpf_margin_width > bw_trapez_margin) {
		bw_trapez_bw_hpf_margin_width_offset = bw_trapez_bw_hpf_margin_width - bw_trapez_margin;
	}
	if (bw_trapez_bw_left_width > 0) // left wing
	{
		LCDDriver_Fill_RectWH(LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2 - bw_trapez_left_width, LAYOUT->BW_TRAPEZ_POS_Y + 1, bw_trapez_left_width - bw_trapez_bw_hpf_margin_width,
		                      LAYOUT->BW_TRAPEZ_HEIGHT - 2, COLOR->BW_TRAPEZ_FILL);
		LCDDriver_Fill_Triangle(LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2 - bw_trapez_left_width, LAYOUT->BW_TRAPEZ_POS_Y + LAYOUT->BW_TRAPEZ_HEIGHT - 1,
		                        LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2 - bw_trapez_left_width, LAYOUT->BW_TRAPEZ_POS_Y + 1,
		                        LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2 - bw_trapez_left_width - bw_trapez_margin + 1, LAYOUT->BW_TRAPEZ_POS_Y + LAYOUT->BW_TRAPEZ_HEIGHT - 1,
		                        COLOR->BW_TRAPEZ_FILL);
		if (bw_trapez_bw_hpf_margin_width > 0) {
			LCDDriver_Fill_Triangle(LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2 - bw_trapez_bw_hpf_margin_width, LAYOUT->BW_TRAPEZ_POS_Y + 1,
			                        LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2 - bw_trapez_bw_hpf_margin_width, LAYOUT->BW_TRAPEZ_POS_Y + LAYOUT->BW_TRAPEZ_HEIGHT - 1,
			                        LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2 - bw_trapez_bw_hpf_margin_width_offset, LAYOUT->BW_TRAPEZ_POS_Y + LAYOUT->BW_TRAPEZ_HEIGHT - 1,
			                        COLOR->BW_TRAPEZ_FILL);
		}
	}
	if (bw_trapez_bw_right_width > 0) // right wing
	{
		LCDDriver_Fill_RectWH(LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2 + bw_trapez_bw_hpf_margin_width, LAYOUT->BW_TRAPEZ_POS_Y + 1,
		                      bw_trapez_right_width - bw_trapez_bw_hpf_margin_width, LAYOUT->BW_TRAPEZ_HEIGHT - 2, COLOR->BW_TRAPEZ_FILL);
		LCDDriver_Fill_Triangle(LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2 + bw_trapez_right_width + bw_trapez_margin - 1, LAYOUT->BW_TRAPEZ_POS_Y + LAYOUT->BW_TRAPEZ_HEIGHT - 1,
		                        LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2 + bw_trapez_right_width, LAYOUT->BW_TRAPEZ_POS_Y + 1,
		                        LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2 + bw_trapez_right_width, LAYOUT->BW_TRAPEZ_POS_Y + LAYOUT->BW_TRAPEZ_HEIGHT - 1, COLOR->BW_TRAPEZ_FILL);
		if (bw_trapez_bw_hpf_margin_width > 0) {
			LCDDriver_Fill_Triangle(LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2 + bw_trapez_bw_hpf_margin_width, LAYOUT->BW_TRAPEZ_POS_Y + 1,
			                        LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2 + bw_trapez_bw_hpf_margin_width, LAYOUT->BW_TRAPEZ_POS_Y + LAYOUT->BW_TRAPEZ_HEIGHT - 1,
			                        LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2 + bw_trapez_bw_hpf_margin_width_offset, LAYOUT->BW_TRAPEZ_POS_Y + LAYOUT->BW_TRAPEZ_HEIGHT - 1,
			                        COLOR->BW_TRAPEZ_FILL);
		}
	}
	// shift stripe
	if ((!TRX.RIT_Enabled && !TRX.XIT_Enabled) || LCD_bw_trapez_stripe_pos == 0) {
		LCD_bw_trapez_stripe_pos = LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2;
	}
	LCDDriver_Fill_RectWH(LCD_bw_trapez_stripe_pos - 1, LAYOUT->BW_TRAPEZ_POS_Y + LAYOUT->BW_TRAPEZ_HEIGHT / 2, 3, LAYOUT->BW_TRAPEZ_HEIGHT / 2, COLOR->BW_TRAPEZ_STRIPE);
	/////END BW trapezoid

	LCD_UpdateQuery.StatusInfoGUI = false;
	if (redraw) {
		LCD_UpdateQuery.StatusInfoGUIRedraw = false;
	}
	LCD_UpdateQuery.StatusInfoBarRedraw = redraw;
	LCD_busy = false;
}

static float32_t LCD_GetSMeterValPosition(float32_t dBm, bool correct_vhf) {
	int32_t width = LAYOUT->STATUS_SMETER_WIDTH - 2;
	float32_t TRX_s_meter = 0;
	if (!LAYOUT->STATUS_SMETER_ANALOG) // digital version
	{
		TRX_s_meter = (127.0f + dBm); // 127dBm - S0, 6dBm - 1S div
		if (correct_vhf && CurrentVFO->Freq >= VHF_S_METER_FREQ_START) {
			TRX_s_meter = (147.0f + dBm); // 147dBm - S0 for frequencies above 144MHz
		}

		if (TRX_s_meter < 54.01f) { // first 9 points of meter is 6 dB each
			TRX_s_meter = (width / 15.0f) * (TRX_s_meter / 6.0f);
		} else { // the remaining 6 points, 10 dB each
			TRX_s_meter = ((width / 15.0f) * 9.0f) + ((TRX_s_meter - 54.0f) / 10.0f) * (width / 15.0f);
		}

		TRX_s_meter += 1.0f;
		if (TRX_s_meter > width) {
			TRX_s_meter = width;
		}
		if (TRX_s_meter < 1.0f) {
			TRX_s_meter = 1.0f;
		}
	} else // analog meter version
	{
		TRX_s_meter = (127.0f + dBm); // 127dBm - S0, 6dBm - 1S div
		if (correct_vhf && CurrentVFO->Freq >= VHF_S_METER_FREQ_START) {
			TRX_s_meter = (147.0f + dBm); // 147dBm - S0 for frequencies above 144MHz
		}

		if (TRX_s_meter < 54.01f) { // first 9 points of meter is 6 dB each
			TRX_s_meter = (width / 17.0f) * (TRX_s_meter / 6.0f);
		} else { // the remaining points, 10 dB each
			TRX_s_meter = ((width / 17.0f) * 9.0f) + ((TRX_s_meter - 54.0f) / 10.0f) * (width / 14.0f);
		}

		// ugly corrections :/
		if (dBm > -5.0f) { // > S9+60
			TRX_s_meter -= 2.0f;
		}
		if (dBm > -15.0f) { // > S9+50
			TRX_s_meter += 3.0f;
		}
		if (dBm > -25.0f) { // > S9+40
			TRX_s_meter += 2.0f;
		}
		if (dBm > -35.0f) { // > S9+30
			TRX_s_meter += 2.0f;
		}
		if (dBm > -45.0f) { // > S9+20
			TRX_s_meter += 2.0f;
		}
		if (dBm > -75.0f) { // > S8
			TRX_s_meter -= 2.0f;
		}
		if (dBm < -87.0f) { // < S7
			TRX_s_meter += 2.0f;
		}
		if (dBm < -100.0f) { // < S5
			TRX_s_meter += 2.0f;
		}
		if (dBm < -104.0f) { // < S4
			TRX_s_meter += 2.0f;
		}
		/*if(dBm < -117.0f) // < S2
		  TRX_s_meter -= 7.0f;*/
		if (dBm < -122.0f) { // < S1
			TRX_s_meter += 2.0f;
		}

		if (TRX_s_meter > width + 60.0f) {
			TRX_s_meter = width + 60.0f;
		}
		if (TRX_s_meter < -30.0f) {
			TRX_s_meter = -30.0f;
		}
	}
	return TRX_s_meter;
}

static float32_t LCD_GetAnalowPowerValPosition(float32_t _pwr) {
	int32_t width = LAYOUT->STATUS_SMETER_WIDTH - 2;
	float32_t pos = 20.0f; // zero

	float32_t pwr = _pwr;
	if (CALIBRATE.MAX_RF_POWER_ON_METER <= 20) {
		pwr *= 10.0f; // power scaling
	}

	// ugly corrections :/

	if (pwr > 0) { // 0-10w
		pos += fminf(10, pwr) * 4.0f;
		pwr -= 10.0f;
	}

	if (pwr > 0) { // 10-20w
		pos += fminf(10, pwr) * 2.1f;
		pwr -= 10.0f;
	}

	if (pwr > 0) { // 20-30w
		pos += fminf(10, pwr) * 1.0f;
		pwr -= 10.0f;
	}

	if (pwr > 0) { // 30-40w
		pos += fminf(10, pwr) * 1.5f;
		pwr -= 10.0f;
	}

	if (pwr > 0) { // 40-50w
		pos += fminf(10, pwr) * 1.3f;
		pwr -= 10.0f;
	}

	if (pwr > 0) { // 50-90w
		pos += fminf(40, pwr) * 1.0f;
		pwr -= 40.0f;
	}

	if (pwr > 0) { // 90w+
		pos += pwr * 0.9f;
	}

	// limits
	if (pos > width + 60.0f) {
		pos = width + 60.0f;
	}
	if (pos < -30.0f) {
		pos = -30.0f;
	}

	return pos;
}

static void LCD_PrintMeterArrow(int16_t target_pixel_x, int16_t target_color) {
	if (target_pixel_x < 0) {
		target_pixel_x = 0;
	}
	if (target_pixel_x > 220) {
		target_pixel_x = 220;
	}
	float32_t x0 = LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_SMETER_WIDTH / 2 + 2;
	float32_t y0 = LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_SMETER_ANALOG_HEIGHT + 140;
	float32_t x1 = LAYOUT->STATUS_BAR_X_OFFSET + target_pixel_x;
	float32_t y1 = LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + 1;

	// length cut
	const uint32_t max_length = 220;
	float32_t x_diff;
	float32_t y_diff;
	float32_t length;
	arm_sqrt_f32((x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0), &length);
	if (length > max_length) {
		float32_t coeff = (float32_t)max_length / length;
		x_diff = (x1 - x0) * coeff;
		y_diff = (y1 - y0) * coeff;
		x1 = x0 + x_diff;
		y1 = y0 + y_diff;
	}
	// right cut
	uint16_t tryes = 0;
	while ((x1 > LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_SMETER_WIDTH) && tryes < 100) {
		x_diff = (x1 - x0) * 0.99f;
		y_diff = (y1 - y0) * 0.99f;
		x1 = x0 + x_diff;
		y1 = y0 + y_diff;
		tryes++;
	}
	// left cut
	tryes = 0;
	while ((x1 < LAYOUT->STATUS_BAR_X_OFFSET) && tryes < 100) {
		x_diff = (x1 - x0) * 0.99f;
		y_diff = (y1 - y0) * 0.99f;
		x1 = x0 + x_diff;
		y1 = y0 + y_diff;
		tryes++;
	}
	// start cut
	tryes = 0;
	while ((y0 > LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_SMETER_ANALOG_HEIGHT) && tryes < 150) {
		x_diff = (x1 - x0) * 0.99f;
		y_diff = (y1 - y0) * 0.99f;
		x0 = x1 - x_diff;
		y0 = y1 - y_diff;
		tryes++;
	}

	// draw
	if (x1 < x0) {
		LCDDriver_drawLine(x0, y0, x1, y1, target_color);
		LCDDriver_drawLine(x0 + 1, y0, x1 + 1, y1, target_color);
	} else {
		LCDDriver_drawLine(x0, y0, x1, y1, target_color);
		LCDDriver_drawLine(x0 - 1, y0, x1 - 1, y1, target_color);
	}
}

static float32_t LCD_SWR2DBM_meter(float32_t swr) {
	if (swr < 1.0f) {
		swr = 1.0f;
	}
	if (swr > 8.0f) {
		swr = 8.0f;
	}

	float32_t swr_to_dBm = -115.0f;
	if (swr <= 1.5f) {
		swr_to_dBm += (swr - 1.0f) * 40.0f;
	}
	if (swr > 1.5f && swr <= 2.0f) {
		swr_to_dBm += 0.5f * 40.0f + (swr - 1.5f) * 20.0f;
	}
	if (swr > 2.0f && swr <= 3.0f) {
		swr_to_dBm += 0.5f * 40.0f + 0.5f * 20.0f + (swr - 2.0f) * 10.0f;
	}
	if (swr > 3.0f) {
		swr_to_dBm += 0.5f * 40.0f + 0.5f * 20.0f + 1.0f * 20.0f + (swr - 3.0f) * 10.0f;
	}

	return swr_to_dBm;
}

static void LCD_displayStatusInfoBar(bool redraw) {
	// S-meter and other information
	if (LCD_systemMenuOpened || LCD_window.opened) {
		return;
	}
	if (LCD_busy) {
		if (redraw) {
			LCD_UpdateQuery.StatusInfoBarRedraw = true;
		}
		LCD_UpdateQuery.StatusInfoBar = true;
		return;
	}
	LCD_busy = true;

	static bool old_TRX_on_TX = false;
	char ctmp[50];

	if (!TRX_on_TX) {
		static float32_t TRX_RX1_dBm_lowrate = 0;
		static uint32_t TRX_RX1_dBm_lowrate_time = 0;
		if ((HAL_GetTick() - TRX_RX1_dBm_lowrate_time) > 100) {
			TRX_RX1_dBm_lowrate_time = HAL_GetTick();
			TRX_RX1_dBm_lowrate = TRX_RX1_dBm_lowrate * 0.5f + TRX_RX1_dBm * 0.5f;
		}

		float32_t s_width = 0.0f;
		static uint16_t sql_stripe_x_pos_old = 0;

		if (CurrentVFO->Mode == TRX_MODE_CW) {
			s_width = LCD_last_s_meter * 0.5f + LCD_GetSMeterValPosition(TRX_RX1_dBm_lowrate, true) * 0.5f; // smooth CW faster!
		} else {
			s_width = LCD_last_s_meter * 0.75f + LCD_GetSMeterValPosition(TRX_RX1_dBm_lowrate, true) * 0.25f; // smooth the movement of the S-meter
		}
		// println(LCD_last_s_meter, " ", s_width, " ", TRX_RX1_dBm, " ", LCD_GetSMeterValPosition(TRX_RX1_dBm, true));

		// digital s-meter version
		static uint32_t last_s_meter_draw_time = 0;
		if (!LAYOUT->STATUS_SMETER_ANALOG &&
		    (redraw || (fabsf(LCD_last_s_meter - s_width) >= 1.0f) || (fabsf(LCD_smeter_peak_x - s_width) >= 1.0f) || (HAL_GetTick() - last_s_meter_draw_time) > 500)) {
			bool show_sql_stripe = (CurrentVFO->Mode == TRX_MODE_NFM || CurrentVFO->Mode == TRX_MODE_WFM) && CurrentVFO->SQL;
			last_s_meter_draw_time = HAL_GetTick();
			uint16_t sql_stripe_x_pos = show_sql_stripe ? LCD_GetSMeterValPosition(CurrentVFO->FM_SQL_threshold_dBm, true) : 0;

			// clear old bar
			if ((LCD_last_s_meter - s_width) > 0) {
				LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)s_width, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 2,
				                      (uint16_t)(LCD_last_s_meter - s_width + 1), LAYOUT->STATUS_BAR_HEIGHT - 3, BG_COLOR);
			}
			// and stripe
			LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)LCD_last_s_meter, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + 5, 2, LAYOUT->STATUS_SMETER_MARKER_HEIGHT,
			                      BG_COLOR);
			LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)s_width, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + 5, 2, LAYOUT->STATUS_SMETER_MARKER_HEIGHT,
			                      COLOR->STATUS_SMETER_STRIPE);
			// clear old SQL stripe
			if (sql_stripe_x_pos_old != sql_stripe_x_pos) {
				LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + sql_stripe_x_pos_old, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 2, 2,
				                      LAYOUT->STATUS_BAR_HEIGHT - 3, BG_COLOR);
				sql_stripe_x_pos_old = sql_stripe_x_pos;
			}

			// bar
			const float32_t s9_position = LAYOUT->STATUS_SMETER_WIDTH / 15.0f * 9.0f;
			if (s_width <= s9_position) {
				LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 2, (uint16_t)s_width,
				                      LAYOUT->STATUS_BAR_HEIGHT - 3, COLOR->STATUS_SMETER);
			} else {
				LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 2, (uint16_t)s9_position,
				                      LAYOUT->STATUS_BAR_HEIGHT - 3, COLOR->STATUS_SMETER);
				LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + s9_position, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 2,
				                      (uint16_t)(s_width - s9_position), LAYOUT->STATUS_BAR_HEIGHT - 3, COLOR->STATUS_SMETER_HIGH);
			}

			// peak
			static uint32_t smeter_peak_settime = 0;
			if (LCD_smeter_peak_x > s_width) {
				LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + LCD_smeter_peak_x, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 2, 2,
				                      LAYOUT->STATUS_BAR_HEIGHT - 3, BG_COLOR); // clear old peak
			}
			if (LCD_smeter_peak_x > 0 && ((HAL_GetTick() - smeter_peak_settime) > LAYOUT->STATUS_SMETER_PEAK_HOLDTIME)) {
				LCD_smeter_peak_x--;
			}
			if (s_width > LCD_smeter_peak_x) {
				LCD_smeter_peak_x = s_width;
				smeter_peak_settime = HAL_GetTick();
			}
			LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + LCD_smeter_peak_x, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 2, 2,
			                      LAYOUT->STATUS_BAR_HEIGHT - 3, COLOR->STATUS_SMETER_PEAK);

			// FM Squelch stripe
			if (show_sql_stripe) {
				LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + sql_stripe_x_pos, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 2, 2,
				                      LAYOUT->STATUS_BAR_HEIGHT - 3, COLOR->STATUS_SMETER_FM_SQL);
			}

			// redraw s-meter gui and stripe
			LCD_drawSMeter();
			LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)s_width, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + 5, 2, LAYOUT->STATUS_SMETER_MARKER_HEIGHT,
			                      COLOR->STATUS_SMETER_STRIPE);

			LCD_last_s_meter = s_width;
		}

		// analog s-meter version
		if (LAYOUT->STATUS_SMETER_ANALOG && (redraw || (fabsf(LCD_last_s_meter - s_width) >= 0.1f))) {
			// analog meter SQL arrow
			float32_t sql_arrow = -9999.0f;
			if (CurrentVFO->SQL && (CurrentVFO->Mode == TRX_MODE_NFM || CurrentVFO->Mode == TRX_MODE_WFM)) {
				sql_arrow = LCD_GetSMeterValPosition(CurrentVFO->FM_SQL_threshold_dBm, true);
			}
			// redraw s-meter gui and line
			LCD_drawSMeter();
			if (sql_arrow != -9999.0f) {
				LCD_PrintMeterArrow(sql_arrow, COLOR->STATUS_SMETER_FM_SQL);
			}
			LCD_PrintMeterArrow(s_width, COLOR->STATUS_SMETER_STRIPE);
			LCD_last_s_meter = s_width;
		}

		// print dBm value
		bool vhf_s_smeter = CurrentVFO->Freq >= VHF_S_METER_FREQ_START;
		if (CN_Theme) {
			LCDDriver_printTextFont(TRX.CALLSIGN, 260, 90, COLOR->FREQ_B_MHZ, BG_COLOR, &FreeSans9pt7b);
		}
		sprintf(ctmp, "%ddBm", (int16_t)TRX_RX1_dBm_lowrate);
		addSymbols(ctmp, ctmp, 7, " ", true);
		LCDDriver_printText(ctmp, LAYOUT->STATUS_LABEL_DBM_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABEL_DBM_Y_OFFSET,
		                    (int16_t)TRX_RX1_dBm_lowrate < (vhf_s_smeter ? -93 : -73) ? COLOR->STATUS_LABEL_DBM : COLOR->STATUS_SMETER_HIGH, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);

		// print s-meter value
		static float32_t TRX_RX_dBm_averaging = -120.0f;
		TRX_RX_dBm_averaging = 0.97f * TRX_RX_dBm_averaging + 0.03f * TRX_RX1_dBm_lowrate;
		if (TRX_RX1_dBm_lowrate > TRX_RX_dBm_averaging) {
			TRX_RX_dBm_averaging = TRX_RX1_dBm_lowrate;
		}

		if (CurrentVFO->Freq < VHF_S_METER_FREQ_START) {
			if (TRX_RX_dBm_averaging <= -118.0f) {
				sprintf(ctmp, "S1");
			} else if (TRX_RX_dBm_averaging <= -112.0f) {
				sprintf(ctmp, "S2");
			} else if (TRX_RX_dBm_averaging <= -106.0f) {
				sprintf(ctmp, "S3");
			} else if (TRX_RX_dBm_averaging <= -100.0f) {
				sprintf(ctmp, "S4");
			} else if (TRX_RX_dBm_averaging <= -94.0f) {
				sprintf(ctmp, "S5");
			} else if (TRX_RX_dBm_averaging <= -88.0f) {
				sprintf(ctmp, "S6");
			} else if (TRX_RX_dBm_averaging <= -82.0f) {
				sprintf(ctmp, "S7");
			} else if (TRX_RX_dBm_averaging <= -76.0f) {
				sprintf(ctmp, "S8");
			} else if (TRX_RX_dBm_averaging <= -68.0f) {
				sprintf(ctmp, "S9");
			} else if (TRX_RX_dBm_averaging <= -63.0f) {
				sprintf(ctmp, "S9+5");
			} else if (TRX_RX_dBm_averaging <= -58.0f) {
				sprintf(ctmp, "S9+10");
			} else if (TRX_RX_dBm_averaging <= -53.0f) {
				sprintf(ctmp, "S9+15");
			} else if (TRX_RX_dBm_averaging <= -48.0f) {
				sprintf(ctmp, "S9+20");
			} else if (TRX_RX_dBm_averaging <= -43.0f) {
				sprintf(ctmp, "S9+25");
			} else if (TRX_RX_dBm_averaging <= -38.0f) {
				sprintf(ctmp, "S9+30");
			} else if (TRX_RX_dBm_averaging <= -33.0f) {
				sprintf(ctmp, "S9+35");
			} else if (TRX_RX_dBm_averaging <= -28.0f) {
				sprintf(ctmp, "S9+40");
			} else if (TRX_RX_dBm_averaging <= -23.0f) {
				sprintf(ctmp, "S9+45");
			} else if (TRX_RX_dBm_averaging <= -18.0f) {
				sprintf(ctmp, "S9+50");
			} else if (TRX_RX_dBm_averaging <= -13.0f) {
				sprintf(ctmp, "S9+55");
			} else {
				sprintf(ctmp, "S9+60");
			}
		} else {
			if (TRX_RX_dBm_averaging <= -138.0f) {
				sprintf(ctmp, "S1");
			} else if (TRX_RX_dBm_averaging <= -132.0f) {
				sprintf(ctmp, "S2");
			} else if (TRX_RX_dBm_averaging <= -126.0f) {
				sprintf(ctmp, "S3");
			} else if (TRX_RX_dBm_averaging <= -120.0f) {
				sprintf(ctmp, "S4");
			} else if (TRX_RX_dBm_averaging <= -114.0f) {
				sprintf(ctmp, "S5");
			} else if (TRX_RX_dBm_averaging <= -108.0f) {
				sprintf(ctmp, "S6");
			} else if (TRX_RX_dBm_averaging <= -102.0f) {
				sprintf(ctmp, "S7");
			} else if (TRX_RX_dBm_averaging <= -96.0f) {
				sprintf(ctmp, "S8");
			} else if (TRX_RX_dBm_averaging <= -88.0f) {
				sprintf(ctmp, "S9");
			} else if (TRX_RX_dBm_averaging <= -83.0f) {
				sprintf(ctmp, "S9+5");
			} else if (TRX_RX_dBm_averaging <= -78.0f) {
				sprintf(ctmp, "S9+10");
			} else if (TRX_RX_dBm_averaging <= -73.0f) {
				sprintf(ctmp, "S9+15");
			} else if (TRX_RX_dBm_averaging <= -68.0f) {
				sprintf(ctmp, "S9+20");
			} else if (TRX_RX_dBm_averaging <= -63.0f) {
				sprintf(ctmp, "S9+25");
			} else if (TRX_RX_dBm_averaging <= -58.0f) {
				sprintf(ctmp, "S9+30");
			} else if (TRX_RX_dBm_averaging <= -53.0f) {
				sprintf(ctmp, "S9+35");
			} else if (TRX_RX_dBm_averaging <= -48.0f) {
				sprintf(ctmp, "S9+40");
			} else if (TRX_RX_dBm_averaging <= -43.0f) {
				sprintf(ctmp, "S9+45");
			} else if (TRX_RX_dBm_averaging <= -38.0f) {
				sprintf(ctmp, "S9+50");
			} else if (TRX_RX_dBm_averaging <= -33.0f) {
				sprintf(ctmp, "S9+55");
			} else {
				sprintf(ctmp, "S9+60");
			}
		}

		addSymbols(ctmp, ctmp, 6, " ", true);
		LCDDriver_printTextFont(ctmp, LAYOUT->STATUS_LABEL_S_VAL_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABEL_S_VAL_Y_OFFSET, COLOR->STATUS_LABEL_S_VAL, BG_COLOR,
		                        LAYOUT->STATUS_LABEL_S_VAL_FONT);

		// Tuner status
		if (CN_Theme) {
			bool tun_off = !TRX.TUNER_Enabled && SYSMENU_HANDL_CHECK_HAS_ATU();
			if (tun_off) {
				strcpy(ctmp, "   TUN OFF    ");
			} else {
				strcpy(ctmp, "              ");
			}
			if (CurrentVFO->Mode == TRX_MODE_CW) {
				if (tun_off) {
					sprintf(ctmp, "WPM:%2d TUN OFF", TRX.CW_KEYER_WPM);
				} else {
					sprintf(ctmp, "   WPM: %2d    ", TRX.CW_KEYER_WPM);
				}
			}
			LCDDriver_printText(ctmp, LAYOUT->STATUS_ATU_I_X - RASTR_FONT_W * 3, LAYOUT->STATUS_ATU_I_Y, FG_COLOR, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
		}
	} else {
		// ATU
		if (SYSMENU_HANDL_CHECK_HAS_ATU() && LAYOUT->STATUS_ATU_I_Y > 0) {
			if (TRX.TUNER_Enabled) {
				const float32_t *atu_i = ATU_I_VALS;
				const float32_t *atu_c = ATU_C_VALS;

				float32_t i_val = 0;
				float32_t c_val = 0;

				for (uint8_t i = 0; i < ATU_MAXPOS; i++) {
					if (bitRead(TRX.ATU_I, i)) {
						i_val += atu_i[i + 1];
					}
					if (bitRead(TRX.ATU_C, i)) {
						c_val += atu_c[i + 1];
					}
				}

				sprintf(ctmp, "I=%.2fuH", (double)i_val);
				addSymbols(ctmp, ctmp, 8, " ", true);
				LCDDriver_printText(ctmp, LAYOUT->STATUS_ATU_I_X, LAYOUT->STATUS_ATU_I_Y, FG_COLOR, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);

				if (TRX.ATU_T) {
					sprintf(ctmp, "CT=%dpF", (uint32_t)c_val);
				} else {
					sprintf(ctmp, "C=%dpF", (uint32_t)c_val);
				}
				addSymbols(ctmp, ctmp, 8, " ", true);
				LCDDriver_printText(ctmp, LAYOUT->STATUS_ATU_C_X, LAYOUT->STATUS_ATU_C_Y, FG_COLOR, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
			} else { // TUNER OFF
				sprintf(ctmp, "TUN OFF ");
				LCDDriver_printText(ctmp, LAYOUT->STATUS_ATU_I_X, LAYOUT->STATUS_ATU_I_Y, FG_COLOR, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
			}
		}

		if (!LAYOUT->STATUS_SMETER_ANALOG) {
			// SWR
			LCDDriver_Fill_RectWH(LAYOUT->STATUS_TX_LABELS_SWR_X, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, LAYOUT->STATUS_TX_LABELS_VAL_WIDTH,
			                      LAYOUT->STATUS_TX_LABELS_VAL_HEIGHT, BG_COLOR);
			sprintf(ctmp, "%.1f", (double)TRX_SWR_SMOOTHED);
			LCDDriver_printText(ctmp, LAYOUT->STATUS_TX_LABELS_SWR_X, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y,
			                    TRX_SWR_PROTECTOR ? COLOR_RED : COLOR_YELLOW, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);

			// FWD
			LCDDriver_Fill_RectWH(LAYOUT->STATUS_TX_LABELS_FWD_X, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, LAYOUT->STATUS_TX_LABELS_VAL_WIDTH,
			                      LAYOUT->STATUS_TX_LABELS_VAL_HEIGHT, BG_COLOR);
			if (TRX_PWR_Forward_SMOOTHED >= 100.0f) {
				sprintf(ctmp, "%dW", (uint16_t)TRX_PWR_Forward_SMOOTHED);
			} else if (TRX_PWR_Forward_SMOOTHED >= 10.0f) {
				sprintf(ctmp, "%dW ", (uint16_t)TRX_PWR_Forward_SMOOTHED);
			} else {
				sprintf(ctmp, "%.1fW", (double)TRX_PWR_Forward_SMOOTHED);
			}
			LCDDriver_printText(ctmp, LAYOUT->STATUS_TX_LABELS_FWD_X, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, COLOR_YELLOW, BG_COLOR,
			                    LAYOUT->STATUS_LABELS_FONT_SIZE);

			// REF
			LCDDriver_Fill_RectWH(LAYOUT->STATUS_TX_LABELS_REF_X, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, LAYOUT->STATUS_TX_LABELS_VAL_WIDTH,
			                      LAYOUT->STATUS_TX_LABELS_VAL_HEIGHT, BG_COLOR);
			if (TRX_PWR_Backward_SMOOTHED >= 100.0f) {
				sprintf(ctmp, "%dW", (uint16_t)TRX_PWR_Backward_SMOOTHED);
			} else if (TRX_PWR_Backward_SMOOTHED >= 10.0f) {
				sprintf(ctmp, "%dW ", (uint16_t)TRX_PWR_Backward_SMOOTHED);
			} else {
				sprintf(ctmp, "%.1fW", (double)TRX_PWR_Backward_SMOOTHED);
			}
			LCDDriver_printText(ctmp, LAYOUT->STATUS_TX_LABELS_REF_X, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, COLOR_YELLOW, BG_COLOR,
			                    LAYOUT->STATUS_LABELS_FONT_SIZE);

			// SWR Meter
			float32_t fwd_power = TRX_PWR_Forward_SMOOTHED;
			if (fwd_power > CALIBRATE.MAX_RF_POWER_ON_METER) {
				fwd_power = CALIBRATE.MAX_RF_POWER_ON_METER;
			}
			uint16_t ref_width = (uint16_t)(TRX_PWR_Backward_SMOOTHED * (LAYOUT->STATUS_PMETER_WIDTH - 2) / CALIBRATE.MAX_RF_POWER_ON_METER);
			uint16_t fwd_width = (uint16_t)(fwd_power * (LAYOUT->STATUS_PMETER_WIDTH - 2) / CALIBRATE.MAX_RF_POWER_ON_METER);
			uint16_t est_width = (uint16_t)((CALIBRATE.MAX_RF_POWER_ON_METER - fwd_power) * (LAYOUT->STATUS_PMETER_WIDTH - 2) / CALIBRATE.MAX_RF_POWER_ON_METER);
			if (ref_width > fwd_width) {
				ref_width = fwd_width;
			}
			fwd_width -= ref_width;
			LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + 1, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 2, fwd_width,
			                      LAYOUT->STATUS_BAR_HEIGHT - 3, COLOR->STATUS_SMETER);
			LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + 1 + fwd_width, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 2, ref_width,
			                      LAYOUT->STATUS_BAR_HEIGHT - 3, COLOR->STATUS_BAR_RIGHT);
			LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + 1 + fwd_width + ref_width, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 2, est_width,
			                      LAYOUT->STATUS_BAR_HEIGHT - 3, BG_COLOR);

			// ALC
			LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_PMETER_WIDTH + LAYOUT->STATUS_TX_ALC_X_OFFSET,
			                      LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, LAYOUT->STATUS_TX_LABELS_VAL_WIDTH,
			                      LAYOUT->STATUS_TX_LABELS_VAL_HEIGHT, BG_COLOR);
			uint8_t alc_level = (uint8_t)(TRX_ALC_OUT * 100.0f);
			sprintf(ctmp, "%d%%", alc_level);
			LCDDriver_printText(ctmp, LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_PMETER_WIDTH + LAYOUT->STATUS_TX_ALC_X_OFFSET,
			                    LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, COLOR->STATUS_BAR_LABELS, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
			uint16_t alc_level_width = LAYOUT->STATUS_AMETER_WIDTH * alc_level / 100;
			if (alc_level_width > LAYOUT->STATUS_AMETER_WIDTH) {
				alc_level_width = LAYOUT->STATUS_AMETER_WIDTH;
			}
			LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_PMETER_WIDTH + LAYOUT->STATUS_ALC_BAR_X_OFFSET,
			                      LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 2, alc_level_width, LAYOUT->STATUS_BAR_HEIGHT - 3,
			                      COLOR->STATUS_SMETER);
			if (alc_level < 100) {
				LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_PMETER_WIDTH + LAYOUT->STATUS_ALC_BAR_X_OFFSET + alc_level_width,
				                      LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 2, LAYOUT->STATUS_AMETER_WIDTH - alc_level_width,
				                      LAYOUT->STATUS_BAR_HEIGHT - 3, COLOR->STATUS_LABEL_NOTCH);
			}
		} else // analog meter version
		{
			// SWR
			sprintf(ctmp, "%.2f ", (double)TRX_SWR_SMOOTHED);
			/**********************************************/
			if (CN_Theme) {
				LCDDriver_printText(ctmp, LAYOUT->STATUS_LABEL_DBM_X_OFFSET + 5, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABEL_DBM_Y_OFFSET + 3, COLOR->STATUS_LABEL_DBM, BG_COLOR,
				                    LAYOUT->STATUS_LABELS_FONT_SIZE);
			} else {
				LCDDriver_printText(ctmp, LAYOUT->STATUS_LABEL_DBM_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABEL_DBM_Y_OFFSET - 5, COLOR->STATUS_LABEL_DBM, BG_COLOR,
				                    LAYOUT->STATUS_LABELS_FONT_SIZE);
			}
			/**********************************************/

			// FWD
			if (TRX_PWR_Forward_SMOOTHED >= 99.9f) {
				sprintf(ctmp, "%dW", (uint16_t)TRX_PWR_Forward_SMOOTHED);
			} else if (TRX_PWR_Forward_SMOOTHED >= 9.9f) {
				sprintf(ctmp, "%2.1fW", (double)TRX_PWR_Forward_SMOOTHED);
			} else {
				sprintf(ctmp, "%1.1fW", (double)TRX_PWR_Forward_SMOOTHED);
			}
			addSymbols(ctmp, ctmp, 5, " ", true);
			/**********************************************/
			if (CN_Theme) {
				LCDDriver_printText(ctmp, LAYOUT->STATUS_LABEL_DBM_X_OFFSET + 5, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABEL_DBM_Y_OFFSET + 13, COLOR->STATUS_LABEL_DBM, BG_COLOR,
				                    LAYOUT->STATUS_LABELS_FONT_SIZE);
			} else {
				LCDDriver_printText(ctmp, LAYOUT->STATUS_LABEL_DBM_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABEL_DBM_Y_OFFSET + 5, COLOR->STATUS_LABEL_DBM, BG_COLOR,
				                    LAYOUT->STATUS_LABELS_FONT_SIZE);
			}
			/**********************************************/

			LCD_drawSMeter();
			if (TRX.AnalogMeterShowPWR) {
				LCD_PrintMeterArrow(LCD_GetAnalowPowerValPosition(TRX_PWR_Forward_SMOOTHED), COLOR->STATUS_SMETER_STRIPE);
			} else {
				LCD_PrintMeterArrow(LCD_GetAnalowPowerValPosition(TRX_PWR_Forward_SMOOTHED), COLOR->STATUS_SMETER_STRIPE);
				LCD_PrintMeterArrow(LCD_GetSMeterValPosition(LCD_SWR2DBM_meter(TRX_SWR_SMOOTHED), false), COLOR->STATUS_SMETER_PEAK);
			}
		}
	}

	// Info labels
	char buff[32] = "";

	// ENC2 Mode
	switch (TRX.ENC2_func_mode) {
	case ENC_FUNC_PAGER:
		strcpy(buff, "PAGE");
		break;
	case ENC_FUNC_FAST_STEP:
		strcpy(buff, "STEP");
		break;
	case ENC_FUNC_SET_WPM:
		strcpy(buff, " WPM ");
		break;
	case ENC_FUNC_SET_RIT:
		strcpy(buff, " XIT ");
		if (TRX.RIT_Enabled) {
			strcpy(buff, " RIT ");
		}
		break;
	case ENC_FUNC_SET_NOTCH:
		strcpy(buff, "NTCH");
		break;
	case ENC_FUNC_SET_LPF:
		strcpy(buff, " LPF ");
		break;
	case ENC_FUNC_SET_HPF:
		strcpy(buff, " HPF ");
		break;
	case ENC_FUNC_SET_SQL:
		strcpy(buff, " SQL ");
		break;
	case ENC_FUNC_SET_VOLUME:
		strcpy(buff, " VOL ");
		break;
	case ENC_FUNC_SET_MEM:
		strcpy(buff, " MEM ");
		break;
	case ENC_FUNC_SET_IF:
		break;
	}

	uint16_t enc_mode_width = RASTR_FONT_W * strlen(buff);
	LCDDriver_printText(buff, LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2 - enc_mode_width / 2, LAYOUT->BW_TRAPEZ_POS_Y - 10, FG_COLOR, BG_COLOR, 1);

	// BW HPF-LPF
	if (CurrentVFO->Mode == TRX_MODE_WFM) {
		sprintf(buff, "BW:FULL");
	} else if ((TRX_on_TX ? CurrentVFO->HPF_TX_Filter_Width : CurrentVFO->HPF_RX_Filter_Width) > 0) {
		sprintf(buff, "BW:%d-%d", TRX_on_TX ? CurrentVFO->HPF_TX_Filter_Width : CurrentVFO->HPF_RX_Filter_Width, TRX_on_TX ? CurrentVFO->LPF_TX_Filter_Width : CurrentVFO->LPF_RX_Filter_Width);
	} else {
		sprintf(buff, "BW:%d", TRX_on_TX ? CurrentVFO->LPF_TX_Filter_Width : CurrentVFO->LPF_RX_Filter_Width);
	}
	addSymbols(buff, buff, 12, " ", true);
	LCDDriver_printText(buff, LAYOUT->STATUS_LABEL_BW_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABEL_BW_Y_OFFSET, COLOR->STATUS_LABEL_BW, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);

	if (CALIBRATE.INA226_EN && !TRX.SPLIT_Enabled && !TRX.RIT_Enabled && !TRX.XIT_Enabled) ////INA226 current voltage indication  Is the INA226 used (installed)
	{
		float32_t current = Get_INA226_Current();
		if (current < 1.0f) {
			sprintf(buff, "%2.1fV/%umA", (double)Get_INA226_Voltage(), (uint32_t)(current * 1000.0f));
		} else {
			sprintf(buff, "%2.1fV/%2.1fA ", (double)Get_INA226_Voltage(), (double)current);
		}
	} else {
		// RIT
		if (TRX.SPLIT_Enabled) {
			sprintf(buff, "SPLIT");
		} else if ((!TRX.XIT_Enabled || !TRX_on_TX) && TRX.RIT_Enabled && TRX_RIT > 0) {
			sprintf(buff, "RIT:+%d", TRX_RIT);
		} else if ((!TRX.XIT_Enabled || !TRX_on_TX) && TRX.RIT_Enabled) {
			sprintf(buff, "RIT:%d", TRX_RIT);
		} else if ((!TRX.RIT_Enabled || TRX_on_TX) && TRX.XIT_Enabled && TRX_XIT > 0) {
			sprintf(buff, "XIT:+%d", TRX_XIT);
		} else if ((!TRX.RIT_Enabled || TRX_on_TX) && TRX.XIT_Enabled) {
			sprintf(buff, "XIT:%d", TRX_XIT);
		} else {
			sprintf(buff, "RIT:OFF");
		}
	}
	addSymbols(buff, buff, 12, " ", true);
	LCDDriver_printText(buff, LAYOUT->STATUS_LABEL_RIT_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABEL_RIT_Y_OFFSET, COLOR->STATUS_LABEL_RIT, BG_COLOR,
	                    LAYOUT->STATUS_LABELS_FONT_SIZE);

	if ((TRX.LayoutThemeId == 4) || (TRX.LayoutThemeId == 5) || (TRX.LayoutThemeId == 6) || (TRX.LayoutThemeId == 7)) // layout theme Default+ or Analog+ or CN or CN+
	{
		uint16_t if_width = (uint16_t)((float32_t)TRX.IF_Gain / ((float32_t)CALIBRATE.IF_GAIN_MAX / (float32_t)LAYOUT->STATUS_IFGAIN_BAR_WIDTH));
		uint16_t af_width = (uint16_t)((float32_t)TRX.Volume / ((float32_t)MAX_VOLUME_VALUE / (float32_t)LAYOUT->STATUS_AFGAIN_BAR_WIDTH));

		// IF Gain
		sprintf(buff, "IF");
		LCDDriver_printText(buff, LAYOUT->STATUS_IFGAIN_BAR_OFFSET_X, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_IFGAIN_BAR_OFFSET_Y, COLOR->STATUS_LABEL_RIT, BG_COLOR,
		                    LAYOUT->STATUS_LABELS_FONT_SIZE);
		LCDDriver_Fill_RectWH(LAYOUT->STATUS_IFGAIN_BAR_OFFSET_X + 15 + if_width, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_IFGAIN_BAR_OFFSET_Y, LAYOUT->STATUS_IFGAIN_BAR_WIDTH - if_width,
		                      LAYOUT->STATUS_IFGAIN_BAR_HEIGTH, COLOR->IFGAIN_BAR_BG);
		LCDDriver_Fill_RectWH(LAYOUT->STATUS_IFGAIN_BAR_OFFSET_X + 15, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_IFGAIN_BAR_OFFSET_Y, if_width, LAYOUT->STATUS_IFGAIN_BAR_HEIGTH,
		                      COLOR->IFGAIN_BAR_FG);

		// AF Gain
		sprintf(buff, "AF");
		LCDDriver_printText(buff, LAYOUT->STATUS_AFGAIN_BAR_OFFSET_X, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_AFGAIN_BAR_OFFSET_Y, COLOR->STATUS_LABEL_RIT, BG_COLOR,
		                    LAYOUT->STATUS_LABELS_FONT_SIZE);
		LCDDriver_Fill_RectWH(LAYOUT->STATUS_AFGAIN_BAR_OFFSET_X + 15 + af_width, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_AFGAIN_BAR_OFFSET_Y, LAYOUT->STATUS_AFGAIN_BAR_WIDTH - af_width,
		                      LAYOUT->STATUS_AFGAIN_BAR_HEIGTH, COLOR->AFGAIN_BAR_BG);
		LCDDriver_Fill_RectWH(LAYOUT->STATUS_AFGAIN_BAR_OFFSET_X + 15, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_AFGAIN_BAR_OFFSET_Y, af_width, LAYOUT->STATUS_AFGAIN_BAR_HEIGTH,
		                      COLOR->AFGAIN_BAR_FG);
	}

	// THERMAL
#if (defined(LAY_800x480))
	sprintf(buff, "PA:%02d^oC MB:%02d^oC", (int16_t)TRX_RF_Temperature, (int16_t)TRX_STM32_TEMPERATURE);
#else
	sprintf(buff, "PA:%02d^o MB:%02d^o", (int16_t)TRX_RF_Temperature, (int16_t)TRX_STM32_TEMPERATURE);
#endif
	addSymbols(buff, buff, 12, " ", true);
	LCDDriver_printText(buff, LAYOUT->STATUS_LABEL_THERM_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABEL_THERM_Y_OFFSET, COLOR->STATUS_LABEL_THERM, BG_COLOR,
	                    LAYOUT->STATUS_LABELS_FONT_SIZE);
	// NOTCH
	if (CurrentVFO->AutoNotchFilter) {
		sprintf(buff, "NOTCH:AUTO");
	} else if (CurrentVFO->ManualNotchFilter) {
		sprintf(buff, "NOTCH:%uHz", CurrentVFO->NotchFC);
	} else {
		sprintf(buff, "NOTCH:OFF");
	}
	addSymbols(buff, buff, 12, " ", true);
	LCDDriver_printText(buff, LAYOUT->STATUS_LABEL_NOTCH_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABEL_NOTCH_Y_OFFSET, COLOR->STATUS_LABEL_NOTCH, BG_COLOR,
	                    LAYOUT->STATUS_LABELS_FONT_SIZE);
	// FFT BW
	uint8_t fft_zoom = TRX.FFT_Zoom;
	if (CurrentVFO->Mode == TRX_MODE_CW) {
		fft_zoom = TRX.FFT_ZoomCW;
	}

	sprintf(buff, "%dkHz x%d", FFT_current_spectrum_width_Hz / 1000, fft_zoom);
	addSymbols(buff, buff, 10, " ", true);
	LCDDriver_printText(buff, LAYOUT->STATUS_LABEL_FFT_BW_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABEL_FFT_BW_Y_OFFSET, COLOR->STATUS_LABELS_BW, BG_COLOR,
	                    LAYOUT->STATUS_LABELS_FONT_SIZE);

#if (defined(LAY_800x480))
	// PWR + CPU/SQL
	if (TRX.RF_Gain < 100) {
		sprintf(buff, "PWR:%d%% CPU:%d%% ", (uint32_t)TRX.RF_Gain, (uint32_t)CPU_LOAD.Load);
		if (CurrentVFO->SQL && (CurrentVFO->Mode == TRX_MODE_NFM || CurrentVFO->Mode == TRX_MODE_WFM)) {
			sprintf(buff, "PWR:%d%% SQL:%d ", (uint32_t)TRX.RF_Gain, CurrentVFO->FM_SQL_threshold_dBm);
		}
	} else {
		sprintf(buff, "PWR:%d CPU:%d%% ", (uint32_t)TRX.RF_Gain, (uint32_t)CPU_LOAD.Load);
		if (CurrentVFO->SQL && (CurrentVFO->Mode == TRX_MODE_NFM || CurrentVFO->Mode == TRX_MODE_WFM)) {
			sprintf(buff, "PWR:%d SQL:%d ", (uint32_t)TRX.RF_Gain, CurrentVFO->FM_SQL_threshold_dBm);
		}
	}

	LCDDriver_printText(buff, LAYOUT->STATUS_LABEL_CPU_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABEL_CPU_Y_OFFSET, COLOR->STATUS_LABEL_THERM, BG_COLOR,
	                    LAYOUT->STATUS_LABELS_FONT_SIZE);
	// AUTOGAIN
	LCDDriver_printText("AUTOGAIN", LAYOUT->STATUS_LABEL_AUTOGAIN_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABEL_AUTOGAIN_Y_OFFSET,
	                    TRX.AutoGain ? COLOR->STATUS_LABEL_ACTIVE : COLOR->STATUS_LABEL_INACTIVE, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	// LOCK
	LCDDriver_printText("LOCK", LAYOUT->STATUS_LABEL_LOCK_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABEL_LOCK_Y_OFFSET, TRX.Locked ? COLOR_RED : COLOR->STATUS_LABEL_INACTIVE,
	                    BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
#endif

	// SD REC
	LCDDriver_printText(SD_RecordInProcess ? "REC" : "   ", LAYOUT->STATUS_LABEL_REC_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABEL_REC_Y_OFFSET, COLOR_RED, BG_COLOR,
	                    LAYOUT->STATUS_LABELS_FONT_SIZE);

	// ERRORS LABELS
	bool has_error = false;
#if !defined(LAY_800x480)
	LCDDriver_Fill_RectWH(LAYOUT->STATUS_ERR_OFFSET_X, LAYOUT->STATUS_ERR_OFFSET_Y, LAYOUT->STATUS_ERR_WIDTH, LAYOUT->STATUS_ERR_HEIGHT, BG_COLOR);
#endif
	if (TRX_ADC_OTR && !TRX_on_TX && !TRX.ADC_SHDN) {
		LCDDriver_printText("OVR", LAYOUT->STATUS_ERR_OFFSET_X, LAYOUT->STATUS_ERR_OFFSET_Y, COLOR->STATUS_ERR, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
		has_error = true;
	} else if (TRX_ADC_MAXAMPLITUDE > (ADC_FULL_SCALE * 0.499f) || TRX_ADC_MINAMPLITUDE < -(ADC_FULL_SCALE * 0.499f)) {
		if (ADCDAC_OVR_StatusLatency >= 10) {
			TRX_ADC_OTR = true;
		}
	}
	// if(TRX_MIC_BELOW_NOISEGATE)
	// LCDDriver_printText("GAT", LAYOUT->STATUS_ERR_OFFSET_X, LAYOUT->STATUS_ERR_OFFSET_Y, COLOR->STATUS_ERR, BG_COLOR,
	// LAYOUT->STATUS_LABELS_FONT_SIZE);
	if (APROC_IFGain_Overflow) {
		LCDDriver_printText("IFO", LAYOUT->STATUS_ERR_OFFSET_X, LAYOUT->STATUS_ERR_OFFSET_Y, COLOR->STATUS_ERR, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
		has_error = true;
	}
	if (TRX_PWR_ALC_SWR_OVERFLOW) {
		LCDDriver_printText("OVS", LAYOUT->STATUS_ERR_OFFSET_X, LAYOUT->STATUS_ERR_OFFSET_Y, COLOR->STATUS_ERR, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
		has_error = true;
	}
	if (TRX_DAC_OTR) {
		LCDDriver_printText("OVR", LAYOUT->STATUS_ERR_OFFSET_X, LAYOUT->STATUS_ERR_OFFSET_Y, COLOR->STATUS_ERR, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
		has_error = true;
	}
	if (CODEC_Buffer_underrun && !TRX_on_TX) {
		LCDDriver_printText("WBF", LAYOUT->STATUS_ERR_OFFSET_X, LAYOUT->STATUS_ERR_OFFSET_Y, COLOR->STATUS_ERR, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
		has_error = true;
	}
	if (FPGA_Buffer_underrun && TRX_on_TX) {
		LCDDriver_printText("FBF", LAYOUT->STATUS_ERR_OFFSET_X, LAYOUT->STATUS_ERR_OFFSET_Y, COLOR->STATUS_ERR, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
		has_error = true;
	}
	if (RX_USB_AUDIO_underrun) {
		LCDDriver_printText("UBF", LAYOUT->STATUS_ERR_OFFSET_X, LAYOUT->STATUS_ERR_OFFSET_Y, COLOR->STATUS_ERR, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
		has_error = true;
	}
	if (SD_underrun) {
		LCDDriver_printText("SDF", LAYOUT->STATUS_ERR_OFFSET_X, LAYOUT->STATUS_ERR_OFFSET_Y, COLOR->STATUS_ERR, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
		has_error = true;
	}

	RTC_TimeTypeDef sTime = {0};
	RTC_DateTypeDef sDate = {0};
	getLocalDateTime(&sDate, &sTime);
	Day = sDate.Date;
	Month = sDate.Month;
	Hours = sTime.Hours;
	Minutes = sTime.Minutes;
	Seconds = sTime.Seconds;

#if (defined(LAY_800x480))
	if (has_error) {
		Last_showed_Day = 255;
		Last_showed_Month = 255;
		LCDDriver_printText("  ", LAYOUT->STATUS_ERR_OFFSET_X + LAYOUT->STATUS_LABELS_FONT_SIZE * RASTR_FONT_W * 3, LAYOUT->STATUS_ERR_OFFSET_Y, COLOR->CLOCK, BG_COLOR,
		                    LAYOUT->STATUS_LABELS_FONT_SIZE);
	}
	if (!has_error && (redraw || Day != Last_showed_Day || Month != Last_showed_Month)) {
		sprintf(ctmp, "%02d.%02d", Day, Month);
		addSymbols(ctmp, ctmp, 2, "0", false);
		LCDDriver_printText(ctmp, LAYOUT->STATUS_ERR_OFFSET_X, LAYOUT->STATUS_ERR_OFFSET_Y, COLOR->CLOCK, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
		Last_showed_Day = Day;
		Last_showed_Month = Month;
	}
#endif
	if (redraw || Hours != Last_showed_Hours) {
		sprintf(ctmp, "%d", Hours);
		addSymbols(ctmp, ctmp, 2, "0", false);
		LCDDriver_printTextFont(ctmp, LAYOUT->CLOCK_POS_HRS_X, LAYOUT->CLOCK_POS_Y, COLOR->CLOCK, BG_COLOR, LAYOUT->CLOCK_FONT);
		LCDDriver_printTextFont(":", LCDDriver_GetCurrentXOffset(), LAYOUT->CLOCK_POS_Y, COLOR->CLOCK, BG_COLOR, LAYOUT->CLOCK_FONT);
		Last_showed_Hours = Hours;
	}
	if (redraw || Minutes != Last_showed_Minutes) {
		sprintf(ctmp, "%d", Minutes);
		addSymbols(ctmp, ctmp, 2, "0", false);
		LCDDriver_printTextFont(ctmp, LAYOUT->CLOCK_POS_MIN_X, LAYOUT->CLOCK_POS_Y, COLOR->CLOCK, BG_COLOR, LAYOUT->CLOCK_FONT);
		LCDDriver_printTextFont(":", LCDDriver_GetCurrentXOffset(), LAYOUT->CLOCK_POS_Y, COLOR->CLOCK, BG_COLOR, LAYOUT->CLOCK_FONT);
		Last_showed_Minutes = Minutes;
	}
	if (redraw || Seconds != Last_showed_Seconds) {
		sprintf(ctmp, "%d", Seconds);
		addSymbols(ctmp, ctmp, 2, "0", false);
		LCDDriver_printTextFont(ctmp, LAYOUT->CLOCK_POS_SEC_X, LAYOUT->CLOCK_POS_Y, COLOR->CLOCK, BG_COLOR, LAYOUT->CLOCK_FONT);
		Last_showed_Seconds = Seconds;
	}

	old_TRX_on_TX = TRX_on_TX;

	LCD_UpdateQuery.StatusInfoBar = false;
	LCD_UpdateQuery.StatusInfoBarRedraw = false;
	LCD_busy = false;
}

static void LCD_displayTextBar(void) {
	// display the text under the waterfall
	if (LCD_systemMenuOpened || LCD_window.opened) {
		return;
	}
	if (LCD_busy) {
		LCD_UpdateQuery.TextBar = true;
		return;
	}
	LCD_busy = true;

	if (TRX.CW_Decoder && (CurrentVFO->Mode == TRX_MODE_CW || CurrentVFO->Mode == TRX_MODE_LOOPBACK)) {
		char ctmp[70];
		sprintf(ctmp, "WPM:%d %s", (uint8_t)roundf(CW_Decoder_WPM), CW_Decoder_Text);
		LCDDriver_printText(ctmp, 2, (LAYOUT->FFT_FFTWTF_BOTTOM - LAYOUT->FFT_CWDECODER_OFFSET + 1), COLOR->CLOCK, BG_COLOR, LAYOUT->TEXTBAR_FONT);
	} else if (NeedProcessDecoder && CurrentVFO->Mode == TRX_MODE_WFM) {
		LCDDriver_printText(RDS_Decoder_Text, 2, (LAYOUT->FFT_FFTWTF_BOTTOM - LAYOUT->FFT_CWDECODER_OFFSET + 1), COLOR->CLOCK, BG_COLOR, LAYOUT->TEXTBAR_FONT);
	} else if (NeedProcessDecoder && CurrentVFO->Mode == TRX_MODE_RTTY) {
		LCDDriver_printText(RTTY_Decoder_Text, 2, (LAYOUT->FFT_FFTWTF_BOTTOM - LAYOUT->FFT_CWDECODER_OFFSET + 1), COLOR->CLOCK, BG_COLOR, LAYOUT->TEXTBAR_FONT);
	}

	LCD_UpdateQuery.TextBar = false;
	LCD_busy = false;
}

void LCD_cleanTouchpadButtons(void) { TouchpadButton_handlers_count = 0; }

void LCD_redraw(bool do_now) {
	TouchpadButton_handlers_count = 0;
	LCD_UpdateQuery.Background = true;
	LCD_UpdateQuery.FreqInfoRedraw = true;
	LCD_UpdateQuery.StatusInfoBarRedraw = true;
	LCD_UpdateQuery.StatusInfoGUIRedraw = true;
	LCD_UpdateQuery.TopButtonsRedraw = true;
	LCD_UpdateQuery.BottomButtonsRedraw = true;
	LCD_UpdateQuery.SystemMenuRedraw = true;
	LCD_UpdateQuery.TextBar = true;
	LCD_last_s_meter = 0;
	LCD_last_showed_freq = 0;
	Last_showed_Hours = 255;
	Last_showed_Minutes = 255;
	Last_showed_Seconds = 255;
	LCD_last_showed_freq_MHz = 9999;
	LCD_last_showed_freq_kHz = 9999;
	LCD_last_showed_freq_Hz = 9999;
	NeedWTFRedraw = true;
	if (do_now) {
		LCD_doEvents();
	}
}

bool LCD_doEvents(void) {
	if (LCD_UpdateQuery.SystemMenuRedraw && LCD_showInfo_opened) {
		LCD_busy = false;
	}
	if (LCD_busy) {
		return false;
	}

	if (LCD_UpdateQuery.Background) {
		LCD_busy = true;
		LCDDriver_Fill(BG_COLOR);
		LCD_UpdateQuery.Background = false;
		LCD_busy = false;
	}
	if (LCD_UpdateQuery.BottomButtonsRedraw) {
		LCD_displayBottomButtons(true);
	}
	if (LCD_UpdateQuery.BottomButtons) {
		LCD_displayBottomButtons(false);
	}
	if (LCD_UpdateQuery.TopButtonsRedraw) {
		LCD_displayTopButtons(true);
	}
	if (LCD_UpdateQuery.TopButtons) {
		LCD_displayTopButtons(false);
	}
	if (LCD_UpdateQuery.FreqInfoRedraw) {
		LCD_displayFreqInfo(true);
	}
	if (LCD_UpdateQuery.FreqInfo) {
		LCD_displayFreqInfo(false);
	}
	if (LCD_UpdateQuery.StatusInfoGUIRedraw) {
		LCD_displayStatusInfoGUI(true);
	}
	if (LCD_UpdateQuery.StatusInfoGUI) {
		LCD_displayStatusInfoGUI(false);
	}
	if (LCD_UpdateQuery.StatusInfoBarRedraw) {
		LCD_displayStatusInfoBar(true);
	}
	if (LCD_UpdateQuery.StatusInfoBar) {
		LCD_displayStatusInfoBar(false);
	}
	if (LCD_UpdateQuery.SystemMenuRedraw) {
		if (LCD_systemMenuOpened) {
			TouchpadButton_handlers_count = 0;
		}
		SYSMENU_drawSystemMenu(true, false);
	}
	if (LCD_UpdateQuery.SystemMenu) {
		SYSMENU_drawSystemMenu(false, false);
	}
	if (LCD_UpdateQuery.SystemMenuInfolines) {
		SYSMENU_drawSystemMenu(false, true);
	}
	if (LCD_UpdateQuery.SystemMenuCurrent) {
		SYSMENU_redrawCurrentItem(false);
		LCD_UpdateQuery.SystemMenuCurrent = false;
	}
	if (LCD_UpdateQuery.TextBar) {
		LCD_displayTextBar();
	}
	if (LCD_UpdateQuery.Tooltip) {
		LCD_printTooltip();
	}
	return true;
}

static void printInfoSmall(uint16_t x, uint16_t y, uint16_t width, uint16_t height, char *text, uint16_t back_color, uint16_t text_color, uint16_t inactive_color, bool active) {
	uint16_t x1, y1, w, h;
	LCDDriver_Fill_RectWH(x, y, width, height, back_color);
	LCDDriver_getTextBoundsFont(text, x, y, &x1, &y1, &w, &h, (GFXfont *)&FreeSans7pt7b);
	// sendToDebug_str(text); sendToDebug_str(" "); sendToDebug_uint16(w, false);
	LCDDriver_printTextFont(text, x + (width - w) / 2, y + (height / 2) + h / 2 - 1, active ? text_color : inactive_color, back_color, (GFXfont *)&FreeSans7pt7b);
}

static void printInfo(uint16_t x, uint16_t y, uint16_t width, uint16_t height, char *text, uint16_t back_color, uint16_t text_color, uint16_t inactive_color, bool active) {
	uint16_t x1, y1, w, h;
	LCDDriver_Fill_RectWH(x, y, width, height, back_color);
	LCDDriver_getTextBoundsFont(text, x, y, &x1, &y1, &w, &h, (GFXfont *)&FreeSans9pt7b);
	// sendToDebug_str(text); sendToDebug_str(" "); sendToDebug_uint16(w, false);
	LCDDriver_printTextFont(text, x + (width - w) / 2, y + (height / 2) + h / 2 - 1, active ? text_color : inactive_color, back_color, (GFXfont *)&FreeSans9pt7b);
}

#if (defined(LAY_800x480))
static void printButton(uint16_t x, uint16_t y, uint16_t width, uint16_t height, char *text, bool active, bool show_lighter, bool in_window, uint32_t parameter,
                        void (*clickHandler)(uint32_t parameter), void (*holdHandler)(uint32_t parameter), uint16_t active_color, uint16_t inactive_color, uint16_t border_color,
                        uint16_t background_color, uint8_t round, const GFXfont *font) {
	uint16_t x1_text, y1_text, w_text, h_text;
	if (in_window) {
		x += LCD_window.x;
		y += LCD_window.y;
	}
	uint16_t x_act = x + LAYOUT->BUTTON_PADDING;
	uint16_t y_act = y + LAYOUT->BUTTON_PADDING;
	uint16_t w_act = width - LAYOUT->BUTTON_PADDING * 2;
	uint16_t h_act = height - LAYOUT->BUTTON_PADDING * 2;

	if (round > 0) {
		LCDDriver_drawRoundedRectWH(x_act, y_act, w_act, h_act, border_color, 2, border_color == background_color); // border
		if (border_color != background_color) {
			LCDDriver_Fill_RectWH(x_act + 1, y_act + 1, w_act - 2, h_act - 2, background_color); // button body
		}
	} else {
		LCDDriver_Fill_RectWH(x_act, y_act, w_act, h_act, background_color); // button body
		if (border_color != background_color) {
			LCDDriver_drawRectXY(x_act, y_act, x_act + w_act, y_act + h_act, border_color); // border
		}
	}

	LCDDriver_getTextBoundsFont(text, x_act, y_act, &x1_text, &y1_text, &w_text, &h_text, font); // get text bounds

	if (show_lighter && LAYOUT->BUTTON_LIGHTER_HEIGHT > 0) {
		LCDDriver_printTextFont(text, x_act + (w_act - w_text) / 2, y_act + (h_act * 2 / 5) + h_text / 2 - 1, active ? active_color : inactive_color, background_color, font); // text
		uint16_t lighter_width = (uint16_t)((float32_t)w_act * LAYOUT->BUTTON_LIGHTER_WIDTH);
		LCDDriver_Fill_RectWH(x_act + ((w_act - lighter_width) / 2), y_act + h_act * 3 / 4, lighter_width, LAYOUT->BUTTON_LIGHTER_HEIGHT,
		                      active ? COLOR->BUTTON_LIGHTER_ACTIVE : COLOR->BUTTON_LIGHTER_INACTIVE); // lighter
	} else {
		LCDDriver_printTextFont(text, x_act + (w_act - w_text) / 2, y_act + (h_act / 2) + h_text / 2 - 1, active ? active_color : inactive_color, background_color, font); // text
	}
	// add handler
	if (clickHandler == NULL && holdHandler == NULL) {
		return;
	}

	if (in_window) {
		LCD_window.buttons[LCD_window.buttons_count].x1 = x;
		LCD_window.buttons[LCD_window.buttons_count].y1 = y;
		LCD_window.buttons[LCD_window.buttons_count].x2 = x + width;
		LCD_window.buttons[LCD_window.buttons_count].y2 = y + height;
		LCD_window.buttons[LCD_window.buttons_count].parameter = parameter;
		LCD_window.buttons[LCD_window.buttons_count].clickHandler = clickHandler;
		LCD_window.buttons[LCD_window.buttons_count].holdHandler = holdHandler;
		LCD_window.buttons_count++;
	} else {
		bool exist = false;
		for (uint8_t i = 0; i < TouchpadButton_handlers_count; i++) {
			if (TouchpadButton_handlers[i].x1 == x && TouchpadButton_handlers[i].y1 == y) {
				exist = true;
				break;
			}
		}
		if (!exist) {
			TouchpadButton_handlers[TouchpadButton_handlers_count].x1 = x;
			TouchpadButton_handlers[TouchpadButton_handlers_count].y1 = y;
			TouchpadButton_handlers[TouchpadButton_handlers_count].x2 = x + width;
			TouchpadButton_handlers[TouchpadButton_handlers_count].y2 = y + height;
			TouchpadButton_handlers[TouchpadButton_handlers_count].parameter = parameter;
			TouchpadButton_handlers[TouchpadButton_handlers_count].clickHandler = clickHandler;
			TouchpadButton_handlers[TouchpadButton_handlers_count].holdHandler = holdHandler;
			TouchpadButton_handlers_count++;
		}
	}
}

void printSystemMenuButton(uint16_t x, uint16_t y, uint16_t width, uint16_t height, char *title, char *value, bool selected, bool active, uint32_t parameter,
                           void (*clickHandler)(uint32_t parameter), void (*holdHandler)(uint32_t parameter), uint16_t active_color, uint16_t inactive_color, uint16_t border_color,
                           uint16_t back_color, SystemMenuType type) {
	uint16_t x1_title, y1_title, w_title, h_title;
	uint16_t x1_value, y1_value, w_value, h_value;

	uint16_t title_back_color = mixColors(back_color, selected ? active_color : inactive_color, 0.2f);
	uint16_t value_back_color = mixColors(back_color, selected ? border_color : back_color, 0.3f);
	if (type == SYSMENU_NAVBUTTON) {
		title_back_color = back_color;
	}

	LCDDriver_drawRoundedRectWH(x, y, width, height, active ? active_color : border_color, 2, false);

	if (type == SYSMENU_RUN || type == SYSMENU_MENU || type == SYSMENU_NAVBUTTON) {
		LCDDriver_Fill_RectWH(x + 1, y + 1, width - 2, (height - 2), title_back_color);
		LCDDriver_getTextBoundsFont(title, x, y, &x1_title, &y1_title, &w_title, &h_title, (GFXfont *)&FreeSans9pt7b); // get title bounds
		LCDDriver_getTextBoundsFont(value, x, y, &x1_value, &y1_value, &w_value, &h_value, (GFXfont *)&FreeSans7pt7b); // get value bounds

		LCDDriver_printTextFont(title, x + (width - w_title) / 2, y + h_title + 11, selected ? active_color : inactive_color, title_back_color, &FreeSans9pt7b); // title
		LCDDriver_printTextFont(value, x + (width - w_value) / 2, y + (height / 2) + h_value + 10, inactive_color, title_back_color, &FreeSans7pt7b);            // value
	} else if (type == SYSMENU_INFOLINE) {
		LCDDriver_Fill_RectWH(x + 1, y + 1, width - 2, (height - 2), value_back_color);
		LCDDriver_getTextBoundsFont(title, x, y, &x1_title, &y1_title, &w_title, &h_title, (GFXfont *)&FreeSans7pt7b); // get title bounds
		LCDDriver_printTextFont(title, x + (width - w_title) / 2, y + h_title + 11 + 11, selected ? active_color : inactive_color, value_back_color,
		                        &FreeSans7pt7b); // title
	} else {
		LCDDriver_Fill_RectWH(x + 1, y + 1, width - 2, (height - 2) / 2 + 2, title_back_color); // button body
		LCDDriver_Fill_RectWH(x + 1, y + 4 + (height - 2) / 2, width - 2, (height - 2) / 2 - 3, value_back_color);
		LCDDriver_getTextBoundsFont(title, x, y, &x1_title, &y1_title, &w_title, &h_title, (GFXfont *)&FreeSans9pt7b); // get title bounds
		LCDDriver_getTextBoundsFont(value, x, y, &x1_value, &y1_value, &w_value, &h_value, (GFXfont *)&FreeSans7pt7b); // get value bounds

		LCDDriver_printTextFont(title, x + (width - w_title) / 2, y + h_title + 11, selected ? active_color : inactive_color, title_back_color, &FreeSans9pt7b); // title
		LCDDriver_printTextFont(value, x + (width - w_value) / 2, y + (height / 2) + h_value + 10, inactive_color, value_back_color, &FreeSans7pt7b);            // value
	}

	// add handler
	bool exist = false;
	for (uint8_t i = 0; i < TouchpadButton_handlers_count; i++) {
		if (TouchpadButton_handlers[i].x1 == x && TouchpadButton_handlers[i].y1 == y) {
			exist = true;
			break;
		}
	}
	if (!exist && type != SYSMENU_INFOLINE) {
		TouchpadButton_handlers[TouchpadButton_handlers_count].x1 = x;
		TouchpadButton_handlers[TouchpadButton_handlers_count].y1 = y;
		TouchpadButton_handlers[TouchpadButton_handlers_count].x2 = x + width;
		TouchpadButton_handlers[TouchpadButton_handlers_count].y2 = y + height;
		TouchpadButton_handlers[TouchpadButton_handlers_count].parameter = parameter;
		TouchpadButton_handlers[TouchpadButton_handlers_count].clickHandler = clickHandler;
		TouchpadButton_handlers[TouchpadButton_handlers_count].holdHandler = holdHandler;
		TouchpadButton_handlers_count++;
	}
}
#endif

void LCD_showError(char text[], bool redraw) {
	LCD_busy = true;
	if (!LCD_inited) {
		LCD_Init();
	}

	LCDDriver_Fill(COLOR_RED);
	uint16_t x1, y1, w, h;
	LCDDriver_getTextBoundsFont(text, 0, 0, &x1, &y1, &w, &h, (GFXfont *)&FreeSans12pt7b);
	LCDDriver_printTextFont(text, LCD_WIDTH / 2 - w / 2, LCD_HEIGHT / 2 - h / 2, COLOR_WHITE, COLOR_RED, (GFXfont *)&FreeSans12pt7b);
	if (redraw) {
		HAL_Delay(2000);
	}
	LCD_busy = false;
	if (redraw) {
		LCD_redraw(false);
	}
}

void LCD_showErrorColored(char text[], bool redraw, uint16_t bg_color) {
	LCD_busy = true;
	if (!LCD_inited) {
		LCD_Init();
	}

	LCDDriver_Fill(bg_color);
	uint16_t x1, y1, w, h;
	LCDDriver_getTextBoundsFont(text, 0, 0, &x1, &y1, &w, &h, (GFXfont *)&FreeSans12pt7b);
	LCDDriver_printTextFont(text, LCD_WIDTH / 2 - w / 2, LCD_HEIGHT / 2 - h / 2, COLOR_WHITE, bg_color, (GFXfont *)&FreeSans12pt7b);
	if (redraw) {
		HAL_Delay(2000);
	}
	LCD_busy = false;
	if (redraw) {
		LCD_redraw(false);
	}
}

void LCD_showInfo(char text[], bool autohide) {
	LCD_showInfo_opened = true;
	LCD_busy = true;
	if (!LCD_inited) {
		LCD_Init();
	}
	println((char *)text);
	LCDDriver_Fill(BG_COLOR);
	uint16_t x1, y1, w, h;
	LCDDriver_getTextBoundsFont(text, 0, 0, &x1, &y1, &w, &h, (GFXfont *)&FreeSans12pt7b);
	LCDDriver_printTextFont(text, LCD_WIDTH / 2 - w / 2, LCD_HEIGHT / 2 - h / 2, COLOR->CLOCK, BG_COLOR, (GFXfont *)&FreeSans12pt7b);
	if (autohide) {
		HAL_Delay(2000);
		LCD_showInfo_opened = false;
		LCD_busy = false;
		LCD_redraw(false);
	}
}

void LCD_processTouch(uint16_t x, uint16_t y) {
#if (defined(HAS_TOUCHPAD) && defined(LAY_800x480))
	TRX_Inactive_Time = 0;
	if (TRX.Locked) {
		return;
	}

	if (SYSMENU_FT8_DECODER_opened) { // FT8 buttons
		// CQ
		if (y >= (FT8_button_line) && y <= (FT8_button_line + FT8_button_height) && x >= (FT8_button_spac_x * 0) && x <= ((FT8_button_spac_x * 0) + FT8_button_width)) {
			FT8_Menu_Idx = 0;
			Update_FT8_Menu_Cursor();
			FT8_Enc2Click();
		}

		// TUNE
		if (y >= (FT8_button_line) && y <= (FT8_button_line + FT8_button_height) && x >= (FT8_button_spac_x * 1) && x <= ((FT8_button_spac_x * 1) + FT8_button_width)) {
			FT8_Menu_Idx = 1;
			Update_FT8_Menu_Cursor();
			FT8_Enc2Click();
		}

		// RT_C
		if (y >= (FT8_button_line) && y <= (FT8_button_line + FT8_button_height) && x >= (FT8_button_spac_x * 2) && x <= ((FT8_button_spac_x * 2) + FT8_button_width)) {
			FT8_Menu_Idx = 2;
			Update_FT8_Menu_Cursor();
			FT8_Enc2Click();
		}

		return;
	}

	if (!LCD_screenKeyboardOpened && LCD_systemMenuOpened) {
		// try to find system menu buttons
		for (uint8_t i = 0; i < TouchpadButton_handlers_count; i++) {
			if ((TouchpadButton_handlers[i].x1 <= x) && (TouchpadButton_handlers[i].y1 <= y) && (TouchpadButton_handlers[i].x2 >= x) && (TouchpadButton_handlers[i].y2 >= y)) // touch height
			{
				if (TouchpadButton_handlers[i].clickHandler != NULL) {
					TouchpadButton_handlers[i].clickHandler(TouchpadButton_handlers[i].parameter);
					return;
				}
			}
		}

		// not found, close menu
		SYSMENU_HANDL_Back(0);
		return;
	}

	// windows
	if (LCD_window.opened) {
		// outline touch (window close)
		if (y <= LCD_window.y || y >= (LCD_window.y + LCD_window.h) || x <= LCD_window.x || x >= (LCD_window.x + LCD_window.w)) {
			LCD_closeWindow();
			return;
		}

		// window buttons
		for (uint8_t i = 0; i < LCD_window.buttons_count; i++) {
			if ((LCD_window.buttons[i].x1 <= x) && (LCD_window.buttons[i].y1 <= y) && (LCD_window.buttons[i].x2 >= x) && (LCD_window.buttons[i].y2 >= y)) {
				if (LCD_window.buttons[i].clickHandler != NULL) {
					LCD_window.buttons[i].clickHandler(LCD_window.buttons[i].parameter);
					return;
				}
			}
		}

		return;
	}

	if (!LCD_screenKeyboardOpened) {
		// main mode swich
		if (CN_Theme) {
			if (y >= 0 && y <= 30 && x > LAYOUT->STATUS_MODE_AM_X && x < LAYOUT->STATUS_MODE_SAM_X) {
				BUTTONHANDLER_SETMODE(TRX_MODE_AM); // set mode AM;
				return;
			}
			if (y >= 0 && y <= 30 && x > LAYOUT->STATUS_MODE_SAM_X && x < LAYOUT->STATUS_MODE_CW_X) {
				BUTTONHANDLER_SETMODE(TRX_MODE_SAM_STEREO); // set mode SAM;
				return;
			}
			if (y >= 0 && y <= 30 && x > LAYOUT->STATUS_MODE_CW_X && x < LAYOUT->STATUS_MODE_DIGL_X) {
				BUTTONHANDLER_SETMODE(TRX_MODE_CW); // set mode CW;
				return;
			}
			if (y >= 0 && y <= 30 && x > LAYOUT->STATUS_MODE_DIGL_X && x < LAYOUT->STATUS_MODE_DIGU_X) {
				BUTTONHANDLER_SETMODE(TRX_MODE_DIGI_L); // set mode DIGL;
				return;
			}
			if (y >= 0 && y <= 30 && x > LAYOUT->STATUS_MODE_DIGU_X && x < LAYOUT->STATUS_MODE_NFM_X) {
				BUTTONHANDLER_SETMODE(TRX_MODE_DIGI_U); // set mode DIGU;
				return;
			}
			if (y >= 0 && y <= 30 && x > LAYOUT->STATUS_MODE_NFM_X && x < LAYOUT->STATUS_MODE_WFM_X) {
				BUTTONHANDLER_SETMODE(TRX_MODE_NFM); // set mode NFM;
				return;
			}
			if (y >= 0 && y <= 30 && x > LAYOUT->STATUS_MODE_WFM_X && x < LAYOUT->STATUS_MODE_LSB_X) {
				BUTTONHANDLER_SETMODE(TRX_MODE_WFM); // set mode WFM;
				return;
			}
			if (y >= 0 && y <= 30 && x > LAYOUT->STATUS_MODE_LSB_X && x < LAYOUT->STATUS_MODE_USB_X) {
				BUTTONHANDLER_SETMODE(TRX_MODE_LSB); // set mode LSB;
				return;
			}
			if (y >= 0 && y <= 30 && x > LAYOUT->STATUS_MODE_USB_X && x < LAYOUT->STATUS_MODE_RTTY_X) {
				BUTTONHANDLER_SETMODE(TRX_MODE_USB); // set mode USB;
				return;
			}
			if (y >= 0 && y <= 30 && x > LAYOUT->STATUS_MODE_RTTY_X && x < LAYOUT->STATUS_MODE_SELECT_X) {
				BUTTONHANDLER_SETMODE(TRX_MODE_RTTY); // set mode RTTY;
				return;
			}
			// if (y >= 0 && y <= 30 && x > LAYOUT->STATUS_MODE_LOOP_X && x < LAYOUT->STATUS_MODE_LOOP_X + 34) {
			//	BUTTONHANDLER_SETMODE(TRX_MODE_LOOPBACK); // set mode LOOP;
			//	return;
			// }
			if (y >= 0 && y <= 30 && x > LAYOUT->STATUS_MODE_SELECT_X && x < LAYOUT->STATUS_MODE_SELECT_X + 35) {
				LCD_showModeWindow(false);
				return;
			}

			if (y >= 70 && y <= 90 && x > 265 && x < 350) {
				if (!LCD_systemMenuOpened) {
					LCD_systemMenuOpened = true;
					SYSMEUN_CALLSIGN_INFO_HOTKEY();
				} else {
					SYSMENU_eventCloseAllSystemMenu();
				};
				return;
			}
			// Set time
			if (y >= 0 && y <= 22 && x > LAYOUT->CLOCK_POS_HRS_X && x < LCD_WIDTH) {
				if (!LCD_systemMenuOpened) {
					LCD_systemMenuOpened = true;
					SYSMEUN_TIME_HOTKEY();
				} else {
					SYSMENU_eventCloseAllSystemMenu();
				};
				return;
			}
			// SD Card
			if (y >= 0 && y <= 22 && x > LAYOUT->CLOCK_POS_HRS_X - 44 && x < LAYOUT->CLOCK_POS_HRS_X) {
				if (!LCD_systemMenuOpened) {
					LCD_systemMenuOpened = true;
					SYSMEUN_SD_HOTKEY();
				} else {
					SYSMENU_eventCloseAllSystemMenu();
				};
				return;
			}
		}
		// ant click
		// ANT indicator
		if (y >= (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_ANT_Y_OFFSET - 20) && y <= (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_ANT_Y_OFFSET + LAYOUT->STATUS_ANT_BLOCK_HEIGHT + 20) &&
		    x >= LAYOUT->STATUS_ANT_X_OFFSET && x <= (LAYOUT->STATUS_ANT_X_OFFSET + LAYOUT->STATUS_ANT_BLOCK_WIDTH + 20)) {
			BUTTONHANDLER_ANT(false);
			return;
		}

		// main mode click
		if (!CN_Theme) {
			if (y >= (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_MODE_Y_OFFSET - 20) && y <= (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_MODE_Y_OFFSET + LAYOUT->STATUS_MODE_BLOCK_HEIGHT + 20) &&
			    x >= (LAYOUT->STATUS_MODE_X_OFFSET - 20) && x <= LAYOUT->STATUS_MODE_X_OFFSET + LAYOUT->STATUS_MODE_BLOCK_WIDTH + 20) {
				LCD_showModeWindow(false);
				return;
			}
		}

		// sec mode click
		if (y >= (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_MODE_B_Y_OFFSET - 20) && y <= (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_MODE_B_Y_OFFSET + LAYOUT->STATUS_MODE_BLOCK_HEIGHT + 20) &&
		    x >= (LAYOUT->STATUS_MODE_B_X_OFFSET - 20) && x <= (LAYOUT->STATUS_MODE_B_X_OFFSET + LAYOUT->STATUS_MODE_BLOCK_WIDTH + 20)) {
			LCD_showModeWindow(true);
			return;
		}

		// bw click
		if (y >= LAYOUT->BW_TRAPEZ_POS_Y && y <= LAYOUT->BW_TRAPEZ_POS_Y + LAYOUT->BW_TRAPEZ_HEIGHT && x >= LAYOUT->BW_TRAPEZ_POS_X && x <= LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH) {
			LCD_showBWWindow();
			return;
		}

		// main freq click
		if (y >= LAYOUT->FREQ_Y_TOP && y <= LAYOUT->FREQ_Y_TOP + LAYOUT->FREQ_BLOCK_HEIGHT && x >= LAYOUT->FREQ_LEFT_MARGIN && x <= LAYOUT->FREQ_LEFT_MARGIN + LAYOUT->FREQ_WIDTH) {
			if (!TRX.selected_vfo) { // vfo-a
				LCD_showBandWindow(false);
			} else {
				BUTTONHANDLER_AsB(0);
			}
			return;
		}

		// sec freq click
		if (y >= LAYOUT->FREQ_B_Y_TOP && y <= LAYOUT->FREQ_B_Y_TOP + LAYOUT->FREQ_B_BLOCK_HEIGHT && x >= LAYOUT->FREQ_B_LEFT_MARGIN && x <= LAYOUT->FREQ_B_LEFT_MARGIN + LAYOUT->FREQ_B_WIDTH) {
			if (TRX.selected_vfo) { // vfo-b
				LCD_showBandWindow(true);
			} else {
				BUTTONHANDLER_AsB(0);
			}
			return;
		}

		// analog meter swr/pwr switch
		if (TRX_on_TX && LAYOUT->STATUS_SMETER_ANALOG && y >= (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET) &&
		    y <= (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + image_data_meter.height) && x >= (LAYOUT->STATUS_BAR_X_OFFSET) &&
		    x <= (LAYOUT->STATUS_BAR_X_OFFSET + image_data_meter.width)) {
			TRX.AnalogMeterShowPWR = !TRX.AnalogMeterShowPWR;

			if (TRX.AnalogMeterShowPWR) {
				LCD_showTooltip("POWER");
			} else {
				LCD_showTooltip("SWR");
			}

			return;
		}

		// power inducator
		if (y >= LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABEL_CPU_Y_OFFSET && y <= LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABEL_CPU_Y_OFFSET + 20 && x >= LAYOUT->STATUS_LABEL_CPU_X_OFFSET &&
		    x <= LAYOUT->STATUS_LABEL_CPU_X_OFFSET + 50) {
			BUTTONHANDLER_RF_POWER(0);
			return;
		}
	}

	// buttons main zone
	for (uint8_t i = 0; i < TouchpadButton_handlers_count; i++) {
		if ((TouchpadButton_handlers[i].x1 <= x) && (TouchpadButton_handlers[i].y1 <= y) && (TouchpadButton_handlers[i].x2 >= x) && (TouchpadButton_handlers[i].y2 >= y)) // touch height
		{
			if (TouchpadButton_handlers[i].clickHandler != NULL) {
				TouchpadButton_handlers[i].clickHandler(TouchpadButton_handlers[i].parameter);
				return;
			}
		}
	}

	// bottom buttons extended zone
	if (y >= 430) {
		for (uint8_t i = 0; i < TouchpadButton_handlers_count; i++) {
			if ((TouchpadButton_handlers[i].x1 <= x) && (TouchpadButton_handlers[i].y1 - 30 <= y) && (TouchpadButton_handlers[i].x2 >= x) && (TouchpadButton_handlers[i].y2 >= y)) // touch height
			{
				if (TouchpadButton_handlers[i].clickHandler != NULL) {
					TouchpadButton_handlers[i].clickHandler(TouchpadButton_handlers[i].parameter);
					return;
				}
			}
		}
	}

	if (!LCD_screenKeyboardOpened && !TRX_on_TX) {
		// fft/wtf tap
		if (((LAYOUT->FFT_FFTWTF_POS_Y + 50) <= y) && (LAYOUT->FFT_PRINT_SIZE >= x) && ((LAYOUT->FFT_FFTWTF_POS_Y + LAYOUT->FFT_FFTWTF_BOTTOM - 50) >= y)) {
			TRX_ScanMode = false;
			// frequency tap
			uint64_t newfreq = getFreqOnFFTPosition(x);
			newfreq = newfreq / 500 * 500;
			TRX_setFrequencySlowly(newfreq);
			LCD_UpdateQuery.FreqInfo = true;
			return;
		}
	}
#endif
}

void LCD_processHoldTouch(uint16_t x, uint16_t y) {
#if (defined(HAS_TOUCHPAD) && defined(LAY_800x480))
	TRX_Inactive_Time = 0;
	if (TRX.Locked) {
		return;
	}

	if (LCD_systemMenuOpened) {
		SYSMENU_eventCloseAllSystemMenu();
		LCD_redraw(false);
		return;
	}

	if (CN_Theme) {
		// Input Callsign
		if (y >= 70 && y <= 90 && x > 265 && x < 350) {
			if (!LCD_systemMenuOpened) {
				LCD_systemMenuOpened = true;
				SYSMEUN_CALLSIGN_HOTKEY();
			} else {
				SYSMENU_eventCloseAllSystemMenu();
			};
			return;
		}
		// Hold WIFI
		if (y >= 0 && y <= 22 && x > LAYOUT->CLOCK_POS_HRS_X - 44 && x < LAYOUT->CLOCK_POS_HRS_X) {
			if (!LCD_systemMenuOpened) {
				LCD_systemMenuOpened = true;
				SYSMEUN_WIFI_HOTKEY();
				LCD_redraw(false);
				return;
			}
		}

		// Hold FT8 Mode
		if (y >= 0 && y <= 20 && x > LAYOUT->STATUS_MODE_CW_X && x < LAYOUT->STATUS_MODE_NFM_X) {
			BUTTONHANDLER_FT8(0); // set mode DIGu;
			return;
		}

		// main freq hold
		if (y >= LAYOUT->FREQ_Y_TOP && y <= LAYOUT->FREQ_Y_TOP + LAYOUT->FREQ_BLOCK_HEIGHT && x >= LAYOUT->FREQ_LEFT_MARGIN && x <= LAYOUT->FREQ_LEFT_MARGIN + LAYOUT->FREQ_WIDTH) {
			if (!TRX.selected_vfo) { // vfo-a
				LCD_showManualFreqWindow(0);
				return;
			}
		}

		// sec freq hold
		if (y >= LAYOUT->FREQ_B_Y_TOP && y <= LAYOUT->FREQ_B_Y_TOP + LAYOUT->FREQ_B_BLOCK_HEIGHT && x >= LAYOUT->FREQ_B_LEFT_MARGIN && x <= LAYOUT->FREQ_B_LEFT_MARGIN + LAYOUT->FREQ_B_WIDTH) {
			if (TRX.selected_vfo) { // vfo-b
				LCD_showManualFreqWindow(0);
				return;
			}
		}
	}

	// windows
	// outline touch (window close)
	if (LCD_window.opened && (y <= LCD_window.y || y >= (LCD_window.y + LCD_window.h) || x <= LCD_window.x || x >= (LCD_window.x + LCD_window.w))) {
		LCD_closeWindow();
		return;
	}

	// window buttons
	for (uint8_t i = 0; i < LCD_window.buttons_count; i++) {
		if ((LCD_window.buttons[i].x1 <= x) && (LCD_window.buttons[i].y1 <= y) && (LCD_window.buttons[i].x2 >= x) && (LCD_window.buttons[i].y2 >= y)) {
			if (LCD_window.buttons[i].holdHandler != NULL) {
				LCD_window.buttons[i].holdHandler(LCD_window.buttons[i].parameter);
				return;
			}
		}
	}

	if (LCD_window.opened) {
		return;
	}

	// bw hold click
	if (y >= LAYOUT->BW_TRAPEZ_POS_Y && y <= LAYOUT->BW_TRAPEZ_POS_Y + LAYOUT->BW_TRAPEZ_HEIGHT && x >= LAYOUT->BW_TRAPEZ_POS_X && x <= LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH) {
		BUTTONHANDLER_BW(0);
		return;
	}

	for (uint8_t i = 0; i < TouchpadButton_handlers_count; i++) {
		if ((TouchpadButton_handlers[i].x1 <= x) && (TouchpadButton_handlers[i].y1 <= y) && (TouchpadButton_handlers[i].x2 >= x) && (TouchpadButton_handlers[i].y2 >= y)) {
			if (TouchpadButton_handlers[i].holdHandler != NULL) {
				TouchpadButton_handlers[i].holdHandler(TouchpadButton_handlers[i].parameter);
				return;
			}
		}
	}

	// bottom buttons extended zone
	if (y >= 430) {
		for (uint8_t i = 0; i < TouchpadButton_handlers_count; i++) {
			if ((TouchpadButton_handlers[i].x1 <= x) && (TouchpadButton_handlers[i].y1 - 30 <= y) && (TouchpadButton_handlers[i].x2 >= x) && (TouchpadButton_handlers[i].y2 >= y)) {
				if (TouchpadButton_handlers[i].holdHandler != NULL) {
					TouchpadButton_handlers[i].holdHandler(TouchpadButton_handlers[i].parameter);
					return;
				}
			}
		}
	}

	// Free Tune / Center
	if (y > LAYOUT->FFT_FFTWTF_POS_Y && y <= (LAYOUT->FFT_FFTWTF_POS_Y + FFT_AND_WTF_HEIGHT - 50) && x > 0 && x < LCD_WIDTH && !TRX.SPLIT_Enabled) {
		if (!LCD_systemMenuOpened) {
			BUTTONHANDLER_Free_tune(0);
			LCD_redraw(false);
			return;
		}
	}
#endif
}

bool LCD_processSwipeTouch(uint16_t x, uint16_t y, int16_t dx, int16_t dy) {
#if (defined(HAS_TOUCHPAD))
	TRX_Inactive_Time = 0;
	if (TRX.Locked) {
		return false;
	}
	if (LCD_systemMenuOpened || LCD_window.opened) {
		return false;
	}

	// fft/wtf swipe
	if (((LAYOUT->FFT_FFTWTF_POS_Y + 50) <= y) && (LAYOUT->FFT_PRINT_SIZE >= x) && ((LAYOUT->FFT_FFTWTF_POS_Y + FFT_AND_WTF_HEIGHT - 50) >= y)) {
		if (TRX_on_TX) {
			return false;
		}

		TRX_ScanMode = false;
		VFO *vfo = TRX.SPLIT_Enabled ? SecondaryVFO : CurrentVFO;

		const uint8_t slowler = 4;
		if (dx > -slowler && dx < slowler) {
			return false;
		}

		float64_t step = TRX.FRQ_STEP_SSB_Hz;
		if (vfo->Mode == TRX_MODE_CW) {
			step = (float64_t)TRX.FRQ_STEP_CW_Hz;
		}
		if (TRX.Fast) {
			step *= (float64_t)TRX.FAST_STEP_Multiplier;
		}
		if (step < 1.0) {
			step = 1.0;
		}

		uint64_t newfreq = getFreqOnFFTPosition(getFreqPositionOnFFT(vfo->Freq, true) + (float64_t)dx / slowler);
		if (dx > 0) {
			newfreq = ceill(newfreq / step) * step;
		}
		if (dx < 0) {
			newfreq = floorl(newfreq / step) * step;
		}

		TRX_setFrequency(newfreq, vfo);
		LCD_UpdateQuery.FreqInfo = true;
		return true;
	}

	// bottom buttons
	if ((LAYOUT->FFT_FFTWTF_POS_Y + FFT_AND_WTF_HEIGHT - 50) < y) {
		if (dx < -50) {
			TRX.FRONTPANEL_funcbuttons_page++;
			if (TRX.FRONTPANEL_funcbuttons_page >= FUNCBUTTONS_PAGES) {
				TRX.FRONTPANEL_funcbuttons_page = 0;
			}
			LCD_UpdateQuery.BottomButtons = true;
			LCD_UpdateQuery.TopButtons = true;
			return true; // stop
		}
		if (dx > 50) {
			if (TRX.FRONTPANEL_funcbuttons_page == 0) {
				TRX.FRONTPANEL_funcbuttons_page = FUNCBUTTONS_PAGES - 1;
			} else {
				TRX.FRONTPANEL_funcbuttons_page--;
			}
			LCD_UpdateQuery.BottomButtons = true;
			LCD_UpdateQuery.TopButtons = true;
			return true; // stop
		}
	}
#endif
	return false;
}

bool LCD_processSwipeTwoFingersTouch(uint16_t x, uint16_t y, int16_t dx, int16_t dy) {
#if (defined(HAS_TOUCHPAD))
	TRX_Inactive_Time = 0;
	if (TRX.Locked) {
		return false;
	}
	if (LCD_systemMenuOpened || LCD_window.opened) {
		return false;
	}
	if (TRX_on_TX) {
		return false;
	}

	// fft/wtf two finger swipe zoom
	if (((LAYOUT->FFT_FFTWTF_POS_Y + 50) <= y) && (LAYOUT->FFT_PRINT_SIZE >= x) && ((LAYOUT->FFT_FFTWTF_POS_Y + FFT_AND_WTF_HEIGHT - 50) >= y)) {
		if (dx < -50) {
			BUTTONHANDLER_ZOOM_P(0);
			return true;
		} else if (dx > 50) {
			BUTTONHANDLER_ZOOM_N(0);
			return true;
		}
	}
#endif
	return false;
}

void LCD_showTooltip(char text[]) {
	Tooltip_DiplayStartTime = HAL_GetTick();
	strcpy(Tooltip_string, text);
	Tooltip_first_draw = true;
	if (LCD_UpdateQuery.Tooltip) // redraw old tooltip
	{
		LCD_UpdateQuery.StatusInfoBarRedraw = true;
		LCD_UpdateQuery.FreqInfoRedraw = true;
		LCD_UpdateQuery.StatusInfoGUI = true;
	}
	LCD_UpdateQuery.Tooltip = true;
	println((char *)text);
}

static void LCD_printTooltip(void) {
	LCD_UpdateQuery.Tooltip = true;
	if (LCD_busy) {
		return;
	}
	if (LCD_systemMenuOpened || LCD_window.opened) {
		LCD_UpdateQuery.Tooltip = false;
		return;
	}
	LCD_busy = true;

	uint16_t x1, y1, w, h;
	LCDDriver_getTextBoundsFont(Tooltip_string, LAYOUT->TOOLTIP_POS_X, LAYOUT->TOOLTIP_POS_Y, &x1, &y1, &w, &h, LAYOUT->TOOLTIP_FONT);
	if (Tooltip_first_draw) {
		LCDDriver_Fill_RectWH(LAYOUT->TOOLTIP_POS_X - w / 2, LAYOUT->TOOLTIP_POS_Y, w + LAYOUT->TOOLTIP_MARGIN * 2, h + LAYOUT->TOOLTIP_MARGIN * 2, COLOR->TOOLTIP_BACK);
		LCDDriver_drawRectXY(LAYOUT->TOOLTIP_POS_X - w / 2, LAYOUT->TOOLTIP_POS_Y, LAYOUT->TOOLTIP_POS_X - w / 2 + w + LAYOUT->TOOLTIP_MARGIN * 2,
		                     LAYOUT->TOOLTIP_POS_Y + h + LAYOUT->TOOLTIP_MARGIN * 2, COLOR->TOOLTIP_BORD);
		Tooltip_first_draw = false;
	}
	LCDDriver_printTextFont(Tooltip_string, LAYOUT->TOOLTIP_POS_X - w / 2 + LAYOUT->TOOLTIP_MARGIN, LAYOUT->TOOLTIP_POS_Y + LAYOUT->TOOLTIP_MARGIN + h, COLOR->TOOLTIP_FORE,
	                        COLOR->TOOLTIP_BACK, LAYOUT->TOOLTIP_FONT);

	LCD_busy = false;
	if ((HAL_GetTick() - Tooltip_DiplayStartTime) > LAYOUT->TOOLTIP_TIMEOUT) {
		LCD_UpdateQuery.Tooltip = false;
		LCD_UpdateQuery.FreqInfoRedraw = true;
		LCD_UpdateQuery.StatusInfoGUI = true;
		LCD_UpdateQuery.StatusInfoBarRedraw = true;
	}
}

void LCD_openWindow(uint16_t w, uint16_t h) {
#if (defined(HAS_TOUCHPAD))
	LCD_busy = true;
	LCD_window.opened = true;
	LCDDriver_fadeScreen(0.2f);
	uint16_t x = LCD_WIDTH / 2 - w / 2;
	uint16_t y = LCD_HEIGHT / 2 - h / 2;
	LCD_window.y = y;
	LCD_window.x = x;
	LCD_window.w = w;
	LCD_window.h = h;
	LCDDriver_drawRoundedRectWH(x, y, w, h, COLOR->WINDOWS_BORDER, 5, false);
	LCDDriver_drawRoundedRectWH(x + 1, y + 1, w - 2, h - 2, COLOR->WINDOWS_BG, 5, true);
	LCD_busy = false;

	LCD_window.buttons_count = 0;
#endif
}

void LCD_closeWindow(void) {
#if (defined(HAS_TOUCHPAD))
	LCD_window.opened = false;
	LCD_window.buttons_count = 0;
	LCD_redraw(false);
#endif
}

static void LCD_showBandWindow(bool secondary_vfo) {
#if (defined(HAS_TOUCHPAD) && defined(LAY_800x480))
	// TX block
	if (TRX_on_TX) {
		return;
	}

	uint8_t buttons_in_line = 6;
	if (CALIBRATE.ENABLE_23cm_band || TRX.Transverter_23cm || TRX.Transverter_13cm || TRX.Transverter_6cm || TRX.Transverter_3cm || TRX.Transverter_QO100 || TRX.Transverter_1_2cm ||
	    BAND_SELECTABLE[BANDID_60m] || BAND_SELECTABLE[BANDID_4m] || BAND_SELECTABLE[BANDID_AIR] || BAND_SELECTABLE[BANDID_Marine]) {
		buttons_in_line = 7;
	}

	uint8_t selectable_bands_count = 0;
	uint8_t broadcast_bands_count = 0;
	for (uint8_t i = 0; i < BANDS_COUNT; i++) {
		if (BAND_SELECTABLE[i]) {
			selectable_bands_count++;
		}
		if (BANDS[i].broadcast && !(i == BANDID_FM && !CALIBRATE.ENABLE_FM_band)) {
			broadcast_bands_count++;
		}
	}
	selectable_bands_count++; // memory bank
	selectable_bands_count++; // manual enter

	const uint8_t buttons_lines_selectable = ceilf((float32_t)selectable_bands_count / (float32_t)buttons_in_line);
	const uint8_t buttons_lines_broadcast = ceilf((float32_t)broadcast_bands_count / (float32_t)buttons_in_line);
	const uint8_t divider_height = 30;
	uint16_t window_width = LAYOUT->WINDOWS_BUTTON_WIDTH * buttons_in_line + LAYOUT->WINDOWS_BUTTON_MARGIN * (buttons_in_line + 1);
	uint16_t window_height = LAYOUT->WINDOWS_BUTTON_HEIGHT * (buttons_lines_selectable + buttons_lines_broadcast) + divider_height +
	                         LAYOUT->WINDOWS_BUTTON_MARGIN * (buttons_lines_selectable + buttons_lines_broadcast + 1);
	LCD_openWindow(window_width, window_height);
	LCD_busy = true;
	int8_t curband = getBandFromFreq(TRX.VFO_A.Freq, true);
	if (secondary_vfo) {
		curband = getBandFromFreq(TRX.VFO_B.Freq, true);
	}

	// selectable bands first
	uint8_t yi = 0;
	uint8_t xi = 0;
	for (uint8_t bindx = 0; bindx < BANDS_COUNT; bindx++) {
		if (!BAND_SELECTABLE[bindx] || BANDS[bindx].broadcast) {
			continue;
		}

		void *band_function = secondary_vfo ? BUTTONHANDLER_SET_VFOB_BAND : BUTTONHANDLER_SET_VFOA_BAND;

		printButton(LAYOUT->WINDOWS_BUTTON_MARGIN + xi * (LAYOUT->WINDOWS_BUTTON_WIDTH + LAYOUT->WINDOWS_BUTTON_MARGIN),
		            LAYOUT->WINDOWS_BUTTON_MARGIN + yi * (LAYOUT->WINDOWS_BUTTON_HEIGHT + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_WIDTH, LAYOUT->WINDOWS_BUTTON_HEIGHT,
		            (char *)BANDS[bindx].name, (curband == bindx), true, true, bindx, band_function, band_function, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT, COLOR->BUTTON_BORDER,
		            COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND, LAYOUT->TOPBUTTONS_FONT);

		xi++;
		if (xi >= buttons_in_line) {
			yi++;
			xi = 0;
		}
	}

	// manual enter
	void *band_function = secondary_vfo ? BUTTONHANDLER_SET_VFOB_FREQ_MANUAL : BUTTONHANDLER_SET_VFOA_FREQ_MANUAL;
	printButton(LAYOUT->WINDOWS_BUTTON_MARGIN + xi * (LAYOUT->WINDOWS_BUTTON_WIDTH + LAYOUT->WINDOWS_BUTTON_MARGIN),
	            LAYOUT->WINDOWS_BUTTON_MARGIN + yi * (LAYOUT->WINDOWS_BUTTON_HEIGHT + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_WIDTH, LAYOUT->WINDOWS_BUTTON_HEIGHT, "Manual",
	            false, true, true, 0, band_function, band_function, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT, COLOR->BUTTON_BORDER, COLOR->BUTTON_SWITCH_BACKGROUND,
	            LAYOUT->TOPBUTTONS_ROUND, LAYOUT->TOPBUTTONS_FONT);
	xi++;
	if (xi >= buttons_in_line) {
		yi++;
		xi = 0;
	}

	// memory bank
	printButton(LAYOUT->WINDOWS_BUTTON_MARGIN + xi * (LAYOUT->WINDOWS_BUTTON_WIDTH + LAYOUT->WINDOWS_BUTTON_MARGIN),
	            LAYOUT->WINDOWS_BUTTON_MARGIN + yi * (LAYOUT->WINDOWS_BUTTON_HEIGHT + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_WIDTH, LAYOUT->WINDOWS_BUTTON_HEIGHT, "MEM", false,
	            true, true, 0, LCD_ShowMemoryChannelsButtonHandler, LCD_ShowMemoryChannelsButtonHandler, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT, COLOR->BUTTON_BORDER,
	            COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND, LAYOUT->TOPBUTTONS_FONT);
	xi++;
	if (xi >= buttons_in_line) {
		yi++;
		xi = 0;
	}

	// divider
	if (xi != 0) {
		yi++;
	}
	LCDDriver_drawFastHLine(LCD_WIDTH / 2 - window_width / 2,
	                        LCD_window.y + LAYOUT->WINDOWS_BUTTON_MARGIN + divider_height / 3 + yi * (LAYOUT->WINDOWS_BUTTON_HEIGHT + LAYOUT->WINDOWS_BUTTON_MARGIN), window_width,
	                        COLOR->WINDOWS_BORDER);
	// broadcast bands next
	xi = 0;
	for (uint8_t bindx = 0; bindx < BANDS_COUNT; bindx++) {
		if (!BANDS[bindx].broadcast || (bindx == BANDID_FM && !CALIBRATE.ENABLE_FM_band)) {
			continue;
		}

		if (!secondary_vfo) {
			printButton(LAYOUT->WINDOWS_BUTTON_MARGIN + xi * (LAYOUT->WINDOWS_BUTTON_WIDTH + LAYOUT->WINDOWS_BUTTON_MARGIN),
			            LAYOUT->WINDOWS_BUTTON_MARGIN + divider_height + yi * (LAYOUT->WINDOWS_BUTTON_HEIGHT + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_WIDTH,
			            LAYOUT->WINDOWS_BUTTON_HEIGHT, (char *)BANDS[bindx].name, (curband == bindx), true, true, bindx, BUTTONHANDLER_SET_VFOA_BAND, BUTTONHANDLER_SET_VFOA_BAND,
			            COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT, COLOR->BUTTON_BORDER, COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND, LAYOUT->TOPBUTTONS_FONT);
		} else {
			printButton(LAYOUT->WINDOWS_BUTTON_MARGIN + xi * (LAYOUT->WINDOWS_BUTTON_WIDTH + LAYOUT->WINDOWS_BUTTON_MARGIN),
			            LAYOUT->WINDOWS_BUTTON_MARGIN + divider_height + yi * (LAYOUT->WINDOWS_BUTTON_HEIGHT + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_WIDTH,
			            LAYOUT->WINDOWS_BUTTON_HEIGHT, (char *)BANDS[bindx].name, (curband == bindx), true, true, bindx, BUTTONHANDLER_SET_VFOB_BAND, BUTTONHANDLER_SET_VFOB_BAND,
			            COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT, COLOR->BUTTON_BORDER, COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND, LAYOUT->TOPBUTTONS_FONT);
		}

		xi++;
		if (xi >= buttons_in_line) {
			yi++;
			xi = 0;
		}
	}

	LCD_busy = false;
#endif
}

static void LCD_showModeWindow(bool secondary_vfo) {
#if (defined(HAS_TOUCHPAD) && defined(LAY_800x480))
	const uint8_t buttons_in_line = 5;
	const uint8_t buttons_lines = ceilf((float32_t)TRX_MODE_COUNT / (float32_t)buttons_in_line);
	uint16_t window_width = LAYOUT->WINDOWS_BUTTON_WIDTH * buttons_in_line + LAYOUT->WINDOWS_BUTTON_MARGIN * (buttons_in_line + 1);
	uint16_t window_height = LAYOUT->WINDOWS_BUTTON_HEIGHT * buttons_lines + LAYOUT->WINDOWS_BUTTON_MARGIN * (buttons_lines + 1);
	LCD_openWindow(window_width, window_height);
	LCD_busy = true;
	int8_t curmode = TRX.VFO_A.Mode;
	if (secondary_vfo) {
		curmode = TRX.VFO_B.Mode;
	}
	for (uint8_t yi = 0; yi < buttons_lines; yi++) {
		for (uint8_t xi = 0; xi < buttons_in_line; xi++) {
			uint8_t index = yi * buttons_in_line + xi;
			if (index < TRX_MODE_COUNT) {
				if (!secondary_vfo) {
					printButton(LAYOUT->WINDOWS_BUTTON_MARGIN + xi * (LAYOUT->WINDOWS_BUTTON_WIDTH + LAYOUT->WINDOWS_BUTTON_MARGIN),
					            LAYOUT->WINDOWS_BUTTON_MARGIN + yi * (LAYOUT->WINDOWS_BUTTON_HEIGHT + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_WIDTH, LAYOUT->WINDOWS_BUTTON_HEIGHT,
					            (char *)MODE_DESCR[index], (curmode == index), true, true, index, BUTTONHANDLER_SETMODE, BUTTONHANDLER_SETMODE, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT,
					            COLOR->BUTTON_BORDER, COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND, LAYOUT->TOPBUTTONS_FONT);
				} else {
					printButton(LAYOUT->WINDOWS_BUTTON_MARGIN + xi * (LAYOUT->WINDOWS_BUTTON_WIDTH + LAYOUT->WINDOWS_BUTTON_MARGIN),
					            LAYOUT->WINDOWS_BUTTON_MARGIN + yi * (LAYOUT->WINDOWS_BUTTON_HEIGHT + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_WIDTH, LAYOUT->WINDOWS_BUTTON_HEIGHT,
					            (char *)MODE_DESCR[index], (curmode == index), true, true, index, BUTTONHANDLER_SETSECMODE, BUTTONHANDLER_SETSECMODE, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT,
					            COLOR->BUTTON_BORDER, COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND, LAYOUT->TOPBUTTONS_FONT);
				}
			}
		}
	}
	LCD_busy = false;
#endif
}

static void LCD_showBWWindow(void) {
#if (defined(HAS_TOUCHPAD) && defined(LAY_800x480))

	uint8_t filters_count = 0;
	uint32_t cur_width = CurrentVFO->LPF_RX_Filter_Width;
	if (TRX_on_TX) {
		cur_width = CurrentVFO->LPF_TX_Filter_Width;
	}
	if (CurrentVFO->Mode == TRX_MODE_CW) {
		filters_count = CW_LPF_COUNT;
	}
	if (CurrentVFO->Mode == TRX_MODE_LSB || CurrentVFO->Mode == TRX_MODE_USB || CurrentVFO->Mode == TRX_MODE_DIGI_L || CurrentVFO->Mode == TRX_MODE_DIGI_U ||
	    CurrentVFO->Mode == TRX_MODE_RTTY) {
		filters_count = SSB_LPF_COUNT;
	}
	if (CurrentVFO->Mode == TRX_MODE_AM || CurrentVFO->Mode == TRX_MODE_SAM_STEREO || CurrentVFO->Mode == TRX_MODE_SAM_LSB || CurrentVFO->Mode == TRX_MODE_SAM_USB) {
		filters_count = AM_LPF_COUNT;
	}
	if (CurrentVFO->Mode == TRX_MODE_NFM) {
		filters_count = NFM_LPF_COUNT;
	}
	if (filters_count == 0) {
		return;
	}

	const uint8_t buttons_in_line = 6;
	const uint8_t buttons_lines = ceilf((float32_t)filters_count / (float32_t)buttons_in_line);
	uint16_t window_width = LAYOUT->WINDOWS_BUTTON_WIDTH * buttons_in_line + LAYOUT->WINDOWS_BUTTON_MARGIN * (buttons_in_line + 1);
	uint16_t window_height = LAYOUT->WINDOWS_BUTTON_HEIGHT * buttons_lines + LAYOUT->WINDOWS_BUTTON_MARGIN * (buttons_lines + 1);
	LCD_openWindow(window_width, window_height);
	LCD_busy = true;
	for (uint8_t yi = 0; yi < buttons_lines; yi++) {
		for (uint8_t xi = 0; xi < buttons_in_line; xi++) {
			uint8_t index = yi * buttons_in_line + xi;
			uint32_t width = 0;
			if (CurrentVFO->Mode == TRX_MODE_CW) {
				width = AUTIO_FILTERS_LPF_CW_LIST[index];
			}
			if (CurrentVFO->Mode == TRX_MODE_LSB || CurrentVFO->Mode == TRX_MODE_USB || CurrentVFO->Mode == TRX_MODE_DIGI_L || CurrentVFO->Mode == TRX_MODE_DIGI_U ||
			    CurrentVFO->Mode == TRX_MODE_RTTY) {
				width = AUTIO_FILTERS_LPF_SSB_LIST[index];
			}
			if (CurrentVFO->Mode == TRX_MODE_AM || CurrentVFO->Mode == TRX_MODE_SAM_STEREO || CurrentVFO->Mode == TRX_MODE_SAM_LSB || CurrentVFO->Mode == TRX_MODE_SAM_USB) {
				width = AUTIO_FILTERS_LPF_AM_LIST[index];
			}
			if (CurrentVFO->Mode == TRX_MODE_NFM) {
				width = AUTIO_FILTERS_LPF_NFM_LIST[index];
			}
			char str[16];
			sprintf(str, "%d", width);
			if (index < filters_count) {
				if (TRX_on_TX) {
					printButton(LAYOUT->WINDOWS_BUTTON_MARGIN + xi * (LAYOUT->WINDOWS_BUTTON_WIDTH + LAYOUT->WINDOWS_BUTTON_MARGIN),
					            LAYOUT->WINDOWS_BUTTON_MARGIN + yi * (LAYOUT->WINDOWS_BUTTON_HEIGHT + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_WIDTH, LAYOUT->WINDOWS_BUTTON_HEIGHT, str,
					            (width == cur_width), true, true, width, BUTTONHANDLER_SET_TX_BW, BUTTONHANDLER_SET_TX_BW, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT, COLOR->BUTTON_BORDER,
					            COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND, LAYOUT->TOPBUTTONS_FONT);
				} else {
					printButton(LAYOUT->WINDOWS_BUTTON_MARGIN + xi * (LAYOUT->WINDOWS_BUTTON_WIDTH + LAYOUT->WINDOWS_BUTTON_MARGIN),
					            LAYOUT->WINDOWS_BUTTON_MARGIN + yi * (LAYOUT->WINDOWS_BUTTON_HEIGHT + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_WIDTH, LAYOUT->WINDOWS_BUTTON_HEIGHT, str,
					            (width == cur_width), true, true, width, BUTTONHANDLER_SET_RX_BW, BUTTONHANDLER_SET_RX_BW, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT, COLOR->BUTTON_BORDER,
					            COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND, LAYOUT->TOPBUTTONS_FONT);
				}
			}
		}
	}
	LCD_busy = false;
#endif
}

void LCD_showRFPowerWindow(void) {
#if (defined(HAS_TOUCHPAD) && defined(LAY_800x480))
	const uint8_t buttons_in_line = 5;
	const uint8_t buttons_lines = 2;
	uint16_t window_width = LAYOUT->WINDOWS_BUTTON_WIDTH * buttons_in_line + LAYOUT->WINDOWS_BUTTON_MARGIN * (buttons_in_line + 1);
	uint16_t window_height = LAYOUT->WINDOWS_BUTTON_HEIGHT * buttons_lines + LAYOUT->WINDOWS_BUTTON_MARGIN * (buttons_lines + 1);
	while (LCD_busy) {
		;
	}
	LCD_openWindow(window_width, window_height);
	LCD_busy = true;
	for (uint8_t yi = 0; yi < buttons_lines; yi++) {
		for (uint8_t xi = 0; xi < buttons_in_line; xi++) {
			uint8_t index = 0 + (yi * buttons_in_line + xi) * 10;
			if (index > 50) {
				index += 10;
			}
			char str[8];
			sprintf(str, "%d%%", index);
			printButton(LAYOUT->WINDOWS_BUTTON_MARGIN + xi * (LAYOUT->WINDOWS_BUTTON_WIDTH + LAYOUT->WINDOWS_BUTTON_MARGIN),
			            LAYOUT->WINDOWS_BUTTON_MARGIN + yi * (LAYOUT->WINDOWS_BUTTON_HEIGHT + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_WIDTH, LAYOUT->WINDOWS_BUTTON_HEIGHT, str,
			            (TRX.RF_Gain == index), true, true, index, BUTTONHANDLER_SETRF_POWER, BUTTONHANDLER_SETRF_POWER, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT, COLOR->BUTTON_BORDER,
			            COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND, LAYOUT->TOPBUTTONS_FONT);
		}
	}
	LCD_busy = false;
#endif
}

void LCD_showManualFreqWindow(uint32_t parameter) {
	if (LCD_busy) {
		return;
	}
#if (defined(HAS_TOUCHPAD) && defined(LAY_800x480))
	LCD_busy = true;
	strcpy(manualFreqEnterString, "");
	const uint8_t buttons_in_line = 3;
	const uint8_t buttons_top_offset = 100;
	const uint8_t buttons_lines = 5;
	uint16_t window_width = LAYOUT->WINDOWS_BUTTON_WIDTH * buttons_in_line + LAYOUT->WINDOWS_BUTTON_MARGIN * (buttons_in_line + 1);
	uint16_t window_height = LAYOUT->WINDOWS_BUTTON_HEIGHT * buttons_lines + buttons_top_offset + LAYOUT->WINDOWS_BUTTON_MARGIN * (buttons_lines + 1);
	LCD_openWindow(window_width, window_height);

	printButton(LAYOUT->WINDOWS_BUTTON_MARGIN + 0 * (LAYOUT->WINDOWS_BUTTON_WIDTH + LAYOUT->WINDOWS_BUTTON_MARGIN),
	            buttons_top_offset + LAYOUT->WINDOWS_BUTTON_MARGIN + 0 * (LAYOUT->WINDOWS_BUTTON_HEIGHT + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_WIDTH,
	            LAYOUT->WINDOWS_BUTTON_HEIGHT, "1", false, 0, true, 1, LCD_ManualFreqButtonHandler, LCD_ManualFreqButtonHandler, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT,
	            COLOR->BUTTON_BORDER, COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND, LAYOUT->TOPBUTTONS_FONT);
	printButton(LAYOUT->WINDOWS_BUTTON_MARGIN + 1 * (LAYOUT->WINDOWS_BUTTON_WIDTH + LAYOUT->WINDOWS_BUTTON_MARGIN),
	            buttons_top_offset + LAYOUT->WINDOWS_BUTTON_MARGIN + 0 * (LAYOUT->WINDOWS_BUTTON_HEIGHT + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_WIDTH,
	            LAYOUT->WINDOWS_BUTTON_HEIGHT, "2", false, 0, true, 2, LCD_ManualFreqButtonHandler, LCD_ManualFreqButtonHandler, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT,
	            COLOR->BUTTON_BORDER, COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND, LAYOUT->TOPBUTTONS_FONT);
	printButton(LAYOUT->WINDOWS_BUTTON_MARGIN + 2 * (LAYOUT->WINDOWS_BUTTON_WIDTH + LAYOUT->WINDOWS_BUTTON_MARGIN),
	            buttons_top_offset + LAYOUT->WINDOWS_BUTTON_MARGIN + 0 * (LAYOUT->WINDOWS_BUTTON_HEIGHT + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_WIDTH,
	            LAYOUT->WINDOWS_BUTTON_HEIGHT, "3", false, 0, true, 3, LCD_ManualFreqButtonHandler, LCD_ManualFreqButtonHandler, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT,
	            COLOR->BUTTON_BORDER, COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND, LAYOUT->TOPBUTTONS_FONT);

	printButton(LAYOUT->WINDOWS_BUTTON_MARGIN + 0 * (LAYOUT->WINDOWS_BUTTON_WIDTH + LAYOUT->WINDOWS_BUTTON_MARGIN),
	            buttons_top_offset + LAYOUT->WINDOWS_BUTTON_MARGIN + 1 * (LAYOUT->WINDOWS_BUTTON_HEIGHT + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_WIDTH,
	            LAYOUT->WINDOWS_BUTTON_HEIGHT, "4", false, 0, true, 4, LCD_ManualFreqButtonHandler, LCD_ManualFreqButtonHandler, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT,
	            COLOR->BUTTON_BORDER, COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND, LAYOUT->TOPBUTTONS_FONT);
	printButton(LAYOUT->WINDOWS_BUTTON_MARGIN + 1 * (LAYOUT->WINDOWS_BUTTON_WIDTH + LAYOUT->WINDOWS_BUTTON_MARGIN),
	            buttons_top_offset + LAYOUT->WINDOWS_BUTTON_MARGIN + 1 * (LAYOUT->WINDOWS_BUTTON_HEIGHT + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_WIDTH,
	            LAYOUT->WINDOWS_BUTTON_HEIGHT, "5", false, 0, true, 5, LCD_ManualFreqButtonHandler, LCD_ManualFreqButtonHandler, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT,
	            COLOR->BUTTON_BORDER, COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND, LAYOUT->TOPBUTTONS_FONT);
	printButton(LAYOUT->WINDOWS_BUTTON_MARGIN + 2 * (LAYOUT->WINDOWS_BUTTON_WIDTH + LAYOUT->WINDOWS_BUTTON_MARGIN),
	            buttons_top_offset + LAYOUT->WINDOWS_BUTTON_MARGIN + 1 * (LAYOUT->WINDOWS_BUTTON_HEIGHT + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_WIDTH,
	            LAYOUT->WINDOWS_BUTTON_HEIGHT, "6", false, 0, true, 6, LCD_ManualFreqButtonHandler, LCD_ManualFreqButtonHandler, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT,
	            COLOR->BUTTON_BORDER, COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND, LAYOUT->TOPBUTTONS_FONT);

	printButton(LAYOUT->WINDOWS_BUTTON_MARGIN + 0 * (LAYOUT->WINDOWS_BUTTON_WIDTH + LAYOUT->WINDOWS_BUTTON_MARGIN),
	            buttons_top_offset + LAYOUT->WINDOWS_BUTTON_MARGIN + 2 * (LAYOUT->WINDOWS_BUTTON_HEIGHT + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_WIDTH,
	            LAYOUT->WINDOWS_BUTTON_HEIGHT, "7", false, 0, true, 7, LCD_ManualFreqButtonHandler, LCD_ManualFreqButtonHandler, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT,
	            COLOR->BUTTON_BORDER, COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND, LAYOUT->TOPBUTTONS_FONT);
	printButton(LAYOUT->WINDOWS_BUTTON_MARGIN + 1 * (LAYOUT->WINDOWS_BUTTON_WIDTH + LAYOUT->WINDOWS_BUTTON_MARGIN),
	            buttons_top_offset + LAYOUT->WINDOWS_BUTTON_MARGIN + 2 * (LAYOUT->WINDOWS_BUTTON_HEIGHT + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_WIDTH,
	            LAYOUT->WINDOWS_BUTTON_HEIGHT, "8", false, 0, true, 8, LCD_ManualFreqButtonHandler, LCD_ManualFreqButtonHandler, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT,
	            COLOR->BUTTON_BORDER, COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND, LAYOUT->TOPBUTTONS_FONT);
	printButton(LAYOUT->WINDOWS_BUTTON_MARGIN + 2 * (LAYOUT->WINDOWS_BUTTON_WIDTH + LAYOUT->WINDOWS_BUTTON_MARGIN),
	            buttons_top_offset + LAYOUT->WINDOWS_BUTTON_MARGIN + 2 * (LAYOUT->WINDOWS_BUTTON_HEIGHT + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_WIDTH,
	            LAYOUT->WINDOWS_BUTTON_HEIGHT, "9", false, 0, true, 9, LCD_ManualFreqButtonHandler, LCD_ManualFreqButtonHandler, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT,
	            COLOR->BUTTON_BORDER, COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND, LAYOUT->TOPBUTTONS_FONT);

	printButton(LAYOUT->WINDOWS_BUTTON_MARGIN + 1 * (LAYOUT->WINDOWS_BUTTON_WIDTH + LAYOUT->WINDOWS_BUTTON_MARGIN),
	            buttons_top_offset + LAYOUT->WINDOWS_BUTTON_MARGIN + 3 * (LAYOUT->WINDOWS_BUTTON_HEIGHT + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_WIDTH,
	            LAYOUT->WINDOWS_BUTTON_HEIGHT, "0", false, 0, true, 0, LCD_ManualFreqButtonHandler, LCD_ManualFreqButtonHandler, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT,
	            COLOR->BUTTON_BORDER, COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND, LAYOUT->TOPBUTTONS_FONT);
	printButton(LAYOUT->WINDOWS_BUTTON_MARGIN + 2 * (LAYOUT->WINDOWS_BUTTON_WIDTH + LAYOUT->WINDOWS_BUTTON_MARGIN),
	            buttons_top_offset + LAYOUT->WINDOWS_BUTTON_MARGIN + 3 * (LAYOUT->WINDOWS_BUTTON_HEIGHT + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_WIDTH,
	            LAYOUT->WINDOWS_BUTTON_HEIGHT, ".", false, 0, true, 10, LCD_ManualFreqButtonHandler, LCD_ManualFreqButtonHandler, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT,
	            COLOR->BUTTON_BORDER, COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND, LAYOUT->TOPBUTTONS_FONT);

	printButton(LAYOUT->WINDOWS_BUTTON_MARGIN + LAYOUT->WINDOWS_BUTTON_WIDTH * 2 + LAYOUT->WINDOWS_BUTTON_MARGIN * 2, 30, LAYOUT->WINDOWS_BUTTON_WIDTH, LAYOUT->WINDOWS_BUTTON_HEIGHT, "<-",
	            false, 0, true, 11, LCD_ManualFreqButtonHandler, LCD_ManualFreqButtonHandler, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT, COLOR->BUTTON_BORDER,
	            COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND, LAYOUT->TOPBUTTONS_FONT);

	printButton(LAYOUT->WINDOWS_BUTTON_MARGIN + 0 * (LAYOUT->WINDOWS_BUTTON_WIDTH + LAYOUT->WINDOWS_BUTTON_MARGIN),
	            buttons_top_offset + LAYOUT->WINDOWS_BUTTON_MARGIN + 4 * (LAYOUT->WINDOWS_BUTTON_HEIGHT + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_WIDTH,
	            LAYOUT->WINDOWS_BUTTON_HEIGHT, "Hz", false, 0, true, 14, LCD_ManualFreqButtonHandler, LCD_ManualFreqButtonHandler, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT,
	            COLOR->BUTTON_BORDER, COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND, LAYOUT->TOPBUTTONS_FONT);
	printButton(LAYOUT->WINDOWS_BUTTON_MARGIN + 1 * (LAYOUT->WINDOWS_BUTTON_WIDTH + LAYOUT->WINDOWS_BUTTON_MARGIN),
	            buttons_top_offset + LAYOUT->WINDOWS_BUTTON_MARGIN + 4 * (LAYOUT->WINDOWS_BUTTON_HEIGHT + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_WIDTH,
	            LAYOUT->WINDOWS_BUTTON_HEIGHT, "kHz", false, 0, true, 13, LCD_ManualFreqButtonHandler, LCD_ManualFreqButtonHandler, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT,
	            COLOR->BUTTON_BORDER, COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND, LAYOUT->TOPBUTTONS_FONT);
	printButton(LAYOUT->WINDOWS_BUTTON_MARGIN + 2 * (LAYOUT->WINDOWS_BUTTON_WIDTH + LAYOUT->WINDOWS_BUTTON_MARGIN),
	            buttons_top_offset + LAYOUT->WINDOWS_BUTTON_MARGIN + 4 * (LAYOUT->WINDOWS_BUTTON_HEIGHT + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_WIDTH,
	            LAYOUT->WINDOWS_BUTTON_HEIGHT, "MHz", false, 0, true, 12, LCD_ManualFreqButtonHandler, LCD_ManualFreqButtonHandler, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT,
	            COLOR->BUTTON_BORDER, COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND, LAYOUT->TOPBUTTONS_FONT);

	LCD_ManualFreqButtonHandler(99);
	LCD_busy = false;
#endif
}

void LCD_ManualFreqButtonHandler(uint32_t parameter) {
#if (defined(HAS_TOUCHPAD) && defined(LAY_800x480))
	if (parameter < 10) {
		if (strlen(manualFreqEnterString) < 11) {
			char digit[2] = {parameter + '0', 0};
			strncat(manualFreqEnterString, digit, 1);
		}
	}
	if (parameter == 10) {
		strncat(manualFreqEnterString, ".", 1);
	}
	if (parameter == 11 && strlen(manualFreqEnterString) > 0) {
		manualFreqEnterString[strlen(manualFreqEnterString) - 1] = '\0';
	}

	uint64_t newfreq = 0;
	float64_t float64val = atof(manualFreqEnterString);
	if (parameter == 12) // MHz
	{
		newfreq = round(float64val * 1000000.0);
		LCD_closeWindow();
	}
	if (parameter == 13) // kHz
	{
		newfreq = round(float64val * 1000.0);
		LCD_closeWindow();
	}
	if (parameter == 14) // Hz
	{
		newfreq = round(float64val * 1.0);
		LCD_closeWindow();
	}
	if (newfreq > 0) {
		TRX_setFrequency(newfreq, CurrentVFO);
	}

	printButton(LAYOUT->WINDOWS_BUTTON_MARGIN, 30, LAYOUT->WINDOWS_BUTTON_WIDTH * 2 + LAYOUT->WINDOWS_BUTTON_MARGIN, LAYOUT->WINDOWS_BUTTON_HEIGHT, manualFreqEnterString, false, false, true,
	            0, NULL, NULL, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT, COLOR->BUTTON_BORDER, COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND, LAYOUT->TOPBUTTONS_FONT);
#endif
}

void LCD_ShowMemoryChannelsButtonHandler(uint32_t parameter) {
#if (defined(HAS_TOUCHPAD) && defined(LAY_800x480))
	const uint8_t buttons_in_line = 7;
	const uint8_t buttons_lines = MEMORY_CHANNELS_COUNT / buttons_in_line;
	const uint8_t buttons_top_offset = 0;
	uint16_t window_width = LAYOUT->WINDOWS_BUTTON_WIDTH * buttons_in_line + LAYOUT->WINDOWS_BUTTON_MARGIN * (buttons_in_line + 1);
	uint16_t window_height = LAYOUT->WINDOWS_BUTTON_HEIGHT * buttons_lines + buttons_top_offset + LAYOUT->WINDOWS_BUTTON_MARGIN * (buttons_lines + 1);
	while (LCD_busy) {
		;
	}
	LCD_openWindow(window_width, window_height);
	LCD_busy = true;

	uint8_t channel_num = 1;
	char tmp[32] = {0};
	for (uint8_t y = 0; y < buttons_lines; y++) {
		for (uint8_t x = 0; x < buttons_in_line; x++) {
			if (CALIBRATE.MEMORY_CHANNELS[(channel_num - 1)].freq == 0) {
				sprintf(tmp, "-");
			} else {
				strcpy(tmp, CALIBRATE.MEMORY_CHANNELS[(channel_num - 1)].name);
			}

			printButton(LAYOUT->WINDOWS_BUTTON_MARGIN + x * (LAYOUT->WINDOWS_BUTTON_WIDTH + LAYOUT->WINDOWS_BUTTON_MARGIN),
			            buttons_top_offset + LAYOUT->WINDOWS_BUTTON_MARGIN + y * (LAYOUT->WINDOWS_BUTTON_HEIGHT + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_WIDTH,
			            LAYOUT->WINDOWS_BUTTON_HEIGHT, tmp, (CALIBRATE.MEMORY_CHANNELS[(channel_num - 1)].freq == CurrentVFO->Freq), true, true, (channel_num - 1),
			            BUTTONHANDLER_SelectMemoryChannels, BUTTONHANDLER_SaveMemoryChannels, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT, COLOR->BUTTON_BORDER,
			            COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND, LAYOUT->TOPBUTTONS_FONT);
			channel_num++;
		}
	}

	LCD_busy = false;
#endif
}

static void LCD_keyboardHandler(uint32_t parameter) {
	if (LCD_keyboardHandlerCurrent == NULL) {
		return;
	}

	LCD_keyboardHandlerCurrent(LCD_keyboards_current_string, LCD_keyboards_current_max_string_size, parameter);
}

static void LCD_printKeyboardShiftHandler(uint32_t parameter) {
	LCD_printKeyboard(LCD_keyboardHandlerCurrent, LCD_keyboards_current_string, LCD_keyboards_current_max_string_size, !LCD_keyboards_current_lowcase_status);
}

void LCD_printKeyboard(void (*keyboardHandler)(char *string, uint32_t max_size, char entered), char *string, uint32_t max_size, bool lowcase) {
#if (defined(HAS_TOUCHPAD) && defined(LAY_800x480))
	TouchpadButton_handlers_count = 0;
	LCD_keyboards_current_string = string;
	LCD_keyboards_current_max_string_size = max_size;
	LCD_keyboardHandlerCurrent = keyboardHandler;

	LCD_screenKeyboardOpened = true;
	LCDDriver_Fill_RectWH(0, LCD_HEIGHT / 2, LCD_WIDTH, LCD_HEIGHT / 2, COLOR->KEYBOARD_BG);
	const uint16_t button_width = (LCD_HEIGHT / 2 - LAYOUT->WINDOWS_BUTTON_MARGIN * 2) / 5;
	const uint16_t button_height = button_width;
	const uint16_t buttons_top_offset = LCD_HEIGHT / 2 + LAYOUT->WINDOWS_BUTTON_MARGIN;
	uint16_t buttons_left_offset = button_width / 2 - 4;
	uint16_t x = 0;
	uint16_t y = 0;

	LCD_keyboards_current_lowcase_status = lowcase;
	//
	char line1[] = "~1234567890_+<";
	if (lowcase) {
		strcpy(line1, "`1234567890-=<");
	}
	for (uint8_t i = 0; i < strlen(line1); i++) {
		char text[2] = {0};
		text[0] = line1[i];
		x = i;
		if (i == strlen(line1) - 1) { // backspace
			printButton(buttons_left_offset + LAYOUT->WINDOWS_BUTTON_MARGIN + x * (button_width + LAYOUT->WINDOWS_BUTTON_MARGIN),
			            buttons_top_offset + LAYOUT->WINDOWS_BUTTON_MARGIN + y * (button_height + LAYOUT->WINDOWS_BUTTON_MARGIN), button_width, button_height, text, true, false, false, text[0],
			            LCD_keyboardHandler, LCD_keyboardHandler, COLOR->DENY_BUTTON_TEXT, COLOR->DENY_BUTTON_TEXT, COLOR->BUTTON_BORDER, COLOR->DENY_BUTTON_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND,
			            LAYOUT->TOPBUTTONS_FONT);
		} else {
			printButton(buttons_left_offset + LAYOUT->WINDOWS_BUTTON_MARGIN + x * (button_width + LAYOUT->WINDOWS_BUTTON_MARGIN),
			            buttons_top_offset + LAYOUT->WINDOWS_BUTTON_MARGIN + y * (button_height + LAYOUT->WINDOWS_BUTTON_MARGIN), button_width, button_height, text, true, false, false, text[0],
			            LCD_keyboardHandler, LCD_keyboardHandler, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT, COLOR->BUTTON_BORDER, COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND,
			            LAYOUT->TOPBUTTONS_FONT);
		}
	}
	y++;
	// buttons_left_offset -= button_width;
	//
	char line2[] = "QWERTYUIOP{}|&";
	if (lowcase) {
		strcpy(line2, "qwertyuiop[]\\*");
	}
	for (uint8_t i = 0; i < strlen(line2); i++) {
		char text[2] = {0};
		text[0] = line2[i];
		x = i;
		printButton(buttons_left_offset + LAYOUT->WINDOWS_BUTTON_MARGIN + x * (button_width + LAYOUT->WINDOWS_BUTTON_MARGIN),
		            buttons_top_offset + LAYOUT->WINDOWS_BUTTON_MARGIN + y * (button_height + LAYOUT->WINDOWS_BUTTON_MARGIN), button_width, button_height, text, true, false, false, text[0],
		            LCD_keyboardHandler, LCD_keyboardHandler, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT, COLOR->BUTTON_BORDER, COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND,
		            LAYOUT->TOPBUTTONS_FONT);
	}
	y++;
	// buttons_left_offset += button_width / 2;
	//
	char line3[] = "ASDFGHJKL:\"$@";
	if (lowcase) {
		strcpy(line3, "asdfghjkl;'%^");
	}
	for (uint8_t i = 0; i < strlen(line3); i++) {
		char text[2] = {0};
		text[0] = line3[i];
		x = i;
		printButton(buttons_left_offset + LAYOUT->WINDOWS_BUTTON_MARGIN + x * (button_width + LAYOUT->WINDOWS_BUTTON_MARGIN),
		            buttons_top_offset + LAYOUT->WINDOWS_BUTTON_MARGIN + y * (button_height + LAYOUT->WINDOWS_BUTTON_MARGIN), button_width, button_height, text, true, false, false, text[0],
		            LCD_keyboardHandler, LCD_keyboardHandler, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT, COLOR->BUTTON_BORDER, COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND,
		            LAYOUT->TOPBUTTONS_FONT);
	}
	// enter
	x = strlen(line3);
	printButton(buttons_left_offset + LAYOUT->WINDOWS_BUTTON_MARGIN + x * (button_width + LAYOUT->WINDOWS_BUTTON_MARGIN),
	            buttons_top_offset + LAYOUT->WINDOWS_BUTTON_MARGIN + y * (button_height + LAYOUT->WINDOWS_BUTTON_MARGIN), button_width, button_height, "<=", true, false, false, NULL,
	            BUTTONHANDLER_MENU, BUTTONHANDLER_MENU, COLOR->ACCEPT_BUTTON_TEXT, COLOR->ACCEPT_BUTTON_TEXT, COLOR->BUTTON_BORDER, COLOR->ACCEPT_BUTTON_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND,
	            LAYOUT->TOPBUTTONS_FONT);
	//
	y++;
	//
	char line4[] = "^ZXCVBNM()?! ";
	if (lowcase) {
		strcpy(line4, "^zxcvbnm,./# ");
	}
	for (uint8_t i = 0; i < strlen(line4); i++) {
		char text[2] = {0};
		text[0] = line4[i];
		x = i;
		// LCD_keyboards_current_lowcase_status
		if (i == 0) { // shift
			printButton(buttons_left_offset + LAYOUT->WINDOWS_BUTTON_MARGIN + x * (button_width + LAYOUT->WINDOWS_BUTTON_MARGIN),
			            buttons_top_offset + LAYOUT->WINDOWS_BUTTON_MARGIN + y * (button_height + LAYOUT->WINDOWS_BUTTON_MARGIN), button_width, button_height, text, true, false, false, text[0],
			            LCD_printKeyboardShiftHandler, LCD_printKeyboardShiftHandler, COLOR->NORMAL_BUTTON_TEXT, COLOR->NORMAL_BUTTON_TEXT, COLOR->BUTTON_BORDER, COLOR->NORMAL_BUTTON_BACKGROUND,
			            LAYOUT->TOPBUTTONS_ROUND, LAYOUT->TOPBUTTONS_FONT);
		} else if (i == strlen(line4) - 1) { // space
			printButton(buttons_left_offset + LAYOUT->WINDOWS_BUTTON_MARGIN + x * (button_width + LAYOUT->WINDOWS_BUTTON_MARGIN),
			            buttons_top_offset + LAYOUT->WINDOWS_BUTTON_MARGIN + y * (button_height + LAYOUT->WINDOWS_BUTTON_MARGIN), button_width * 2 + LAYOUT->WINDOWS_BUTTON_MARGIN, button_height,
			            text, true, false, false, text[0], LCD_keyboardHandler, LCD_keyboardHandler, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT, COLOR->BUTTON_BORDER,
			            COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND, LAYOUT->TOPBUTTONS_FONT);
		} else {
			printButton(buttons_left_offset + LAYOUT->WINDOWS_BUTTON_MARGIN + x * (button_width + LAYOUT->WINDOWS_BUTTON_MARGIN),
			            buttons_top_offset + LAYOUT->WINDOWS_BUTTON_MARGIN + y * (button_height + LAYOUT->WINDOWS_BUTTON_MARGIN), button_width, button_height, text, true, false, false, text[0],
			            LCD_keyboardHandler, LCD_keyboardHandler, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT, COLOR->BUTTON_BORDER, COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND,
			            LAYOUT->TOPBUTTONS_FONT);
		}
	}
#endif
}

void LCD_hideKeyboard(void) {
	LCD_screenKeyboardOpened = false;
	LCD_keyboardHandlerCurrent = NULL;
}

void LCD_showATTWindow(uint32_t parameter) {
#if (defined(HAS_TOUCHPAD) && defined(LAY_800x480))
#ifdef FRONTPANEL_KT_100S
	const uint8_t buttons_in_line = 3;
	const uint8_t buttons_lines = 1;
	uint8_t dB = 6;
#else
	const uint8_t buttons_in_line = 5;
	const uint8_t buttons_lines = 2;
	uint8_t dB = 3;
#endif
	uint16_t window_width = LAYOUT->WINDOWS_BUTTON_WIDTH * buttons_in_line + LAYOUT->WINDOWS_BUTTON_MARGIN * (buttons_in_line + 1);
	uint16_t window_height = LAYOUT->WINDOWS_BUTTON_HEIGHT * buttons_lines + LAYOUT->WINDOWS_BUTTON_MARGIN * (buttons_lines + 1);
	while (LCD_busy) {
		;
	}
	LCD_openWindow(window_width, window_height);
	LCD_busy = true;
	for (uint8_t yi = 0; yi < buttons_lines; yi++) {
		for (uint8_t xi = 0; xi < buttons_in_line; xi++) {
			char str[8];
			sprintf(str, "%d dB", dB);
			printButton(LAYOUT->WINDOWS_BUTTON_MARGIN + xi * (LAYOUT->WINDOWS_BUTTON_WIDTH + LAYOUT->WINDOWS_BUTTON_MARGIN),
			            LAYOUT->WINDOWS_BUTTON_MARGIN + yi * (LAYOUT->WINDOWS_BUTTON_HEIGHT + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_WIDTH, LAYOUT->WINDOWS_BUTTON_HEIGHT, str,
			            (TRX.ATT_DB == dB), true, true, dB, BUTTONHANDLER_SET_ATT_DB, BUTTONHANDLER_SET_ATT_DB, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT, COLOR->BUTTON_BORDER,
			            COLOR->BUTTON_SWITCH_BACKGROUND, LAYOUT->TOPBUTTONS_ROUND, LAYOUT->TOPBUTTONS_FONT);

#ifdef FRONTPANEL_KT_100S
			dB += 6;
#else
			dB += 3;
			if (dB > 31) {
				dB = 31;
			}
#endif
		}
	}
	LCD_busy = false;
#endif
}
