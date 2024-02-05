#include "auto_calibration.h"
#include "audio_processor.h"
#include "fonts.h"
#include "front_unit.h"
#include "functions.h"
#include "lcd.h"
#include "lcd_driver.h"
#include "main.h"
#include "settings.h"
#include "trx_manager.h"

// Public variables
bool SYSMENU_auto_calibration_opened = false;

// Private Variables
static uint16_t pos_y = 0;

static int8_t AUTO_CALIBRATION_current_page = 0;
static int8_t AUTO_CALIBRATION_calibration_type = 0; // 0-swr 1-power

static bool AUTO_CALIBRATION_started = false;
static bool AUTO_CALIBRATION_finished = false;
static uint8_t AUTO_CALIBRATION_current_power_calib = 0;
static uint8_t AUTO_CALIBRATION_current_power_delay = 0;
static float32_t AUTO_CALIBRATION_prev_power = 0;
static uint8_t AUTO_CALIBRATION_current_power_low_diff = 0;

static bool LastAutogainer = false;
static uint64_t LastFreq = 0;
static bool LastLNA = false;
static bool LastDRV = false;
static bool LastPGA = false;
static bool LastATT = false;
static float32_t LastATT_DB = false;

// Prototypes
static void AUTO_CALIBRATION_DoPowerCalibration(uint8_t *calibration_parameter, uint64_t freq, char *band_name);
static bool AUTO_CALIBRATION_ProcessPowerCalibration(uint8_t *rf_calibration);

// start
static void AUTO_CALIBRATION_Start(void) {
	LCD_busy = true;

	LastLNA = TRX.LNA;
	LastDRV = TRX.ADC_Driver;
	LastPGA = TRX.ADC_PGA;
	LastATT = TRX.ATT;
	LastATT_DB = TRX.ATT_DB;
	LastAutogainer = TRX.AutoGain;
	LastFreq = CurrentVFO->Freq;

	TRX.TUNER_Enabled = false;
	TRX.ATU_Enabled = false;

	AUTO_CALIBRATION_current_page = 0;
	LCDDriver_Fill(BG_COLOR);

	LCD_busy = false;
	LCD_UpdateQuery.SystemMenu = true;
}

void AUTO_CALIBRATION_Start_SWR(void) {
	AUTO_CALIBRATION_calibration_type = 0;
	AUTO_CALIBRATION_Start();
}

void AUTO_CALIBRATION_Start_POWER(void) {
	AUTO_CALIBRATION_calibration_type = 1;
	AUTO_CALIBRATION_Start();
}

// stop
void AUTO_CALIBRATION_Stop(void) {
	TRX_Tune = false;
	TRX.AutoGain = LastAutogainer;
	TRX_setFrequency(LastFreq, CurrentVFO);

	TRX.LNA = LastLNA;
	TRX.ADC_Driver = LastDRV;
	TRX.ADC_PGA = LastPGA;
	TRX.ATT = LastATT;
	TRX.ATT_DB = LastATT_DB;
	TRX.TUNER_Enabled = true;
	TRX.ATU_Enabled = true;

	NeedSaveCalibration = true;
}

