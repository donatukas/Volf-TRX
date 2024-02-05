#ifndef AUDIO_FILTERS_h
#define AUDIO_FILTERS_h

#include "BiquadDesigner/biquad.h"
#include "fpga.h"
#include "functions.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#ifdef STM32F407xx
#define IQ_HILBERT_TAPS_RX 101                 // Hilbert filter order RX
#define IQ_HILBERT_TAPS_TX 201                 // Hilbert filter order TX
#define IIR_LPF_STAGES IIR_BIQUAD_MAX_SECTIONS // order of IIR LPF filters
#define IIR_HPF_STAGES 3                       // order of IIR HPF filters
#define IIR_DECIMATOR_FILTER_STAGES 5          // order of decimator filter
#else
#define IQ_HILBERT_TAPS_RX 201                 // Hilbert filter order RX
#define IQ_HILBERT_TAPS_TX 501                 // Hilbert filter order TX
#define IIR_LPF_STAGES IIR_BIQUAD_MAX_SECTIONS // order of IIR LPF filters (12)
#define IIR_HPF_STAGES 5                       // order of IIR HPF filters
#define IIR_DECIMATOR_FILTER_STAGES 9          // order of decimator filter
#endif

#define NOTCH_STAGES 3          // order of manual Notch filter
#define EQ_STAGES 1             // order of the biquad of the equalizer filter
#define GAUSS_STAGES 1          // order of the gauss CW filter
#define BIQUAD_COEFF_IN_STAGE 5 // coefficients in manual Notch filter order
#define SFM_FILTER_STAGES 3
#define FIR_RX_HILBERT_STATE_SIZE (IQ_HILBERT_TAPS_RX + AUDIO_BUFFER_HALF_SIZE - 1) // size of state buffers
#define FIR_TX_HILBERT_STATE_SIZE (IQ_HILBERT_TAPS_TX + AUDIO_BUFFER_HALF_SIZE - 1)
#define IIR_RX_LPF_Taps_STATE_SIZE (IIR_LPF_STAGES * 2)
#define IIR_RX_GAUSS_Taps_STATE_SIZE (GAUSS_STAGES * 2)
#define IIR_TX_LPF_Taps_STATE_SIZE (IIR_LPF_STAGES * 2)
#define IIR_RX_HPF_Taps_STATE_SIZE (IIR_HPF_STAGES * 2)
#define IIR_TX_HPF_Taps_STATE_SIZE (IIR_HPF_STAGES * 2)
#define IIR_RX_HPF_SQL_STATE_SIZE (IIR_HPF_STAGES * 2)

#define CW_HPF_COUNT 8
#define SSB_HPF_COUNT 8
#define CW_LPF_COUNT 24
#define SSB_LPF_COUNT 12
#define AM_LPF_COUNT 20
#define NFM_LPF_COUNT 12

#define MAX_HPF_WIDTH 3000
#define MAX_LPF_WIDTH_CW 2000
#define MAX_LPF_WIDTH_SSB 3500
#define MAX_LPF_WIDTH_AM 20000
#define MAX_LPF_WIDTH_NFM 44000

#define CTCSS_FREQS_COUNT 66

typedef enum // BiQuad filter type for automatic calculation
{ BIQUAD_onepolelp,
	BIQUAD_onepolehp,
	BIQUAD_lowpass,
	BIQUAD_highpass,
	BIQUAD_bandpass,
	BIQUAD_notch,
	BIQUAD_peak,
	BIQUAD_lowShelf,
	BIQUAD_highShelf } BIQUAD_TYPE;

typedef enum // states of DC correctors for each user
{ DC_FILTER_RX1_I,
	DC_FILTER_RX1_Q,
#if HRDW_HAS_DUAL_RX
	DC_FILTER_RX2_I,
	DC_FILTER_RX2_Q,
#endif
	DC_FILTER_TX_I,
	DC_FILTER_TX_Q,
	DC_FILTER_FFT_I,
	DC_FILTER_FFT_Q,
} DC_FILTER_STATE;

typedef struct // keep old sample values for DC filter. Multiple states for different consumers
{
	float32_t x_prev;
	float32_t y_prev;
} DC_filter_state_type;

