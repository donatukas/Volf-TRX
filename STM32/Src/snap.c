#include "snap.h"
#include "fft.h"
#include "lcd.h"
#include "trx_manager.h"

static arm_sort_instance_f32 SNAP_SortInstance;
SRAM_ON_F407 SRAM4_ON_H743 static float32_t SNAP_buffer[FFT_SIZE] = {0};
SRAM4_ON_H743 static float32_t SNAP_buffer_tmp[FFT_SIZE] = {0};
static bool SNAP_need_buffer = false;
static bool SNAP_process_from_auto = false;
static uint8_t SNAP_process_mode = 0; // mode 0 - unidirectional, 1 - left, 2 - right
static uint8_t SNAP_buffer_avg_index = 0;

static void SNAP_Process();

void SNAP_FillBuffer(float32_t *buff) {
	if (!SNAP_need_buffer) {
		return;
	}

	if (CurrentVFO->Mode == TRX_MODE_LOOPBACK || CurrentVFO->Mode == TRX_MODE_WFM || CurrentVFO->Mode == TRX_MODE_IQ) {
		SNAP_buffer_avg_index = 0;
		SNAP_need_buffer = false;
		return;
	}

	if (SNAP_process_from_auto && CurrentVFO->Mode != TRX_MODE_CW && CurrentVFO->Mode != TRX_MODE_NFM) {
		SNAP_buffer_avg_index = 0;
		SNAP_need_buffer = false;
		return;
	}

	if (SNAP_buffer_avg_index == 0) {
		dma_memset(SNAP_buffer, 0x00, sizeof(SNAP_buffer));
	}

	arm_add_f32(buff, SNAP_buffer, SNAP_buffer, FFT_SIZE);
	SNAP_buffer_avg_index++;

	if (SNAP_buffer_avg_index >= SNAP_AVERAGING) {
		arm_scale_f32(SNAP_buffer, 1.0f / (float32_t)SNAP_AVERAGING, SNAP_buffer, FFT_SIZE);
		SNAP_need_buffer = false;
		SNAP_Process();
	}
}

void SNAP_DoSnap(bool do_auto, uint8_t mode) {
	if (do_auto && TRX_Inactive_Time < SNAP_AUTO_TIMEOUT) {
		return;
	}

	SNAP_process_from_auto = do_auto;
	SNAP_process_mode = mode;
	SNAP_buffer_avg_index = 0;
	SNAP_need_buffer = true;
}

