#include "lcd.h"
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
bool TRX_X1_VLT_CUR_Mode = false; // false - VLT, true - CUR
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

static uint32_t manualFreqEnter = 0;
static bool LCD_screenKeyboardOpened = false;
static void (*LCD_keyboardHandler)(uint32_t parameter) = NULL;

static bool LCD_inited = false;
static float32_t LCD_last_s_meter = 1.0f;
static uint32_t Time;
static uint8_t Hours;
static uint8_t Last_showed_Hours = 255;
static uint8_t Minutes;
static uint8_t Last_showed_Minutes = 255;
static uint8_t Seconds;
static uint8_t Last_showed_Seconds = 255;

static uint32_t Tooltip_DiplayStartTime = 0;
static bool Tooltip_first_draw = true;
static char Tooltip_string[64] = {0};
static bool LCD_showInfo_opened = false;

static TouchpadButton_handler TouchpadButton_handlers[64] = {0};
static uint8_t TouchpadButton_handlers_count = 0;

static void printInfoSmall(uint16_t x, uint16_t y, uint16_t width, uint16_t height, char *text, uint16_t back_color, uint16_t text_color, uint16_t in_active_color, bool active);
static void printInfo(uint16_t x, uint16_t y, uint16_t width, uint16_t height, char *text, uint16_t back_color, uint16_t text_color, uint16_t in_active_color, bool active, bool border,
                      bool full_redraw);
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
static void LCD_ShowMemoryChannelsButtonHandler(uint32_t parameter);

