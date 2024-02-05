#include "trx_manager.h"
#include "FT8/FT8_main.h"
#include "agc.h"
#include "atu.h"
#include "audio_filters.h"
#include "auto_notch.h"
#include "bands.h"
#include "codec.h"
#include "cw.h"
#include "cw_decoder.h"
#include "filemanager.h"
#include "fpga.h"
#include "front_unit.h"
#include "functions.h"
#include "hardware.h"
#include "lcd.h"
#include "main.h"
#include "noise_reduction.h"
#include "pre_distortion.h"
#include "rf_unit.h"
#include "satellite.h"
#include "sd.h"
#include "self_test.h"
#include "settings.h"
#include "snap.h"
#include "swr_analyzer.h"
#include "system_menu.h"
#include "usbd_audio_if.h"
#include "vad.h"

volatile bool TRX_ptt_hard = false;
volatile bool TRX_ptt_soft = false;
volatile bool TRX_old_ptt_soft = false;
volatile bool TRX_RX1_IQ_swap = false;
volatile bool TRX_RX2_IQ_swap = false;
volatile bool TRX_TX_IQ_swap = false;
volatile bool TRX_Tune = false;
volatile bool TRX_Inited = false;
volatile float32_t TRX_RX1_dBm = -100.0f;
volatile float32_t TRX_RX2_dBm = -100.0f;
volatile float32_t TRX_RX1_dBm_measurement = -100.0f;
volatile bool TRX_ADC_OTR = false;
volatile bool TRX_DAC_OTR = false;
volatile bool TRX_PWR_ALC_SWR_OVERFLOW = false;
volatile bool TRX_MIC_BELOW_NOISEGATE = false;
volatile int16_t TRX_ADC_MINAMPLITUDE = 0;
volatile int16_t TRX_ADC_MAXAMPLITUDE = 0;
volatile uint32_t TRX_SNTP_Synced = 0; // time of the last time synchronization
volatile int_fast16_t TRX_RIT = 0;
volatile int_fast16_t TRX_XIT = 0;
volatile float32_t TRX_MAX_TX_Amplitude = 1.0f;
volatile float32_t TRX_PWR_Forward = 0;
volatile float32_t TRX_PWR_Backward = 0;
volatile float32_t TRX_SWR = 0;
volatile float32_t TRX_PWR_Forward_SMOOTHED = 0;
volatile float32_t TRX_PWR_Backward_SMOOTHED = 0;
volatile float32_t TRX_SWR_SMOOTHED = 0;
char TRX_SWR_SMOOTHED_STR[12] = "SWR: 1.0";
volatile float32_t TRX_VLT_forward = 0;  // Tisho
volatile float32_t TRX_VLT_backward = 0; // Tisho
volatile float32_t TRX_ALC_OUT = 0;
volatile float32_t TRX_ALC_IN = 0;
volatile bool TRX_SWR_PROTECTOR = false;
volatile bool TRX_DAC_DIV0 = false;
volatile bool TRX_DAC_DIV1 = false;
volatile bool TRX_DAC_HP1 = false;
volatile bool TRX_DAC_HP2 = false;
volatile bool TRX_DAC_X4 = false;
volatile bool TRX_DCDC_Freq = false;
volatile bool TRX_DAC_DRV_A0 = true;
volatile bool TRX_DAC_DRV_A1 = true;
volatile float32_t TRX_STM32_VREF = 3.3f;         // voltage on STM32
volatile float32_t TRX_STM32_TEMPERATURE = 30.0f; // STM32 temperature
volatile float32_t TRX_IQ_phase_error = 0.0f;
volatile bool TRX_Temporary_Stop_BandMap = false;
volatile uint32_t TRX_Temporary_Mute_StartTime = 0;
uint32_t TRX_freq_phrase = 0;
uint32_t TRX_freq_phrase2 = 0;
uint32_t TRX_freq_phrase_tx = 0;
volatile float32_t TRX_RF_Temperature = 0.0f;
volatile bool TRX_ScanMode = false;
bool TRX_phase_restarted = false;
uint32_t TRX_TX_StartTime = 0;
uint32_t TRX_TX_EndTime = 0;
uint32_t TRX_Inactive_Time = 0;
uint32_t TRX_DXCluster_UpdateTime = 0;
uint32_t TRX_SAT_UpdateTime = 0;
uint32_t TRX_WOLF_Cluster_UpdateTime = 0;
volatile float32_t TRX_PWR_Voltage = 12.0f;
volatile float32_t TRX_PWR_Current = 0.0f;
volatile float32_t TRX_RF_Current = 0.0f;
volatile float32_t TRX_VBAT_Voltage = 0.0f;
volatile float32_t TRX_NoiseFloor = -150.0f;
volatile uint_fast16_t CW_Key_Timeout_est = 0;
uint32_t dbg_FPGA_samples = 0;
uint8_t TRX_TX_Harmonic = 1;
uint8_t TRX_TX_sendZeroes = 0;
int8_t TRX_MemoryChannelSelected;
volatile bool TRX_REPEATER_Applied = false;

static uint_fast8_t TRX_TXRXMode = 0; // 0 - undef, 1 - rx, 2 - tx, 3 - txrx
static bool TRX_SPLIT_Applied = false;
static void TRX_Start_RX(void);
static void TRX_Start_TX(void);
static void TRX_Start_TXRX(void);

void TRX_Init() {
	TRX_Start_RX();
	CODEC_TXRX_mode();
	CODEC_start_i2s_and_dma();
	uint_fast8_t saved_mode = CurrentVFO->Mode;
	TRX_setFrequency(CurrentVFO->Freq, CurrentVFO);
	TRX_setMode(saved_mode, CurrentVFO);
	HRDW_Init();
#if HRDW_HAS_SD
	SAT_init();
#endif
}

void TRX_Restart_Mode() {
	ADCDAC_OVR_StatusLatency = 0;
	uint_fast8_t mode = CurrentVFO->Mode;

	// Switch mode
	if (TRX_on_TX) {
		if (mode == TRX_MODE_LOOPBACK || mode == TRX_MODE_CW || FULL_DUPLEX) {
			TRX_Start_TXRX();
		} else {
			TRX_Start_TX();
		}
	} else {
		TRX_Start_RX();
		TRX_TX_sendZeroes = 0;
		TRX_SWR_PROTECTOR = false;
	}

	// SPLIT
	if (TRX.SPLIT_Enabled && !TRX_SPLIT_Applied) {
		TRX_SPLIT_Applied = true;

		TRX.selected_vfo = !TRX.selected_vfo;
		if (!TRX.selected_vfo) {
			CurrentVFO = &TRX.VFO_A;
			SecondaryVFO = &TRX.VFO_B;
		} else {
			CurrentVFO = &TRX.VFO_B;
			SecondaryVFO = &TRX.VFO_A;
		}
		TRX_setFrequency(CurrentVFO->Freq, CurrentVFO);
		TRX_setMode(CurrentVFO->Mode, CurrentVFO);

		int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
		TRX.ANT_RX = TRX.BANDS_SAVED_SETTINGS[band].ANT_RX;
		TRX.ANT_TX = TRX.BANDS_SAVED_SETTINGS[band].ANT_TX;

		LCD_UpdateQuery.FreqInfoRedraw = true;
		LCD_UpdateQuery.TopButtons = true;
		LCD_UpdateQuery.StatusInfoGUIRedraw = true;
	}

	// Repeater mode
	if (TRX.RepeaterMode_shadow && !TRX_REPEATER_Applied) {
		TRX_REPEATER_Applied = true;

		if (TRX_on_TX) {
			TRX_setFrequency(CurrentVFO->Freq + TRX.REPEATER_Offset * 1000, CurrentVFO);
		} else {
			TRX_setFrequency(CurrentVFO->Freq - TRX.REPEATER_Offset * 1000, CurrentVFO);
		}

		LCD_UpdateQuery.FreqInfoRedraw = true;
		LCD_UpdateQuery.TopButtons = true;
		LCD_UpdateQuery.StatusInfoGUIRedraw = true;
	}

	// Ant swap for mode 1RX/2TX and others
	ATU_Load_Memory(TRX_on_TX ? TRX.ANT_TX : TRX.ANT_RX, CurrentVFO->Freq);

	// CW_In_SSB
	if (CW_In_SSB_applyed == 1 && !TRX_on_TX) { // return to LSB
		CW_In_SSB_applyed = 0;
		TRX_setMode(TRX_MODE_LSB, CurrentVFO);
		TRX_setFrequency(CurrentVFO->Freq + TRX.CW_Pitch, CurrentVFO);
	}
	if (CW_In_SSB_applyed == 2 && !TRX_on_TX) { // return to USB
		CW_In_SSB_applyed = 0;
		TRX_setMode(TRX_MODE_USB, CurrentVFO);
		TRX_setFrequency(CurrentVFO->Freq - TRX.CW_Pitch, CurrentVFO);
	}

	// XIT
	if (TRX.XIT_Enabled) {
		LCD_UpdateQuery.FreqInfoRedraw = true;
	}
}

static void TRX_Start_RX() {
	if (TRX_TXRXMode == 1) {
		return;
	}
	println("RX MODE");
	TRX_phase_restarted = false;
	RF_UNIT_UpdateState(false);
	CODEC_CleanBuffer();
	Processor_NeedRXBuffer = false;
	CODEC_Buffer_underrun = false;
	CODEC_DMA_state = true;
	TRX_SPLIT_Applied = false;
	TRX_REPEATER_Applied = false;
	TRX_TXRXMode = 1;

	// clean TX buffer
	dma_memset((void *)&FPGA_Audio_SendBuffer_Q[0], 0x00, sizeof(FPGA_Audio_SendBuffer_Q));
	dma_memset((void *)&FPGA_Audio_SendBuffer_I[0], 0x00, sizeof(FPGA_Audio_SendBuffer_I));

	LCD_UpdateQuery.StatusInfoGUIRedraw = true;
	NeedReinitReverber = true;
	NeedFFTReinit = true;

	// FPGA_NeedRestart_TX = true;
}

static void TRX_Start_TX() {
	if (TRX_TXRXMode == 2) {
		return;
	}
	println("TX MODE");
	RF_UNIT_UpdateState(false);
	CODEC_CleanBuffer();
	TRX_TX_StartTime = HAL_GetTick();
	TRX_SPLIT_Applied = false;
	TRX_REPEATER_Applied = false;
	TRX_TXRXMode = 2;
#if !defined(FRONTPANEL_LITE)
	CWDecoder_SetWPM(TRX.CW_KEYER_WPM);
#endif

	LCD_UpdateQuery.StatusInfoGUIRedraw = true;
	NeedReinitReverber = true;
	NeedFFTReinit = true;
}

static void TRX_Start_TXRX() {
	if (TRX_TXRXMode == 3) {
		return;
	}
	println("TXRX MODE");
	RF_UNIT_UpdateState(false);
	CODEC_CleanBuffer();
	TRX_TX_StartTime = HAL_GetTick();
	TRX_SPLIT_Applied = false;
	TRX_REPEATER_Applied = false;
	TRX_TXRXMode = 3;
#if !defined(FRONTPANEL_LITE)
	CWDecoder_SetWPM(TRX.CW_KEYER_WPM);
#endif

	LCD_UpdateQuery.StatusInfoGUIRedraw = true;
	NeedReinitReverber = true;
	NeedFFTReinit = true;

	// FPGA_NeedRestart_TX = true;
}

void TRX_ptt_change(void) {
	TRX_Inactive_Time = 0;

	if (!TRX_Inited) {
		return;
	}

	if (TRX_Tune) {
		TRX_Tune = false;
	}

	bool notx = TRX_TX_Disabled(CurrentVFO->Freq) && !TRX_REPEATER_Applied;
	if (notx) {
		TRX_ptt_soft = false;
		TRX_ptt_hard = false;
		return;
	}

	if (CurrentVFO->Mode == TRX_MODE_CW && TRX.CW_PTT_Type == KEY_PTT) // cw ptt type
	{
		TRX_ptt_soft = false;
		CW_key_serial = false;
		return;
	}

	bool TRX_new_ptt_hard = !HAL_GPIO_ReadPin(PTT_IN_GPIO_Port, PTT_IN_Pin);
	if (TRX_ptt_hard != TRX_new_ptt_hard) {
		if (TRX.Auto_Input_Switch) {
			if (CurrentVFO->Mode == TRX_MODE_DIGI_L || CurrentVFO->Mode == TRX_MODE_DIGI_U || CurrentVFO->Mode == TRX_MODE_RTTY || CurrentVFO->Mode == TRX_MODE_IQ) {
				TRX.InputType_DIGI = TRX_INPUT_MIC;
			} else {
				TRX.InputType_MAIN = TRX_INPUT_MIC;
			}
		}

		TRX_ptt_hard = TRX_new_ptt_hard;
		TRX_ptt_soft = false;
		CW_key_serial = false;
		LCD_UpdateQuery.StatusInfoGUIRedraw = true;
		FPGA_NeedSendParams = true;
		TRX_Restart_Mode();
	}
	if (TRX_ptt_soft != TRX_old_ptt_soft) {
		if (TRX.Auto_Input_Switch) {
			if (CurrentVFO->Mode == TRX_MODE_DIGI_L || CurrentVFO->Mode == TRX_MODE_DIGI_U || CurrentVFO->Mode == TRX_MODE_RTTY || CurrentVFO->Mode == TRX_MODE_IQ) {
				TRX.InputType_DIGI = TRX_INPUT_USB;
			} else {
				TRX.InputType_MAIN = TRX_INPUT_USB;
			}
		}

		TRX_old_ptt_soft = TRX_ptt_soft;
		LCD_UpdateQuery.StatusInfoGUIRedraw = true;
		FPGA_NeedSendParams = true;
		TRX_Restart_Mode();
	}
}

bool TRX_TX_Disabled(uint64_t freq) {
	bool notx = false;
	int8_t band = getBandFromFreq(freq, false);
	switch (band) {
	case BANDID_2200m:
		if (CALIBRATE.NOTX_2200m) {
			notx = true;
		}
		break;
	case BANDID_630m:
		if (CALIBRATE.NOTX_630m) {
			notx = true;
		}
		break;
	case BANDID_160m:
		if (CALIBRATE.NOTX_160m) {
			notx = true;
		}
		break;
	case BANDID_80m:
		if (CALIBRATE.NOTX_80m) {
			notx = true;
		}
		break;
	case BANDID_60m:
		if (CALIBRATE.NOTX_60m) {
			notx = true;
		}
		break;
	case BANDID_40m:
		if (CALIBRATE.NOTX_40m) {
			notx = true;
		}
		break;
	case BANDID_30m:
		if (CALIBRATE.NOTX_30m) {
			notx = true;
		}
		break;
	case BANDID_20m:
		if (CALIBRATE.NOTX_20m) {
			notx = true;
		}
		break;
	case BANDID_17m:
		if (CALIBRATE.NOTX_17m) {
			notx = true;
		}
		break;
	case BANDID_15m:
		if (CALIBRATE.NOTX_15m) {
			notx = true;
		}
		break;
	case BANDID_12m:
		if (CALIBRATE.NOTX_12m) {
			notx = true;
		}
		break;
	case BANDID_CB:
		if (CALIBRATE.NOTX_CB) {
			notx = true;
		}
		break;
	case BANDID_10m:
		if (CALIBRATE.NOTX_10m) {
			notx = true;
		}
		break;
	case BANDID_6m:
		if (CALIBRATE.NOTX_6m) {
			notx = true;
		}
		break;
	case BANDID_4m:
		if (CALIBRATE.NOTX_4m) {
			notx = true;
		}
		break;
	case BANDID_2m:
	case BANDID_Marine:
		if (CALIBRATE.NOTX_2m) {
			notx = true;
		}
		break;
	case BANDID_70cm:
		if (CALIBRATE.NOTX_70cm) {
			notx = true;
		}
		break;
	case BANDID_23cm:
		if (CALIBRATE.NOTX_23cm) {
			notx = true;
		}
		break;
	case BANDID_13cm:
		if (!TRX.Transverter_13cm) {
			notx = true;
		}
		break;
	case BANDID_6cm:
		if (!TRX.Transverter_6cm) {
			notx = true;
		}
		break;
	case BANDID_3cm:
		if (!TRX.Transverter_3cm) {
			notx = true;
		}
		break;
	case BANDID_QO100:
		if (!TRX.Transverter_QO100) {
			notx = true;
		}
		break;
	case BANDID_1_2cm:
		if (!TRX.Transverter_1_2cm) {
			notx = true;
		}
		break;
	default:
		if (CALIBRATE.NOTX_NOTHAM) {
			notx = true;
		}
		break;
	}
	return notx;
}

