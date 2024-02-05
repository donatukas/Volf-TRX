#include "fpga.h"
#include "lcd.h"
#include "main.h"
#include "trx_manager.h"

// Public variables
volatile uint32_t FPGA_samples = 0;                 // counter of the number of samples when exchanging with FPGA
volatile bool FPGA_NeedSendParams = false;          // flag of the need to send parameters to FPGA
volatile bool FPGA_NeedGetParams = false;           // flag of the need to get parameters from FPGA
volatile bool FPGA_NeedRestart_RX = false;          // flag of necessity to restart FPGA modules
volatile bool FPGA_NeedRestart_TX = false;          // flag of necessity to restart FPGA modules
volatile bool FPGA_Buffer_underrun = false;         // flag of lack of data from FPGA
static uint_fast16_t FPGA_Audio_RXBuffer_Index = 0; // current index in FPGA buffers
static uint_fast16_t FPGA_Audio_TXBuffer_Index = 0; // current index in FPGA buffers
bool FPGA_Audio_Buffer_State = true;                // buffer state, half or full full true - compleate; false - half
bool FPGA_RX_Buffer_Current = true;                 // buffer state, false - fill B, work A
bool FPGA_RX_buffer_ready = true;
volatile float32_t FPGA_Audio_Buffer_RX1_Q_A[FPGA_RX_IQ_BUFFER_HALF_SIZE] = {0}; // FPGA buffers
volatile float32_t FPGA_Audio_Buffer_RX1_I_A[FPGA_RX_IQ_BUFFER_HALF_SIZE] = {0};
volatile float32_t FPGA_Audio_Buffer_RX1_Q_B[FPGA_RX_IQ_BUFFER_HALF_SIZE] = {0}; // FPGA buffers
volatile float32_t FPGA_Audio_Buffer_RX1_I_B[FPGA_RX_IQ_BUFFER_HALF_SIZE] = {0};
SRAM_ON_F407 volatile float32_t FPGA_Audio_SendBuffer_Q[FPGA_TX_IQ_BUFFER_SIZE] = {0};
SRAM_ON_F407 volatile float32_t FPGA_Audio_SendBuffer_I[FPGA_TX_IQ_BUFFER_SIZE] = {0};
uint16_t FPGA_FW_Version[3] = {0};
uint8_t ADCDAC_OVR_StatusLatency = 0;
bool FPGA_bus_stop = true;                 // suspend the FPGA bus
volatile bool FPGA_bus_test_result = true; // self-test flag
int16_t ADC_RAW_IN = 0;
float32_t *FFTInput_I_current = (float32_t *)&FFTInput_I_A[0];
float32_t *FFTInput_Q_current = (float32_t *)&FFTInput_Q_A[0];

// Private variables
static GPIO_InitTypeDef FPGA_GPIO_InitStruct; // structure of GPIO ports

// Prototypes
static inline void FPGA_clockFall(void);            // remove CLK signal
static inline void FPGA_clockRise(void);            // raise the CLK signal
static inline void FPGA_clockPulse(void);           // raise and fall the CLK signal
static inline void FPGA_syncAndClockRiseFall(void); // raise CLK and SYNC signals, then release
static void FPGA_fpgadata_sendiq(bool clean);       // send IQ data
static void FPGA_fpgadata_getiq(void);              // get IQ data
static void FPGA_fpgadata_getparam(void);           // get parameters
static void FPGA_fpgadata_sendparam(void);          // send parameters
static void FPGA_setBusInput(void);                 // switch the bus to input
static void FPGA_setBusOutput(void);                // switch bus to pin
static void FPGA_restart_RX(void);                  // restart FPGA modules
static void FPGA_restart_TX(void);                  // restart FPGA modules

// initialize exchange with FPGA
void FPGA_Init(bool bus_test, bool firmware_test) {
	FPGA_bus_stop = true;

	HAL_GPIO_WritePin(FPGA_BUS_D0_GPIO_Port, FPGA_BUS_D0_Pin | FPGA_BUS_D1_Pin | FPGA_BUS_D2_Pin | FPGA_BUS_D3_Pin | FPGA_BUS_D4_Pin | FPGA_BUS_D5_Pin | FPGA_BUS_D6_Pin | FPGA_BUS_D7_Pin,
	                  GPIO_PIN_RESET);
	FPGA_GPIO_InitStruct.Pin = FPGA_BUS_D0_Pin | FPGA_BUS_D1_Pin | FPGA_BUS_D2_Pin | FPGA_BUS_D3_Pin | FPGA_BUS_D4_Pin | FPGA_BUS_D5_Pin | FPGA_BUS_D6_Pin | FPGA_BUS_D7_Pin;
	FPGA_GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	FPGA_GPIO_InitStruct.Pull = GPIO_PULLUP;
	FPGA_GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(FPGA_BUS_D0_GPIO_Port, &FPGA_GPIO_InitStruct);

	HAL_GPIO_WritePin(FPGA_CLK_GPIO_Port, FPGA_CLK_Pin | FPGA_SYNC_Pin, GPIO_PIN_RESET);
	FPGA_GPIO_InitStruct.Pin = FPGA_CLK_Pin | FPGA_SYNC_Pin;
	FPGA_GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	FPGA_GPIO_InitStruct.Pull = GPIO_PULLUP;
	FPGA_GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(FPGA_CLK_GPIO_Port, &FPGA_GPIO_InitStruct);

	// pre-reset FPGA to sync IQ data
	FPGA_setBusOutput();
	FPGA_writePacket(6); // RX RESET OFF
	FPGA_syncAndClockRiseFall();

	FPGA_setBusOutput();
	FPGA_writePacket(10); // TX RESET OFF
	FPGA_syncAndClockRiseFall();

	// BUS TEST
	for (uint16_t i = 0; i < 256; i++) {
		FPGA_setBusOutput();
		FPGA_writePacket(0);
		FPGA_syncAndClockRiseFall();

		FPGA_writePacket(i);
		FPGA_clockRise();
		FPGA_clockFall();

		FPGA_setBusInput();
		FPGA_clockRise();
		uint8_t ret = FPGA_readPacket;
		FPGA_clockFall();

		if (ret != i) {
			char buff[64];
			sprintf(buff, "BUS Error: %d -> %d", i, ret);
			FPGA_bus_test_result = false;
			LCD_showError(buff, false);
			HAL_Delay(300);
		}
	}

	// GET FW VERSION
	FPGA_setBusOutput();
	FPGA_writePacket(8);
	FPGA_syncAndClockRiseFall();

	FPGA_setBusInput();
	FPGA_clockRise();
	FPGA_FW_Version[2] = FPGA_readPacket;
	FPGA_clockFall();
	FPGA_clockRise();
	FPGA_FW_Version[1] = FPGA_readPacket;
	FPGA_clockFall();
	FPGA_clockRise();
	FPGA_FW_Version[0] = FPGA_readPacket;
	FPGA_clockFall();

	if (bus_test) // BUS STRESS TEST MODE
	{
		LCD_showError("Check FPGA BUS...", false);
		HAL_Delay(1000);

		while (bus_test) {
			for (uint16_t i = 0; i < 256; i++) {
				FPGA_setBusOutput();
				FPGA_writePacket(0);
				FPGA_syncAndClockRiseFall();

				FPGA_writePacket(i);
				FPGA_clockRise();
				FPGA_clockFall();

				FPGA_setBusInput();
				FPGA_clockRise();
				uint8_t ret = FPGA_readPacket;
				FPGA_clockFall();

				if (ret != i) {
					char buff[64];
					sprintf(buff, "BUS Error: %d -> %d", i, ret);
					LCD_showError(buff, false);
					HAL_Delay(1000);
				}
			}
			LCD_showError("Check compleate!", false);
		}
	}

	// start FPGA bus
	FPGA_bus_stop = false;
}

