#include "rf_unit.h"
#include "audio_filters.h"
#include "front_unit.h"
#include "functions.h"
#include "lcd.h"
#include "main.h"
#include "settings.h"
#include "stm32h7xx_hal.h"
#include "system_menu.h"
#include "trx_manager.h"

static uint8_t getBPFByFreq(uint32_t freq) {
	if (!TRX.RF_Filters) {
		return 255;
	}

	if (freq >= CALIBRATE.RFU_BPF_0_START && freq < CALIBRATE.RFU_BPF_0_END) {
		return 1;
	}
	if (freq >= CALIBRATE.RFU_BPF_1_START && freq < CALIBRATE.RFU_BPF_1_END) {
		return 2;
	}
	if (freq >= CALIBRATE.RFU_BPF_2_START && freq < CALIBRATE.RFU_BPF_2_END) {
		return 3;
	}
	if (freq >= CALIBRATE.RFU_BPF_3_START && freq < CALIBRATE.RFU_BPF_3_END) {
		return 4;
	}
	if (freq >= CALIBRATE.RFU_BPF_4_START && freq < CALIBRATE.RFU_BPF_4_END) {
		return 5;
	}
	if (freq >= CALIBRATE.RFU_BPF_5_START && freq < CALIBRATE.RFU_BPF_5_END) {
		return 6;
	}
	if (freq >= CALIBRATE.RFU_BPF_6_START && freq < CALIBRATE.RFU_BPF_6_END) {
		return 7;
	}
	if (freq >= CALIBRATE.RFU_HPF_START) {
		return 8;
	}
	return 255;
}