void LCD_Init(void) {
	COLOR = &COLOR_THEMES[TRX.ColorThemeId];
	LAYOUT = &LAYOUT_THEMES[TRX.LayoutThemeId];

	LCDDriver_Init();
#if SCREEN_ROTATE
	LCDDriver_setRotation(1);
#else
	LCDDriver_setRotation(3);
#endif
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

	// display buttons
	bool full_redraw = redraw;
	static uint8_t old_funcpage = 0;
	if (TRX.FRONTPANEL_funcbuttons_page != old_funcpage) {
		full_redraw = true;
		old_funcpage = TRX.FRONTPANEL_funcbuttons_page;
	}

	uint16_t curr_x = 0;
	for (uint8_t i = 0; i < 4; i++) {
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

		if (strcmp((char *)PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[menuPosition]].name, "DNR") == 0) {
			if (CurrentVFO->DNR_Type == 0) {
				printInfo(curr_x, LAYOUT->TOPBUTTONS_Y1, LAYOUT->TOPBUTTONS_WIDTH - 1, LAYOUT->TOPBUTTONS_HEIGHT, "DNR", COLOR->BUTTON_STATUS_BACKGROUND, COLOR->BUTTON_TEXT,
				          COLOR->BUTTON_INACTIVE_TEXT, false, TRX.ENC2_func_mode == ENC_FUNC_PAGER, full_redraw);
			}
			if (CurrentVFO->DNR_Type == 1) {
				printInfo(curr_x, LAYOUT->TOPBUTTONS_Y1, LAYOUT->TOPBUTTONS_WIDTH - 1, LAYOUT->TOPBUTTONS_HEIGHT, "NR1", COLOR->BUTTON_STATUS_BACKGROUND, COLOR->BUTTON_TEXT,
				          COLOR->BUTTON_INACTIVE_TEXT, true, TRX.ENC2_func_mode == ENC_FUNC_PAGER, full_redraw);
			}
			if (CurrentVFO->DNR_Type == 2) {
				printInfo(curr_x, LAYOUT->TOPBUTTONS_Y1, LAYOUT->TOPBUTTONS_WIDTH - 1, LAYOUT->TOPBUTTONS_HEIGHT, "NR2", COLOR->BUTTON_STATUS_BACKGROUND, COLOR->BUTTON_TEXT,
				          COLOR->BUTTON_INACTIVE_TEXT, true, TRX.ENC2_func_mode == ENC_FUNC_PAGER, full_redraw);
			}
		} else {
			if (!dummyButton) {
				printInfo(curr_x, LAYOUT->TOPBUTTONS_Y1, LAYOUT->TOPBUTTONS_WIDTH - 1, LAYOUT->TOPBUTTONS_HEIGHT, (char *)PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[menuPosition]].name,
				          COLOR->BUTTON_STATUS_BACKGROUND, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT, enabled, TRX.ENC2_func_mode == ENC_FUNC_PAGER, full_redraw);
			} else {
				printInfo(curr_x, LAYOUT->TOPBUTTONS_Y1, LAYOUT->TOPBUTTONS_WIDTH - 1, LAYOUT->TOPBUTTONS_HEIGHT, " ", COLOR->BUTTON_STATUS_BACKGROUND, COLOR->BUTTON_TEXT,
				          COLOR->BUTTON_INACTIVE_TEXT, false, TRX.ENC2_func_mode == ENC_FUNC_PAGER, full_redraw);
			}
		}
		curr_x += LAYOUT->TOPBUTTONS_WIDTH + LAYOUT->TOPBUTTONS_LR_MARGIN;
	}

	LCD_UpdateQuery.TopButtons = false;
	if (redraw) {
		LCD_UpdateQuery.TopButtonsRedraw = false;
	}
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

	uint64_t display_freq = CurrentVFO->Freq;

	if (TRX.Custom_Transverter_Enabled) {
		display_freq += (uint64_t)CALIBRATE.Transverter_Custom_Offset_MHz * 1000 * 1000;
	}
	if (TRX_on_TX && !TRX.selected_vfo) {
		display_freq += TRX_XIT;
	}

	if (!redraw && (LCD_last_showed_freq == display_freq)) {
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

	LCD_last_showed_freq = display_freq;
	uint16_t Hz = (display_freq % 1000);
	uint16_t kHz = ((display_freq / 1000) % 1000);
	uint16_t MHz = ((display_freq / 1000000) % 1000);
	uint16_t GHz = ((display_freq / 1000000000) % 1000);
	if (display_freq >= 1000000000) //>= 1GHz
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
		LCDDriver_Fill_RectWH(LAYOUT->FREQ_LEFT_MARGIN, LAYOUT->FREQ_Y_TOP, LCD_WIDTH - LAYOUT->FREQ_LEFT_MARGIN - LAYOUT->FREQ_RIGHT_MARGIN, LAYOUT->FREQ_TOP_OFFSET + LAYOUT->FREQ_HEIGHT + 2,
		                      BG_COLOR);
	}

	if ((MHz_x_offset - LAYOUT->FREQ_LEFT_MARGIN) > 0) {
		LCDDriver_Fill_RectWH(LAYOUT->FREQ_LEFT_MARGIN, LAYOUT->FREQ_Y_TOP, MHz_x_offset - LAYOUT->FREQ_LEFT_MARGIN - 1, LAYOUT->FREQ_TOP_OFFSET + LAYOUT->FREQ_HEIGHT + 2, BG_COLOR);
	}

	// add spaces to output the frequency
	sprintf(LCD_freq_string_Hz, "%d", Hz);
	sprintf(LCD_freq_string_kHz, "%d", kHz);
	sprintf(LCD_freq_string_MHz, "%d", MHz);

	if (redraw || (LCD_last_showed_freq_MHz != MHz)) {
		LCDDriver_printTextFont(LCD_freq_string_MHz, MHz_x_offset, LAYOUT->FREQ_Y_BASELINE, COLOR->FREQ_MHZ, BG_COLOR, LAYOUT->FREQ_FONT);
		LCD_last_showed_freq_MHz = MHz;
	}

	char buff[50] = "";
	if (redraw || (LCD_last_showed_freq_kHz != kHz)) {
		addSymbols(buff, LCD_freq_string_kHz, 3, "0", false);
		LCDDriver_printTextFont(buff, LAYOUT->FREQ_X_OFFSET_KHZ, LAYOUT->FREQ_Y_BASELINE, COLOR->FREQ_KHZ, BG_COLOR, LAYOUT->FREQ_FONT);
		LCD_last_showed_freq_kHz = kHz;
	}
	if (redraw || (LCD_last_showed_freq_Hz != Hz) || TRX.ChannelMode) {
		addSymbols(buff, LCD_freq_string_Hz, 3, "0", false);
		int_fast8_t band = -1;
		int_fast8_t channel = -1;

		uint16_t Hz_color = COLOR->FREQ_HZ;
		if (TRX.ENC2_func_mode == ENC_FUNC_FAST_STEP) {
			Hz_color = COLOR->ACTIVE_BORDER;
		}

		if (TRX.ChannelMode) {
			band = getBandFromFreq(display_freq, false);
			if (!TRX.selected_vfo) {
				channel = getChannelbyFreq(display_freq, false);
			} else {
				channel = getChannelbyFreq(display_freq, true);
			}
		}
		if (TRX.ChannelMode && band >= 0 && BANDS[band].channelsCount > 0) {
			if (band != -1 && channel != -1 && strlen((char *)BANDS[band].channels[channel].subname) > 0) {
				sprintf(buff, "%s", (char *)BANDS[band].channels[channel].subname);
			} else {
				sprintf(buff, "CH");
			}
			addSymbols(buff, buff, 2, " ", false);
			LCDDriver_printText(buff, LAYOUT->FREQ_X_OFFSET_HZ + 2, LAYOUT->FREQ_Y_BASELINE_SMALL - RASTR_FONT_H * 2, COLOR->FREQ_HZ, BG_COLOR, 2);

			if (band != -1 && channel != -1) {
				sprintf(buff, "%d", BANDS[band].channels[channel].number);
			} else {
				sprintf(buff, "-");
			}
			addSymbols(buff, buff, 2, " ", true);
			LCDDriver_printTextFont(buff, LAYOUT->FREQ_X_OFFSET_HZ + 2 + RASTR_FONT_W * 2 * 2, LAYOUT->FREQ_Y_BASELINE_SMALL, COLOR->STATUS_MODE, BG_COLOR, LAYOUT->FREQ_CH_FONT);
		} else {
			LCDDriver_printTextFont(buff, LAYOUT->FREQ_X_OFFSET_HZ, LAYOUT->FREQ_Y_BASELINE_SMALL, Hz_color, BG_COLOR, LAYOUT->FREQ_SMALL_FONT);
		}
		LCD_last_showed_freq_Hz = Hz;
	}

	if (redraw) {
		// Frequency delimiters
		LCDDriver_printTextFont(".", LAYOUT->FREQ_DELIMITER_X1_OFFSET, LAYOUT->FREQ_Y_BASELINE + LAYOUT->FREQ_DELIMITER_Y_OFFSET, COLOR->FREQ_KHZ, BG_COLOR, LAYOUT->FREQ_FONT);
		LCDDriver_printTextFont(".", LAYOUT->FREQ_DELIMITER_X2_OFFSET, LAYOUT->FREQ_Y_BASELINE + LAYOUT->FREQ_DELIMITER_Y_OFFSET, COLOR->FREQ_HZ, BG_COLOR, LAYOUT->FREQ_FONT);
	}

	NeedSaveSettings = true;

	LCD_UpdateQuery.FreqInfo = false;
	if (redraw) {
		LCD_UpdateQuery.FreqInfoRedraw = false;
	}

	LCD_busy = false;
}

