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

// static void FRONTPANEL_ENCODER2_Rotated(int8_t direction);
static void FRONTPANEL_ENCODER2_Rotated(int8_t direction);
static void FRONTPANEL_ENC2SW_click_handler(uint32_t parameter);
static void FRONTPANEL_ENC2SW_hold_handler(uint32_t parameter);

PERIPH_FrontPanel_Button PERIPH_FrontPanel_Buttons[] = {
    // buttons
    {.port = 1,
     .channel = 1,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 90,
     .tres_max = 110,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = 0,
     .clickHandler = BUTTONHANDLER_FUNCH,
     .holdHandler = BUTTONHANDLER_FUNCH}, // SB1
    {.port = 1,
     .channel = 2,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 90,
     .tres_max = 110,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = 1,
     .clickHandler = BUTTONHANDLER_FUNCH,
     .holdHandler = BUTTONHANDLER_FUNCH}, // SB2
    {.port = 1,
     .channel = 3,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 90,
     .tres_max = 110,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = 2,
     .clickHandler = BUTTONHANDLER_FUNC,
     .holdHandler = BUTTONHANDLER_FUNCH}, // SB3
    {.port = 1,
     .channel = 4,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 90,
     .tres_max = 110,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = 3,
     .clickHandler = BUTTONHANDLER_FUNC,
     .holdHandler = BUTTONHANDLER_FUNCH}, // SB4
    {.port = 1,
     .channel = 5,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 90,
     .tres_max = 110,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = 0,
     .clickHandler = BUTTONHANDLER_RIGHT_ARR,
     .holdHandler = BUTTONHANDLER_LEFT_ARR}, // SB5
    {.port = 1,
     .channel = 6,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 90,
     .tres_max = 110,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = 0,
     .clickHandler = BUTTONHANDLER_BAND_P,
     .holdHandler = BUTTONHANDLER_BAND_N}, // SB6
    {.port = 1,
     .channel = 7,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 90,
     .tres_max = 110,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = 0,
     .clickHandler = BUTTONHANDLER_MENU,
     .holdHandler = BUTTONHANDLER_LOCK}, // SB7
};

