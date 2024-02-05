#include "self_test.h"
#include "fonts.h"
#include "fpga.h"
#include "front_unit.h"
#include "functions.h"
#include "lcd.h"
#include "lcd_driver.h"
#include "main.h"
#include "trx_manager.h"

// Public variables
bool SYSMENU_selftest_opened = false;

// Private Variables
static int8_t SELF_TEST_current_page = 0;
static bool SELF_TEST_old_autogainer = false;
static uint32_t SELF_TEST_old_freq = SELF_TEST_frequency_HF;
static bool LastLNA = false;
static bool LastDRV = false;
static bool LastPGA = false;
static bool LastATT = false;
static float32_t LastATT_DB = false;

// Prototypes
static void SELF_TEST_printResult(bool result, uint16_t pos_y);

// start
void SELF_TEST_Start(void) {
	LCD_busy = true;

	LastLNA = TRX.LNA;
	LastDRV = TRX.ADC_Driver;
	LastPGA = TRX.ADC_PGA;
	LastATT = TRX.ATT;
	LastATT_DB = TRX.ATT_DB;

	SELF_TEST_old_autogainer = TRX.AutoGain;
	SELF_TEST_old_freq = CurrentVFO->Freq;
	SELF_TEST_current_page = 0;
	LCDDriver_Fill(BG_COLOR);

	LCD_busy = false;
	LCD_UpdateQuery.SystemMenu = true;
}

// stop
void SELF_TEST_Stop(void) {
	TRX.AutoGain = SELF_TEST_old_autogainer;
	TRX_setFrequency(SELF_TEST_old_freq, CurrentVFO);

	TRX.LNA = LastLNA;
	TRX.ADC_Driver = LastDRV;
	TRX.ADC_PGA = LastPGA;
	TRX.ATT = LastATT;
	TRX.ATT_DB = LastATT_DB;
}