// Public variables
extern arm_fir_instance_f32 FIR_RX1_Hilbert_I; // filter instances
extern arm_fir_instance_f32 FIR_RX1_Hilbert_Q;
extern arm_fir_instance_f32 FIR_TX_Hilbert_90;
extern arm_biquad_cascade_df2T_instance_f32 IIR_RX1_LPF_I;
extern arm_biquad_cascade_df2T_instance_f32 IIR_RX1_LPF_Q;
extern arm_biquad_cascade_df2T_instance_f32 IIR_RX1_LPF2_I;
extern arm_biquad_cascade_df2T_instance_f32 IIR_RX1_LPF2_Q;
extern arm_biquad_cascade_df2T_instance_f32 IIR_RX1_GAUSS_I;
extern arm_biquad_cascade_df2T_instance_f32 IIR_RX1_GAUSS_Q;
extern arm_biquad_cascade_df2T_instance_f32 IIR_TX_LPF_I;
extern arm_biquad_cascade_df2T_instance_f32 IIR_TX_LPF_CESSB_I;
extern arm_biquad_cascade_df2T_instance_f32 IIR_TX_LPF_CESSB_Q;
extern arm_biquad_cascade_df2T_instance_f32 IIR_RX1_HPF_I;
extern arm_biquad_cascade_df2T_instance_f32 IIR_RX1_HPF_Q;
extern arm_biquad_cascade_df2T_instance_f32 IIR_TX_HPF_I;
extern arm_biquad_cascade_df2T_instance_f32 IIR_TX_HPF_Q;
extern arm_biquad_cascade_df2T_instance_f32 NOTCH_RX1_FILTER;
extern arm_biquad_cascade_df2T_instance_f32 EQ_RX_I_P1_FILTER;
extern arm_biquad_cascade_df2T_instance_f32 EQ_RX_I_P2_FILTER;
extern arm_biquad_cascade_df2T_instance_f32 EQ_RX_I_P3_FILTER;
extern arm_biquad_cascade_df2T_instance_f32 EQ_RX_I_P4_FILTER;
extern arm_biquad_cascade_df2T_instance_f32 EQ_RX_I_P5_FILTER;
extern arm_biquad_cascade_df2T_instance_f32 EQ_RX_I_P6_FILTER;
extern arm_biquad_cascade_df2T_instance_f32 EQ_RX_Q_P1_FILTER;
extern arm_biquad_cascade_df2T_instance_f32 EQ_RX_Q_P2_FILTER;
extern arm_biquad_cascade_df2T_instance_f32 EQ_RX_Q_P3_FILTER;
extern arm_biquad_cascade_df2T_instance_f32 EQ_RX_Q_P4_FILTER;
extern arm_biquad_cascade_df2T_instance_f32 EQ_RX_Q_P5_FILTER;
extern arm_biquad_cascade_df2T_instance_f32 EQ_RX_Q_P6_FILTER;
extern arm_biquad_cascade_df2T_instance_f32 EQ_MIC_P1_FILTER_SSB;
extern arm_biquad_cascade_df2T_instance_f32 EQ_MIC_P2_FILTER_SSB;
extern arm_biquad_cascade_df2T_instance_f32 EQ_MIC_P3_FILTER_SSB;
extern arm_biquad_cascade_df2T_instance_f32 EQ_MIC_P4_FILTER_SSB;
extern arm_biquad_cascade_df2T_instance_f32 EQ_MIC_P5_FILTER_SSB;
extern arm_biquad_cascade_df2T_instance_f32 EQ_MIC_P6_FILTER_SSB;
extern arm_biquad_cascade_df2T_instance_f32 EQ_MIC_P1_FILTER_AMFM;
extern arm_biquad_cascade_df2T_instance_f32 EQ_MIC_P2_FILTER_AMFM;
extern arm_biquad_cascade_df2T_instance_f32 EQ_MIC_P3_FILTER_AMFM;
extern arm_biquad_cascade_df2T_instance_f32 EQ_MIC_P4_FILTER_AMFM;
extern arm_biquad_cascade_df2T_instance_f32 EQ_MIC_P5_FILTER_AMFM;
extern arm_biquad_cascade_df2T_instance_f32 EQ_MIC_P6_FILTER_AMFM;
extern arm_biquad_cascade_df2T_instance_f32 AGC_RX1_KW_HSHELF_FILTER;
extern arm_biquad_cascade_df2T_instance_f32 AGC_RX1_KW_HPASS_FILTER;
extern arm_fir_decimate_instance_f32 DECIMATE_FIR_RX1_AUDIO_I;
extern arm_fir_decimate_instance_f32 DECIMATE_FIR_RX1_AUDIO_Q;
extern arm_biquad_cascade_df2T_instance_f32 DECIMATE_IIR_RX1_AUDIO_I;
extern arm_biquad_cascade_df2T_instance_f32 DECIMATE_IIR_RX1_AUDIO_Q;
extern arm_biquad_cascade_df2T_instance_f32 SFM_RX1_Pilot_Filter;
extern arm_biquad_cascade_df2T_instance_f32 SFM_RX1_Audio_Filter;
extern volatile bool NeedReinitNotch;             // need to reinitialize the manual Notch filter
extern volatile bool NeedReinitAudioFilters;      // need to reinitialize the Audio filters
extern volatile bool NeedReinitAudioFiltersClean; // also clean state
extern const uint32_t AUTIO_FILTERS_HPF_CW_LIST[CW_HPF_COUNT];
extern const uint32_t AUTIO_FILTERS_HPF_SSB_LIST[SSB_HPF_COUNT];
extern const uint32_t AUTIO_FILTERS_LPF_CW_LIST[CW_LPF_COUNT];
extern const uint32_t AUTIO_FILTERS_LPF_SSB_LIST[SSB_LPF_COUNT];
extern const uint32_t AUTIO_FILTERS_LPF_AM_LIST[AM_LPF_COUNT];
extern const uint32_t AUTIO_FILTERS_LPF_NFM_LIST[NFM_LPF_COUNT];
extern float32_t CTCSS_Freqs[CTCSS_FREQS_COUNT];