// restart FPGA modules
static void FPGA_restart_RX(void) // restart FPGA modules
{
	static bool FPGA_restart_state = false;
	if (!FPGA_restart_state) {
		FPGA_setBusOutput();
		FPGA_writePacket(5); // RESET ON
		FPGA_syncAndClockRiseFall();
		FPGA_restart_state = true;
	} else {
		FPGA_setBusOutput();
		FPGA_writePacket(6); // RESET OFF
		FPGA_syncAndClockRiseFall();
		FPGA_NeedRestart_RX = false;
		FPGA_restart_state = false;
	}
}

static void FPGA_restart_TX(void) // restart FPGA modules
{
	static bool FPGA_restart_state = false;
	if (!FPGA_restart_state) {
		FPGA_setBusOutput();
		FPGA_writePacket(9); // RESET ON
		FPGA_syncAndClockRiseFall();
		FPGA_restart_state = true;
		// print("TX RESET ON ");
	} else {
		FPGA_setBusOutput();
		FPGA_writePacket(10); // RESET OFF
		FPGA_syncAndClockRiseFall();
		FPGA_NeedRestart_TX = false;
		FPGA_restart_state = false;
		// println("OFF");
	}
}

// exchange parameters with FPGA
void FPGA_fpgadata_stuffclock(void) {
	if (!FPGA_NeedSendParams && !FPGA_NeedGetParams && !FPGA_NeedRestart_RX && !FPGA_NeedRestart_TX) {
		return;
	}
	if (FPGA_bus_stop) {
		return;
	}

	// data exchange
	if (FPGA_NeedSendParams) {
		FPGA_fpgadata_sendparam();
		FPGA_NeedSendParams = false;
	} else if (FPGA_NeedGetParams) {
		FPGA_fpgadata_getparam();
		FPGA_NeedGetParams = false;
	} else if (FPGA_NeedRestart_RX) {
		FPGA_restart_RX();
	} else if (FPGA_NeedRestart_TX) {
		FPGA_restart_TX();
	}
}

// exchange IQ data with FPGA
void FPGA_fpgadata_iqclock(void) {
	if (FPGA_bus_stop) {
		return;
	}
	VFO *current_vfo = CurrentVFO;
	if (current_vfo->Mode == TRX_MODE_LOOPBACK) {
		return;
	}
	// data exchange

	bool need_send_tx_zeroes = TRX_TX_sendZeroes < 100;

	// STAGE 1
	// out
	FPGA_setBusOutput();
	if (need_send_tx_zeroes) {
		TRX_TX_sendZeroes++;
		FPGA_writePacket(3); // TX SEND CLEAN IQ
		FPGA_syncAndClockRiseFall();
		FPGA_fpgadata_sendiq(true);
	} else if (TRX_on_TX) {
		FPGA_writePacket(3); // TX SEND IQ
		FPGA_syncAndClockRiseFall();
		FPGA_fpgadata_sendiq(false);
	}

	if (TRX_on_RX) {
		FPGA_writePacket(4); // RX GET IQ
		FPGA_syncAndClockRiseFall();

		// blocks by 48k
		FPGA_setBusInput();
		switch (TRX_GetRXSampleRateENUM) {
		case TRX_SAMPLERATE_K48:
			FPGA_fpgadata_getiq();
			break;
		case TRX_SAMPLERATE_K96:
			FPGA_fpgadata_getiq();
			FPGA_fpgadata_getiq();
			break;
		case TRX_SAMPLERATE_K192:
			FPGA_fpgadata_getiq();
			FPGA_fpgadata_getiq();
			FPGA_fpgadata_getiq();
			FPGA_fpgadata_getiq();
			break;
		case TRX_SAMPLERATE_K384:
			FPGA_fpgadata_getiq();
			FPGA_fpgadata_getiq();
			FPGA_fpgadata_getiq();
			FPGA_fpgadata_getiq();

			FPGA_fpgadata_getiq();
			FPGA_fpgadata_getiq();
			FPGA_fpgadata_getiq();
			FPGA_fpgadata_getiq();
			break;
		default:
			break;
		}
	}
}

