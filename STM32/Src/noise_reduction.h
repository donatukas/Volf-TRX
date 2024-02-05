#ifndef NOISE_REDUCTION_h
#define NOISE_REDUCTION_h

#include "audio_processor.h"
#include "hardware.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#ifdef STM32H743xx
#define NOISE_REDUCTION_FFT_SIZE 512                         // FFT size for processing in DNR
#define NOISE_REDUCTION_FFT_INSTANCE &arm_cfft_sR_f32_len512 // FFT instance
#endif

#ifdef STM32F407xx
#define NOISE_REDUCTION_FFT_SIZE 256                         // FFT size for processing in DNR
#define NOISE_REDUCTION_FFT_INSTANCE &arm_cfft_sR_f32_len256 // FFT instance
#endif

#define NOISE_REDUCTION_BLOCK_SIZE 64 // block size for processing in DNR
#define NOISE_REDUCTION_FFT_SIZE_HALF (NOISE_REDUCTION_FFT_SIZE / 2)
#define NOISE_REDUCTION_ALPHA 0.98f // time averaging constant
#define NOISE_REDUCTION_BETA 0.25f  // frequency averaging constant

typedef struct {
	float32_t InputBuffer[NOISE_REDUCTION_FFT_SIZE];
	uint16_t InputBuffer_index;
	float32_t OutputBuffer[NOISE_REDUCTION_FFT_SIZE];
	uint16_t OutputBuffer_index;
	float32_t Prev_Buffer[NOISE_REDUCTION_FFT_SIZE_HALF];

	float32_t FFT_Buffer[NOISE_REDUCTION_FFT_SIZE * 2];
	const arm_cfft_instance_f32 *FFT_Inst;
	float32_t FFT_COMPLEX_MAG[NOISE_REDUCTION_FFT_SIZE_HALF];
	float32_t FFT_AVERAGE_MAG[NOISE_REDUCTION_FFT_SIZE_HALF];
	float32_t FFT_MINIMUM_MAG[NOISE_REDUCTION_FFT_SIZE_HALF];
	float32_t GAIN[NOISE_REDUCTION_FFT_SIZE_HALF];
	float32_t LAST_IFFT_RESULT[NOISE_REDUCTION_FFT_SIZE_HALF];

	// DNR2
	float32_t GAIN_old[NOISE_REDUCTION_FFT_SIZE_HALF];
	float32_t SNR_post[NOISE_REDUCTION_FFT_SIZE_HALF];
	float32_t SNR_prio[NOISE_REDUCTION_FFT_SIZE_HALF];

	// AGC
	float32_t need_gain_dB;
	uint32_t last_agc_peak_time;
	uint16_t hold_time;
} NR_Instance;

// Public methods
extern void InitNoiseReduction(void);  // initialize DNR
extern void CleanNoiseReduction(void); // Clean old DNR data
extern void processNoiseReduction(float32_t *buffer, AUDIO_PROC_RX_NUM rx_id, uint8_t nr_type, uint_fast8_t mode,
                                  bool do_agc); // run DNR for the data block

#endif
