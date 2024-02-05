#ifndef NOISE_BLANKER_h
#define NOISE_BLANKER_h

#include "audio_processor.h"
#include "hardware.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define NB_BLOCK_SIZE (AUDIO_BUFFER_HALF_SIZE / 3) // size of the NB filter processing block

// NB1
#define NB1_DELAY_STAGE 2 // buffer blocks count
#define NB1_DELAY_BUFFER_SIZE (NB_BLOCK_SIZE * NB1_DELAY_STAGE)
#define NB1_SIGNAL_SMOOTH 0.08f   // reaction filter
#define NB1_EDGES_SMOOTH 0.5f     // edges smooth filter
#define NB1_DELAY_BUFFER_ITEMS 30 // delay zero samples

// NB2
#define NB2_c1 0.92f           // averaging coefficients
#define NB2_c2 (1.0f - NB2_c1) // averaging coefficients
#define NB2_c3 0.999f          // averaging coefficients
#define NB2_c4 (1.0f - NB2_c3) // averaging coefficients
#define NB2_TAPS 32            // filter order
#define NB2_STEP 0.05f         // LMS algorithm step

typedef struct {
	// NB1
	float32_t NB1_delay_buf[NB1_DELAY_BUFFER_SIZE];
	int32_t NB1_delbuf_inptr;
	int32_t NB1_delbuf_outptr;
	float32_t NB1_agc;
	uint32_t NB1_delay;
	float32_t NB1_edge_strength;
	// NB2
	float32_t NB2_d_avgmag_nb2;
	arm_lms_norm_instance_f32 NB2_lms2_Norm_instance;
	float32_t NB2_lms2_stateF32[NB2_TAPS + 1];
	float32_t NB2_lms2_normCoeff_f32[NB2_TAPS];
	float32_t NB2_lms2_err;
} NB_Instance;

// Public methods
extern void NB_Init(void);
extern void processNoiseBlanking(float32_t *buffer, AUDIO_PROC_RX_NUM rx_id); // start NB for the data block

#endif