void TRX_setFrequency(uint64_t _freq, VFO *vfo) {
	if (_freq < 1) {
		return;
	}

	bool transverter_enabled = false;
	if (TRX.Transverter_2m || TRX.Transverter_70cm || TRX.Transverter_23cm || TRX.Transverter_13cm || TRX.Transverter_6cm || TRX.Transverter_3cm || TRX.Transverter_QO100 ||
	    TRX.Transverter_1_2cm) {
		transverter_enabled = true;
	}

	// save old band data
	int_fast8_t bandFromOldFreq = getBandFromFreq(vfo->Freq, false);
	TRX_SaveRFGain_Data(vfo->Mode, bandFromOldFreq);

	int64_t freq_diff = _freq - vfo->Freq;
	vfo->Freq = _freq;

	if (TRX.FREE_Tune && vfo->Mode != TRX_MODE_WFM) {
		uint64_t half_spectum_width = FFT_current_spectrum_width_Hz / 2 * 0.8 - vfo->LPF_RX_Filter_Width;

		if (llabs((int64_t)vfo->SpectrumCenterFreq - (int64_t)vfo->Freq) > (half_spectum_width * 2)) {
			vfo->SpectrumCenterFreq = vfo->Freq;
		}
		if ((vfo->SpectrumCenterFreq + half_spectum_width) < vfo->Freq) {
			vfo->SpectrumCenterFreq = vfo->Freq - half_spectum_width;
		}
		if ((vfo->SpectrumCenterFreq - half_spectum_width) > vfo->Freq) {
			vfo->SpectrumCenterFreq = vfo->Freq + half_spectum_width;
		}
		if (vfo->LPF_RX_Filter_Width == 0) {
			vfo->SpectrumCenterFreq = vfo->Freq;
		}
		vfo->SpectrumAndRXDiff = (int64_t)CurrentVFO->SpectrumCenterFreq - (int64_t)CurrentVFO->Freq;
		NeedWTFRedraw = true;
	} else {
		vfo->SpectrumCenterFreq = vfo->Freq;
		vfo->SpectrumAndRXDiff = 0;
	}

	// get settings and fpga freq phrase
	int64_t cur_vfo_freq = CurrentVFO->SpectrumCenterFreq + (TRX.RIT_Enabled ? TRX_RIT : 0);
	int8_t cur_vfo_band = getBandFromFreq(cur_vfo_freq, true);

	if (TRX.Transverter_2m && cur_vfo_band == BANDID_2m) {
		cur_vfo_freq = ((int64_t)CALIBRATE.Transverter_2m_IF_MHz * HZ_IN_MHZ) + (cur_vfo_freq - (int64_t)CALIBRATE.Transverter_2m_RF_MHz * HZ_IN_MHZ);
	}
	if (TRX.Transverter_70cm && cur_vfo_band == BANDID_70cm) {
		cur_vfo_freq = ((int64_t)CALIBRATE.Transverter_70cm_IF_MHz * HZ_IN_MHZ) + (cur_vfo_freq - (int64_t)CALIBRATE.Transverter_70cm_RF_MHz * HZ_IN_MHZ);
	}
	if (TRX.Transverter_23cm && cur_vfo_band == BANDID_23cm) {
		cur_vfo_freq = ((int64_t)CALIBRATE.Transverter_23cm_IF_MHz * HZ_IN_MHZ) + (cur_vfo_freq - (int64_t)CALIBRATE.Transverter_23cm_RF_MHz * HZ_IN_MHZ);
	}
	if (TRX.Transverter_13cm && cur_vfo_band == BANDID_13cm) {
		cur_vfo_freq = ((int64_t)CALIBRATE.Transverter_13cm_IF_MHz * HZ_IN_MHZ) + (cur_vfo_freq - (int64_t)CALIBRATE.Transverter_13cm_RF_MHz * HZ_IN_MHZ);
	}
	if (TRX.Transverter_6cm && cur_vfo_band == BANDID_6cm) {
		cur_vfo_freq = ((int64_t)CALIBRATE.Transverter_6cm_IF_MHz * HZ_IN_MHZ) + (cur_vfo_freq - (int64_t)CALIBRATE.Transverter_6cm_RF_MHz * HZ_IN_MHZ);
	}
	if (TRX.Transverter_3cm && cur_vfo_band == BANDID_3cm) {
		cur_vfo_freq = ((int64_t)CALIBRATE.Transverter_3cm_IF_MHz * HZ_IN_MHZ) + (cur_vfo_freq - (int64_t)CALIBRATE.Transverter_3cm_RF_MHz * HZ_IN_MHZ);
	}
	if (TRX.Transverter_QO100 && cur_vfo_band == BANDID_QO100) {
		cur_vfo_freq = ((int64_t)CALIBRATE.Transverter_QO100_IF_RX_kHz * 1000) + (cur_vfo_freq - (int64_t)CALIBRATE.Transverter_QO100_RF_kHz * 1000);
	}
	if (TRX.Transverter_1_2cm && cur_vfo_band == BANDID_1_2cm) {
		cur_vfo_freq = ((int64_t)CALIBRATE.Transverter_1_2cm_IF_MHz * HZ_IN_MHZ) + (cur_vfo_freq - (int64_t)CALIBRATE.Transverter_1_2cm_RF_MHz * HZ_IN_MHZ);
	}

	int64_t sec_vfo_freq = SecondaryVFO->Freq + (TRX.RIT_Enabled ? TRX_RIT : 0);
	int8_t sec_vfo_band = getBandFromFreq(sec_vfo_freq, true);

	if (TRX.Transverter_2m && sec_vfo_band == BANDID_2m) {
		sec_vfo_freq = ((int64_t)CALIBRATE.Transverter_2m_IF_MHz * HZ_IN_MHZ) + (sec_vfo_freq - (int64_t)CALIBRATE.Transverter_2m_RF_MHz * HZ_IN_MHZ);
	}
	if (TRX.Transverter_70cm && sec_vfo_band == BANDID_70cm) {
		sec_vfo_freq = ((int64_t)CALIBRATE.Transverter_70cm_IF_MHz * HZ_IN_MHZ) + (sec_vfo_freq - (int64_t)CALIBRATE.Transverter_70cm_RF_MHz * HZ_IN_MHZ);
	}
	if (TRX.Transverter_23cm && sec_vfo_band == BANDID_23cm) {
		sec_vfo_freq = ((int64_t)CALIBRATE.Transverter_23cm_IF_MHz * HZ_IN_MHZ) + (sec_vfo_freq - (int64_t)CALIBRATE.Transverter_23cm_RF_MHz * HZ_IN_MHZ);
	}
	if (TRX.Transverter_13cm && sec_vfo_band == BANDID_13cm) {
		sec_vfo_freq = ((int64_t)CALIBRATE.Transverter_13cm_IF_MHz * HZ_IN_MHZ) + (sec_vfo_freq - (int64_t)CALIBRATE.Transverter_13cm_RF_MHz * HZ_IN_MHZ);
	}
	if (TRX.Transverter_6cm && sec_vfo_band == BANDID_6cm) {
		sec_vfo_freq = ((int64_t)CALIBRATE.Transverter_6cm_IF_MHz * HZ_IN_MHZ) + (sec_vfo_freq - (int64_t)CALIBRATE.Transverter_6cm_RF_MHz * HZ_IN_MHZ);
	}
	if (TRX.Transverter_3cm && sec_vfo_band == BANDID_3cm) {
		sec_vfo_freq = ((int64_t)CALIBRATE.Transverter_3cm_IF_MHz * HZ_IN_MHZ) + (sec_vfo_freq - (int64_t)CALIBRATE.Transverter_3cm_RF_MHz * HZ_IN_MHZ);
	}
	if (TRX.Transverter_QO100 && sec_vfo_band == BANDID_QO100) {
		sec_vfo_freq = ((int64_t)CALIBRATE.Transverter_QO100_IF_RX_kHz * 1000) + (sec_vfo_freq - (int64_t)CALIBRATE.Transverter_QO100_RF_kHz * 1000);
	}
	if (TRX.Transverter_1_2cm && sec_vfo_band == BANDID_1_2cm) {
		sec_vfo_freq = ((int64_t)CALIBRATE.Transverter_1_2cm_IF_MHz * HZ_IN_MHZ) + (sec_vfo_freq - (int64_t)CALIBRATE.Transverter_1_2cm_RF_MHz * HZ_IN_MHZ);
	}

	int64_t vfo_tx_freq = CurrentVFO->Freq + (TRX.XIT_Enabled ? TRX_XIT : 0);
	TRX_MAX_TX_Amplitude = getMaxTXAmplitudeOnFreq(vfo_tx_freq);
	int8_t tx_vfo_band = getBandFromFreq(vfo_tx_freq, true);

	if (TRX.Transverter_2m && tx_vfo_band == BANDID_2m) {
		vfo_tx_freq = ((int64_t)CALIBRATE.Transverter_2m_IF_MHz * HZ_IN_MHZ) + (vfo_tx_freq - (int64_t)CALIBRATE.Transverter_2m_RF_MHz * HZ_IN_MHZ);
	}
	if (TRX.Transverter_70cm && tx_vfo_band == BANDID_70cm) {
		vfo_tx_freq = ((int64_t)CALIBRATE.Transverter_70cm_IF_MHz * HZ_IN_MHZ) + (vfo_tx_freq - (int64_t)CALIBRATE.Transverter_70cm_RF_MHz * HZ_IN_MHZ);
	}
	if (TRX.Transverter_23cm && tx_vfo_band == BANDID_23cm) {
		vfo_tx_freq = ((int64_t)CALIBRATE.Transverter_23cm_IF_MHz * HZ_IN_MHZ) + (vfo_tx_freq - (int64_t)CALIBRATE.Transverter_23cm_RF_MHz * HZ_IN_MHZ);
	}
	if (TRX.Transverter_13cm && tx_vfo_band == BANDID_13cm) {
		vfo_tx_freq = ((int64_t)CALIBRATE.Transverter_13cm_IF_MHz * HZ_IN_MHZ) + (vfo_tx_freq - (int64_t)CALIBRATE.Transverter_13cm_RF_MHz * HZ_IN_MHZ);
	}
	if (TRX.Transverter_6cm && tx_vfo_band == BANDID_6cm) {
		vfo_tx_freq = ((int64_t)CALIBRATE.Transverter_6cm_IF_MHz * HZ_IN_MHZ) + (vfo_tx_freq - (int64_t)CALIBRATE.Transverter_6cm_RF_MHz * HZ_IN_MHZ);
	}
	if (TRX.Transverter_3cm && tx_vfo_band == BANDID_3cm) {
		vfo_tx_freq = ((int64_t)CALIBRATE.Transverter_3cm_IF_MHz * HZ_IN_MHZ) + (vfo_tx_freq - (int64_t)CALIBRATE.Transverter_3cm_RF_MHz * HZ_IN_MHZ);
	}
	if (TRX.Transverter_QO100 && tx_vfo_band == BANDID_QO100) {
		vfo_tx_freq = ((int64_t)CALIBRATE.Transverter_QO100_IF_TX_MHz * HZ_IN_MHZ) + (vfo_tx_freq - (int64_t)CALIBRATE.Transverter_QO100_RF_kHz * 1000);
	}
	if (TRX.Transverter_1_2cm && tx_vfo_band == BANDID_1_2cm) {
		vfo_tx_freq = ((int64_t)CALIBRATE.Transverter_1_2cm_IF_MHz * HZ_IN_MHZ) + (vfo_tx_freq - (int64_t)CALIBRATE.Transverter_1_2cm_RF_MHz * HZ_IN_MHZ);
	}

	bool rx1_invert_iq_by_mixer = false;
	bool rx2_invert_iq_by_mixer = false;

#if HRDW_HAS_VHF_MIXER
	// delete if transverter chain needed in future
	cur_vfo_freq = CurrentVFO->SpectrumCenterFreq + (TRX.RIT_Enabled ? TRX_RIT : 0);
	vfo_tx_freq = CurrentVFO->Freq + (TRX.XIT_Enabled ? TRX_XIT : 0);
	sec_vfo_freq = SecondaryVFO->Freq + (TRX.RIT_Enabled ? TRX_RIT : 0);
	//

	uint64_t cur_vfo_freq_mhz = cur_vfo_freq / HZ_IN_MHZ;
	uint64_t sec_vfo_freq_mhz = sec_vfo_freq / HZ_IN_MHZ;

	if (cur_vfo_freq_mhz >= RFMIXER_MIN_FREQ_MHz && cur_vfo_freq_mhz <= RFMIXER_MAX_FREQ_MHz) {
		int64_t lo_freq = cur_vfo_freq + RFMIXER_IF_FREQ;
		int64_t result_lo_freq = RFMIXER_Freq_Set(lo_freq);
		int64_t lo_freq_diff = result_lo_freq - lo_freq;
		cur_vfo_freq = RFMIXER_IF_FREQ + lo_freq_diff;
		vfo_tx_freq = cur_vfo_freq;
		rx1_invert_iq_by_mixer = true;
	} else {
		RFMIXER_disable();
	}

	if (sec_vfo_freq_mhz >= RFMIXER_MIN_FREQ_MHz && sec_vfo_freq_mhz <= RFMIXER_MAX_FREQ_MHz) {
		rx2_invert_iq_by_mixer = true;
	}
#endif

	CurrentVFO->RXFreqAfterTransverters = cur_vfo_freq;
	TRX_freq_phrase = getRXPhraseFromFrequency(cur_vfo_freq, 1, rx1_invert_iq_by_mixer);
	SecondaryVFO->RXFreqAfterTransverters = sec_vfo_freq;
	TRX_freq_phrase2 = getRXPhraseFromFrequency(sec_vfo_freq, 2, rx2_invert_iq_by_mixer);
	TRX_freq_phrase_tx = getTXPhraseFromFrequency(vfo_tx_freq);

	// println("RX1: ", CurrentVFO->RealRXFreq, "TX: ", vfo_tx_freq);
	FPGA_NeedSendParams = true;

	// set DC-DC Sync freq
	uint64_t dcdc_0_harmonic_num = cur_vfo_freq / DCDC_FREQ_0;
	uint64_t dcdc_1_harmonic_num = cur_vfo_freq / DCDC_FREQ_1;
	uint64_t dcdc_0_harmonic_1ow = dcdc_0_harmonic_num * DCDC_FREQ_0;
	uint64_t dcdc_0_harmonic_high = (dcdc_0_harmonic_num + 1) * DCDC_FREQ_0;
	uint64_t dcdc_1_harmonic_1ow = dcdc_1_harmonic_num * DCDC_FREQ_1;
	uint64_t dcdc_1_harmonic_high = (dcdc_1_harmonic_num + 1) * DCDC_FREQ_1;
	uint64_t dcdc_0_harmonic_1ow_diff = llabs((int64_t)dcdc_0_harmonic_1ow - (int64_t)cur_vfo_freq);
	uint64_t dcdc_0_harmonic_high_diff = llabs((int64_t)dcdc_0_harmonic_high - (int64_t)cur_vfo_freq);
	uint64_t dcdc_1_harmonic_1ow_diff = llabs((int64_t)dcdc_1_harmonic_1ow - (int64_t)cur_vfo_freq);
	uint64_t dcdc_1_harmonic_high_diff = llabs((int64_t)dcdc_1_harmonic_high - (int64_t)cur_vfo_freq);
	uint64_t dcdc_0_harmonic_nearest = dcdc_0_harmonic_1ow_diff < dcdc_0_harmonic_high_diff ? dcdc_0_harmonic_1ow_diff : dcdc_0_harmonic_high_diff;
	uint64_t dcdc_1_harmonic_nearest = dcdc_1_harmonic_1ow_diff < dcdc_1_harmonic_high_diff ? dcdc_1_harmonic_1ow_diff : dcdc_1_harmonic_high_diff;

	if (dcdc_0_harmonic_nearest < dcdc_1_harmonic_nearest) {
		TRX_DCDC_Freq = 1;
	} else {
		TRX_DCDC_Freq = 0;
	}

	// services
	if (LCD_systemMenuOpened || SYSMENU_spectrum_opened || SYSMENU_swr_opened || SYSMENU_selftest_opened) {
		return;
	}

	// get band
	int_fast8_t bandFromFreq = getBandFromFreq(_freq, false);
	if (bandFromFreq >= 0) {
		TRX.BANDS_SAVED_SETTINGS[bandFromFreq].Freq = _freq;

		if (vfo == CurrentVFO) {
			TRX_LoadRFGain_Data(CurrentVFO->Mode, bandFromFreq);
		}
	}
	if (TRX.BandMapEnabled && !TRX_Temporary_Stop_BandMap) {
		uint_fast8_t mode_from_bandmap = getModeFromFreq(vfo->Freq);
		if (vfo->Mode != mode_from_bandmap) {
			TRX_setMode(mode_from_bandmap, vfo);
			if (bandFromFreq >= 0) {
				TRX.BANDS_SAVED_SETTINGS[bandFromFreq].Mode = mode_from_bandmap;
			}
			LCD_UpdateQuery.TopButtons = true;
		}
	}

	// Restore band settings
	if (bandFromFreq >= 0 && bandFromOldFreq != bandFromFreq) {
		TRX_RestoreBandSettings(bandFromFreq);
	}

	// Get ATU settings
	ATU_Load_Memory(TRX_on_TX ? TRX.ANT_TX : TRX.ANT_RX, CurrentVFO->Freq);

	// SPLIT freq secondary VFO sync
	if (TRX.Split_Mode_Sync_Freq && TRX.SPLIT_Enabled && vfo == CurrentVFO) {
		TRX_setFrequency(SecondaryVFO->Freq + freq_diff, SecondaryVFO);
	}

	LCD_UpdateQuery.FreqInfo = true;
}

