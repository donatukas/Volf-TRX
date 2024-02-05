#include "front_unit.h"
#include "agc.h"
#include "atu.h"
#include "audio_filters.h"
#include "auto_notch.h"
#include "functions.h"
#include "hardware.h"
#include "lcd.h"
#include "main.h"
#include "noise_reduction.h"
#include "rf_unit.h"
#include "sd.h"
#include "settings.h"
#include "system_menu.h"
#include "trx_manager.h"
#include "vad.h"

int8_t FRONTPANEL_ProcessEncoder1 = 0;
int8_t FRONTPANEL_ProcessEncoder2 = 0;

#ifdef HRDW_MCP3008_1
bool FRONTPanel_MCP3008_1_Enabled = true;
#endif
#ifdef HRDW_MCP3008_2
bool FRONTPanel_MCP3008_2_Enabled = true;
#endif
#ifdef HRDW_MCP3008_3
bool FRONTPanel_MCP3008_3_Enabled = true;
#endif

static void FRONTPANEL_ENCODER2_Rotated(int8_t direction);
static void FRONTPANEL_ENCODER2_Rotated(int8_t direction);
static void FRONTPANEL_ENC2SW_click_handler(uint32_t parameter);
static void FRONTPANEL_ENC2SW_hold_handler(uint32_t parameter);
static uint16_t FRONTPANEL_ReadMCP3008_Value(uint8_t channel, uint8_t adc_num, uint8_t count);
static void BUTTONHANDLER_W_LITE_MENU(uint32_t parameter);
static void BUTTONHANDLER_W_LITE_MENU_HOLD(uint32_t parameter);

#if !FRONTPANEL_LITE_ALEX
PERIPH_FrontPanel_Button PERIPH_FrontPanel_Buttons[] = {
    // buttons
    {.port = 1, .channel = 0, .type = FUNIT_CTRL_BUTTON}, // NONE
    {.port = 1, .channel = 1, .type = FUNIT_CTRL_BUTTON}, // NONE
    {.port = 1, .channel = 2, .type = FUNIT_CTRL_BUTTON}, // NONE
    {.port = 1, .channel = 3, .type = FUNIT_CTRL_BUTTON}, // NONE
    {.port = 1, .channel = 4, .type = FUNIT_CTRL_BUTTON}, // NONE
    {.port = 1,
     .channel = 5,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 10,
     .tres_max = 300,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = 0,
     .clickHandler = BUTTONHANDLER_BAND_P,
     .holdHandler = BUTTONHANDLER_BAND_N}, // SB1
    {.port = 1,
     .channel = 5,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 300,
     .tres_max = 500,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = 0,
     .clickHandler = BUTTONHANDLER_MODE_N,
     .holdHandler = BUTTONHANDLER_MODE_P}, // SB2
    {.port = 1,
     .channel = 5,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 500,
     .tres_max = 700,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = 0,
     .clickHandler = BUTTONHANDLER_FUNC,
     .holdHandler = BUTTONHANDLER_FUNCH}, // SB3
    {.port = 1,
     .channel = 6,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 10,
     .tres_max = 300,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = 3,
     .clickHandler = BUTTONHANDLER_FUNC,
     .holdHandler = BUTTONHANDLER_FUNCH}, // SB6
    {.port = 1,
     .channel = 6,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 300,
     .tres_max = 500,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = 2,
     .clickHandler = BUTTONHANDLER_FUNC,
     .holdHandler = BUTTONHANDLER_FUNCH}, // SB5
    {.port = 1,
     .channel = 6,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 500,
     .tres_max = 700,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = 1,
     .clickHandler = BUTTONHANDLER_FUNC,
     .holdHandler = BUTTONHANDLER_FUNCH}, // SB4
    {.port = 1,
     .channel = 7,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 10,
     .tres_max = 300,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = 4,
     .clickHandler = BUTTONHANDLER_FUNC,
     .holdHandler = BUTTONHANDLER_FUNCH}, // SB7
    {.port = 1,
     .channel = 7,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 300,
     .tres_max = 500,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = 0,
     .clickHandler = BUTTONHANDLER_W_LITE_MENU,
     .holdHandler = BUTTONHANDLER_W_LITE_MENU_HOLD}, // SB9
};
#endif

#if FRONTPANEL_LITE_ALEX
PERIPH_FrontPanel_Button PERIPH_FrontPanel_Buttons[] = {
    // buttons
    {.port = 1, .channel = 0, .type = FUNIT_CTRL_BUTTON}, // NONE
    {.port = 1, .channel = 1, .type = FUNIT_CTRL_BUTTON}, // NONE
    {.port = 1, .channel = 2, .type = FUNIT_CTRL_BUTTON}, // NONE
    {.port = 1,
     .channel = 5,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 10,
     .tres_max = 300,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = 2,
     .clickHandler = BUTTONHANDLER_FUNC,
     .holdHandler = BUTTONHANDLER_FUNCH}, // SB1
    {.port = 1,
     .channel = 5,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 300,
     .tres_max = 500,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = 1,
     .clickHandler = BUTTONHANDLER_FUNC,
     .holdHandler = BUTTONHANDLER_FUNCH}, // SB2
    {.port = 1,
     .channel = 5,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 500,
     .tres_max = 700,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = 0,
     .clickHandler = BUTTONHANDLER_FUNC,
     .holdHandler = BUTTONHANDLER_FUNCH}, // SB3
    {.port = 1,
     .channel = 6,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 10,
     .tres_max = 300,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = 0,
     .clickHandler = BUTTONHANDLER_W_LITE_MENU,
     .holdHandler = BUTTONHANDLER_W_LITE_MENU_HOLD}, // SB6
    {.port = 1,
     .channel = 6,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 300,
     .tres_max = 500,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = 4,
     .clickHandler = BUTTONHANDLER_FUNC,
     .holdHandler = BUTTONHANDLER_FUNCH}, // SB5
    {.port = 1,
     .channel = 6,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 500,
     .tres_max = 700,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = 3,
     .clickHandler = BUTTONHANDLER_FUNC,
     .holdHandler = BUTTONHANDLER_FUNCH}, // SB4
    {.port = 1,
     .channel = 7,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 10,
     .tres_max = 300,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = 0,
     .clickHandler = BUTTONHANDLER_BAND_P,
     .holdHandler = BUTTONHANDLER_BAND_N}, // SB7
    {.port = 1,
     .channel = 7,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 500,
     .tres_max = 700,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = 0,
     .clickHandler = BUTTONHANDLER_DRV_ONLY,
     .holdHandler = BUTTONHANDLER_DRV_ONLY}, // SB9
    {.port = 1,
     .channel = 7,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 700,
     .tres_max = 900,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = 0,
     .clickHandler = BUTTONHANDLER_ATT,
     .holdHandler = BUTTONHANDLER_ATTHOLD}, // SB9
    {.port = 1,
     .channel = 7,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 300,
     .tres_max = 500,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = 0,
     .clickHandler = BUTTONHANDLER_MODE_P,
     .holdHandler = BUTTONHANDLER_MODE_N}, // SB9
};
#endif

