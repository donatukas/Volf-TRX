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

bool FAN_Active = false;
static bool FAN_Active_old = false;

#define SENS_TABLE_COUNT 24
static const int16_t KTY81_120_sensTable[SENS_TABLE_COUNT][2] = { // table of sensor characteristics
    {-55, 490}, {-50, 515}, {-40, 567}, {-30, 624}, {-20, 684}, {-10, 747},  {0, 815},    {10, 886},   {20, 961},   {25, 1000},  {30, 1040},  {40, 1122},
    {50, 1209}, {60, 1299}, {70, 1392}, {80, 1490}, {90, 1591}, {100, 1696}, {110, 1805}, {120, 1915}, {125, 1970}, {130, 2023}, {140, 2124}, {150, 2211}};

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
	if (CALIBRATE.RF_unit_type == RF_UNIT_BIG || CALIBRATE.RF_unit_type == RF_UNIT_SPLIT || CALIBRATE.RF_unit_type == RF_UNIT_WF_100D) {
		if (freq >= CALIBRATE.RFU_BPF_7_START && freq < CALIBRATE.RFU_BPF_7_END) {
			return 7;
		}
	}
	if (CALIBRATE.RF_unit_type == RF_UNIT_BIG || CALIBRATE.RF_unit_type == RF_UNIT_SPLIT) {
		if (freq >= CALIBRATE.RFU_BPF_8_START && freq < CALIBRATE.RFU_BPF_8_END) {
			return 8;
		}
	}
	if (CALIBRATE.RF_unit_type == RF_UNIT_QRP || CALIBRATE.RF_unit_type == RF_UNIT_RU4PN || CALIBRATE.RF_unit_type == RF_UNIT_KT_100S) {
		if (freq >= CALIBRATE.RFU_HPF_START) {
			return 7;
		}
	}
	return 255;
}