// send parameters
static inline void FPGA_fpgadata_sendparam(void) {
	uint8_t FPGA_fpgadata_out_tmp8 = 0;

	// STAGE 1
	FPGA_setBusOutput();
	FPGA_writePacket(1);
	FPGA_syncAndClockRiseFall();

	// ATT CALC
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

	// STAGE 2
	// out PTT+PREAMP
	bitWrite(FPGA_fpgadata_out_tmp8, 0, (TRX_on_RX && CurrentVFO->Mode != TRX_MODE_LOOPBACK)); // RX1
	bitWrite(FPGA_fpgadata_out_tmp8, 1, false);                                                // RX2
	bitWrite(FPGA_fpgadata_out_tmp8, 2, (TRX_on_TX && CurrentVFO->Mode != TRX_MODE_LOOPBACK)); // TX
	bitWrite(FPGA_fpgadata_out_tmp8, 3, att_val_05);                                           // ATT_05
	bitWrite(FPGA_fpgadata_out_tmp8, 4, !TRX_on_RX || TRX.ADC_SHDN);
	if (TRX_on_TX) {
		bitWrite(FPGA_fpgadata_out_tmp8, 4, true); // shutdown ADC on TX
	}
	if (TRX.ATT) {
		bitWrite(FPGA_fpgadata_out_tmp8, 5, att_val_1); // ATT_1
	}
	if (TRX.ATT) {
		bitWrite(FPGA_fpgadata_out_tmp8, 6, att_val_2); // ATT_2
	}
	bitWrite(FPGA_fpgadata_out_tmp8, 7, TRX_on_RX && TRX.ADC_Driver);
	FPGA_writePacket(FPGA_fpgadata_out_tmp8);
	FPGA_clockRise();
	FPGA_clockFall();

	// STAGE 3
	// out RX1-FREQ
	FPGA_writePacket(((TRX_freq_phrase & (0xFFU << 24)) >> 24));
	FPGA_clockRise();
	FPGA_clockFall();

	// STAGE 4
	// out RX1-FREQ
	FPGA_writePacket(((TRX_freq_phrase & (0XFFU << 16)) >> 16));
	FPGA_clockRise();
	FPGA_clockFall();

	// STAGE 5
	// OUT RX1-FREQ
	FPGA_writePacket(((TRX_freq_phrase & (0XFFU << 8)) >> 8));
	FPGA_clockRise();
	FPGA_clockFall();

	// STAGE 6
	// OUT RX1-FREQ
	FPGA_writePacket(TRX_freq_phrase & 0XFFU);
	FPGA_clockRise();
	FPGA_clockFall();

	// STAGE 7
	// out RX2-FREQ
	FPGA_writePacket(((TRX_freq_phrase2 & (0XFFU << 24)) >> 24));
	FPGA_clockRise();
	FPGA_clockFall();

	// STAGE 8
	// out RX2-FREQ
	FPGA_writePacket(((TRX_freq_phrase2 & (0XFFU << 16)) >> 16));
	FPGA_clockRise();
	FPGA_clockFall();

	// STAGE 9
	// OUT RX2-FREQ
	FPGA_writePacket(((TRX_freq_phrase2 & (0XFFU << 8)) >> 8));
	FPGA_clockRise();
	FPGA_clockFall();

	// STAGE 10
	// OUT RX2-FREQ
	FPGA_writePacket(TRX_freq_phrase2 & 0XFF);
	FPGA_clockRise();
	FPGA_clockFall();

	// STAGE 11
	// OUT CICCOMP-GAIN
	switch (TRX_GetRXSampleRateENUM) {
	case TRX_SAMPLERATE_K48:
		FPGA_writePacket(CALIBRATE.CICFIR_GAINER_48K_val);
		break;
	case TRX_SAMPLERATE_K96:
		FPGA_writePacket(CALIBRATE.CICFIR_GAINER_96K_val);
		break;
	case TRX_SAMPLERATE_K192:
		FPGA_writePacket(CALIBRATE.CICFIR_GAINER_192K_val);
		break;
	case TRX_SAMPLERATE_K384:
		FPGA_writePacket(CALIBRATE.CICFIR_GAINER_384K_val);
		break;
	default:
		break;
	}
	FPGA_clockRise();
	FPGA_clockFall();

	// STAGE 12
	// OUT TX-CICCOMP-GAIN
	FPGA_writePacket(CALIBRATE.TXCICFIR_GAINER_val);
	FPGA_clockRise();
	FPGA_clockFall();

	// STAGE 13
	// OUT DAC-GAIN
	FPGA_writePacket(CALIBRATE.DAC_GAINER_val);
	FPGA_clockRise();
	FPGA_clockFall();

	// STAGE 14
	// OUT ADC OFFSET
	FPGA_writePacket(((0 & (0XFFU << 8)) >> 8)); // CALIBRATE.adc_offset
	FPGA_clockRise();
	FPGA_clockFall();

	// STAGE 15
	// OUT ADC OFFSET
	FPGA_writePacket(0 & 0XFFU); // CALIBRATE.adc_offset
	FPGA_clockRise();
	FPGA_clockFall();

	// STAGE 16
	uint16_t VCXO_PWM = 32750 + CALIBRATE.VCXO_correction;
	// OUT VCXO PWM
	FPGA_writePacket((VCXO_PWM & (0XFFU << 8)) >> 8);
	FPGA_clockRise();
	FPGA_clockFall();

	// STAGE 16
	// OUT VCXO PWM
	FPGA_writePacket(VCXO_PWM & 0XFFU);
	FPGA_clockRise();
	FPGA_clockFall();

	// LPF/BPF
	bool BPF_A, BPF_B, BPF_OE1, BPF_OE2, LPF_1, LPF_2, LPF_3;
	if (CurrentVFO->Freq >= 100000 && CurrentVFO->Freq <= 2400000) // 160m      1500000
	{
		BPF_A = false;                                                       // BPF_A
		BPF_B = false;                                                       // BPF_B
		BPF_OE1 = true;                                                      // BPF_OE1
		BPF_OE2 = false;                                                     // BPF_OE2
	} else if (CurrentVFO->Freq >= 2400000 && CurrentVFO->Freq <= 4500000) // 80m
	{
		BPF_A = true;                                                        // BPF_A
		BPF_B = false;                                                       // BPF_B
		BPF_OE1 = true;                                                      // BPF_OE1
		BPF_OE2 = false;                                                     // BPF_OE2
	} else if (CurrentVFO->Freq >= 4500000 && CurrentVFO->Freq <= 7500000) // 40m
	{
		BPF_A = false;                                                        // BPF_A
		BPF_B = true;                                                         // BPF_B
		BPF_OE1 = true;                                                       // BPF_OE1
		BPF_OE2 = false;                                                      // BPF_OE2
	} else if (CurrentVFO->Freq >= 7500000 && CurrentVFO->Freq <= 12000000) // 30m
	{
		BPF_A = true;                                                          // BPF_A
		BPF_B = true;                                                          // BPF_B
		BPF_OE1 = true;                                                        // BPF_OE1
		BPF_OE2 = false;                                                       // BPF_OE2
	} else if (CurrentVFO->Freq >= 12000000 && CurrentVFO->Freq <= 14800000) // 20m
	{
		BPF_A = false;                                                         // BPF_A
		BPF_B = false;                                                         // BPF_B
		BPF_OE1 = false;                                                       // BPF_OE1
		BPF_OE2 = true;                                                        // BPF_OE2
	} else if (CurrentVFO->Freq >= 14800000 && CurrentVFO->Freq <= 22000000) // 17,15m
	{
		BPF_A = true;    // BPF_A
		BPF_B = false;   // BPF_B
		BPF_OE1 = false; // BPF_OE1
		BPF_OE2 = true;  // BPF_OE2

	} else if (CurrentVFO->Freq >= 22000000 && CurrentVFO->Freq <= 32000000) // 12,10m
	{
		BPF_A = false;   // BPF_A
		BPF_B = true;    // BPF_B
		BPF_OE1 = false; // BPF_OE1
		BPF_OE2 = true;  // BPF_OE2
	} else             // LPF 0-53Mhz
	{
		BPF_A = true;    // BPF_A
		BPF_B = true;    // BPF_B
		BPF_OE1 = false; // BPF_OE1
		BPF_OE2 = true;  // BPF_OE2
	}

	if (CurrentVFO->Freq >= 100000 && CurrentVFO->Freq < 2400000) // 160m
	{
		LPF_1 = false;
		LPF_2 = false;
		LPF_3 = false;
	} else if (CurrentVFO->Freq >= 2400000 && CurrentVFO->Freq < 4500000) // 80m
	{
		LPF_1 = true;
		LPF_2 = false;
		LPF_3 = false;
	} else if (CurrentVFO->Freq >= 4500000 && CurrentVFO->Freq < 7500000) // 40m
	{
		LPF_1 = false;
		LPF_2 = true;
		LPF_3 = false;
	} else if (CurrentVFO->Freq >= 7500000 && CurrentVFO->Freq < 15000000) // 30m,20m
	{
		LPF_1 = true;
		LPF_2 = true;
		LPF_3 = false;
	} else if (CurrentVFO->Freq >= 15000000 && CurrentVFO->Freq < 32000000) // 17,15,12,10m
	{
		LPF_1 = false;
		LPF_2 = false;
		LPF_3 = true;
	} else if (CurrentVFO->Freq >= 32000000 && CurrentVFO->Freq < 70000000) // 6m
	{
		LPF_1 = true;
		LPF_2 = false;
		LPF_3 = true;
	} else if (CurrentVFO->Freq >= 70000000) { // FM+
		LPF_1 = false;
		LPF_2 = true;
		LPF_3 = true;
	}

	// STAGE 17
	// OUT SETTINGS
	FPGA_fpgadata_out_tmp8 = 0;
	if (TRX.ATT) {
		bitWrite(FPGA_fpgadata_out_tmp8, 0, att_val_4); // ATT_4
	}
	if (TRX.ATT) {
		bitWrite(FPGA_fpgadata_out_tmp8, 1, att_val_8); // ATT_8
	}
	if (TRX.ATT) {
		bitWrite(FPGA_fpgadata_out_tmp8, 2, att_val_16); // ATT_16
	}
	bitWrite(FPGA_fpgadata_out_tmp8, 3, BPF_A);    // BPF_A
	bitWrite(FPGA_fpgadata_out_tmp8, 4, BPF_B);    // BPF_B
	bitWrite(FPGA_fpgadata_out_tmp8, 5, !BPF_OE1); // BPF_OE1
	// 11 - 48kHz 01 - 96kHz 10 - 192kHz 00 - 384kHz IQ speed
	switch (TRX_GetRXSampleRateENUM) {
	case TRX_SAMPLERATE_K48:
		bitWrite(FPGA_fpgadata_out_tmp8, 6, 1);
		bitWrite(FPGA_fpgadata_out_tmp8, 7, 1);
		break;
	case TRX_SAMPLERATE_K96:
		bitWrite(FPGA_fpgadata_out_tmp8, 6, 0);
		bitWrite(FPGA_fpgadata_out_tmp8, 7, 1);
		break;
	case TRX_SAMPLERATE_K192:
		bitWrite(FPGA_fpgadata_out_tmp8, 6, 1);
		bitWrite(FPGA_fpgadata_out_tmp8, 7, 0);
		break;
	case TRX_SAMPLERATE_K384:
		bitWrite(FPGA_fpgadata_out_tmp8, 6, 0);
		bitWrite(FPGA_fpgadata_out_tmp8, 7, 0);
		break;
	default:
		break;
	}
	FPGA_writePacket(FPGA_fpgadata_out_tmp8);
	FPGA_clockRise();
	FPGA_clockFall();

	// STAGE 18
	// out TX-FREQ
	FPGA_writePacket(((TRX_freq_phrase_tx & (0XFFU << 24)) >> 24));
	FPGA_clockRise();
	FPGA_clockFall();

	// STAGE 19
	// out TX-FREQ
	FPGA_writePacket(((TRX_freq_phrase_tx & (0XFFU << 16)) >> 16));
	FPGA_clockRise();
	FPGA_clockFall();

	// STAGE 20
	// OUT TX-FREQ
	FPGA_writePacket(((TRX_freq_phrase_tx & (0XFFU << 8)) >> 8));
	FPGA_clockRise();
	FPGA_clockFall();

	// STAGE 21
	// OUT TX-FREQ
	FPGA_writePacket(TRX_freq_phrase_tx & 0XFFU);
	FPGA_clockRise();
	FPGA_clockFall();

	// STAGE 22
	// OUT PARAMS
	FPGA_fpgadata_out_tmp8 = 0;
	bitWrite(FPGA_fpgadata_out_tmp8, 0, !BPF_OE2);
	bitWrite(FPGA_fpgadata_out_tmp8, 1, LPF_1);
	bitWrite(FPGA_fpgadata_out_tmp8, 2, LPF_2);
	bitWrite(FPGA_fpgadata_out_tmp8, 3, LPF_3);
	FPGA_writePacket(FPGA_fpgadata_out_tmp8 & 0XFFU);
	FPGA_clockRise();
	FPGA_clockFall();
}