// draw
void AUTO_CALIBRATION_Draw(void) {
	if (LCD_busy) {
		LCD_UpdateQuery.SystemMenuRedraw = true;
		return;
	}
	LCD_busy = true;
	LCD_UpdateQuery.SystemMenu = false;
	LCD_UpdateQuery.SystemMenuRedraw = false;

	// predefine
#ifdef LCD_SMALL_INTERFACE
#define margin_left 5
#define margin_bottom 10
#define font_size 1
#else
#define margin_left 5
#define margin_bottom 20
#define font_size 2
#endif

	pos_y = margin_left;
	char str[64] = {0};

	// print pages
	if (AUTO_CALIBRATION_calibration_type == 0) // swr test
	{
		if (AUTO_CALIBRATION_current_page == 0) // Info HF
		{
			TRX_Tune = false;

			TRX_setFrequency(14000000, CurrentVFO);
			TRX_setMode(TRX_MODE_CW, CurrentVFO);
			TRX.ANT_RX = TRX_ANT_1;
			TRX.ANT_TX = TRX_ANT_1;
			TRX.RF_Gain = 30;

			LCDDriver_printText("Plug HF dummy load", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			LCDDriver_printText("and power meter to ANT1", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;

			// redraw loop
			LCD_UpdateQuery.SystemMenuRedraw = true;
		}
		if (AUTO_CALIBRATION_current_page == 1) // HF FWD
		{
			TRX_Tune = true;

			TRX_setFrequency(14000000, CurrentVFO);
			TRX_setMode(TRX_MODE_CW, CurrentVFO);
			TRX.ANT_RX = TRX_ANT_1;
			TRX.ANT_TX = TRX_ANT_1;
			TRX.RF_Gain = 30;

			LCDDriver_printText("14MHz, 30% power", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			LCDDriver_printText("Rotate to adjust FORWARD", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;

			sprintf(str, "%.1f W Forward", (double)TRX_PWR_Forward_SMOOTHED);
			LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;

			sprintf(str, "%.1f W Backward", (double)TRX_PWR_Backward_SMOOTHED);
			LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;

			pos_y += margin_bottom;

			sprintf(str, "%.2f W FWD Calibrate", (double)CALIBRATE.SWR_FWD_Calibration_HF);
			LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;

			sprintf(str, "%.2f W BWD Calibrate", (double)CALIBRATE.SWR_BWD_Calibration_HF);
			LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;

			// redraw loop
			LCD_UpdateQuery.SystemMenuRedraw = true;
		}
		if (AUTO_CALIBRATION_current_page == 2) // HF BWD
		{
			TRX_Tune = true;

			TRX_setFrequency(14000000, CurrentVFO);
			TRX_setMode(TRX_MODE_CW, CurrentVFO);
			TRX.ANT_RX = TRX_ANT_1;
			TRX.ANT_TX = TRX_ANT_1;
			TRX.RF_Gain = 30;

			LCDDriver_printText("14MHz, 30% power", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			LCDDriver_printText("Rotate to adjust BACKWARD", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;

			sprintf(str, "%.1f W Forward", (double)TRX_PWR_Forward_SMOOTHED);
			LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;

			sprintf(str, "%.1f W Backward", (double)TRX_PWR_Backward_SMOOTHED);
			LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;

			pos_y += margin_bottom;

			sprintf(str, "%.2f W FWD Calibrate", (double)CALIBRATE.SWR_FWD_Calibration_HF);
			LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;

			sprintf(str, "%.2f W BWD Calibrate", (double)CALIBRATE.SWR_BWD_Calibration_HF);
			LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;

			// redraw loop
			LCD_UpdateQuery.SystemMenuRedraw = true;
		}
		if (AUTO_CALIBRATION_current_page == 3) // Info 6m
		{
			TRX_Tune = false;

			TRX_setFrequency(52000000, CurrentVFO);
			TRX_setMode(TRX_MODE_CW, CurrentVFO);
			TRX.ANT_RX = TRX_ANT_1;
			TRX.ANT_TX = TRX_ANT_1;
			TRX.RF_Gain = 30;

			LCDDriver_printText("Plug 6M dummy load", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			LCDDriver_printText("and power meter to ANT1", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;

			// redraw loop
			LCD_UpdateQuery.SystemMenuRedraw = true;
		}
		if (AUTO_CALIBRATION_current_page == 4) // 6m FWD
		{
			TRX_Tune = true;

			TRX_setFrequency(52000000, CurrentVFO);
			TRX_setMode(TRX_MODE_CW, CurrentVFO);
			TRX.ANT_RX = TRX_ANT_1;
			TRX.ANT_TX = TRX_ANT_1;
			TRX.RF_Gain = 30;

			LCDDriver_printText("52MHz, 30% power", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			LCDDriver_printText("Rotate to adjust FORWARD", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;

			sprintf(str, "%.1f W Forward", (double)TRX_PWR_Forward_SMOOTHED);
			LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;

			sprintf(str, "%.1f W Backward", (double)TRX_PWR_Backward_SMOOTHED);
			LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;

			pos_y += margin_bottom;

			sprintf(str, "%.2f W FWD Calibrate", (double)CALIBRATE.SWR_FWD_Calibration_6M);
			LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;

			sprintf(str, "%.2f W BWD Calibrate", (double)CALIBRATE.SWR_BWD_Calibration_6M);
			LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;

			// redraw loop
			LCD_UpdateQuery.SystemMenuRedraw = true;
		}
		if (AUTO_CALIBRATION_current_page == 5) // 6m BWD
		{
			TRX_Tune = true;

			TRX_setFrequency(52000000, CurrentVFO);
			TRX_setMode(TRX_MODE_CW, CurrentVFO);
			TRX.ANT_RX = TRX_ANT_1;
			TRX.ANT_TX = TRX_ANT_1;
			TRX.RF_Gain = 30;

			LCDDriver_printText("52MHz, 30% power", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			LCDDriver_printText("Rotate to adjust BACKWARD", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;

			sprintf(str, "%.1f W Forward", (double)TRX_PWR_Forward_SMOOTHED);
			LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;

			sprintf(str, "%.1f W Backward", (double)TRX_PWR_Backward_SMOOTHED);
			LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;

			pos_y += margin_bottom;

			sprintf(str, "%.2f W FWD Calibrate", (double)CALIBRATE.SWR_FWD_Calibration_6M);
			LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;

			sprintf(str, "%.2f W BWD Calibrate", (double)CALIBRATE.SWR_BWD_Calibration_6M);
			LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;

			// redraw loop
			LCD_UpdateQuery.SystemMenuRedraw = true;
		}
		if (AUTO_CALIBRATION_current_page == 6) // Info VFH
		{
			TRX_Tune = false;

			TRX_setFrequency(145000000, CurrentVFO);
			TRX_setMode(TRX_MODE_CW, CurrentVFO);
			TRX.ANT_RX = TRX_ANT_1;
			TRX.ANT_TX = TRX_ANT_1;
			TRX.RF_Gain = 100;

			LCDDriver_printText("Plug VHF dummy load", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			LCDDriver_printText("and power meter to ANT1", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;

			// redraw loop
			LCD_UpdateQuery.SystemMenuRedraw = true;
		}
		if (AUTO_CALIBRATION_current_page == 7) // VHF FWD
		{
			TRX_Tune = true;

			TRX_setFrequency(145000000, CurrentVFO);
			TRX_setMode(TRX_MODE_CW, CurrentVFO);
			TRX.ANT_RX = TRX_ANT_1;
			TRX.ANT_TX = TRX_ANT_1;
			TRX.RF_Gain = 100;

			LCDDriver_printText("145MHz, 100% power", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			LCDDriver_printText("Rotate to adjust FORWARD", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;

			sprintf(str, "%.1f W Forward", (double)TRX_PWR_Forward_SMOOTHED);
			LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;

			sprintf(str, "%.1f W Backward", (double)TRX_PWR_Backward_SMOOTHED);
			LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;

			pos_y += margin_bottom;

			sprintf(str, "%.2f W FWD Calibrate", (double)CALIBRATE.SWR_FWD_Calibration_VHF);
			LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;

			sprintf(str, "%.2f W BWD Calibrate", (double)CALIBRATE.SWR_BWD_Calibration_VHF);
			LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;

			// redraw loop
			LCD_UpdateQuery.SystemMenuRedraw = true;
		}
		if (AUTO_CALIBRATION_current_page == 8) // VHF BWD
		{
			TRX_Tune = true;

			TRX_setFrequency(145000000, CurrentVFO);
			TRX_setMode(TRX_MODE_CW, CurrentVFO);
			TRX.ANT_RX = TRX_ANT_1;
			TRX.ANT_TX = TRX_ANT_1;
			TRX.RF_Gain = 100;

			LCDDriver_printText("145MHz, 100% power", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			LCDDriver_printText("Rotate to adjust BACKWARD", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;

			sprintf(str, "%.1f W Forward", (double)TRX_PWR_Forward_SMOOTHED);
			LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;

			sprintf(str, "%.1f W Backward", (double)TRX_PWR_Backward_SMOOTHED);
			LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;

			pos_y += margin_bottom;

			sprintf(str, "%.2f W FWD Calibrate", (double)CALIBRATE.SWR_FWD_Calibration_VHF);
			LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;

			sprintf(str, "%.2f W BWD Calibrate", (double)CALIBRATE.SWR_BWD_Calibration_VHF);
			LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;

			// redraw loop
			LCD_UpdateQuery.SystemMenuRedraw = true;
		}
		if (AUTO_CALIBRATION_current_page == 9) // Done
		{
			TRX_Tune = false;

			LCDDriver_printText("Calibration complete", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;

			// redraw loop
			LCD_UpdateQuery.SystemMenuRedraw = true;
		}

		if (AUTO_CALIBRATION_current_page > 9) {
			AUTO_CALIBRATION_current_page = 9;
		}
	}

	if (AUTO_CALIBRATION_calibration_type == 1) // Power calibration
	{
		if (AUTO_CALIBRATION_current_page == 0) // Start Info
		{
			TRX_Tune = false;

			LCDDriver_printText("Plug dummy load to ANT1", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;

			sprintf(str, "MAX Power %uW", CALIBRATE.MAX_RF_POWER_ON_METER);
			LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;

			// redraw loop
			LCD_UpdateQuery.SystemMenuRedraw = true;
		}

		if (AUTO_CALIBRATION_current_page == 1) {
			AUTO_CALIBRATION_DoPowerCalibration(&CALIBRATE.rf_out_power_160m, 1900 * 1000, "160M Calibration");
		}
		if (AUTO_CALIBRATION_current_page == 2) {
			AUTO_CALIBRATION_DoPowerCalibration(&CALIBRATE.rf_out_power_80m, 3650 * 1000, "80M Calibration");
		}
		if (AUTO_CALIBRATION_current_page == 3) {
			AUTO_CALIBRATION_DoPowerCalibration(&CALIBRATE.rf_out_power_40m, 7100 * 1000, "40M Calibration");
		}
		if (AUTO_CALIBRATION_current_page == 4) {
			AUTO_CALIBRATION_DoPowerCalibration(&CALIBRATE.rf_out_power_30m, 10110 * 1000, "30M Calibration");
		}
		if (AUTO_CALIBRATION_current_page == 5) {
			AUTO_CALIBRATION_DoPowerCalibration(&CALIBRATE.rf_out_power_20m, 14100 * 1000, "20M Calibration");
		}
		if (AUTO_CALIBRATION_current_page == 6) {
			AUTO_CALIBRATION_DoPowerCalibration(&CALIBRATE.rf_out_power_17m, 18100 * 1000, "17M Calibration");
		}
		if (AUTO_CALIBRATION_current_page == 7) {
			AUTO_CALIBRATION_DoPowerCalibration(&CALIBRATE.rf_out_power_15m, 21200 * 1000, "15M Calibration");
		}
		if (AUTO_CALIBRATION_current_page == 8) {
			AUTO_CALIBRATION_DoPowerCalibration(&CALIBRATE.rf_out_power_12m, 24900 * 1000, "12M Calibration");
		}
		if (AUTO_CALIBRATION_current_page == 9) {
			AUTO_CALIBRATION_DoPowerCalibration(&CALIBRATE.rf_out_power_cb, 27000 * 1000, "CB Calibration");
		}
		if (AUTO_CALIBRATION_current_page == 10) {
			AUTO_CALIBRATION_DoPowerCalibration(&CALIBRATE.rf_out_power_10m, 28800 * 1000, "10M Calibration");
		}
		if (AUTO_CALIBRATION_current_page == 11) {
			AUTO_CALIBRATION_DoPowerCalibration(&CALIBRATE.rf_out_power_6m, 52000 * 1000, "6M Calibration");
		}
		if (AUTO_CALIBRATION_current_page == 12) {
			AUTO_CALIBRATION_DoPowerCalibration(&CALIBRATE.rf_out_power_2m, 145000 * 1000, "2M Calibration");
		}

		if (AUTO_CALIBRATION_current_page == 13) // Done
		{
			TRX_Tune = false;

			LCDDriver_printText("Calibration complete", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;

			// redraw loop
			LCD_UpdateQuery.SystemMenuRedraw = true;
		}

		if (AUTO_CALIBRATION_current_page > 0 && AUTO_CALIBRATION_current_page < 13) {
			sprintf(str, "%.1f SWR", (double)TRX_SWR_SMOOTHED);
			LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			sprintf(str, "%.1f W Out", (double)AUTO_CALIBRATION_prev_power);
			LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;

			pos_y += margin_bottom;
			if (!AUTO_CALIBRATION_finished) {
				LCDDriver_printText("Calibration started", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			} else {
				LCDDriver_printText("Calibration finished", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			}
			pos_y += margin_bottom;

			pos_y += margin_bottom;
			LCDDriver_printText("ENC2 Click to retry", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			// redraw loop
			LCD_UpdateQuery.SystemMenuRedraw = true;
		}

		if (AUTO_CALIBRATION_current_page > 13) {
			AUTO_CALIBRATION_current_page = 13;
		}
	}

	// Pager
	pos_y += margin_bottom;
	LCDDriver_printText("Rotate ENC2 - select page", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
	pos_y += margin_bottom;

	LCD_busy = false;
}

// events to the encoder
void AUTO_CALIBRATION_EncRotate(int8_t direction) {
	if (LCD_busy) {
		return;
	}

	if (AUTO_CALIBRATION_calibration_type == 0) // swr test
	{
		if (AUTO_CALIBRATION_current_page == 1) // HF FWD
		{
			CALIBRATE.SWR_FWD_Calibration_HF += (float32_t)direction * 0.1f;
			if (CALIBRATE.SWR_FWD_Calibration_HF < 1.0f) {
				CALIBRATE.SWR_FWD_Calibration_HF = 1.0f;
			}
			if (CALIBRATE.SWR_FWD_Calibration_HF > 200.0f) {
				CALIBRATE.SWR_FWD_Calibration_HF = 200.0f;
			}
		}
		if (AUTO_CALIBRATION_current_page == 2) // HF BWD
		{
			CALIBRATE.SWR_BWD_Calibration_HF += (float32_t)direction * 0.1f;
			if (CALIBRATE.SWR_BWD_Calibration_HF < 1.0f) {
				CALIBRATE.SWR_BWD_Calibration_HF = 1.0f;
			}
			if (CALIBRATE.SWR_BWD_Calibration_HF > 200.0f) {
				CALIBRATE.SWR_BWD_Calibration_HF = 200.0f;
			}
		}
		if (AUTO_CALIBRATION_current_page == 4) // 6m FWD
		{
			CALIBRATE.SWR_FWD_Calibration_6M += (float32_t)direction * 0.1f;
			if (CALIBRATE.SWR_FWD_Calibration_6M < 1.0f) {
				CALIBRATE.SWR_FWD_Calibration_6M = 1.0f;
			}
			if (CALIBRATE.SWR_FWD_Calibration_6M > 200.0f) {
				CALIBRATE.SWR_FWD_Calibration_6M = 200.0f;
			}
		}
		if (AUTO_CALIBRATION_current_page == 5) // 6m BWD
		{
			CALIBRATE.SWR_BWD_Calibration_6M += (float32_t)direction * 0.1f;
			if (CALIBRATE.SWR_BWD_Calibration_6M < 1.0f) {
				CALIBRATE.SWR_BWD_Calibration_6M = 1.0f;
			}
			if (CALIBRATE.SWR_BWD_Calibration_6M > 200.0f) {
				CALIBRATE.SWR_BWD_Calibration_6M = 200.0f;
			}
		}
		if (AUTO_CALIBRATION_current_page == 7) // VHF FWD
		{
			CALIBRATE.SWR_FWD_Calibration_VHF += (float32_t)direction * 0.1f;
			if (CALIBRATE.SWR_FWD_Calibration_VHF < 1.0f) {
				CALIBRATE.SWR_FWD_Calibration_VHF = 1.0f;
			}
			if (CALIBRATE.SWR_FWD_Calibration_VHF > 200.0f) {
				CALIBRATE.SWR_FWD_Calibration_VHF = 200.0f;
			}
		}
		if (AUTO_CALIBRATION_current_page == 8) // VHF BWD
		{
			CALIBRATE.SWR_BWD_Calibration_VHF += (float32_t)direction * 0.1f;
			if (CALIBRATE.SWR_BWD_Calibration_VHF < 1.0f) {
				CALIBRATE.SWR_BWD_Calibration_VHF = 1.0f;
			}
			if (CALIBRATE.SWR_BWD_Calibration_VHF > 200.0f) {
				CALIBRATE.SWR_BWD_Calibration_VHF = 200.0f;
			}
		}
	}

	LCD_UpdateQuery.SystemMenuRedraw = true;
}

void AUTO_CALIBRATION_Enc2Rotate(int8_t direction) {
	if (LCD_busy) {
		return;
	}

	LCD_busy = true;
	LCDDriver_Fill(BG_COLOR);
	LCD_busy = false;

	// restart power calibration
	AUTO_CALIBRATION_started = false;
	AUTO_CALIBRATION_finished = false;

	AUTO_CALIBRATION_current_page += direction;
	if (AUTO_CALIBRATION_current_page < 0) {
		AUTO_CALIBRATION_current_page = 0;
	}

	LCD_UpdateQuery.SystemMenuRedraw = true;
}

void AUTO_CALIBRATION_Enc2Click() {
	if (LCD_busy) {
		return;
	}

	LCD_busy = true;
	LCDDriver_Fill(BG_COLOR);
	LCD_busy = false;

	// restart power calibration
	AUTO_CALIBRATION_started = false;
	AUTO_CALIBRATION_finished = false;

	LCD_UpdateQuery.SystemMenuRedraw = true;
}

static void AUTO_CALIBRATION_DoPowerCalibration(uint8_t *calibration_parameter, uint64_t freq, char *band_name) {
	char str[64] = {0};
	TRX_setFrequency(freq, CurrentVFO);
	TRX_setMode(TRX_MODE_CW, CurrentVFO);
	TRX.ANT_RX = TRX_ANT_1;
	TRX.ANT_TX = TRX_ANT_1;
	TRX.RF_Gain = 100;
	TRX.TUNER_Enabled = false;
	TRX.ATU_Enabled = false;

	LCDDriver_printText(band_name, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
	pos_y += margin_bottom;

	if (!AUTO_CALIBRATION_started) {
		AUTO_CALIBRATION_started = true;

		*calibration_parameter = 0;
		TRX_PWR_Forward_SMOOTHED = 0;
		TRX_SWR_SMOOTHED = 1.0f;
		APROC_TX_clip_gain = 1.0f;
		TRX_MAX_TX_Amplitude = getMaxTXAmplitudeOnFreq(CurrentVFO->Freq);

		TRX_Tune = true;

		AUTO_CALIBRATION_current_power_calib = 0;
		AUTO_CALIBRATION_prev_power = 0;
		AUTO_CALIBRATION_current_power_low_diff = 0;
	}
	if (!AUTO_CALIBRATION_finished && AUTO_CALIBRATION_ProcessPowerCalibration(calibration_parameter)) {
		AUTO_CALIBRATION_finished = true;
		TRX_Tune = false;
	}

	sprintf(str, "%u Calibration", *calibration_parameter);
	LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
	pos_y += margin_bottom;
}

static bool AUTO_CALIBRATION_ProcessPowerCalibration(uint8_t *rf_calibration) {
	TRX_Tune = true;

	if (AUTO_CALIBRATION_current_power_delay < 3) {
		AUTO_CALIBRATION_current_power_delay++;
		return false;
	}
	AUTO_CALIBRATION_current_power_delay = 0;

	float32_t power_diff = TRX_PWR_Forward_SMOOTHED - AUTO_CALIBRATION_prev_power;
	AUTO_CALIBRATION_prev_power = TRX_PWR_Forward_SMOOTHED;

	println("[AutoCal] PWR: ", (double)TRX_PWR_Forward_SMOOTHED, " CAL: ", *rf_calibration, " PWR DIFF: ", (double)power_diff, " SWR: ", (double)TRX_SWR_SMOOTHED);

	if (*rf_calibration >= 10 && power_diff <= 0.1f && TRX_PWR_Forward_SMOOTHED > 1.0f) { // no gain enough, stop
		AUTO_CALIBRATION_current_power_low_diff++;

		if (AUTO_CALIBRATION_current_power_low_diff > 2) {
			*rf_calibration = *rf_calibration - 1;
			return true;
		}
	} else {
		AUTO_CALIBRATION_current_power_low_diff = 0; // reset diff error
	}

	if (TRX_SWR_SMOOTHED > 3.0f) { // swr is dangerous
		return true;
	}

	if (*rf_calibration >= 50 && TRX_PWR_Forward_SMOOTHED < 1.0f) { // no signal, stop
		return true;
	}

	if (*rf_calibration >= 100) { // max power
		*rf_calibration = 100;
		return true;
	}

	if (TRX_PWR_Forward_SMOOTHED >= CALIBRATE.MAX_RF_POWER_ON_METER) { // target
		if (*rf_calibration > 0) {
			*rf_calibration = *rf_calibration - 1;
		}
		return true;
	}

	AUTO_CALIBRATION_current_power_calib++;
	APROC_TX_clip_gain = 1.0f;
	*rf_calibration = AUTO_CALIBRATION_current_power_calib;
	TRX_MAX_TX_Amplitude = getMaxTXAmplitudeOnFreq(CurrentVFO->Freq);
	return false;
}
