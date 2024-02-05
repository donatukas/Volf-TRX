#ifndef TRX_MANAGER_H
#define TRX_MANAGER_H

#include "hardware.h"
#include "settings.h"
#include <stdbool.h>

#define TRX_on_TX \
	(TRX_ptt_hard || TRX_ptt_soft || TRX_Tune || CurrentVFO->Mode == TRX_MODE_LOOPBACK || ((TRX.CW_PTT_Type == KEY_PTT || TRX.CW_PTT_Type == KEY_AND_EXT_PTT) && CW_Key_Timeout_est > 0))
#define TRX_on_RX (FULL_DUPLEX || !TRX_on_TX)
#define TRX_SLOW_SETFREQ_MIN_STEPSIZE 100 // step in Hz for slowly touchpad tuning
#define TRX_GetSamplerateByENUM(rate) (((rate) == TRX_SAMPLERATE_K48) ? 48000 : ((rate) == TRX_SAMPLERATE_K96) ? 96000 : ((rate) == TRX_SAMPLERATE_K192) ? 192000 : 384000)
#define TRX_GetRXSampleRate \
	((CurrentVFO->Mode != TRX_MODE_WFM && CurrentVFO->Mode != TRX_MODE_NFM) ? TRX_GetSamplerateByENUM(TRX.SAMPLERATE_MAIN) : TRX_GetSamplerateByENUM(TRX.SAMPLERATE_FM))
#define TRX_GetRXSampleRateENUM ((CurrentVFO->Mode != TRX_MODE_WFM && CurrentVFO->Mode != TRX_MODE_NFM) ? TRX.SAMPLERATE_MAIN : TRX.SAMPLERATE_FM)

#define NeedProcessDecoder                                                                                                                                    \
	((TRX.CW_Decoder && (CurrentVFO->Mode == TRX_MODE_CW || CurrentVFO->Mode == TRX_MODE_LOOPBACK)) || (TRX.RDS_Decoder && CurrentVFO->Mode == TRX_MODE_WFM) || \
	 CurrentVFO->Mode == TRX_MODE_RTTY)

extern void TRX_Init(void);
extern void TRX_setFrequency(uint64_t _freq, VFO *vfo);
extern void TRX_setTXFrequencyFloat(float64_t _freq, VFO *vfo); // for WSPR and other
extern void TRX_setMode(TRX_MODE _mode, VFO *vfo);
extern void TRX_ptt_change(void);
extern void TRX_DoAutoGain(void);
extern void TRX_Restart_Mode(void);
extern void TRX_TemporaryMute(void);
extern void TRX_ProcessScanMode(void);
extern void TRX_setFrequencySlowly(uint64_t target_freq);
extern void TRX_setFrequencySlowly_Process(void);
extern bool TRX_TX_Disabled(uint64_t freq);
extern void TRX_DoFrequencyEncoder(float32_t direction, bool secondary_encoder);
extern void TRX_SaveRFGain_Data(uint8_t mode, int8_t band);
extern void TRX_LoadRFGain_Data(uint8_t mode, int8_t band);
extern void TRX_RestoreBandSettings(int8_t band);