const PERIPH_FrontPanel_FuncButton PERIPH_FrontPanel_FuncButtonsList[FUNCBUTTONS_COUNT] = {
    {.name = "ZOOM-", .work_in_menu = false, .clickHandler = BUTTONHANDLER_ZOOM_N, .holdHandler = BUTTONHANDLER_ZOOM_N, .checkBool = NULL},
    {.name = "ZOOM+", .work_in_menu = false, .clickHandler = BUTTONHANDLER_ZOOM_P, .holdHandler = BUTTONHANDLER_ZOOM_P, .checkBool = NULL},
    {.name = "BAND-", .work_in_menu = false, .clickHandler = BUTTONHANDLER_BAND_N, .holdHandler = BUTTONHANDLER_MODE_N, .checkBool = NULL},
    {.name = "BAND+", .work_in_menu = false, .clickHandler = BUTTONHANDLER_BAND_P, .holdHandler = BUTTONHANDLER_MODE_P, .checkBool = NULL},
    {.name = "MUTE", .work_in_menu = false, .clickHandler = BUTTONHANDLER_MUTE, .holdHandler = BUTTONHANDLER_MUTE_AFAMP, .checkBool = (uint32_t *)&TRX.AutoGain},
    //
    {.name = "POWER", .work_in_menu = true, .clickHandler = BUTTONHANDLER_RF_POWER, .holdHandler = BUTTONHANDLER_RF_POWER, .checkBool = NULL},
    {.name = "BW", .work_in_menu = true, .clickHandler = BUTTONHANDLER_BW, .holdHandler = BUTTONHANDLER_BW, .checkBool = NULL},
    {.name = "CESSB", .work_in_menu = true, .clickHandler = BUTTONHANDLER_CESSB, .holdHandler = BUTTONHANDLER_CESSB_HOLD, .checkBool = NULL},
    {.name = "SERV", .work_in_menu = true, .clickHandler = BUTTONHANDLER_SERVICES, .holdHandler = BUTTONHANDLER_SERVICES, .checkBool = NULL},
    {.name = "WPM", .work_in_menu = true, .clickHandler = BUTTONHANDLER_WPM, .holdHandler = BUTTONHANDLER_WPM, .checkBool = NULL},
    //
    {.name = "AGC", .work_in_menu = true, .clickHandler = BUTTONHANDLER_AGC, .holdHandler = BUTTONHANDLER_AGC_MaxGain, .checkBool = (uint32_t *)&TRX.AGC_shadow},
    {.name = "DRV", .work_in_menu = false, .clickHandler = BUTTONHANDLER_DRV_ONLY, .holdHandler = BUTTONHANDLER_DRV_ONLY, .checkBool = (uint32_t *)&TRX.LNA},
    {.name = "NOTCH", .work_in_menu = false, .clickHandler = BUTTONHANDLER_NOTCH, .holdHandler = BUTTONHANDLER_NOTCH_MANUAL, .checkBool = (uint32_t *)&TRX.Notch_on_shadow},
    {.name = "DNR", .work_in_menu = false, .clickHandler = BUTTONHANDLER_DNR, .holdHandler = BUTTONHANDLER_DNR, .checkBool = NULL},
    {.name = "NB", .work_in_menu = false, .clickHandler = BUTTONHANDLER_NB, .holdHandler = BUTTONHANDLER_NB_HOLD, .checkBool = NULL},
    //
    {.name = "ATT", .work_in_menu = false, .clickHandler = BUTTONHANDLER_ATT, .holdHandler = BUTTONHANDLER_ATTHOLD, .checkBool = (uint32_t *)&TRX.ATT},
    {.name = "A<>B", .work_in_menu = false, .clickHandler = BUTTONHANDLER_AsB, .holdHandler = BUTTONHANDLER_AsB, .checkBool = NULL},
    {.name = "B=A", .work_in_menu = false, .clickHandler = BUTTONHANDLER_ArB, .holdHandler = BUTTONHANDLER_ArB, .checkBool = NULL},
    {.name = "RIT", .work_in_menu = false, .clickHandler = BUTTONHANDLER_RIT, .holdHandler = BUTTONHANDLER_XIT, .checkBool = (uint32_t *)&TRX.RIT_Enabled},
    {.name = "SPLIT", .work_in_menu = false, .clickHandler = BUTTONHANDLER_SPLIT, .holdHandler = BUTTONHANDLER_SPLIT, .checkBool = (uint32_t *)&TRX.SPLIT_Enabled},
    //
    {.name = "MODE-", .work_in_menu = false, .clickHandler = BUTTONHANDLER_MODE_N, .holdHandler = BUTTONHANDLER_MODE_N, .checkBool = NULL},
    {.name = "MODE+", .work_in_menu = false, .clickHandler = BUTTONHANDLER_MODE_P, .holdHandler = BUTTONHANDLER_MODE_P, .checkBool = NULL},
    {.name = "SNAP<", .work_in_menu = true, .clickHandler = BUTTONHANDLER_SNAP_LEFT, .holdHandler = BUTTONHANDLER_AUTO_SNAP, .checkBool = (uint32_t *)&TRX.Auto_Snap},
    {.name = "SNAP>", .work_in_menu = true, .clickHandler = BUTTONHANDLER_SNAP_RIGHT, .holdHandler = BUTTONHANDLER_AUTO_SNAP, .checkBool = (uint32_t *)&TRX.Auto_Snap},
    {.name = "FAST", .work_in_menu = false, .clickHandler = BUTTONHANDLER_FAST, .holdHandler = BUTTONHANDLER_STEP, .checkBool = (uint32_t *)&TRX.Fast},
    //
    {.name = "M>VFO", .work_in_menu = false, .clickHandler = BUTTONHANDLER_MEMO_READ, .holdHandler = BUTTONHANDLER_MEMO_READ, .checkBool = NULL},
    {.name = "VFO>M", .work_in_menu = false, .clickHandler = BUTTONHANDLER_MEMO_WRITE, .holdHandler = BUTTONHANDLER_MEMO_WRITE, .checkBool = NULL},
    {.name = "SCAN", .work_in_menu = false, .clickHandler = BUTTONHANDLER_SCAN, .holdHandler = BUTTONHANDLER_SCAN, .checkBool = (uint32_t *)&TRX_ScanMode},
    {.name = "TUNE", .work_in_menu = true, .clickHandler = BUTTONHANDLER_TUNE, .holdHandler = BUTTONHANDLER_TUNE, .checkBool = (uint32_t *)&TRX_Tune},
    {.name = "SQL", .work_in_menu = false, .clickHandler = BUTTONHANDLER_SQL, .holdHandler = BUTTONHANDLER_SQUELCH, .checkBool = (uint32_t *)&TRX.SQL_shadow},
    //
    {.name = TRX.CW_Macros_Name_1, .work_in_menu = false, .clickHandler = BUTTONHANDLER_CW_MACROS, .holdHandler = BUTTONHANDLER_CW_MACROS, .parameter = 1},
    {.name = TRX.CW_Macros_Name_2, .work_in_menu = false, .clickHandler = BUTTONHANDLER_CW_MACROS, .holdHandler = BUTTONHANDLER_CW_MACROS, .parameter = 2},
    {.name = TRX.CW_Macros_Name_3, .work_in_menu = false, .clickHandler = BUTTONHANDLER_CW_MACROS, .holdHandler = BUTTONHANDLER_CW_MACROS, .parameter = 3},
    {.name = TRX.CW_Macros_Name_4, .work_in_menu = false, .clickHandler = BUTTONHANDLER_CW_MACROS, .holdHandler = BUTTONHANDLER_CW_MACROS, .parameter = 4},
    {.name = TRX.CW_Macros_Name_5, .work_in_menu = false, .clickHandler = BUTTONHANDLER_CW_MACROS, .holdHandler = BUTTONHANDLER_CW_MACROS, .parameter = 5},
    //
    // hidden entry for menu editor
    {.name = "NONE", .work_in_menu = false, .clickHandler = NULL, .holdHandler = NULL, .checkBool = NULL},
};