void TRX_setTXFrequencyFloat(float64_t _freq, VFO *vfo) {
	TRX_setFrequency((uint32_t)_freq, vfo);
	TRX_freq_phrase_tx = getTXPhraseFromFrequency(_freq);
	FPGA_NeedSendParams = true;
}

void TRX_setMode(TRX_MODE _mode, VFO *vfo) {
	TRX_MODE old_mode = vfo->Mode;

	// save old mode data
	int_fast8_t bandFromFreq = getBandFromFreq(vfo->Freq, false);
	TRX_SaveRFGain_Data(old_mode, bandFromFreq);

	// switch mode
	vfo->Mode = _mode;
	if (vfo->Mode == TRX_MODE_LOOPBACK) {
		TRX_Start_TXRX();
	}

	// get new mode filters
	switch (_mode) {
	case TRX_MODE_AM:
	case TRX_MODE_SAM_STEREO:
	case TRX_MODE_SAM_LSB:
	case TRX_MODE_SAM_USB:
		vfo->LPF_RX_Filter_Width = vfo->AM_LPF_RX_Filter;
		vfo->LPF_TX_Filter_Width = vfo->AM_LPF_TX_Filter;
		vfo->HPF_RX_Filter_Width = 0;
		vfo->HPF_TX_Filter_Width = 0;
		break;
	case TRX_MODE_LSB:
	case TRX_MODE_USB:
		vfo->LPF_RX_Filter_Width = vfo->SSB_LPF_RX_Filter;
		vfo->LPF_TX_Filter_Width = vfo->SSB_LPF_TX_Filter;
		vfo->HPF_RX_Filter_Width = vfo->SSB_HPF_RX_Filter;
		vfo->HPF_TX_Filter_Width = vfo->SSB_HPF_TX_Filter;
		break;
	case TRX_MODE_DIGI_L:
	case TRX_MODE_DIGI_U:
		vfo->LPF_RX_Filter_Width = vfo->DIGI_LPF_Filter;
		vfo->LPF_TX_Filter_Width = vfo->DIGI_LPF_Filter;
		vfo->HPF_RX_Filter_Width = vfo->DIGI_HPF_Filter;
		vfo->HPF_TX_Filter_Width = vfo->DIGI_HPF_Filter;
		break;
	case TRX_MODE_RTTY:
		vfo->LPF_RX_Filter_Width = TRX.RTTY_Freq + TRX.RTTY_Shift * 2;
		vfo->LPF_TX_Filter_Width = vfo->LPF_RX_Filter_Width;
		vfo->HPF_RX_Filter_Width = TRX.RTTY_Freq - TRX.RTTY_Shift * 2;
		vfo->HPF_TX_Filter_Width = 0;
		break;
	case TRX_MODE_CW:
		vfo->LPF_RX_Filter_Width = vfo->CW_LPF_Filter;
		vfo->LPF_TX_Filter_Width = vfo->CW_LPF_Filter;
		vfo->HPF_RX_Filter_Width = 0;
		vfo->HPF_TX_Filter_Width = 0;
		LCD_UpdateQuery.TextBar = true;
		break;
	case TRX_MODE_NFM:
		vfo->LPF_RX_Filter_Width = vfo->FM_LPF_RX_Filter;
		vfo->LPF_TX_Filter_Width = vfo->FM_LPF_TX_Filter;
		vfo->HPF_RX_Filter_Width = vfo->FM_HPF_RX_Filter;
		vfo->HPF_TX_Filter_Width = 0;
		break;
	case TRX_MODE_LOOPBACK:
	case TRX_MODE_IQ:
	case TRX_MODE_WFM:
		vfo->LPF_RX_Filter_Width = 16000;
		vfo->LPF_TX_Filter_Width = 16000;
		vfo->HPF_RX_Filter_Width = 0;
		vfo->HPF_TX_Filter_Width = 0;
		break;
	}

	// get new mode data
	TRX_LoadRFGain_Data(_mode, bandFromFreq);

	// FFT Zoom change
	if (TRX.FFT_Zoom != TRX.FFT_ZoomCW) {
		if (old_mode == TRX_MODE_CW && _mode != TRX_MODE_CW) {
			NeedFFTReinit = true;
		}
		if (old_mode != TRX_MODE_CW && _mode == TRX_MODE_CW) {
			NeedFFTReinit = true;
		}
	}

	// FM Samplerate change
	if (TRX.SAMPLERATE_MAIN != TRX.SAMPLERATE_FM && old_mode != TRX_MODE_WFM && old_mode != TRX_MODE_NFM && (_mode == TRX_MODE_WFM || _mode == TRX_MODE_NFM)) {
		NeedFFTReinit = true;
	}
	if (TRX.SAMPLERATE_MAIN != TRX.SAMPLERATE_FM && (old_mode == TRX_MODE_WFM || old_mode == TRX_MODE_NFM) && _mode != TRX_MODE_WFM && _mode != TRX_MODE_NFM) {
		NeedFFTReinit = true;
	}

	if (old_mode != _mode) {
		TRX_TemporaryMute();
		NeedReinitAudioFiltersClean = true;
	}

	CODEC_TXRX_mode();
	FRONTPANEL_ENC2SW_validate();

	NeedReinitNotch = true;
	NeedReinitAudioFilters = true;
	NeedSaveSettings = true;
	LCD_UpdateQuery.StatusInfoBar = true;
	LCD_UpdateQuery.StatusInfoGUI = true;
	NeedWTFRedraw = true;
}

void TRX_SaveRFGain_Data(TRX_MODE mode, int8_t band) {
	switch (mode) {
	case TRX_MODE_AM:
	case TRX_MODE_SAM_STEREO:
	case TRX_MODE_SAM_LSB:
	case TRX_MODE_SAM_USB:
		TRX.RF_Gain_By_Mode_AM = TRX.RF_Gain;
		break;
	case TRX_MODE_LSB:
	case TRX_MODE_USB:
	case TRX_MODE_LOOPBACK:
		TRX.RF_Gain_By_Mode_SSB = TRX.RF_Gain;
		break;
	case TRX_MODE_DIGI_L:
	case TRX_MODE_DIGI_U:
	case TRX_MODE_RTTY:
	case TRX_MODE_IQ:
		TRX.RF_Gain_By_Mode_DIGI = TRX.RF_Gain;
		break;
	case TRX_MODE_CW:
		TRX.RF_Gain_By_Mode_CW = TRX.RF_Gain;
		break;
	case TRX_MODE_NFM:
	case TRX_MODE_WFM:
		TRX.RF_Gain_By_Mode_FM = TRX.RF_Gain;
		break;
	}

	if (band >= 0) {
		TRX.BANDS_SAVED_SETTINGS[band].RF_Gain = TRX.RF_Gain;

		switch (mode) {
		case TRX_MODE_AM:
		case TRX_MODE_SAM_STEREO:
		case TRX_MODE_SAM_LSB:
		case TRX_MODE_SAM_USB:
			TRX.BANDS_SAVED_SETTINGS[band].RF_Gain_By_Mode_AM = TRX.RF_Gain;
			break;
		case TRX_MODE_LSB:
		case TRX_MODE_USB:
		case TRX_MODE_LOOPBACK:
			TRX.BANDS_SAVED_SETTINGS[band].RF_Gain_By_Mode_SSB = TRX.RF_Gain;
			break;
		case TRX_MODE_DIGI_L:
		case TRX_MODE_DIGI_U:
		case TRX_MODE_RTTY:
		case TRX_MODE_IQ:
			TRX.BANDS_SAVED_SETTINGS[band].RF_Gain_By_Mode_DIGI = TRX.RF_Gain;
			break;
		case TRX_MODE_CW:
			TRX.BANDS_SAVED_SETTINGS[band].RF_Gain_By_Mode_CW = TRX.RF_Gain;
			break;
		case TRX_MODE_NFM:
		case TRX_MODE_WFM:
			TRX.BANDS_SAVED_SETTINGS[band].RF_Gain_By_Mode_FM = TRX.RF_Gain;
			break;
		}
	}
}

void TRX_LoadRFGain_Data(TRX_MODE mode, int8_t band) {
	if (TRX.RF_Gain_For_Each_Mode) {
		switch (mode) {
		case TRX_MODE_AM:
		case TRX_MODE_SAM_STEREO:
		case TRX_MODE_SAM_LSB:
		case TRX_MODE_SAM_USB:
			TRX.RF_Gain = TRX.RF_Gain_By_Mode_AM;
			break;
		case TRX_MODE_LSB:
		case TRX_MODE_USB:
		case TRX_MODE_LOOPBACK:
			TRX.RF_Gain = TRX.RF_Gain_By_Mode_SSB;
			break;
		case TRX_MODE_DIGI_L:
		case TRX_MODE_DIGI_U:
		case TRX_MODE_RTTY:
		case TRX_MODE_IQ:
			TRX.RF_Gain = TRX.RF_Gain_By_Mode_DIGI;
			break;
		case TRX_MODE_CW:
			TRX.RF_Gain = TRX.RF_Gain_By_Mode_CW;
			break;
		case TRX_MODE_NFM:
		case TRX_MODE_WFM:
			TRX.RF_Gain = TRX.RF_Gain_By_Mode_FM;
			break;
		}
	}

	if (TRX.RF_Gain_For_Each_Band) {
		TRX.RF_Gain = TRX.BANDS_SAVED_SETTINGS[band].RF_Gain;
	}

	if (TRX.RF_Gain_For_Each_Mode && TRX.RF_Gain_For_Each_Band && band >= 0) {
		switch (mode) {
		case TRX_MODE_AM:
		case TRX_MODE_SAM_STEREO:
		case TRX_MODE_SAM_LSB:
		case TRX_MODE_SAM_USB:
			TRX.RF_Gain = TRX.BANDS_SAVED_SETTINGS[band].RF_Gain_By_Mode_AM;
			break;
		case TRX_MODE_LSB:
		case TRX_MODE_USB:
		case TRX_MODE_LOOPBACK:
			TRX.RF_Gain = TRX.BANDS_SAVED_SETTINGS[band].RF_Gain_By_Mode_SSB;
			break;
		case TRX_MODE_DIGI_L:
		case TRX_MODE_DIGI_U:
		case TRX_MODE_RTTY:
		case TRX_MODE_IQ:
			TRX.RF_Gain = TRX.BANDS_SAVED_SETTINGS[band].RF_Gain_By_Mode_DIGI;
			break;
		case TRX_MODE_CW:
			TRX.RF_Gain = TRX.BANDS_SAVED_SETTINGS[band].RF_Gain_By_Mode_CW;
			break;
		case TRX_MODE_NFM:
		case TRX_MODE_WFM:
			TRX.RF_Gain = TRX.BANDS_SAVED_SETTINGS[band].RF_Gain_By_Mode_FM;
			break;
		}
	}
}