// draw
void SELF_TEST_Draw(void) {
	if (LCD_busy) {
		LCD_UpdateQuery.SystemMenuRedraw = true;
		return;
	}
	LCD_busy = true;
	LCD_UpdateQuery.SystemMenu = false;
	LCD_UpdateQuery.SystemMenuRedraw = false;

	// predefine
#ifdef LAY_160x128
#define margin_left 5
#define margin_bottom 10
#define font_size 1
#elif defined(LAY_320x240)
#define margin_left 5
#define margin_bottom 20
#define font_size 2
#else
#define margin_left 5
#define margin_bottom 20
#define font_size 2
#endif

	uint16_t pos_y = margin_left;
	char str[64] = {0};
	bool pass = true;

	// print pages
	if (SELF_TEST_current_page == 0) {
		// FPGA BUS test
		LCDDriver_printText("FPGA", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
		SELF_TEST_printResult(FPGA_bus_test_result, pos_y);
		pos_y += margin_bottom;

		// MCP3008 test
#ifdef HRDW_MCP3008_1
		if (!FRONTPanel_MCP3008_1_Enabled) {
			pass = false;
		}
#endif
#ifdef HRDW_MCP3008_2
		if (!FRONTPanel_MCP3008_2_Enabled) {
			pass = false;
		}
#endif
#ifdef HRDW_MCP3008_3
		if (!FRONTPanel_MCP3008_3_Enabled) {
			pass = false;
		}
#endif
#ifdef HRDW_MCP3008_1
		if (FPGA_bus_test_result) {
			LCDDriver_printText("MCP3008", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			SELF_TEST_printResult(pass, pos_y);
			pos_y += margin_bottom;
		}
#endif

		// STM32 EEPROM test
		if (FPGA_bus_test_result) {
			LCDDriver_printText("STM32 EEPROM", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			SELF_TEST_printResult(EEPROM_Enabled, pos_y);
			pos_y += margin_bottom;
		}

		// WM8731 test
		LCDDriver_printText("CODEC", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
		SELF_TEST_printResult(CODEC_test_result, pos_y);
		pos_y += margin_bottom;

		// Samplerate test
		LCDDriver_printText("FPGA Clocks", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
		SELF_TEST_printResult(abs((int32_t)TRX_GetRXSampleRate - (int32_t)dbg_FPGA_samples) < (TRX_GetRXSampleRate * 0.05f), pos_y); // 5%
		pos_y += margin_bottom;

		// redraw loop
		LCD_UpdateQuery.SystemMenuRedraw = true;
	}

	if (SELF_TEST_current_page == 1) {
		static uint8_t current_test = 0;
		static uint32_t current_test_start_time = 0;

		LCDDriver_printText("Audio test", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
		pos_y += margin_bottom;

		if (current_test == 0 && (HAL_GetTick() - current_test_start_time) > SELF_TEST_adc_test_latency) {
			current_test_start_time = HAL_GetTick();
			LCDDriver_printText("LEFT ", margin_left, pos_y, COLOR_GREEN, BG_COLOR, font_size);
			current_test = 1;

			CODEC_TestBeep(false);
		}
		if (current_test == 1 && (HAL_GetTick() - current_test_start_time) > SELF_TEST_adc_test_latency) {
			current_test_start_time = HAL_GetTick();
			LCDDriver_printText("RIGHT", margin_left, pos_y, COLOR_GREEN, BG_COLOR, font_size);
			current_test = 0;

			CODEC_TestBeep(true);
		}

		pos_y += margin_bottom;

		// redraw loop
		LCD_UpdateQuery.SystemMenuRedraw = true;
	}

	if (SELF_TEST_current_page == 2) {
		static bool ok[16] = {false};
		static int8_t prev_adc_state[16] = {0};

		TRX_setFrequency(SELF_TEST_frequency_HF, CurrentVFO);
		LCDDriver_printText("Connect ANT 14MHz", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
		pos_y += margin_bottom;

		LCDDriver_printText("ADC RAW Data", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
		sprintf(str, " %d         ", ADC_RAW_IN);
		LCDDriver_printText(str, LCDDriver_GetCurrentXOffset(), pos_y, COLOR_GREEN, BG_COLOR, font_size);
		pos_y += margin_bottom;

		for (uint8_t i = 0; i < 16; i++) {
			int8_t bit = bitRead(ADC_RAW_IN, i);
			if (bit == 0) {
				bit = -1;
			}

			if (prev_adc_state[i] != 0 && prev_adc_state[i] != bit) {
				ok[i] = true;
			}

			prev_adc_state[i] = bit;
		}

		LCDDriver_printText("ADC BITS: ", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
		LCDDriver_printText("1 ", LCDDriver_GetCurrentXOffset(), pos_y, (ok[0]) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
		LCDDriver_printText("2 ", LCDDriver_GetCurrentXOffset(), pos_y, (ok[1]) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
		LCDDriver_printText("3 ", LCDDriver_GetCurrentXOffset(), pos_y, (ok[2]) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
		LCDDriver_printText("4 ", LCDDriver_GetCurrentXOffset(), pos_y, (ok[3]) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
		LCDDriver_printText("5 ", LCDDriver_GetCurrentXOffset(), pos_y, (ok[4]) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
		LCDDriver_printText("6 ", LCDDriver_GetCurrentXOffset(), pos_y, (ok[5]) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
		LCDDriver_printText("7 ", LCDDriver_GetCurrentXOffset(), pos_y, (ok[6]) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
		LCDDriver_printText("8 ", LCDDriver_GetCurrentXOffset(), pos_y, (ok[7]) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
		pos_y += margin_bottom;
		LCDDriver_printText("9 ", margin_left, pos_y, (ok[8]) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
		LCDDriver_printText("10 ", LCDDriver_GetCurrentXOffset(), pos_y, (ok[9]) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
		LCDDriver_printText("11 ", LCDDriver_GetCurrentXOffset(), pos_y, (ok[10]) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
		LCDDriver_printText("12 ", LCDDriver_GetCurrentXOffset(), pos_y, (ok[11]) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
#if !defined(FRONTPANEL_LITE)
		LCDDriver_printText("13 ", LCDDriver_GetCurrentXOffset(), pos_y, (ok[12]) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
		LCDDriver_printText("14 ", LCDDriver_GetCurrentXOffset(), pos_y, (ok[13]) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
		LCDDriver_printText("15 ", LCDDriver_GetCurrentXOffset(), pos_y, (ok[14]) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
		LCDDriver_printText("16 ", LCDDriver_GetCurrentXOffset(), pos_y, (ok[15]) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
#endif
		pos_y += margin_bottom;

		// redraw loop
		LCD_UpdateQuery.SystemMenuRedraw = true;
	}

	if (!HRDW_HAS_LNA_VHF && SELF_TEST_current_page == 4) {
		SELF_TEST_current_page++;
	}

	if (SELF_TEST_current_page == 3 || SELF_TEST_current_page == 4) {
		static uint8_t current_test = 0;
		static uint32_t current_test_start_time = 0;

		if (SELF_TEST_current_page == 3) {
			TRX_setFrequency(SELF_TEST_frequency_HF, CurrentVFO);
			LCDDriver_printText("Connect ANT 14MHz", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
		}
		if (SELF_TEST_current_page == 4) {
			TRX_setFrequency(SELF_TEST_frequency_VHF, CurrentVFO);
			LCDDriver_printText("Connect ANT 145MHz", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
		}
		pos_y += margin_bottom;

		// predefine
		static float32_t base_signal = 0;

		// get base signal
		if (current_test == 0) {
			TRX.AutoGain = false;
			TRX.ATT = false;
			TRX.ATT_DB = 0;
			TRX.LNA = false;
			TRX.ADC_PGA = false;
			TRX.ADC_Driver = false;
			FPGA_NeedSendParams = true;
			current_test = 1;
			current_test_start_time = HAL_GetTick();
		}
		if (current_test == 1 && (HAL_GetTick() - current_test_start_time) > SELF_TEST_adc_test_latency) {
			// get base signal strength
			base_signal = fmaxf(fabsf((float32_t)TRX_ADC_MINAMPLITUDE), fabsf((float32_t)TRX_ADC_MAXAMPLITUDE));
			LCDDriver_printText("Signal", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			sprintf(str, " %d / %d         ", TRX_ADC_MINAMPLITUDE, TRX_ADC_MAXAMPLITUDE);
			LCDDriver_printText(str, LCDDriver_GetCurrentXOffset(), pos_y, (base_signal > 20 && base_signal < 2000) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
			pos_y += margin_bottom;

			// ADC symmetry
			pass = true;
			if (TRX_ADC_MINAMPLITUDE > 0) {
				pass = false;
			}
			if (TRX_ADC_MAXAMPLITUDE < 0) {
				pass = false;
			}
			if ((abs(TRX_ADC_MINAMPLITUDE) > abs(TRX_ADC_MAXAMPLITUDE)) && (abs(TRX_ADC_MINAMPLITUDE) > (abs(TRX_ADC_MAXAMPLITUDE) * 4))) {
				pass = false;
			}
			if ((abs(TRX_ADC_MINAMPLITUDE) < abs(TRX_ADC_MAXAMPLITUDE)) && ((abs(TRX_ADC_MINAMPLITUDE) * 4) < abs(TRX_ADC_MAXAMPLITUDE))) {
				pass = false;
			}
			LCDDriver_printText("ADC Symmetry", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			SELF_TEST_printResult(pass, pos_y);
			pos_y += margin_bottom;

			current_test = 2;
		} else {
			pos_y += margin_bottom * 2;
		}

		// test ADC Driver
		if (current_test == 2) {
			TRX.ADC_PGA = false;
			TRX.LNA = false;
			TRX.ADC_Driver = true;
			FPGA_NeedSendParams = true;
			current_test = 3;
			current_test_start_time = HAL_GetTick();
		}
		if (current_test == 3 && (HAL_GetTick() - current_test_start_time) > SELF_TEST_adc_test_latency) {
			float32_t ADC_Driver_signal = fmaxf(fabsf((float32_t)TRX_ADC_MINAMPLITUDE), fabsf((float32_t)TRX_ADC_MAXAMPLITUDE));
			float32_t ADC_Driver_dB = rate2dbV(ADC_Driver_signal / base_signal);

			LCDDriver_printText("ADC Driver signal", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			sprintf(str, " %d          ", (uint16_t)ADC_Driver_signal);
			LCDDriver_printText(str, LCDDriver_GetCurrentXOffset(), pos_y, (ADC_Driver_signal < 32000.0f) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
			pos_y += margin_bottom;

			LCDDriver_printText("ADC Driver gain", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			sprintf(str, " %.2f dB          ", (double)ADC_Driver_dB);
			LCDDriver_printText(str, LCDDriver_GetCurrentXOffset(), pos_y, (ADC_Driver_dB > 17.0f && ADC_Driver_dB < 30.0f) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
			pos_y += margin_bottom;

			current_test = 4;
		} else {
			pos_y += margin_bottom * 2;
		}

		// test ADC PGA
		if (current_test == 4) {
			TRX.ADC_Driver = false;
			TRX.LNA = false;
			TRX.ADC_PGA = true;
			FPGA_NeedSendParams = true;
			current_test = 5;
			current_test_start_time = HAL_GetTick();
		}
		if (current_test == 5 && (HAL_GetTick() - current_test_start_time) > SELF_TEST_adc_test_latency) {
			float32_t ADC_PGA_signal = fmaxf(fabsf((float32_t)TRX_ADC_MINAMPLITUDE), fabsf((float32_t)TRX_ADC_MAXAMPLITUDE));
			float32_t ADC_PGA_dB = rate2dbV(ADC_PGA_signal / base_signal);

#if !defined(FRONTPANEL_LITE)
			LCDDriver_printText("ADC PGA signal", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			sprintf(str, " %d          ", (uint16_t)ADC_PGA_signal);
			LCDDriver_printText(str, LCDDriver_GetCurrentXOffset(), pos_y, (ADC_PGA_signal < 32000.0f) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
#endif
			pos_y += margin_bottom;

#if !defined(FRONTPANEL_LITE)
			LCDDriver_printText("ADC PGA gain", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			sprintf(str, " %.2f dB          ", (double)ADC_PGA_dB);
			LCDDriver_printText(str, LCDDriver_GetCurrentXOffset(), pos_y, (ADC_PGA_dB > 2.0f && ADC_PGA_dB < 7.0f) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
#endif
			pos_y += margin_bottom;

			current_test = 6;
		} else {
			pos_y += margin_bottom * 2;
		}

		// test LNA
		if (current_test == 6) {
			TRX.ADC_PGA = false;
			TRX.ADC_Driver = false;
			TRX.LNA = true;
			FPGA_NeedSendParams = true;
			current_test = 7;
			current_test_start_time = HAL_GetTick();
		}
		if (current_test == 7 && (HAL_GetTick() - current_test_start_time) > SELF_TEST_adc_test_latency) {
			float32_t ADC_LNA_signal = fmaxf(fabsf((float32_t)TRX_ADC_MINAMPLITUDE), fabsf((float32_t)TRX_ADC_MAXAMPLITUDE));
			float32_t ADC_LNA_dB = rate2dbV(ADC_LNA_signal / base_signal);

			if ((HRDW_HAS_LNA_HF && SELF_TEST_current_page == 3) || (HRDW_HAS_LNA_VHF && SELF_TEST_current_page == 4)) {
				LCDDriver_printText("LNA signal", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
				sprintf(str, " %d          ", (uint16_t)ADC_LNA_signal);
				LCDDriver_printText(str, LCDDriver_GetCurrentXOffset(), pos_y, (ADC_LNA_signal < 32000.0f) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
				pos_y += margin_bottom;

				LCDDriver_printText("LNA gain", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
				sprintf(str, " %.2f dB          ", (double)ADC_LNA_dB);
				LCDDriver_printText(str, LCDDriver_GetCurrentXOffset(), pos_y, (ADC_LNA_dB > 23.0f && ADC_LNA_dB < 30.0f) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
				pos_y += margin_bottom;
			} else {
				pos_y += margin_bottom * 2;
			}

			current_test = 0;
		} else {
			pos_y += margin_bottom * 2;
		}

		// redraw loop
		LCD_UpdateQuery.SystemMenuRedraw = true;
	}

	if (SELF_TEST_current_page == 5) {
		static uint8_t current_test = 0;
		static uint32_t current_test_start_time = 0;

		TRX_setFrequency(SELF_TEST_frequency_HF, CurrentVFO);
		LCDDriver_printText("Connect ANT 14MHz", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
		pos_y += margin_bottom;

		// predefine
		static float32_t base_signal = 0;

		// get base signal
		if (current_test == 0) {
			TRX.AutoGain = false;
			TRX.ATT = false;
			TRX.ATT_DB = 0;
			TRX.LNA = true;
			TRX.ADC_PGA = true;
			TRX.ADC_Driver = !HRDW_HAS_LNA_HF;
			FPGA_NeedSendParams = true;
			current_test = 1;
			current_test_start_time = HAL_GetTick();
		}
		if (current_test == 1 && (HAL_GetTick() - current_test_start_time) > SELF_TEST_adc_test_latency) {
			// get base signal strength
			base_signal = fmaxf(fabsf((float32_t)TRX_ADC_MINAMPLITUDE), fabsf((float32_t)TRX_ADC_MAXAMPLITUDE));
			LCDDriver_printText("Signal", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			sprintf(str, " %d / %d         ", TRX_ADC_MINAMPLITUDE, TRX_ADC_MAXAMPLITUDE);
			LCDDriver_printText(str, LCDDriver_GetCurrentXOffset(), pos_y, (base_signal > 1000 && base_signal < 32000) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
			pos_y += margin_bottom;

			current_test = 2;
		} else {
			pos_y += margin_bottom;
		}

		// ATT ON
		if (current_test == 2) {
			TRX.ATT = true;
			TRX.ATT_DB = 0;
			FPGA_NeedSendParams = true;
			current_test = 3;
			current_test_start_time = HAL_GetTick();
		}
		if (current_test == 3 && (HAL_GetTick() - current_test_start_time) > SELF_TEST_adc_test_latency) {
			float32_t ATT_signal = fmaxf(fabsf((float32_t)TRX_ADC_MINAMPLITUDE), fabsf((float32_t)TRX_ADC_MAXAMPLITUDE));
			float32_t ATT_dB = rate2dbV(ATT_signal / base_signal);

			LCDDriver_printText("ATT ON", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			sprintf(str, " %d / %.2f dB         ", (uint16_t)ATT_signal, (double)ATT_dB);
			LCDDriver_printText(str, LCDDriver_GetCurrentXOffset(), pos_y, (ATT_signal < 32000.0f && (ATT_dB > -2.0f && ATT_dB < 1.0f)) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
			pos_y += margin_bottom;

			current_test = 4;
		} else {
			pos_y += margin_bottom;
		}

		// ATT 0.5 ON
		if (current_test == 4) {
			TRX.ATT = true;
			TRX.ATT_DB = 0.5f;
			FPGA_NeedSendParams = true;
			current_test = 5;
			current_test_start_time = HAL_GetTick();
		}
		if (current_test == 5 && (HAL_GetTick() - current_test_start_time) > SELF_TEST_adc_test_latency) {
			float32_t ATT_signal = fmaxf(fabsf((float32_t)TRX_ADC_MINAMPLITUDE), fabsf((float32_t)TRX_ADC_MAXAMPLITUDE));
			float32_t ATT_dB = rate2dbV(ATT_signal / base_signal);

			LCDDriver_printText("ATT 0.5", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			sprintf(str, " %d / %.2f dB         ", (uint16_t)ATT_signal, (double)ATT_dB);
			LCDDriver_printText(str, LCDDriver_GetCurrentXOffset(), pos_y, (ATT_signal < 32000.0f && (ATT_dB > -2.0f && ATT_dB < 1.0f)) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
			pos_y += margin_bottom;

			current_test = 6;
		} else {
			pos_y += margin_bottom;
		}

		// ATT 1 ON
		if (current_test == 6) {
			TRX.ATT = true;
			TRX.ATT_DB = 1.0f;
			FPGA_NeedSendParams = true;
			current_test = 7;
			current_test_start_time = HAL_GetTick();
		}
		if (current_test == 7 && (HAL_GetTick() - current_test_start_time) > SELF_TEST_adc_test_latency) {
			float32_t ATT_signal = fmaxf(fabsf((float32_t)TRX_ADC_MINAMPLITUDE), fabsf((float32_t)TRX_ADC_MAXAMPLITUDE));
			float32_t ATT_dB = rate2dbV(ATT_signal / base_signal);

			LCDDriver_printText("ATT 1.0", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			sprintf(str, " %d / %.2f dB         ", (uint16_t)ATT_signal, (double)ATT_dB);
			LCDDriver_printText(str, LCDDriver_GetCurrentXOffset(), pos_y, (ATT_signal < 32000.0f && (ATT_dB > -2.0f && ATT_dB < 1.0f)) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
			pos_y += margin_bottom;

			current_test = 8;
		} else {
			pos_y += margin_bottom;
		}

		// ATT 2 ON
		if (current_test == 8) {
			TRX.ATT = true;
			TRX.ATT_DB = 2.0f;
			FPGA_NeedSendParams = true;
			current_test = 9;
			current_test_start_time = HAL_GetTick();
		}
		if (current_test == 9 && (HAL_GetTick() - current_test_start_time) > SELF_TEST_adc_test_latency) {
			float32_t ATT_signal = fmaxf(fabsf((float32_t)TRX_ADC_MINAMPLITUDE), fabsf((float32_t)TRX_ADC_MAXAMPLITUDE));
			float32_t ATT_dB = rate2dbV(ATT_signal / base_signal);

			LCDDriver_printText("ATT 2.0", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			sprintf(str, " %d / %.2f dB         ", (uint16_t)ATT_signal, (double)ATT_dB);
			LCDDriver_printText(str, LCDDriver_GetCurrentXOffset(), pos_y, (ATT_signal < 32000.0f && (ATT_dB > -3.0f && ATT_dB < -1.0f)) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
			pos_y += margin_bottom;

			current_test = 10;
		} else {
			pos_y += margin_bottom;
		}

		// ATT 4 ON
		if (current_test == 10) {
			TRX.ATT = true;
			TRX.ATT_DB = 4.0f;
			FPGA_NeedSendParams = true;
			current_test = 11;
			current_test_start_time = HAL_GetTick();
		}
		if (current_test == 11 && (HAL_GetTick() - current_test_start_time) > SELF_TEST_adc_test_latency) {
			float32_t ATT_signal = fmaxf(fabsf((float32_t)TRX_ADC_MINAMPLITUDE), fabsf((float32_t)TRX_ADC_MAXAMPLITUDE));
			float32_t ATT_dB = rate2dbV(ATT_signal / base_signal);

			LCDDriver_printText("ATT 4.0", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			sprintf(str, " %d / %.2f dB         ", (uint16_t)ATT_signal, (double)ATT_dB);
			LCDDriver_printText(str, LCDDriver_GetCurrentXOffset(), pos_y, (ATT_signal < 32000.0f && (ATT_dB > -6.0f && ATT_dB < -2.0f)) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
			pos_y += margin_bottom;

			current_test = 12;
		} else {
			pos_y += margin_bottom;
		}

		// ATT 8 ON
		if (current_test == 12) {
			TRX.ATT = true;
			TRX.ATT_DB = 8.0f;
			FPGA_NeedSendParams = true;
			current_test = 13;
			current_test_start_time = HAL_GetTick();
		}
		if (current_test == 13 && (HAL_GetTick() - current_test_start_time) > SELF_TEST_adc_test_latency) {
			float32_t ATT_signal = fmaxf(fabsf((float32_t)TRX_ADC_MINAMPLITUDE), fabsf((float32_t)TRX_ADC_MAXAMPLITUDE));
			float32_t ATT_dB = rate2dbV(ATT_signal / base_signal);

			LCDDriver_printText("ATT 8.0", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			sprintf(str, " %d / %.2f dB         ", (uint16_t)ATT_signal, (double)ATT_dB);
			LCDDriver_printText(str, LCDDriver_GetCurrentXOffset(), pos_y, (ATT_signal < 32000.0f && (ATT_dB > -10.0f && ATT_dB < -4.0f)) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
			pos_y += margin_bottom;

			current_test = 14;
		} else {
			pos_y += margin_bottom;
		}

		// ATT 16 ON
		if (current_test == 14) {
			TRX.ATT = true;
			TRX.ATT_DB = 16.0f;
			FPGA_NeedSendParams = true;
			current_test = 15;
			current_test_start_time = HAL_GetTick();
		}
		if (current_test == 15 && (HAL_GetTick() - current_test_start_time) > SELF_TEST_adc_test_latency) {
			float32_t ATT_signal = fmaxf(fabsf((float32_t)TRX_ADC_MINAMPLITUDE), fabsf((float32_t)TRX_ADC_MAXAMPLITUDE));
			float32_t ATT_dB = rate2dbV(ATT_signal / base_signal);

			LCDDriver_printText("ATT 16.0", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			sprintf(str, " %d / %.2f dB         ", (uint16_t)ATT_signal, (double)ATT_dB);
			LCDDriver_printText(str, LCDDriver_GetCurrentXOffset(), pos_y, (ATT_signal < 32000.0f && (ATT_dB > -20.0f && ATT_dB < -8.0f)) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
			pos_y += margin_bottom;

			current_test = 16;
		} else {
			pos_y += margin_bottom;
		}

		// ATT 32 ON
		if (current_test == 16) {
			TRX.ATT = true;
			TRX.ATT_DB = 32.0f;
			FPGA_NeedSendParams = true;
			current_test = 17;
			current_test_start_time = HAL_GetTick();
		}
		if (current_test == 17 && (HAL_GetTick() - current_test_start_time) > SELF_TEST_adc_test_latency) {
			float32_t ATT_signal = fmaxf(fabsf((float32_t)TRX_ADC_MINAMPLITUDE), fabsf((float32_t)TRX_ADC_MAXAMPLITUDE));
			float32_t ATT_dB = rate2dbV(ATT_signal / base_signal);

			LCDDriver_printText("ATT 32.0", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			sprintf(str, " %d / %.2f dB         ", (uint16_t)ATT_signal, (double)ATT_dB);
			LCDDriver_printText(str, LCDDriver_GetCurrentXOffset(), pos_y, (ATT_signal < 32000.0f && (ATT_dB > -33.0f && ATT_dB < -16.0f)) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
			pos_y += margin_bottom;

			current_test = 0;
		} else {
			pos_y += margin_bottom;
		}

		// redraw loop
		LCD_UpdateQuery.SystemMenuRedraw = true;
	}

	if (SELF_TEST_current_page == 6) {
		// redraw loop
		LCD_UpdateQuery.SystemMenuRedraw = true;

		static uint32_t current_test_start_time = 0;
		if ((HAL_GetTick() - current_test_start_time) < SELF_TEST_adc_test_latency) {
			// wait
			LCD_busy = false;
			return;
		}
		current_test_start_time = HAL_GetTick();

		LCDDriver_printText("LCD Test", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
		pos_y += margin_bottom;

		for (uint16_t x = 0; x < LCD_WIDTH / 3; x++) {
			uint8_t color = roundf(255.0f * ((float32_t)((LCD_WIDTH / 3) - x) / (float32_t)(LCD_WIDTH / 3)));
			LCDDriver_drawFastVLine(x, pos_y, margin_bottom, rgb888torgb565(color, 255 - color, 0));
			LCDDriver_drawFastVLine(x + LCD_WIDTH / 3, pos_y, margin_bottom, rgb888torgb565(0, color, 255 - color));
			LCDDriver_drawFastVLine(x + LCD_WIDTH / 3 * 2, pos_y, margin_bottom, rgb888torgb565(255 - color, 0, color));
		}
		pos_y += margin_bottom + 1;

		// single colors brightness
		for (uint16_t x = 0; x < LCD_WIDTH; x++) {
			uint8_t color = roundf(255.0f * ((float32_t)(LCD_WIDTH - x) / (float32_t)LCD_WIDTH));
			LCDDriver_drawFastVLine(x, pos_y, margin_bottom, rgb888torgb565(color, 0, 0));
			LCDDriver_drawFastVLine(x, pos_y + margin_bottom, margin_bottom, rgb888torgb565(0, color, 0));
			LCDDriver_drawFastVLine(x, pos_y + margin_bottom * 2, margin_bottom, rgb888torgb565(0, 0, color));
		}
		pos_y += margin_bottom * 3;
	}

#if defined(FRONTPANEL_MINI)
	// Pager
	pos_y += margin_bottom;
	LCDDriver_printText("Rotate ENC2", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
#else
	pos_y += margin_bottom;
	LCDDriver_printText("Rotate ENC2 to print next page", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
#endif
	LCD_busy = false;
}

static void SELF_TEST_printResult(bool result, uint16_t pos_y) {
	char pass[] = " OK     ";
	char error[] = " ERROR";

	if (result) {
		LCDDriver_printText(pass, LCDDriver_GetCurrentXOffset(), pos_y, COLOR_GREEN, BG_COLOR, font_size);
	} else {
		LCDDriver_printText(error, LCDDriver_GetCurrentXOffset(), pos_y, COLOR_RED, BG_COLOR, font_size);
	}
}

// events to the encoder
void SELF_TEST_EncRotate(int8_t direction) {
	if (LCD_busy) {
		return;
	}

	LCD_busy = true;
	LCDDriver_Fill(BG_COLOR);
	LCD_busy = false;

	SELF_TEST_current_page += direction;
	if (!HRDW_HAS_LNA_VHF && SELF_TEST_current_page == 4 && direction < 0) {
		SELF_TEST_current_page--;
	}

	if (SELF_TEST_current_page < 0) {
		SELF_TEST_current_page = 0;
	}
	if (SELF_TEST_current_page >= SELF_TEST_pages) {
		SELF_TEST_current_page = SELF_TEST_pages - 1;
		BUTTONHANDLER_SERVICES(0);
	}

	LCD_UpdateQuery.SystemMenuRedraw = true;
}
