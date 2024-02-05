#include "rf_unit.h"
#include "atu.h"
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
		return 8;
	}
	if (freq >= CALIBRATE.RFU_BPF_2_START && freq < CALIBRATE.RFU_BPF_2_END) {
		return 7;
	}
	if (freq >= CALIBRATE.RFU_BPF_3_START && freq < CALIBRATE.RFU_BPF_3_END) {
		return 5;
	}
	if (freq >= CALIBRATE.RFU_BPF_4_START && freq < CALIBRATE.RFU_BPF_4_END) {
		return 6;
	}
	if (freq >= CALIBRATE.RFU_BPF_5_START && freq < CALIBRATE.RFU_BPF_5_END) {
		return 3;
	}
	if (freq >= CALIBRATE.RFU_BPF_6_START && freq < CALIBRATE.RFU_BPF_6_END) {
		return 4;
	}
	if (freq >= CALIBRATE.RFU_HPF_START) {
		return 2;
	}

	return 255;
}

void RF_UNIT_UpdateState(bool clean) // pass values to RF-UNIT
{
	if (TRX_Tune && CurrentVFO->RXFreqAfterTransverters <= 70000000) {
		ATU_Process();
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

	uint8_t lpf_index = 6;                                // 12-10m
	if (CurrentVFO->RXFreqAfterTransverters <= 2000000) { // 160m
		lpf_index = 1;
	}
	if (CurrentVFO->RXFreqAfterTransverters > 2000000 && CurrentVFO->RXFreqAfterTransverters <= 5000000) { // 80m
		lpf_index = 2;
	}
	if (CurrentVFO->RXFreqAfterTransverters > 5000000 && CurrentVFO->RXFreqAfterTransverters <= 9000000) { // 40m
		lpf_index = 3;
	}
	if (CurrentVFO->RXFreqAfterTransverters > 9000000 && CurrentVFO->RXFreqAfterTransverters <= 16000000) { // 30m,20m
		lpf_index = 4;
	}
	if (CurrentVFO->RXFreqAfterTransverters > 16000000 && CurrentVFO->RXFreqAfterTransverters <= 22000000) { // 17m,15m
		lpf_index = 5;
	}

// Shift array
#define SHIFT_ARRAY_SIZE 48
	bool shift_array[SHIFT_ARRAY_SIZE];
	static bool shift_array_old[SHIFT_ARRAY_SIZE];

	if (CALIBRATE.RF_unit_type == RF_UNIT_HF) {
		shift_array[47] = false;                                          // U17-D0 unused
		shift_array[46] = TRX.BluetoothAudio_Enabled;                     // U17-D1 Bluetooth
		shift_array[45] = TRX.LNA;                                        // U17-D2 LNA
		shift_array[44] = !att_val_05;                                    // U17-D3 ATT_ON_05
		shift_array[43] = (bpf == 5 || bpf == 6 || bpf == 7 || bpf == 8); // U17-D4 BPF_2_EN
		shift_array[42] = (bpf == 1 || bpf == 2 || bpf == 3 || bpf == 4); // U17-D5 BPF_EN
		shift_array[41] = (bpf == 1 || bpf == 2);                         // U17-D6 BPF_A1
		shift_array[40] = (bpf == 1 || bpf == 3);                         // U17-D7 BPF_A0

		shift_array[39] = TRX.LNA;                // U16-D0 Net_LNA
		shift_array[38] = !att_val_2;             // U16-D1 ATT_ON_2
		shift_array[37] = !att_val_1;             // U16-D2 ATT_ON_1
		shift_array[36] = !att_val_16;            // U16-D3 ATT_ON_16
		shift_array[35] = !att_val_8;             // U16-D4 ATT_ON_8
		shift_array[34] = !att_val_4;             // U16-D5 ATT_ON_4
		shift_array[33] = (bpf == 5 || bpf == 6); // U16-D6 BPF_2_A1
		shift_array[32] = (bpf == 5 || bpf == 7); // U16-D7 BPF_2_A0

		shift_array[31] = lpf_index == 6;                                     // U3-D0 LPF_6
		shift_array[30] = lpf_index == 5;                                     // U3-D1 LPF_5
		shift_array[29] = lpf_index == 4;                                     // U3-D2 LPF_4
		shift_array[28] = lpf_index == 3;                                     // U3-D3 LPF_3
		shift_array[27] = lpf_index == 2;                                     // U3-D4 LPF_2
		shift_array[26] = lpf_index == 1;                                     // U3-D5 LPF_1
		shift_array[25] = false;                                              // U3-D6 unused
		shift_array[24] = TRX_on_TX && CurrentVFO->Mode != TRX_MODE_LOOPBACK; // U3-D7 RX/TX

		shift_array[23] = TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 5);         // U4-D0 TUN_C6
		shift_array[22] = TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 4);         // U4-D1 TUN_C5
		shift_array[21] = TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 3);         // U4-D2 TUN_C4
		shift_array[20] = TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 2);         // U4-D3 TUN_C3
		shift_array[19] = TRX_on_TX && CurrentVFO->Mode != TRX_MODE_LOOPBACK; // U4-D4 RX/TX
		shift_array[18] = TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 0);         // U4-D5 TUN_C1
		shift_array[17] = TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 1);         // U4-D6 TUN_C2
		shift_array[16] = false;                                              // U4-D7 unused

		shift_array[15] = false;                                      // U5-D0 unused
		shift_array[14] = TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 5); // U5-D1 TUN_I6
		shift_array[13] = TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 4); // U5-D2 TUN_I5
		shift_array[12] = TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 3); // U5-D3 TUN_I4
		shift_array[11] = TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 2); // U5-D4 TUN_I3
		shift_array[10] = TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 1); // U5-D5 TUN_I2
		shift_array[9] = TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 0);  // U5-D6 TUN_I1
		shift_array[8] = TRX.TUNER_Enabled && TRX.ATU_T;              // U5-D7 TUN_T

		shift_array[7] = TRX_on_TX; // U7-D0 J1
		shift_array[6] = TRX_on_TX; // U7-D1 J2
		shift_array[5] = TRX_on_TX; // U7-D2 J3
		shift_array[4] = TRX_on_TX; // U7-D2 J4
		shift_array[3] = TRX_on_TX; // U7-D2 J5
		shift_array[2] = TRX_on_TX; // U7-D2 J6
		shift_array[1] = TRX_on_TX; // U7-D2 J7
		shift_array[0] = TRX_on_TX; // U7-D2 J8
	}

	if (CALIBRATE.RF_unit_type == RF_UNIT_VHF) {
		shift_array[47] = false;                                          // U17-D0 unused
		shift_array[46] = TRX.BluetoothAudio_Enabled;                     // U17-D1 Bluetooth
		shift_array[45] = TRX.LNA;                                        // U17-D2 LNA
		shift_array[44] = !att_val_05;                                    // U17-D3 ATT_ON_05
		shift_array[43] = (bpf == 5 || bpf == 6 || bpf == 7 || bpf == 8); // U17-D4 BPF_2_EN
		shift_array[42] = (bpf == 1 || bpf == 2 || bpf == 3 || bpf == 4); // U17-D5 BPF_EN
		shift_array[41] = (bpf == 1 || bpf == 2);                         // U17-D6 BPF_A1
		shift_array[40] = (bpf == 1 || bpf == 3);                         // U17-D7 BPF_A0

		shift_array[39] = TRX.LNA;                // U16-D0 Net_LNA
		shift_array[38] = !att_val_2;             // U16-D1 ATT_ON_2
		shift_array[37] = !att_val_1;             // U16-D2 ATT_ON_1
		shift_array[36] = !att_val_16;            // U16-D3 ATT_ON_16
		shift_array[35] = !att_val_8;             // U16-D4 ATT_ON_8
		shift_array[34] = !att_val_4;             // U16-D5 ATT_ON_4
		shift_array[33] = (bpf == 5 || bpf == 6); // U16-D6 BPF_2_A1
		shift_array[32] = (bpf == 5 || bpf == 7); // U16-D7 BPF_2_A0

		shift_array[31] = lpf_index == 6;                                     // U3-D0 Net_B6
		shift_array[30] = lpf_index == 5;                                     // U3-D1 Net_B5
		shift_array[29] = lpf_index == 4;                                     // U3-D2 Net_B4
		shift_array[28] = lpf_index == 3;                                     // U3-D3 Net_B3
		shift_array[27] = lpf_index == 2;                                     // U3-D4 Net_B2
		shift_array[26] = lpf_index == 1;                                     // U3-D5 Net_B1
		shift_array[25] = TRX_on_TX && CurrentVFO->Mode != TRX_MODE_LOOPBACK; // U3-D6 Net_RX/TX
		shift_array[24] = TRX_on_TX && CurrentVFO->Mode != TRX_MODE_LOOPBACK; // U3-D7 RX/TX

		shift_array[23] = false;                                                                                                 // U7-D0 unused
		shift_array[22] = false;                                                                                                 // U7-D1 unused
		shift_array[21] = false;                                                                                                 // U7-D2 unused
		shift_array[20] = CurrentVFO->RXFreqAfterTransverters >= 60000000;                                                       // U7-D3 VHF_HF
		shift_array[19] = TRX_on_TX && CurrentVFO->Mode != TRX_MODE_LOOPBACK;                                                    // U7-D4 PTT_OUT
		shift_array[18] = false;                                                                                                 // U7-D5 unused
		shift_array[17] = TRX_on_TX && CurrentVFO->Mode != TRX_MODE_LOOPBACK && CurrentVFO->RXFreqAfterTransverters >= 60000000; // U7-D6 TX_VHF_R
		shift_array[16] = TRX_on_TX && CurrentVFO->Mode != TRX_MODE_LOOPBACK && CurrentVFO->RXFreqAfterTransverters >= 60000000; // U7-D7 TX_VHF

		shift_array[15] = false; // unused
		shift_array[14] = false; // unused
		shift_array[13] = false; // unused
		shift_array[12] = false; // unused
		shift_array[11] = false; // unused
		shift_array[10] = false; // unused
		shift_array[9] = false;  // unused
		shift_array[8] = false;  // unused

		shift_array[7] = false; // unused
		shift_array[6] = false; // unused
		shift_array[5] = false; // unused
		shift_array[4] = false; // unused
		shift_array[3] = false; // unused
		shift_array[2] = false; // unused
		shift_array[1] = false; // unused
		shift_array[0] = false; // unused
	}

	/// Set array
	bool array_equal = true;
	for (uint8_t i = 0; i < SHIFT_ARRAY_SIZE; i++) {
		if (shift_array[i] != shift_array_old[i]) {
			shift_array_old[i] = shift_array[i];
			array_equal = false;
		}
	}
	if (array_equal && !clean) {
		return;
	}

	HAL_GPIO_WritePin(RFUNIT_RCLK_GPIO_Port, RFUNIT_RCLK_Pin, GPIO_PIN_RESET); // latch
	MINI_DELAY
	for (uint8_t registerNumber = 0; registerNumber < SHIFT_ARRAY_SIZE; registerNumber++) {
		HAL_GPIO_WritePin(RFUNIT_CLK_GPIO_Port, RFUNIT_CLK_Pin, GPIO_PIN_RESET); // data block
		MINI_DELAY
		HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_RESET); // data
		MINI_DELAY
		if (!clean) {
			if (shift_array[registerNumber]) {
				SET_DATA_PIN;
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
	PWR_Voltage = PWR_Voltage * CALIBRATE.PWR_VLT_Calibration / 100.0f;
	if (fabsf(PWR_Voltage - TRX_PWR_Voltage) > 0.3f) {
		TRX_PWR_Voltage = TRX_PWR_Voltage * 0.99f + PWR_Voltage * 0.01f;
	}
	if (fabsf(PWR_Voltage - TRX_PWR_Voltage) > 1.0f) {
		TRX_PWR_Voltage = PWR_Voltage;
	}

	TRX_VBAT_Voltage = (float32_t)(HAL_ADCEx_InjectedGetValue(&hadc3, ADC_INJECTED_RANK_3)) * TRX_STM32_VREF / B14_RANGE; // why 14bit?

	// SWR
	float32_t forward = (float32_t)(HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_2)) * TRX_STM32_VREF / B16_RANGE;
	float32_t backward = (float32_t)(HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_1)) * TRX_STM32_VREF / B16_RANGE;

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
	static float32_t SW1_Voltage = 0.0f;
	static float32_t SW2_Voltage = 0.0f;
	float32_t SW1_Voltage_now = (float32_t)HAL_ADCEx_InjectedGetValue(&hadc2, ADC_INJECTED_RANK_1) * TRX_STM32_VREF / B16_RANGE * 1000.0f;
	float32_t SW2_Voltage_now = (float32_t)HAL_ADCEx_InjectedGetValue(&hadc2, ADC_INJECTED_RANK_2) * TRX_STM32_VREF / B16_RANGE * 1000.0f;

	bool SW_Voltage_Stabilized = true;
	if (fabsf(SW1_Voltage - SW1_Voltage_now) > 100.0f || fabsf(SW2_Voltage - SW2_Voltage_now) > 100.0f) {
		SW_Voltage_Stabilized = false;
	}
	SW1_Voltage = SW1_Voltage * 0.2f + SW1_Voltage_now * 0.8f;
	SW2_Voltage = SW2_Voltage * 0.2f + SW2_Voltage_now * 0.8f;

	// println((double)SW1_Voltage, " ", (double)SW2_Voltage);

	// Yaesu MH-48
	if (SW_Voltage_Stabilized) {
		for (uint16_t tb = 0; tb < (sizeof(PERIPH_FrontPanel_TANGENT_MH48) / sizeof(PERIPH_FrontPanel_Button)); tb++) {
			if (SW2_Voltage < 500.0f && PERIPH_FrontPanel_TANGENT_MH48[tb].channel == 1) {
				FRONTPANEL_CheckButton(&PERIPH_FrontPanel_TANGENT_MH48[tb], SW1_Voltage);
			} else if (SW1_Voltage > 2500.0f & PERIPH_FrontPanel_TANGENT_MH48[tb].channel == 2) {
				FRONTPANEL_CheckButton(&PERIPH_FrontPanel_TANGENT_MH48[tb], SW2_Voltage);
			} else {
				FRONTPANEL_CheckButton(&PERIPH_FrontPanel_TANGENT_MH48[tb], 3500.0f);
			}
		}
	}

	HAL_ADCEx_InjectedStart(&hadc1); // start next manual conversion
}
