#include "noise_blanker.h"

// Private variables
static NB_Instance NB_RX1 = {};

#if HRDW_HAS_DUAL_RX
SRAM4 static NB_Instance NB_RX2 = {};
#endif

void NB_Init(void) {
	dma_memset(&NB_RX1, 0x00, sizeof NB_RX1);
	// NB1
	NB_RX1.NB1_delbuf_inptr = 0;
	NB_RX1.NB1_delbuf_outptr = NB1_DELAY_STAGE;
	NB_RX1.NB1_edge_strength = 1.0f;
	// NB2
	arm_lms_norm_init_f32(&NB_RX1.NB2_lms2_Norm_instance, NB2_TAPS, NB_RX1.NB2_lms2_normCoeff_f32, NB_RX1.NB2_lms2_stateF32, NB2_STEP, 1);

#if HRDW_HAS_DUAL_RX
	dma_memset(&NB_RX2, 0x00, sizeof NB_RX2);
	// NB1
	NB_RX2.NB1_delbuf_inptr = 0;
	NB_RX2.NB1_delbuf_outptr = NB1_DELAY_STAGE;
	NB_RX2.NB1_edge_strength = 1.0f;
	// NB2
	arm_lms_norm_init_f32(&NB_RX2.NB2_lms2_Norm_instance, NB2_TAPS, NB_RX2.NB2_lms2_normCoeff_f32, NB_RX2.NB2_lms2_stateF32, NB2_STEP, 1);
#endif
}

void processNoiseBlanking(float32_t *buffer, AUDIO_PROC_RX_NUM rx_id) {
	NB_Instance *instance = &NB_RX1;
#if HRDW_HAS_DUAL_RX
	if (rx_id == AUDIO_RX2) {
		instance = &NB_RX2;
	}
#endif

	// NB1 with muting
	if (TRX.NOISE_BLANKER1) {
		float32_t nb_short_setting = (float32_t)TRX.NOISE_BLANKER1_THRESHOLD / 2.0f;

		float32_t nb_sig_filt = NB1_SIGNAL_SMOOTH;  // de-linearize and save in "new signal" contribution parameter
		float32_t nb_agc_filt = 1.0f - nb_sig_filt; // calculate parameter for recyling "old" AGC value

		for (uint32_t i = 0; i < NB_BLOCK_SIZE; i++) // Noise blanker function
		{
			float32_t sig = fabsf(buffer[i]);                                  // get signal amplitude.  We need only look at one of the two audio channels since they will be the same.
			instance->NB1_delay_buf[instance->NB1_delbuf_inptr++] = buffer[i]; // copy first byte into delay buffer

			instance->NB1_agc = (nb_agc_filt * instance->NB1_agc) + (nb_sig_filt * sig); // IIR-filtered "AGC" of current overall signal level

			// println("SIG: ", (double)sig, " TH: ", (double)(instance->nb_agc * (((NB_MAX_SETTING / 20.0f) + 1.75f) - nb_short_setting)));

			if ((sig > (instance->NB1_agc * (((10.0f / 2.0f) + 1.75f) - nb_short_setting))) && (instance->NB1_delay == 0)) // did a pulse exceed the threshold?
			{
				instance->NB1_delay = NB1_DELAY_BUFFER_ITEMS; // yes - set the blanking duration counter
			}

			if (!instance->NB1_delay) // blank counter not active
			{
				buffer[i] = instance->NB1_delay_buf[instance->NB1_delbuf_outptr++] * instance->NB1_edge_strength; // pass through delayed audio, unchanged
				instance->NB1_edge_strength = 1.0f * NB1_EDGES_SMOOTH + (instance->NB1_edge_strength * (1.0f - NB1_EDGES_SMOOTH));
			} else // It is within the blanking pulse period
			{
				buffer[i] = instance->NB1_edge_strength * buffer[i]; // set the audio buffer to "mute" during the blanking period
				instance->NB1_edge_strength = instance->NB1_edge_strength * NB1_EDGES_SMOOTH;
				instance->NB1_delay--; // count down the number of samples that we are to blank
			}

			// RINGBUFFER
			instance->NB1_delbuf_outptr %= NB1_DELAY_BUFFER_SIZE;
			instance->NB1_delbuf_inptr %= NB1_DELAY_BUFFER_SIZE;
		}
	}

	// NB2 with LMS
	if (TRX.NOISE_BLANKER2) {
		static float32_t last_value_in_buffer = 0.0f;
		float32_t d_thld_nb2 = (float32_t)(20 - TRX.NOISE_BLANKER2_THRESHOLD) / 5.0f;

		for (int i = 0; i < NB_BLOCK_SIZE; i++) {
			float32_t cmag = fabsf(buffer[i]);
			instance->NB2_d_avgmag_nb2 = NB2_c3 * instance->NB2_d_avgmag_nb2 + NB2_c4 * cmag;

			if (cmag > d_thld_nb2 * instance->NB2_d_avgmag_nb2) {
				if (i > 0) {
					arm_lms_norm_f32(&instance->NB2_lms2_Norm_instance, &buffer[i], &buffer[i - 1], &buffer[i], &instance->NB2_lms2_err, 1);
				} else {
					arm_lms_norm_f32(&instance->NB2_lms2_Norm_instance, &buffer[i], &last_value_in_buffer, &buffer[i], &instance->NB2_lms2_err, 1);
				}

				if (isnanf(buffer[i]) || isinff(buffer[i])) {
					buffer[i] = 0.0f;
				}
			}
		}
		last_value_in_buffer = buffer[NB_BLOCK_SIZE - 1];

		// overflow protect
		float32_t maxValOut;
		arm_max_no_idx_f32(instance->NB2_lms2_Norm_instance.pCoeffs, NB2_TAPS, &maxValOut);
		if (maxValOut > 1.0f) {
			NB_Init();
		}
	}
}
