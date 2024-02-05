#ifndef VAD_h
#define VAD_h

#include "audio_processor.h"
#include "hardware.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define VAD_BLOCK_SIZE 64 // block size for processing in VAD
#define VAD_MAGNIFY 8     // FFT Zooming
#define VAD_FFT_SIZE 128  // FFT size for processing in VAD
#define VAD_FFT_SIZE_HALF (VAD_FFT_SIZE / 2)
#define VAD_ZOOMED_SAMPLES (VAD_BLOCK_SIZE / VAD_MAGNIFY)
#define VAD_DECIMATE_NUM_TAPS 4

typedef struct {
	float32_t FFTBufferCharge[VAD_FFT_SIZE * 2]; // cumulative buffer
	float32_t decimState[VAD_BLOCK_SIZE + 4 - 1];
	arm_fir_decimate_instance_f32 FirDecimate;

	float32_t Min_E1;
	float32_t Min_E2;
	float32_t Min_MD1;
	float32_t Min_MD2;
	uint32_t start_counter;

	bool state;
	uint16_t state_no_counter;
	uint16_t state_yes_counter;
} VAD_Instance;

// Public methods
extern void InitVAD(void);                                          // initialize VAD
extern void processVAD(AUDIO_PROC_RX_NUM rx_id, float32_t *buffer); // run VAD for the data block
extern void resetVAD(void);                                         // reset VAD status

// extern float32_t VAD_FFTBuffer_Export [VAD_FFT_SIZE];	//debug buffer
extern volatile bool VAD_RX1_Muting; // Muting flag
extern volatile bool VAD_RX2_Muting; // Muting flag

#endif
