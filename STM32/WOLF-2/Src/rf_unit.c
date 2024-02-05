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

	return 255;
}

void RF_UNIT_UpdateState(bool clean) // pass values to RF-UNIT
{
	// STM32 PTT_OUT
	HAL_GPIO_WritePin(PTT_OUT_GPIO_Port, PTT_OUT_Pin, TRX_on_TX ? GPIO_PIN_SET : GPIO_PIN_RESET);

	// Attenuator
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

	// Transverters / External port
	uint8_t band_out = 0;
	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);

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

	// ATU
	if (TRX_Tune && CurrentVFO->RXFreqAfterTransverters <= 70000000) {
		ATU_Process();
	}

	// TX LPF
	// bool turn_on_tx_lpf = true;
	// if (((HAL_GetTick() - TRX_TX_EndTime) > TX_LPF_TIMEOUT || TRX_TX_EndTime == 0) && !TRX_on_TX) {
	// 	turn_on_tx_lpf = false;
	// }

	bool tx_lpf_0 = true;
	bool tx_lpf_1 = true;
	bool tx_lpf_2 = true;
	bool tx_lpf_3 = true;
	if (CurrentVFO->RXFreqAfterTransverters <= 2000000) { // 160m
		tx_lpf_0 = false;
		tx_lpf_1 = true;
		tx_lpf_2 = true;
		tx_lpf_3 = false;
	}
	if (CurrentVFO->RXFreqAfterTransverters > 2000000 && CurrentVFO->RXFreqAfterTransverters <= 5000000) { // 80m
		tx_lpf_0 = true;
		tx_lpf_1 = false;
		tx_lpf_2 = true;
		tx_lpf_3 = false;
	}
	if (CurrentVFO->RXFreqAfterTransverters > 5000000 && CurrentVFO->RXFreqAfterTransverters <= 9000000) { // 40m
		tx_lpf_0 = false;
		tx_lpf_1 = false;
		tx_lpf_2 = false;
		tx_lpf_3 = false;
	}
	if (CurrentVFO->RXFreqAfterTransverters > 9000000 && CurrentVFO->RXFreqAfterTransverters <= 16000000) { // 30m,20m
		tx_lpf_0 = true;
		tx_lpf_1 = false;
		tx_lpf_2 = false;
		tx_lpf_3 = false;
	}
	if (CurrentVFO->RXFreqAfterTransverters > 16000000 && CurrentVFO->RXFreqAfterTransverters <= 22000000) { // 17m,15m
		tx_lpf_0 = false;
		tx_lpf_1 = true;
		tx_lpf_2 = false;
		tx_lpf_3 = false;
	}
	if (CurrentVFO->RXFreqAfterTransverters > 22000000 && CurrentVFO->RXFreqAfterTransverters <= 30000000) { // 12m,CB,10m
		tx_lpf_0 = true;
		tx_lpf_1 = true;
		tx_lpf_2 = false;
		tx_lpf_3 = false;
	}
	if (CurrentVFO->RXFreqAfterTransverters > 30000000 && CurrentVFO->RXFreqAfterTransverters < 60000000) { // 6m+
		tx_lpf_0 = false;
		tx_lpf_1 = false;
		tx_lpf_2 = true;
		tx_lpf_3 = false;
	}

	// Dual RX
	uint8_t bpf = getBPFByFreq(CurrentVFO->RXFreqAfterTransverters);
	uint8_t second_bpf = getBPFByFreq(SecondaryVFO->RXFreqAfterTransverters);
	bool dualrx_bpf_disabled = TRX.Dual_RX && bpf != second_bpf;

	// BPF/LPF
	bool BPF1_EN = false;
	bool BPF1_A1 = false;
	bool BPF1_A0 = false;
	bool BPF2_EN = false;
	bool BPF2_A1 = false;
	bool BPF2_A0 = false;

	if (CurrentVFO->RXFreqAfterTransverters >= 60000000) { // VHF, disabled bpf
		BPF1_EN = true;
		BPF1_A1 = false;
		BPF1_A0 = false;
		BPF2_EN = true;
		BPF2_A1 = false;
		BPF2_A0 = false;
	} else if (!TRX.RF_Filters || dualrx_bpf_disabled || bpf == 255) {
		BPF1_EN = false;
		BPF1_A1 = false;
		BPF1_A0 = false;
		BPF2_EN = true;
		BPF2_A1 = false;
		BPF2_A0 = false;
	} else if (bpf == 0) { // 160
		BPF1_EN = true;
		BPF1_A1 = false;
		BPF1_A0 = false;
		BPF2_EN = false;
		BPF2_A1 = true;
		BPF2_A0 = true;
	} else if (bpf == 1) { // 80
		BPF1_EN = true;
		BPF1_A1 = false;
		BPF1_A0 = false;
		BPF2_EN = false;
		BPF2_A1 = true;
		BPF2_A0 = false;
	} else if (bpf == 2) { // 40
		BPF1_EN = true;
		BPF1_A1 = false;
		BPF1_A0 = false;
		BPF2_EN = false;
		BPF2_A1 = false;
		BPF2_A0 = true;
	} else if (bpf == 3) { // 30
		BPF1_EN = true;
		BPF1_A1 = false;
		BPF1_A0 = false;
		BPF2_EN = false;
		BPF2_A1 = false;
		BPF2_A0 = false;
	} else if (bpf == 4) { // 20
		BPF1_EN = false;
		BPF1_A1 = true;
		BPF1_A0 = true;
		BPF2_EN = true;
		BPF2_A1 = false;
		BPF2_A0 = false;
	} else if (bpf == 5) { // 17,15
		BPF1_EN = false;
		BPF1_A1 = true;
		BPF1_A0 = false;
		BPF2_EN = true;
		BPF2_A1 = false;
		BPF2_A0 = false;
	} else if (bpf == 6) { // 12,10
		BPF1_EN = false;
		BPF1_A1 = false;
		BPF1_A0 = true;
		BPF2_EN = true;
		BPF2_A1 = false;
		BPF2_A0 = false;
	}

	uint8_t currentAnt = TRX_on_TX ? TRX.ANT_TX : TRX.ANT_RX;

	// VHF_RX
	bool wideband = CurrentVFO->RXFreqAfterTransverters > 60000000 && band != BANDID_FM && band != BANDID_2m && band != BANDID_70cm && band != BANDID_23cm;
	bool VHF_RXA = band == BANDID_70cm || band == BANDID_23cm || band == BANDID_FM;
	bool VHF_RXB = band == BANDID_2m || band == BANDID_23cm;
	bool VHF_RXC = band == BANDID_FM || wideband;

	// Tuner
	bool TUNER_Enabled = TRX.TUNER_Enabled && CurrentVFO->RXFreqAfterTransverters < 60000000;

	// Shift array