static void LCD_drawSMeter(void) {
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
	// LCDDriver_printText("+20", LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 11.0f) - 10, LAYOUT->STATUS_Y_OFFSET +
	// LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, COLOR->STATUS_BAR_LABELS, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	LCDDriver_printText("+40", LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 13.0f) - 10, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y,
	                    COLOR->STATUS_BAR_LABELS, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	// LCDDriver_printText("+60", LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 15.0f) - 10, LAYOUT->STATUS_Y_OFFSET +
	// LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, COLOR->STATUS_BAR_LABELS, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	for (uint8_t i = 0; i <= 15; i++) {
		uint16_t color = COLOR->STATUS_BAR_LEFT;
		if (i >= 9) {
			color = COLOR->STATUS_BAR_RIGHT;
		}
		if ((i % 2) != 0 || i == 0) {
			// LCDDriver_drawFastVLine(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * i) - 1, LAYOUT->STATUS_Y_OFFSET +
			// LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET, -4, color);
			LCDDriver_drawFastVLine(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * i), LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET, -3, color);
			// LCDDriver_drawFastVLine(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * i) + 1, LAYOUT->STATUS_Y_OFFSET +
			// LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET, -4, color);
		} else {
			LCDDriver_drawFastVLine(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * i), LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET, -1, color);
		}
	}

	// S-meter frame
	LCDDriver_drawFastHLine(LAYOUT->STATUS_BAR_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET, (uint16_t)(step * 9.0f) - 1,
	                        COLOR->STATUS_BAR_LEFT);
	LCDDriver_drawFastHLine(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 9.0f), LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET,
	                        LAYOUT->STATUS_SMETER_WIDTH - (step * 9.0f) + 1, COLOR->STATUS_BAR_RIGHT);

	LCDDriver_drawFastHLine(LAYOUT->STATUS_BAR_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + LAYOUT->STATUS_BAR_HEIGHT,
	                        (uint16_t)(step * 9.0f) - 1, COLOR->STATUS_BAR_LEFT);
	LCDDriver_drawFastHLine(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 9.0f),
	                        LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + LAYOUT->STATUS_BAR_HEIGHT,
	                        LAYOUT->STATUS_SMETER_WIDTH - (step * 9.0f) + 1, COLOR->STATUS_BAR_RIGHT);
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
	}

	if (TRX_on_TX) {
		if (TRX_Tune) {
			LCDDriver_printText("TU", LAYOUT->STATUS_TXRX_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_TXRX_Y_OFFSET), COLOR->STATUS_TU, BG_COLOR, 1);
		} else {
			LCDDriver_printText("TX", LAYOUT->STATUS_TXRX_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_TXRX_Y_OFFSET), COLOR->STATUS_TX, BG_COLOR, 1);
		}

		// frame of the SWR meter
		const float32_t step = LAYOUT->STATUS_PMETER_WIDTH / 16.0f;
		LCDDriver_drawRectXY(LAYOUT->STATUS_BAR_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET,
		                     LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 9.0f), LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 0,
		                     COLOR->STATUS_BAR_LEFT);
		LCDDriver_drawRectXY(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 9.0f), LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET,
		                     LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_PMETER_WIDTH, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 0,
		                     COLOR->STATUS_BAR_RIGHT);
		LCDDriver_drawRectXY(LAYOUT->STATUS_BAR_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + LAYOUT->STATUS_BAR_HEIGHT,
		                     LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 9.0f),
		                     LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + LAYOUT->STATUS_BAR_HEIGHT + 0, COLOR->STATUS_BAR_LEFT);
		LCDDriver_drawRectXY(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 9.0f),
		                     LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + LAYOUT->STATUS_BAR_HEIGHT,
		                     LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_PMETER_WIDTH,
		                     LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + LAYOUT->STATUS_BAR_HEIGHT + 0, COLOR->STATUS_BAR_RIGHT);

		LCDDriver_printText("SWR", LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_TX_LABELS_OFFSET_X, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y,
		                    COLOR->STATUS_LABELS_TX, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
		LCDDriver_printText("PWR:", LAYOUT->STATUS_TX_LABELS_FWD_X, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, COLOR->STATUS_LABELS_TX,
		                    BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
		// LCDDriver_printText("REF:", LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_TX_LABELS_OFFSET_X + LAYOUT->STATUS_TX_LABELS_MARGIN_X * 2,
		// LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, COLOR->STATUS_LABELS_TX, BG_COLOR,
		// LAYOUT->STATUS_LABELS_FONT_SIZE);

		// frame of the ALC meter
		LCDDriver_drawRectXY(LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_PMETER_WIDTH + LAYOUT->STATUS_ALC_BAR_X_OFFSET,
		                     LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET,
		                     LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_PMETER_WIDTH + LAYOUT->STATUS_ALC_BAR_X_OFFSET + LAYOUT->STATUS_AMETER_WIDTH,
		                     LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 0, COLOR->STATUS_BAR_LEFT);
		LCDDriver_drawRectXY(LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_PMETER_WIDTH + LAYOUT->STATUS_ALC_BAR_X_OFFSET,
		                     LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + LAYOUT->STATUS_BAR_HEIGHT,
		                     LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_PMETER_WIDTH + LAYOUT->STATUS_ALC_BAR_X_OFFSET + LAYOUT->STATUS_AMETER_WIDTH,
		                     LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + LAYOUT->STATUS_BAR_HEIGHT + 0, COLOR->STATUS_BAR_LEFT);
		LCDDriver_printText("ALC:", LAYOUT->STATUS_TX_LABELS_ALC_X, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, COLOR->STATUS_LABELS_TX,
		                    BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	} else {
		LCD_UpdateQuery.StatusInfoBar = true;
		LCDDriver_printText("RX", LAYOUT->STATUS_TXRX_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_TXRX_Y_OFFSET), COLOR->STATUS_RX, BG_COLOR, 1);
	}

	// VFO indicator
	if (!TRX.selected_vfo) // VFO-A
	{
		if (!TRX.Dual_RX) {
			printInfo(LAYOUT->STATUS_VFO_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_VFO_Y_OFFSET), LAYOUT->STATUS_VFO_BLOCK_WIDTH, LAYOUT->STATUS_VFO_BLOCK_HEIGHT, "A",
			          COLOR->STATUS_VFO_BG, COLOR->STATUS_VFO, COLOR->STATUS_VFO, true, false, true);
		} else if (TRX.Dual_RX_Type == VFO_A_AND_B) {
			printInfo(LAYOUT->STATUS_VFO_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_VFO_Y_OFFSET), LAYOUT->STATUS_VFO_BLOCK_WIDTH, LAYOUT->STATUS_VFO_BLOCK_HEIGHT, "A&B",
			          COLOR->STATUS_VFO_BG, COLOR->STATUS_VFO, COLOR->STATUS_VFO, true, false, true);
		} else {
			printInfo(LAYOUT->STATUS_VFO_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_VFO_Y_OFFSET), LAYOUT->STATUS_VFO_BLOCK_WIDTH, LAYOUT->STATUS_VFO_BLOCK_HEIGHT, "A+B",
			          COLOR->STATUS_VFO_BG, COLOR->STATUS_VFO, COLOR->STATUS_VFO, true, false, true);
		}
	} else // VFO-B
	{
		if (!TRX.Dual_RX) {
			printInfo(LAYOUT->STATUS_VFO_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_VFO_Y_OFFSET), LAYOUT->STATUS_VFO_BLOCK_WIDTH, LAYOUT->STATUS_VFO_BLOCK_HEIGHT, "B",
			          COLOR->STATUS_VFO_BG, COLOR->STATUS_VFO, COLOR->STATUS_VFO, true, false, true);
		} else if (TRX.Dual_RX_Type == VFO_A_AND_B) {
			printInfo(LAYOUT->STATUS_VFO_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_VFO_Y_OFFSET), LAYOUT->STATUS_VFO_BLOCK_WIDTH, LAYOUT->STATUS_VFO_BLOCK_HEIGHT, "B&A",
			          COLOR->STATUS_VFO_BG, COLOR->STATUS_VFO, COLOR->STATUS_VFO, true, false, true);
		} else {
			printInfo(LAYOUT->STATUS_VFO_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_VFO_Y_OFFSET), LAYOUT->STATUS_VFO_BLOCK_WIDTH, LAYOUT->STATUS_VFO_BLOCK_HEIGHT, "B+A",
			          COLOR->STATUS_VFO_BG, COLOR->STATUS_VFO, COLOR->STATUS_VFO, true, false, true);
		}
	}

	// Mode indicator
	printInfo(LAYOUT->STATUS_MODE_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_MODE_Y_OFFSET), LAYOUT->STATUS_MODE_BLOCK_WIDTH, LAYOUT->STATUS_MODE_BLOCK_HEIGHT,
	          (char *)MODE_DESCR[CurrentVFO->Mode], BG_COLOR, COLOR->STATUS_MODE, COLOR->STATUS_MODE, true, false, true);

	// Redraw TextBar
	if (NeedProcessDecoder) {
		LCDDriver_Fill_RectWH(0, LCD_HEIGHT - LAYOUT->FFT_CWDECODER_OFFSET - LAYOUT->BOTTOM_BUTTONS_BLOCK_HEIGHT, LAYOUT->FFT_PRINT_SIZE, LAYOUT->FFT_CWDECODER_OFFSET, BG_COLOR);
		LCD_UpdateQuery.TextBar = true;
	}

	LCD_UpdateQuery.StatusInfoGUI = false;
	if (redraw) {
		LCD_UpdateQuery.StatusInfoGUIRedraw = false;
	}
	LCD_busy = false;
}