void RF_UNIT_UpdateState(bool clean) // pass values to RF-UNIT
{
	bool dualrx_hpf_disabled = false;
	bool dualrx_bpf_disabled = false;

	if (TRX.Dual_RX && (CurrentVFO->Freq < CALIBRATE.RFU_HPF_START || SecondaryVFO->Freq > CALIBRATE.RFU_HPF_START)) {
		dualrx_hpf_disabled = true;
	}
	if (TRX.Dual_RX && getBPFByFreq(CurrentVFO->Freq) != getBPFByFreq(SecondaryVFO->Freq)) {
		dualrx_bpf_disabled = true;
	}

	float32_t att_val = TRX.ATT_DB;
	bool att_val_16 = false, att_val_8 = false, att_val_4 = false, att_val_2 = false, att_val_1 = false, att_val_05 = false;
	if (TRX.ATT && att_val >= 16.0f) {
		att_val_16 = true;
		att_val -= 16.0f;
	}
	if (TRX.ATT && att_val >= 8.0f) {
		att_val_8 = true;
		att_val -= 8.0f;
	}
	if (TRX.ATT && att_val >= 4.0f) {
		att_val_4 = true;
		att_val -= 4.0f;
	}
	if (TRX.ATT && att_val >= 2.0f) {
		att_val_2 = true;
		att_val -= 2.0f;
	}
	if (TRX.ATT && att_val >= 1.0f) {
		att_val_1 = true;
		att_val -= 1.0f;
	}
	if (TRX.ATT && att_val >= 0.5f) {
		att_val_05 = true;
		att_val -= 0.5f;
	}

	uint8_t bpf = getBPFByFreq(CurrentVFO->Freq);
	uint8_t bpf_second = getBPFByFreq(SecondaryVFO->Freq);

	uint8_t band_out = 0;
	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);

	// Transverters
	if (TRX.Transverter_1_2cm && band == BANDID_1_2cm) { // 1.2cm
		band_out = CALIBRATE.EXT_1_2cm;
	}
	if (TRX.Transverter_QO100 && band == BANDID_QO100) { // QO-100
		band_out = CALIBRATE.EXT_QO100;
	}
	if (TRX.Transverter_3cm && band == BANDID_3cm) { // 3cm
		band_out = CALIBRATE.EXT_3cm;
	}
	if (TRX.Transverter_6cm && band == BANDID_6cm) { // 6cm
		band_out = CALIBRATE.EXT_6cm;
	}
	if (TRX.Transverter_13cm && band == BANDID_13cm) { // 13cm
		band_out = CALIBRATE.EXT_13cm;
	}

	if (band < BANDID_13cm) { // 23cm
		band_out = CALIBRATE.EXT_23cm;
	}
	if (band < BANDID_23cm) { // 70cm
		band_out = CALIBRATE.EXT_70cm;
	}
	if (band < BANDID_70cm) { // 2m
		band_out = CALIBRATE.EXT_2m;
	}
	if (band < BANDID_2m) { // FM
		band_out = CALIBRATE.EXT_FM;
	}
	if (band < BANDID_FM) { // 4m
		band_out = CALIBRATE.EXT_4m;
	}
	if (band < BANDID_4m) { // 6m
		band_out = CALIBRATE.EXT_6m;
	}
	if (band < BANDID_6m) { // 10m
		band_out = CALIBRATE.EXT_10m;
	}
	if (band < BANDID_10m) { // CB
		band_out = CALIBRATE.EXT_CB;
	}
	if (band < BANDID_CB) { // 12m
		band_out = CALIBRATE.EXT_12m;
	}
	if (band < BANDID_12m) { // 15m
		band_out = CALIBRATE.EXT_15m;
	}
	if (band < BANDID_15m) { // 17m
		band_out = CALIBRATE.EXT_17m;
	}
	if (band < BANDID_17m) { // 20m
		band_out = CALIBRATE.EXT_20m;
	}
	if (band < BANDID_20m) { // 30m
		band_out = CALIBRATE.EXT_30m;
	}
	if (band < BANDID_30m) { // 40m
		band_out = CALIBRATE.EXT_40m;
	}
	if (band < BANDID_40m) { // 60m
		band_out = CALIBRATE.EXT_60m;
	}
	if (band < BANDID_60m) { // 80m
		band_out = CALIBRATE.EXT_80m;
	}
	if (band < BANDID_80m) { // 160m
		band_out = CALIBRATE.EXT_160m;
	}
	if (band < BANDID_160m) { // 630m
		band_out = CALIBRATE.EXT_630m;
	}
	if (band < BANDID_630m) { // 2200m
		band_out = CALIBRATE.EXT_2200m;
	}

	// ATT
	HAL_GPIO_WritePin(ATT_ON_16_GPIO_Port, ATT_ON_16_Pin, att_val_16);
	HAL_GPIO_WritePin(ATT_ON_8_GPIO_Port, ATT_ON_8_Pin, att_val_8);
	HAL_GPIO_WritePin(ATT_ON_4_GPIO_Port, ATT_ON_4_Pin, att_val_4);
	HAL_GPIO_WritePin(ATT_ON_2_GPIO_Port, ATT_ON_2_Pin, att_val_2);
	HAL_GPIO_WritePin(ATT_ON_1_GPIO_Port, ATT_ON_1_Pin, att_val_1);
	HAL_GPIO_WritePin(ATT_ON_0_5_GPIO_Port, ATT_ON_0_5_Pin, att_val_05);

	// FILTERS
	if (bpf == 1 && !dualrx_bpf_disabled) // 1.5-2.5
	{
		HAL_GPIO_WritePin(BPF_A_GPIO_Port, BPF_A_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(BPF_B_GPIO_Port, BPF_B_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(BPF_OE1_GPIO_Port, BPF_OE1_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(BPF_OE2_GPIO_Port, BPF_OE2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(BPF_OE3_GPIO_Port, BPF_OE3_Pin, GPIO_PIN_RESET);
	} else if (bpf == 2 && !dualrx_bpf_disabled) // 2.4-4.7
	{
		HAL_GPIO_WritePin(BPF_A_GPIO_Port, BPF_A_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(BPF_B_GPIO_Port, BPF_B_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(BPF_OE1_GPIO_Port, BPF_OE1_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(BPF_OE2_GPIO_Port, BPF_OE2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(BPF_OE3_GPIO_Port, BPF_OE3_Pin, GPIO_PIN_RESET);
	} else if (bpf == 3 && !dualrx_bpf_disabled) // 4.7-8
	{
		HAL_GPIO_WritePin(BPF_A_GPIO_Port, BPF_A_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(BPF_B_GPIO_Port, BPF_B_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(BPF_OE1_GPIO_Port, BPF_OE1_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(BPF_OE2_GPIO_Port, BPF_OE2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(BPF_OE3_GPIO_Port, BPF_OE3_Pin, GPIO_PIN_RESET);
	} else if (bpf == 4 && !dualrx_bpf_disabled) // 8-14.7
	{
		HAL_GPIO_WritePin(BPF_A_GPIO_Port, BPF_A_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(BPF_B_GPIO_Port, BPF_B_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(BPF_OE1_GPIO_Port, BPF_OE1_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(BPF_OE2_GPIO_Port, BPF_OE2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(BPF_OE3_GPIO_Port, BPF_OE3_Pin, GPIO_PIN_RESET);
	}

	else if (bpf == 5 && !dualrx_bpf_disabled) // 14.7-22.1
	{
		HAL_GPIO_WritePin(BPF_A_GPIO_Port, BPF_A_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(BPF_B_GPIO_Port, BPF_B_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(BPF_OE1_GPIO_Port, BPF_OE1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(BPF_OE2_GPIO_Port, BPF_OE2_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(BPF_OE3_GPIO_Port, BPF_OE3_Pin, GPIO_PIN_RESET);
	} else if (bpf == 6 && !dualrx_bpf_disabled) // 22.1-32
	{
		HAL_GPIO_WritePin(BPF_A_GPIO_Port, BPF_A_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(BPF_B_GPIO_Port, BPF_B_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(BPF_OE1_GPIO_Port, BPF_OE1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(BPF_OE2_GPIO_Port, BPF_OE2_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(BPF_OE3_GPIO_Port, BPF_OE3_Pin, GPIO_PIN_RESET);
	} else if (bpf == 8 || (TRX.Dual_RX && !dualrx_hpf_disabled)) // HPF 60+MHz
	{
		HAL_GPIO_WritePin(BPF_A_GPIO_Port, BPF_A_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(BPF_B_GPIO_Port, BPF_B_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(BPF_OE1_GPIO_Port, BPF_OE1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(BPF_OE2_GPIO_Port, BPF_OE2_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(BPF_OE3_GPIO_Port, BPF_OE3_Pin, GPIO_PIN_RESET);
	} else if (bpf == 7 && !dualrx_bpf_disabled) // 135-150MHz
	{
		HAL_GPIO_WritePin(BPF_A_GPIO_Port, BPF_A_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(BPF_B_GPIO_Port, BPF_B_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(BPF_OE1_GPIO_Port, BPF_OE1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(BPF_OE2_GPIO_Port, BPF_OE2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(BPF_OE3_GPIO_Port, BPF_OE3_Pin, GPIO_PIN_SET);
	} else // bypass
	{
		HAL_GPIO_WritePin(BPF_A_GPIO_Port, BPF_A_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(BPF_B_GPIO_Port, BPF_B_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(BPF_OE1_GPIO_Port, BPF_OE1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(BPF_OE2_GPIO_Port, BPF_OE2_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(BPF_OE3_GPIO_Port, BPF_OE3_Pin, GPIO_PIN_RESET);
	}

	// RF Unit
	HAL_GPIO_WritePin(RFUNIT_RCLK_GPIO_Port, RFUNIT_RCLK_Pin, GPIO_PIN_RESET); // latch
	MINI_DELAY
	for (uint8_t registerNumber = 0; registerNumber < 8; registerNumber++) {
		HAL_GPIO_WritePin(RFUNIT_CLK_GPIO_Port, RFUNIT_CLK_Pin, GPIO_PIN_RESET); // data block
		MINI_DELAY
		HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_RESET); // data
		MINI_DELAY
		if (!clean) {
			// U3-D7 BAND_OUT_4
			if (registerNumber == 0 && bitRead(band_out, 3)) {
				HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
			}
			// U3-D6 Net_RX/TX
			if (registerNumber == 1 && TRX_on_TX && CurrentVFO->Mode != TRX_MODE_LOOPBACK) {
				HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
			}
			// U3-D5 BAND_OUT_3
			if (registerNumber == 2 && bitRead(band_out, 2)) {
				HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
			}
			// U3-D4 VHF_AMP_BIAS_OUT
			if (registerNumber == 3 && TRX_on_TX && CurrentVFO->Mode != TRX_MODE_LOOPBACK && CurrentVFO->Freq > 70000000) {
				HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
			}
			// U3-D3 HF-VHF-SELECT
			if (registerNumber == 4 && TRX_on_TX && CurrentVFO->Mode != TRX_MODE_LOOPBACK && CurrentVFO->Freq > 70000000) {
				HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
			}
			// U3-D2 LNA
			if (registerNumber == 5 && TRX.LNA) {
				HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
			}
			// U3-D1 BAND_OUT_2
			if (registerNumber == 6 && bitRead(band_out, 1)) {
				HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
			}
			// U3-D0 BAND_OUT_1
			if (registerNumber == 7 && bitRead(band_out, 0)) {
				HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
			}
		}
		MINI_DELAY
		HAL_GPIO_WritePin(RFUNIT_CLK_GPIO_Port, RFUNIT_CLK_Pin, GPIO_PIN_SET);
	}
	MINI_DELAY
	HAL_GPIO_WritePin(RFUNIT_CLK_GPIO_Port, RFUNIT_CLK_Pin, GPIO_PIN_RESET);
	MINI_DELAY
	HAL_GPIO_WritePin(RFUNIT_RCLK_GPIO_Port, RFUNIT_RCLK_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(RFUNIT_OE_GPIO_Port, RFUNIT_OE_Pin, GPIO_PIN_RESET);
}

void RF_UNIT_ProcessSensors(void) {
#define B16_RANGE 65535.0f
#define B14_RANGE 16383.0f

	HAL_ADCEx_InjectedPollForConversion(&hadc1, 100); // wait if prev conversion not ended

	// PWR Voltage
	float32_t PWR_Voltage = (float32_t)HAL_ADCEx_InjectedGetValue(&hadc2, ADC_INJECTED_RANK_3) * TRX_STM32_VREF / B16_RANGE;
	PWR_Voltage = PWR_Voltage / (CALIBRATE.PWR_VLT_Calibration / (10000.0f + CALIBRATE.PWR_VLT_Calibration));
	if (fabsf(PWR_Voltage - TRX_PWR_Voltage) > 0.3f) {
		TRX_PWR_Voltage = TRX_PWR_Voltage * 0.99f + PWR_Voltage * 0.01f;
	}
	if (fabsf(PWR_Voltage - TRX_PWR_Voltage) > 1.0f) {
		TRX_PWR_Voltage = PWR_Voltage;
	}

	float32_t PWR_Current_Voltage = (float32_t)HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_3) * TRX_STM32_VREF / B16_RANGE;
	float32_t PWR_Current = (PWR_Current_Voltage - CALIBRATE.PWR_CUR_Calibration) / 0.100f; // 0.066 - ACS712-30, 0.100 - ACS712-20
	if (fabsf(PWR_Current - TRX_PWR_Current) > 0.1f) {
		TRX_PWR_Current = TRX_PWR_Current * 0.95f + PWR_Current * 0.05f;
	}
	if (fabsf(PWR_Current - TRX_PWR_Current) > 1.0f) {
		TRX_PWR_Current = PWR_Current;
	}

	// println(PWR_Current_Voltage, " ", PWR_Current, " ", TRX_PWR_Current, " ", TRX_STM32_VREF);

	TRX_VBAT_Voltage = (float32_t)(HAL_ADCEx_InjectedGetValue(&hadc3, ADC_INJECTED_RANK_3)) * TRX_STM32_VREF / B14_RANGE; // why 14bit?

	// SWR
	float32_t forward = (float32_t)(HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_1)) * TRX_STM32_VREF / B16_RANGE;
	float32_t backward = (float32_t)(HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_2)) * TRX_STM32_VREF / B16_RANGE;

	// println("-", VBAT_Voltage, " ", forward, " ", backward);
	if (forward > 3.2f || backward > 3.2f) {
		TRX_PWR_ALC_SWR_OVERFLOW = true;
	}

	// forward = forward / (510.0f / (0.0f + 510.0f)); // adjust the voltage based on the voltage divider (0 ohm and 510 ohm)
	if (forward < 0.05f) // do not measure less than 50mV
	{
		TRX_VLT_forward = 0.0f;
		TRX_VLT_backward = 0.0f;
		TRX_PWR_Forward = 0.0f;
		TRX_PWR_Backward = 0.0f;
		TRX_SWR = 1.0f;
	} else {
		// Transformation ratio of the SWR meter
		if (CurrentVFO->Freq >= 80000000) {
			forward = forward * CALIBRATE.SWR_FWD_Calibration_VHF;
		} else if (CurrentVFO->Freq >= 40000000) {
			forward = forward * CALIBRATE.SWR_FWD_Calibration_6M;
		} else {
			forward = forward * CALIBRATE.SWR_FWD_Calibration_HF;
		}

		forward += 0.21f; // drop on diode

		// backward = backward / (510.0f / (0.0f + 510.0f)); // adjust the voltage based on the voltage divider (0 ohm and 510 ohm)
		if (backward >= 0.05f) // do not measure less than 50mV
		{
			// Transformation ratio of the SWR meter
			if (CurrentVFO->Freq >= 80000000) {
				backward = backward * CALIBRATE.SWR_BWD_Calibration_VHF;
			} else if (CurrentVFO->Freq >= 40000000) {
				backward = backward * CALIBRATE.SWR_BWD_Calibration_6M;
			} else {
				backward = backward * CALIBRATE.SWR_BWD_Calibration_HF;
			}

			backward += 0.21f; // drop on diode
		} else {
			backward = 0.001f;
		}

		// smooth process
		TRX_VLT_forward = TRX_VLT_forward + (forward - TRX_VLT_forward) / 2;
		TRX_VLT_backward = TRX_VLT_backward + (backward - TRX_VLT_backward) / 2;
		if ((TRX_VLT_forward - TRX_VLT_backward) > 0.0f) {
			TRX_SWR = (TRX_VLT_forward + TRX_VLT_backward) / (TRX_VLT_forward - TRX_VLT_backward);
		}

		if (TRX_VLT_backward > TRX_VLT_forward) {
			TRX_SWR = 9.9f;
		}
		if (TRX_SWR > 9.9f) {
			TRX_SWR = 9.9f;
		}

		TRX_PWR_Forward = (TRX_VLT_forward * TRX_VLT_forward) / 50.0f;
		if (TRX_PWR_Forward < 0.0f) {
			TRX_PWR_Forward = 0.0f;
		}
		TRX_PWR_Backward = (TRX_VLT_backward * TRX_VLT_backward) / 50.0f;

		if (TRX_PWR_Backward < 0.0f) {
			TRX_PWR_Backward = 0.0f;
		}

		if (TRX_PWR_Forward < TRX_PWR_Backward) {
			TRX_PWR_Backward = TRX_PWR_Forward;
		}
	}

#define smooth_stick_time 100
#define smooth_up_coeff 0.3f
#define smooth_down_coeff 0.03f
	static uint32_t forw_smooth_time = 0;
	if ((HAL_GetTick() - forw_smooth_time) > smooth_stick_time) {
		TRX_PWR_Forward_SMOOTHED = TRX_PWR_Forward_SMOOTHED * (1.0f - smooth_down_coeff) + TRX_PWR_Forward * smooth_down_coeff;
		TRX_PWR_Backward_SMOOTHED = TRX_PWR_Backward_SMOOTHED * (1.0f - smooth_down_coeff) + TRX_PWR_Backward * smooth_down_coeff;
	}

	if (TRX_PWR_Forward > TRX_PWR_Forward_SMOOTHED) {
		TRX_PWR_Forward_SMOOTHED = TRX_PWR_Forward_SMOOTHED * (1.0f - smooth_up_coeff) + TRX_PWR_Forward * smooth_up_coeff;
		TRX_PWR_Backward_SMOOTHED = TRX_PWR_Backward_SMOOTHED * (1.0f - smooth_up_coeff) + TRX_PWR_Backward * smooth_up_coeff;
		forw_smooth_time = HAL_GetTick();
	}

	TRX_SWR_SMOOTHED = TRX_SWR_SMOOTHED * (1.0f - smooth_down_coeff) + TRX_SWR * smooth_down_coeff;

	// TANGENT
	float32_t SW1_Voltage = (float32_t)HAL_ADCEx_InjectedGetValue(&hadc2, ADC_INJECTED_RANK_1) * TRX_STM32_VREF / B16_RANGE * 1000.0f;
	float32_t SW2_Voltage = (float32_t)HAL_ADCEx_InjectedGetValue(&hadc2, ADC_INJECTED_RANK_2) * TRX_STM32_VREF / B16_RANGE * 1000.0f;
	// println(SW1_Voltage, " ", SW2_Voltage);

	// Yaesu MH-48
	for (uint16_t tb = 0; tb < (sizeof(PERIPH_FrontPanel_TANGENT_MH48) / sizeof(PERIPH_FrontPanel_Button)); tb++) {
		if (TRX_on_TX) {
			break;
		}

		if ((SW2_Voltage < 500.0f || SW2_Voltage > 3100.0f) && PERIPH_FrontPanel_TANGENT_MH48[tb].channel == 1) {
			FRONTPANEL_CheckButton(&PERIPH_FrontPanel_TANGENT_MH48[tb], SW1_Voltage);
		}
		if (SW1_Voltage > 2800.0f & PERIPH_FrontPanel_TANGENT_MH48[tb].channel == 2) {
			FRONTPANEL_CheckButton(&PERIPH_FrontPanel_TANGENT_MH48[tb], SW2_Voltage);
		}
	}

	HAL_ADCEx_InjectedStart(&hadc1); // start next manual conversion
}