// get parameters
static inline void FPGA_fpgadata_getparam(void) {
	register uint8_t FPGA_fpgadata_in_tmp8 = 0;
	register int16_t FPGA_fpgadata_in_tmp16 = 0;
	register int32_t FPGA_fpgadata_in_tmp32 = 0;

	// STAGE 1
	FPGA_setBusOutput();
	FPGA_writePacket(2);
	FPGA_syncAndClockRiseFall();

	// STAGE 2
	FPGA_setBusInput();
	FPGA_clockRise();
	FPGA_fpgadata_in_tmp8 = FPGA_readPacket;
	if (ADCDAC_OVR_StatusLatency >= 50) {
		TRX_ADC_OTR = bitRead(FPGA_fpgadata_in_tmp8, 0);
		TRX_DAC_OTR = bitRead(FPGA_fpgadata_in_tmp8, 1);
	} else {
		ADCDAC_OVR_StatusLatency++;
	}
	/*bool IQ_overrun = bitRead(FPGA_fpgadata_in_tmp8, 2);
	if(IQ_overrun)
	  println("iq overrun");*/
	FPGA_clockFall();

	// STAGE 3
	FPGA_clockRise();
	FPGA_fpgadata_in_tmp8 = FPGA_readPacket;
	FPGA_clockFall();
	// STAGE 4
	FPGA_clockRise();
	TRX_ADC_MINAMPLITUDE = (int16_t)(((FPGA_fpgadata_in_tmp8 << 8) & 0xFF00) | FPGA_readPacket);
	FPGA_clockFall();

	// STAGE 5
	FPGA_clockRise();
	FPGA_fpgadata_in_tmp8 = FPGA_readPacket;
	FPGA_clockFall();
	// STAGE 6
	FPGA_clockRise();
	TRX_ADC_MAXAMPLITUDE = (int16_t)(((FPGA_fpgadata_in_tmp8 << 8) & 0xFF00) | FPGA_readPacket);
	FPGA_clockFall();

	// STAGE 7 - ADC RAW DATA
	FPGA_fpgadata_in_tmp16 = 0;
	FPGA_clockRise();
	FPGA_fpgadata_in_tmp16 |= ((FPGA_readPacket & 0x0F) << 8);
	FPGA_clockFall();
	// STAGE 8
	FPGA_clockRise();
	FPGA_fpgadata_in_tmp16 |= (FPGA_readPacket);
	FPGA_clockFall();

	if (bitRead(FPGA_fpgadata_in_tmp16, 11) == 1) {
		FPGA_fpgadata_in_tmp16 |= 0xF000; // int12 to int16 extension
	}

	ADC_RAW_IN = FPGA_fpgadata_in_tmp16;
}

