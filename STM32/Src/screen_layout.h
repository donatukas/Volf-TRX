#ifndef LCD_Layout_h
#define LCD_Layout_h

#ifdef __cplusplus
extern "C" {
#endif

#include "color_themes.h"
#include "fonts.h"
#include "lcd_driver.h"

// LCD dimensions defines
typedef const struct {
	// Top row of status buttons
	const uint16_t TOPBUTTONS_X1;
	const uint16_t TOPBUTTONS_X2;
	const uint16_t TOPBUTTONS_Y1;
	const uint16_t TOPBUTTONS_Y2;
#if (defined(LAY_800x480))
	const uint16_t TOPBUTTONS_COUNT;
#endif
	const float32_t TOPBUTTONS_WIDTH;
	const uint16_t TOPBUTTONS_HEIGHT;
	const uint16_t TOPBUTTONS_TB_MARGIN;
	const uint16_t TOPBUTTONS_LR_MARGIN;
	const float32_t TOPBUTTONS_PRE_X;
	const uint16_t TOPBUTTONS_PRE_Y;
	const float32_t TOPBUTTONS_ATT_X;
	const uint16_t TOPBUTTONS_ATT_Y;
	const float32_t TOPBUTTONS_PGA_X;
	const uint16_t TOPBUTTONS_PGA_Y;
	const float32_t TOPBUTTONS_DRV_X;
	const uint16_t TOPBUTTONS_DRV_Y;
	const float32_t TOPBUTTONS_AGC_X;
	const uint16_t TOPBUTTONS_AGC_Y;
	const uint16_t TOPBUTTONS_SECOND_LINE_Y;
	const float32_t TOPBUTTONS_DNR_X;
	const uint16_t TOPBUTTONS_DNR_Y;
	const float32_t TOPBUTTONS_NB_X;
	const uint16_t TOPBUTTONS_NB_Y;
#if (defined(LAY_800x480))
	const float32_t TOPBUTTONS_NOTCH_X;
	const uint16_t TOPBUTTONS_NOTCH_Y;
	const float32_t TOPBUTTONS_SQL_X;
	const uint16_t TOPBUTTONS_SQL_Y;
#endif
	const float32_t TOPBUTTONS_FAST_X;
	const uint16_t TOPBUTTONS_FAST_Y;
	const float32_t TOPBUTTONS_MUTE_X;
	const uint16_t TOPBUTTONS_MUTE_Y;
	const float32_t TOPBUTTONS_LOCK_X;
	const uint16_t TOPBUTTONS_LOCK_Y;
	const uint8_t TOPBUTTONS_ROUND;
	const GFXfont *TOPBUTTONS_FONT;
	// Clock
	const uint16_t CLOCK_POS_Y;
	const uint16_t CLOCK_POS_HRS_X;
	const uint16_t CLOCK_POS_MIN_X;
	const uint16_t CLOCK_POS_SEC_X;
	const GFXfont *CLOCK_FONT;
	// WIFI
	const uint16_t STATUS_WIFI_ICON_X;
	const uint16_t STATUS_WIFI_ICON_Y;
	// SD
	const uint16_t STATUS_SD_ICON_X;
	const uint16_t STATUS_SD_ICON_Y;
	// FAN
	const uint16_t STATUS_FAN_ICON_X;
	const uint16_t STATUS_FAN_ICON_Y;
	// Frequency output
	const uint16_t FREQ_A_LEFT;
	const uint16_t FREQ_X_OFFSET_100;
	const uint16_t FREQ_X_OFFSET_10;
	const uint16_t FREQ_X_OFFSET_1;
	const uint16_t FREQ_X_OFFSET_KHZ;
	const uint16_t FREQ_X_OFFSET_HZ;
	const uint16_t FREQ_HEIGHT;
	const uint16_t FREQ_WIDTH;
	const uint16_t FREQ_TOP_OFFSET;
	const uint16_t FREQ_LEFT_MARGIN;
	const uint16_t FREQ_RIGHT_MARGIN;
	const uint16_t FREQ_BOTTOM_OFFSET;
	const uint16_t FREQ_BLOCK_HEIGHT;
	const uint16_t FREQ_Y_TOP;
	const uint16_t FREQ_Y_BASELINE;
	const uint16_t FREQ_Y_BASELINE_SMALL;
	const GFXfont *FREQ_FONT;
	const GFXfont *FREQ_SMALL_FONT;
	const GFXfont *FREQ_CH_FONT;
	const GFXfont *FREQ_CH_B_FONT;
	const uint16_t FREQ_DELIMITER_Y_OFFSET;
	const uint16_t FREQ_DELIMITER_X1_OFFSET;
	const uint16_t FREQ_DELIMITER_X2_OFFSET;
#if (defined(LAY_800x480))
	// Frequency B output
	const uint16_t FREQ_B_LEFT;
	const uint16_t FREQ_B_X_OFFSET_100;
	const uint16_t FREQ_B_X_OFFSET_10;
	const uint16_t FREQ_B_X_OFFSET_1;
	const uint16_t FREQ_B_X_OFFSET_KHZ;
	const uint16_t FREQ_B_X_OFFSET_HZ;
	const uint16_t FREQ_B_HEIGHT;
	const uint16_t FREQ_B_WIDTH;
	const uint16_t FREQ_B_TOP_OFFSET;
	const uint16_t FREQ_B_LEFT_MARGIN;
	const uint16_t FREQ_B_RIGHT_MARGIN;
	const uint16_t FREQ_B_BOTTOM_OFFSET;
	const uint16_t FREQ_B_BLOCK_HEIGHT;
	const uint16_t FREQ_B_Y_TOP;
	const uint16_t FREQ_B_Y_BASELINE;
	const uint16_t FREQ_B_Y_BASELINE_SMALL;
	const GFXfont *FREQ_B_FONT;
	const GFXfont *FREQ_B_SMALL_FONT;
	const uint16_t FREQ_B_DELIMITER_Y_OFFSET;
	const uint16_t FREQ_B_DELIMITER_X1_OFFSET;
	const uint16_t FREQ_B_DELIMITER_X2_OFFSET;
#endif
	// Display statuses under frequency
	const uint16_t STATUS_Y_OFFSET;
	const uint16_t STATUS_HEIGHT;
	const uint16_t STATUS_BAR_X_OFFSET;
	const uint16_t STATUS_BAR_Y_OFFSET;
	const uint16_t STATUS_BAR_HEIGHT;
	const uint16_t STATUS_TXRX_X_OFFSET;
	const int16_t STATUS_TXRX_Y_OFFSET;
	const GFXfont *STATUS_TXRX_FONT;
	const uint16_t STATUS_VFO_X_OFFSET;
	const int16_t STATUS_VFO_Y_OFFSET;
	const int16_t STATUS_VFO_B_Y_OFFSET;
	const uint16_t STATUS_VFO_BLOCK_WIDTH;
	const uint16_t STATUS_VFO_BLOCK_HEIGHT;
	const uint16_t STATUS_ANT_X_OFFSET;
	const int16_t STATUS_ANT_Y_OFFSET;
	const uint16_t STATUS_ANT_BLOCK_WIDTH;
	const uint16_t STATUS_ANT_BLOCK_HEIGHT;
	const int16_t STATUS_TX_LABELS_OFFSET_X;
	const uint16_t STATUS_TX_LABELS_MARGIN_X;
	bool STATUS_SMETER_ANALOG;
	const int16_t STATUS_SMETER_TOP_OFFSET;
	const uint16_t STATUS_SMETER_ANALOG_HEIGHT;
	const uint16_t STATUS_SMETER_WIDTH;
	const uint16_t STATUS_SMETER_MARKER_HEIGHT;
	const uint16_t STATUS_PMETER_WIDTH;
	const uint16_t STATUS_AMETER_WIDTH;
	const uint16_t STATUS_ALC_BAR_X_OFFSET;
	const uint16_t STATUS_LABELS_OFFSET_Y;
	const uint8_t STATUS_LABELS_FONT_SIZE;
	const uint16_t STATUS_LABEL_S_VAL_X_OFFSET;
	const uint16_t STATUS_LABEL_S_VAL_Y_OFFSET;
	const GFXfont *STATUS_LABEL_S_VAL_FONT;
	const uint16_t STATUS_LABEL_DBM_X_OFFSET;
	const int16_t STATUS_LABEL_DBM_Y_OFFSET;
	const uint16_t STATUS_LABEL_BW_X_OFFSET;
	const int16_t STATUS_LABEL_BW_Y_OFFSET;
	const uint16_t STATUS_LABEL_RIT_X_OFFSET;
	const uint16_t STATUS_LABEL_RIT_Y_OFFSET;
	const uint16_t STATUS_IFGAIN_BAR_OFFSET_X;
	const uint16_t STATUS_IFGAIN_BAR_OFFSET_Y;
	const uint16_t STATUS_IFGAIN_BAR_WIDTH;
	const uint16_t STATUS_IFGAIN_BAR_HEIGTH;
	const uint16_t STATUS_AFGAIN_BAR_OFFSET_X;
	const uint16_t STATUS_AFGAIN_BAR_OFFSET_Y;
	const uint16_t STATUS_AFGAIN_BAR_WIDTH;
	const uint16_t STATUS_AFGAIN_BAR_HEIGTH;
	const uint16_t STATUS_LABEL_THERM_X_OFFSET;
	const uint16_t STATUS_LABEL_THERM_Y_OFFSET;
	const uint16_t STATUS_LABEL_NOTCH_X_OFFSET;
	const uint16_t STATUS_LABEL_NOTCH_Y_OFFSET;
	const uint16_t STATUS_LABEL_FFT_BW_X_OFFSET;
	const uint16_t STATUS_LABEL_FFT_BW_Y_OFFSET;
	// Information panel (Lite, Mini)
	const uint16_t STATUS_INFOA_X_OFFSET;
	const uint16_t STATUS_INFOB_X_OFFSET;
	const uint16_t STATUS_INFOC_X_OFFSET;
	const uint16_t STATUS_INFOD_X_OFFSET;
	const uint16_t STATUS_INFOE_X_OFFSET;
	const uint16_t STATUS_INFOF_X_OFFSET;
	const uint16_t STATUS_INFO_Y_OFFSET;
	const uint16_t STATUS_INFO_WIDTH;
	const uint16_t STATUS_INFO_HEIGHT;
#if (defined(LAY_800x480))
	const uint16_t STATUS_LABEL_CPU_X_OFFSET;
	const uint16_t STATUS_LABEL_CPU_Y_OFFSET;
	const uint16_t STATUS_LABEL_AUTOGAIN_X_OFFSET;
	const uint16_t STATUS_LABEL_AUTOGAIN_Y_OFFSET;
	const uint16_t STATUS_LABEL_LOCK_X_OFFSET;
	const uint16_t STATUS_LABEL_LOCK_Y_OFFSET;
#endif
	const uint16_t STATUS_LABEL_REC_X_OFFSET;
	const uint16_t STATUS_LABEL_REC_Y_OFFSET;
	const uint16_t STATUS_SMETER_PEAK_HOLDTIME;
	const uint16_t STATUS_SMETER_TXLABELS_MARGIN;
	const uint16_t STATUS_SMETER_TXLABELS_PADDING;
	const uint16_t STATUS_ATU_I_X;
	const uint16_t STATUS_ATU_I_Y;
	const uint16_t STATUS_ATU_C_X;
	const uint16_t STATUS_ATU_C_Y;
	const uint16_t STATUS_TX_LABELS_VAL_WIDTH;
	const uint16_t STATUS_TX_LABELS_VAL_HEIGHT;
	const uint16_t STATUS_TX_LABELS_SWR_X;
	const uint16_t STATUS_TX_LABELS_FWD_X;
	const uint16_t STATUS_TX_LABELS_REF_X;
	const uint16_t STATUS_TX_LABELS_ALC_X;
	const uint16_t STATUS_TX_ALC_X_OFFSET;
	const uint16_t STATUS_MODE_ALL_X;
	const uint16_t STATUS_MODE_AM_X;
	const uint16_t STATUS_MODE_SAM_X;
	const uint16_t STATUS_MODE_CW_X;
	const uint16_t STATUS_MODE_DIGL_X;
	const uint16_t STATUS_MODE_DIGU_X;
	const uint16_t STATUS_MODE_NFM_X;
	const uint16_t STATUS_MODE_WFM_X;
	const uint16_t STATUS_MODE_LSB_X;
	const uint16_t STATUS_MODE_USB_X;
	const uint16_t STATUS_MODE_RTTY_X;
	// const uint16_t STATUS_MODE_IQ_X;
	// const uint16_t STATUS_MODE_LOOP_X;
	const uint16_t STATUS_MODE_SELECT_X;
	const uint16_t STATUS_MODE_X_OFFSET;
	const int16_t STATUS_MODE_Y_OFFSET;
	const uint16_t STATUS_MODE_BLOCK_WIDTH;
	const uint16_t STATUS_MODE_BLOCK_HEIGHT;
#if (defined(LAY_800x480))
	const uint16_t STATUS_MODE_B_X_OFFSET;
	const int16_t STATUS_MODE_B_Y_OFFSET;
#endif
	const uint16_t STATUS_ERR_OFFSET_X;
	const uint16_t STATUS_ERR_OFFSET_Y;
	const uint16_t STATUS_ERR_WIDTH;
	const uint16_t STATUS_ERR_HEIGHT;
	const uint8_t TEXTBAR_FONT;
	// bottom buttons
	const uint16_t BOTTOM_BUTTONS_BLOCK_HEIGHT;
	const uint16_t BOTTOM_BUTTONS_BLOCK_TOP;
	const uint16_t BOTTOM_BUTTONS_COUNT;
	const uint16_t BOTTOM_BUTTONS_ONE_WIDTH;
	// FFT and waterfall
	const uint16_t FFT_HEIGHT_STYLE1;
	const uint16_t WTF_HEIGHT_STYLE1;
	const uint16_t FFT_HEIGHT_STYLE2;
	const uint16_t WTF_HEIGHT_STYLE2;
	const uint16_t FFT_HEIGHT_STYLE3;
	const uint16_t WTF_HEIGHT_STYLE3;
	const uint16_t FFT_HEIGHT_STYLE4;
	const uint16_t WTF_HEIGHT_STYLE4;
	const uint16_t FFT_HEIGHT_STYLE5;
	const uint16_t WTF_HEIGHT_STYLE5;
	const uint16_t FFT_PRINT_SIZE;
	const uint16_t FFT_CWDECODER_OFFSET;
	const uint16_t FFT_FFTWTF_POS_Y;
	const uint16_t FFT_FFTWTF_BOTTOM;
	const uint16_t FFT_FREQLABELS_HEIGHT;
	// System menu
	const uint8_t SYSMENU_FONT_SIZE;
	const uint16_t SYSMENU_X1;
	const uint16_t SYSMENU_X2;
	const uint16_t SYSMENU_W;
	const uint16_t SYSMENU_ITEM_HEIGHT;
	const uint16_t SYSMENU_MAX_ITEMS_ON_PAGE;
	// Touch system menu
	const uint16_t SYSMENU_BUTTON_HEIGHT;
	const uint16_t SYSMENU_BUTTON_WIDTH;
	const uint16_t SYSMENU_BUTTON_MARGIN;
	// Stuff
	const uint16_t GREETINGS_X;
	const uint16_t GREETINGS_Y;
	// Tooltip
	const GFXfont *TOOLTIP_FONT;
	const uint16_t TOOLTIP_TIMEOUT;
	const uint16_t TOOLTIP_MARGIN;
	const uint16_t TOOLTIP_POS_X;
	const uint16_t TOOLTIP_POS_Y;
	// BW Trapezoid
	const uint16_t BW_TRAPEZ_POS_X;
	const uint16_t BW_TRAPEZ_POS_Y;
	const uint16_t BW_TRAPEZ_HEIGHT;
	const uint16_t BW_TRAPEZ_WIDTH;
	// Touch buttons layout
	const uint16_t BUTTON_PADDING;
	const float32_t BUTTON_LIGHTER_WIDTH;
	const uint16_t BUTTON_LIGHTER_HEIGHT;
#if (defined(LAY_800x480))
	// Windows
	const uint16_t WINDOWS_BUTTON_WIDTH;
	const uint16_t WINDOWS_BUTTON_HEIGHT;
	const uint16_t WINDOWS_BUTTON_MARGIN;
#endif
} STRUCT_LAYOUT_THEME;

#if (defined(LAY_160x128))
#define LAYOUT_THEMES_COUNT 1
#define MAX_FFT_PRINT_SIZE LCD_WIDTH
#define MAX_FFT_HEIGHT 55
#define MAX_WTF_HEIGHT 55
#define FFT_AND_WTF_HEIGHT 55
#endif
#if (defined(LAY_320x240))
#define LAYOUT_THEMES_COUNT 1
#define MAX_FFT_PRINT_SIZE LCD_WIDTH
#define MAX_FFT_HEIGHT 60
#define MAX_WTF_HEIGHT 60
#define FFT_AND_WTF_HEIGHT 78
#endif
#if (defined(LAY_480x320))
#define MAX_FFT_PRINT_SIZE LCD_WIDTH
#ifdef FRONTPANEL_LITE
#define LAYOUT_THEMES_COUNT 1
#define MAX_FFT_HEIGHT 90
#define MAX_WTF_HEIGHT 90
#define FFT_AND_WTF_HEIGHT 130
#else
#define LAYOUT_THEMES_COUNT 2
#define MAX_FFT_HEIGHT 132
#define MAX_WTF_HEIGHT 102
#define FFT_AND_WTF_HEIGHT 152
#endif
#endif
#if (defined(LAY_800x480))
#define LAYOUT_THEMES_COUNT 8
#define MAX_FFT_PRINT_SIZE LCD_WIDTH
#define MAX_FFT_HEIGHT 275
#define MAX_WTF_HEIGHT 195
#define FFT_AND_WTF_HEIGHT 290
#endif

extern const STRUCT_LAYOUT_THEME LAYOUT_THEMES[LAYOUT_THEMES_COUNT];

#ifdef __cplusplus
}
#endif

#endif