static void SNAP_Process() {
	arm_sort_init_f32(&SNAP_SortInstance, ARM_SORT_QUICK, ARM_SORT_ASCENDING);
	arm_sort_f32(&SNAP_SortInstance, SNAP_buffer, SNAP_buffer_tmp, FFT_SIZE);

	float32_t noise_level = SNAP_buffer_tmp[(uint32_t)(SNAP_NOISE_FLOOR * (float32_t)FFT_SIZE)];
	float32_t Hz_in_bin = (float32_t)FFT_current_spectrum_width_Hz / (float32_t)FFT_SIZE;

	uint32_t bandwidth_bin_start = 0;
	uint32_t bandwidth_bin_end = 0;
	uint32_t bins_in_bandwidth = (float32_t)CurrentVFO->LPF_RX_Filter_Width / Hz_in_bin;

	if (SNAP_process_mode == 0) { // unidirect
		bandwidth_bin_start = (FFT_SIZE / 2) - (bins_in_bandwidth / 2);
		bandwidth_bin_end = (FFT_SIZE / 2) + (bins_in_bandwidth / 2);
	}
	uint32_t bandwidth_bin_count = bandwidth_bin_end - bandwidth_bin_start;

	if (bandwidth_bin_count == 0) {
		bandwidth_bin_start = (FFT_SIZE / 2) - 0;
		bandwidth_bin_end = (FFT_SIZE / 2) + 1;
		bandwidth_bin_count = 1;
	}

	uint64_t fft_freq_start = (float64_t)CurrentVFO->Freq - (float64_t)FFT_current_spectrum_width_Hz / 2.0;
	float32_t maxAmplValue;
	uint32_t maxAmplIndex;
	float32_t signal_snr = 0;
	uint64_t target_freq = 0;

	// search in BW
	if (SNAP_process_mode == 0) { // unidirect
		arm_max_f32(&SNAP_buffer[bandwidth_bin_start], bandwidth_bin_count, &maxAmplValue, &maxAmplIndex);
		signal_snr = rate2dbP(maxAmplValue / noise_level);
		target_freq = fft_freq_start + ((bandwidth_bin_start + maxAmplIndex) * Hz_in_bin);
	}

	// search in all FFT (nearest)
	if (signal_snr < SNAP_BW_SNR_THRESHOLD && !SNAP_process_from_auto) {

		if (SNAP_process_mode == 0) { // unidirect
			for (int32_t allfft_bin_start = bandwidth_bin_start; allfft_bin_start > 0; allfft_bin_start -= bins_in_bandwidth / 2) {
				uint32_t allfft_bin_end = bandwidth_bin_end + (bandwidth_bin_start - allfft_bin_start);
				uint32_t allfft_bin_count = allfft_bin_end - allfft_bin_start;

				arm_max_f32(&SNAP_buffer[allfft_bin_start], allfft_bin_count, &maxAmplValue, &maxAmplIndex);
				signal_snr = rate2dbP(maxAmplValue / noise_level);
				target_freq = fft_freq_start + ((allfft_bin_start + maxAmplIndex) * Hz_in_bin);

				if (signal_snr >= SNAP_BW_SNR_AUTO_THRESHOLD) {
					break;
				}
			}
		}

		if (SNAP_process_mode == 1) { // left
			uint32_t allfft_bin_end = (FFT_SIZE / 2) - (bins_in_bandwidth / 2);

			for (int32_t allfft_bin_start = allfft_bin_end - 1; allfft_bin_start > 0; allfft_bin_start -= bins_in_bandwidth / 2) {
				uint32_t allfft_bin_count = allfft_bin_end - allfft_bin_start;

				arm_max_f32(&SNAP_buffer[allfft_bin_start], allfft_bin_count, &maxAmplValue, &maxAmplIndex);
				signal_snr = rate2dbP(maxAmplValue / noise_level);
				target_freq = fft_freq_start + ((allfft_bin_start + maxAmplIndex) * Hz_in_bin);

				if (signal_snr >= SNAP_BW_SNR_AUTO_THRESHOLD) {
					break;
				}
			}
		}

		if (SNAP_process_mode == 2) { // right
			uint32_t allfft_bin_start = (FFT_SIZE / 2) + (bins_in_bandwidth / 2);

			for (int32_t allfft_bin_end = allfft_bin_start + 1; allfft_bin_end < FFT_SIZE; allfft_bin_end += bins_in_bandwidth / 2) {
				uint32_t allfft_bin_count = allfft_bin_end - allfft_bin_start;

				arm_max_f32(&SNAP_buffer[allfft_bin_start], allfft_bin_count, &maxAmplValue, &maxAmplIndex);
				signal_snr = rate2dbP(maxAmplValue / noise_level);
				target_freq = fft_freq_start + ((allfft_bin_start + maxAmplIndex) * Hz_in_bin);

				if (signal_snr >= SNAP_BW_SNR_AUTO_THRESHOLD) {
					break;
				}
			}
		}
	}

	// SSB Part
	if (CurrentVFO->Mode == TRX_MODE_LSB) {
		if (SNAP_process_mode == 1) { // left
			target_freq += 500;
		}
		if (SNAP_process_mode == 2) { // right
			target_freq += CurrentVFO->LPF_RX_Filter_Width;
		}

		target_freq = roundl(target_freq / 500) * 500;
	}

	if (CurrentVFO->Mode == TRX_MODE_USB) {
		if (SNAP_process_mode == 1) { // left
			target_freq -= CurrentVFO->LPF_RX_Filter_Width;
		}
		if (SNAP_process_mode == 2) { // right
			target_freq -= 500;
		}

		target_freq = roundl(target_freq / 500) * 500;
	}
	//

	bool result_ok = false;
	if (!SNAP_process_from_auto && signal_snr >= SNAP_BW_SNR_THRESHOLD) {
		result_ok = true;
	}
	if (SNAP_process_from_auto && signal_snr >= SNAP_BW_SNR_AUTO_THRESHOLD) {
		result_ok = true;
	}

	if (result_ok) {
		TRX_setFrequencySlowly(target_freq);
		LCD_UpdateQuery.FreqInfo = true;
	}

	println("NOISE: ", (double)noise_level, " MAX: ", (double)maxAmplValue, " SNR: ", (double)signal_snr);
	println("INDEX: ", maxAmplIndex, " Hz/BIN: ", (double)Hz_in_bin, " TARGET: ", target_freq, " OK: ", (uint8_t)result_ok);
	println("");
}