void TRX_RestoreBandSettings(int8_t band) {
	DPD_Init();

	TRX.IF_Gain = TRX.BANDS_SAVED_SETTINGS[band].IF_Gain;
	TRX.LNA = TRX.BANDS_SAVED_SETTINGS[band].LNA;
	TRX.ATT = TRX.BANDS_SAVED_SETTINGS[band].ATT;
	if (TRX.ANT_RX != TRX.BANDS_SAVED_SETTINGS[band].ANT_RX || TRX.ANT_TX != TRX.BANDS_SAVED_SETTINGS[band].ANT_TX) {
		LCD_UpdateQuery.StatusInfoGUI = true;
	}
	TRX.ANT_RX = TRX.BANDS_SAVED_SETTINGS[band].ANT_RX;
	TRX.ANT_TX = TRX.BANDS_SAVED_SETTINGS[band].ANT_TX;
	TRX.ATT_DB = TRX.BANDS_SAVED_SETTINGS[band].ATT_DB;
	TRX.ADC_Driver = TRX.BANDS_SAVED_SETTINGS[band].ADC_Driver;
	TRX.ADC_PGA = TRX.BANDS_SAVED_SETTINGS[band].ADC_PGA;
	TRX.Fast = TRX.BANDS_SAVED_SETTINGS[band].Fast;
	ATU_Load_Memory(TRX_on_TX ? TRX.ANT_TX : TRX.ANT_RX, CurrentVFO->Freq);

	if (CurrentVFO == &TRX.VFO_A) {
		CurrentVFO->CW_LPF_Filter = TRX.BANDS_SAVED_SETTINGS[band].VFO_A_CW_LPF_Filter;
		CurrentVFO->SSB_LPF_RX_Filter = TRX.BANDS_SAVED_SETTINGS[band].VFO_A_SSB_LPF_RX_Filter;
		CurrentVFO->AM_LPF_RX_Filter = TRX.BANDS_SAVED_SETTINGS[band].VFO_A_AM_LPF_RX_Filter;
		CurrentVFO->FM_LPF_RX_Filter = TRX.BANDS_SAVED_SETTINGS[band].VFO_A_FM_LPF_RX_Filter;
	} else {
		CurrentVFO->CW_LPF_Filter = TRX.BANDS_SAVED_SETTINGS[band].VFO_B_CW_LPF_Filter;
		CurrentVFO->SSB_LPF_RX_Filter = TRX.BANDS_SAVED_SETTINGS[band].VFO_B_SSB_LPF_RX_Filter;
		CurrentVFO->AM_LPF_RX_Filter = TRX.BANDS_SAVED_SETTINGS[band].VFO_B_AM_LPF_RX_Filter;
		CurrentVFO->FM_LPF_RX_Filter = TRX.BANDS_SAVED_SETTINGS[band].VFO_B_FM_LPF_RX_Filter;
	}

	CurrentVFO->FM_SQL_threshold_dBm = TRX.BANDS_SAVED_SETTINGS[band].FM_SQL_threshold_dBm;
	CurrentVFO->DNR_Type = TRX.BANDS_SAVED_SETTINGS[band].DNR_Type;
	CurrentVFO->AGC = TRX.BANDS_SAVED_SETTINGS[band].AGC;
	CurrentVFO->RepeaterMode = TRX.BANDS_SAVED_SETTINGS[band].RepeaterMode;
	CurrentVFO->SQL = TRX.BANDS_SAVED_SETTINGS[band].SQL;

	TRX.FM_SQL_threshold_dBm_shadow = TRX.BANDS_SAVED_SETTINGS[band].FM_SQL_threshold_dBm;
	TRX.SQL_shadow = TRX.BANDS_SAVED_SETTINGS[band].SQL;
	TRX.AGC_shadow = TRX.BANDS_SAVED_SETTINGS[band].AGC;
	TRX.RepeaterMode_shadow = TRX.BANDS_SAVED_SETTINGS[band].RepeaterMode;
	TRX.CW_LPF_Filter_shadow = CurrentVFO->CW_LPF_Filter;
	TRX.DIGI_LPF_Filter_shadow = CurrentVFO->DIGI_LPF_Filter;
	TRX.DIGI_HPF_Filter_shadow = CurrentVFO->DIGI_HPF_Filter;
	TRX.SSB_LPF_RX_Filter_shadow = CurrentVFO->SSB_LPF_RX_Filter;
	TRX.SSB_LPF_TX_Filter_shadow = CurrentVFO->SSB_LPF_TX_Filter;
	TRX.SSB_HPF_RX_Filter_shadow = CurrentVFO->SSB_HPF_RX_Filter;
	TRX.SSB_HPF_TX_Filter_shadow = CurrentVFO->SSB_HPF_TX_Filter;
	TRX.AM_LPF_RX_Filter_shadow = CurrentVFO->AM_LPF_RX_Filter;
	TRX.AM_LPF_TX_Filter_shadow = CurrentVFO->AM_LPF_TX_Filter;
	TRX.FM_LPF_RX_Filter_shadow = CurrentVFO->FM_LPF_RX_Filter;
	TRX.FM_LPF_TX_Filter_shadow = CurrentVFO->FM_LPF_TX_Filter;
	TRX.FM_HPF_RX_Filter_shadow = CurrentVFO->FM_HPF_RX_Filter;

	TRX_Temporary_Stop_BandMap = false;
	TRX_DXCluster_UpdateTime = 0;
	TRX_WOLF_Cluster_UpdateTime = 0;

	LCD_UpdateQuery.TopButtons = true;
}

void TRX_DoAutoGain(void) {
	static uint8_t skip_cycles = 0;
	if (skip_cycles > 0) {
		skip_cycles--;
		return;
	}

	// Process AutoGain feature
	if (TRX.AutoGain && TRX_on_RX) {
		if (!TRX.ATT) {
			TRX.ATT = true;
			LCD_UpdateQuery.TopButtons = true;
		}

		int32_t max_amplitude = abs(TRX_ADC_MAXAMPLITUDE);
		if (abs(TRX_ADC_MINAMPLITUDE) > max_amplitude) {
			max_amplitude = abs(TRX_ADC_MINAMPLITUDE);
		}

		float32_t new_att_val = TRX.ATT_DB;
		if (new_att_val < 0.5f) {
			new_att_val = 0.5f;
		}
		if (max_amplitude > (AUTOGAINER_TAGET + AUTOGAINER_HYSTERESIS) && new_att_val < 31.5f) {
			new_att_val += 0.5f;
		} else if (max_amplitude < (AUTOGAINER_TAGET - AUTOGAINER_HYSTERESIS) && new_att_val > 0.5f) {
			new_att_val -= 0.5f;
		}

#ifndef FRONTPANEL_LITE
		if (new_att_val <= 0.5f && max_amplitude < (AUTOGAINER_TAGET - AUTOGAINER_HYSTERESIS) && !TRX.ADC_Driver) {
			TRX.ADC_Driver = !TRX_on_TX;
			LCD_UpdateQuery.TopButtons = true;
			skip_cycles = 5;
		} else if (new_att_val <= 0.5f && max_amplitude < (AUTOGAINER_TAGET - AUTOGAINER_HYSTERESIS) && !TRX.ADC_PGA) {
			TRX.ADC_PGA = !TRX_on_TX;
			LCD_UpdateQuery.TopButtons = true;
			skip_cycles = 5;
		}
#endif

		if (TRX_on_TX) {
			TRX.ADC_Driver = !TRX_on_TX;
			TRX.ADC_PGA = !TRX_on_TX;
			LCD_UpdateQuery.TopButtons = true;
		}

		if (new_att_val != TRX.ATT_DB) {
			TRX.ATT_DB = new_att_val;
			LCD_UpdateQuery.TopButtons = true;
			// save settings
			int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
			TRX.BANDS_SAVED_SETTINGS[band].ATT_DB = TRX.ATT_DB;
			TRX.BANDS_SAVED_SETTINGS[band].ADC_Driver = TRX.ADC_Driver;
			TRX.BANDS_SAVED_SETTINGS[band].ADC_PGA = TRX.ADC_PGA;
		}
	}
}

void TRX_TemporaryMute(void) {
	CODEC_Mute();
	TRX_Temporary_Mute_StartTime = HAL_GetTick();
}

// process frequency scanner
void TRX_ProcessScanMode(void) {
	static bool oldState = false;
	static uint32_t StateChangeTime = 0;
	bool goSweep = false;

	uint32_t noSignalTime = TRX.ENC2_func_mode == ENC_FUNC_SET_MEM ? SCANNER_NOSIGNAL_TIME_MEM : SCANNER_NOSIGNAL_TIME;

	if (CurrentVFO->Mode == TRX_MODE_WFM || CurrentVFO->Mode == TRX_MODE_NFM) {
		if (oldState != DFM_RX1.squelched) {
			oldState = DFM_RX1.squelched;
			StateChangeTime = HAL_GetTick();
		}

		if (DFM_RX1.squelched && ((HAL_GetTick() - StateChangeTime) > noSignalTime)) {
			goSweep = true;
		}
		if (!DFM_RX1.squelched && ((HAL_GetTick() - StateChangeTime) > SCANNER_SIGNAL_TIME_FM)) {
			goSweep = true;
		}
	} else {
		if (oldState != VAD_RX1_Muting) {
			oldState = VAD_RX1_Muting;
			StateChangeTime = HAL_GetTick();
		}

		if (VAD_RX1_Muting && ((HAL_GetTick() - StateChangeTime) > noSignalTime)) {
			goSweep = true;
		}
		if (!VAD_RX1_Muting && ((HAL_GetTick() - StateChangeTime) > SCANNER_SIGNAL_TIME_OTHER)) {
			goSweep = true;
		}
	}

	if (goSweep) {
		int8_t band = getBandFromFreq(CurrentVFO->Freq, false);

		if (TRX.ChannelMode && band != -1 && BANDS[band].channelsCount > 0) // channel mode
		{
			int_fast16_t channel = getChannelbyFreq(CurrentVFO->Freq, false);
			int_fast16_t new_channel = channel + 1;
			if (new_channel < 0) {
				new_channel = BANDS[band].channelsCount - 1;
			}
			if (new_channel >= BANDS[band].channelsCount) {
				new_channel = 0;
			}

			float64_t newfreq = BANDS[band].channels[new_channel].rxFreq;
			TRX_setFrequency(newfreq, CurrentVFO);

			TRX.SPLIT_Enabled = (BANDS[band].channels[new_channel].rxFreq != BANDS[band].channels[new_channel].txFreq);
			if (TRX.SPLIT_Enabled) {
				TRX_setFrequency(BANDS[band].channels[new_channel].txFreq, SecondaryVFO);
			}

			LCD_UpdateQuery.FreqInfoRedraw = true;
			// LCD_UpdateQuery.StatusInfoGUI = true;
			// LCD_UpdateQuery.StatusInfoBarRedraw = true;
			StateChangeTime = HAL_GetTick();
		} else if (TRX.ENC2_func_mode == ENC_FUNC_SET_MEM) { // memory channels mode
			int8_t currentMemIndex = 0;
			for (uint8_t i = 0; i < MEMORY_CHANNELS_COUNT; i++) {
				if (CurrentVFO->Freq == CALIBRATE.MEMORY_CHANNELS[i].freq) {
					currentMemIndex = i;
				}
			}
			currentMemIndex++;
			if (currentMemIndex >= MEMORY_CHANNELS_COUNT) {
				currentMemIndex = 0;
			}
			while (currentMemIndex < MEMORY_CHANNELS_COUNT && currentMemIndex > 0 && CALIBRATE.MEMORY_CHANNELS[currentMemIndex].freq == 0) {
				currentMemIndex++;
			}
			if (currentMemIndex >= MEMORY_CHANNELS_COUNT) {
				currentMemIndex = 0;
			}

			BUTTONHANDLER_SelectMemoryChannels(currentMemIndex);
			StateChangeTime = HAL_GetTick();
		} else if (band != -1) // common region mode
		{
			for (uint8_t region_id = 0; region_id < BANDS[band].regionsCount; region_id++) {
				if ((BANDS[band].regions[region_id].startFreq <= CurrentVFO->Freq) && (BANDS[band].regions[region_id].endFreq > CurrentVFO->Freq)) {
					float64_t step = SCANNER_FREQ_STEP_OTHER;
					if (CurrentVFO->Mode == TRX_MODE_WFM) {
						step = SCANNER_FREQ_STEP_WFM;
					}
					if (CurrentVFO->Mode == TRX_MODE_NFM) {
						step = SCANNER_FREQ_STEP_NFM;
					}

					float64_t new_freq = (float64_t)(CurrentVFO->Freq + step) / step * step;
					if (new_freq >= BANDS[band].regions[region_id].endFreq) {
						new_freq = BANDS[band].regions[region_id].startFreq;
					}

					TRX_setFrequency(new_freq, CurrentVFO);
					LCD_UpdateQuery.FreqInfo = true;
					StateChangeTime = HAL_GetTick();
					break;
				}
			}
		}
	}
}

static uint64_t setFreqSlowly_target = 0;
static bool setFreqSlowly_processing = 0;
void TRX_setFrequencySlowly(uint64_t target_freq) {
	setFreqSlowly_target = target_freq;
	setFreqSlowly_processing = true;
}

void TRX_setFrequencySlowly_Process(void) {
	if (!setFreqSlowly_processing) {
		return;
	}

	VFO *vfo = TRX.SPLIT_Enabled ? SecondaryVFO : CurrentVFO;

	int64_t diff = vfo->Freq - setFreqSlowly_target;
	if (diff > TRX_SLOW_SETFREQ_MIN_STEPSIZE || diff < -TRX_SLOW_SETFREQ_MIN_STEPSIZE) {
		TRX_setFrequency(vfo->Freq - diff / 4, vfo);
		LCD_UpdateQuery.FreqInfo = true;
	} else {
		TRX_setFrequency(setFreqSlowly_target, vfo);
		LCD_UpdateQuery.FreqInfo = true;
		setFreqSlowly_processing = false;
	}
}

void TRX_DoFrequencyEncoder(float32_t direction, bool secondary_encoder) {
	float64_t newfreq = (float64_t)CurrentVFO->Freq;
	float64_t step;

	if (TRX.ChannelMode && getBandFromFreq(CurrentVFO->Freq, false) != -1 && BANDS[getBandFromFreq(CurrentVFO->Freq, false)].channelsCount > 0) {
		int_fast8_t band = getBandFromFreq(CurrentVFO->Freq, false);
		int_fast16_t channel = getChannelbyFreq(CurrentVFO->Freq, false);
		int_fast16_t new_channel = channel + direction;
		if (new_channel < 0) {
			new_channel = BANDS[band].channelsCount - 1;
		}
		if (new_channel >= BANDS[band].channelsCount) {
			new_channel = 0;
		}

		newfreq = BANDS[band].channels[new_channel].rxFreq;
		TRX.SPLIT_Enabled = (BANDS[band].channels[new_channel].rxFreq != BANDS[band].channels[new_channel].txFreq);
		if (TRX.SPLIT_Enabled) {
			TRX_setFrequency(BANDS[band].channels[new_channel].txFreq, SecondaryVFO);
		}
		LCD_UpdateQuery.FreqInfoRedraw = true;
		LCD_UpdateQuery.StatusInfoGUI = true;
		LCD_UpdateQuery.StatusInfoBarRedraw = true;
	} else {
		bool air_step = false;
		step = TRX.FRQ_STEP_SSB_Hz;
		if (CurrentVFO->Mode == TRX_MODE_CW) {
			step = (float64_t)TRX.FRQ_STEP_CW_Hz;
		}
		if (CurrentVFO->Mode == TRX_MODE_DIGI_L || CurrentVFO->Mode == TRX_MODE_DIGI_U || CurrentVFO->Mode == TRX_MODE_RTTY) {
			step = (float64_t)TRX.FRQ_STEP_DIGI_Hz;
		}
		if (CurrentVFO->Mode == TRX_MODE_WFM) {
			step = (float64_t)TRX.FRQ_STEP_WFM_Hz;
		}
		if (CurrentVFO->Mode == TRX_MODE_NFM) {
			step = (float64_t)TRX.FRQ_STEP_FM_Hz;
		}
		if (CurrentVFO->Mode == TRX_MODE_AM || CurrentVFO->Mode == TRX_MODE_SAM_STEREO || CurrentVFO->Mode == TRX_MODE_SAM_LSB || CurrentVFO->Mode == TRX_MODE_SAM_USB) {
			step = (float64_t)TRX.FRQ_STEP_AM_Hz;
			air_step = step == 8333;
		}

		if (TRX.Fast) {
			step *= (float64_t)TRX.FAST_STEP_Multiplier;
		}

		if (secondary_encoder) {
			step *= (float64_t)TRX.ENC2_STEP_Multiplier;
		}

		step = roundl(step);
		if (step < 1.0) {
			step = 1.0;
		}

		if (air_step) {
			step = 8333;
		}

		if (direction == -1.0f && !air_step) {
			newfreq = ceill(newfreq / step) * step;
		}
		if (direction == 1.0f && !air_step) {
			newfreq = floorl(newfreq / step) * step;
		}

		newfreq = newfreq + step * (float64_t)direction;
		if (air_step) {
			float64_t mod = fmodl(newfreq, 1000);
			if (mod == 999) {
				newfreq += 1;
			}
			if (mod == 334 || mod == 667 || mod == 1) {
				newfreq -= 1;
			}
		}
	}

	TRX_setFrequency(newfreq, CurrentVFO);
}

