#include "lcd.h"
#include "INA226_PWR_monitor.h" //Tisho
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

char MemoryChanName[MAX_CHANNEL_MEMORY_NAME_LENGTH];
uint32_t MemoryChanFreq;

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

static uint64_t manualFreqEnter = 0;
static bool LCD_screenKeyboardOpened = false;
static void (*LCD_keyboardHandler)(uint32_t parameter) = NULL;

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

static uint32_t Tooltip_DiplayStartTime = 0;
static bool Tooltip_first_draw = true;
static char Tooltip_string[64] = {0};
static bool LCD_showInfo_opened = false;

static void printInfoSmall(uint16_t x, uint16_t y, uint16_t width, uint16_t height, char *text, uint16_t back_color, uint16_t text_color, uint16_t in_active_color, bool active);
static void printInfoStatus(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t button_color, char *text, uint16_t back_color, uint16_t text_color, uint16_t inactive_color,
                            bool active);
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
static void LCD_ShowMemoryChannelsButtonHandler(uint32_t parameter);
static void printButton(uint16_t x, uint16_t y, uint16_t width, uint16_t height, char *text, bool active, bool show_lighter, bool in_window, uint32_t parameter,
                        void (*clickHandler)(uint32_t parameter), void (*holdHandler)(uint32_t parameter), uint16_t active_color, uint16_t inactive_color);