extern void BUTTONHANDLER_MODE_P(uint32_t parameter);
extern void BUTTONHANDLER_MODE_N(uint32_t parameter);
extern void BUTTONHANDLER_BAND_P(uint32_t parameter);
extern void BUTTONHANDLER_BAND_N(uint32_t parameter);
extern void BUTTONHANDLER_SAMPLE_N(uint32_t parameter);
extern void BUTTONHANDLER_SAMPLE_P(uint32_t parameter);
extern void BUTTONHANDLER_WPM(uint32_t parameter);
extern void BUTTONHANDLER_KEYER(uint32_t parameter);
extern void BUTTONHANDLER_SCAN(uint32_t parameter);
extern void BUTTONHANDLER_REC(uint32_t parameter);
extern void BUTTONHANDLER_CQ(uint32_t parameter);
extern void BUTTONHANDLER_RIT(uint32_t parameter);
extern void BUTTONHANDLER_XIT(uint32_t parameter);
extern void BUTTONHANDLER_RITXIT(uint32_t parameter);
extern void BUTTONHANDLER_SPLIT(uint32_t parameter);
extern void BUTTONHANDLER_STEP(uint32_t parameter);
extern void BUTTONHANDLER_BANDMAP(uint32_t parameter);
extern void BUTTONHANDLER_VOX(uint32_t parameter);
extern void BUTTONHANDLER_FILEMANAGER(uint32_t parameter);
extern void BUTTONHANDLER_FT8(uint32_t parameter);
extern void BUTTONHANDLER_AUTOGAINER(uint32_t parameter);
extern void BUTTONHANDLER_UP(uint32_t parameter);
extern void BUTTONHANDLER_DOWN(uint32_t parameter);
extern void BUTTONHANDLER_FUNC(uint32_t parameter);
extern void BUTTONHANDLER_FUNCH(uint32_t parameter);
extern void BUTTONHANDLER_DOUBLE(uint32_t parameter);
extern void BUTTONHANDLER_DOUBLEMODE(uint32_t parameter);
extern void BUTTONHANDLER_PRE(uint32_t parameter);
extern void BUTTONHANDLER_ATT(uint32_t parameter);
extern void BUTTONHANDLER_ATTHOLD(uint32_t parameter);
extern void BUTTONHANDLER_PGA(uint32_t parameter);
extern void BUTTONHANDLER_PGA_ONLY(uint32_t parameter);
extern void BUTTONHANDLER_DRV_ONLY(uint32_t parameter);
extern void BUTTONHANDLER_AGC(uint32_t parameter);
extern void BUTTONHANDLER_AGC_SPEED(uint32_t parameter);
extern void BUTTONHANDLER_AGC_MaxGain(uint32_t parameter);
extern void BUTTONHANDLER_REPEATER_MODE(uint32_t parameter);
extern void BUTTONHANDLER_DNR(uint32_t parameter);
extern void BUTTONHANDLER_DNR_HOLD(uint32_t parameter);
extern void BUTTONHANDLER_NB(uint32_t parameter);
extern void BUTTONHANDLER_NB_HOLD(uint32_t parameter);
extern void BUTTONHANDLER_NOTCH(uint32_t parameter);
extern void BUTTONHANDLER_NOTCH_MANUAL(uint32_t parameter);
extern void BUTTONHANDLER_NOTCH_SWITCH(uint32_t parameter);
extern void BUTTONHANDLER_FAST(uint32_t parameter);
extern void BUTTONHANDLER_MUTE(uint32_t parameter);
extern void BUTTONHANDLER_MUTE_AFAMP(uint32_t parameter);
extern void BUTTONHANDLER_BLUETOOTH_AUDIO_ENABLED(uint32_t parameter);
extern void BUTTONHANDLER_PTT_TOGGLE(uint32_t parameter);
extern void BUTTONHANDLER_AsB(uint32_t parameter);
extern void BUTTONHANDLER_ArB(uint32_t parameter);
extern void BUTTONHANDLER_TUNE(uint32_t parameter);
extern void BUTTONHANDLER_TUNER(uint32_t parameter);
extern void BUTTONHANDLER_RF_POWER(uint32_t parameter);
extern void BUTTONHANDLER_ANT(uint32_t parameter);
extern void BUTTONHANDLER_BW(uint32_t parameter);
extern void BUTTONHANDLER_HPF(uint32_t parameter);
extern void BUTTONHANDLER_SERVICES(uint32_t parameter);
extern void BUTTONHANDLER_MENU(uint32_t parameter);
extern void BUTTONHANDLER_MENUHOLD(uint32_t parameter);
extern void BUTTONHANDLER_LOCK(uint32_t parameter);
extern void BUTTONHANDLER_SET_CUR_VFO_BAND(uint32_t parameter);
extern void BUTTONHANDLER_SET_VFOA_BAND(uint32_t parameter);
extern void BUTTONHANDLER_SET_VFOB_BAND(uint32_t parameter);
extern void BUTTONHANDLER_SET_BAND_MEMORY(uint32_t parameter);
extern void BUTTONHANDLER_GET_BAND_MEMORY(uint32_t parameter);
extern void BUTTONHANDLER_SETMODE(uint32_t parameter);
extern void BUTTONHANDLER_SETSECMODE(uint32_t parameter);
extern void BUTTONHANDLER_SET_RX_BW(uint32_t parameter);
extern void BUTTONHANDLER_SET_TX_BW(uint32_t parameter);
extern void BUTTONHANDLER_SETRF_POWER(uint32_t parameter);
extern void BUTTONHANDLER_SET_ATT_DB(uint32_t parameter);
extern void BUTTONHANDLER_LEFT_ARR(uint32_t parameter);
extern void BUTTONHANDLER_RIGHT_ARR(uint32_t parameter);
extern void BUTTONHANDLER_SQL(uint32_t parameter);
extern void BUTTONHANDLER_SQUELCH(uint32_t parameter);
extern void BUTTONHANDLER_ZOOM_N(uint32_t parameter);
extern void BUTTONHANDLER_ZOOM_P(uint32_t parameter);
extern void BUTTONHANDLER_SelectMemoryChannels(uint32_t parameter);
extern void BUTTONHANDLER_SaveMemoryChannels(uint32_t parameter);
extern void BUTTONHANDLER_IF(uint32_t parameter);
extern void BUTTONHANDLER_VLT(uint32_t parameter);
extern void BUTTONHANDLER_SNAP_LEFT(uint32_t parameter);
extern void BUTTONHANDLER_SNAP_RIGHT(uint32_t parameter);
extern void BUTTONHANDLER_AUTO_SNAP(uint32_t parameter);
extern void BUTTONHANDLER_CESSB(uint32_t parameter);
extern void BUTTONHANDLER_CESSB_HOLD(uint32_t parameter);
extern void BUTTONHANDLER_SCREENSHOT(uint32_t parameter);
extern void BUTTONHANDLER_CW_MACROS(uint32_t parameter);
extern void BUTTONHANDLER_DPD(uint32_t parameter);
extern void BUTTONHANDLER_DPD_CALIBRATE(uint32_t parameter);
extern void BUTTONHANDLER_MEMO_WRITE(uint32_t parameter);
extern void BUTTONHANDLER_MEMO_READ(uint32_t parameter);
extern void BUTTONHANDLER_SET_VFOA_FREQ_MANUAL(uint32_t parameter);
extern void BUTTONHANDLER_SET_VFOB_FREQ_MANUAL(uint32_t parameter);
extern void BUTTONHANDLER_Free_tune(uint32_t parameter);