PERIPH_FrontPanel_Button PERIPH_FrontPanel_TANGENT_MH48[6] = {
    {.port = 1,
     .channel = 1,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 1700,
     .tres_max = 2200,
     .state = false,
     .prev_state = false,
     .work_in_menu = false,
     .parameter = 0,
     .clickHandler = BUTTONHANDLER_DOWN,
     .holdHandler = BUTTONHANDLER_DOWN}, // PTT_SW1 - DOWN 1908 200
    {.port = 1,
     .channel = 1,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 1000,
     .tres_max = 1400,
     .state = false,
     .prev_state = false,
     .work_in_menu = false,
     .parameter = 0,
     .clickHandler = BUTTONHANDLER_UP,
     .holdHandler = BUTTONHANDLER_UP}, // PTT_SW1 - UP 1163 200
    {.port = 1,
     .channel = 2,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 1000,
     .tres_max = 1450,
     .state = false,
     .prev_state = false,
     .work_in_menu = false,
     .parameter = 0,
     .clickHandler = BUTTONHANDLER_TUNE,
     .holdHandler = BUTTONHANDLER_TUNER}, // PTT_SW2 - P1 3000 1300
    {.port = 1,
     .channel = 2,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 1500,
     .tres_max = 2050,
     .state = false,
     .prev_state = false,
     .work_in_menu = false,
     .parameter = 0,
     .clickHandler = BUTTONHANDLER_AsB,
     .holdHandler = BUTTONHANDLER_ArB}, // PTT_SW2 - P2 3000 2050
    {.port = 1,
     .channel = 2,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 2300,
     .tres_max = 2600,
     .state = false,
     .prev_state = false,
     .work_in_menu = false,
     .parameter = 0,
     .clickHandler = BUTTONHANDLER_BAND_N,
     .holdHandler = BUTTONHANDLER_MODE_N}, // PTT_SW2 - P3 3000 2600
    {.port = 1,
     .channel = 2,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 2700,
     .tres_max = 3100,
     .state = false,
     .prev_state = false,
     .work_in_menu = false,
     .parameter = 0,
     .clickHandler = BUTTONHANDLER_BAND_P,
     .holdHandler = BUTTONHANDLER_MODE_P}, // PTT_SW2 - P4 3000 3050
};