// get IQ data
static float32_t *FPGA_Audio_Buffer_RX1_I_current = (float32_t *)&FPGA_Audio_Buffer_RX1_I_A[0];
static float32_t *FPGA_Audio_Buffer_RX1_Q_current = (float32_t *)&FPGA_Audio_Buffer_RX1_Q_A[0];
#if HRDW_HAS_DUAL_RX
static float32_t *FPGA_Audio_Buffer_RX2_I_current = (float32_t *)&FPGA_Audio_Buffer_RX2_I_A[0];
static float32_t *FPGA_Audio_Buffer_RX2_Q_current = (float32_t *)&FPGA_Audio_Buffer_RX2_Q_A[0];
#endif

static inline void FPGA_fpgadata_getiq(void) {
	float32_t FPGA_fpgadata_in_float32_i;
	float32_t FPGA_fpgadata_in_float32_q;
	struct {
		signed int q : 24;
		signed int i : 24;
	} FPGA_fpgadata_in_int24;

	FPGA_samples++;

	// STAGE 2 in Q RX1
	FPGA_clockPulse();
	FPGA_fpgadata_in_int24.q = (FPGA_readPacket << 16);

	// STAGE 3
	FPGA_clockPulse();
	FPGA_fpgadata_in_int24.q |= (FPGA_readPacket << 8);

	// STAGE 4
	FPGA_clockPulse();
	FPGA_fpgadata_in_int24.q |= (FPGA_readPacket);

	// STAGE 5 in I RX1
	FPGA_clockPulse();
	FPGA_fpgadata_in_int24.i = (FPGA_readPacket << 16);

	// STAGE 6
	FPGA_clockPulse();
	FPGA_fpgadata_in_int24.i |= (FPGA_readPacket << 8);

	// STAGE 7
	FPGA_clockPulse();
	FPGA_fpgadata_in_int24.i |= (FPGA_readPacket);

	FPGA_fpgadata_in_float32_i = FPGA_fpgadata_in_int24.i * 1.192093037616377e-7f; // int24 to float (+-8388607.0f)
	FPGA_fpgadata_in_float32_q = FPGA_fpgadata_in_int24.q * 1.192093037616377e-7f; // int24 to float (+-8388607.0f)

	*FFTInput_Q_current++ = FPGA_fpgadata_in_float32_q;
	*FPGA_Audio_Buffer_RX1_Q_current++ = FPGA_fpgadata_in_float32_q;
	*FFTInput_I_current++ = FPGA_fpgadata_in_float32_i;
	*FPGA_Audio_Buffer_RX1_I_current++ = FPGA_fpgadata_in_float32_i;

	FPGA_Audio_RXBuffer_Index++;
	if (FPGA_Audio_RXBuffer_Index == FPGA_RX_IQ_BUFFER_HALF_SIZE) {
		FPGA_Audio_RXBuffer_Index = 0;
		if (FPGA_RX_buffer_ready) {
			FPGA_Buffer_underrun = true;
			// println("fpga overrun");
		} else {
			FPGA_RX_buffer_ready = true;
			FPGA_RX_Buffer_Current = !FPGA_RX_Buffer_Current;
		}

		if (TRX_RX1_IQ_swap) {
			if (FPGA_RX_Buffer_Current) {
				FPGA_Audio_Buffer_RX1_I_current = (float32_t *)&FPGA_Audio_Buffer_RX1_Q_A[0];
				FPGA_Audio_Buffer_RX1_Q_current = (float32_t *)&FPGA_Audio_Buffer_RX1_I_A[0];
			} else {
				FPGA_Audio_Buffer_RX1_I_current = (float32_t *)&FPGA_Audio_Buffer_RX1_Q_B[0];
				FPGA_Audio_Buffer_RX1_Q_current = (float32_t *)&FPGA_Audio_Buffer_RX1_I_B[0];
			}
		} else {
			if (FPGA_RX_Buffer_Current) {
				FPGA_Audio_Buffer_RX1_I_current = (float32_t *)&FPGA_Audio_Buffer_RX1_I_A[0];
				FPGA_Audio_Buffer_RX1_Q_current = (float32_t *)&FPGA_Audio_Buffer_RX1_Q_A[0];
			} else {
				FPGA_Audio_Buffer_RX1_I_current = (float32_t *)&FPGA_Audio_Buffer_RX1_I_B[0];
				FPGA_Audio_Buffer_RX1_Q_current = (float32_t *)&FPGA_Audio_Buffer_RX1_Q_B[0];
			}
		}
	}

	FFT_buff_index++;
	if (FFT_buff_index == FFT_HALF_SIZE) {
		FFT_buff_index = 0;
		if (FFT_new_buffer_ready) {
			// println("fft overrun");
		} else {
			FFT_new_buffer_ready = true;
			FFT_buff_current = !FFT_buff_current;
		}
		if (TRX_RX1_IQ_swap) {
			if (FFT_buff_current) {
				FFTInput_I_current = (float32_t *)&FFTInput_Q_A[0];
				FFTInput_Q_current = (float32_t *)&FFTInput_I_A[0];
			} else {
				FFTInput_I_current = (float32_t *)&FFTInput_Q_B[0];
				FFTInput_Q_current = (float32_t *)&FFTInput_I_B[0];
			}
		} else {
			if (FFT_buff_current) {
				FFTInput_I_current = (float32_t *)&FFTInput_I_A[0];
				FFTInput_Q_current = (float32_t *)&FFTInput_Q_A[0];
			} else {
				FFTInput_I_current = (float32_t *)&FFTInput_I_B[0];
				FFTInput_Q_current = (float32_t *)&FFTInput_Q_B[0];
			}
		}
	}
}