/// BUTTON HANDLERS ///

void BUTTONHANDLER_DOUBLE(uint32_t parameter) {
#if HRDW_HAS_DUAL_RX
	TRX.Dual_RX = !TRX.Dual_RX;
	FPGA_NeedSendParams = true;
	LCD_UpdateQuery.StatusInfoGUI = true;
	NeedReinitAudioFilters = true;
#endif

	LCD_UpdateQuery.TopButtons = true;
}

void BUTTONHANDLER_DOUBLEMODE(uint32_t parameter) {
#if HRDW_HAS_DUAL_RX
	if (!TRX.Dual_RX) {
		return;
	}

	if (TRX.Dual_RX_Type == VFO_A_AND_B) {
		TRX.Dual_RX_Type = VFO_A_PLUS_B;
	} else {
		TRX.Dual_RX_Type = VFO_A_AND_B;
	}
	LCD_UpdateQuery.StatusInfoGUI = true;
	NeedReinitAudioFilters = true;
#endif

	LCD_UpdateQuery.TopButtons = true;
}

void BUTTONHANDLER_PTT_TOGGLE(uint32_t parameter) { TRX_ptt_soft = !TRX_ptt_soft; }

void BUTTONHANDLER_AsB(uint32_t parameter) // A<>B
{
	// Block for LITE and MINI in MEMO mode
#if defined(FRONTPANEL_MINI) || defined(FRONTPANEL_FRONTPANEL_LITE)
	if (TRX.ENC2_func_mode == ENC_FUNC_SET_MEM) {
		return;
	}
#endif

	// TX block
	if (TRX_on_TX) {
		return;
	}

	TRX_TemporaryMute();

	TRX.selected_vfo = !TRX.selected_vfo;
	// VFO settings
	if (!TRX.selected_vfo) {
		CurrentVFO = &TRX.VFO_A;
		SecondaryVFO = &TRX.VFO_B;
	} else {
		CurrentVFO = &TRX.VFO_B;
		SecondaryVFO = &TRX.VFO_A;
	}

	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	if (band != -1) {
		TRX_LoadRFGain_Data(CurrentVFO->Mode, band);
	}

	TRX_setFrequency(CurrentVFO->Freq, CurrentVFO);
	TRX_setMode(CurrentVFO->Mode, CurrentVFO);
	if (band != -1) {
		TRX_RestoreBandSettings(band);
	}

	TRX_DXCluster_UpdateTime = 0;
	TRX_WOLF_Cluster_UpdateTime = 0;
	LCD_UpdateQuery.TopButtons = true;
	LCD_UpdateQuery.BottomButtons = true;
	LCD_UpdateQuery.FreqInfoRedraw = true;
	LCD_UpdateQuery.StatusInfoGUI = true;
	LCD_UpdateQuery.StatusInfoBarRedraw = true;
	NeedSaveSettings = true;
	NeedReinitAudioFiltersClean = true;
	NeedReinitAudioFilters = true;
	resetVAD();
	FFT_Init();
	TRX_ScanMode = false;
}

void BUTTONHANDLER_TUNE(uint32_t parameter) {
	if (parameter == 255 && !TRX_Tune) { // only tune off mode
		return;
	}

	if (!TRX_Tune) { // go TUNE
		APROC_TX_tune_power = 0.0f;
		ATU_Load_Memory(TRX.ANT_TX, CurrentVFO->Freq);
		ATU_Invalidate();
		ATU_TunePowerStabilized = false;
		LCD_UpdateQuery.StatusInfoBar = true;
	} else { // go RX
		ATU_Load_Memory(TRX.ANT_RX, CurrentVFO->Freq);
	}

	TRX_Tune = !TRX_Tune;
	TRX_ptt_hard = TRX_Tune;

	if (TRX_TX_Disabled(CurrentVFO->Freq)) {
		TRX_Tune = false;
		TRX_ptt_hard = false;
	}

	LCD_UpdateQuery.StatusInfoGUIRedraw = true;
	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
	TRX_Restart_Mode();
}

void BUTTONHANDLER_TUNER(uint32_t parameter) {
	TRX.TUNER_Enabled = !TRX.TUNER_Enabled;

	if (TRX.TUNER_Enabled) {
		LCD_showTooltip("Tuner ON");
	} else {
		LCD_showTooltip("Tuner OFF");
	}

	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
}

void BUTTONHANDLER_PRE(uint32_t parameter) {
	TRX.LNA = !TRX.LNA;
	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	if (band >= 0) {
		TRX.BANDS_SAVED_SETTINGS[band].LNA = TRX.LNA;
	}
	LCD_UpdateQuery.TopButtons = true;
	FPGA_NeedSendParams = true;
	NeedSaveSettings = true;
	resetVAD();
}

void BUTTONHANDLER_MEMO_WRITE(uint32_t parameter) {
	int8_t channel = TRX_MemoryChannelSelected;
	if (TRX.ENC2_func_mode == ENC_FUNC_SET_MEM) {
		BUTTONHANDLER_SaveMemoryChannels(channel);
		LCD_UpdateQuery.FreqInfoRedraw = true;
	}
}
void BUTTONHANDLER_MEMO_READ(uint32_t parameter) {
	int8_t channel = TRX_MemoryChannelSelected;
	if (TRX.ENC2_func_mode == ENC_FUNC_SET_MEM) {
		BUTTONHANDLER_SelectMemoryChannels(channel);
		LCD_showTooltip("Channel read");
		LCD_UpdateQuery.FreqInfoRedraw = true;
	}
}

void BUTTONHANDLER_ATT(uint32_t parameter) {
	if (TRX.ATT && TRX.ATT_DB < 1.0f) {
		TRX.ATT_DB = TRX.ATT_STEP;
	} else {
		TRX.ATT = !TRX.ATT;
	}

	if (TRX.ATT_DB < 1.0f) {
		TRX.ATT_DB = TRX.ATT_STEP;
	}

	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	if (band >= 0) {
		TRX.BANDS_SAVED_SETTINGS[band].ATT = TRX.ATT;
		TRX.BANDS_SAVED_SETTINGS[band].ATT_DB = TRX.ATT_DB;
	}

	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
	resetVAD();
}

void BUTTONHANDLER_ATTHOLD(uint32_t parameter) {
	TRX.ATT_DB += TRX.ATT_STEP;
	if (TRX.ATT_DB > 31.0f) {
		TRX.ATT_DB = TRX.ATT_STEP;
	}

	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	if (band >= 0) {
		TRX.BANDS_SAVED_SETTINGS[band].ATT = TRX.ATT;
		TRX.BANDS_SAVED_SETTINGS[band].ATT_DB = TRX.ATT_DB;
	}

	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
	resetVAD();
}

void BUTTONHANDLER_ANT(uint32_t parameter) {
	if (TRX.ANT_RX != TRX.ANT_TX) {
		TRX.ANT_RX = TRX_ANT_1;
		TRX.ANT_TX = TRX.ANT_RX;
	} else {
		TRX.ANT_RX++;
		TRX.ANT_TX = TRX.ANT_RX;

		if (TRX.ANT_RX >= HARDW_GetAntCount()) {
			TRX.ANT_RX = TRX_ANT_1;
			TRX.ANT_TX = TRX_ANT_2;
		}
	}

	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	if (band >= 0) {
		TRX.BANDS_SAVED_SETTINGS[band].ANT_RX = TRX.ANT_RX;
		TRX.BANDS_SAVED_SETTINGS[band].ANT_TX = TRX.ANT_TX;
	}
	ATU_Load_Memory(TRX_on_TX ? TRX.ANT_TX : TRX.ANT_RX, CurrentVFO->Freq);

	LCD_UpdateQuery.StatusInfoGUI = true;
	NeedSaveSettings = true;
}

void BUTTONHANDLER_PGA(uint32_t parameter) {
	if (!TRX.ADC_Driver && !TRX.ADC_PGA) {
		TRX.ADC_Driver = true;
		TRX.ADC_PGA = false;
	} else if (TRX.ADC_Driver && !TRX.ADC_PGA) {
		TRX.ADC_Driver = true;
		TRX.ADC_PGA = true;
	} else if (TRX.ADC_Driver && TRX.ADC_PGA) {
		TRX.ADC_Driver = false;
		TRX.ADC_PGA = true;
	} else if (!TRX.ADC_Driver && TRX.ADC_PGA) {
		TRX.ADC_Driver = false;
		TRX.ADC_PGA = false;
	}
	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	if (band >= 0) {
		TRX.BANDS_SAVED_SETTINGS[band].ADC_Driver = TRX.ADC_Driver;
		TRX.BANDS_SAVED_SETTINGS[band].ADC_PGA = TRX.ADC_PGA;
	}
	LCD_UpdateQuery.TopButtons = true;
	FPGA_NeedSendParams = true;
	NeedSaveSettings = true;
	resetVAD();
}

void BUTTONHANDLER_PGA_ONLY(uint32_t parameter) {
	TRX.ADC_PGA = !TRX.ADC_PGA;
	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	if (band >= 0) {
		TRX.BANDS_SAVED_SETTINGS[band].ADC_PGA = TRX.ADC_PGA;
	}
	LCD_UpdateQuery.TopButtons = true;
	FPGA_NeedSendParams = true;
	NeedSaveSettings = true;
	resetVAD();
}

void BUTTONHANDLER_DRV_ONLY(uint32_t parameter) {
	TRX.ADC_Driver = !TRX.ADC_Driver;
	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	if (band >= 0) {
		TRX.BANDS_SAVED_SETTINGS[band].ADC_Driver = TRX.ADC_Driver;
	}
	LCD_UpdateQuery.TopButtons = true;
	FPGA_NeedSendParams = true;
	NeedSaveSettings = true;
	resetVAD();
}

void BUTTONHANDLER_FAST(uint32_t parameter) {
	TRX.Fast = !TRX.Fast;

	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	if (band >= 0) {
		TRX.BANDS_SAVED_SETTINGS[band].Fast = TRX.Fast;
	}

	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
}

void BUTTONHANDLER_MODE_P(uint32_t parameter) {
	int8_t mode = (int8_t)CurrentVFO->Mode;
	if (mode == TRX_MODE_LSB) {
		mode = TRX_MODE_USB;
	} else if (mode == TRX_MODE_USB) {
		mode = TRX_MODE_LSB;
	} else if (mode == TRX_MODE_CW) {
		// mode = TRX_MODE_CW;
	} else if (mode == TRX_MODE_NFM) {
		mode = TRX_MODE_WFM;
	} else if (mode == TRX_MODE_WFM) {
		mode = TRX_MODE_NFM;
	} else if (mode == TRX_MODE_DIGI_L) {
		mode = TRX_MODE_DIGI_U;
	} else if (mode == TRX_MODE_DIGI_U) {
		mode = TRX_MODE_RTTY;
	} else if (mode == TRX_MODE_RTTY) {
		mode = TRX_MODE_DIGI_L;
	} else if (mode == TRX_MODE_AM) {
		mode = TRX_MODE_SAM_STEREO;
	} else if (mode == TRX_MODE_SAM_STEREO) {
		mode = TRX_MODE_SAM_LSB;
	} else if (mode == TRX_MODE_SAM_LSB) {
		mode = TRX_MODE_SAM_USB;
	} else if (mode == TRX_MODE_SAM_USB) {
		mode = TRX_MODE_IQ;
	} else if (mode == TRX_MODE_IQ) {
		mode = TRX_MODE_LOOPBACK;
		LCD_UpdateQuery.StatusInfoGUIRedraw = true;
	} else if (mode == TRX_MODE_LOOPBACK) {
		mode = TRX_MODE_AM;
		LCD_UpdateQuery.StatusInfoGUIRedraw = true;
	}

	TRX_setMode((uint8_t)mode, CurrentVFO);
	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	if (band >= 0) {
		TRX.BANDS_SAVED_SETTINGS[band].Mode = (uint8_t)mode;
	}
	TRX_Temporary_Stop_BandMap = true;
	resetVAD();
	TRX_ScanMode = false;
}

void BUTTONHANDLER_MODE_N(uint32_t parameter) {
	int8_t mode = (int8_t)CurrentVFO->Mode;
	if (mode == TRX_MODE_LOOPBACK) {
		LCD_UpdateQuery.StatusInfoGUIRedraw = true;
	}
	if (mode == TRX_MODE_LSB) {
		mode = TRX_MODE_CW;
	} else if (mode == TRX_MODE_USB) {
		mode = TRX_MODE_CW;
	} else if (mode == TRX_MODE_CW) {
		mode = TRX_MODE_DIGI_U;
	} else if (mode == TRX_MODE_DIGI_L || mode == TRX_MODE_DIGI_U || mode == TRX_MODE_RTTY) {
		mode = TRX_MODE_NFM;
	} else if (mode == TRX_MODE_NFM || mode == TRX_MODE_WFM) {
		mode = TRX_MODE_AM;
	} else {
		if (CurrentVFO->Freq < 10000000) {
			mode = TRX_MODE_LSB;
		} else {
			mode = TRX_MODE_USB;
		}
	}

	TRX_setMode((uint8_t)mode, CurrentVFO);
	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	if (band >= 0) {
		TRX.BANDS_SAVED_SETTINGS[band].Mode = (uint8_t)mode;
	}
	TRX_Temporary_Stop_BandMap = true;
	resetVAD();
	TRX_ScanMode = false;
}

void BUTTONHANDLER_BAND_P(uint32_t parameter) {
	// TX block
	TRX.SPLIT_Enabled = false;
	if (TRX_on_TX) {
		return;
	}

	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	band++;
	if (band >= BANDS_COUNT) {
		band = 0;
	}
	while (!BAND_SELECTABLE[band]) {
		band++;
		if (band >= BANDS_COUNT) {
			band = 0;
		}
	}

	TRX_setFrequency(TRX.BANDS_SAVED_SETTINGS[band].Freq, CurrentVFO);
	TRX_setMode(TRX.BANDS_SAVED_SETTINGS[band].Mode, CurrentVFO);

	LCD_UpdateQuery.TopButtons = true;
	LCD_UpdateQuery.FreqInfoRedraw = true;
	LCD_UpdateQuery.StatusInfoBarRedraw = true;
	LCD_UpdateQuery.StatusInfoGUI = true;
	resetVAD();
	TRX_ScanMode = false;
}

void BUTTONHANDLER_BAND_N(uint32_t parameter) {
	// TX block
	TRX.SPLIT_Enabled = false;
	if (TRX_on_TX) {
		return;
	}

	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	band--;
	if (band < 0) {
		band = BANDS_COUNT - 1;
	}
	while (!BAND_SELECTABLE[band]) {
		band--;
		if (band < 0) {
			band = BANDS_COUNT - 1;
		}
	}

	TRX_setFrequency(TRX.BANDS_SAVED_SETTINGS[band].Freq, CurrentVFO);
	TRX_setMode(TRX.BANDS_SAVED_SETTINGS[band].Mode, CurrentVFO);

	LCD_UpdateQuery.TopButtons = true;
	LCD_UpdateQuery.FreqInfoRedraw = true;
	LCD_UpdateQuery.StatusInfoBarRedraw = true;
	LCD_UpdateQuery.StatusInfoGUI = true;
	resetVAD();
	TRX_ScanMode = false;
}