const PERIPH_FrontPanel_FuncButton PERIPH_FrontPanel_FuncButtonsList[FUNCBUTTONS_COUNT] = {
    {.name = "BAND-", .work_in_menu = false, .clickHandler = BUTTONHANDLER_BAND_N, .holdHandler = BUTTONHANDLER_BAND_N, .checkBool = NULL},
    {.name = "BAND+", .work_in_menu = false, .clickHandler = BUTTONHANDLER_BAND_P, .holdHandler = BUTTONHANDLER_BAND_P, .checkBool = NULL},
    {.name = "ZOOM-", .work_in_menu = false, .clickHandler = BUTTONHANDLER_ZOOM_N, .holdHandler = BUTTONHANDLER_ZOOM_N, .checkBool = NULL},
    {.name = "ZOOM+", .work_in_menu = false, .clickHandler = BUTTONHANDLER_ZOOM_P, .holdHandler = BUTTONHANDLER_ZOOM_P, .checkBool = NULL},

    {.name = "A<>B", .work_in_menu = false, .clickHandler = BUTTONHANDLER_AsB, .holdHandler = BUTTONHANDLER_AsB, .checkBool = NULL},
    {.name = "A->B", .work_in_menu = false, .clickHandler = BUTTONHANDLER_ArB, .holdHandler = BUTTONHANDLER_ArB, .checkBool = NULL},
    {.name = "RIT", .work_in_menu = false, .clickHandler = BUTTONHANDLER_RIT, .holdHandler = BUTTONHANDLER_XIT, .checkBool = (uint32_t *)&TRX.RIT_Enabled},
    {.name = "SPLIT", .work_in_menu = false, .clickHandler = BUTTONHANDLER_SPLIT, .holdHandler = BUTTONHANDLER_SPLIT, .checkBool = NULL}, //(uint32_t *)&TRX.SPLIT_Enabled},

    {.name = "M>VFO", .work_in_menu = false, .clickHandler = BUTTONHANDLER_MEMO_READ, .holdHandler = BUTTONHANDLER_MEMO_READ, .checkBool = NULL},
    {.name = "VFO>M", .work_in_menu = false, .clickHandler = BUTTONHANDLER_MEMO_WRITE, .holdHandler = BUTTONHANDLER_MEMO_WRITE, .checkBool = NULL},
    {.name = "SCAN", .work_in_menu = false, .clickHandler = BUTTONHANDLER_SCAN, .holdHandler = BUTTONHANDLER_SCAN, .checkBool = (uint32_t *)&TRX_ScanMode},
    {.name = "TUNE", .work_in_menu = true, .clickHandler = BUTTONHANDLER_TUNE, .holdHandler = BUTTONHANDLER_TUNER, .checkBool = (uint32_t *)&TRX.TUNER_Enabled},

    {.name = "MODE-", .work_in_menu = false, .clickHandler = BUTTONHANDLER_MODE_N, .holdHandler = BUTTONHANDLER_MODE_N, .checkBool = NULL},
    {.name = "MODE+", .work_in_menu = false, .clickHandler = BUTTONHANDLER_MODE_P, .holdHandler = BUTTONHANDLER_MODE_P, .checkBool = NULL},
    {.name = "POWER", .work_in_menu = true, .clickHandler = BUTTONHANDLER_RF_POWER, .holdHandler = BUTTONHANDLER_RF_POWER, .checkBool = NULL},
    {.name = "SQL", .work_in_menu = false, .clickHandler = BUTTONHANDLER_SQL, .holdHandler = BUTTONHANDLER_SQL, .checkBool = (uint32_t *)&TRX.SQL_shadow},

    {.name = "PRE", .work_in_menu = false, .clickHandler = BUTTONHANDLER_PRE, .holdHandler = BUTTONHANDLER_PRE, .checkBool = (uint32_t *)&TRX.LNA},
    {.name = "NOTCH", .work_in_menu = false, .clickHandler = BUTTONHANDLER_NOTCH, .holdHandler = BUTTONHANDLER_NOTCH, .checkBool = (uint32_t *)&TRX.Notch_on_shadow},
    {.name = "ATT", .work_in_menu = false, .clickHandler = BUTTONHANDLER_ATT, .holdHandler = BUTTONHANDLER_ATT, .checkBool = (uint32_t *)&TRX.ATT},
    {.name = "NB", .work_in_menu = true, .clickHandler = BUTTONHANDLER_NB, .holdHandler = BUTTONHANDLER_NB_HOLD, .checkBool = NULL},

    {.name = "AGC", .work_in_menu = false, .clickHandler = BUTTONHANDLER_AGC, .holdHandler = BUTTONHANDLER_AGC, .checkBool = (uint32_t *)&TRX.AGC_shadow},
    {.name = "IF", .work_in_menu = true, .clickHandler = BUTTONHANDLER_IF, .holdHandler = BUTTONHANDLER_IF, .checkBool = NULL},
    {.name = "DNR", .work_in_menu = false, .clickHandler = BUTTONHANDLER_DNR, .holdHandler = BUTTONHANDLER_DNR, .checkBool = NULL},
    {.name = "FAST", .work_in_menu = false, .clickHandler = BUTTONHANDLER_FAST, .holdHandler = BUTTONHANDLER_FAST, .checkBool = (uint32_t *)&TRX.Fast},
    //
    {.name = "SERV", .work_in_menu = true, .clickHandler = BUTTONHANDLER_SERVICES, .holdHandler = BUTTONHANDLER_SERVICES, .checkBool = NULL},
    {.name = "WPM", .work_in_menu = true, .clickHandler = BUTTONHANDLER_WPM, .holdHandler = BUTTONHANDLER_WPM, .checkBool = NULL},
    {.name = "CQ", .work_in_menu = false, .clickHandler = BUTTONHANDLER_CQ, .holdHandler = BUTTONHANDLER_CQ, .checkBool = (uint32_t *)&SD_PlayInProcess},
    {.name = "REC", .work_in_menu = false, .clickHandler = BUTTONHANDLER_REC, .holdHandler = BUTTONHANDLER_REC, .checkBool = (uint32_t *)&SD_RecordInProcess},
    //
    {.name = "SMPL-", .work_in_menu = false, .clickHandler = BUTTONHANDLER_SAMPLE_N, .holdHandler = BUTTONHANDLER_SAMPLE_N, .checkBool = NULL},
    {.name = "SMPL+", .work_in_menu = false, .clickHandler = BUTTONHANDLER_SAMPLE_P, .holdHandler = BUTTONHANDLER_SAMPLE_P, .checkBool = NULL},
    {.name = "SNAP<", .work_in_menu = true, .clickHandler = BUTTONHANDLER_SNAP_LEFT, .holdHandler = BUTTONHANDLER_SNAP_LEFT, .checkBool = (uint32_t *)&TRX.Auto_Snap},
    {.name = "SNAP>", .work_in_menu = true, .clickHandler = BUTTONHANDLER_SNAP_RIGHT, .holdHandler = BUTTONHANDLER_SNAP_RIGHT, .checkBool = (uint32_t *)&TRX.Auto_Snap},
    //
    {.name = TRX.CW_Macros_Name_1, .work_in_menu = false, .clickHandler = BUTTONHANDLER_CW_MACROS, .holdHandler = BUTTONHANDLER_CW_MACROS, .parameter = 1},
    {.name = TRX.CW_Macros_Name_2, .work_in_menu = false, .clickHandler = BUTTONHANDLER_CW_MACROS, .holdHandler = BUTTONHANDLER_CW_MACROS, .parameter = 2},
    {.name = TRX.CW_Macros_Name_3, .work_in_menu = false, .clickHandler = BUTTONHANDLER_CW_MACROS, .holdHandler = BUTTONHANDLER_CW_MACROS, .parameter = 3},
    {.name = TRX.CW_Macros_Name_4, .work_in_menu = false, .clickHandler = BUTTONHANDLER_CW_MACROS, .holdHandler = BUTTONHANDLER_CW_MACROS, .parameter = 4},
    // hidden entry for menu editor
    {.name = TRX.CW_Macros_Name_5, .work_in_menu = false, .clickHandler = BUTTONHANDLER_CW_MACROS, .holdHandler = BUTTONHANDLER_CW_MACROS, .parameter = 5},
    {.name = "CESSB", .work_in_menu = false, .clickHandler = BUTTONHANDLER_CESSB, .holdHandler = BUTTONHANDLER_CESSB, .checkBool = (uint32_t *)&TRX.TX_CESSB},
    {.name = "FT8", .work_in_menu = false, .clickHandler = BUTTONHANDLER_FT8, .holdHandler = BUTTONHANDLER_FT8, .checkBool = NULL},
    {.name = "VOX", .work_in_menu = true, .clickHandler = BUTTONHANDLER_VOX, .holdHandler = BUTTONHANDLER_VOX, .checkBool = (uint32_t *)&TRX.VOX},
    {.name = "FILES", .work_in_menu = false, .clickHandler = BUTTONHANDLER_FILEMANAGER, .holdHandler = BUTTONHANDLER_FILEMANAGER, .checkBool = NULL},
    {.name = "REPIT", .work_in_menu = false, .clickHandler = BUTTONHANDLER_REPEATER_MODE, .holdHandler = BUTTONHANDLER_REPEATER_MODE, .checkBool = (uint32_t *)&TRX.RepeaterMode_shadow},
    {.name = "DPD", .work_in_menu = true, .clickHandler = BUTTONHANDLER_DPD, .holdHandler = BUTTONHANDLER_DPD_CALIBRATE, .checkBool = (uint32_t *)&TRX.Digital_Pre_Distortion},
    {.name = "CESSB", .work_in_menu = true, .clickHandler = BUTTONHANDLER_CESSB, .holdHandler = BUTTONHANDLER_CESSB_HOLD, .checkBool = NULL},
    {.name = "BW", .work_in_menu = true, .clickHandler = BUTTONHANDLER_BW, .holdHandler = BUTTONHANDLER_BW, .checkBool = NULL},
    {.name = "NONE", .work_in_menu = false, .clickHandler = NULL, .holdHandler = NULL, .checkBool = NULL},

};