void FRONTPANEL_ENCODER_Rotated(float32_t direction) // rotated encoder, handler here, direction -1 - left, 1 - right
{
	TRX_Inactive_Time = 0;
	if (TRX.Locked || LCD_window.opened) {
		return;
	}
	if (LCD_systemMenuOpened) {
		FRONTPANEL_ProcessEncoder1 = (int8_t)direction;
		return;
	}
	if (fabsf(direction) <= ENCODER_MIN_RATE_ACCELERATION) {
		direction = (direction < 0.0f) ? -1.0f : 1.0f;
	}

	if (TRX_on_TX) {
		if (direction > 0 || TRX.RF_Gain > 0) {
			TRX.RF_Gain += direction;
		}
		if (TRX.RF_Gain > 100) {
			TRX.RF_Gain = 100;
		}

		int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
		if (band >= 0) {
			TRX.BANDS_SAVED_SETTINGS[band].RF_Gain = TRX.RF_Gain;
		}

		ATU_TunePowerStabilized = false;

		char sbuff[32] = {0};
		sprintf(sbuff, "Power: %u", TRX.RF_Gain);
		LCD_showTooltip(sbuff);

		return;
	}

	TRX_DoFrequencyEncoder(direction, false);

	LCD_UpdateQuery.FreqInfo = true;
	NeedSaveSettings = true;
}