void BUTTONHANDLER_RF_POWER(uint32_t parameter) {
#ifdef HAS_TOUCHPAD
	LCD_showRFPowerWindow();
#else
	if (!LCD_systemMenuOpened) {
		LCD_systemMenuOpened = true;
		SYSMENU_TX_RFPOWER_HOTKEY();
	} else {
		SYSMENU_eventCloseAllSystemMenu();
	}
#endif
}

void BUTTONHANDLER_AGC(uint32_t parameter) {
	CurrentVFO->AGC = !CurrentVFO->AGC;
	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	if (band >= 0) {
		TRX.BANDS_SAVED_SETTINGS[band].AGC = CurrentVFO->AGC;
	}
	TRX.AGC_shadow = CurrentVFO->AGC;
	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
}

void BUTTONHANDLER_AGC_SPEED(uint32_t parameter) {
	if (!LCD_systemMenuOpened) {
		LCD_systemMenuOpened = true;
		SYSMENU_RX_AGC_HOTKEY();
	} else {
		SYSMENU_eventCloseAllSystemMenu();
	}
}

void BUTTONHANDLER_AGC_MaxGain(uint32_t parameter) {
	if (!LCD_systemMenuOpened) {
		LCD_systemMenuOpened = true;
		SYSMENU_RX_AGC_MaxGain_HOTKEY();
	} else {
		SYSMENU_eventCloseAllSystemMenu();
	}
}

void BUTTONHANDLER_REPEATER_MODE(uint32_t parameter) {
	CurrentVFO->RepeaterMode = !CurrentVFO->RepeaterMode;
	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	if (band >= 0) {
		TRX.BANDS_SAVED_SETTINGS[band].RepeaterMode = CurrentVFO->RepeaterMode;
	}
	TRX.RepeaterMode_shadow = CurrentVFO->RepeaterMode;
	LCD_UpdateQuery.TopButtons = true;
	LCD_UpdateQuery.BottomButtons = true;
	NeedSaveSettings = true;
}

void BUTTONHANDLER_SQUELCH(uint32_t parameter) {
	if (!LCD_systemMenuOpened) {
		LCD_systemMenuOpened = true;
		SYSMENU_RX_SQUELCH_HOTKEY();
	} else {
		SYSMENU_eventCloseAllSystemMenu();
	}
}

void BUTTONHANDLER_WPM(uint32_t parameter) {
	if (!LCD_systemMenuOpened) {
		LCD_systemMenuOpened = true;
		SYSMENU_CW_WPM_HOTKEY();
	} else {
		SYSMENU_eventCloseAllSystemMenu();
	}
}

void BUTTONHANDLER_KEYER(uint32_t parameter) {
	TRX.CW_KEYER = !TRX.CW_KEYER;
	if (TRX.CW_KEYER) {
		LCD_showTooltip("KEYER ON");
	} else {
		LCD_showTooltip("KEYER OFF");
	}
}

void BUTTONHANDLER_STEP(uint32_t parameter) {
	if (!LCD_systemMenuOpened) {
		LCD_systemMenuOpened = true;
		SYSMENU_TRX_STEP_HOTKEY();
	} else {
		SYSMENU_eventCloseAllSystemMenu();
	}
}

void BUTTONHANDLER_DNR(uint32_t parameter) {
	TRX_TemporaryMute();
	CleanNoiseReduction();
	if (parameter > 0) {
		CurrentVFO->DNR_Type = (uint8_t)parameter - 1;
	} else {

		if (CurrentVFO->DNR_Type == 0) {
			CurrentVFO->DNR_Type = 1;
		} else if (CurrentVFO->DNR_Type == 1) {
			CurrentVFO->DNR_Type = 2;
		} else {
			CurrentVFO->DNR_Type = 0;
		}
	}
#ifdef STM32F407xx
	if (CurrentVFO->DNR_Type == 2) {
		CurrentVFO->DNR_Type = 0;
	}
#endif

	TRX.DNR_shadow = CurrentVFO->DNR_Type;

	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	if (band >= 0) {
		TRX.BANDS_SAVED_SETTINGS[band].DNR_Type = CurrentVFO->DNR_Type;
	}
	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
}

void BUTTONHANDLER_DNR_HOLD(uint32_t parameter) {
	if (!LCD_systemMenuOpened) {
		LCD_systemMenuOpened = true;
		SYSMENU_RX_DNR_HOTKEY();
	} else {
		SYSMENU_eventCloseAllSystemMenu();
	}
}

void BUTTONHANDLER_NB(uint32_t parameter) {
	if (!TRX.NOISE_BLANKER1 && !TRX.NOISE_BLANKER2) {
		TRX.NOISE_BLANKER1 = true;
		TRX.NOISE_BLANKER2 = false;
	} else if (TRX.NOISE_BLANKER1 && !TRX.NOISE_BLANKER2) {
		TRX.NOISE_BLANKER1 = false;
		TRX.NOISE_BLANKER2 = true;
	} else if (!TRX.NOISE_BLANKER1 && TRX.NOISE_BLANKER2) {
		TRX.NOISE_BLANKER1 = true;
		TRX.NOISE_BLANKER2 = true;
	} else {
		TRX.NOISE_BLANKER1 = false;
		TRX.NOISE_BLANKER2 = false;
	}

	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
}

void BUTTONHANDLER_NB_HOLD(uint32_t parameter) {
	if (!LCD_systemMenuOpened) {
		LCD_systemMenuOpened = true;
		if (TRX.NOISE_BLANKER2) {
			SYSMENU_RX_NB2_HOTKEY();
		} else {
			SYSMENU_RX_NB1_HOTKEY();
		}
	} else {
		SYSMENU_eventCloseAllSystemMenu();
	}
}

void BUTTONHANDLER_BW(uint32_t parameter) {
	if (!LCD_systemMenuOpened) {
		LCD_systemMenuOpened = true;
		if (CurrentVFO->Mode == TRX_MODE_CW) {
			SYSMENU_FILTER_BW_CW_HOTKEY();
		} else if (CurrentVFO->Mode == TRX_MODE_NFM || CurrentVFO->Mode == TRX_MODE_WFM) {
			SYSMENU_FILTER_BW_FM_HOTKEY();
		} else if (CurrentVFO->Mode == TRX_MODE_AM || CurrentVFO->Mode == TRX_MODE_SAM_STEREO || CurrentVFO->Mode == TRX_MODE_SAM_LSB || CurrentVFO->Mode == TRX_MODE_SAM_USB) {
			SYSMENU_FILTER_BW_AM_HOTKEY();
		} else {
			SYSMENU_FILTER_BW_SSB_HOTKEY();
		}
	} else {
		SYSMENU_eventCloseAllSystemMenu();
	}
}

void BUTTONHANDLER_HPF(uint32_t parameter) {
	if (!LCD_systemMenuOpened) {
		LCD_systemMenuOpened = true;
		SYSMENU_FILTER_HPF_SSB_HOTKEY();
	} else {
		SYSMENU_eventCloseAllSystemMenu();
	}
}

void BUTTONHANDLER_ArB(uint32_t parameter) // A=B
{
	// Block for LITE and MINI in MEMO mode
#if defined(FRONTPANEL_MINI) || defined(FRONTPANEL_FRONTPANEL_LITE)
	if (TRX.ENC2_func_mode == ENC_FUNC_SET_MEM) {
		return;
	}
#endif

	if (TRX.selected_vfo) {
		dma_memcpy(&TRX.VFO_A, &TRX.VFO_B, sizeof TRX.VFO_B);
	} else {
		dma_memcpy(&TRX.VFO_B, &TRX.VFO_A, sizeof TRX.VFO_B);
	}

	LCD_showTooltip("VFO COPIED");

	LCD_UpdateQuery.TopButtons = true;
	LCD_UpdateQuery.FreqInfo = true;
	NeedSaveSettings = true;
}

void BUTTONHANDLER_NOTCH(uint32_t parameter) {
	TRX_TemporaryMute();

	if (CurrentVFO->NotchFC > CurrentVFO->LPF_RX_Filter_Width) {
		CurrentVFO->NotchFC = CurrentVFO->LPF_RX_Filter_Width;
		NeedReinitNotch = true;
	}
	CurrentVFO->ManualNotchFilter = false;

	if (!CurrentVFO->AutoNotchFilter) {
		InitAutoNotchReduction();
		CurrentVFO->AutoNotchFilter = true;
	} else {
		CurrentVFO->AutoNotchFilter = false;
	}

	LCD_UpdateQuery.StatusInfoGUI = true;
	LCD_UpdateQuery.TopButtons = true;
	NeedWTFRedraw = true;
	NeedSaveSettings = true;
}

void BUTTONHANDLER_NOTCH_MANUAL(uint32_t parameter) {
	if (CurrentVFO->NotchFC > CurrentVFO->LPF_RX_Filter_Width) {
		CurrentVFO->NotchFC = CurrentVFO->LPF_RX_Filter_Width;
	}
	CurrentVFO->AutoNotchFilter = false;
	if (!CurrentVFO->ManualNotchFilter) {
		CurrentVFO->ManualNotchFilter = true;
	} else {
		CurrentVFO->ManualNotchFilter = false;
	}

	LCD_UpdateQuery.TopButtons = true;
	LCD_UpdateQuery.StatusInfoGUI = true;
	NeedReinitNotch = true;
	NeedWTFRedraw = true;
	NeedSaveSettings = true;
}

void BUTTONHANDLER_NOTCH_SWITCH(uint32_t parameter) {
	if (!CurrentVFO->AutoNotchFilter && !CurrentVFO->ManualNotchFilter) {
		BUTTONHANDLER_NOTCH(parameter);
		return;
	}
	if (CurrentVFO->AutoNotchFilter) {
		BUTTONHANDLER_NOTCH(parameter);
		BUTTONHANDLER_NOTCH_MANUAL(parameter);
		return;
	}
	if (CurrentVFO->ManualNotchFilter) {
		BUTTONHANDLER_NOTCH_MANUAL(parameter);
		return;
	}
}

void BUTTONHANDLER_RIT(uint32_t parameter) {
	TRX.RIT_Enabled = !TRX.RIT_Enabled;

#ifndef FRONTPANEL_WOLF_2
	if (TRX.RIT_Enabled) {
		TRX.ENC2_func_mode = ENC_FUNC_SET_RIT;
	}
	if (!TRX.XIT_Enabled && !TRX.RIT_Enabled && TRX.ENC2_func_mode == ENC_FUNC_SET_RIT) {
		TRX.ENC2_func_mode = ENC_FUNC_FAST_STEP;
	}
#endif

	TRX.SPLIT_Enabled = false;
	TRX_setFrequency(CurrentVFO->Freq, CurrentVFO);
	TRX_setFrequency(SecondaryVFO->Freq, SecondaryVFO);
	LCD_UpdateQuery.TopButtons = true;
	LCD_UpdateQuery.StatusInfoGUI = true;
	NeedSaveSettings = true;
}

void BUTTONHANDLER_XIT(uint32_t parameter) {
	TRX.XIT_Enabled = !TRX.XIT_Enabled;

#ifndef FRONTPANEL_WOLF_2
	if (TRX.XIT_Enabled) {
		TRX.ENC2_func_mode = ENC_FUNC_SET_RIT;
	}
	if (!TRX.XIT_Enabled && !TRX.RIT_Enabled && TRX.ENC2_func_mode == ENC_FUNC_SET_RIT) {
		TRX.ENC2_func_mode = ENC_FUNC_FAST_STEP;
	}
#endif

	TRX.SPLIT_Enabled = false;
	TRX_setFrequency(CurrentVFO->Freq, CurrentVFO);
	TRX_setFrequency(SecondaryVFO->Freq, SecondaryVFO);
	LCD_UpdateQuery.FreqInfo = true;
	LCD_UpdateQuery.TopButtons = true;
	LCD_UpdateQuery.StatusInfoGUI = true;
	NeedSaveSettings = true;
}

void BUTTONHANDLER_RITXIT(uint32_t parameter) {
	if (!TRX.RIT_Enabled && !TRX.XIT_Enabled) {
		BUTTONHANDLER_RIT(parameter);
		return;
	}
	if (TRX.RIT_Enabled) {
		BUTTONHANDLER_RIT(parameter);
		BUTTONHANDLER_XIT(parameter);
		return;
	}
	if (TRX.XIT_Enabled) {
		BUTTONHANDLER_XIT(parameter);
		return;
	}
}

void BUTTONHANDLER_SPLIT(uint32_t parameter) {
	TRX.SPLIT_Enabled = !TRX.SPLIT_Enabled;
	TRX.XIT_Enabled = false;
	TRX.RIT_Enabled = false;
	TRX_setFrequency(CurrentVFO->Freq, CurrentVFO);
	TRX_setFrequency(SecondaryVFO->Freq, SecondaryVFO);
	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
}

void BUTTONHANDLER_LOCK(uint32_t parameter) {
	if (!LCD_systemMenuOpened) {
		TRX.Locked = !TRX.Locked;
	} else {
		SYSMENU_hiddenmenu_enabled = true;
		LCD_redraw(false);
	}
	LCD_UpdateQuery.TopButtons = true;
	LCD_UpdateQuery.StatusInfoBar = true;
	NeedSaveSettings = true;
}

void BUTTONHANDLER_MENU(uint32_t parameter) {
	if (!LCD_systemMenuOpened) {
		LCD_systemMenuOpened = true;
	} else {
		SYSMENU_eventCloseSystemMenu();
	}
	LCD_redraw(false);
}

void BUTTONHANDLER_MENUHOLD(uint32_t parameter) {
	if (LCD_systemMenuOpened) {
		SYSMENU_hiddenmenu_enabled = true;
		LCD_redraw(false);
		return;
	} else {
		BUTTONHANDLER_MENU(parameter);
	}
}

void BUTTONHANDLER_MUTE(uint32_t parameter) {
	TRX.Mute = !TRX.Mute;
	if (!TRX.Mute) {
		TRX.AFAmp_Mute = false;
		CODEC_UnMute_AF_AMP();
	}
	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
}

void BUTTONHANDLER_MUTE_AFAMP(uint32_t parameter) {
	TRX.AFAmp_Mute = !TRX.AFAmp_Mute;
	if (TRX.AFAmp_Mute) {
		CODEC_Mute_AF_AMP();
	} else {
		CODEC_UnMute_AF_AMP();
	}
	TRX.Mute = false;

	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
}

void BUTTONHANDLER_BLUETOOTH_AUDIO_ENABLED(uint32_t parameter) {
	TRX.BluetoothAudio_Enabled = !TRX.BluetoothAudio_Enabled;
	CODEC_TXRX_mode();
	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
}

void BUTTONHANDLER_BANDMAP(uint32_t parameter) {
	TRX.BandMapEnabled = !TRX.BandMapEnabled;

	if (TRX.BandMapEnabled) {
		LCD_showTooltip("BANDMAP ON");
	} else {
		LCD_showTooltip("BANDMAP OFF");
	}

	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
}

void BUTTONHANDLER_VOX(uint32_t parameter) {
	TRX.VOX = !TRX.VOX;

	if (TRX.VOX) {
		LCD_showTooltip("VOX ON");
	} else {
		LCD_showTooltip("VOX OFF");
	}

	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
}