// send IQ data
static inline void FPGA_fpgadata_sendiq(bool clean) {
	int32_t FPGA_fpgadata_out_q_tmp32 = 0;
	int32_t FPGA_fpgadata_out_i_tmp32 = 0;
	FPGA_samples++;

	if (!clean) {
		if (!TRX_TX_IQ_swap) {
			FPGA_fpgadata_out_i_tmp32 = (int32_t)((float32_t)FPGA_Audio_SendBuffer_I[FPGA_Audio_TXBuffer_Index] * 8388607.0f); // float -> int24
			FPGA_fpgadata_out_q_tmp32 = (int32_t)((float32_t)FPGA_Audio_SendBuffer_Q[FPGA_Audio_TXBuffer_Index] * 8388607.0f);
		} else {
			FPGA_fpgadata_out_i_tmp32 = (int32_t)((float32_t)FPGA_Audio_SendBuffer_Q[FPGA_Audio_TXBuffer_Index] * 8388607.0f);
			FPGA_fpgadata_out_q_tmp32 = (int32_t)((float32_t)FPGA_Audio_SendBuffer_I[FPGA_Audio_TXBuffer_Index] * 8388607.0f);
		}
	}

	// STAGE 2 out Q
	FPGA_writePacket((FPGA_fpgadata_out_q_tmp32 >> 16) & 0xFF);
	FPGA_clockRise();
	FPGA_clockFall();

	// STAGE 3
	FPGA_writePacket((FPGA_fpgadata_out_q_tmp32 >> 8) & 0xFF);
	FPGA_clockRise();
	FPGA_clockFall();

	// STAGE 4
	FPGA_writePacket((FPGA_fpgadata_out_q_tmp32 >> 0) & 0xFF);
	FPGA_clockRise();
	FPGA_clockFall();

	// STAGE 5 out I
	FPGA_writePacket((FPGA_fpgadata_out_i_tmp32 >> 16) & 0xFF);
	FPGA_clockRise();
	FPGA_clockFall();

	// STAGE 6
	FPGA_writePacket((FPGA_fpgadata_out_i_tmp32 >> 8) & 0xFF);
	FPGA_clockRise();
	FPGA_clockFall();

	// STAGE 7
	FPGA_writePacket((FPGA_fpgadata_out_i_tmp32 >> 0) & 0xFF);
	FPGA_clockRise();
	FPGA_clockFall();

	FPGA_Audio_TXBuffer_Index++;
	if (FPGA_Audio_TXBuffer_Index == FPGA_TX_IQ_BUFFER_SIZE) {
		if (Processor_NeedTXBuffer) {
			FPGA_Buffer_underrun = true;
			FPGA_Audio_TXBuffer_Index--;
		} else {
			FPGA_Audio_TXBuffer_Index = 0;
			FPGA_Audio_Buffer_State = true;
			Processor_NeedTXBuffer = true;
		}
	} else if (FPGA_Audio_TXBuffer_Index == FPGA_TX_IQ_BUFFER_HALF_SIZE) {
		if (Processor_NeedTXBuffer) {
			FPGA_Buffer_underrun = true;
			FPGA_Audio_TXBuffer_Index--;
		} else {
			FPGA_Audio_Buffer_State = false;
			Processor_NeedTXBuffer = true;
		}
	}
}