static void FRONTPANEL_ENCODER2_Rotated(int8_t direction) // rotated encoder, handler here, direction -1 - left, 1 - right
{
	TRX_Inactive_Time = 0;
	if (TRX.Locked || LCD_window.opened) {
		return;
	}

	if (LCD_systemMenuOpened) {
		SYSMENU_eventSecRotateSystemMenu(direction);
		return;
	}

	FRONTPANEL_ENC2SW_validate();

	if (TRX.ENC2_func_mode == ENC_FUNC_PAGER) // pager
	{
		if (direction < 0) {
			BUTTONHANDLER_LEFT_ARR(0);
		}
		if (direction > 0) {
			BUTTONHANDLER_RIGHT_ARR(0);
		}
	}

	if (TRX.ENC2_func_mode == ENC_FUNC_FAST_STEP) // fast step
	{
		if (TRX_on_TX) {
			return;
		}

		TRX_DoFrequencyEncoder(direction, true);

		LCD_UpdateQuery.FreqInfo = true;
	}

	if (TRX.ENC2_func_mode == ENC_FUNC_SET_WPM) {
		// ENC2 Func mode (WPM)
		TRX.CW_KEYER_WPM += direction;
		if (TRX.CW_KEYER_WPM < 1) {
			TRX.CW_KEYER_WPM = 1;
		}
		if (TRX.CW_KEYER_WPM > 200) {
			TRX.CW_KEYER_WPM = 200;
		}
		char sbuff[32] = {0};
		sprintf(sbuff, "WPM: %u", TRX.CW_KEYER_WPM);
		LCD_showTooltip(sbuff);
	}

	if (TRX.ENC2_func_mode == ENC_FUNC_SET_RIT) // Fine RIT/XIT
	{
		if ((!TRX.XIT_Enabled || !TRX_on_TX) && TRX.RIT_Enabled && TRX.FineRITTune) {
			TRX_RIT += direction * 10;
			if (TRX_RIT > TRX.RIT_INTERVAL) {
				TRX_RIT = TRX.RIT_INTERVAL;
			}
			if (TRX_RIT < -TRX.RIT_INTERVAL) {
				TRX_RIT = -TRX.RIT_INTERVAL;
			}

			TRX_setFrequency(CurrentVFO->Freq, CurrentVFO);
		}

		if ((!TRX.RIT_Enabled || TRX_on_TX) && TRX.XIT_Enabled && TRX.FineRITTune) {
			TRX_XIT += direction * 10;
			if (TRX_XIT > TRX.XIT_INTERVAL) {
				TRX_XIT = TRX.XIT_INTERVAL;
			}
			if (TRX_XIT < -TRX.XIT_INTERVAL) {
				TRX_XIT = -TRX.XIT_INTERVAL;
			}

			TRX_setFrequency(CurrentVFO->Freq, CurrentVFO);
		}
	}

	if (TRX.ENC2_func_mode == ENC_FUNC_SET_NOTCH) // Manual Notch
	{
		if (CurrentVFO->ManualNotchFilter) {
			float64_t step = (float64_t)TRX.NOTCH_STEP_Hz;

			if (CurrentVFO->NotchFC > step && direction < 0) {
				CurrentVFO->NotchFC -= step;
			} else if (direction > 0) {
				CurrentVFO->NotchFC += step;
			}

			CurrentVFO->NotchFC = round((float64_t)CurrentVFO->NotchFC / step) * step;

			if (CurrentVFO->NotchFC < step) {
				CurrentVFO->NotchFC = step;
			}

			LCD_UpdateQuery.StatusInfoGUI = true;
			NeedReinitNotch = true;
			NeedWTFRedraw = true;
		}
	}

	if (TRX.ENC2_func_mode == ENC_FUNC_SET_LPF) // LPF
	{
		if (!TRX_on_TX) {
			if (CurrentVFO->Mode == TRX_MODE_CW) {
				SYSMENU_HANDL_FILTER_CW_LPF_pass(direction);
			}
			if (CurrentVFO->Mode == TRX_MODE_LSB || CurrentVFO->Mode == TRX_MODE_USB || CurrentVFO->Mode == TRX_MODE_DIGI_U || CurrentVFO->Mode == TRX_MODE_RTTY) {
				SYSMENU_HANDL_FILTER_SSB_LPF_RX_pass(direction);
			}
			if (CurrentVFO->Mode == TRX_MODE_AM || CurrentVFO->Mode == TRX_MODE_SAM_STEREO || CurrentVFO->Mode == TRX_MODE_SAM_LSB || CurrentVFO->Mode == TRX_MODE_SAM_USB) {
				SYSMENU_HANDL_FILTER_AM_LPF_RX_pass(direction);
			}
			if (CurrentVFO->Mode == TRX_MODE_NFM) {
				SYSMENU_HANDL_FILTER_FM_LPF_RX_pass(direction);
			}
			if (CurrentVFO->Mode == TRX_MODE_DIGI_L) {
				SYSMENU_HANDL_FILTER_DIGI_LPF_pass(direction);
			}
		} else {
			if (CurrentVFO->Mode == TRX_MODE_CW) {
				SYSMENU_HANDL_FILTER_CW_LPF_pass(direction);
			}
			if (CurrentVFO->Mode == TRX_MODE_LSB || CurrentVFO->Mode == TRX_MODE_USB || CurrentVFO->Mode == TRX_MODE_DIGI_U || CurrentVFO->Mode == TRX_MODE_RTTY) {
				SYSMENU_HANDL_FILTER_SSB_LPF_TX_pass(direction);
			}
			if (CurrentVFO->Mode == TRX_MODE_AM || CurrentVFO->Mode == TRX_MODE_SAM_STEREO || CurrentVFO->Mode == TRX_MODE_SAM_LSB || CurrentVFO->Mode == TRX_MODE_SAM_USB) {
				SYSMENU_HANDL_FILTER_AM_LPF_TX_pass(direction);
			}
			if (CurrentVFO->Mode == TRX_MODE_NFM) {
				SYSMENU_HANDL_FILTER_FM_LPF_TX_pass(direction);
			}
			if (CurrentVFO->Mode == TRX_MODE_DIGI_L) {
				SYSMENU_HANDL_FILTER_DIGI_LPF_pass(direction);
			}
		}
	}

	if (TRX.ENC2_func_mode == ENC_FUNC_SET_HPF) // HPF
	{
		if (!TRX_on_TX) {
			if (CurrentVFO->Mode == TRX_MODE_LSB || CurrentVFO->Mode == TRX_MODE_USB || CurrentVFO->Mode == TRX_MODE_DIGI_U || CurrentVFO->Mode == TRX_MODE_RTTY) {
				SYSMENU_HANDL_FILTER_SSB_HPF_RX_pass(direction);
			}
			if (CurrentVFO->Mode == TRX_MODE_NFM) {
				SYSMENU_HANDL_FILTER_FM_HPF_RX_pass(direction);
			}
		} else {
			if (CurrentVFO->Mode == TRX_MODE_LSB || CurrentVFO->Mode == TRX_MODE_USB || CurrentVFO->Mode == TRX_MODE_DIGI_U || CurrentVFO->Mode == TRX_MODE_RTTY) {
				SYSMENU_HANDL_FILTER_SSB_HPF_TX_pass(direction);
			}
		}
	}

	if (TRX.ENC2_func_mode == ENC_FUNC_SET_SQL) // SQL
	{
		CurrentVFO->FM_SQL_threshold_dBm += direction * 1;
		TRX.FM_SQL_threshold_dBm_shadow = CurrentVFO->FM_SQL_threshold_dBm;

		if (CurrentVFO->FM_SQL_threshold_dBm > 0) {
			CurrentVFO->FM_SQL_threshold_dBm = 0;
		}
		if (CurrentVFO->FM_SQL_threshold_dBm < -126) {
			CurrentVFO->FM_SQL_threshold_dBm = -126;
		}

		int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
		if (band >= 0) {
			TRX.BANDS_SAVED_SETTINGS[band].FM_SQL_threshold_dBm = CurrentVFO->FM_SQL_threshold_dBm;
		}

		LCD_UpdateQuery.StatusInfoBarRedraw = true;
	}

	if (TRX.ENC2_func_mode == ENC_FUNC_SET_VOLUME) // volume
	{
		// ENC2 Volume control
		if (direction < 0 && TRX.Volume < TRX.Volume_Step) {
			TRX.Volume = 0;
		}
		if (direction > 0 || TRX.Volume >= TRX.Volume_Step) {
			TRX.Volume += direction * TRX.Volume_Step;
		}
		if (TRX.Volume > 100) {
			TRX.Volume = 100;
		}
		char sbuff[32] = {0};
		sprintf(sbuff, "Vol: %u%%", TRX.Volume);
		LCD_showTooltip(sbuff);
	}

	if (TRX.ENC2_func_mode == ENC_FUNC_SET_IF) // IF
	{
		if (TRX.IF_Gain > 0 || direction > 0) {
			TRX.IF_Gain += direction * 1;
		}
		if (TRX.IF_Gain < CALIBRATE.IF_GAIN_MIN) {
			TRX.IF_Gain = CALIBRATE.IF_GAIN_MIN;
		}
		if (TRX.IF_Gain > CALIBRATE.IF_GAIN_MAX) {
			TRX.IF_Gain = CALIBRATE.IF_GAIN_MAX;
		}

		int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
		if (band > 0) {
			TRX.BANDS_SAVED_SETTINGS[band].IF_Gain = TRX.IF_Gain;
		}

		LCD_UpdateQuery.StatusInfoBar = true;
	}

	if (TRX.ENC2_func_mode == ENC_FUNC_SET_MEM) // Memory
	{
		if (direction > 0) {
			TRX_MemoryChannelSelected++;
			if (TRX_MemoryChannelSelected > 19) {
				TRX_MemoryChannelSelected = 0;
			}
		} else {
			TRX_MemoryChannelSelected--;
			if (TRX_MemoryChannelSelected < 0) {
				TRX_MemoryChannelSelected = 19;
			}
		}
		LCD_UpdateQuery.FreqInfoRedraw = true;
	}
}