void BUTTONHANDLER_AUTOGAINER(uint32_t parameter) {
	TRX.AutoGain = !TRX.AutoGain;

	if (TRX.AutoGain) {
		LCD_showTooltip("AUTOGAIN ON");
	} else {
		LCD_showTooltip("AUTOGAIN OFF");
	}

	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
}

void BUTTONHANDLER_SERVICES(uint32_t parameter) {
	if (!LCD_systemMenuOpened) {
		LCD_systemMenuOpened = true;
		SYSMENU_HANDL_SERVICESMENU(1);
	} else {
		SYSMENU_eventCloseSystemMenu();
	}
}

void BUTTONHANDLER_SQL(uint32_t parameter) {
	CurrentVFO->SQL = !CurrentVFO->SQL;
	TRX.SQL_shadow = CurrentVFO->SQL;

	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	if (band >= 0) {
		TRX.BANDS_SAVED_SETTINGS[band].SQL = CurrentVFO->SQL;
	}

	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
}

void BUTTONHANDLER_SCAN(uint32_t parameter) {
	TRX_ScanMode = !TRX_ScanMode;
	LCD_UpdateQuery.TopButtons = true;
}

void BUTTONHANDLER_CQ(uint32_t parameter) {
#if HRDW_HAS_SD
	if (SD_RecordInProcess) {
		SD_NeedStopRecord = true;
		return;
	}
	if (SD_PlayInProcess) {
		SD_NeedStopPlay = true;
		return;
	}
	if (SD_PlayCQMessageInProcess) {
		SD_NeedStopPlay = true;
		return;
	}

	// go tx
	TRX_ptt_soft = true;
	TRX_ptt_change();

	// start play cq message
	SD_PlayCQMessageInProcess = true;
	dma_memset(SD_workbuffer_A, 0, sizeof(SD_workbuffer_A));
	strcat((char *)SD_workbuffer_A, SD_CQ_MESSAGE_FILE);
	SD_doCommand(SDCOMM_START_PLAY, false);
#endif

	LCD_UpdateQuery.TopButtons = true;
}

void BUTTONHANDLER_REC(uint32_t parameter) {
#if HRDW_HAS_SD
	if (!SD_RecordInProcess) {
		SD_doCommand(SDCOMM_START_RECORD, false);
	} else {
		SD_NeedStopRecord = true;
	}
#endif

	LCD_UpdateQuery.TopButtons = true;
}

void BUTTONHANDLER_FUNC(uint32_t parameter) {
	if (!TRX.Locked) { // LOCK BUTTON
		if (!LCD_systemMenuOpened || PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[TRX.FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + parameter]].work_in_menu) {
			if (PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[TRX.FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + parameter]].clickHandler == NULL) {
				return;
			}

			PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[TRX.FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + parameter]].clickHandler(
			    PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[TRX.FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + parameter]].parameter);
		}
	}
}

void BUTTONHANDLER_FUNCH(uint32_t parameter) {
	if (parameter == 7 && LCD_systemMenuOpened) {
		SYSMENU_hiddenmenu_enabled = true;
		LCD_redraw(false);
	} else if (!TRX.Locked ||
	           PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[TRX.FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + parameter]].holdHandler == BUTTONHANDLER_LOCK) { // LOCK BUTTON
		if (!LCD_systemMenuOpened || PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[TRX.FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + parameter]].work_in_menu) {
			if (PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[TRX.FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + parameter]].holdHandler == NULL) {
				return;
			}

			PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[TRX.FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + parameter]].holdHandler(
			    PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[TRX.FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + parameter]].parameter);
		}
	}
}

void BUTTONHANDLER_UP(uint32_t parameter) {
	uint32_t newfreq = CurrentVFO->Freq + 500;
	newfreq = newfreq / 500 * 500;
	TRX_setFrequency(newfreq, CurrentVFO);
	LCD_UpdateQuery.FreqInfo = true;
}

void BUTTONHANDLER_DOWN(uint32_t parameter) {
	uint32_t newfreq = CurrentVFO->Freq - 500;
	newfreq = newfreq / 500 * 500;
	TRX_setFrequency(newfreq, CurrentVFO);
	LCD_UpdateQuery.FreqInfo = true;
}

void BUTTONHANDLER_SET_CUR_VFO_BAND(uint32_t parameter) {
	int8_t band = parameter;
	if (band >= BANDS_COUNT) {
		band = 0;
	}

	// manual freq enter
	if (LCD_window.opened && TRX.BANDS_SAVED_SETTINGS[band].Freq == CurrentVFO->Freq) {
		TRX_Temporary_Stop_BandMap = false;
		resetVAD();
		TRX_ScanMode = false;
		LCD_closeWindow();
		LCD_redraw(true);
		LCD_showManualFreqWindow(false);
		return;
	}
	//

	TRX_setFrequency(TRX.BANDS_SAVED_SETTINGS[band].Freq, CurrentVFO);
	TRX_setMode(TRX.BANDS_SAVED_SETTINGS[band].Mode, CurrentVFO);

	LCD_UpdateQuery.TopButtons = true;
	LCD_UpdateQuery.FreqInfoRedraw = true;
	LCD_UpdateQuery.StatusInfoBarRedraw = true;

	resetVAD();
	TRX_ScanMode = false;
	if (LCD_window.opened) {
		LCD_closeWindow();
	}

#if FT8_SUPPORT
	if (SYSMENU_FT8_DECODER_opened) {
		InitFT8_Decoder();
	}
#endif
}

void BUTTONHANDLER_SET_VFOA_FREQ_MANUAL(uint32_t parameter) {
	TRX_Temporary_Stop_BandMap = false;
	resetVAD();
	TRX_ScanMode = false;
	LCD_closeWindow();
	LCD_redraw(true);
	LCD_showManualFreqWindow(false);
	return;
}

void BUTTONHANDLER_SET_VFOB_FREQ_MANUAL(uint32_t parameter) {
	TRX_Temporary_Stop_BandMap = false;
	resetVAD();
	TRX_ScanMode = false;
	LCD_closeWindow();
	LCD_redraw(true);
	LCD_showManualFreqWindow(true);
	return;
}

void BUTTONHANDLER_SET_VFOA_BAND(uint32_t parameter) {
	int8_t band = parameter;
	if (band >= BANDS_COUNT) {
		band = 0;
	}

	TRX_setFrequency(TRX.BANDS_SAVED_SETTINGS[band].Freq, &TRX.VFO_A);
	TRX_setMode(TRX.BANDS_SAVED_SETTINGS[band].Mode, &TRX.VFO_A);

	resetVAD();
	TRX_ScanMode = false;
	if (LCD_window.opened) {
		LCD_closeWindow();
	}
}

void BUTTONHANDLER_SET_VFOB_BAND(uint32_t parameter) {
	int8_t band = parameter;
	if (band >= BANDS_COUNT) {
		band = 0;
	}

	TRX_setFrequency(TRX.BANDS_SAVED_SETTINGS[band].Freq, &TRX.VFO_B);
	TRX_setMode(TRX.BANDS_SAVED_SETTINGS[band].Mode, &TRX.VFO_B);

	LCD_UpdateQuery.TopButtons = true;
	LCD_UpdateQuery.FreqInfoRedraw = true;
	LCD_UpdateQuery.StatusInfoBarRedraw = true;

	resetVAD();
	TRX_ScanMode = false;
	LCD_closeWindow();
}

void BUTTONHANDLER_SETMODE(uint32_t parameter) {
	int8_t mode = parameter;
	TRX_setMode((uint8_t)mode, &TRX.VFO_A);
	int8_t band = getBandFromFreq(TRX.VFO_A.Freq, true);
	if (band >= 0) {
		TRX.BANDS_SAVED_SETTINGS[band].Mode = (uint8_t)mode;
	}
	TRX_Temporary_Stop_BandMap = true;
	resetVAD();
	if (CurrentVFO->NotchFC > CurrentVFO->LPF_RX_Filter_Width) {
		CurrentVFO->NotchFC = CurrentVFO->LPF_RX_Filter_Width;
		NeedReinitNotch = true;
	}
	TRX_ScanMode = false;
	LCD_closeWindow();
}

void BUTTONHANDLER_SETSECMODE(uint32_t parameter) {
	int8_t mode = parameter;
	TRX_setMode((uint8_t)mode, &TRX.VFO_B);
	int8_t band = getBandFromFreq(TRX.VFO_B.Freq, true);
	if (band >= 0) {
		TRX.BANDS_SAVED_SETTINGS[band].Mode = (uint8_t)mode;
	}
	TRX_Temporary_Stop_BandMap = true;
	resetVAD();
	if (SecondaryVFO->NotchFC > SecondaryVFO->LPF_RX_Filter_Width) {
		SecondaryVFO->NotchFC = SecondaryVFO->LPF_RX_Filter_Width;
		NeedReinitNotch = true;
	}
	TRX_ScanMode = false;
	LCD_closeWindow();
}

void BUTTONHANDLER_SET_RX_BW(uint32_t parameter) {
	if (CurrentVFO->Mode == TRX_MODE_CW) {
		CurrentVFO->CW_LPF_Filter = parameter;
	}
	if (CurrentVFO->Mode == TRX_MODE_LSB || CurrentVFO->Mode == TRX_MODE_USB) {
		CurrentVFO->SSB_LPF_RX_Filter = parameter;
	}
	if (CurrentVFO->Mode == TRX_MODE_DIGI_L || CurrentVFO->Mode == TRX_MODE_DIGI_U || CurrentVFO->Mode == TRX_MODE_RTTY) {
		CurrentVFO->DIGI_LPF_Filter = parameter;
	}
	if (CurrentVFO->Mode == TRX_MODE_AM || CurrentVFO->Mode == TRX_MODE_SAM_STEREO || CurrentVFO->Mode == TRX_MODE_SAM_LSB || CurrentVFO->Mode == TRX_MODE_SAM_USB) {
		CurrentVFO->AM_LPF_RX_Filter = parameter;
	}
	if (CurrentVFO->Mode == TRX_MODE_NFM) {
		CurrentVFO->FM_LPF_RX_Filter = parameter;
	}

	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	if (band >= 0) {
		if (CurrentVFO == &TRX.VFO_A) {
			TRX.BANDS_SAVED_SETTINGS[band].VFO_A_CW_LPF_Filter = CurrentVFO->CW_LPF_Filter;
			TRX.BANDS_SAVED_SETTINGS[band].VFO_A_SSB_LPF_RX_Filter = CurrentVFO->SSB_LPF_RX_Filter;
			TRX.BANDS_SAVED_SETTINGS[band].VFO_A_AM_LPF_RX_Filter = CurrentVFO->AM_LPF_RX_Filter;
			TRX.BANDS_SAVED_SETTINGS[band].VFO_A_FM_LPF_RX_Filter = CurrentVFO->FM_LPF_RX_Filter;
		} else {
			TRX.BANDS_SAVED_SETTINGS[band].VFO_B_CW_LPF_Filter = CurrentVFO->CW_LPF_Filter;
			TRX.BANDS_SAVED_SETTINGS[band].VFO_B_SSB_LPF_RX_Filter = CurrentVFO->SSB_LPF_RX_Filter;
			TRX.BANDS_SAVED_SETTINGS[band].VFO_B_AM_LPF_RX_Filter = CurrentVFO->AM_LPF_RX_Filter;
			TRX.BANDS_SAVED_SETTINGS[band].VFO_B_FM_LPF_RX_Filter = CurrentVFO->FM_LPF_RX_Filter;
		}
	}

	TRX.CW_LPF_Filter_shadow = CurrentVFO->CW_LPF_Filter;
	TRX.SSB_LPF_RX_Filter_shadow = CurrentVFO->SSB_LPF_RX_Filter;
	TRX.AM_LPF_RX_Filter_shadow = CurrentVFO->AM_LPF_RX_Filter;
	TRX.FM_LPF_RX_Filter_shadow = CurrentVFO->FM_LPF_RX_Filter;

	TRX_setMode(SecondaryVFO->Mode, SecondaryVFO);
	TRX_setMode(CurrentVFO->Mode, CurrentVFO);

	LCD_closeWindow();
}

void BUTTONHANDLER_SET_TX_BW(uint32_t parameter) {
	if (CurrentVFO->Mode == TRX_MODE_CW) {
		CurrentVFO->CW_LPF_Filter = parameter;
	}
	if (CurrentVFO->Mode == TRX_MODE_LSB || CurrentVFO->Mode == TRX_MODE_USB) {
		CurrentVFO->SSB_LPF_TX_Filter = parameter;
	}
	if (CurrentVFO->Mode == TRX_MODE_DIGI_L || CurrentVFO->Mode == TRX_MODE_DIGI_U || CurrentVFO->Mode == TRX_MODE_RTTY) {
		CurrentVFO->DIGI_LPF_Filter = parameter;
	}
	if (CurrentVFO->Mode == TRX_MODE_AM || CurrentVFO->Mode == TRX_MODE_SAM_STEREO || CurrentVFO->Mode == TRX_MODE_SAM_LSB || CurrentVFO->Mode == TRX_MODE_SAM_USB) {
		CurrentVFO->AM_LPF_TX_Filter = parameter;
	}
	if (CurrentVFO->Mode == TRX_MODE_NFM) {
		CurrentVFO->FM_LPF_TX_Filter = parameter;
	}

	TRX_setMode(SecondaryVFO->Mode, SecondaryVFO);
	TRX_setMode(CurrentVFO->Mode, CurrentVFO);

	LCD_closeWindow();
}

void BUTTONHANDLER_SETRF_POWER(uint32_t parameter) {
	TRX.RF_Gain = parameter;

	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	if (band >= 0) {
		TRX.BANDS_SAVED_SETTINGS[band].RF_Gain = TRX.RF_Gain;
	}

	APROC_TX_clip_gain = 1.0f;
	APROC_TX_tune_power = 0.0f;
	ATU_TunePowerStabilized = false;
	LCD_closeWindow();
}

void BUTTONHANDLER_SET_ATT_DB(uint32_t parameter) {
	TRX.ATT_DB = parameter;

	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	if (band >= 0) {
		TRX.BANDS_SAVED_SETTINGS[band].ATT = TRX.ATT;
		TRX.BANDS_SAVED_SETTINGS[band].ATT_DB = TRX.ATT_DB;
	}

	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
	resetVAD();

	LCD_closeWindow();
}

void BUTTONHANDLER_LEFT_ARR(uint32_t parameter) {
	if (TRX.FRONTPANEL_funcbuttons_page == 0) {
		TRX.FRONTPANEL_funcbuttons_page = (FUNCBUTTONS_PAGES - 1);
	} else {
		TRX.FRONTPANEL_funcbuttons_page--;
	}

	// check page fully hidden
	bool allButtonsHidden = true;
	for (uint8_t i = 0; i < FUNCBUTTONS_ON_PAGE; i++) {
		uint16_t menuPosition = TRX.FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + i;
		if (PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[menuPosition]].clickHandler != NULL || PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[menuPosition]].holdHandler != NULL) {
			allButtonsHidden = false;
		}
	}
	if (allButtonsHidden) {
		BUTTONHANDLER_LEFT_ARR(parameter);
	}

	LCD_UpdateQuery.BottomButtons = true;
	LCD_UpdateQuery.TopButtons = true;
}