#define SHIFT_ARRAY_SIZE 64
	bool shift_array[SHIFT_ARRAY_SIZE];
	static bool shift_array_old[SHIFT_ARRAY_SIZE];

	shift_array[63] = !(TRX.ATT && att_val_4);          // U19 STPIC6C595TTR	VHF_ATT4
	shift_array[62] = TRX_on_TX && band == BANDID_23cm; // U19	VHF_TX_1.2G
	shift_array[61] = VHF_RXC;                          // U19	VHF_RXC
	shift_array[60] = !TRX_on_TX && TRX.LNA;            // U19	LNA
	shift_array[59] = TRX_on_TX && band == BANDID_2m;   // U19	VHF_TX_144
	shift_array[58] = TRX_on_TX && band == BANDID_70cm; // U19	VHF_TX_430
	shift_array[57] = VHF_RXA;                          // U19	VHF_RXA
	shift_array[56] = VHF_RXB;                          // U19	VHF_RXB

	shift_array[55] = !tx_lpf_0;                                                                                            // U20 STPIC6C595TTR	HFAMP_B0
	shift_array[54] = !tx_lpf_1;                                                                                            // U20	HFAMP_B1
	shift_array[53] = !tx_lpf_2;                                                                                            // U20	HFAMP_B2
	shift_array[52] = !tx_lpf_3;                                                                                            // U20	HFAMP_B3
	shift_array[51] = TRX_on_TX && CurrentVFO->Mode != TRX_MODE_LOOPBACK && CurrentVFO->RXFreqAfterTransverters < 60000000; // U20	HF TX
	shift_array[50] = !(TRX.ATT && att_val_8);                                                                              // U20	VHF_ATT8
	shift_array[49] = !(TRX.ATT && att_val_16);                                                                             // U20	VHF_ATT16
	shift_array[48] = false;                                                                                                // U20	FAN (code in bottom)

	shift_array[47] = currentAnt == TRX_ANT_2;    // U21 STPIC6C595TTR	ANT
	shift_array[46] = TUNER_Enabled && TRX.ATU_T; // U21	TUNE T
	shift_array[45] = false;                      // U21	Reserved
	shift_array[44] = false;                      // U21	Reserved
	shift_array[43] = false;                      // U21	Reserved
	shift_array[42] = false;                      // U21	Reserved
	shift_array[41] = false;                      // U21	Reserved
	shift_array[40] = false;                      // U21	Reserved

	shift_array[39] = false;                                              // U23 74HC595	Reserved
	shift_array[38] = bitRead(band_out, 0);                               // U23	EXT BAND0
	shift_array[37] = bitRead(band_out, 1);                               // U23	EXT BAND1
	shift_array[36] = bitRead(band_out, 2);                               // U23	EXT BAND2
	shift_array[35] = bitRead(band_out, 3);                               // U23	EXT BAND3
	shift_array[34] = TRX_on_TX && CurrentVFO->Mode != TRX_MODE_LOOPBACK; // U23	EXT TX
	shift_array[33] = TRX_Tune;                                           // U23	EXT TUNE
	shift_array[32] = false;                                              // U23	EXT Reserved

	shift_array[31] = false;                                           // U24 74HC595	Reserved
	shift_array[30] = CurrentVFO->RXFreqAfterTransverters >= 60000000; // U24	HF/VHF
	shift_array[29] = false;                                           // U24	Reserved
	shift_array[28] = false;                                           // U24	Reserved
	shift_array[27] = TRX.ATT && att_val_16;                           // U24	ATT 16
	shift_array[26] = TRX.ATT && att_val_05;                           // U24	ATT 0.5
	shift_array[25] = TRX.ATT && att_val_1;                            // U24	ATT 1
	shift_array[24] = TRX.ATT && att_val_2;                            // U24	ATT 2

	shift_array[23] = TRX.ATT && att_val_4; // U22 74HC595	ATT 4
	shift_array[22] = TRX.ATT && att_val_8; // U22	ATT 8
	shift_array[21] = BPF2_EN;              // U22	BPF2_EN
	shift_array[20] = BPF2_A1;              // U22	BPF2_A1
	shift_array[19] = BPF2_A0;              // U22	BPF2_A0
	shift_array[18] = BPF1_EN;              // U22	BPF1_EN
	shift_array[17] = BPF1_A1;              // U22	BPF1_A1
	shift_array[16] = BPF1_A0;              // U22	BPF1_A0

	shift_array[15] = TUNER_Enabled && bitRead(TRX.ATU_I, 3); // Tuner_U1 TUN_L4
	shift_array[14] = TUNER_Enabled && bitRead(TRX.ATU_I, 4); // Tuner_U1	TUN_L5
	shift_array[13] = TUNER_Enabled && bitRead(TRX.ATU_I, 5); // Tuner_U1	TUN_L6
	shift_array[12] = TUNER_Enabled && bitRead(TRX.ATU_I, 6); // Tuner_U1	TUN_L7
	shift_array[11] = false;                                  // Tuner_U1	Reserved
	shift_array[10] = TUNER_Enabled && bitRead(TRX.ATU_I, 2); // Tuner_U1	TUN_L3
	shift_array[9] = TUNER_Enabled && bitRead(TRX.ATU_I, 1);  // Tuner_U1	TUN_L2
	shift_array[8] = TUNER_Enabled && bitRead(TRX.ATU_I, 0);  // Tuner_U1	TUN_L1

	shift_array[7] = TUNER_Enabled && bitRead(TRX.ATU_C, 3); // Tuner_U2 TUN_С4
	shift_array[6] = TUNER_Enabled && bitRead(TRX.ATU_C, 2); // Tuner_U2	TUN_С3
	shift_array[5] = TUNER_Enabled && bitRead(TRX.ATU_C, 1); // Tuner_U2	TUN_С2
	shift_array[4] = TUNER_Enabled && bitRead(TRX.ATU_C, 0); // Tuner_U2	TUN_С1
	shift_array[3] = TUNER_Enabled && bitRead(TRX.ATU_C, 4); // Tuner_U2	TUN_С5
	shift_array[2] = TUNER_Enabled && bitRead(TRX.ATU_C, 5); // Tuner_U2	TUN_С6
	shift_array[1] = TUNER_Enabled && bitRead(TRX.ATU_C, 6); // Tuner_U2	TUN_С7
	shift_array[0] = false;                                  // Tuner_U2	Reserved

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
				shift_array[48] = true;
			}
			if ((pwm_status && pwm_ticks == on_ticks) || (!pwm_status && pwm_ticks == off_ticks)) {
				pwm_status = !pwm_status;
				pwm_ticks = 0;
			}
		} else {
			shift_array[48] = true;
		}
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

	// println("ALC: ", TRX_ALC_IN, "FWD: ", forward, " BKW: ", backward);
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