void LCD_Init(void) {
	COLOR = &COLOR_THEMES[TRX.ColorThemeId];
	LAYOUT = &LAYOUT_THEMES[TRX.LayoutThemeId];

	// DMA2D LCD addr
	// WRITE_REG(hdma2d.Instance->OMAR, LCD_FSMC_DATA_ADDR);
	// DMA2D FILL MODE
	// MODIFY_REG(hdma2d.Instance->CR, DMA2D_CR_MODE | DMA2D_CR_LOM, DMA2D_R2M | DMA2D_LOM_PIXELS);
	// DMA2D PIXEL FORMAT
	// MODIFY_REG(hdma2d.Instance->OPFCCR, DMA2D_OPFCCR_CM | DMA2D_OPFCCR_SB, DMA2D_OUTPUT_ARGB8888 | DMA2D_BYTES_REGULAR);

	LCDDriver_Init();
#if SCREEN_ROTATE
	LCDDriver_setRotation(3);
#else
	LCDDriver_setRotation(1);
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

	// display information about the operation of the transceiver
	printInfo(LAYOUT->TOPBUTTONS_X1, LAYOUT->TOPBUTTONS_PRE_Y, LAYOUT->TOPBUTTONS_WIDTH - 10, LAYOUT->TOPBUTTONS_HEIGHT, "DRV", BG_COLOR, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT,
	          TRX.ADC_Driver);
	char buff[64] = {0};
	if (TRX.ATT_DB >= 1.0f) {
		sprintf(buff, "AT%d", (uint8_t)TRX.ATT_DB);
	} else {
		sprintf(buff, "ATT");
	}
	printInfo(LAYOUT->TOPBUTTONS_ATT_X, LAYOUT->TOPBUTTONS_ATT_Y, LAYOUT->TOPBUTTONS_WIDTH + 5, LAYOUT->TOPBUTTONS_HEIGHT, buff, BG_COLOR, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT,
	          (TRX.ATT && TRX.ATT_DB >= 1.0f));
	printInfo(LAYOUT->TOPBUTTONS_PGA_X, LAYOUT->TOPBUTTONS_PGA_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "SQL", BG_COLOR, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT,
	          CurrentVFO->SQL);
	if (TRX.SPLIT_Enabled) {
		printInfo(LAYOUT->TOPBUTTONS_DRV_X, LAYOUT->TOPBUTTONS_DRV_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "SPLIT", BG_COLOR, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT,
		          TRX.SPLIT_Enabled);
	} else {
		printInfo(LAYOUT->TOPBUTTONS_DRV_X, LAYOUT->TOPBUTTONS_DRV_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "RIT", BG_COLOR, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT,
		          TRX.RIT_Enabled || TRX.XIT_Enabled);
	}
	printInfo(LAYOUT->TOPBUTTONS_AGC_X, LAYOUT->TOPBUTTONS_AGC_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "AGC", BG_COLOR, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT,
	          CurrentVFO->AGC);
	if (CurrentVFO->DNR_Type == 1) {
		printInfo(LAYOUT->TOPBUTTONS_DNR_X, LAYOUT->TOPBUTTONS_DNR_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "DNR", BG_COLOR, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT,
		          true);
	} else if (CurrentVFO->DNR_Type == 2) {
		printInfo(LAYOUT->TOPBUTTONS_DNR_X, LAYOUT->TOPBUTTONS_DNR_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "DNR2", BG_COLOR, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT,
		          true);
	} else {
		printInfo(LAYOUT->TOPBUTTONS_DNR_X, LAYOUT->TOPBUTTONS_DNR_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "DNR", BG_COLOR, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT,
		          false);
	}
	if (TRX.NOISE_BLANKER1 == false && TRX.NOISE_BLANKER2 == false) {
		printInfo(LAYOUT->TOPBUTTONS_NB_X, LAYOUT->TOPBUTTONS_NB_Y + 1, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "NB", BG_COLOR, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT,
		          false);
	}
	if (TRX.NOISE_BLANKER1 == true && TRX.NOISE_BLANKER2 == false) {
		printInfo(LAYOUT->TOPBUTTONS_NB_X, LAYOUT->TOPBUTTONS_NB_Y + 1, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "NB1", BG_COLOR, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT,
		          false);
	}
	if (TRX.NOISE_BLANKER1 == false && TRX.NOISE_BLANKER2 == true) {
		printInfo(LAYOUT->TOPBUTTONS_NB_X, LAYOUT->TOPBUTTONS_NB_Y + 1, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "NB2", BG_COLOR, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT,
		          false);
	}
	if (TRX.NOISE_BLANKER1 == true && TRX.NOISE_BLANKER2 == true) {
		printInfo(LAYOUT->TOPBUTTONS_NB_X, LAYOUT->TOPBUTTONS_NB_Y + 1, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "NB12", BG_COLOR, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT,
		          true);
	}
	if (!TRX.BluetoothAudio_Enabled) {
		printInfo(LAYOUT->TOPBUTTONS_MUTE_X, LAYOUT->TOPBUTTONS_MUTE_Y + 1, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "MUTE", BG_COLOR, COLOR_RED, COLOR->BUTTON_INACTIVE_TEXT,
		          TRX.Mute);
	} else {
		printInfo(LAYOUT->TOPBUTTONS_MUTE_X, LAYOUT->TOPBUTTONS_MUTE_Y + 1, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "BT", BG_COLOR, COLOR_RED, COLOR->BUTTON_INACTIVE_TEXT,
		          TRX.BluetoothAudio_Enabled);
	}
	if (TRX.Locked) {
		printInfoSmall(LAYOUT->TOPBUTTONS_MUTE_X + 5, LAYOUT->TOPBUTTONS_LOCK_Y + 50, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "LOCK", COLOR_RED, BG_COLOR,
		               COLOR->BUTTON_INACTIVE_TEXT, TRX.Locked);
	} else {
		printInfoSmall(LAYOUT->TOPBUTTONS_MUTE_X + 5, LAYOUT->TOPBUTTONS_LOCK_Y + 50, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "FAST", BG_COLOR, COLOR->BUTTON_TEXT,
		               COLOR->BUTTON_INACTIVE_TEXT, TRX.Fast);
	}

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

	if (redraw) {
		LCDDriver_Fill_RectWH(0, LAYOUT->BOTTOM_BUTTONS_BLOCK_TOP, LCD_WIDTH, LAYOUT->BOTTOM_BUTTONS_BLOCK_HEIGHT, BG_COLOR);
	}

	uint16_t curr_x = 0;
	for (uint8_t i = 0; i < FUNCBUTTONS_ON_PAGE; i++) {
		uint16_t menuPosition = TRX.FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + i;

		bool enabled = true;
		if (PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[menuPosition]].checkBool != NULL) {
			if ((uint8_t)*PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[menuPosition]].checkBool == 0) {
				enabled = false;
			}
		}

		bool dummyButton = false;
		if (PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[menuPosition]].clickHandler == NULL && PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[menuPosition]].holdHandler == NULL) {
			dummyButton = true;
		}

		if (!dummyButton) {
			printButton(curr_x, LAYOUT->BOTTOM_BUTTONS_BLOCK_TOP, LAYOUT->BOTTOM_BUTTONS_ONE_WIDTH, LAYOUT->BOTTOM_BUTTONS_BLOCK_HEIGHT,
			            (char *)PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[menuPosition]].name, enabled, false, false,
			            PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[menuPosition]].parameter, PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[menuPosition]].clickHandler,
			            PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[menuPosition]].holdHandler, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT);
		} else {
			printButton(curr_x, LAYOUT->BOTTOM_BUTTONS_BLOCK_TOP, LAYOUT->BOTTOM_BUTTONS_ONE_WIDTH, LAYOUT->BOTTOM_BUTTONS_BLOCK_HEIGHT, " ", false, false, false, 0, NULL, NULL,
			            COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT);
		}
		curr_x += LAYOUT->BOTTOM_BUTTONS_ONE_WIDTH;
	}

	LCD_UpdateQuery.BottomButtons = false;
	if (redraw) {
		LCD_UpdateQuery.BottomButtonsRedraw = false;
	}
	LCD_busy = false;
}
// VFO-B
static void LCD_displayFreqV2(uint64_t freqV2) {

	uint16_t Hz = (freqV2 % 1000);
	uint16_t kHz = ((freqV2 / 1000) % 1000);
	uint16_t MHz = ((freqV2 / 1000000) % 1000);

	sprintf(LCD_freq_string_Hz, "%d", Hz);
	sprintf(LCD_freq_string_kHz, "%d", kHz);
	sprintf(LCD_freq_string_MHz, "%d", MHz);

	char buff[50] = "";
	addSymbols(buff, LCD_freq_string_MHz, 3, " ", false);
	LCDDriver_printText(buff, LAYOUT->FREQ_DELIMITER_X2_OFFSET - 60, LAYOUT->FREQ_Y_BASELINE + LAYOUT->FREQ_DELIMITER_Y_OFFSET - 12, COLOR->FREQ_HZ, BG_COLOR, 2);
	LCDDriver_printText(".", LAYOUT->FREQ_DELIMITER_X2_OFFSET - 25, LAYOUT->FREQ_Y_BASELINE + LAYOUT->FREQ_DELIMITER_Y_OFFSET - 12, COLOR->FREQ_HZ, BG_COLOR, 2);
	addSymbols(buff, LCD_freq_string_kHz, 3, "0", false);
	LCDDriver_printText(buff, LAYOUT->FREQ_DELIMITER_X2_OFFSET - 15, LAYOUT->FREQ_Y_BASELINE + LAYOUT->FREQ_DELIMITER_Y_OFFSET - 12, COLOR->FREQ_HZ, BG_COLOR, 2);
	LCDDriver_printText(".", LAYOUT->FREQ_DELIMITER_X2_OFFSET + 20, LAYOUT->FREQ_Y_BASELINE + LAYOUT->FREQ_DELIMITER_Y_OFFSET - 12, COLOR->FREQ_HZ, BG_COLOR, 2);
	addSymbols(buff, LCD_freq_string_Hz, 3, "0", false);
	LCDDriver_printText(buff, LAYOUT->FREQ_DELIMITER_X2_OFFSET + 30, LAYOUT->FREQ_Y_BASELINE + LAYOUT->FREQ_DELIMITER_Y_OFFSET - 12, COLOR->FREQ_HZ, BG_COLOR, 2);
}
// MEMO
static void MemoryReadChan(uint8_t parameter) {
	int8_t channel = parameter;
	if (channel >= MEMORY_CHANNELS_COUNT) {
	}
	MemoryChanFreq = CALIBRATE.MEMORY_CHANNELS[channel].freq;
	strcpy(MemoryChanName, CALIBRATE.MEMORY_CHANNELS[channel].name);
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
		LCDDriver_Fill_RectWH(LAYOUT->FREQ_LEFT_MARGIN + 14, LAYOUT->FREQ_Y_TOP - 35, LCD_WIDTH - LAYOUT->FREQ_LEFT_MARGIN - LAYOUT->FREQ_RIGHT_MARGIN - 14, LAYOUT->FREQ_BLOCK_HEIGHT, BG_COLOR);
	}

	if ((MHz_x_offset - LAYOUT->FREQ_LEFT_MARGIN) > 0) {
		LCDDriver_Fill_RectWH(LAYOUT->FREQ_LEFT_MARGIN + 14, LAYOUT->FREQ_Y_TOP - 35, MHz_x_offset - LAYOUT->FREQ_LEFT_MARGIN - 14, LAYOUT->FREQ_BLOCK_HEIGHT, BG_COLOR);
	}

	// add spaces to output the frequency
	sprintf(LCD_freq_string_Hz, "%d", Hz);
	sprintf(LCD_freq_string_kHz, "%d", kHz);
	sprintf(LCD_freq_string_MHz, "%d", MHz);

	if (redraw || (LCD_last_showed_freq_MHz != MHz)) {
		LCDDriver_printTextFont(LCD_freq_string_MHz, MHz_x_offset, LAYOUT->FREQ_Y_BASELINE - 25, COLOR->FREQ_MHZ, BG_COLOR, LAYOUT->FREQ_FONT);
		LCD_last_showed_freq_MHz = MHz;
	}

	char buff[50] = "";
	if (redraw || (LCD_last_showed_freq_kHz != kHz)) {
		addSymbols(buff, LCD_freq_string_kHz, 3, "0", false);
		LCDDriver_printTextFont(buff, LAYOUT->FREQ_X_OFFSET_KHZ, LAYOUT->FREQ_Y_BASELINE - 25, COLOR->FREQ_KHZ, BG_COLOR, LAYOUT->FREQ_FONT);
		LCD_last_showed_freq_kHz = kHz;
	}
	if (redraw || (LCD_last_showed_freq_Hz != Hz) || TRX.ChannelMode) {
		addSymbols(buff, LCD_freq_string_Hz, 3, "0", false);
		int_fast8_t band = -1;
		int_fast16_t channel = -1;
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
			LCDDriver_printText(buff, LAYOUT->FREQ_X_OFFSET_HZ + 2, LAYOUT->FREQ_Y_BASELINE_SMALL - 20 - RASTR_FONT_H * 2, COLOR->FREQ_HZ, BG_COLOR, 2);

			if (band != -1 && channel != -1) {
				sprintf(buff, "%d", BANDS[band].channels[channel].number);
			} else {
				sprintf(buff, "-");
			}
			addSymbols(buff, buff, 2, " ", true);
			LCDDriver_printTextFont(buff, LAYOUT->FREQ_X_OFFSET_HZ + 2 + RASTR_FONT_W * 2 * 2, LAYOUT->FREQ_Y_BASELINE_SMALL - 23, !TRX.selected_vfo ? COLOR->STATUS_MODE : COLOR->FREQ_A_INACTIVE,
			                        BG_COLOR, LAYOUT->FREQ_CH_FONT);
		} else {
			LCDDriver_printTextFont(buff, LAYOUT->FREQ_X_OFFSET_HZ, LAYOUT->FREQ_Y_BASELINE_SMALL - 25, COLOR->FREQ_HZ, BG_COLOR, LAYOUT->FREQ_SMALL_FONT);
		}
		LCD_last_showed_freq_Hz = Hz;
	}

	if (redraw) {
		// Frequency delimiters
		LCDDriver_printTextFont(".", LAYOUT->FREQ_DELIMITER_X1_OFFSET, LAYOUT->FREQ_Y_BASELINE - 25 + LAYOUT->FREQ_DELIMITER_Y_OFFSET, COLOR->FREQ_KHZ, BG_COLOR, LAYOUT->FREQ_FONT);
		LCDDriver_printTextFont(".", LAYOUT->FREQ_DELIMITER_X2_OFFSET, LAYOUT->FREQ_Y_BASELINE - 25 + LAYOUT->FREQ_DELIMITER_Y_OFFSET, COLOR->FREQ_HZ, BG_COLOR, LAYOUT->FREQ_FONT);

		if (TRX.ENC2_func_mode == ENC_FUNC_SET_MEM) {
			MemoryReadChan(TRX_MemoryChannelSelected);
			addSymbols(buff, MemoryChanName, 6, " ", false);
			LCDDriver_printText(buff, LAYOUT->FREQ_DELIMITER_X2_OFFSET - 140, LAYOUT->FREQ_Y_BASELINE + LAYOUT->FREQ_DELIMITER_Y_OFFSET + -11, COLOR->FREQ_HZ, BG_COLOR, 2);
			LCD_displayFreqV2(MemoryChanFreq);
		} else {
			LCDDriver_printText("              ", LAYOUT->FREQ_DELIMITER_X2_OFFSET - 140, LAYOUT->FREQ_Y_BASELINE + LAYOUT->FREQ_DELIMITER_Y_OFFSET - 11, COLOR->FREQ_HZ, BG_COLOR, 2);
			LCD_displayFreqV2(SecondaryVFO->Freq);
		}
	}

	NeedSaveSettings = true;

	LCD_UpdateQuery.FreqInfo = false;
	if (redraw) {
		LCD_UpdateQuery.FreqInfoRedraw = false;
	}

	LCD_busy = false;
}