// switch the bus to input
static inline void FPGA_setBusInput(void) {
	// Configure IO Direction mode (Input)
	/*register uint32_t temp = GPIOA->MODER;
	temp &= ~(GPIO_MODER_MODE0 << (0 * 2U));
	temp |= ((GPIO_MODE_INPUT & 0x00000003U) << (0 * 2U));
	temp &= ~(GPIO_MODER_MODE0 << (1 * 2U));
	temp |= ((GPIO_MODE_INPUT & 0x00000003U) << (1 * 2U));
	temp &= ~(GPIO_MODER_MODE0 << (2 * 2U));
	temp |= ((GPIO_MODE_INPUT & 0x00000003U) << (2 * 2U));
	temp &= ~(GPIO_MODER_MODE0 << (3 * 2U));
	temp |= ((GPIO_MODE_INPUT & 0x00000003U) << (3 * 2U));
	temp &= ~(GPIO_MODER_MODE0 << (4 * 2U));
	temp |= ((GPIO_MODE_INPUT & 0x00000003U) << (4 * 2U));
	temp &= ~(GPIO_MODER_MODE0 << (5 * 2U));
	temp |= ((GPIO_MODE_INPUT & 0x00000003U) << (5 * 2U));
	temp &= ~(GPIO_MODER_MODE0 << (6 * 2U));
	temp |= ((GPIO_MODE_INPUT & 0x00000003U) << (6 * 2U));
	temp &= ~(GPIO_MODER_MODE0 << (7 * 2U));
	temp |= ((GPIO_MODE_INPUT & 0x00000003U) << (7 * 2U));
	//println(temp,false);
	GPIOA->MODER = temp;*/

	FPGA_setGPIOBusInput; // macros
}

// switch bus to pin
static inline void FPGA_setBusOutput(void) {
	// Configure IO Direction mode (Output)
	/*uint32_t temp = GPIOA->MODER;
	temp &= ~(GPIO_MODER_MODE0 << (0 * 2U));
	temp |= ((GPIO_MODE_OUTPUT_PP & 0x00000003U) << (0 * 2U));
	temp &= ~(GPIO_MODER_MODE0 << (1 * 2U));
	temp |= ((GPIO_MODE_OUTPUT_PP & 0x00000003U) << (1 * 2U));
	temp &= ~(GPIO_MODER_MODE0 << (2 * 2U));
	temp |= ((GPIO_MODE_OUTPUT_PP & 0x00000003U) << (2 * 2U));
	temp &= ~(GPIO_MODER_MODE0 << (3 * 2U));
	temp |= ((GPIO_MODE_OUTPUT_PP & 0x00000003U) << (3 * 2U));
	temp &= ~(GPIO_MODER_MODE0 << (4 * 2U));
	temp |= ((GPIO_MODE_OUTPUT_PP & 0x00000003U) << (4 * 2U));
	temp &= ~(GPIO_MODER_MODE0 << (5 * 2U));
	temp |= ((GPIO_MODE_OUTPUT_PP & 0x00000003U) << (5 * 2U));
	temp &= ~(GPIO_MODER_MODE0 << (6 * 2U));
	temp |= ((GPIO_MODE_OUTPUT_PP & 0x00000003U) << (6 * 2U));
	temp &= ~(GPIO_MODER_MODE0 << (7 * 2U));
	temp |= ((GPIO_MODE_OUTPUT_PP & 0x00000003U) << (7 * 2U));
	//println(temp,false);
	GPIOA->MODER = temp;*/

	FPGA_setGPIOBusOutput; // macros
}

// raise the CLK signal
static inline void FPGA_clockRise(void) {
	FPGA_CLK_GPIO_Port->BSRR = FPGA_CLK_Pin;
	__asm("nop");
	__asm("nop");
	__asm("nop");
}

// remove CLK signal
static inline void FPGA_clockFall(void) { FPGA_CLK_GPIO_Port->BSRR = (FPGA_CLK_Pin << 16U); }

// raise and fall the CLK signal
static inline void FPGA_clockPulse(void) {
	FPGA_CLK_GPIO_Port->BSRR = FPGA_CLK_Pin;
	__asm("nop");
	FPGA_CLK_GPIO_Port->BSRR = (FPGA_CLK_Pin << 16U);
}

// raise CLK and SYNC signal, then lower
static inline void FPGA_syncAndClockRiseFall(void) {
	FPGA_CLK_GPIO_Port->BSRR = FPGA_SYNC_Pin;
	FPGA_CLK_GPIO_Port->BSRR = FPGA_CLK_Pin;
	FPGA_CLK_GPIO_Port->BSRR = (FPGA_SYNC_Pin << 16U) | (FPGA_CLK_Pin << 16U);
}

static uint8_t FPGA_spi_start_command(uint8_t command) // execute command to SPI flash
{
	// STAGE 1
	FPGA_setBusOutput();
	FPGA_writePacket(7); // FPGA FLASH READ command
	FPGA_syncAndClockRiseFall();
	FPGA_FLASH_COMMAND_DELAY

	// STAGE 2 WRITE (F700)
	FPGA_writePacket(command);
	FPGA_clockRise();
	FPGA_clockFall();
	FPGA_FLASH_WRITE_DELAY

	// STAGE 3 READ ANSWER (F701)
	FPGA_setBusInput();
	FPGA_clockRise();
	uint8_t data = FPGA_readPacket;
	FPGA_clockFall();
	FPGA_FLASH_READ_DELAY

	return data;
}

static void FPGA_spi_stop_command(void) // shutdown with SPI flash
{
	// STAGE 1
	FPGA_setBusOutput();
	FPGA_writePacket(7); // FPGA FLASH READ command
	FPGA_syncAndClockRiseFall();
	FPGA_FLASH_COMMAND_DELAY
}

static uint8_t FPGA_spi_continue_command(uint8_t writedata) // Continue reading and writing SPI flash
{
	// STAGE 2 WRITE (F700)
	FPGA_setBusOutput();
	FPGA_writePacket(writedata);
	FPGA_clockRise();
	FPGA_clockFall();
	FPGA_FLASH_WRITE_DELAY

	// STAGE 3 READ ANSWER (F701)
	FPGA_setBusInput();
	FPGA_clockRise();
	uint8_t data = FPGA_readPacket;
	FPGA_clockFall();
	FPGA_FLASH_READ_DELAY

	return data;
}

static void FPGA_spi_flash_wait_WIP(void) // We are waiting for the end of writing to the flash (resetting the WIP register)
{
	uint8_t status = 1;
	while (bitRead(status, 0) == 1) {
		FPGA_spi_start_command(M25P80_READ_STATUS_REGISTER);
		status = FPGA_spi_continue_command(M25P80_READ_STATUS_REGISTER);
		FPGA_spi_stop_command();
	}
}