void FRONTPANEL_check_ENC2SW(bool state) {
// check touchpad events
#ifdef HAS_TOUCHPAD
	return;
#endif

	static uint32_t menu_enc2_click_starttime = 0;
	static bool ENC2SW_Last = true;
	static bool ENC2SW_clicked = false;
	static bool ENC2SW_hold_start = false;
	static bool ENC2SW_holded = false;
	ENC2SW_clicked = false;
	ENC2SW_holded = false;

	bool ENC2SW_AND_TOUCH_Now = HAL_GPIO_ReadPin(ENC2_SW_GPIO_Port, ENC2_SW_Pin);
	// check hold and click
	if (ENC2SW_Last != ENC2SW_AND_TOUCH_Now) {
		ENC2SW_Last = ENC2SW_AND_TOUCH_Now;
		if (!ENC2SW_AND_TOUCH_Now) {
			menu_enc2_click_starttime = HAL_GetTick();
			ENC2SW_hold_start = true;
		}
	}
	if (!ENC2SW_AND_TOUCH_Now && ENC2SW_hold_start) {
		if ((HAL_GetTick() - menu_enc2_click_starttime) > KEY_HOLD_TIME) {
			ENC2SW_holded = true;
			ENC2SW_hold_start = false;
		}
	}
	if (ENC2SW_AND_TOUCH_Now && ENC2SW_hold_start) {
		if ((HAL_GetTick() - menu_enc2_click_starttime) > 1) {
			ENC2SW_clicked = true;
			ENC2SW_hold_start = false;
		}
	}

	// ENC2 Button hold
	if (ENC2SW_holded) {
		FRONTPANEL_ENC2SW_hold_handler(0);
	}

	// ENC2 Button click
	if (ENC2SW_clicked) {
		if (TRX.Locked) {
			return;
		}

		menu_enc2_click_starttime = HAL_GetTick();
		FRONTPANEL_ENC2SW_click_handler(0);
	}
}

static void FRONTPANEL_ENC2SW_click_handler(uint32_t parameter) {
	TRX_Inactive_Time = 0;
	// ENC2 CLICK
	if (!LCD_systemMenuOpened && !LCD_window.opened) {
		TRX.ENC2_func_mode++; // enc2 rotary mode

		if (TRX.ENC2_func_mode == ENC_FUNC_PAGER) { // disabled
			TRX.ENC2_func_mode++;
		}
		if (TRX.ENC2_func_mode == ENC_FUNC_SET_WPM && CurrentVFO->Mode != TRX_MODE_CW) { // no WPM if not CW
			TRX.ENC2_func_mode++;
		}
		if (TRX.ENC2_func_mode == ENC_FUNC_SET_RIT && ((!TRX.RIT_Enabled && !TRX.XIT_Enabled) || !TRX.FineRITTune)) { // nothing to RIT tune
			TRX.ENC2_func_mode++;
		}
		if (TRX.ENC2_func_mode == ENC_FUNC_SET_NOTCH && !CurrentVFO->ManualNotchFilter) { // nothing to NOTCH tune
			TRX.ENC2_func_mode++;
		}
		if (TRX.ENC2_func_mode == ENC_FUNC_SET_LPF && CurrentVFO->Mode == TRX_MODE_WFM) { // nothing to LPF tune
			TRX.ENC2_func_mode++;
		}
		if (TRX.ENC2_func_mode == ENC_FUNC_SET_HPF) { // disabled
			TRX.ENC2_func_mode++;
		}
		if (TRX.ENC2_func_mode == ENC_FUNC_SET_SQL && ((CurrentVFO->Mode != TRX_MODE_NFM && CurrentVFO->Mode != TRX_MODE_WFM) || !CurrentVFO->SQL)) { // nothing to SQL tune
			TRX.ENC2_func_mode++;
		}
		if (TRX.ENC2_func_mode > ENC_FUNC_SET_MEM) {
			TRX.ENC2_func_mode = ENC_FUNC_FAST_STEP;
		}

		LCD_UpdateQuery.StatusInfoGUI = true;

		if (TRX.ENC2_func_mode == ENC_FUNC_PAGER) {
			LCD_showTooltip("BUTTONS");
		}
		if (TRX.ENC2_func_mode == ENC_FUNC_FAST_STEP) {
			LCD_showTooltip("FAST STEP");
		}
		if (TRX.ENC2_func_mode == ENC_FUNC_SET_WPM) {
			LCD_showTooltip("SET WPM");
		}
		if (TRX.ENC2_func_mode == ENC_FUNC_SET_RIT) {
			LCD_showTooltip("SET RIT");
		}
		if (TRX.ENC2_func_mode == ENC_FUNC_SET_NOTCH) {
			LCD_showTooltip("SET NOTCH");
		}
		if (TRX.ENC2_func_mode == ENC_FUNC_SET_LPF) {
			LCD_showTooltip("SET LPF");
		}
		if (TRX.ENC2_func_mode == ENC_FUNC_SET_HPF) {
			LCD_showTooltip("SET HPF");
		}
		if (TRX.ENC2_func_mode == ENC_FUNC_SET_SQL) {
			LCD_showTooltip("SET SQL");
		}
		if (TRX.ENC2_func_mode == ENC_FUNC_SET_VOLUME) {
			LCD_showTooltip("VOLUME");
		}
		if (TRX.ENC2_func_mode == ENC_FUNC_SET_IF) {
			LCD_showTooltip("SET IF");
		}
		if (TRX.ENC2_func_mode == ENC_FUNC_SET_MEM) {
			LCD_showTooltip("MEMORY");
		}
	} else {
		if (LCD_systemMenuOpened) {
			// navigate in menu
			SYSMENU_eventSecEncoderClickSystemMenu();
		}
	}
}

static void FRONTPANEL_ENC2SW_hold_handler(uint32_t parameter) {
	TRX_Inactive_Time = 0;
	BUTTONHANDLER_LOCK(0);
}

