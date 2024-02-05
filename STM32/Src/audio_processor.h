#ifndef AUDIO_PROCESSOR_h
#define AUDIO_PROCESSOR_h

#include "functions.h"
#include "settings.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define FM_TX_HPF_ALPHA 0.95f         // For FM modulator: "Alpha" (high-pass) factor to pre-emphasis
#define FM_SQUELCH_HYSTERESIS 0.3f    // Hysteresis for FM squelch
#define FM_SQUELCH_PROC_DECIMATION 10 // Number of times we go through the FM demod algorithm before we do a squelch calculation
#define FM_RX_SQL_SMOOTHING 0.25f     // Smoothing factor for IIR squelch noise averaging
#define FM_RX_SQL_TIMEOUT_MS 20       // time to open/close squelch
#define AUDIO_MAX_REVERBER_TAPS 10
#define TWO_TONE_GEN_FREQ1 1000 // two tone generator freq, Hz
#define TWO_TONE_GEN_FREQ2 2200 // two tone generator freq, Hz

// SAM
#define SAM_PLL_HILBERT_STAGES 7
#define SAM_OUT_IDX (3 * SAM_PLL_HILBERT_STAGES)
#define SAM_omegaN 300.0f // PLL bandwidth 50.0 - 1000.0
#define SAM_zeta 0.7f     // PLL step response: smaller, slower response 1.0 - 0.1

// Stereo WFM
#define SWFM_PILOT_TONE_FREQ 19000.0f
#define SWFM_STEREO_PILOT_TONE_FREQ (2.0f * SWFM_PILOT_TONE_FREQ)
#define SWFM_STEREO_AUDIO_HALF_WIDTH 14000.0f

typedef struct {
	bool inited;
	uint16_t count;

	float32_t fil_out;
	float32_t lowpass;
	float32_t omega2;
	float32_t phs;

	float32_t dsI; // delayed sample, I path
	float32_t dsQ; // delayed sample, Q path

	float32_t a[SAM_OUT_IDX + 3]; // Filter a variables
	float32_t b[SAM_OUT_IDX + 3]; // Filter b variables
	float32_t c[SAM_OUT_IDX + 3]; // Filter c variables
	float32_t d[SAM_OUT_IDX + 3]; // Filter d variables

	float32_t adb_sam_g1;
	float32_t adb_sam_g2;
	float32_t adb_sam_omega_min;
	float32_t adb_sam_omega_max;

	float32_t adb_sam_mtauR;
	float32_t adb_sam_onem_mtauR;
	float32_t adb_sam_mtauI;
	float32_t adb_sam_onem_mtauI;
} demod_sam_data_t;

typedef struct {
	const float32_t c0[SAM_PLL_HILBERT_STAGES]; // Filter coefficients - path 0
	const float32_t c1[SAM_PLL_HILBERT_STAGES]; // Filter coefficients - path 1
} demod_sam_const_t;
//

typedef struct {
	float32_t stereo_fm_pilot_out[FPGA_RX_IQ_BUFFER_HALF_SIZE];
	float32_t stereo_fm_audio_out[FPGA_RX_IQ_BUFFER_HALF_SIZE];
	float32_t i_prev;
	float32_t q_prev;

	float32_t squelchRate;
	uint32_t squelchSuggested_starttime;
	bool squelchSuggested;
	bool squelched;
} demod_fm_instance;

typedef enum // receiver number
{ AUDIO_RX1,
#if HRDW_HAS_DUAL_RX
	AUDIO_RX2
#endif
} AUDIO_PROC_RX_NUM;

// Public variables
extern volatile uint32_t AUDIOPROC_samples;                             // audio samples processed in the processor
extern volatile bool Processor_NeedRXBuffer;                            // codec needs data from processor for RX
extern volatile bool Processor_NeedTXBuffer;                            // codec needs data from processor for TX
extern float32_t APROC_Audio_Buffer_RX1_Q[FPGA_RX_IQ_BUFFER_HALF_SIZE]; // copy of the working part of the FPGA buffers for processing
extern float32_t APROC_Audio_Buffer_RX1_I[FPGA_RX_IQ_BUFFER_HALF_SIZE];
#if HRDW_HAS_DUAL_RX
extern float32_t APROC_Audio_Buffer_RX2_Q[FPGA_RX_IQ_BUFFER_HALF_SIZE];
extern float32_t APROC_Audio_Buffer_RX2_I[FPGA_RX_IQ_BUFFER_HALF_SIZE];
#endif
extern float32_t APROC_Audio_Buffer_TX_Q[FPGA_TX_IQ_BUFFER_HALF_SIZE];
extern float32_t APROC_Audio_Buffer_TX_I[FPGA_TX_IQ_BUFFER_HALF_SIZE];
extern volatile float32_t Processor_TX_MAX_amplitude_OUT; // TX uplift after ALC
extern volatile float32_t Processor_RX1_Power_value;      // RX signal magnitude
extern volatile float32_t Processor_RX2_Power_value;      // RX signal magnitude
extern bool NeedReinitReverber;
extern bool APROC_IFGain_Overflow;
extern float32_t APROC_TX_clip_gain;
extern float32_t APROC_TX_tune_power;
extern demod_fm_instance DFM_RX1;
extern demod_fm_instance DFM_RX1;
extern float32_t SAM_Carrier_offset;
extern float32_t APROC_TUNE_DigiTone_Freq;

// Public methods
extern void processRxAudio(void);     // start audio processor for RX
extern void preProcessRxAudio(void);  // start audio pre-processor for RX
extern void processTxAudio(void);     // start audio processor for TX
extern void initAudioProcessor(void); // initialize audio processor
extern void APROC_doVOX(void);

#endif