bool FPGA_is_present(void) // check that the FPGA has firmware
{
	HAL_Delay(1);
	uint8_t data = 0;
	FPGA_spi_start_command(M25P80_RELEASE_from_DEEP_POWER_DOWN); // Wake-Up
	FPGA_spi_stop_command();
	FPGA_spi_flash_wait_WIP();
	FPGA_spi_start_command(M25P80_READ_DATA_BYTES); // READ DATA BYTES
	FPGA_spi_continue_command(0x00);                // addr 1
	FPGA_spi_continue_command(0x00);                // addr 2
	FPGA_spi_continue_command(0x00);                // addr 3
	data = FPGA_spi_continue_command(0xFF);
	FPGA_spi_stop_command();
	FPGA_spi_start_command(M25P80_DEEP_POWER_DOWN); // Go sleep
	FPGA_spi_stop_command();
	if (data != 0xFF) {
		LCD_showError("FPGA not found", true);
		println("[ERR] FPGA not found");
		return false;
	} else {
		return true;
	}
}

bool FPGA_spi_flash_verify(uint32_t flash_pos, uint8_t *buff, uint32_t size) // check flash memory
{
	HAL_Delay(1);
	uint8_t data = 0;
	FPGA_spi_start_command(M25P80_RELEASE_from_DEEP_POWER_DOWN); // Wake-Up
	FPGA_spi_stop_command();
	FPGA_spi_flash_wait_WIP();
	FPGA_spi_start_command(M25P80_READ_DATA_BYTES);      // READ DATA BYTES
	FPGA_spi_continue_command((flash_pos >> 16) & 0xFF); // addr 1
	FPGA_spi_continue_command((flash_pos >> 8) & 0xFF);  // addr 2
	FPGA_spi_continue_command(flash_pos & 0xFF);         // addr 3
	data = FPGA_spi_continue_command(0xFF);
	uint8_t progress_prev = 0;
	uint8_t progress = 0;

	// Decompress RLE and verify
	uint32_t errors = 0;
	uint32_t file_pos = 0;
	while (file_pos < size) {
		if (rev8((uint8_t)data) != buff[file_pos]) {
			errors++;
			print(flash_pos, ": FPGA: ");
			print_hex(rev8((uint8_t)data), true);
			println(" HEX: ", buff[file_pos]);
			print_flush();
		}
		data = FPGA_spi_continue_command(0xFF);
		flash_pos++;
		file_pos++;

		progress = (uint8_t)((float32_t)flash_pos / (float32_t)FPGA_flash_size * 100.0f);
		if (progress_prev != progress && ((progress - progress_prev) >= 5)) {
			println("[FLASH] Verify: ", progress);
			progress_prev = progress;
		}
		if (errors > 10) {
			break;
		}
	}
	FPGA_spi_stop_command();
	FPGA_spi_start_command(M25P80_DEEP_POWER_DOWN); // Go sleep
	FPGA_spi_stop_command();
	//
	if (errors > 0) {
		println("[ERR] FPGA Flash verification failed");
		return false;
	} else {
		return true;
	}
}

void FPGA_spi_flash_erase(void) // clear flash memory
{
	HAL_Delay(1);
	FPGA_spi_start_command(M25P80_RELEASE_from_DEEP_POWER_DOWN); // Wake-Up
	FPGA_spi_stop_command();
	FPGA_spi_flash_wait_WIP(); // wait write in progress

	FPGA_spi_start_command(M25P80_WRITE_ENABLE); // Write Enable
	FPGA_spi_stop_command();
	FPGA_spi_start_command(M25P80_BULK_ERASE); // BULK FULL CHIP ERASE
	FPGA_spi_stop_command();
	FPGA_spi_flash_wait_WIP(); // wait write in progress
}

void FPGA_spi_flash_write(uint32_t flash_pos, uint8_t *buff, uint32_t size) // write new contents of FPGA SPI memory
{
	HAL_Delay(1);
	uint16_t page_pos = 0;
	FPGA_spi_start_command(M25P80_RELEASE_from_DEEP_POWER_DOWN); // Wake-Up
	FPGA_spi_stop_command();
	FPGA_spi_flash_wait_WIP();                   // wait write in progress
	FPGA_spi_start_command(M25P80_WRITE_ENABLE); // Write Enable
	FPGA_spi_stop_command();
	FPGA_spi_start_command(M25P80_PAGE_PROGRAM);         // Page programm
	FPGA_spi_continue_command((flash_pos >> 16) & 0xFF); // addr 1
	FPGA_spi_continue_command((flash_pos >> 8) & 0xFF);  // addr 2
	FPGA_spi_continue_command(flash_pos & 0xFF);         // addr 3
	uint8_t progress_prev = 0;
	uint8_t progress = 0;

	// Decompress RLE and write
	uint32_t file_pos = 0;
	while (file_pos < size) {
		FPGA_spi_continue_command(rev8((uint8_t)buff[file_pos]));
		flash_pos++;
		page_pos++;
		if (page_pos >= FPGA_page_size) {
			FPGA_spi_stop_command();
			FPGA_spi_flash_wait_WIP();                   // wait write in progress
			FPGA_spi_start_command(M25P80_WRITE_ENABLE); // Write Enable
			FPGA_spi_stop_command();
			FPGA_spi_start_command(M25P80_PAGE_PROGRAM);         // Page programm
			FPGA_spi_continue_command((flash_pos >> 16) & 0xFF); // addr 1
			FPGA_spi_continue_command((flash_pos >> 8) & 0xFF);  // addr 2
			FPGA_spi_continue_command(flash_pos & 0xFF);         // addr 3
			page_pos = 0;
		}
		file_pos++;
		progress = (uint8_t)((float32_t)flash_pos / (float32_t)FPGA_flash_size * 100.0f);
		if (progress_prev != progress) {
			println("[FLASH] Progress: ", progress);
			progress_prev = progress;
		}
	}
	FPGA_spi_stop_command();
	FPGA_spi_flash_wait_WIP();                    // wait write in progress
	FPGA_spi_start_command(M25P80_WRITE_DISABLE); // Write Disable
	FPGA_spi_stop_command();
}

void FPGA_choise_DAC_PLL(uint64_t freq) {
	// only one dac pll in fpga
}