static void LCD_drawSMeter(void) {
	// analog version
	if (LAYOUT->STATUS_SMETER_ANALOG) {
		LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_BAR_X_OFFSET - 2, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET, &image_data_meter, COLOR_BLACK, BG_COLOR);
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
			LCDDriver_printTextFont("TU", LAYOUT->STATUS_TXRX_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_TXRX_Y_OFFSET), COLOR->STATUS_TU, BG_COLOR, LAYOUT->STATUS_TXRX_FONT);
		} else {
			LCDDriver_printTextFont("TX", LAYOUT->STATUS_TXRX_X_OFFSET + 1, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_TXRX_Y_OFFSET), COLOR->STATUS_TX, BG_COLOR, LAYOUT->STATUS_TXRX_FONT);
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
		LCDDriver_printTextFont("RX", LAYOUT->STATUS_TXRX_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_TXRX_Y_OFFSET), COLOR->STATUS_RX, BG_COLOR, LAYOUT->STATUS_TXRX_FONT);
	}

	// Mode indicator
	printInfo(LAYOUT->STATUS_MODE_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_MODE_Y_OFFSET), LAYOUT->STATUS_MODE_BLOCK_WIDTH, LAYOUT->STATUS_MODE_BLOCK_HEIGHT,
	          (char *)MODE_DESCR[CurrentVFO->Mode], BG_COLOR, COLOR->STATUS_MODE, COLOR->STATUS_MODE, true);
	// Redraw TextBar
	if (NeedProcessDecoder) {
		LCDDriver_Fill_RectWH(0, LAYOUT->FFT_FFTWTF_BOTTOM - LAYOUT->FFT_CWDECODER_OFFSET, LAYOUT->FFT_PRINT_SIZE, LAYOUT->FFT_CWDECODER_OFFSET, BG_COLOR);
		LCD_UpdateQuery.TextBar = true;
	}

	// WIFI indicator
	if (WIFI_connected) {
		// WiFi image GREEN
		LCDDriver_drawFastHLine(LAYOUT->STATUS_WIFI_ICON_X - 2, LAYOUT->STATUS_WIFI_ICON_Y - 7, 4, COLOR_GREEN);
		LCDDriver_drawFastHLine(LAYOUT->STATUS_WIFI_ICON_X - 4, LAYOUT->STATUS_WIFI_ICON_Y - 6, 8, COLOR_GREEN);
		LCDDriver_drawFastHLine(LAYOUT->STATUS_WIFI_ICON_X - 5, LAYOUT->STATUS_WIFI_ICON_Y - 5, 10, COLOR_GREEN);
		LCDDriver_drawFastHLine(LAYOUT->STATUS_WIFI_ICON_X - 4, LAYOUT->STATUS_WIFI_ICON_Y - 4, 8, COLOR_GREEN);
		LCDDriver_drawFastHLine(LAYOUT->STATUS_WIFI_ICON_X - 3, LAYOUT->STATUS_WIFI_ICON_Y - 3, 6, COLOR_GREEN);
		LCDDriver_drawFastHLine(LAYOUT->STATUS_WIFI_ICON_X - 2, LAYOUT->STATUS_WIFI_ICON_Y - 2, 4, COLOR_GREEN);
		LCDDriver_drawFastHLine(LAYOUT->STATUS_WIFI_ICON_X - 1, LAYOUT->STATUS_WIFI_ICON_Y - 1, 2, COLOR_GREEN);
		LCDDriver_drawPixel(LAYOUT->STATUS_WIFI_ICON_X, LAYOUT->STATUS_WIFI_ICON_Y, COLOR_GREEN);
	} else {
		// WiFi image RED
		LCDDriver_drawFastHLine(LAYOUT->STATUS_WIFI_ICON_X - 2, LAYOUT->STATUS_WIFI_ICON_Y - 7, 4, COLOR_RED);
		LCDDriver_drawFastHLine(LAYOUT->STATUS_WIFI_ICON_X - 4, LAYOUT->STATUS_WIFI_ICON_Y - 6, 8, COLOR_RED);
		LCDDriver_drawFastHLine(LAYOUT->STATUS_WIFI_ICON_X - 5, LAYOUT->STATUS_WIFI_ICON_Y - 5, 10, COLOR_RED);
		LCDDriver_drawFastHLine(LAYOUT->STATUS_WIFI_ICON_X - 4, LAYOUT->STATUS_WIFI_ICON_Y - 4, 8, COLOR_RED);
		LCDDriver_drawFastHLine(LAYOUT->STATUS_WIFI_ICON_X - 3, LAYOUT->STATUS_WIFI_ICON_Y - 3, 6, COLOR_RED);
		LCDDriver_drawFastHLine(LAYOUT->STATUS_WIFI_ICON_X - 2, LAYOUT->STATUS_WIFI_ICON_Y - 2, 4, COLOR_RED);
		LCDDriver_drawFastHLine(LAYOUT->STATUS_WIFI_ICON_X - 1, LAYOUT->STATUS_WIFI_ICON_Y - 1, 2, COLOR_RED);
		LCDDriver_drawPixel(LAYOUT->STATUS_WIFI_ICON_X, LAYOUT->STATUS_WIFI_ICON_Y, COLOR_RED);
	}

	// ENC2 State indicator
	char enc2_state_str[5] = {0};
	if (TRX.ENC2_func_mode == ENC_FUNC_PAGER) {
		sprintf(enc2_state_str, "FUNC");
	}
	if (TRX.ENC2_func_mode == ENC_FUNC_FAST_STEP) {
		sprintf(enc2_state_str, "STEP");
	}
	if (TRX.ENC2_func_mode == ENC_FUNC_SET_WPM) {
		sprintf(enc2_state_str, "WPM");
	}
	if (TRX.ENC2_func_mode == ENC_FUNC_SET_RIT) {
		sprintf(enc2_state_str, "RIT");
	}
	if (TRX.ENC2_func_mode == ENC_FUNC_SET_NOTCH) {
		sprintf(enc2_state_str, "NTCH");
	}
	if (TRX.ENC2_func_mode == ENC_FUNC_SET_LPF) {
		sprintf(enc2_state_str, "LPF");
	}
	if (TRX.ENC2_func_mode == ENC_FUNC_SET_HPF) {
		sprintf(enc2_state_str, "HPF");
	}
	if (TRX.ENC2_func_mode == ENC_FUNC_SET_SQL) {
		sprintf(enc2_state_str, "SQL");
	}
	if (TRX.ENC2_func_mode == ENC_FUNC_SET_VOLUME) {
		sprintf(enc2_state_str, "VOL");
	}
	if (TRX.ENC2_func_mode == ENC_FUNC_SET_IF) {
		sprintf(enc2_state_str, " IF");
	}
	if (TRX.ENC2_func_mode == ENC_FUNC_SET_MEM) {
		sprintf(enc2_state_str, "MEM0");
	}
	LCDDriver_Fill_RectWH(LAYOUT->STATUS_ANT_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_ANT_Y_OFFSET) - 1, 45, 19, COLOR_WHITE); // rectangl back

	LCDDriver_Fill_RectWH(LAYOUT->STATUS_ANT_X_OFFSET + 1, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_ANT_Y_OFFSET), 43, 17, COLOR_BLACK); // clear rectangl

	if ((TRX.ENC2_func_mode == ENC_FUNC_PAGER) || (TRX.ENC2_func_mode == ENC_FUNC_FAST_STEP) || (TRX.ENC2_func_mode == ENC_FUNC_SET_NOTCH)) {
		printInfoSmall(LAYOUT->STATUS_ANT_X_OFFSET + 2, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_ANT_Y_OFFSET + 1), LAYOUT->STATUS_ANT_BLOCK_WIDTH, LAYOUT->STATUS_ANT_BLOCK_HEIGHT - 7,
		               enc2_state_str, BG_COLOR, COLOR->STATUS_RX, COLOR->STATUS_RX, true);
	} else {
		printInfoSmall(LAYOUT->STATUS_ANT_X_OFFSET + 4, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_ANT_Y_OFFSET + 1), LAYOUT->STATUS_ANT_BLOCK_WIDTH, LAYOUT->STATUS_ANT_BLOCK_HEIGHT - 7,
		               enc2_state_str, BG_COLOR, COLOR->STATUS_RX, COLOR->STATUS_RX, true);
	}