#if HRDW_HAS_DUAL_RX
extern arm_fir_instance_f32 FIR_RX2_Hilbert_I;
extern arm_fir_instance_f32 FIR_RX2_Hilbert_Q;
extern arm_biquad_cascade_df2T_instance_f32 IIR_RX2_LPF_I;
extern arm_biquad_cascade_df2T_instance_f32 IIR_RX2_LPF_Q;
extern arm_biquad_cascade_df2T_instance_f32 IIR_RX2_LPF2_I;
extern arm_biquad_cascade_df2T_instance_f32 IIR_RX2_LPF2_Q;
extern arm_biquad_cascade_df2T_instance_f32 IIR_RX2_GAUSS_I;
extern arm_biquad_cascade_df2T_instance_f32 IIR_RX2_GAUSS_Q;
extern arm_biquad_cascade_df2T_instance_f32 IIR_RX2_HPF_I;
extern arm_biquad_cascade_df2T_instance_f32 IIR_RX2_HPF_Q;
extern arm_biquad_cascade_df2T_instance_f32 NOTCH_RX2_FILTER;
extern arm_biquad_cascade_df2T_instance_f32 AGC_RX2_KW_HSHELF_FILTER;
extern arm_biquad_cascade_df2T_instance_f32 AGC_RX2_KW_HPASS_FILTER;
extern arm_fir_decimate_instance_f32 DECIMATE_FIR_RX2_AUDIO_I;
extern arm_fir_decimate_instance_f32 DECIMATE_FIR_RX2_AUDIO_Q;
extern arm_biquad_cascade_df2T_instance_f32 DECIMATE_IIR_RX2_AUDIO_I;
extern arm_biquad_cascade_df2T_instance_f32 DECIMATE_IIR_RX2_AUDIO_Q;
extern arm_biquad_cascade_df2T_instance_f32 SFM_RX2_Pilot_Filter;
extern arm_biquad_cascade_df2T_instance_f32 SFM_RX2_Audio_Filter;
#endif

// Public methods
extern void InitAudioFilters(void);                                                    // initialize audio filters
extern void ReinitAudioFilters(void);                                                  // reinitialize audio filters
extern void InitNotchFilter(void);                                                     // initialize the manual Notch filter
extern void dc_filter(float32_t *Buffer, int16_t blockSize, DC_FILTER_STATE stateNum); // start DC corrector
extern void fill_biquad_coeffs(iir_filter_t *filter, float32_t *coeffs, uint8_t sect_num);

#endif