PERIPH_FrontPanel_Button PERIPH_FrontPanel_TANGENT_MH48[6] = {
    {.port = 1,
     .channel = 1,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 1600,
     .tres_max = 2200,
     .state = false,
     .prev_state = false,
     .work_in_menu = false,
     .parameter = 0,
     .clickHandler = BUTTONHANDLER_DOWN,
     .holdHandler = BUTTONHANDLER_DOWN}, // PTT_SW1 - DOWN 2000 200
    {.port = 1,
     .channel = 1,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 1000,
     .tres_max = 1500,
     .state = false,
     .prev_state = false,
     .work_in_menu = false,
     .parameter = 0,
     .clickHandler = BUTTONHANDLER_UP,
     .holdHandler = BUTTONHANDLER_UP}, // PTT_SW1 - UP 1250 200
    {.port = 1,
     .channel = 2,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 1000,
     .tres_max = 1500,
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
     .tres_max = 2100,
     .state = false,
     .prev_state = false,
     .work_in_menu = false,
     .parameter = 0,
     .clickHandler = BUTTONHANDLER_AsB,
     .holdHandler = BUTTONHANDLER_ArB}, // PTT_SW2 - P2 3000 2050
    {.port = 1,
     .channel = 2,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 2100,
     .tres_max = 2700,
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

	if (TRX.Locked) {
		return;
	}

	// uint8_t a = (ENC2_SW_GPIO_Port->IDR & ENC2_SW_Pin);
	// uint8_t b = (ENC2_SW_GPIO_Port->IDR & 0xFF);
	// println("A: ", a, " B: ", b);

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
			TRX.FRONTPANEL_funcbuttons_page = 2;
			LCD_UpdateQuery.BottomButtons = true;
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
	BUTTONHANDLER_MENU(0);
}

void FRONTPANEL_Init(void) { FRONTPANEL_Process(); }

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

	FRONTPANEL_check_ENC2SW(true);

#if HRDW_HAS_SD
	if (SD_USBCardReader) {
		return;
	}
#endif

	static uint32_t fu_debug_lasttime = 0;
	uint16_t buttons_count = sizeof(PERIPH_FrontPanel_Buttons) / sizeof(PERIPH_FrontPanel_Button);

	// process buttons
	for (uint16_t b = 0; b < buttons_count; b++) {
		PERIPH_FrontPanel_Button *button = &PERIPH_FrontPanel_Buttons[b];

		bool button_state = false;
		if (button->channel == 1) {
			button_state = !HAL_GPIO_ReadPin(KEY_1_GPIO_Port, KEY_1_Pin);
		}
		if (button->channel == 2) {
			button_state = !HAL_GPIO_ReadPin(KEY_2_GPIO_Port, KEY_2_Pin);
		}
		if (button->channel == 3) {
			button_state = !HAL_GPIO_ReadPin(KEY_3_GPIO_Port, KEY_3_Pin);
		}
		if (button->channel == 4) {
			button_state = !HAL_GPIO_ReadPin(KEY_4_GPIO_Port, KEY_4_Pin);
		}
		if (button->channel == 5) {
			button_state = !HAL_GPIO_ReadPin(KEY_5_GPIO_Port, KEY_5_Pin);
		}
		if (button->channel == 6) {
			button_state = !HAL_GPIO_ReadPin(KEY_6_GPIO_Port, KEY_6_Pin);
		}
		if (button->channel == 7) {
			button_state = !HAL_GPIO_ReadPin(KEY_7_GPIO_Port, KEY_7_Pin);
		}

		if (TRX.Debug_Type == TRX_DEBUG_BUTTONS) {
			static uint8_t fu_gebug_lastchannel = 255;
			if ((HAL_GetTick() - fu_debug_lasttime > 500 && fu_gebug_lastchannel != button->channel) || fu_debug_lasttime == 0) {
				println("F_UNIT: port ", button->port, " channel ", button->channel, " value ", (uint8_t)button_state);
				fu_gebug_lastchannel = button->channel;
			}
		}

		FRONTPANEL_CheckButton(button, button_state ? 100 : 0); // other buttons / resistors
	}

	if (TRX.Debug_Type == TRX_DEBUG_BUTTONS) {
		if (HAL_GetTick() - fu_debug_lasttime > 500) {
			println("");
			fu_debug_lasttime = HAL_GetTick();
		}
	}
}

void FRONTPANEL_CheckButton(PERIPH_FrontPanel_Button *button, uint16_t mcp3008_value) {
	// AF_GAIN
	if (button->type == FUNIT_CTRL_AF_GAIN) {
		static float32_t AF_VOLUME_mcp3008_averaged = 0.0f;
		AF_VOLUME_mcp3008_averaged = AF_VOLUME_mcp3008_averaged * 0.6f + mcp3008_value * 0.4f;

		TRX.Volume = (uint16_t)(MAX_VOLUME_VALUE - AF_VOLUME_mcp3008_averaged);
	}

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