// WIFI indicator
#if HRDW_HAS_WIFI
// if (WIFI_connected)
// LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_WIFI_ICON_X, LAYOUT->STATUS_WIFI_ICON_Y, &IMAGES_wifi_active, COLOR_BLACK, BG_COLOR);
// else
// LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_WIFI_ICON_X, LAYOUT->STATUS_WIFI_ICON_Y, &IMAGES_wifi_inactive, COLOR_BLACK, BG_COLOR);
#endif

	if (FAN_Active) {
		// FAN indicator
		// LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_FAN_ICON_X, LAYOUT->STATUS_FAN_ICON_Y, &IMAGES_fan, COLOR_BLACK, BG_COLOR);
	} else {
#if HRDW_HAS_SD
// SD indicator
// if (SD_Present)
// LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_SD_ICON_X, LAYOUT->STATUS_SD_ICON_Y, &IMAGES_sd_active, COLOR_BLACK, BG_COLOR);
// else
// LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_SD_ICON_X, LAYOUT->STATUS_SD_ICON_Y, &IMAGES_sd_inactive, COLOR_BLACK, BG_COLOR);
#endif
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
		static uint16_t sql_stripe_x_pos_old = 0;

		if (CurrentVFO->Mode == TRX_MODE_CW) {
			s_width = LCD_last_s_meter * 0.5f + LCD_GetSMeterValPosition(TRX_RX1_dBm_lowrate, true) * 0.5f; // smooth CW faster!
		} else {
			s_width = LCD_last_s_meter * 0.8f + LCD_GetSMeterValPosition(TRX_RX1_dBm_lowrate, true) * 0.2f; // smooth the movement of the S-meter
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
			LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)LCD_last_s_meter, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + 5, 2,
			                      LAYOUT->STATUS_SMETER_MARKER_HEIGHT - 8, BG_COLOR);
			LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)s_width, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + 5, 2, LAYOUT->STATUS_SMETER_MARKER_HEIGHT - 8,
			                      COLOR->STATUS_SMETER_STRIPE);
			// clear old SQL stripe
			if (sql_stripe_x_pos_old != sql_stripe_x_pos) {
				LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + sql_stripe_x_pos_old, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 2, 2,
				                      LAYOUT->STATUS_BAR_HEIGHT - 3, BG_COLOR);
				sql_stripe_x_pos_old = sql_stripe_x_pos;
			}

			// bar
			LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 2, (uint16_t)s_width,
			                      LAYOUT->STATUS_BAR_HEIGHT - 3, COLOR->STATUS_SMETER);

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
			LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)s_width, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + 5, 2, LAYOUT->STATUS_SMETER_MARKER_HEIGHT - 8,
			                      COLOR->STATUS_SMETER_STRIPE);

			LCD_last_s_meter = s_width;
		}

		// print dBm value
		sprintf(ctmp, "%ddBm", (int16_t)TRX_RX1_dBm_lowrate);
		addSymbols(ctmp, ctmp, 7, " ", true);
		LCDDriver_printText(ctmp, LAYOUT->STATUS_LABEL_DBM_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABEL_DBM_Y_OFFSET, COLOR->STATUS_LABEL_DBM, BG_COLOR,
		                    LAYOUT->STATUS_LABELS_FONT_SIZE);

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
	} else {
		// ATU
		if (SYSMENU_HANDL_CHECK_HAS_ATU() && LAYOUT->STATUS_ATU_I_Y > 0) {
			if (TRX.TUNER_Enabled) {
				float32_t *atu_i = ATU_I_VALS;
				float32_t *atu_c = ATU_C_VALS;

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

				sprintf(ctmp, "I=%.2fuH", (float64_t)i_val);
				addSymbols(ctmp, ctmp, 8, " ", true);
				LCDDriver_printText(ctmp, LAYOUT->STATUS_ATU_I_X, LAYOUT->STATUS_ATU_I_Y, FG_COLOR, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);

				if (TRX.ATU_T) {
					sprintf(ctmp, "T+%dpF", (uint32_t)c_val);
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
			} else if (TRX_PWR_Forward_SMOOTHED >= 9.5f) {
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
			} else if (TRX_PWR_Backward_SMOOTHED >= 9.5f) {
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
			LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_PMETER_WIDTH + LAYOUT->STATUS_TX_ALC_X_OFFSET + 8,
			                      LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, LAYOUT->STATUS_TX_LABELS_VAL_WIDTH,
			                      LAYOUT->STATUS_TX_LABELS_VAL_HEIGHT, BG_COLOR);
			uint8_t alc_level = (uint8_t)(TRX_ALC_OUT * 100.0f);
			sprintf(ctmp, "%d%%", alc_level);
			LCDDriver_printText(ctmp, LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_PMETER_WIDTH + LAYOUT->STATUS_TX_ALC_X_OFFSET + 8,
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
		}
	}

	// Info labels
	char buff[32] = "";
	// BW HPF-LPF
	if (CurrentVFO->Mode == TRX_MODE_CW) {
		sprintf(buff, "%.1f", (float64_t)CurrentVFO->CW_LPF_Filter / 1000.0);
	} else if ((CurrentVFO->Mode == TRX_MODE_DIGI_L || CurrentVFO->Mode == TRX_MODE_DIGI_U || CurrentVFO->Mode == TRX_MODE_RTTY)) {
		sprintf(buff, "%.1f", (float64_t)CurrentVFO->DIGI_LPF_Filter / 1000.0);
	} else if ((CurrentVFO->Mode == TRX_MODE_LSB || CurrentVFO->Mode == TRX_MODE_USB)) {
		if (TRX_on_TX) {
			sprintf(buff, "%.1f", (float64_t)CurrentVFO->SSB_LPF_TX_Filter / 1000.0);
		} else {
			sprintf(buff, "%.1f", (float64_t)CurrentVFO->SSB_LPF_RX_Filter / 1000.0);
		}
	} else if (CurrentVFO->Mode == TRX_MODE_AM || CurrentVFO->Mode == TRX_MODE_SAM_STEREO || CurrentVFO->Mode == TRX_MODE_SAM_LSB || CurrentVFO->Mode == TRX_MODE_SAM_USB) {
		if (TRX_on_TX) {
			sprintf(buff, "%.1f", (float64_t)CurrentVFO->AM_LPF_TX_Filter / 1000.0);
		} else {
			sprintf(buff, "%.1f", (float64_t)CurrentVFO->AM_LPF_RX_Filter / 1000.0);
		}
	} else if (CurrentVFO->Mode == TRX_MODE_NFM) {
		if (TRX_on_TX) {
			sprintf(buff, "%.1f", (float64_t)CurrentVFO->FM_LPF_TX_Filter / 1000.0);
		} else {
			sprintf(buff, "%.1f", (float64_t)CurrentVFO->FM_LPF_RX_Filter / 1000.0);
		}
	} else {
		sprintf(buff, "%d", TRX_GetRXSampleRate / 1000);
	}
	addSymbols(buff, buff, 4, " ", true);

	static char prev_bw_buff[16] = "";
	if (redraw || strcmp(prev_bw_buff, buff) != 0) {
		strcpy(prev_bw_buff, buff);
		LCDDriver_printText(buff, LAYOUT->STATUS_LABEL_BW_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABEL_BW_Y_OFFSET, COLOR->STATUS_LABEL_BW, BG_COLOR,
		                    LAYOUT->STATUS_LABELS_FONT_SIZE);
	}

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

	static char prev_rit_buff[16] = "";
	if (redraw || strcmp(prev_rit_buff, buff) != 0) {
		strcpy(prev_rit_buff, buff);
		printInfoStatus(LAYOUT->STATUS_INFOB_X_OFFSET, LAYOUT->STATUS_INFO_Y_OFFSET, LAYOUT->STATUS_INFO_WIDTH, LAYOUT->STATUS_INFO_HEIGHT, COLOR->STATUS_INFO_BORDER, buff, COLOR->BACKGROUND,
		                COLOR_WHITE, COLOR->BUTTON_INACTIVE_TEXT, true);
	}

	// IN-OUT
	// sprintf(buff, "MIC:%d", TRX.MIC_GAIN);

	// static char prev_mic_buff[16] = "";
	// if(redraw || strcmp(prev_mic_buff, buff) != 0) {
	// strcpy(prev_mic_buff, buff);
	// printInfoStatus(LAYOUT->STATUS_INFOA_X_OFFSET, LAYOUT->STATUS_INFO_Y_OFFSET, LAYOUT->STATUS_INFO_WIDTH, LAYOUT->STATUS_INFO_HEIGHT,
	// COLOR->STATUS_INFO_BORDER, buff, COLOR->BACKGROUND, COLOR_RED, COLOR->BUTTON_INACTIVE_TEXT, true);
	//}

	// VOLTAGE
	sprintf(buff, "%0.1fV", (float64_t)TRX_PWR_Voltage);
	static char prev_vlt_buff[16] = "";
	if (redraw || strcmp(prev_vlt_buff, buff) != 0) {
		strcpy(prev_vlt_buff, buff);
		printInfoStatus(LAYOUT->STATUS_INFOC_X_OFFSET, LAYOUT->STATUS_INFO_Y_OFFSET, LAYOUT->STATUS_INFO_WIDTH, LAYOUT->STATUS_INFO_HEIGHT, COLOR->STATUS_INFO_BORDER, buff, COLOR->BACKGROUND,
		                COLOR_WHITE, COLOR->BUTTON_INACTIVE_TEXT, true);
	}

	// IF GAIN
	sprintf(buff, "IF:%d", TRX.IF_Gain);

	static char prev_if_buff[16] = "";
	if (redraw || strcmp(prev_if_buff, buff) != 0) {
		strcpy(prev_if_buff, buff);
		printInfoStatus(LAYOUT->STATUS_INFOD_X_OFFSET, LAYOUT->STATUS_INFO_Y_OFFSET, LAYOUT->STATUS_INFO_WIDTH, LAYOUT->STATUS_INFO_HEIGHT, COLOR->STATUS_INFO_BORDER, buff, COLOR->BACKGROUND,
		                COLOR_WHITE, COLOR->BUTTON_INACTIVE_TEXT, true);
	}

	// NOTCH
	if (CurrentVFO->AutoNotchFilter) {
		sprintf(buff, "NH:AUTO");
	} else if (CurrentVFO->ManualNotchFilter) {
		sprintf(buff, "NH:%uHz", CurrentVFO->NotchFC);
	} else {
		sprintf(buff, "NH:OFF");
	}

	static char prev_notch_buff[16] = "";
	if (redraw || strcmp(prev_notch_buff, buff) != 0) {
		strcpy(prev_notch_buff, buff);
		printInfoStatus(LAYOUT->STATUS_INFOE_X_OFFSET, LAYOUT->STATUS_INFO_Y_OFFSET, LAYOUT->STATUS_INFO_WIDTH, LAYOUT->STATUS_INFO_HEIGHT, COLOR->STATUS_INFO_BORDER, buff, COLOR->BACKGROUND,
		                COLOR_WHITE, COLOR->BUTTON_INACTIVE_TEXT, true);
	}

	// FFT BW
	uint8_t fft_zoom = TRX.FFT_Zoom;
	if (CurrentVFO->Mode == TRX_MODE_CW) {
		fft_zoom = TRX.FFT_ZoomCW;
	}

	sprintf(buff, "%dk x%d", FFT_current_spectrum_width_Hz / 1000, fft_zoom);

	static char prev_fftbw_buff[16] = "";
	if (redraw || strcmp(prev_fftbw_buff, buff) != 0) {
		strcpy(prev_fftbw_buff, buff);
		printInfoStatus(LAYOUT->STATUS_INFOF_X_OFFSET, LAYOUT->STATUS_INFO_Y_OFFSET, LAYOUT->STATUS_INFO_WIDTH, LAYOUT->STATUS_INFO_HEIGHT, COLOR->STATUS_INFO_BORDER, buff, COLOR->BACKGROUND,
		                COLOR_WHITE, COLOR->BUTTON_INACTIVE_TEXT, true);
	}

	// ERRORS LABELS !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	LCDDriver_Fill_RectWH(LAYOUT->STATUS_ERR_OFFSET_X, LAYOUT->STATUS_ERR_OFFSET_Y, LAYOUT->STATUS_ERR_WIDTH, LAYOUT->STATUS_ERR_HEIGHT, BG_COLOR);
	if (TRX_ADC_OTR && !TRX_on_TX && !TRX.ADC_SHDN) {
		LCDDriver_printText("OVR", LAYOUT->STATUS_ERR_OFFSET_X, LAYOUT->STATUS_ERR_OFFSET_Y, COLOR->STATUS_ERR, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
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
	}
	if (TRX_PWR_ALC_SWR_OVERFLOW) {
		LCDDriver_printText("OVS", LAYOUT->STATUS_ERR_OFFSET_X, LAYOUT->STATUS_ERR_OFFSET_Y, COLOR->STATUS_ERR, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	}
	if (TRX_DAC_OTR) {
		LCDDriver_printText("OVR", LAYOUT->STATUS_ERR_OFFSET_X, LAYOUT->STATUS_ERR_OFFSET_Y, COLOR->STATUS_ERR, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	}
	if (CODEC_Buffer_underrun && !TRX_on_TX) {
		LCDDriver_printText("WBF", LAYOUT->STATUS_ERR_OFFSET_X, LAYOUT->STATUS_ERR_OFFSET_Y, COLOR->STATUS_ERR, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	}
	if (FPGA_Buffer_underrun && TRX_on_TX) {
		LCDDriver_printText("FBF", LAYOUT->STATUS_ERR_OFFSET_X, LAYOUT->STATUS_ERR_OFFSET_Y, COLOR->STATUS_ERR, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	}
	if (RX_USB_AUDIO_underrun) {
		LCDDriver_printText("UBF", LAYOUT->STATUS_ERR_OFFSET_X, LAYOUT->STATUS_ERR_OFFSET_Y, COLOR->STATUS_ERR, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	}
#if HRDW_HAS_SD
	if (SD_underrun) {
		LCDDriver_printText("SDF", LAYOUT->STATUS_ERR_OFFSET_X, LAYOUT->STATUS_ERR_OFFSET_Y, COLOR->STATUS_ERR, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	}
#endif

	RTC_TimeTypeDef sTime = {0};
	RTC_DateTypeDef sDate = {0};
	getLocalDateTime(&sDate, &sTime);
	Hours = sTime.Hours;
	Minutes = sTime.Minutes;
	Seconds = sTime.Seconds;

	if (redraw || (Hours != Last_showed_Hours)) {
		sprintf(ctmp, "%d", Hours);
		addSymbols(ctmp, ctmp, 2, "0", false);
		// LCDDriver_printTextFont(ctmp, LAYOUT->CLOCK_POS_HRS_X, LAYOUT->CLOCK_POS_Y, COLOR->CLOCK, BG_COLOR, LAYOUT->CLOCK_FONT);
		// LCDDriver_printTextFont(":", LCDDriver_GetCurrentXOffset(), LAYOUT->CLOCK_POS_Y, COLOR->CLOCK, BG_COLOR, LAYOUT->CLOCK_FONT);
		Last_showed_Hours = Hours;
	}
	if (redraw || (Minutes != Last_showed_Minutes)) {
		sprintf(ctmp, "%d", Minutes);
		addSymbols(ctmp, ctmp, 2, "0", false);
		// LCDDriver_printTextFont(ctmp, LAYOUT->CLOCK_POS_MIN_X, LAYOUT->CLOCK_POS_Y, COLOR->CLOCK, BG_COLOR, LAYOUT->CLOCK_FONT);
		// LCDDriver_printTextFont(":", LCDDriver_GetCurrentXOffset(), LAYOUT->CLOCK_POS_Y, COLOR->CLOCK, BG_COLOR, LAYOUT->CLOCK_FONT);
		Last_showed_Minutes = Minutes;
	}
	if (redraw || (Seconds != Last_showed_Seconds)) {
		sprintf(ctmp, "%d", Seconds);
		addSymbols(ctmp, ctmp, 2, "0", false);
		// LCDDriver_printTextFont(ctmp, LAYOUT->CLOCK_POS_SEC_X, LAYOUT->CLOCK_POS_Y, COLOR->CLOCK, BG_COLOR, LAYOUT->CLOCK_FONT);
		Last_showed_Seconds = Seconds;
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
		LCDDriver_printText(RTTY_Decoder_Text, 2, (LAYOUT->FFT_FFTWTF_BOTTOM - LAYOUT->FFT_CWDECODER_OFFSET + 1), COLOR->CLOCK, BG_COLOR, LAYOUT->TEXTBAR_FONT);
	}

	LCD_UpdateQuery.TextBar = false;
	LCD_busy = false;
}

void LCD_redraw(bool do_now) {
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

static void printInfoStatus(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t button_color, char *text, uint16_t back_color, uint16_t text_color, uint16_t inactive_color,
                            bool active) {
	uint16_t x1, y1, w, h, color;
	LCDDriver_drawFastHLine(x, y - 1, width, active ? COLOR_GBLUE : COLOR_DARKBLUE);
	LCDDriver_drawFastHLine(x, y + (height + 1), width, active ? COLOR_GBLUE : COLOR_DARKBLUE);
	LCDDriver_drawFastVLine(x - 1, y - 1, height + 3, active ? COLOR_GBLUE : COLOR_DARKBLUE);
	LCDDriver_drawFastVLine(x + (width + 1), y - 1, height + 3, active ? COLOR_GBLUE : COLOR_DARKBLUE);

	LCDDriver_Fill_RectWH(x, y, width, height, back_color);
	LCDDriver_getTextBoundsFont(text, x, y, &x1, &y1, &w, &h, (GFXfont *)&PrestigeEliteStd_Bd7pt7b);
	LCDDriver_printTextFont(text, x + (width - w) / 2, y + (height / 2) + h / 2, active ? COLOR_GREENYELLOW : inactive_color, back_color, (GFXfont *)&PrestigeEliteStd_Bd7pt7b);
}

static void printInfo(uint16_t x, uint16_t y, uint16_t width, uint16_t height, char *text, uint16_t back_color, uint16_t text_color, uint16_t inactive_color, bool active) {
	uint16_t x1, y1, w, h;
	LCDDriver_Fill_RectWH(x, y, width, height, back_color);
	LCDDriver_getTextBoundsFont(text, x, y, &x1, &y1, &w, &h, (GFXfont *)&FreeSans9pt7b);
	// sendToDebug_str(text); sendToDebug_str(" "); sendToDebug_uint16(w, false);
	LCDDriver_printTextFont(text, x + (width - w) / 2, y + (height / 2) + h / 2 - 1, active ? text_color : inactive_color, back_color, (GFXfont *)&FreeSans9pt7b);
}

static void printButton(uint16_t x, uint16_t y, uint16_t width, uint16_t height, char *text, bool active, bool show_lighter, bool in_window, uint32_t parameter,
                        void (*clickHandler)(uint32_t parameter), void (*holdHandler)(uint32_t parameter), uint16_t active_color, uint16_t inactive_color) {
	uint16_t x1_text, y1_text, w_text, h_text;
	if (in_window) {
		x += LCD_window.x;
		y += LCD_window.y;
	}
	uint16_t x_act = x + LAYOUT->BUTTON_PADDING;
	uint16_t y_act = y + LAYOUT->BUTTON_PADDING;
	uint16_t w_act = width - LAYOUT->BUTTON_PADDING * 2;
	uint16_t h_act = height - LAYOUT->BUTTON_PADDING * 2;
	LCDDriver_Fill_RectWH(x_act, y_act, w_act, h_act, COLOR->BUTTON_SWITCH_BACKGROUND);                               // button body
	LCDDriver_drawRectXY(x_act, y_act, x_act + w_act, y_act + h_act, COLOR->BUTTON_BORDER);                           // border
	LCDDriver_getTextBoundsFont(text, x_act, y_act, &x1_text, &y1_text, &w_text, &h_text, (GFXfont *)&FreeSans9pt7b); // get text bounds
	if (show_lighter && LAYOUT->BUTTON_LIGHTER_HEIGHT > 0) {
		LCDDriver_printTextFont(text, x_act + (w_act - w_text) / 2, y_act + (h_act * 2 / 5) + h_text / 2 - 1, active ? active_color : inactive_color, COLOR->BUTTON_SWITCH_BACKGROUND,
		                        &FreeSans9pt7b); // text
		uint16_t lighter_width = (uint16_t)((float32_t)w_act * LAYOUT->BUTTON_LIGHTER_WIDTH);
		LCDDriver_Fill_RectWH(x_act + ((w_act - lighter_width) / 2), y_act + h_act * 3 / 4, lighter_width, LAYOUT->BUTTON_LIGHTER_HEIGHT,
		                      active ? COLOR->BUTTON_LIGHTER_ACTIVE : COLOR->BUTTON_LIGHTER_INACTIVE); // lighter
	} else {
		LCDDriver_printTextFont(text, x_act + (w_act - w_text) / 2, y_act + (h_act / 2) + h_text / 2 - 1, active ? active_color : inactive_color, COLOR->BUTTON_SWITCH_BACKGROUND,
		                        &FreeSans9pt7b); // text
	}
}

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

void LCD_processTouch(uint16_t x, uint16_t y) {}

void LCD_processHoldTouch(uint16_t x, uint16_t y) {}

bool LCD_processSwipeTouch(uint16_t x, uint16_t y, int16_t dx, int16_t dy) { return false; }

bool LCD_processSwipeTwoFingersTouch(uint16_t x, uint16_t y, int16_t dx, int16_t dy) { return false; }

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

void LCD_openWindow(uint16_t w, uint16_t h) {}

void LCD_closeWindow(void) {}

static void LCD_showBandWindow(bool secondary_vfo) {}

static void LCD_showModeWindow(bool secondary_vfo) {}

static void LCD_showBWWindow(void) {}

void LCD_showRFPowerWindow(void) {}

void LCD_showManualFreqWindow(uint32_t parameter) {}

void LCD_ManualFreqButtonHandler(uint32_t parameter) {}

static void LCD_ShowMemoryChannelsButtonHandler(uint32_t parameter) {}

void LCD_printKeyboard(void (*keyboardHandler)(char *string, uint32_t max_size, char entered), char *string, uint32_t max_size, bool lowcase) {}

void LCD_hideKeyboard(void) {
	LCD_screenKeyboardOpened = false;
	LCD_keyboardHandler = NULL;
}

void LCD_showATTWindow(uint32_t parameter) {}
