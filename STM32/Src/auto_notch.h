#ifndef AUTO_NOTCH_h
#define AUTO_NOTCH_h

#include "audio_processor.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define AUTO_NOTCH_BLOCK_SIZE (AUDIO_BUFFER_HALF_SIZE / 3)    // block size for processing
#define AUTO_NOTCH_TAPS 32                                    // filter order
#define AUTO_NOTCH_REFERENCE_SIZE (AUTO_NOTCH_BLOCK_SIZE * 2) // size of the reference buffer
#define AUTO_NOTCH_STEP 0.0005f                               // LMS algorithm step
#define AUTO_NOTCH_DEBUG false

typedef struct // filter instance
{
	arm_lms_norm_instance_f32 lms2_Norm_instance;
	float32_t lms2_stateF32[AUTO_NOTCH_TAPS + AUTO_NOTCH_BLOCK_SIZE - 1];
	float32_t lms2_normCoeff_f32[AUTO_NOTCH_TAPS];
	float32_t lms2_reference[AUTO_NOTCH_REFERENCE_SIZE];
	float32_t lms2_errsig2[AUTO_NOTCH_BLOCK_SIZE];
	uint_fast16_t reference_index_old;
	uint_fast16_t reference_index_new;
} AN_Instance;

// Public methods
extern void InitAutoNotchReduction(void);                                          // initialize the automatic notch filter
extern void processAutoNotchReduction(float32_t *buffer, AUDIO_PROC_RX_NUM rx_id); // start automatic notch filter

#endif