static float32_t LCD_GetSMeterValPosition(float32_t dBm, bool correct_vhf) {
	int32_t width = LAYOUT->STATUS_SMETER_WIDTH - 2;
	float32_t TRX_s_meter = 0;

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

	return TRX_s_meter;
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
		LCD_UpdateQuery.StatusInfoBar = true;
		return;
	}
	LCD_busy = true;
	char ctmp[50];

	if (!TRX_on_TX) {
		static float32_t TRX_RX1_dBm_lowrate = 0;
		static uint32_t TRX_RX1_dBm_lowrate_time = 0;
		if ((HAL_GetTick() - TRX_RX1_dBm_lowrate_time) > 100) {
			TRX_RX1_dBm_lowrate_time = HAL_GetTick();
			TRX_RX1_dBm_lowrate = TRX_RX1_dBm_lowrate * 0.5f + TRX_RX1_dBm * 0.5f;
		}

		float32_t s_width = 0.0f;

		if (CurrentVFO->Mode == TRX_MODE_CW) {
			s_width = LCD_last_s_meter * 0.5f + LCD_GetSMeterValPosition(TRX_RX1_dBm_lowrate, true) * 0.5f; // smooth CW faster!
		} else {
			s_width = LCD_last_s_meter * 0.75f + LCD_GetSMeterValPosition(TRX_RX1_dBm_lowrate, true) * 0.25f; // smooth the movement of the S-meter
		}

		// digital s-meter version
		static uint32_t last_s_meter_draw_time = 0;
		if (redraw || (LCD_last_s_meter != s_width) || (HAL_GetTick() - last_s_meter_draw_time) > 500) {
			last_s_meter_draw_time = HAL_GetTick();
			// clear old bar
			if ((LCD_last_s_meter - s_width) > 0) {
				LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)s_width, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 2,
				                      (uint16_t)(LCD_last_s_meter - s_width + 1), LAYOUT->STATUS_BAR_HEIGHT - 3, BG_COLOR);
			}
			// and stripe
			LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)LCD_last_s_meter, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + 5, 1, LAYOUT->STATUS_SMETER_MARKER_HEIGHT,
			                      BG_COLOR);
			LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)s_width, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + 5, 1, LAYOUT->STATUS_SMETER_MARKER_HEIGHT,
			                      COLOR->STATUS_SMETER_STRIPE);

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
			static uint16_t smeter_peak_x = 0;
			static uint32_t smeter_peak_settime = 0;
			if (smeter_peak_x > s_width) {
				LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + smeter_peak_x, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 2, 1,
				                      LAYOUT->STATUS_BAR_HEIGHT - 3, BG_COLOR); // clear old peak
			}
			if (smeter_peak_x > 0 && ((HAL_GetTick() - smeter_peak_settime) > LAYOUT->STATUS_SMETER_PEAK_HOLDTIME)) {
				smeter_peak_x--;
			}
			if (s_width > smeter_peak_x) {
				smeter_peak_x = (uint16_t)s_width;
				smeter_peak_settime = HAL_GetTick();
			}
			LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + smeter_peak_x, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 2, 1,
			                      LAYOUT->STATUS_BAR_HEIGHT - 3, COLOR->STATUS_SMETER_PEAK);

			// FM Squelch stripe
			if (CurrentVFO->Mode == TRX_MODE_NFM || CurrentVFO->Mode == TRX_MODE_WFM) {
				uint16_t x_pos = LCD_GetSMeterValPosition(CurrentVFO->FM_SQL_threshold_dBm, true);
				LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + x_pos, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 2, 1,
				                      LAYOUT->STATUS_BAR_HEIGHT - 3, COLOR->STATUS_SMETER_PEAK);
			}

			// redraw s-meter gui and stripe
			LCD_drawSMeter();
			LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)s_width, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + 5, 1, LAYOUT->STATUS_SMETER_MARKER_HEIGHT,
			                      COLOR->STATUS_SMETER_STRIPE);

			LCD_last_s_meter = s_width;
		}

		// print dBm value
		static float32_t TRX_RX_dBm_averaging = -120.0f;
		TRX_RX_dBm_averaging = 0.97f * TRX_RX_dBm_averaging + 0.03f * TRX_RX1_dBm_lowrate;
		if (TRX_RX1_dBm_lowrate > TRX_RX_dBm_averaging) {
			TRX_RX_dBm_averaging = TRX_RX1_dBm_lowrate;
		}

		sprintf(ctmp, "%d", (int16_t)TRX_RX_dBm_averaging);
		addSymbols(ctmp, ctmp, 4, " ", true);
		LCDDriver_printText(ctmp, LAYOUT->STATUS_LABEL_DBM_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABEL_DBM_Y_OFFSET, COLOR->STATUS_LABEL_DBM, BG_COLOR,
		                    LAYOUT->STATUS_LABELS_FONT_SIZE);

		// print s-meter value
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
				sprintf(ctmp, "+5");
			} else if (TRX_RX_dBm_averaging <= -58.0f) {
				sprintf(ctmp, "+10");
			} else if (TRX_RX_dBm_averaging <= -53.0f) {
				sprintf(ctmp, "+15");
			} else if (TRX_RX_dBm_averaging <= -48.0f) {
				sprintf(ctmp, "+20");
			} else if (TRX_RX_dBm_averaging <= -43.0f) {
				sprintf(ctmp, "+25");
			} else if (TRX_RX_dBm_averaging <= -38.0f) {
				sprintf(ctmp, "+30");
			} else if (TRX_RX_dBm_averaging <= -33.0f) {
				sprintf(ctmp, "+35");
			} else if (TRX_RX_dBm_averaging <= -28.0f) {
				sprintf(ctmp, "+40");
			} else if (TRX_RX_dBm_averaging <= -23.0f) {
				sprintf(ctmp, "+45");
			} else if (TRX_RX_dBm_averaging <= -18.0f) {
				sprintf(ctmp, "+50");
			} else if (TRX_RX_dBm_averaging <= -13.0f) {
				sprintf(ctmp, "+55");
			} else {
				sprintf(ctmp, "+60");
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
				sprintf(ctmp, "+5");
			} else if (TRX_RX_dBm_averaging <= -78.0f) {
				sprintf(ctmp, "+10");
			} else if (TRX_RX_dBm_averaging <= -73.0f) {
				sprintf(ctmp, "+15");
			} else if (TRX_RX_dBm_averaging <= -68.0f) {
				sprintf(ctmp, "+20");
			} else if (TRX_RX_dBm_averaging <= -63.0f) {
				sprintf(ctmp, "+25");
			} else if (TRX_RX_dBm_averaging <= -58.0f) {
				sprintf(ctmp, "+30");
			} else if (TRX_RX_dBm_averaging <= -53.0f) {
				sprintf(ctmp, "+35");
			} else if (TRX_RX_dBm_averaging <= -48.0f) {
				sprintf(ctmp, "+40");
			} else if (TRX_RX_dBm_averaging <= -43.0f) {
				sprintf(ctmp, "+45");
			} else if (TRX_RX_dBm_averaging <= -38.0f) {
				sprintf(ctmp, "+50");
			} else if (TRX_RX_dBm_averaging <= -33.0f) {
				sprintf(ctmp, "+55");
			} else {
				sprintf(ctmp, "+60");
			}
		}

		addSymbols(ctmp, ctmp, 3, " ", true);
		LCDDriver_printText(ctmp, LAYOUT->STATUS_LABEL_S_VAL_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABEL_S_VAL_Y_OFFSET, COLOR->STATUS_LABEL_S_VAL, BG_COLOR, 1);
	} else {
		// SWR
		sprintf(ctmp, "%.1f", (double)TRX_SWR_SMOOTHED);
		LCDDriver_printText(ctmp, LAYOUT->STATUS_TX_LABELS_SWR_X, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y + 9, COLOR_RED, BG_COLOR,
		                    LAYOUT->STATUS_LABELS_FONT_SIZE);

		// FWD
		float32_t TRX_Power = TRX_PWR_Forward_SMOOTHED - TRX_PWR_Backward_SMOOTHED;
		if (TRX_Power >= 99.0f) {
			sprintf(ctmp, "%d", (uint16_t)TRX_Power);
		} else {
			sprintf(ctmp, "%dW", (uint16_t)TRX_Power);
		}
		addSymbols(ctmp, ctmp, 3, " ", true);
		LCDDriver_printText(ctmp, LAYOUT->STATUS_TX_LABELS_FWD_X + 5 * RASTR_FONT_W, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, COLOR_RED,
		                    BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);

		// REF
		/*if(TRX_PWR_Backward_SMOOTHED >= 100.0f)
		  sprintf(ctmp, "%dW", (uint16_t)TRX_PWR_Backward_SMOOTHED);
		else if(TRX_PWR_Backward >= 9.5f)
		  sprintf(ctmp, "%dW", (uint16_t)TRX_PWR_Backward_SMOOTHED);
		else
		  sprintf(ctmp, "%dW ", (uint16_t)TRX_PWR_Backward_SMOOTHED);
		LCDDriver_printText(ctmp, LAYOUT->STATUS_TX_LABELS_REF_X, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET +
		LAYOUT->STATUS_LABELS_OFFSET_Y, COLOR_RED, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);*/

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
		uint8_t alc_level = (uint8_t)(TRX_ALC_OUT * 100.0f);
		sprintf(ctmp, "%d%%", alc_level);
		addSymbols(ctmp, ctmp, 4, " ", true);
		LCDDriver_printText(ctmp, LAYOUT->STATUS_TX_LABELS_ALC_X + 5 * RASTR_FONT_W, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y,
		                    COLOR->STATUS_BAR_LABELS, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
		uint16_t alc_level_width = LAYOUT->STATUS_AMETER_WIDTH * alc_level / 100;
		if (alc_level_width > LAYOUT->STATUS_AMETER_WIDTH) {
			alc_level_width = LAYOUT->STATUS_AMETER_WIDTH;
		}
		LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_PMETER_WIDTH + LAYOUT->STATUS_ALC_BAR_X_OFFSET,
		                      LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 2, alc_level_width, LAYOUT->STATUS_BAR_HEIGHT - 3, COLOR->STATUS_SMETER);
		if (alc_level < 100) {
			LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_PMETER_WIDTH + LAYOUT->STATUS_ALC_BAR_X_OFFSET + alc_level_width,
			                      LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 2, LAYOUT->STATUS_AMETER_WIDTH - alc_level_width,
			                      LAYOUT->STATUS_BAR_HEIGHT - 3, COLOR->STATUS_LABEL_NOTCH);
		}
	}

	// Info labels
	char buff[32] = "";
	// BW HPF-LPF
	float32_t lpf_width = TRX_on_TX ? CurrentVFO->LPF_TX_Filter_Width : CurrentVFO->LPF_RX_Filter_Width;
	if (lpf_width == 0) {
		sprintf(buff, "FULL");
	} else {
		if (lpf_width < 1000) {
			sprintf(buff, "%dHz", (uint32_t)lpf_width);
		} else {
			lpf_width /= 1000;
			sprintf(buff, "%.1fkHz", (float64_t)lpf_width);
		}
	}

	addSymbols(buff, buff, 7, " ", true);
	LCDDriver_printText(buff, LAYOUT->STATUS_LABEL_BW_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABEL_BW_Y_OFFSET, COLOR->STATUS_LABEL_BW, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	// RIT
	uint16_t color = COLOR->STATUS_LABEL_RIT;
	if (TRX.SPLIT_Enabled) {
		sprintf(buff, "SPLT");
	} else if ((!TRX.XIT_Enabled || !TRX_on_TX) && TRX.RIT_Enabled) {
		if (TRX_RIT == 0) {
			sprintf(buff, "RIT");
		} else if (TRX_RIT > 0) {
			sprintf(buff, "+%d", TRX_RIT);
		} else {
			sprintf(buff, "%d", TRX_RIT);
		}
	} else if ((!TRX.RIT_Enabled || TRX_on_TX) && TRX.XIT_Enabled) {
		if (TRX_XIT == 0) {
			sprintf(buff, "XIT");
		} else if (TRX_XIT > 0) {
			sprintf(buff, "+%d", TRX_XIT);
		} else {
			sprintf(buff, "%d", TRX_XIT);
		}
	} else {
		sprintf(buff, "RIT");
		color = COLOR->BUTTON_INACTIVE_TEXT;
	}
	addSymbols(buff, buff, 4, " ", true);
	LCDDriver_printText(buff, LAYOUT->STATUS_LABEL_RIT_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABEL_RIT_Y_OFFSET, color, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	// VOLTS / CURRENTS
	if (!TRX_on_TX) {
		if (!TRX_X1_VLT_CUR_Mode) {
			sprintf(buff, "%.1fV", (float64_t)TRX_PWR_Voltage);
		} else {
			sprintf(buff, "%.1fA", (float64_t)TRX_PWR_Current);
		}
		addSymbols(buff, buff, 5, " ", true);
		LCDDriver_printText(buff, LAYOUT->STATUS_LABEL_THERM_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABEL_THERM_Y_OFFSET, COLOR->STATUS_LABEL_THERM, BG_COLOR,
		                    LAYOUT->STATUS_LABELS_FONT_SIZE);
		// NOTCH
		color = COLOR->STATUS_LABEL_NOTCH;

		if (CurrentVFO->AutoNotchFilter) {
			sprintf(buff, "ANTCH");
		} else if (CurrentVFO->ManualNotchFilter && CurrentVFO->NotchFC < 1000) {
			sprintf(buff, "%uHz", CurrentVFO->NotchFC);
		} else if (CurrentVFO->ManualNotchFilter && CurrentVFO->NotchFC >= 1000) {
			sprintf(buff, "%u", CurrentVFO->NotchFC);
		} else {
			sprintf(buff, "NOTCH");
			color = COLOR->BUTTON_INACTIVE_TEXT;
		}

		if (TRX.ENC2_func_mode == ENC_FUNC_SET_NOTCH) {
			color = COLOR->ACTIVE_BORDER;

			if (!CurrentVFO->ManualNotchFilter) {
				sprintf(buff, "%u%%", TRX.Volume);
			}

			// if(CurrentVFO->Mode == TRX_MODE_CW)
			// sprintf(buff, "%uWPM", TRX.CW_KEYER_WPM);
		}

		addSymbols(buff, buff, 5, " ", true);
		LCDDriver_printText(buff, LAYOUT->STATUS_LABEL_NOTCH_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABEL_NOTCH_Y_OFFSET, color, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	} else // on TX -> VLT+CUR
	{
		sprintf(buff, "%.1fV", (float64_t)TRX_PWR_Voltage);
		addSymbols(buff, buff, 5, " ", true);
		LCDDriver_printText(buff, LAYOUT->STATUS_LABEL_THERM_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABEL_THERM_Y_OFFSET, COLOR->STATUS_LABEL_THERM, BG_COLOR,
		                    LAYOUT->STATUS_LABELS_FONT_SIZE);

		sprintf(buff, "%.1fA", (float64_t)TRX_PWR_Current);
		addSymbols(buff, buff, 5, " ", true);
		LCDDriver_printText(buff, LAYOUT->STATUS_LABEL_NOTCH_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABEL_NOTCH_Y_OFFSET, COLOR->STATUS_LABEL_THERM, BG_COLOR,
		                    LAYOUT->STATUS_LABELS_FONT_SIZE);
	}

	// FFT BW
	uint8_t fft_zoom = TRX.FFT_Zoom;
	if (CurrentVFO->Mode == TRX_MODE_CW) {
		fft_zoom = TRX.FFT_ZoomCW;
	}
	uint16_t fft_width = FFT_current_spectrum_width_Hz / 1000;
	if (fft_width < 100) {
		sprintf(buff, "%dk", fft_width);
	} else {
		sprintf(buff, "%d", fft_width);
	}
	addSymbols(buff, buff, 3, " ", true);
	LCDDriver_printText(buff, LAYOUT->STATUS_LABEL_FFT_BW_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABEL_FFT_BW_Y_OFFSET, COLOR->STATUS_LABELS_BW, BG_COLOR,
	                    LAYOUT->STATUS_LABELS_FONT_SIZE);

// ERRORS LABELS
#define CLEAN_SPACE LCDDriver_Fill_RectWH(LAYOUT->STATUS_ERR_OFFSET_X, LAYOUT->STATUS_ERR_OFFSET_Y, LAYOUT->STATUS_ERR_WIDTH, LAYOUT->STATUS_ERR_HEIGHT, BG_COLOR);
	if (TRX_ADC_MAXAMPLITUDE > (ADC_FULL_SCALE * 0.499f) || TRX_ADC_MINAMPLITUDE < -(ADC_FULL_SCALE * 0.499f)) {
		if (ADCDAC_OVR_StatusLatency >= 10) {
			TRX_ADC_OTR = true;
		}
	}

	if (TRX_ADC_OTR && !TRX_on_TX && !TRX.ADC_SHDN) {
		CLEAN_SPACE
		LCDDriver_printText("OVR", LAYOUT->STATUS_ERR_OFFSET_X, LAYOUT->STATUS_ERR_OFFSET_Y, COLOR->STATUS_ERR, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	} else if (APROC_IFGain_Overflow) {
		CLEAN_SPACE
		LCDDriver_printText("IFO", LAYOUT->STATUS_ERR_OFFSET_X, LAYOUT->STATUS_ERR_OFFSET_Y, COLOR->STATUS_ERR, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	} else if (TRX_PWR_ALC_SWR_OVERFLOW) {
		CLEAN_SPACE
		LCDDriver_printText("OVS", LAYOUT->STATUS_ERR_OFFSET_X, LAYOUT->STATUS_ERR_OFFSET_Y, COLOR->STATUS_ERR, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	} else if (TRX_DAC_OTR) {
		CLEAN_SPACE
		LCDDriver_printText("OVR", LAYOUT->STATUS_ERR_OFFSET_X, LAYOUT->STATUS_ERR_OFFSET_Y, COLOR->STATUS_ERR, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	} else if (CODEC_Buffer_underrun && !TRX_on_TX) {
		CLEAN_SPACE
		LCDDriver_printText("WBF", LAYOUT->STATUS_ERR_OFFSET_X, LAYOUT->STATUS_ERR_OFFSET_Y, COLOR->STATUS_ERR, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	} else if (FPGA_Buffer_underrun && TRX_on_TX) {
		CLEAN_SPACE
		LCDDriver_printText("FBF", LAYOUT->STATUS_ERR_OFFSET_X, LAYOUT->STATUS_ERR_OFFSET_Y, COLOR->STATUS_ERR, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	} else if (RX_USB_AUDIO_underrun) {
		CLEAN_SPACE
		LCDDriver_printText("UBF", LAYOUT->STATUS_ERR_OFFSET_X, LAYOUT->STATUS_ERR_OFFSET_Y, COLOR->STATUS_ERR, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	} else if (SD_underrun) {
		CLEAN_SPACE
		LCDDriver_printText("SDF", LAYOUT->STATUS_ERR_OFFSET_X, LAYOUT->STATUS_ERR_OFFSET_Y, COLOR->STATUS_ERR, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	} else {
		RTC_TimeTypeDef sTime = {0};
		RTC_DateTypeDef sDate = {0};
		getLocalDateTime(&sDate, &sTime);
		Hours = sTime.Hours;
		Minutes = sTime.Minutes;
		Seconds = sTime.Seconds;

		// if (redraw || (Hours != Last_showed_Hours))
		{
			sprintf(ctmp, "%d", Hours);
			addSymbols(ctmp, ctmp, 2, "0", false);
			LCDDriver_printText(ctmp, LAYOUT->CLOCK_POS_HRS_X, LAYOUT->CLOCK_POS_Y, COLOR->CLOCK, BG_COLOR, 1);
			LCDDriver_printText(":", LCDDriver_GetCurrentXOffset(), LAYOUT->CLOCK_POS_Y, COLOR->CLOCK, BG_COLOR, 1);
			Last_showed_Hours = Hours;
		}
		// if (redraw || (Minutes != Last_showed_Minutes))
		{
			sprintf(ctmp, "%d", Minutes);
			addSymbols(ctmp, ctmp, 2, "0", false);
			LCDDriver_printText(ctmp, LAYOUT->CLOCK_POS_MIN_X, LAYOUT->CLOCK_POS_Y, COLOR->CLOCK, BG_COLOR, 1);
			// LCDDriver_printText(":", LCDDriver_GetCurrentXOffset(), LAYOUT->CLOCK_POS_Y, COLOR->CLOCK, BG_COLOR, 1);
			Last_showed_Minutes = Minutes;
		}
		/*if (redraw || (Seconds != Last_showed_Seconds))
		{
		  sprintf(ctmp, "%d", Seconds);
		  addSymbols(ctmp, ctmp, 2, "0", false);
		  LCDDriver_printText(ctmp, LAYOUT->CLOCK_POS_SEC_X, LAYOUT->CLOCK_POS_Y, COLOR->CLOCK, BG_COLOR, 1);
		  Last_showed_Seconds = Seconds;
		}*/
	}

	LCD_UpdateQuery.StatusInfoBar = false;
	if (redraw) {
		LCD_UpdateQuery.StatusInfoBarRedraw = false;
	}
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
		LCDDriver_printText(ctmp, 2, (LCD_HEIGHT - LAYOUT->BOTTOM_BUTTONS_BLOCK_HEIGHT - LAYOUT->FFT_CWDECODER_OFFSET + 1), COLOR->CLOCK, BG_COLOR, LAYOUT->TEXTBAR_FONT);
	} else if (NeedProcessDecoder && CurrentVFO->Mode == TRX_MODE_WFM) {
		LCDDriver_printText(RDS_Decoder_Text, 2, (LCD_HEIGHT - LAYOUT->BOTTOM_BUTTONS_BLOCK_HEIGHT - LAYOUT->FFT_CWDECODER_OFFSET + 1), COLOR->CLOCK, BG_COLOR, LAYOUT->TEXTBAR_FONT);
	} else if (NeedProcessDecoder && CurrentVFO->Mode == TRX_MODE_RTTY) {
		LCDDriver_printText(RTTY_Decoder_Text, 2, (LCD_HEIGHT - LAYOUT->BOTTOM_BUTTONS_BLOCK_HEIGHT - LAYOUT->FFT_CWDECODER_OFFSET + 1), COLOR->CLOCK, BG_COLOR, LAYOUT->TEXTBAR_FONT);
	}

	LCD_UpdateQuery.TextBar = false;
	LCD_busy = false;
}

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
	if (LCD_UpdateQuery.BottomButtons) {
		LCD_displayBottomButtons(false);
	}
	if (LCD_UpdateQuery.BottomButtonsRedraw) {
		LCD_displayBottomButtons(true);
	}
	if (LCD_UpdateQuery.TopButtons) {
		LCD_displayTopButtons(false);
	}
	if (LCD_UpdateQuery.TopButtonsRedraw) {
		LCD_displayTopButtons(true);
	}
	if (LCD_UpdateQuery.FreqInfo) {
		LCD_displayFreqInfo(false);
	}
	if (LCD_UpdateQuery.FreqInfoRedraw) {
		LCD_displayFreqInfo(true);
	}
	if (LCD_UpdateQuery.StatusInfoGUIRedraw) {
		LCD_displayStatusInfoGUI(true);
	}
	if (LCD_UpdateQuery.StatusInfoGUI) {
		LCD_displayStatusInfoGUI(false);
	}
	if (LCD_UpdateQuery.StatusInfoBar) {
		LCD_displayStatusInfoBar(false);
	}
	if (LCD_UpdateQuery.StatusInfoBarRedraw) {
		LCD_displayStatusInfoBar(true);
	}
	if (LCD_UpdateQuery.SystemMenu) {
		SYSMENU_drawSystemMenu(false, false);
	}
	if (LCD_UpdateQuery.SystemMenuInfolines) {
		SYSMENU_drawSystemMenu(false, true);
	}
	if (LCD_UpdateQuery.SystemMenuRedraw) {
		SYSMENU_drawSystemMenu(true, false);
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

static void printInfo(uint16_t x, uint16_t y, uint16_t width, uint16_t height, char *text, uint16_t back_color, uint16_t text_color, uint16_t inactive_color, bool active, bool border,
                      bool full_redraw) {
	uint16_t x1, y1, w, h;
	if (full_redraw) {
		LCDDriver_Fill_RectWH(x, y, width, height, back_color);
		if (border) {
			LCDDriver_drawFastHLine(x, y, width, COLOR->ACTIVE_BORDER);
			LCDDriver_drawFastHLine(x, y + height - 1, width, COLOR->ACTIVE_BORDER);

			LCDDriver_drawFastVLine(x, y, height, COLOR->ACTIVE_BORDER);
			LCDDriver_drawFastVLine(x + width, y, height, COLOR->ACTIVE_BORDER);
		}
	}
	LCDDriver_getTextBounds(text, x, y, &x1, &y1, &w, &h, 1);
	LCDDriver_printText(text, x + (width - w) / 2 + 1, y + (height / 2) - 3, active ? text_color : inactive_color, back_color, 1);
}

void LCD_showError(char text[], bool redraw) {
	LCD_busy = true;
	if (!LCD_inited) {
		LCD_Init();
	}

	LCDDriver_Fill(COLOR_RED);
	uint16_t x1, y1, w, h;
	LCDDriver_getTextBounds(text, 0, 0, &x1, &y1, &w, &h, 1);
	LCDDriver_printText(text, LCD_WIDTH / 2 - w / 2, LCD_HEIGHT / 2 - h / 2, COLOR_WHITE, COLOR_RED, 1);
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
	LCDDriver_getTextBounds(text, 0, 0, &x1, &y1, &w, &h, 1);
	LCDDriver_printText(text, LCD_WIDTH / 2 - w / 2, LCD_HEIGHT / 2 - h / 2, COLOR->CLOCK, BG_COLOR, 1);
	if (autohide) {
		HAL_Delay(2000);
		LCD_showInfo_opened = false;
		LCD_busy = false;
		LCD_redraw(false);
	}
}

void LCD_processTouch(uint16_t x, uint16_t y) {}

void LCD_processHoldTouch(uint16_t x, uint16_t y) {}

bool LCD_processSwipeTouch(uint16_t x, uint16_t y, int16_t dx, int16_t dy) { return false; }

bool LCD_processSwipeTwoFingersTouch(uint16_t x, uint16_t y, int16_t dx, int16_t dy) { return false; }

void LCD_showTooltip(char text[]) {
	Tooltip_DiplayStartTime = HAL_GetTick();
	if (LCD_UpdateQuery.Tooltip && strlen(Tooltip_string) != strlen(text)) { // redraw old tooltip
		LCD_UpdateQuery.FreqInfoRedraw = true;
	}
	Tooltip_first_draw = true;
	strcpy(Tooltip_string, text);
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
	LCDDriver_getTextBounds(Tooltip_string, LAYOUT->TOOLTIP_POS_X, LAYOUT->TOOLTIP_POS_Y, &x1, &y1, &w, &h, 1);
	if (Tooltip_first_draw) {
		LCDDriver_Fill_RectWH(LAYOUT->TOOLTIP_POS_X - w / 2, LAYOUT->TOOLTIP_POS_Y, w + LAYOUT->TOOLTIP_MARGIN * 2, h + LAYOUT->TOOLTIP_MARGIN * 2, COLOR->TOOLTIP_BACK);
		LCDDriver_drawRectXY(LAYOUT->TOOLTIP_POS_X - w / 2, LAYOUT->TOOLTIP_POS_Y, LAYOUT->TOOLTIP_POS_X - w / 2 + w + LAYOUT->TOOLTIP_MARGIN * 2,
		                     LAYOUT->TOOLTIP_POS_Y + h + LAYOUT->TOOLTIP_MARGIN * 2, COLOR->TOOLTIP_BORD);
		Tooltip_first_draw = false;
	}
	LCDDriver_printText(Tooltip_string, LAYOUT->TOOLTIP_POS_X - w / 2 + LAYOUT->TOOLTIP_MARGIN, LAYOUT->TOOLTIP_POS_Y + LAYOUT->TOOLTIP_MARGIN, COLOR->TOOLTIP_FORE, COLOR->TOOLTIP_BACK, 1);

	LCD_busy = false;
	if ((HAL_GetTick() - Tooltip_DiplayStartTime) > LAYOUT->TOOLTIP_TIMEOUT) {
		LCD_UpdateQuery.Tooltip = false;
		LCD_UpdateQuery.FreqInfoRedraw = true;
		LCD_UpdateQuery.StatusInfoGUI = true;
		LCD_UpdateQuery.StatusInfoBarRedraw = true;
	}
}

void LCD_openWindow(uint16_t w, uint16_t h) {}

void LCD_closeWindow(void) {}

static void LCD_showBandWindow(bool secondary_vfo) {}

static void LCD_showModeWindow(bool secondary_vfo) {}

static void LCD_showBWWindow(void) {}

void LCD_showRFPowerWindow(void) {}

void LCD_showManualFreqWindow(uint32_t parameter) {}

void LCD_ManualFreqButtonHandler(uint32_t parameter) {}

void LCD_printKeyboard(void (*keyboardHandler)(char *string, uint32_t max_size, char entered), char *string, uint32_t max_size, bool lowcase) {}

void LCD_hideKeyboard(void) {
	LCD_screenKeyboardOpened = false;
	LCD_keyboardHandler = NULL;
}