void BUTTONHANDLER_RIGHT_ARR(uint32_t parameter) {
	if (TRX.FRONTPANEL_funcbuttons_page >= (FUNCBUTTONS_PAGES - 1)) {
		TRX.FRONTPANEL_funcbuttons_page = 0;
	} else {
		TRX.FRONTPANEL_funcbuttons_page++;
	}

	// check page fully hidden
	bool allButtonsHidden = true;
	for (uint8_t i = 0; i < FUNCBUTTONS_ON_PAGE; i++) {
		uint16_t menuPosition = TRX.FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + i;
		if (PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[menuPosition]].clickHandler != NULL || PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[menuPosition]].holdHandler != NULL) {
			allButtonsHidden = false;
		}
	}
	if (allButtonsHidden) {
		BUTTONHANDLER_RIGHT_ARR(parameter);
	}

	LCD_UpdateQuery.BottomButtons = true;
	LCD_UpdateQuery.TopButtons = true;
}

void BUTTONHANDLER_SAMPLE_N(uint32_t parameter) {
	if (CurrentVFO->Mode == TRX_MODE_NFM || CurrentVFO->Mode == TRX_MODE_WFM) {
		if (TRX.SAMPLERATE_FM > 0) {
			TRX.SAMPLERATE_FM -= 1;
		}
	} else {
		if (TRX.SAMPLERATE_MAIN > 0) {
			TRX.SAMPLERATE_MAIN -= 1;
		}
		int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
		TRX.BANDS_SAVED_SETTINGS[band].SAMPLERATE = TRX.SAMPLERATE_MAIN;
	}

	FFT_Init();
	NeedReinitAudioFilters = true;
	LCD_UpdateQuery.StatusInfoBar = true;

	CurrentVFO->SpectrumCenterFreq = CurrentVFO->Freq;
	TRX_setFrequency(CurrentVFO->Freq, CurrentVFO);
}

void BUTTONHANDLER_SAMPLE_P(uint32_t parameter) {
	if (CurrentVFO->Mode == TRX_MODE_NFM || CurrentVFO->Mode == TRX_MODE_WFM) {
		if (TRX.SAMPLERATE_FM < 3) {
			TRX.SAMPLERATE_FM += 1;
		}
	} else {
		if (TRX.SAMPLERATE_MAIN < 3) {
			TRX.SAMPLERATE_MAIN += 1;
		}
		int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
		TRX.BANDS_SAVED_SETTINGS[band].SAMPLERATE = TRX.SAMPLERATE_MAIN;
	}

	FFT_Init();
	NeedReinitAudioFilters = true;
	LCD_UpdateQuery.StatusInfoBar = true;

	CurrentVFO->SpectrumCenterFreq = CurrentVFO->Freq;
	TRX_setFrequency(CurrentVFO->Freq, CurrentVFO);
}

void BUTTONHANDLER_ZOOM_N(uint32_t parameter) {
	if (CurrentVFO->Mode == TRX_MODE_CW) {
		if (TRX.FFT_ZoomCW == 2) {
			TRX.FFT_ZoomCW = 1;
		} else if (TRX.FFT_ZoomCW == 4) {
			TRX.FFT_ZoomCW = 2;
		} else if (TRX.FFT_ZoomCW == 8) {
			TRX.FFT_ZoomCW = 4;
		} else if (TRX.FFT_ZoomCW == 16) {
			TRX.FFT_ZoomCW = 8;
		} else if (TRX.FFT_ZoomCW == 32) {
			TRX.FFT_ZoomCW = 16;
		}
	} else {
		if (TRX.FFT_Zoom == 2) {
			TRX.FFT_Zoom = 1;
		} else if (TRX.FFT_Zoom == 4) {
			TRX.FFT_Zoom = 2;
		} else if (TRX.FFT_Zoom == 8) {
			TRX.FFT_Zoom = 4;
		} else if (TRX.FFT_Zoom == 16) {
			TRX.FFT_Zoom = 8;
		} else if (TRX.FFT_Zoom == 32) {
			TRX.FFT_Zoom = 16;
		}
	}

	FFT_Init();
	LCD_UpdateQuery.StatusInfoBar = true;

	CurrentVFO->SpectrumCenterFreq = CurrentVFO->Freq;
	TRX_setFrequency(CurrentVFO->Freq, CurrentVFO);
}

void BUTTONHANDLER_ZOOM_P(uint32_t parameter) {
	if (CurrentVFO->Mode == TRX_MODE_CW) {
		if (TRX.FFT_ZoomCW == 1) {
			TRX.FFT_ZoomCW = 2;
		} else if (TRX.FFT_ZoomCW == 2) {
			TRX.FFT_ZoomCW = 4;
		} else if (TRX.FFT_ZoomCW == 4) {
			TRX.FFT_ZoomCW = 8;
		} else if (TRX.FFT_ZoomCW == 8) {
			TRX.FFT_ZoomCW = 16;
		} else if (TRX.FFT_ZoomCW == 16) {
			TRX.FFT_ZoomCW = 32;
		}
	} else {
		if (TRX.FFT_Zoom == 1) {
			TRX.FFT_Zoom = 2;
		} else if (TRX.FFT_Zoom == 2) {
			TRX.FFT_Zoom = 4;
		} else if (TRX.FFT_Zoom == 4) {
			TRX.FFT_Zoom = 8;
		} else if (TRX.FFT_Zoom == 8) {
			TRX.FFT_Zoom = 16;
		} else if (TRX.FFT_Zoom == 16) {
			TRX.FFT_Zoom = 32;
		}
	}

	FFT_Init();
	LCD_UpdateQuery.StatusInfoBar = true;

	CurrentVFO->SpectrumCenterFreq = CurrentVFO->Freq;
	TRX_setFrequency(CurrentVFO->Freq, CurrentVFO);
}

void BUTTONHANDLER_SelectMemoryChannels(uint32_t parameter) {
	int8_t channel = parameter;
	if (channel >= MEMORY_CHANNELS_COUNT) {
		channel = 0;
	}

	TRX_setFrequency(CALIBRATE.MEMORY_CHANNELS[channel].freq, CurrentVFO);
	TRX_setMode(CALIBRATE.MEMORY_CHANNELS[channel].mode, CurrentVFO);
	TRX.CTCSS_Freq = CALIBRATE.MEMORY_CHANNELS[channel].CTCSS_Freq;
	CurrentVFO->RepeaterMode = CALIBRATE.MEMORY_CHANNELS[channel].RepeaterMode;
	TRX.RepeaterMode_shadow = CurrentVFO->RepeaterMode;

	TRX.SQL_shadow = CurrentVFO->SQL;
	TRX.FM_SQL_threshold_dBm_shadow = CurrentVFO->FM_SQL_threshold_dBm;

	TRX_Temporary_Stop_BandMap = false;
	LCD_UpdateQuery.TopButtons = true;
	LCD_UpdateQuery.FreqInfoRedraw = true;

	resetVAD();
	if (TRX.ENC2_func_mode != ENC_FUNC_SET_MEM || !TRX_ScanMode) {
		TRX_ScanMode = false;
		LCD_closeWindow();
		TRX_DXCluster_UpdateTime = 0;
		TRX_WOLF_Cluster_UpdateTime = 0;
	}
}

void BUTTONHANDLER_SaveMemoryChannels(uint32_t parameter) {
	int8_t channel = parameter;
	if (channel >= MEMORY_CHANNELS_COUNT) {
		channel = 0;
	}

	LCD_closeWindow();

	if (CALIBRATE.MEMORY_CHANNELS[channel].freq == CurrentVFO->Freq) {
		CALIBRATE.MEMORY_CHANNELS[channel].freq = 0;

#if (!defined(FRONTPANEL_LITE) && !defined(FRONTPANEL_MINI))
		sprintf(CALIBRATE.MEMORY_CHANNELS[channel].name, "Ch %d", channel + 1);
#endif

		LCD_showTooltip("Channel removed");
	} else {
		CALIBRATE.MEMORY_CHANNELS[channel].freq = CurrentVFO->Freq;
		CALIBRATE.MEMORY_CHANNELS[channel].mode = CurrentVFO->Mode;
		CALIBRATE.MEMORY_CHANNELS[channel].CTCSS_Freq = TRX.CTCSS_Freq;
		CALIBRATE.MEMORY_CHANNELS[channel].RepeaterMode = CurrentVFO->RepeaterMode;

#if (!defined(FRONTPANEL_LITE) && !defined(FRONTPANEL_MINI))
		if (CurrentVFO->Freq < 1000000) {                                                                 // < 1mhz
			sprintf(CALIBRATE.MEMORY_CHANNELS[channel].name, "%.2f", CurrentVFO->Freq / 1000.0);            // 3k.2h
		} else if (CurrentVFO->Freq < 1000000000) {                                                       // < 1000mhz
			sprintf(CALIBRATE.MEMORY_CHANNELS[channel].name, "%.3f", CurrentVFO->Freq / 1000000.0);         // 3m.3k
		} else {                                                                                          // >= 1000mhz
			sprintf(CALIBRATE.MEMORY_CHANNELS[channel].name, "%u", (uint32_t)(CurrentVFO->Freq / 1000000)); // 3m
		}
#endif

		LCD_showTooltip("Channel saved");
	}

	NeedSaveCalibration = true;
}

void BUTTONHANDLER_SET_BAND_MEMORY(uint32_t parameter) {
	int8_t band = parameter;
	if (band >= BANDS_COUNT) {
		band = 0;
	}
	if (band < 0) {
		return;
	}

	int8_t currentband = getBandFromFreq(CurrentVFO->Freq, true);
	if (currentband != band) {
		BUTTONHANDLER_GET_BAND_MEMORY(parameter);
		return;
	}

	// slide mems
	for (uint8_t j = BANDS_MEMORIES_COUNT - 1; j > 0; j--) {
		CALIBRATE.BAND_MEMORIES[band][j] = CALIBRATE.BAND_MEMORIES[band][j - 1];
	}

	CALIBRATE.BAND_MEMORIES[band][0] = CurrentVFO->Freq;

	LCD_showTooltip("Band mem saved");
	NeedSaveCalibration = true;
}

void BUTTONHANDLER_GET_BAND_MEMORY(uint32_t parameter) {
	int8_t band = parameter;
	if (band >= BANDS_COUNT) {
		band = 0;
	}
	if (band < 0) {
		return;
	}

	int8_t currentband = getBandFromFreq(CurrentVFO->Freq, true);
	if (currentband != band) {
		BUTTONHANDLER_SET_CUR_VFO_BAND(band);
		return;
	}

	int8_t mem_num = -1;
	for (uint8_t j = 0; j < BANDS_MEMORIES_COUNT; j++) {
		if (CALIBRATE.BAND_MEMORIES[band][j] == CurrentVFO->Freq) {
			mem_num = j;
			break;
		}
	}

	mem_num++;
	if (mem_num >= BANDS_MEMORIES_COUNT) {
		mem_num = 0;
	}

	if (CALIBRATE.BAND_MEMORIES[band][mem_num] == 0) {
		return;
	}

	println("Get band memory: ", (int16_t)band, " ", (int16_t)mem_num, " ", CALIBRATE.BAND_MEMORIES[band][mem_num]);

	BUTTONHANDLER_SET_CUR_VFO_BAND(band);
	TRX_setFrequency(CALIBRATE.BAND_MEMORIES[band][mem_num], CurrentVFO);
	LCD_UpdateQuery.StatusInfoBarRedraw = true;
}

void BUTTONHANDLER_FILEMANAGER(uint32_t parameter) {
#if HRDW_HAS_SD
	if (!LCD_systemMenuOpened) {
		LCD_systemMenuOpened = true;
		SYSMENU_SD_FILEMANAGER_HOTKEY();
	} else {
		SYSMENU_eventCloseAllSystemMenu();
	}
#endif
}

void BUTTONHANDLER_FT8(uint32_t parameter) {
	if (!LCD_systemMenuOpened) {
		LCD_systemMenuOpened = true;
		SYSMENU_SERVICE_FT8_HOTKEY();
	} else {
		SYSMENU_eventCloseAllSystemMenu();
	}
}

void BUTTONHANDLER_IF(uint32_t parameter) {
	if (!LCD_systemMenuOpened) {
		LCD_systemMenuOpened = true;
		SYSMENU_RX_IF_HOTKEY();
	} else {
		SYSMENU_eventCloseAllSystemMenu();
	}
}

void BUTTONHANDLER_VLT(uint32_t parameter) {
#if FRONTPANEL_X1
	TRX_X1_VLT_CUR_Mode = !TRX_X1_VLT_CUR_Mode;
	LCD_UpdateQuery.StatusInfoBar = true;
	LCD_UpdateQuery.TopButtons = true;
#endif
}

void BUTTONHANDLER_SNAP_LEFT(uint32_t parameter) { SNAP_DoSnap(false, 1); }
void BUTTONHANDLER_SNAP_RIGHT(uint32_t parameter) { SNAP_DoSnap(false, 2); }

void BUTTONHANDLER_AUTO_SNAP(uint32_t parameter) {
	TRX.Auto_Snap = !TRX.Auto_Snap;

	if (TRX.Auto_Snap) {
		LCD_showTooltip("AUTO SNAP ON");
	} else {
		LCD_showTooltip("AUTO SNAP OFF");
	}

	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
}

void BUTTONHANDLER_CESSB(uint32_t parameter) {
	TRX.TX_CESSB = !TRX.TX_CESSB;

	if (TRX.TX_CESSB) {
		LCD_showTooltip("CESSB ON");
	} else {
		LCD_showTooltip("CESSB OFF");
	}

	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
}

void BUTTONHANDLER_CESSB_HOLD(uint32_t parameter) {
	if (!LCD_systemMenuOpened) {
		LCD_systemMenuOpened = true;
		SYSMENU_TX_CESSB_HOTKEY();
	} else {
		SYSMENU_eventCloseAllSystemMenu();
	}
}

void BUTTONHANDLER_DPD(uint32_t parameter) {
	TRX.Digital_Pre_Distortion = !TRX.Digital_Pre_Distortion;

	if (TRX.Digital_Pre_Distortion) {
		LCD_showTooltip("DPD ON");
	} else {
		LCD_showTooltip("DPD OFF");
	}

	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
}

void BUTTONHANDLER_DPD_CALIBRATE(uint32_t parameter) {
	if (!TRX.Digital_Pre_Distortion) {
		BUTTONHANDLER_DPD(0);
	}

	LCD_showTooltip("DPD CALIBRATION");
	DPD_StartCalibration();
}

void BUTTONHANDLER_SCREENSHOT(uint32_t parameter) {
#if HRDW_HAS_SD
	FILEMANAGER_SCREENSHOT_handler();
#endif
}

void BUTTONHANDLER_CW_MACROS(uint32_t parameter) {
	if (CurrentVFO->Mode != TRX_MODE_CW) {
		LCD_showTooltip("CW Macro");
		return;
	}

	if (CW_Process_Macros) {
		CW_Process_Macros = false;
		TRX_ptt_soft = false;
		CW_key_serial = false;
		return;
	}

	switch (parameter) {
	case 1:
		CW_InitMacros(TRX.CW_Macros_1);
		break;
	case 2:
		CW_InitMacros(TRX.CW_Macros_2);
		break;
	case 3:
		CW_InitMacros(TRX.CW_Macros_3);
		break;
	case 4:
		CW_InitMacros(TRX.CW_Macros_4);
		break;
	case 5:
	default:
		CW_InitMacros(TRX.CW_Macros_5);
	}

	TRX_TX_StartTime = HAL_GetTick();
	CW_Process_Macros = true;
	TRX_ptt_soft = true;
	CW_key_serial = true;
}

void BUTTONHANDLER_Free_tune(uint32_t parameter) {
	// Free Tune or Center
	TRX.FREE_Tune = !TRX.FREE_Tune;
	TRX.FREE_Tune ? LCD_showTooltip("Free Tune") : LCD_showTooltip("Center");
	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
}
