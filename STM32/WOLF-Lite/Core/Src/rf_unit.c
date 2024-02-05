#include "rf_unit.h"
#include "audio_filters.h"
#include "front_unit.h"
#include "functions.h"
#include "hardware.h"
#include "lcd.h"
#include "main.h"
#include "settings.h"
#include "system_menu.h"
#include "trx_manager.h"

bool FAN_Active = false;
static bool FAN_Active_old = false;

static uint8_t getBPFByFreq(uint32_t freq) {
	if (freq >= CALIBRATE.RFU_BPF_0_START && freq < CALIBRATE.RFU_BPF_0_END) {
		return 0;
	}
	if (freq >= CALIBRATE.RFU_BPF_1_START && freq < CALIBRATE.RFU_BPF_1_END) {
		return 1;
	}
	if (freq >= CALIBRATE.RFU_BPF_2_START && freq < CALIBRATE.RFU_BPF_2_END) {
		return 2;
	}
	if (freq >= CALIBRATE.RFU_BPF_3_START && freq < CALIBRATE.RFU_BPF_3_END) {
		return 3;
	}
	if (freq >= CALIBRATE.RFU_BPF_4_START && freq < CALIBRATE.RFU_BPF_4_END) {
		return 4;
	}
	if (freq >= CALIBRATE.RFU_BPF_5_START && freq < CALIBRATE.RFU_BPF_5_END) {
		return 5;
	}
	if (freq >= CALIBRATE.RFU_BPF_6_START && freq < CALIBRATE.RFU_BPF_6_END) {
		return 6;
	}
	return 255;
}

void RF_UNIT_UpdateState(bool clean) // pass values to RF-UNIT
{}

void RF_UNIT_ProcessSensors(void) {
#define B12_RANGE 4096.0f
	static float32_t SW1_Voltage = 0.0f;
	static float32_t SW2_Voltage = 0.0f;
	static float32_t SW1_Voltage_new = 0.0f;
	static float32_t SW2_Voltage_new = 0.0f;
	static float32_t k = 0.0f; // averaging ratio

	HAL_ADCEx_InjectedPollForConversion(&hadc3, 100); // wait if prev conversion not ended

	// THERMAL
	TRX_RF_Temperature = 0;

	// VBAT
	float32_t cpu_vbat = (float32_t)(HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_3)) * 3.3f / 4096.0f;
	TRX_VBAT_Voltage = TRX_VBAT_Voltage * 0.9f + cpu_vbat * 2.0f * 0.1f;

	// PWR Voltage
	float32_t PWR_Voltage = (float32_t)HAL_ADCEx_InjectedGetValue(&hadc2, ADC_INJECTED_RANK_1) * TRX_STM32_VREF / B12_RANGE;
	PWR_Voltage = PWR_Voltage * (CALIBRATE.PWR_VLT_Calibration) / 100.0f;
	if (fabsf(PWR_Voltage - TRX_PWR_Voltage) > 0.3f) {
		TRX_PWR_Voltage = TRX_PWR_Voltage * 0.99f + PWR_Voltage * 0.01f;
	}
	if (fabsf(PWR_Voltage - TRX_PWR_Voltage) > 1.0f) {
		TRX_PWR_Voltage = PWR_Voltage;
	}

	// SWR
	// TRX_ALC_IN = (float32_t)HAL_ADCEx_InjectedGetValue(&hadc2, ADC_INJECTED_RANK_2) * TRX_STM32_VREF / B12_RANGE;
	float32_t forward = (float32_t)(HAL_ADCEx_InjectedGetValue(&hadc3, ADC_INJECTED_RANK_1)) * TRX_STM32_VREF / B12_RANGE;
	float32_t backward = (float32_t)(HAL_ADCEx_InjectedGetValue(&hadc3, ADC_INJECTED_RANK_2)) * TRX_STM32_VREF / B12_RANGE;

	if (forward > 3.2f || backward > 3.2f) {
		TRX_PWR_ALC_SWR_OVERFLOW = true;
	}

	// forward = forward / (510.0f / (0.0f + 510.0f)); // adjust the voltage based on the voltage divider (0 ohm and 510 ohm)
	if (forward < 0.05f) // do not measure less than 100mV
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
		if (backward >= 0.05f) // do not measure less than 100mV
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
	SW1_Voltage_new = (float32_t)HAL_ADCEx_InjectedGetValue(&hadc2, ADC_INJECTED_RANK_3) * TRX_STM32_VREF / B12_RANGE * 1000.0f;
	SW2_Voltage_new = (float32_t)HAL_ADCEx_InjectedGetValue(&hadc2, ADC_INJECTED_RANK_4) * TRX_STM32_VREF / B12_RANGE * 1000.0f;

	SW1_Voltage = (SW1_Voltage * k + SW1_Voltage_new * (1.0f - k));
	SW2_Voltage = (SW2_Voltage * k + SW2_Voltage_new * (1.0f - k));

	//	println("SW1_Voltage - ", (double) SW1_Voltage, "  **  SW2_Voltage - ", (double) SW2_Voltage);

	// Yaesu MH-48
	for (uint16_t tb = 0; tb < (sizeof(PERIPH_FrontPanel_TANGENT_MH48) / sizeof(PERIPH_FrontPanel_Button)); tb++) {
		if ((SW2_Voltage < 500.0f || SW2_Voltage > 3100.0f) && PERIPH_FrontPanel_TANGENT_MH48[tb].channel == 1) {
			FRONTPANEL_CheckButton(&PERIPH_FrontPanel_TANGENT_MH48[tb], SW1_Voltage);
		}
		if (SW1_Voltage > 2800.0f & PERIPH_FrontPanel_TANGENT_MH48[tb].channel == 2) {
			FRONTPANEL_CheckButton(&PERIPH_FrontPanel_TANGENT_MH48[tb], SW2_Voltage);
		}
	}

	HAL_ADCEx_InjectedStart(&hadc3); // start next manual conversion
}
