#include "auto_notch.h"
#include "trx_manager.h"

// Private variables
static AN_Instance RX1_AN_instance = {0}; // filter instances for two receivers
#if HRDW_HAS_DUAL_RX
IRAM2 static AN_Instance RX2_AN_instance = {0};
#endif

// initialize the automatic notch filter
void InitAutoNotchReduction(void) {
	dma_memset(&RX1_AN_instance, 0x00, sizeof RX1_AN_instance);
	arm_lms_norm_init_f32(&RX1_AN_instance.lms2_Norm_instance, AUTO_NOTCH_TAPS, RX1_AN_instance.lms2_normCoeff_f32, RX1_AN_instance.lms2_stateF32, AUTO_NOTCH_STEP, AUTO_NOTCH_BLOCK_SIZE);

#if HRDW_HAS_DUAL_RX
	dma_memset(&RX2_AN_instance, 0x00, sizeof RX2_AN_instance);
	arm_lms_norm_init_f32(&RX2_AN_instance.lms2_Norm_instance, AUTO_NOTCH_TAPS, RX2_AN_instance.lms2_normCoeff_f32, RX2_AN_instance.lms2_stateF32, AUTO_NOTCH_STEP, AUTO_NOTCH_BLOCK_SIZE);
#endif
}

// start automatic notch filter
void processAutoNotchReduction(float32_t *buffer, AUDIO_PROC_RX_NUM rx_id) {
	// overflow protect
	static uint32_t temporary_stop = 0;
	if (temporary_stop > 0) {
		temporary_stop--;
		return;
	}

	AN_Instance *instance = &RX1_AN_instance;
#if HRDW_HAS_DUAL_RX
	if (rx_id == AUDIO_RX2) {
		instance = &RX2_AN_instance;
	}
#endif

	dma_memcpy(&instance->lms2_reference[instance->reference_index_new], buffer,
	           sizeof(float32_t) * AUTO_NOTCH_BLOCK_SIZE); // save the data to the reference buffer
	arm_lms_norm_f32(&instance->lms2_Norm_instance, buffer, &instance->lms2_reference[instance->reference_index_old], instance->lms2_errsig2, buffer,
	                 AUTO_NOTCH_BLOCK_SIZE); // start LMS filter

	// overflow protect
	float32_t minValOut = 0;
	float32_t maxValOut = 0;
	arm_min_no_idx_f32(buffer, AUTO_NOTCH_BLOCK_SIZE, &minValOut);
	arm_max_no_idx_f32(buffer, AUTO_NOTCH_BLOCK_SIZE, &maxValOut);
	if (isnanf(minValOut) || isinff(minValOut) || isnanf(maxValOut) || isinff(maxValOut)) {
		if (AUTO_NOTCH_DEBUG) {
			println("auto notch err ", (double)minValOut, " ", (double)maxValOut);
		}
		InitAutoNotchReduction();
		dma_memset(buffer, 0x00, sizeof(float32_t) * AUTO_NOTCH_BLOCK_SIZE);
		temporary_stop = 500;
	}
	arm_max_no_idx_f32(instance->lms2_Norm_instance.pCoeffs, AUTO_NOTCH_TAPS, &maxValOut);
	if (maxValOut > 1.0f) {
		if (AUTO_NOTCH_DEBUG) {
			println("auto notch reset", (double)maxValOut);
		}
		InitAutoNotchReduction();
		temporary_stop = 500;
	}

	instance->reference_index_old += AUTO_NOTCH_BLOCK_SIZE; // move along the reference buffer
	if (instance->reference_index_old >= AUTO_NOTCH_REFERENCE_SIZE) {
		instance->reference_index_old = 0;
	}
	instance->reference_index_new = instance->reference_index_old + AUTO_NOTCH_BLOCK_SIZE;
	if (instance->reference_index_new >= AUTO_NOTCH_REFERENCE_SIZE) {
		instance->reference_index_new = 0;
	}
}