void RF_UNIT_UpdateState(bool clean) // pass values to RF-UNIT
{
	bool dualrx_lpf_disabled = false;
	bool dualrx_bpf_disabled = false;
	if (CALIBRATE.RF_unit_type == RF_UNIT_QRP || CALIBRATE.RF_unit_type == RF_UNIT_RU4PN || CALIBRATE.RF_unit_type == RF_UNIT_KT_100S || CALIBRATE.RF_unit_type == RF_UNIT_WF_100D) {
		if (TRX.Dual_RX && SecondaryVFO->RXFreqAfterTransverters > CALIBRATE.RFU_LPF_END) {
			dualrx_lpf_disabled = true;
		}
		if (TRX.Dual_RX && getBPFByFreq(CurrentVFO->RXFreqAfterTransverters) != getBPFByFreq(SecondaryVFO->RXFreqAfterTransverters)) {
			dualrx_bpf_disabled = true;
		}
	}

	float32_t att_val = TRX.ATT_DB;
	bool att_val_16 = false, att_val_8 = false, att_val_4 = false, att_val_2 = false, att_val_1 = false, att_val_05 = false;
	if (att_val >= 16.0f) {
		att_val_16 = true;
		att_val -= 16.0f;
	}
	if (att_val >= 8.0f) {
		att_val_8 = true;
		att_val -= 8.0f;
	}
	if (att_val >= 4.0f) {
		att_val_4 = true;
		att_val -= 4.0f;
	}
	if (att_val >= 2.0f) {
		att_val_2 = true;
		att_val -= 2.0f;
	}
	if (att_val >= 1.0f) {
		att_val_1 = true;
		att_val -= 1.0f;
	}
	if (att_val >= 0.5f) {
		att_val_05 = true;
		att_val -= 0.5f;
	}

	uint8_t bpf = getBPFByFreq(CurrentVFO->RXFreqAfterTransverters);
	uint8_t bpf_second = getBPFByFreq(SecondaryVFO->RXFreqAfterTransverters);

	bool turn_on_tx_lpf = true;
	if (((HAL_GetTick() - TRX_TX_EndTime) > TX_LPF_TIMEOUT || TRX_TX_EndTime == 0) && !TRX_on_TX) {
		turn_on_tx_lpf = false;
	}

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

	uint8_t currentAnt = TRX_on_TX ? TRX.ANT_TX : TRX.ANT_RX;

#if !defined(FRONTPANEL_WF_100D) && !defined(FRONTPANEL_KT_100S)
	// QRP Version RF Unit ///////////////////////////////////////////////////////////////////////
	if (CALIBRATE.RF_unit_type == RF_UNIT_QRP) {
		HAL_GPIO_WritePin(RFUNIT_RCLK_GPIO_Port, RFUNIT_RCLK_Pin, GPIO_PIN_RESET); // latch
		MINI_DELAY
		for (uint8_t registerNumber = 0; registerNumber < 24; registerNumber++) {
			HAL_GPIO_WritePin(RFUNIT_CLK_GPIO_Port, RFUNIT_CLK_Pin, GPIO_PIN_RESET); // data block
			MINI_DELAY
			HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_RESET); // data
			MINI_DELAY
			if (!clean) {
				// U7-QH LPF_ON
				if (registerNumber == 0 && TRX.RF_Filters && (CurrentVFO->RXFreqAfterTransverters <= CALIBRATE.RFU_LPF_END) && !dualrx_lpf_disabled) {
					SET_DATA_PIN;
				}
				// U7-QG LNA_ON
				if (registerNumber == 1 && !TRX_on_TX && TRX.LNA) {
					SET_DATA_PIN;
				}
				// U7-QF ATT_ON_0.5
				if (registerNumber == 2 && TRX.ATT && att_val_05) {
					SET_DATA_PIN;
				}
				// U7-QE ATT_ON_1
				if (registerNumber == 3 && TRX.ATT && att_val_1) {
					SET_DATA_PIN;
				}
				// U7-QD ATT_ON_2
				if (registerNumber == 4 && TRX.ATT && att_val_2) {
					SET_DATA_PIN;
				}
				// U7-QC ATT_ON_4
				if (registerNumber == 5 && TRX.ATT && att_val_4) {
					SET_DATA_PIN;
				}
				// U7-QB ATT_ON_8
				if (registerNumber == 6 && TRX.ATT && att_val_8) {
					SET_DATA_PIN;
				}
				// U7-QA ATT_ON_16
				if (registerNumber == 7 && TRX.ATT && att_val_16) {
					SET_DATA_PIN;
				}

				// U1-QH NOT USED
				// if (registerNumber == 8)
				// U1-QG BPF_2_A0
				if (registerNumber == 9 && TRX.RF_Filters && !dualrx_bpf_disabled && (bpf == 1 || bpf == 2 || bpf == 5 || bpf == 6)) { // 1,2 - bpf2; 5,6 - bpf1
					SET_DATA_PIN;
				}
				// U1-QF BPF_2_A1
				if (registerNumber == 10 && TRX.RF_Filters && !dualrx_bpf_disabled && (bpf == 4 || bpf == 2 || bpf == 0 || bpf == 6)) { // 4,2 - bpf2; 0,6 - bpf1
					SET_DATA_PIN;
				}
				// U1-QE BPF_2_!EN
				if (registerNumber == 11 && (!TRX.RF_Filters || dualrx_bpf_disabled || (bpf != 1 && bpf != 2 && bpf != 3 && bpf != 4))) { // 1,2,3,4 - bpf2
					SET_DATA_PIN;
				}
				// U1-QD BPF_1_A0
				if (registerNumber == 12 && TRX.RF_Filters && !dualrx_bpf_disabled && (bpf == 1 || bpf == 2 || bpf == 5 || bpf == 6)) { // 1,2 - bpf2; 5,6 - bpf1
					SET_DATA_PIN;
				}
				// U1-QC BPF_1_A1
				if (registerNumber == 13 && TRX.RF_Filters && !dualrx_bpf_disabled && (bpf == 4 || bpf == 2 || bpf == 0 || bpf == 6)) { // 4,2 - bpf2; 0,6 - bpf1
					SET_DATA_PIN;
				}
				// U1-QB BPF_1_!EN
				if (registerNumber == 14 && (!TRX.RF_Filters || dualrx_bpf_disabled || (bpf != 0 && bpf != 5 && bpf != 6 && bpf != 7))) { // 5,6,7,0 - bpf1
					SET_DATA_PIN;
				}
				// U1-QA BPF_ON
				if (registerNumber == 15 && TRX.RF_Filters && !dualrx_bpf_disabled && bpf != 255) {
					SET_DATA_PIN;
				}

				// U3-QH BAND_OUT_0
				if (registerNumber == 16 && bitRead(band_out, 0)) {
					SET_DATA_PIN;
				}
				// U3-QG PTT_OUT
				if (registerNumber == 17 && TRX_on_TX && CurrentVFO->Mode != TRX_MODE_LOOPBACK) {
					SET_DATA_PIN;
				}
				// U3-QF BAND_OUT_2
				if (registerNumber == 18 && bitRead(band_out, 2)) {
					SET_DATA_PIN;
				}
				// U3-QE BAND_OUT_3
				if (registerNumber == 19 && bitRead(band_out, 3)) {
					SET_DATA_PIN;
				}
				// U3-QD TUNE_OUT
				if (registerNumber == 20 && TRX_Tune) {
					SET_DATA_PIN;
				}
				// U3-QC BAND_OUT_1
				if (registerNumber == 21 && bitRead(band_out, 1)) {
					SET_DATA_PIN;
				}
				// U3-QB FAN_OUT
				if (registerNumber == 22) // FAN
				{
					static bool fan_pwm = false;
					if (FAN_Active && TRX_RF_Temperature <= CALIBRATE.FAN_MEDIUM_STOP) { // Temperature at which the fan stops
						FAN_Active = false;
					}
					if (TRX_RF_Temperature >= CALIBRATE.FAN_MEDIUM_START) // Temperature at which the fan starts at half power
					{
						FAN_Active = true;
						fan_pwm = true;
					}
					if (TRX_RF_Temperature >= CALIBRATE.FAN_FULL_START) { // Temperature at which the fan starts at full power
						fan_pwm = false;
					}

					if (FAN_Active != FAN_Active_old) {
						FAN_Active_old = FAN_Active;
						LCD_UpdateQuery.StatusInfoGUI = true;
					}

					if (FAN_Active) {
						if (fan_pwm) // PWM
						{
							uint8_t on_ticks = (CALIBRATE.FAN_Medium_speed / 10);
							uint8_t off_ticks = 10 - (CALIBRATE.FAN_Medium_speed / 10);
							if (on_ticks % 2 == 0 && off_ticks % 2 == 0) {
								on_ticks /= 2;
								off_ticks /= 2;
							}
							if (on_ticks == off_ticks) {
								on_ticks = 1;
								off_ticks = 1;
							}

							static bool pwm_status = false; // true - on false - off
							static uint8_t pwm_ticks = 0;
							pwm_ticks++;
							if (pwm_status) {
								SET_DATA_PIN;
							}
							if ((pwm_status && pwm_ticks == on_ticks) || (!pwm_status && pwm_ticks == off_ticks)) {
								pwm_status = !pwm_status;
								pwm_ticks = 0;
							}
						} else {
							SET_DATA_PIN;
						}
					}
				}
				// U3-QA ANT1_TX_OUT
				if (registerNumber == 23 && currentAnt == TRX_ANT_1 && TRX_on_TX) { // ANT1
					SET_DATA_PIN;
				}
				if (registerNumber == 23 && currentAnt == TRX_ANT_2 && !TRX_on_TX) { // ANT2
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

	// BIG Version RF Unit ///////////////////////////////////////////////////////////////////////
	if (CALIBRATE.RF_unit_type == RF_UNIT_BIG) {
		if (TRX_Tune && CurrentVFO->RXFreqAfterTransverters <= 70000000) {
			ATU_Process();
		}

		HAL_GPIO_WritePin(RFUNIT_RCLK_GPIO_Port, RFUNIT_RCLK_Pin, GPIO_PIN_RESET); // latch
		MINI_DELAY
		for (uint8_t registerNumber = 0; registerNumber < 40; registerNumber++) {
			HAL_GPIO_WritePin(RFUNIT_CLK_GPIO_Port, RFUNIT_CLK_Pin, GPIO_PIN_RESET); // data block
			MINI_DELAY
			HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_RESET); // data
			MINI_DELAY
			if (!clean) {
				// U1-7 HF-VHF-SELECT
				if (registerNumber == 0 && CurrentVFO->RXFreqAfterTransverters >= 70000000) {
					SET_DATA_PIN;
				}
				// U1-6 ATT_ON_1
				if (registerNumber == 1 && !(TRX.ATT && att_val_1)) {
					SET_DATA_PIN;
				}
				// U1-5 ATT_ON_0.5
				if (registerNumber == 2 && !(TRX.ATT && att_val_05)) {
					SET_DATA_PIN;
				}
				// U1-4 ATT_ON_16
				if (registerNumber == 3 && !(TRX.ATT && att_val_16)) {
					SET_DATA_PIN;
				}
				// U1-3 ATT_ON_2
				if (registerNumber == 4 && !(TRX.ATT && att_val_2)) {
					SET_DATA_PIN;
				}
				// U1-2 ATT_ON_4
				if (registerNumber == 5 && !(TRX.ATT && att_val_4)) {
					SET_DATA_PIN;
				}
				// U1-1 ATT_ON_8
				if (registerNumber == 6 && !(TRX.ATT && att_val_8)) {
					SET_DATA_PIN;
				}
				// U1-0 LNA_ON
				if (registerNumber == 7 && !(!TRX_on_TX && TRX.LNA)) {
					SET_DATA_PIN;
				}

				// U3-7 TUN_C_5
				if (registerNumber == 8 && TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 4)) {
					SET_DATA_PIN;
				}
				// U3-6 TUN_C_4
				if (registerNumber == 9 && TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 3)) {
					SET_DATA_PIN;
				}
				// U3-5 BPF_1
				if (registerNumber == 10 && (bpf == 0 || (!TRX_on_TX && TRX.Dual_RX && bpf_second == 0))) {
					SET_DATA_PIN;
				}
				// U3-4 BPF_2
				if (registerNumber == 11 && (bpf == 1 || (!TRX_on_TX && TRX.Dual_RX && bpf_second == 1))) {
					SET_DATA_PIN;
				}
				// U3-3 TX_PTT_OUT
				if (registerNumber == 12 && TRX_on_TX && CurrentVFO->Mode != TRX_MODE_LOOPBACK) {
					SET_DATA_PIN;
				}
				// U3-2 TUN_C_1
				if (registerNumber == 13 && TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 0)) {
					SET_DATA_PIN;
				}
				// U3-1 TUN_C_2
				if (registerNumber == 14 && TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 1)) {
					SET_DATA_PIN;
				}
				// U3-0 TUN_C_3
				if (registerNumber == 15 && TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 2)) {
					SET_DATA_PIN;
				}

				// U2-7 TUN_T
				if (registerNumber == 16 && TRX.TUNER_Enabled && TRX.ATU_T) {
					SET_DATA_PIN;
				}
				// U2-6 TUN_I_5
				if (registerNumber == 17 && TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 4)) {
					SET_DATA_PIN;
				}
				// U2-5 UNUSED
				// if (registerNumber == 18 &&
				// U2-4 VHF_AMP_BIAS_ON
				if (registerNumber == 19 && TRX_on_TX && CurrentVFO->Mode != TRX_MODE_LOOPBACK && CurrentVFO->RXFreqAfterTransverters >= 70000000) {
					SET_DATA_PIN;
				}
				// U2-3 TUN_I_1
				if (registerNumber == 20 && TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 0)) {
					SET_DATA_PIN;
				}
				// U2-2 TUN_I_2
				if (registerNumber == 21 && TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 1)) {
					SET_DATA_PIN;
				}
				// U2-1 TUN_I_3
				if (registerNumber == 22 && TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 2)) {
					SET_DATA_PIN;
				}
				// U2-0 TUN_I_4
				if (registerNumber == 23 && TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 3)) {
					SET_DATA_PIN;
				}

				// U7-7 BPF_6
				if (registerNumber == 24 && (bpf == 5 || (!TRX_on_TX && TRX.Dual_RX && bpf_second == 5))) {
					SET_DATA_PIN;
				}
				// U7-6 BPF_5
				if (registerNumber == 25 && (bpf == 4 || (!TRX_on_TX && TRX.Dual_RX && bpf_second == 4))) {
					SET_DATA_PIN;
				}
				// U7-5 BPF_4
				if (registerNumber == 26 && (bpf == 3 || (!TRX_on_TX && TRX.Dual_RX && bpf_second == 3))) {
					SET_DATA_PIN;
				}
				// U7-4 BPF_3
				if (registerNumber == 27 && (bpf == 2 || (!TRX_on_TX && TRX.Dual_RX && bpf_second == 2))) {
					SET_DATA_PIN;
				}
				// U7-3 BPF_7
				if (registerNumber == 28 && (bpf == 6 || (!TRX_on_TX && TRX.Dual_RX && bpf_second == 6))) {
					SET_DATA_PIN;
				}
				// U7-2 BPF_8
				if (registerNumber == 29 && (bpf == 7 || (!TRX_on_TX && TRX.Dual_RX && bpf_second == 7))) {
					SET_DATA_PIN;
				}
				// U7-1 BPF_9
				if (registerNumber == 30 && (bpf == 8 || (!TRX_on_TX && TRX.Dual_RX && bpf_second == 8))) {
					SET_DATA_PIN;
				}
				// U7-0 HF_AMP_BIAS_ON
				if (registerNumber == 31 && TRX_on_TX && CurrentVFO->Mode != TRX_MODE_LOOPBACK && CurrentVFO->RXFreqAfterTransverters < 70000000) {
					SET_DATA_PIN;
				}

				// U11-7 ANT1-2_OUT
				if (registerNumber == 32 && currentAnt == TRX_ANT_2) {
					SET_DATA_PIN;
				}
				// U11-6 FAN_OUT
				if (registerNumber == 33) {
					static bool fan_pwm = false;
					if (FAN_Active && TRX_RF_Temperature <= CALIBRATE.FAN_MEDIUM_STOP) { // Temperature at which the fan stops
						FAN_Active = false;
					}
					if (TRX_RF_Temperature >= CALIBRATE.FAN_MEDIUM_START) // Temperature at which the fan starts at half power
					{
						FAN_Active = true;
						fan_pwm = true;
					}
					if (TRX_RF_Temperature >= CALIBRATE.FAN_FULL_START) { // Temperature at which the fan starts at full power
						fan_pwm = false;
					}

					if (FAN_Active != FAN_Active_old) {
						FAN_Active_old = FAN_Active;
						LCD_UpdateQuery.StatusInfoGUI = true;
					}

					if (FAN_Active) {
						if (fan_pwm) // PWM
						{
							uint8_t on_ticks = (CALIBRATE.FAN_Medium_speed / 10);
							uint8_t off_ticks = 10 - (CALIBRATE.FAN_Medium_speed / 10);
							if (on_ticks % 2 == 0 && off_ticks % 2 == 0) {
								on_ticks /= 2;
								off_ticks /= 2;
							}
							if (on_ticks == off_ticks) {
								on_ticks = 1;
								off_ticks = 1;
							}

							static bool pwm_status = false; // true - on false - off
							static uint8_t pwm_ticks = 0;
							pwm_ticks++;
							if (pwm_status) {
								SET_DATA_PIN;
							}
							if ((pwm_status && pwm_ticks == on_ticks) || (!pwm_status && pwm_ticks == off_ticks)) {
								pwm_status = !pwm_status;
								pwm_ticks = 0;
							}
						} else {
							SET_DATA_PIN;
						}
					}
				}
				// U11-5 BAND_OUT_3
				if (registerNumber == 34 && bitRead(band_out, 3)) {
					SET_DATA_PIN;
				}
				// U11-4 TX_PTT_OUT
				if (registerNumber == 35 && TRX_on_TX && CurrentVFO->Mode != TRX_MODE_LOOPBACK) {
					SET_DATA_PIN;
				}
				// U11-3 BAND_OUT_1
				if (registerNumber == 36 && bitRead(band_out, 1)) {
					SET_DATA_PIN;
				}
				// U11-2 TUNE_OUT
				if (registerNumber == 37 && TRX_Tune) {
					SET_DATA_PIN;
				}
				// U11-1 BAND_OUT_2
				if (registerNumber == 38 && bitRead(band_out, 2)) {
					SET_DATA_PIN;
				}
				// U11-0 BAND_OUT_0
				if (registerNumber == 39 && bitRead(band_out, 0)) {
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

	// SPLIT Version RF Unit ///////////////////////////////////////////////////////////////////////
	if (CALIBRATE.RF_unit_type == RF_UNIT_SPLIT) {
		if (TRX_Tune && CurrentVFO->RXFreqAfterTransverters <= 70000000) {
			ATU_Process();
		}

		HAL_GPIO_WritePin(RFUNIT_RCLK_GPIO_Port, RFUNIT_RCLK_Pin, GPIO_PIN_RESET); // latch
		MINI_DELAY
		for (uint8_t registerNumber = 0; registerNumber < 48; registerNumber++) {
			HAL_GPIO_WritePin(RFUNIT_CLK_GPIO_Port, RFUNIT_CLK_Pin, GPIO_PIN_RESET); // data block
			MINI_DELAY
			HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_RESET); // data
			MINI_DELAY
			if (!clean) {
				// U5-7 ANT1-2_OUT
				if (registerNumber == 0 && currentAnt == TRX_ANT_2) {
					SET_DATA_PIN;
				}
				// U5-6 TUN_I_4
				if (registerNumber == 1 && TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 3)) {
					SET_DATA_PIN;
				}
				// U5-5 TUN_I_1
				if (registerNumber == 2 && TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 0)) {
					SET_DATA_PIN;
				}
				// U5-4 TUN_I_2
				if (registerNumber == 3 && TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 1)) {
					SET_DATA_PIN;
				}
				// U5-3 TUN_C_4
				if (registerNumber == 4 && TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 3)) {
					SET_DATA_PIN;
				}
				// U5-2 TUN_C_1
				if (registerNumber == 5 && TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 0)) {
					SET_DATA_PIN;
				}
				// U5-1 TUN_C_2
				if (registerNumber == 6 && TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 1)) {
					SET_DATA_PIN;
				}
				// U5-0 FAN_OUT
				if (registerNumber == 7) {
					static bool fan_pwm = false;
					if (FAN_Active && TRX_RF_Temperature <= CALIBRATE.FAN_MEDIUM_STOP) { // Temperature at which the fan stops
						FAN_Active = false;
					}
					if (TRX_RF_Temperature >= CALIBRATE.FAN_MEDIUM_START) // Temperature at which the fan starts at half power
					{
						FAN_Active = true;
						fan_pwm = true;
					}
					if (TRX_RF_Temperature >= CALIBRATE.FAN_FULL_START) { // Temperature at which the fan starts at full power
						fan_pwm = false;
					}

					if (FAN_Active != FAN_Active_old) {
						FAN_Active_old = FAN_Active;
						LCD_UpdateQuery.StatusInfoGUI = true;
					}

					if (FAN_Active) {
						if (fan_pwm) // PWM
						{
							uint8_t on_ticks = (CALIBRATE.FAN_Medium_speed / 10);
							uint8_t off_ticks = 10 - (CALIBRATE.FAN_Medium_speed / 10);
							if (on_ticks % 2 == 0 && off_ticks % 2 == 0) {
								on_ticks /= 2;
								off_ticks /= 2;
							}
							if (on_ticks == off_ticks) {
								on_ticks = 1;
								off_ticks = 1;
							}

							static bool pwm_status = false; // true - on false - off
							static uint8_t pwm_ticks = 0;
							pwm_ticks++;
							if (pwm_status) {
								SET_DATA_PIN;
							}
							if ((pwm_status && pwm_ticks == on_ticks) || (!pwm_status && pwm_ticks == off_ticks)) {
								pwm_status = !pwm_status;
								pwm_ticks = 0;
							}
						} else {
							SET_DATA_PIN;
						}
					}
				}

				// U1-7 TUN_C_5
				if (registerNumber == 8 && TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 4)) {
					SET_DATA_PIN;
				}
				// U1-6 TUN_C_6
				if (registerNumber == 9 && TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 5)) {
					SET_DATA_PIN;
				}
				// U1-5 TUN_C_3
				if (registerNumber == 10 && TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 2)) {
					SET_DATA_PIN;
				}
				// U1-4 TUN_I_7
				if (registerNumber == 11 && TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 6)) {
					SET_DATA_PIN;
				}
				// U1-3 TUN_T
				if (registerNumber == 12 && TRX.TUNER_Enabled && TRX.ATU_T) {
					SET_DATA_PIN;
				}
				// U2-2 TUN_I_5
				if (registerNumber == 13 && TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 4)) {
					SET_DATA_PIN;
				}
				// U2-1 TUN_I_6
				if (registerNumber == 14 && TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 5)) {
					SET_DATA_PIN;
				}
				// U2-0 TUN_I_3
				if (registerNumber == 15 && TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 2)) {
					SET_DATA_PIN;
				}

				// U3-7 BPF_7
				if (registerNumber == 16 && (bpf == 6 || (!TRX_on_TX && TRX.Dual_RX && bpf_second == 6))) {
					SET_DATA_PIN;
				}
				// U3-6 BPF_6
				if (registerNumber == 17 && (bpf == 5 || (!TRX_on_TX && TRX.Dual_RX && bpf_second == 5))) {
					SET_DATA_PIN;
				}
				// U3-5 BPF_5
				if (registerNumber == 18 && (bpf == 4 || (!TRX_on_TX && TRX.Dual_RX && bpf_second == 4))) {
					SET_DATA_PIN;
				}
				// U3-4 BPF_4
				if (registerNumber == 19 && (bpf == 3 || (!TRX_on_TX && TRX.Dual_RX && bpf_second == 3))) {
					SET_DATA_PIN;
				}
				// U3-3 BPF_3
				if (registerNumber == 20 && (bpf == 2 || (!TRX_on_TX && TRX.Dual_RX && bpf_second == 2))) {
					SET_DATA_PIN;
				}
				// U3-2 BPF_2
				if (registerNumber == 21 && (bpf == 1 || (!TRX_on_TX && TRX.Dual_RX && bpf_second == 1))) {
					SET_DATA_PIN;
				}
				// U3-1 BPF_1
				if (registerNumber == 22 && (bpf == 0 || (!TRX_on_TX && TRX.Dual_RX && bpf_second == 0))) {
					SET_DATA_PIN;
				}
				// U3-0 TUN_C_7
				if (registerNumber == 23 && TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 6)) {
					SET_DATA_PIN;
				}

				// U2-7 TX_PTT_OUT
				if (registerNumber == 24 && TRX_on_TX && CurrentVFO->Mode != TRX_MODE_LOOPBACK) {
					SET_DATA_PIN;
				}
				// U2-6 EXT_2
				// if (registerNumber == 25 &&
				// U2-5 EXT_1
				// if (registerNumber == 26 &&
				// U2-4 HF_AMP_BIAS_ON
				if (registerNumber == 27 && TRX_on_TX && CurrentVFO->Mode != TRX_MODE_LOOPBACK && CurrentVFO->RXFreqAfterTransverters < 70000000) {
					SET_DATA_PIN;
				}
				// U2-3 VHF_AMP_BIAS_ON
				if (registerNumber == 28 && TRX_on_TX && CurrentVFO->Mode != TRX_MODE_LOOPBACK && CurrentVFO->RXFreqAfterTransverters >= 70000000) {
					SET_DATA_PIN;
				}
				// U2-2 HF-VHF-SELECT
				if (registerNumber == 29 && CurrentVFO->RXFreqAfterTransverters >= 70000000) {
					SET_DATA_PIN;
				}
				// U2-1 BPF_9
				if (registerNumber == 30 && (bpf == 8 || (!TRX_on_TX && TRX.Dual_RX && bpf_second == 8))) {
					SET_DATA_PIN;
				}
				// U2-0 BPF_8
				if (registerNumber == 31 && (bpf == 7 || (!TRX_on_TX && TRX.Dual_RX && bpf_second == 7))) {
					SET_DATA_PIN;
				}

				// U7-7 ATT_ON_0.5
				if (registerNumber == 32 && !(TRX.ATT && att_val_05)) {
					SET_DATA_PIN;
				}
				// U7-6 ATT_ON_1
				if (registerNumber == 33 && !(TRX.ATT && att_val_1)) {
					SET_DATA_PIN;
				}
				// U7-5 ATT_ON_2
				if (registerNumber == 34 && !(TRX.ATT && att_val_2)) {
					SET_DATA_PIN;
				}
				// U7-4 ATT_ON_4
				if (registerNumber == 35 && !(TRX.ATT && att_val_4)) {
					SET_DATA_PIN;
				}
				// U7-3 EXT_6
				// if (registerNumber == 36 &&
				// U7-2 EXT_5
				// if (registerNumber == 37 &&
				// U7-1 EXT_4
				// if (registerNumber == 38 &&
				// U7-0 EXT_3
				// if (registerNumber == 39 &&

				// U11-7 LNA_ON
				if (registerNumber == 40 && !(!TRX_on_TX && TRX.LNA)) {
					SET_DATA_PIN;
				}
				// U11-6 BAND_OUT_0
				if (registerNumber == 41 && bitRead(band_out, 0)) {
					SET_DATA_PIN;
				}
				// U11-5 BAND_OUT_3
				if (registerNumber == 42 && bitRead(band_out, 3)) {
					SET_DATA_PIN;
				}
				// U11-4 BAND_OUT_2
				if (registerNumber == 43 && bitRead(band_out, 2)) {
					SET_DATA_PIN;
				}
				// U11-3 BAND_OUT_1
				if (registerNumber == 44 && bitRead(band_out, 1)) {
					SET_DATA_PIN;
				}
				// U11-2 TUNE_OUT
				if (registerNumber == 45 && TRX_Tune) {
					SET_DATA_PIN;
				}
				// U11-1 ATT_ON_8
				if (registerNumber == 46 && !(TRX.ATT && att_val_8)) {
					SET_DATA_PIN;
				}
				// U11-0 ATT_ON_16
				if (registerNumber == 47 && !(TRX.ATT && att_val_16)) {
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

	// RU4PN Version RF Unit ///////////////////////////////////////////////////////////////////////
	if (CALIBRATE.RF_unit_type == RF_UNIT_RU4PN) {
		if (TRX_Tune && CurrentVFO->RXFreqAfterTransverters <= 70000000) {
			ATU_Process();
		}

		bool shift_array[48];
		static bool shift_array_old[48];

		shift_array[0] = TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 2);                                                            // U2-7 TUN_C_3
		shift_array[1] = CurrentVFO->RXFreqAfterTransverters >= 70000000;                                                       // U2-6 HF-VHF-SELECT
		shift_array[2] = TRX_on_TX && CurrentVFO->Mode != TRX_MODE_LOOPBACK && CurrentVFO->RXFreqAfterTransverters < 70000000;  // U2-5 HF_AMP_BIAS_ON
		shift_array[3] = TRX_on_TX && CurrentVFO->Mode != TRX_MODE_LOOPBACK && CurrentVFO->RXFreqAfterTransverters >= 70000000; // U2-4 VHF_AMP_BIAS_ON
		shift_array[4] = TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 0);                                                            // U2-3 TUN_C_1
		shift_array[5] = TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 0);                                                            // U2-2 TUN_I_1
		shift_array[6] = TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 1);                                                            // U2-1 TUN_C_2
		shift_array[7] = TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 1);                                                            // U2-0 TUN_I_2

		shift_array[8] = CurrentVFO->RXFreqAfterTransverters >= 70000000; // U3-7 HF-VHF-SELECT
		shift_array[9] = false;                                           // U3-6 NOT USED
		shift_array[10] = false;                                          // U3-5 NOT USED
		shift_array[11] = false;                                          // U3-4 NOT USED
		shift_array[12] = TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 2);     // U3-3 TUN_I_3
		shift_array[13] = TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 3);     // U3-2 TUN_C_4
		shift_array[14] = TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 3);     // U3-1 TUN_I_4
		shift_array[15] = TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 4);     // U3-0 TUN_C_5

		shift_array[16] = currentAnt == TRX_ANT_2; // U11-7 ANT1-2_OUT
		shift_array[17] = false;                   // U11-6 FAN_OUT
		static bool fan_pwm = false;
		if (FAN_Active && TRX_RF_Temperature <= CALIBRATE.FAN_MEDIUM_STOP) { // Temperature at which the fan stops
			FAN_Active = false;
		}
		if (TRX_RF_Temperature >= CALIBRATE.FAN_MEDIUM_START) // Temperature at which the fan starts at half power
		{
			FAN_Active = true;
			fan_pwm = true;
		}
		if (TRX_RF_Temperature >= CALIBRATE.FAN_FULL_START) { // Temperature at which the fan starts at full power
			fan_pwm = false;
		}

		if (FAN_Active != FAN_Active_old) {
			FAN_Active_old = FAN_Active;
			LCD_UpdateQuery.StatusInfoGUI = true;
		}

		if (FAN_Active) {
			if (fan_pwm) // PWM
			{
				uint8_t on_ticks = (CALIBRATE.FAN_Medium_speed / 10);
				uint8_t off_ticks = 10 - (CALIBRATE.FAN_Medium_speed / 10);
				if (on_ticks % 2 == 0 && off_ticks % 2 == 0) {
					on_ticks /= 2;
					off_ticks /= 2;
				}
				if (on_ticks == off_ticks) {
					on_ticks = 1;
					off_ticks = 1;
				}

				static bool pwm_status = false; // true - on false - off
				static uint8_t pwm_ticks = 0;
				pwm_ticks++;
				if (pwm_status) {
					shift_array[17] = true;
				}
				if ((pwm_status && pwm_ticks == on_ticks) || (!pwm_status && pwm_ticks == off_ticks)) {
					pwm_status = !pwm_status;
					pwm_ticks = 0;
				}
			} else {
				shift_array[17] = true;
			}
		}
		shift_array[18] = bitRead(band_out, 3);                               // U11-5 BAND_OUT_3
		shift_array[19] = bitRead(band_out, 1);                               // U11-4 BAND_OUT_1
		shift_array[20] = TRX_Tune;                                           // U11-3 TUNE_OUT
		shift_array[21] = bitRead(band_out, 2);                               // U11-2 BAND_OUT_2
		shift_array[22] = bitRead(band_out, 0);                               // U11-1 BAND_OUT_0
		shift_array[23] = TRX_on_TX && CurrentVFO->Mode != TRX_MODE_LOOPBACK; // U11-0 TX_PTT_OUT

		shift_array[24] = (TRX.RF_Filters && (CurrentVFO->RXFreqAfterTransverters <= CALIBRATE.RFU_LPF_END) && !dualrx_lpf_disabled); // U1-7 LPF_ON
		shift_array[25] = !(TRX.ATT && att_val_1);                                                                                    // U1-6 ATT_ON_1
		shift_array[26] = !(TRX.ATT && att_val_05);                                                                                   // U1-5 ATT_ON_0.5
		shift_array[27] = !(TRX.ATT && att_val_16);                                                                                   // U1-4 ATT_ON_16
		shift_array[28] = !(TRX.ATT && att_val_2);                                                                                    // U1-3 ATT_ON_2
		shift_array[29] = !(TRX.ATT && att_val_4);                                                                                    // U1-2 ATT_ON_4
		shift_array[30] = !(TRX.ATT && att_val_8);                                                                                    // U1-1 ATT_ON_8
		shift_array[31] = !(!TRX_on_TX && TRX.LNA);                                                                                   // U1-0 LNA_ON

		shift_array[32] = (turn_on_tx_lpf || TRX_on_TX) && CurrentVFO->RXFreqAfterTransverters > 31000000 && CurrentVFO->RXFreqAfterTransverters <= 60000000; // U7-7 LPF_7
		shift_array[33] = (turn_on_tx_lpf || TRX_on_TX) && CurrentVFO->RXFreqAfterTransverters > 22000000 && CurrentVFO->RXFreqAfterTransverters <= 31000000; // U7-6 LPF_6
		shift_array[34] = (turn_on_tx_lpf || TRX_on_TX) && CurrentVFO->RXFreqAfterTransverters > 15000000 && CurrentVFO->RXFreqAfterTransverters <= 22000000; // U7-5 LPF_5
		shift_array[35] = (turn_on_tx_lpf || TRX_on_TX) && CurrentVFO->RXFreqAfterTransverters > 7500000 && CurrentVFO->RXFreqAfterTransverters <= 15000000;  // U7-4 LPF_4
		shift_array[36] = TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 4);                                                                                         // U7-3 TUN_I_5
		shift_array[37] = (turn_on_tx_lpf || TRX_on_TX) && CurrentVFO->RXFreqAfterTransverters <= 2500000;                                                    // U7-2 LPF_1
		shift_array[38] = (turn_on_tx_lpf || TRX_on_TX) && CurrentVFO->RXFreqAfterTransverters > 2500000 && CurrentVFO->RXFreqAfterTransverters <= 4500000;   // U7-1 LPF_2
		shift_array[39] = (turn_on_tx_lpf || TRX_on_TX) && CurrentVFO->RXFreqAfterTransverters > 4500000 && CurrentVFO->RXFreqAfterTransverters <= 7500000;   // U7-0 LPF_3

		shift_array[40] = TRX.TUNER_Enabled && TRX.ATU_T;                                                              // U21-7 TUN_T
		shift_array[41] = !(!TRX.RF_Filters || dualrx_bpf_disabled || (bpf != 1 && bpf != 2 && bpf != 3 && bpf != 4)); // U21-6 BPF_2_!EN 1,2,3,4 - bpf2
		shift_array[42] = !(TRX.RF_Filters && !dualrx_bpf_disabled && (bpf == 4 || bpf == 2 || bpf == 0 || bpf == 6)); // U21-5 BPF_2_A1 4,2 - bpf2; 0,6 - bpf1
		shift_array[43] = !(TRX.RF_Filters && !dualrx_bpf_disabled && (bpf == 1 || bpf == 2 || bpf == 5 || bpf == 6)); // U21-4 BPF_2_A0 1,2 - bpf2; 5,6 - bpf1
		shift_array[44] = !(TRX.RF_Filters && !dualrx_bpf_disabled && (bpf == 1 || bpf == 2 || bpf == 5 || bpf == 6)); // U21-3 BPF_1_A0 1,2 - bpf2; 5,6 - bpf1
		shift_array[45] = !(TRX.RF_Filters && !dualrx_bpf_disabled && (bpf == 4 || bpf == 2 || bpf == 0 || bpf == 6)); // U21-2 BPF_1_A1 4,2 - bpf2; 0,6 - bpf1
		shift_array[46] = !(!TRX.RF_Filters || dualrx_bpf_disabled || (bpf != 0 && bpf != 5 && bpf != 6 && bpf != 7)); // U21-1 BPF_1_!EN 5,6,7,0 - bpf1
		shift_array[47] = !(TRX.RF_Filters && !dualrx_bpf_disabled && bpf != 255);                                     // U21-0 BPF_ON

		bool array_equal = true;
		for (uint8_t i = 0; i < 48; i++) {
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
		for (uint8_t registerNumber = 0; registerNumber < 48; registerNumber++) {
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
#endif
#if defined(FRONTPANEL_WF_100D)
	// WF-100D RF Unit ///////////////////////////////////////////////////////////////////////
	if (CALIBRATE.RF_unit_type == RF_UNIT_WF_100D) {
		if (TRX_Tune && CurrentVFO->RXFreqAfterTransverters <= 70000000) {
			ATU_Process();
		}

		uint8_t lpf_index = 7;                                // 6m
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
		if (CurrentVFO->RXFreqAfterTransverters > 22000000 && CurrentVFO->RXFreqAfterTransverters <= 30000000) { // 12m,CB,10m
			lpf_index = 6;
		}

		bool wf_100d_shift_array[56];
		static bool wf_100d_shift_array_old[56];

		wf_100d_shift_array[0] = TRX.TUNER_Enabled && (TRX.ATU_I > 0 || TRX.ATU_C > 0); // U1-7 TU_BY

		// U1-6 FAN_OUT
		wf_100d_shift_array[1] = false;
		static bool fan_pwm = false;
		if (FAN_Active && TRX_RF_Temperature <= CALIBRATE.FAN_MEDIUM_STOP) { // Temperature at which the fan stops
			FAN_Active = false;
		}
		if (TRX_RF_Temperature >= CALIBRATE.FAN_MEDIUM_START) // Temperature at which the fan starts at half power
		{
			FAN_Active = true;
			fan_pwm = true;
		}
		if (TRX_RF_Temperature >= CALIBRATE.FAN_FULL_START) { // Temperature at which the fan starts at full power
			fan_pwm = false;
		}

		if (FAN_Active != FAN_Active_old) {
			FAN_Active_old = FAN_Active;
			LCD_UpdateQuery.StatusInfoGUI = true;
		}

		if (FAN_Active) {
			if (fan_pwm) // PWM
			{
				uint8_t on_ticks = (CALIBRATE.FAN_Medium_speed / 10);
				uint8_t off_ticks = 10 - (CALIBRATE.FAN_Medium_speed / 10);
				if (on_ticks % 2 == 0 && off_ticks % 2 == 0) {
					on_ticks /= 2;
					off_ticks /= 2;
				}
				if (on_ticks == off_ticks) {
					on_ticks = 1;
					off_ticks = 1;
				}

				static bool pwm_status = false; // true - on false - off
				static uint8_t pwm_ticks = 0;
				pwm_ticks++;
				if (pwm_status) {
					wf_100d_shift_array[1] = true;
				}
				if ((pwm_status && pwm_ticks == on_ticks) || (!pwm_status && pwm_ticks == off_ticks)) {
					pwm_status = !pwm_status;
					pwm_ticks = 0;
				}
			} else {
				wf_100d_shift_array[1] = true;
			}
		}
		wf_100d_shift_array[2] = false;                                                                                                 // U1-5 -
		wf_100d_shift_array[3] = false;                                                                                                 // U1-4 -=
		wf_100d_shift_array[4] = TRX_on_TX && CurrentVFO->Mode != TRX_MODE_LOOPBACK;                                                    // U1-3 TX_PTT_OUT
		wf_100d_shift_array[5] = TRX_on_TX && CurrentVFO->Mode != TRX_MODE_LOOPBACK && CurrentVFO->RXFreqAfterTransverters < 70000000;  // U1-2 HF_AMP_BIAS_ON
		wf_100d_shift_array[6] = CurrentVFO->RXFreqAfterTransverters >= 70000000;                                                       // U1-1 HF-VHF-SELECT
		wf_100d_shift_array[7] = TRX_on_TX && CurrentVFO->Mode != TRX_MODE_LOOPBACK && CurrentVFO->RXFreqAfterTransverters >= 70000000; // U1-0 VHF_AMP_BIAS_ON

		wf_100d_shift_array[8] = TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 6);  // U2-7 TUN_I_7
		wf_100d_shift_array[9] = TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 5);  // U2-6 TUN_I_6
		wf_100d_shift_array[10] = TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 4); // U2-5 TUN_I_5
		wf_100d_shift_array[11] = currentAnt == TRX_ANT_1;                    // U2-4 ANT1-2_OUT
		wf_100d_shift_array[12] = TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 3); // U2-3 TUN_I_4
		wf_100d_shift_array[13] = TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 2); // U2-2 TUN_I_3
		wf_100d_shift_array[14] = TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 1); // U2-1 TUN_I_2
		wf_100d_shift_array[15] = TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 0); // U2-0 TUN_I_1

		wf_100d_shift_array[16] = TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 6); // U3-7 TUN_C_7
		wf_100d_shift_array[17] = TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 5); // U3-6 TUN_C_6
		wf_100d_shift_array[18] = TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 4); // U3-5 TUN_C_5
		wf_100d_shift_array[19] = TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 3); // U3-4 TUN_C_4
		wf_100d_shift_array[20] = TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 2); // U3-3 TUN_C_3
		wf_100d_shift_array[21] = TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 1); // U3-2 TUN_C_2
		wf_100d_shift_array[22] = TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 0); // U3-1 TUN_C_1
		wf_100d_shift_array[23] = TRX.TUNER_Enabled && TRX.ATU_T;             // U3-0 TUN_T

		wf_100d_shift_array[24] = false;                    // U23-7 -
		wf_100d_shift_array[25] = !(TRX.ATT && att_val_16); // U23-6 ATT_ON_16
		wf_100d_shift_array[26] = !(TRX.ATT && att_val_05); // U23-5 ATT_ON_0.5
		wf_100d_shift_array[27] = !(TRX.ATT && att_val_1);  // U23-4 ATT_ON_1
		wf_100d_shift_array[28] = !(TRX.ATT && att_val_2);  // U23-3 ATT_ON_2
		wf_100d_shift_array[29] = !(TRX.ATT && att_val_4);  // U23-2 ATT_ON_4
		wf_100d_shift_array[30] = !(TRX.ATT && att_val_8);  // U23-1 ATT_ON_8
		wf_100d_shift_array[31] = false;                    // U23-0 -

		wf_100d_shift_array[32] = (turn_on_tx_lpf || TRX_on_TX) && CurrentVFO->Mode != TRX_MODE_LOOPBACK && lpf_index == 5;             // U24-7 LPF_5
		wf_100d_shift_array[33] = (turn_on_tx_lpf || TRX_on_TX) && CurrentVFO->Mode != TRX_MODE_LOOPBACK && lpf_index == 6;             // U24-6 LPF_6
		wf_100d_shift_array[34] = (turn_on_tx_lpf || TRX_on_TX) && CurrentVFO->Mode != TRX_MODE_LOOPBACK && lpf_index == 7;             // U24-5 LPF_7
		wf_100d_shift_array[35] = TRX_on_TX && CurrentVFO->Mode != TRX_MODE_LOOPBACK && CurrentVFO->RXFreqAfterTransverters < 70000000; // U24-4 HF_AMP_BIAS_ON
		wf_100d_shift_array[36] = (turn_on_tx_lpf || TRX_on_TX) && CurrentVFO->Mode != TRX_MODE_LOOPBACK && lpf_index == 1;             // U24-3 LPF_1
		wf_100d_shift_array[37] = (turn_on_tx_lpf || TRX_on_TX) && CurrentVFO->Mode != TRX_MODE_LOOPBACK && lpf_index == 2;             // U24-2 LPF_2
		wf_100d_shift_array[38] = (turn_on_tx_lpf || TRX_on_TX) && CurrentVFO->Mode != TRX_MODE_LOOPBACK && lpf_index == 3;             // U24-1 LPF_3
		wf_100d_shift_array[39] = (turn_on_tx_lpf || TRX_on_TX) && CurrentVFO->Mode != TRX_MODE_LOOPBACK && lpf_index == 4;             // U24-0 LPF_4

		wf_100d_shift_array[40] =
		    !dualrx_bpf_disabled && TRX.RF_Filters && ((CurrentVFO->RXFreqAfterTransverters >= CALIBRATE.RFU_HPF_START && bpf == 255) || bpf == 7); // U31-H U3 BPF_1_A1 hpf 145(7)
		wf_100d_shift_array[41] = !dualrx_bpf_disabled && TRX.RF_Filters && (bpf == 1 || bpf == 0);                                                 // U31-G U7 BPF_3_A1 2.5-4(1) 1.6-2.5(0)
		wf_100d_shift_array[42] = !dualrx_bpf_disabled && TRX.RF_Filters && (bpf == 3 || bpf == 0);                                                 // U31-F U7 BPF_3_A0 7-12(3) 1.6-2.5(0)
		wf_100d_shift_array[43] =
		    !dualrx_bpf_disabled && TRX.RF_Filters && ((CurrentVFO->RXFreqAfterTransverters <= CALIBRATE.RFU_LPF_END && bpf == 255) || bpf == 7); // U31-E U3 BPF_1_A0 lpf 145(7)
		wf_100d_shift_array[44] = !dualrx_bpf_disabled && TRX.RF_Filters && (bpf == 6 || bpf == 4);                                               // U31-D U5 BPF_2_A0 21.5-30(6) 12-14.5(4)
		wf_100d_shift_array[45] = !dualrx_bpf_disabled && TRX.RF_Filters && (bpf == 5 || bpf == 4);                                               // U31-C U5 BPF_2_A1 14.5-21.5(5) 12-14.5(4)
		wf_100d_shift_array[46] = !(TRX.RF_Filters && !dualrx_bpf_disabled && (bpf == 5 || bpf == 6 || bpf == 4));             // U31-B U5 BPF_2_EN net5?, 21.5-30(6), 14.5-21.5(5), 12-14.5(4)
		wf_100d_shift_array[47] = !(TRX.RF_Filters && !dualrx_bpf_disabled && (bpf == 1 || bpf == 3 || bpf == 0 || bpf == 2)); // U31-A U7 BPF_3_EN 6-7.3(2), 7-12(3), 2.5-4(1), 1.6-2.5(0);

		wf_100d_shift_array[48] = bitRead(band_out, 0);                               // U32-H BAND_OUT_0
		wf_100d_shift_array[49] = bitRead(band_out, 1);                               // U32-G BAND_OUT_1
		wf_100d_shift_array[50] = bitRead(band_out, 2);                               // U32-F BAND_OUT_2
		wf_100d_shift_array[51] = bitRead(band_out, 3);                               // U32-E BAND_OUT_3
		wf_100d_shift_array[52] = TRX_on_TX && CurrentVFO->Mode != TRX_MODE_LOOPBACK; // U32-D Net_RX/TX
		wf_100d_shift_array[53] = !TRX_on_TX && TRX.LNA;                              // U32-C LNA_ON
		wf_100d_shift_array[54] = TRX_Tune;                                           // U32-B TRX_Tune
		wf_100d_shift_array[55] =
		    !(TRX.RF_Filters && ((CurrentVFO->RXFreqAfterTransverters >= CALIBRATE.RFU_HPF_START && bpf == 255) || (CurrentVFO->RXFreqAfterTransverters <= CALIBRATE.RFU_LPF_END && bpf == 255) ||
		                         bpf == 7 || dualrx_bpf_disabled)); // U32-A U3 BPF_1_EN bypass, lpf, hpf, 145(7)

		bool array_equal = true;
		for (uint8_t i = 0; i < 56; i++) {
			if (wf_100d_shift_array[i] != wf_100d_shift_array_old[i]) {
				wf_100d_shift_array_old[i] = wf_100d_shift_array[i];
				array_equal = false;
			}
		}
		if (array_equal && !clean) {
			return;
		}

		HAL_GPIO_WritePin(RFUNIT_RCLK_GPIO_Port, RFUNIT_RCLK_Pin, GPIO_PIN_RESET); // latch
		MINI_DELAY
		for (uint8_t registerNumber = 0; registerNumber < 56; registerNumber++) {
			HAL_GPIO_WritePin(RFUNIT_CLK_GPIO_Port, RFUNIT_CLK_Pin, GPIO_PIN_RESET); // data block
			MINI_DELAY
			HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_RESET); // data
			MINI_DELAY
			if (!clean) {
				if (wf_100d_shift_array[registerNumber]) {
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
#endif
#if defined(FRONTPANEL_KT_100S)
	// KT-100S RF Unit ///////////////////////////////////////////////////////////////////////
	if (CALIBRATE.RF_unit_type == RF_UNIT_KT_100S) {
		if (TRX_Tune && CurrentVFO->RXFreqAfterTransverters <= 70000000) {
			ATU_Process();
		}

		bool shift_array[56];
		static bool shift_array_old[56];

		shift_array[0] = false;                   // 7-7 UNUSED
		shift_array[1] = false;                   // 7-6 UNUSED
		shift_array[2] = false;                   // 7-5 UNUSED
		shift_array[3] = false;                   // 7-4 UNUSED
		shift_array[4] = currentAnt == TRX_ANT_4; // 7-3 ANT4
		shift_array[5] = currentAnt == TRX_ANT_3; // 7-2 ANT3
		shift_array[6] = currentAnt == TRX_ANT_2; // 7-1 ANT2
		shift_array[7] = currentAnt == TRX_ANT_1; // 7-0 ANT1

		shift_array[8] = false;                                                  // 6-7 UNUSED
		shift_array[9] = false;                                                  // 6-6 UNUSED
		shift_array[10] = TRX.TUNER_Enabled && (TRX.ATU_I > 0 || TRX.ATU_C > 0); // 6-5 TUN-BY
		shift_array[11] = TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 0);            // 6-4 TunC1
		shift_array[12] = TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 1);            // 6-3 TunC2
		shift_array[13] = TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 2);            // 6-2 TunC3
		shift_array[14] = TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 3);            // 6-1 TunC4
		shift_array[15] = TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 4);            // 6-0 TunC5

		shift_array[16] = CurrentVFO->RXFreqAfterTransverters >= 70000000;    // 5-7 HF-VHF-SELECT
		shift_array[17] = TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 4);         // 5-6 TunL5
		shift_array[18] = TRX.TUNER_Enabled && TRX.ATU_T;                     // 5-5 TunT1
		shift_array[19] = TRX_on_TX && CurrentVFO->Mode != TRX_MODE_LOOPBACK; // 5-4 RX-TX
		shift_array[20] = TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 3);         // 5-3 TunL4
		shift_array[21] = TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 2);         // 5-2 TunL3
		shift_array[22] = TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 1);         // 5-1 TunL2
		shift_array[23] = TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 0);         // 5-0 TunL1

		shift_array[24] = (turn_on_tx_lpf || TRX_on_TX) && CurrentVFO->RXFreqAfterTransverters > 31000000 && CurrentVFO->RXFreqAfterTransverters <= 60000000; // 4-7 LPF 6M
		shift_array[25] = (turn_on_tx_lpf || TRX_on_TX) && CurrentVFO->RXFreqAfterTransverters > 22000000 && CurrentVFO->RXFreqAfterTransverters <= 31000000; // 4-6 LPF 10M
		shift_array[26] = TRX_on_TX && CurrentVFO->Mode != TRX_MODE_LOOPBACK && CurrentVFO->RXFreqAfterTransverters < 70000000;                               // 4-5 HF BIAS
		shift_array[27] = (turn_on_tx_lpf || TRX_on_TX) && CurrentVFO->RXFreqAfterTransverters <= 2500000;                                                    // 4-4 LPF 160M
		shift_array[28] = (turn_on_tx_lpf || TRX_on_TX) && CurrentVFO->RXFreqAfterTransverters > 2500000 && CurrentVFO->RXFreqAfterTransverters <= 4500000;   // 4-3 LPF 80M
		shift_array[29] = (turn_on_tx_lpf || TRX_on_TX) && CurrentVFO->RXFreqAfterTransverters > 4500000 && CurrentVFO->RXFreqAfterTransverters <= 7500000;   // 4-2 LPF 40M
		shift_array[30] = (turn_on_tx_lpf || TRX_on_TX) && CurrentVFO->RXFreqAfterTransverters > 7500000 && CurrentVFO->RXFreqAfterTransverters <= 15000000;  // 4-1 LPF 20M
		shift_array[31] = (turn_on_tx_lpf || TRX_on_TX) && CurrentVFO->RXFreqAfterTransverters > 15000000 && CurrentVFO->RXFreqAfterTransverters <= 22000000; // 4-0 LPF 15M

		shift_array[32] = false;                                              // 3-7 UNUSED
		shift_array[33] = false;                                              // 3-6 UNUSED
		shift_array[34] = bitRead(band_out, 3);                               // 3-5 EXT BAND OUT 3
		shift_array[35] = bitRead(band_out, 1);                               // 3-4 EXT BAND OUT 1
		shift_array[36] = TRX_Tune;                                           // 3-3 EXT TUNE OUT
		shift_array[37] = bitRead(band_out, 2);                               // 3-2 EXT BAND OUT 2
		shift_array[38] = bitRead(band_out, 0);                               // 3-1 EXT BAND OUT 0
		shift_array[39] = TRX_on_TX && CurrentVFO->Mode != TRX_MODE_LOOPBACK; // 3-0 EXT TX PTT OUT

		shift_array[40] = CurrentVFO->RXFreqAfterTransverters >= 70000000;                        // 2-7 HF-VHF-SELECT
		shift_array[41] = false;                                                                  // 2-6 UNUSED
		shift_array[42] = false;                                                                  // 2-5 UNUSED
		shift_array[43] = false;                                                                  // 2-4 UNUSED
		shift_array[44] = false;                                                                  // 2-3 UNUSED
		shift_array[45] = (TRX.ATT && TRX.ATT_DB > 6);                                            // 2-2 ATT_ON_12
		shift_array[46] = (TRX.ATT && ((TRX.ATT_DB > 0 && TRX.ATT_DB <= 6) || TRX.ATT_DB >= 18)); // 2-1 ATT_ON_6
		shift_array[47] = !(!TRX_on_TX && TRX.LNA);                                               // 2-0 LNA_ON

		shift_array[48] = CurrentVFO->RXFreqAfterTransverters >= 70000000;                                                                      // 1-7 HF-VHF-SELECT
		shift_array[49] = TRX.RF_Filters && CurrentVFO->RXFreqAfterTransverters >= 23000000 && CurrentVFO->RXFreqAfterTransverters <= 60000000; // 1-6 BPF - 21-60 MHz
		shift_array[50] = TRX.RF_Filters && CurrentVFO->RXFreqAfterTransverters >= 13000000 && CurrentVFO->RXFreqAfterTransverters < 23000000;  // 1-5 BPF - 11-25 MHz
		shift_array[51] = TRX.RF_Filters && CurrentVFO->RXFreqAfterTransverters >= 8000000 && CurrentVFO->RXFreqAfterTransverters < 13000000;   // 1-4 BPF - 6,5-15 MHz
		shift_array[52] = TRX.RF_Filters && CurrentVFO->RXFreqAfterTransverters >= 5000000 && CurrentVFO->RXFreqAfterTransverters < 8000000;    // 1-3 BPF - 4,2-9,0 MHz
		shift_array[53] = TRX.RF_Filters && CurrentVFO->RXFreqAfterTransverters >= 2500000 && CurrentVFO->RXFreqAfterTransverters < 5000000;    // 1-2 BPF - 2,3-5,2 MHz
		shift_array[54] = TRX.RF_Filters && CurrentVFO->RXFreqAfterTransverters >= 1500000 && CurrentVFO->RXFreqAfterTransverters < 2500000;    // 1-1 BPF - 1,4-2,8 MHz
		shift_array[55] = TRX.RF_Filters && CurrentVFO->RXFreqAfterTransverters < 1500000;                                                      // 1-0 BPF - 0,0-2,2 MHz

		bool array_equal = true;
		for (uint8_t i = 0; i < 56; i++) {
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
		for (uint8_t registerNumber = 0; registerNumber < 56; registerNumber++) {
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
#endif
}

void RF_UNIT_ProcessSensors(void) {
#define B16_RANGE 65535.0f
#define B14_RANGE 16383.0f

	HAL_ADCEx_InjectedPollForConversion(&hadc1, 100); // wait if prev conversion not ended

	// THERMAL
	float32_t rf_thermal = (float32_t)(HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_3)) * TRX_STM32_VREF / B16_RANGE;

	float32_t therm_resistance = (-(float32_t)CALIBRATE.KTY81_Calibration) * rf_thermal / (-3.3f + rf_thermal);
	uint_fast8_t point_left = 0;
	uint_fast8_t point_right = SENS_TABLE_COUNT - 1;
	for (uint_fast8_t i = 0; i < SENS_TABLE_COUNT; i++) {
		if (KTY81_120_sensTable[i][1] < therm_resistance) {
			point_left = i;
		}
	}
	for (uint_fast8_t i = (SENS_TABLE_COUNT - 1); i > 0; i--) {
		if (KTY81_120_sensTable[i][1] >= therm_resistance) {
			point_right = i;
		}
	}
	float32_t power_left = (float32_t)KTY81_120_sensTable[point_left][0];
	float32_t power_right = (float32_t)KTY81_120_sensTable[point_right][0];
	float32_t part_point_left = therm_resistance - KTY81_120_sensTable[point_left][1];
	float32_t part_point_right = KTY81_120_sensTable[point_right][1] - therm_resistance;
	float32_t part_point = part_point_left / (part_point_left + part_point_right);
	float32_t TRX_RF_Temperature_measured = (power_left * (1.0f - part_point)) + (power_right * (part_point));

	if (TRX_RF_Temperature_measured < -100.0f) {
		TRX_RF_Temperature_measured = 75.0f;
	}
	if (TRX_RF_Temperature_measured < 0.0f) {
		TRX_RF_Temperature_measured = 0.0f;
	}

	static float32_t TRX_RF_Temperature_averaged = 20.0f;
	TRX_RF_Temperature_averaged = TRX_RF_Temperature_averaged * 0.995f + TRX_RF_Temperature_measured * 0.005f;

	if (fabsf(TRX_RF_Temperature_averaged - TRX_RF_Temperature) >= 1.0f) { // hysteresis
		TRX_RF_Temperature = TRX_RF_Temperature_averaged;
	}

	// VBAT
	TRX_VBAT_Voltage = (float32_t)(HAL_ADCEx_InjectedGetValue(&hadc3, ADC_INJECTED_RANK_3)) * TRX_STM32_VREF / B14_RANGE;

	// ALC
	float32_t ALC_IN = (float32_t)HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_4) * TRX_STM32_VREF / B16_RANGE;
	TRX_ALC_IN = TRX_ALC_IN * 0.9f + ALC_IN * 0.1f;

	// SWR
	float32_t forward = (float32_t)(HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_2)) * TRX_STM32_VREF / B16_RANGE;
	float32_t backward = (float32_t)(HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_1)) * TRX_STM32_VREF / B16_RANGE;

	// println("ALC: ", (double)TRX_ALC_IN, "FWD: ", (double)forward, " BKW: ", (double)backward, " THERM: ", (double)rf_thermal, " RESIST: ", (double)therm_resistance);
	if (ALC_IN > 3.2f || forward > 3.2f || backward > 3.2f) {
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
		if (CurrentVFO->RXFreqAfterTransverters >= 80000000) {
			forward = forward * CALIBRATE.SWR_FWD_Calibration_VHF;
		} else if (CurrentVFO->RXFreqAfterTransverters >= 40000000) {
			forward = forward * CALIBRATE.SWR_FWD_Calibration_6M;
		} else {
			forward = forward * CALIBRATE.SWR_FWD_Calibration_HF;
		}

		forward += 0.21f; // drop on diode

		// backward = backward / (510.0f / (0.0f + 510.0f)); // adjust the voltage based on the voltage divider (0 ohm and 510 ohm)
		if (backward >= 0.05f) // do not measure less than 50mV
		{
			// Transformation ratio of the SWR meter
			if (CurrentVFO->RXFreqAfterTransverters >= 80000000) {
				backward = backward * CALIBRATE.SWR_BWD_Calibration_VHF;
			} else if (CurrentVFO->RXFreqAfterTransverters >= 40000000) {
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

	HAL_ADCEx_InjectedStart(&hadc1); // start next manual conversion
}

// Tisho
// used to controll the calibration of the FW and BW power measurments
void RF_UNIT_MeasureVoltage(void) {
#define B16_RANGE 65535.0f
	// #define B14_RANGE 16383.0f

	float32_t forward = (float32_t)(HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_2)) * TRX_STM32_VREF / B16_RANGE;
	float32_t backward = (float32_t)(HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_1)) * TRX_STM32_VREF / B16_RANGE;
	// use the TRX_VLT_forward and TRX_VLT_backward global variables
	// for the raw ADC input voltages
	// in the TDM_Voltages() the other stuff will be calculated localy

	static float32_t VLT_forward = 0.0f;
	static float32_t VLT_backward = 0.0f;
	VLT_forward = VLT_forward + (forward - VLT_forward) / 10;
	VLT_backward = VLT_backward + (backward - VLT_backward) / 10;

	TRX_VLT_forward = VLT_forward;
	TRX_VLT_backward = VLT_backward;
}