void FRONTPANEL_Init(void) {
	uint16_t test_value = 0;
#ifdef HRDW_MCP3008_1
	test_value = FRONTPANEL_ReadMCP3008_Value(0, 1, 1);
	if (test_value == 65535) {
		FRONTPanel_MCP3008_1_Enabled = false;
		println("[ERR] Frontpanel MCP3008 - 1 not found, disabling... (FPGA SPI/I2S CLOCK ERROR?)");
		LCD_showError("MCP3008 - 1 init error (FPGA I2S CLK?)", true);
	}
#endif
#ifdef HRDW_MCP3008_2
	test_value = FRONTPANEL_ReadMCP3008_Value(0, 2, 1);
	if (test_value == 65535) {
		FRONTPanel_MCP3008_2_Enabled = false;
		println("[ERR] Frontpanel MCP3008 - 2 not found, disabling... (FPGA SPI/I2S CLOCK ERROR?)");
		LCD_showError("MCP3008 - 2 init error", true);
	}
#endif
#ifdef HRDW_MCP3008_3
	test_value = FRONTPANEL_ReadMCP3008_Value(0, 3, 1);
	if (test_value == 65535) {
		FRONTPanel_MCP3008_3_Enabled = false;
		println("[ERR] Frontpanel MCP3008 - 3 not found, disabling... (FPGA SPI/I2S CLOCK ERROR?)");
		LCD_showError("MCP3008 - 3 init error", true);
	}
#endif
	FRONTPANEL_Process();
}

void FRONTPANEL_Process(void) {
	if (LCD_systemMenuOpened && !LCD_busy && FRONTPANEL_ProcessEncoder1 != 0) {
		SYSMENU_eventRotateSystemMenu(FRONTPANEL_ProcessEncoder1);
		FRONTPANEL_ProcessEncoder1 = 0;
		TRX_ScanMode = false;
	}

	if (FRONTPANEL_ProcessEncoder2 != 0) {
		FRONTPANEL_ENCODER2_Rotated(FRONTPANEL_ProcessEncoder2);
		FRONTPANEL_ProcessEncoder2 = 0;
		TRX_ScanMode = false;
	}

#ifndef HAS_TOUCHPAD
	FRONTPANEL_check_ENC2SW(true);
#endif

#if HRDW_HAS_SD
	if (SD_USBCardReader) {
		return;
	}
#endif

	static uint32_t fu_debug_lasttime = 0;
	uint16_t buttons_count = sizeof(PERIPH_FrontPanel_Buttons) / sizeof(PERIPH_FrontPanel_Button);
	uint32_t mcp3008_value = 0;

	// process buttons
	for (uint16_t b = 0; b < buttons_count; b++) {
		if (HRDW_SPI_Locked) {
			continue;
		}

		PERIPH_FrontPanel_Button *button = &PERIPH_FrontPanel_Buttons[b];
// check disabled ports
#ifdef HRDW_MCP3008_1
		if (button->port == 1 && !FRONTPanel_MCP3008_1_Enabled) {
			continue;
		}
#endif
#ifdef HRDW_MCP3008_2
		if (button->port == 2 && !FRONTPanel_MCP3008_2_Enabled) {
			continue;
		}
#endif
#ifdef HRDW_MCP3008_3
		if (button->port == 3 && !FRONTPanel_MCP3008_3_Enabled) {
			continue;
		}
#endif

// get state from ADC MCP3008 (10bit - 1024values)
#ifdef HRDW_MCP3008_1
		if (button->port == 1) {
			mcp3008_value = FRONTPANEL_ReadMCP3008_Value(button->channel, 1, 5);
		} else
#endif
#ifdef HRDW_MCP3008_2
		    if (button->port == 2) {
			mcp3008_value = FRONTPANEL_ReadMCP3008_Value(button->channel, 2, 5);
		} else
#endif
#ifdef HRDW_MCP3008_3
		    if (button->port == 3) {
			mcp3008_value = FRONTPANEL_ReadMCP3008_Value(button->channel, 3, 5);
		} else
#endif
			continue;

		if (TRX.Debug_Type == TRX_DEBUG_BUTTONS) {
			static uint8_t fu_gebug_lastchannel = 255;
			if ((HAL_GetTick() - fu_debug_lasttime > 500 && fu_gebug_lastchannel != button->channel) || fu_debug_lasttime == 0) {
				println("F_UNIT: port ", button->port, " channel ", button->channel, " value ", mcp3008_value);
				fu_gebug_lastchannel = button->channel;
			}
		}

		// TANGENT
		if (button->type == FUNIT_CTRL_TANGENT) {
			// Yaesu MH-48
			if (CALIBRATE.TangentType == TANGENT_MH48) {
				for (uint16_t tb = 0; tb < (sizeof(PERIPH_FrontPanel_TANGENT_MH48) / sizeof(PERIPH_FrontPanel_Button)); tb++) {
					if (button->channel == PERIPH_FrontPanel_TANGENT_MH48[tb].channel) {
						FRONTPANEL_CheckButton(&PERIPH_FrontPanel_TANGENT_MH48[tb], mcp3008_value);
					}
				}
			}
		} else {
			FRONTPANEL_CheckButton(button, mcp3008_value); // other buttons / resistors
		}
	}

	if (TRX.Debug_Type == TRX_DEBUG_BUTTONS) {
		if (HAL_GetTick() - fu_debug_lasttime > 500) {
			println("");
			fu_debug_lasttime = HAL_GetTick();
		}
	}
}