volatile extern bool TRX_ptt_hard;
volatile extern bool TRX_ptt_soft;
volatile extern bool TRX_old_ptt_soft;
volatile extern bool TRX_RX1_IQ_swap;
volatile extern bool TRX_RX2_IQ_swap;
volatile extern bool TRX_TX_IQ_swap;
volatile extern bool TRX_Tune;
volatile extern bool TRX_Inited;
volatile extern float32_t TRX_RX1_dBm;
volatile extern float32_t TRX_RX2_dBm;
volatile extern float32_t TRX_RX1_dBm_measurement;
volatile extern bool TRX_ADC_OTR;
volatile extern bool TRX_DAC_OTR;
volatile extern bool TRX_PWR_ALC_SWR_OVERFLOW;
volatile extern bool TRX_MIC_BELOW_NOISEGATE;
volatile extern int16_t TRX_ADC_MINAMPLITUDE;
volatile extern int16_t TRX_ADC_MAXAMPLITUDE;
volatile extern uint32_t TRX_SNTP_Synced;
volatile extern int_fast16_t TRX_RIT;
volatile extern int_fast16_t TRX_XIT;
volatile extern float32_t TRX_MAX_TX_Amplitude;
volatile extern float32_t TRX_PWR_Forward;
volatile extern float32_t TRX_PWR_Backward;
volatile extern float32_t TRX_SWR;
volatile extern float32_t TRX_PWR_Forward_SMOOTHED;
volatile extern float32_t TRX_PWR_Backward_SMOOTHED;
volatile extern float32_t TRX_SWR_SMOOTHED;
extern char TRX_SWR_SMOOTHED_STR[12];
volatile extern float32_t TRX_VLT_forward;
volatile extern float32_t TRX_VLT_backward;
volatile extern float32_t TRX_ALC_IN;
volatile extern float32_t TRX_ALC_OUT;
volatile extern bool TRX_SWR_PROTECTOR;
volatile extern bool TRX_DAC_DIV0;
volatile extern bool TRX_DAC_DIV1;
volatile extern bool TRX_DAC_HP1;
volatile extern bool TRX_DAC_HP2;
volatile extern bool TRX_DAC_X4;
volatile extern bool TRX_DCDC_Freq;
volatile extern bool TRX_DAC_DRV_A0;
volatile extern bool TRX_DAC_DRV_A1;
volatile extern float32_t TRX_STM32_VREF;
volatile extern float32_t TRX_STM32_TEMPERATURE;
volatile extern float32_t TRX_IQ_phase_error;
volatile extern bool TRX_Temporary_Stop_BandMap;
volatile extern float32_t TRX_RF_Temperature;
extern const char *MODE_DESCR[];
extern uint32_t TRX_freq_phrase;
extern uint32_t TRX_freq_phrase2;
extern uint32_t TRX_freq_phrase_tx;
volatile extern uint32_t TRX_Temporary_Mute_StartTime;
volatile extern bool TRX_ScanMode;
extern bool TRX_phase_restarted;
extern uint32_t TRX_TX_StartTime;
extern uint32_t TRX_TX_EndTime;
extern uint32_t TRX_DXCluster_UpdateTime;
extern uint32_t TRX_SAT_UpdateTime;
extern uint32_t TRX_WOLF_Cluster_UpdateTime;
volatile extern float32_t TRX_PWR_Voltage;
volatile extern float32_t TRX_PWR_Current;
volatile extern float32_t TRX_RF_Current;
volatile extern float32_t TRX_VBAT_Voltage;
volatile extern float32_t TRX_NoiseFloor;
extern uint32_t TRX_Inactive_Time;
volatile extern uint_fast16_t CW_Key_Timeout_est;
extern uint32_t dbg_FPGA_samples;
extern uint8_t TRX_TX_Harmonic;
extern uint8_t TRX_TX_sendZeroes;
extern int8_t TRX_MemoryChannelSelected;
volatile extern bool TRX_REPEATER_Applied;

#endif