void FRONTPANEL_CheckButton(PERIPH_FrontPanel_Button *button, uint16_t mcp3008_value) {
	// PTT
	if (button->type == FUNIT_CTRL_PTT) {
		static bool frontunit_ptt_state_prev = false;
		bool frontunit_ptt_state_now = false;
		if (mcp3008_value > button->tres_min && mcp3008_value < button->tres_max) {
			frontunit_ptt_state_now = true;
		}
		if (frontunit_ptt_state_prev != frontunit_ptt_state_now) {
			TRX_ptt_soft = frontunit_ptt_state_now;
			TRX_ptt_change();
			frontunit_ptt_state_prev = frontunit_ptt_state_now;
		}
	}

	// BUTTONS
	if (button->type == FUNIT_CTRL_BUTTON) {
		// set state
		if (mcp3008_value >= button->tres_min && mcp3008_value < button->tres_max) {
			TRX_Inactive_Time = 0;
			button->state = true;
			if (TRX.Debug_Type == TRX_DEBUG_BUTTONS) {
				println("Button pressed: port ", button->port, " channel ", button->channel, " value: ", mcp3008_value);
			}
		} else {
			button->state = false;
		}

		// check state
		if ((button->prev_state != button->state) && button->state) {
			button->start_hold_time = HAL_GetTick();
			button->afterhold = false;
		}

		// check hold state
		if ((button->prev_state == button->state) && button->state && ((HAL_GetTick() - button->start_hold_time) > KEY_HOLD_TIME) && !button->afterhold) {
			button->afterhold = true;
			if (!LCD_systemMenuOpened || button->work_in_menu) {
				if (!LCD_window.opened) {
					if (button->holdHandler != NULL) {
						CODEC_Beep();
						button->holdHandler(button->parameter);
					}
				} else {
					LCD_closeWindow();
				}
			}
		}

		// check click state
		if ((button->prev_state != button->state) && !button->state && ((HAL_GetTick() - button->start_hold_time) < KEY_HOLD_TIME) && !button->afterhold && !TRX.Locked) {
			if (!LCD_systemMenuOpened || button->work_in_menu) {
				if (!LCD_window.opened) {
					if (button->clickHandler != NULL) {
						CODEC_Beep();
						button->clickHandler(button->parameter);
					}
				} else {
					LCD_closeWindow();
				}
			}
		}

		// save prev state
		button->prev_state = button->state;
	}

	// DEBUG BUTTONS
	if (button->type == FUNIT_CTRL_BUTTON_DEBUG) {
		char str[64] = {0};
		sprintf(str, "%d: %d       ", button->channel, mcp3008_value);
		LCDDriver_printText(str, 10, 200 + button->channel * 20, COLOR_RED, BG_COLOR, 2);
	}
}

static uint16_t FRONTPANEL_ReadMCP3008_Value(uint8_t channel, uint8_t adc_num, uint8_t count) {
	HRDW_SPI_Locked = true;

	uint8_t outData[3] = {0};
	uint8_t inData[3] = {0};
	uint32_t mcp3008_value = 0;

	for (uint8_t i = 0; i < count; i++) {
		outData[0] = 0x18 | channel;
		bool res = false;
		if (adc_num == 1) {
			res = HRDW_FrontUnit_SPI(outData, inData, 3, false);
		}
		if (adc_num == 2) {
			res = HRDW_FrontUnit2_SPI(outData, inData, 3, false);
		}
		if (adc_num == 3) {
			res = HRDW_FrontUnit3_SPI(outData, inData, 3, false);
		}
		if (res == false) {
			HRDW_SPI_Locked = false;
			return 65535;
		}
		mcp3008_value += (uint16_t)(0 | ((inData[1] & 0x3F) << 4) | (inData[2] & 0xF0 >> 4));
	}
	mcp3008_value /= count;

	HRDW_SPI_Locked = false;

	return mcp3008_value;
}

void FRONTPANEL_ENC2SW_validate() {
	if (TRX.ENC2_func_mode == ENC_FUNC_SET_WPM && CurrentVFO->Mode != TRX_MODE_CW) { // no WPM if not CW
		TRX.ENC2_func_mode = ENC_FUNC_PAGER;
	}
	if (TRX.ENC2_func_mode == ENC_FUNC_SET_RIT && ((!TRX.RIT_Enabled && !TRX.XIT_Enabled) || !TRX.FineRITTune)) { // nothing to RIT tune
		TRX.ENC2_func_mode = ENC_FUNC_PAGER;
	}
	if (TRX.ENC2_func_mode == ENC_FUNC_SET_NOTCH && !CurrentVFO->ManualNotchFilter) { // nothing to NOTCH tune
		TRX.ENC2_func_mode = ENC_FUNC_PAGER;
	}
	if (TRX.ENC2_func_mode == ENC_FUNC_SET_LPF && CurrentVFO->Mode == TRX_MODE_WFM) { // nothing to LPF tune
		TRX.ENC2_func_mode = ENC_FUNC_PAGER;
	}
	if (TRX.ENC2_func_mode == ENC_FUNC_SET_HPF && CurrentVFO->Mode != TRX_MODE_LSB && CurrentVFO->Mode != TRX_MODE_USB) { // fast tune HPF in SSB only
		TRX.ENC2_func_mode = ENC_FUNC_PAGER;
	}
	if (TRX.ENC2_func_mode == ENC_FUNC_SET_SQL && ((CurrentVFO->Mode != TRX_MODE_NFM && CurrentVFO->Mode != TRX_MODE_WFM) || !CurrentVFO->SQL)) { // nothing to SQL tune
		TRX.ENC2_func_mode = ENC_FUNC_PAGER;
	}
}

static void BUTTONHANDLER_W_LITE_MENU(uint32_t parameter) {
	if (LCD_systemMenuOpened) {
		BUTTONHANDLER_MENU(parameter);
	} else {
		BUTTONHANDLER_RIGHT_ARR(parameter);
	}
}

static void BUTTONHANDLER_W_LITE_MENU_HOLD(uint32_t parameter) {
	if (LCD_systemMenuOpened) {
		BUTTONHANDLER_LOCK(parameter);
	} else {
		BUTTONHANDLER_MENU(parameter);
	}
}
