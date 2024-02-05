#include "rds_decoder.h"
#include "arm_const_structs.h"
#include "arm_math.h"
#include "audio_filters.h"
#include "decoder.h"
#include "fpga.h"
#include "functions.h"
#include "hardware.h"
#include "lcd.h"
#include "settings.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char RDS_Decoder_Text[RDS_DECODER_STRLEN + 1] = {0}; // decoded string
bool RDS_Stereo = false;

static char RDS_Decoder_0A[RDS_STR_MAXLEN];
static char RDS_Decoder_2A[RDS_STR_MAXLEN];
static char RDS_Decoder_2B[RDS_STR_MAXLEN];

// pilot
SRAM_ON_F407 static float32_t RDS_Pilot_Filter_Coeffs[BIQUAD_COEFF_IN_STAGE * RDS_FILTER_STAGES] = {0};
SRAM_ON_F407 static float32_t RDS_Pilot_Filter_State[2 * RDS_FILTER_STAGES] = {0};
SRAM_ON_F407 static arm_biquad_cascade_df2T_instance_f32 RDS_Pilot_Filter;
// RDS center
SRAM_ON_F407 static float32_t RDS_57kPilot_Filter_Coeffs[BIQUAD_COEFF_IN_STAGE * RDS_FILTER_STAGES] = {0};
SRAM_ON_F407 static float32_t RDS_57kPilot_Filter_State[2 * RDS_FILTER_STAGES] = {0};
SRAM_ON_F407 static arm_biquad_cascade_df2T_instance_f32 RDS_57kPilot_Filter;
// signal
SRAM_ON_F407 static float32_t RDS_Signal_Filter_Coeffs[BIQUAD_COEFF_IN_STAGE * RDS_FILTER_STAGES] = {0};
SRAM_ON_F407 static float32_t RDS_Signal_Filter_State[2 * RDS_FILTER_STAGES] = {0};
SRAM_ON_F407 static arm_biquad_cascade_df2T_instance_f32 RDS_Signal_Filter;
// lpf
SRAM_ON_F407 static float32_t RDS_LPF_Filter_Coeffs[BIQUAD_COEFF_IN_STAGE * RDS_FILTER_STAGES] = {0};
SRAM_ON_F407 static float32_t RDS_LPF_Filter_State[2 * RDS_FILTER_STAGES] = {0};
SRAM_ON_F407 static arm_biquad_cascade_df2T_instance_f32 RDS_LPF_Filter;
// decimator
static const float32_t DECIMATE_FIR_Coeffs[4] = {-0.05698952454792, 0.5574889164132, 0.5574889164132, -0.05698952454792};
SRAM_ON_F407 static arm_fir_decimate_instance_f32 DECIMATE_FIR = {.M = 16, .numTaps = 4, .pCoeffs = DECIMATE_FIR_Coeffs, .pState = (float32_t[FPGA_RX_IQ_BUFFER_HALF_SIZE + 4 - 1]){0}};

SRAM_ON_F407 static float32_t RDS_pilot_buff[RDS_DECODER_PACKET_SIZE] = {0};
SRAM_ON_F407 static float32_t RDS_buff[RDS_DECODER_PACKET_SIZE] = {0};

static uint32_t RDS_decoder_samplerate = 0;
static uint32_t RDS_decoder_mainfreq = 0;

static uint16_t RDS_BuildSyndrome(uint32_t raw);
static uint32_t RDS_ApplyFEC(uint32_t *block, uint32_t _syndrome);
static void RDS_AnalyseFrames(uint32_t groupA, uint32_t groupB, uint32_t groupC, uint32_t groupD);

void RDSDecoder_Init(void) {
	RDS_decoder_samplerate = TRX_GetRXSampleRate;
	// no rds in signal
	if (RDS_decoder_samplerate < 192000) {
		return;
	}

	// decimator
	DECIMATE_FIR.M = 16;
	if (RDS_decoder_samplerate == 384000) {
		DECIMATE_FIR.M = 32;
	}

	// RDS signal filter
	iir_filter_t *filter = biquad_create(RDS_FILTER_STAGES);
	biquad_init_bandpass(filter, RDS_decoder_samplerate, RDS_FREQ - RDS_FILTER_WIDTH, RDS_FREQ + RDS_FILTER_WIDTH);
	fill_biquad_coeffs(filter, RDS_Signal_Filter_Coeffs, RDS_FILTER_STAGES);
	arm_biquad_cascade_df2T_init_f32(&RDS_Signal_Filter, RDS_FILTER_STAGES, RDS_Signal_Filter_Coeffs, RDS_Signal_Filter_State);

	// RDS LPF Filter
	filter = biquad_create(RDS_FILTER_STAGES);
	biquad_init_lowpass(filter, RDS_decoder_samplerate, RDS_FILTER_WIDTH);
	fill_biquad_coeffs(filter, RDS_LPF_Filter_Coeffs, RDS_FILTER_STAGES);
	arm_biquad_cascade_df2T_init_f32(&RDS_LPF_Filter, RDS_FILTER_STAGES, RDS_LPF_Filter_Coeffs, RDS_LPF_Filter_State);

	// RDS pilot tone
	filter = biquad_create(RDS_FILTER_STAGES);
	biquad_init_bandpass(filter, RDS_decoder_samplerate, SWFM_PILOT_TONE_FREQ - RDS_PILOT_TONE_MAX_ERROR, SWFM_PILOT_TONE_FREQ + RDS_PILOT_TONE_MAX_ERROR);
	fill_biquad_coeffs(filter, RDS_Pilot_Filter_Coeffs, RDS_FILTER_STAGES);
	arm_biquad_cascade_df2T_init_f32(&RDS_Pilot_Filter, RDS_FILTER_STAGES, RDS_Pilot_Filter_Coeffs, RDS_Pilot_Filter_State);

	// RDS center nco
	filter = biquad_create(RDS_FILTER_STAGES);
	biquad_init_bandpass(filter, RDS_decoder_samplerate, RDS_FREQ - RDS_PILOT_TONE_MAX_ERROR, RDS_FREQ + RDS_PILOT_TONE_MAX_ERROR);
	fill_biquad_coeffs(filter, RDS_57kPilot_Filter_Coeffs, RDS_FILTER_STAGES);
	arm_biquad_cascade_df2T_init_f32(&RDS_57kPilot_Filter, RDS_FILTER_STAGES, RDS_57kPilot_Filter_Coeffs, RDS_57kPilot_Filter_State);

	// Main freq
	RDS_decoder_mainfreq = CurrentVFO->Freq;

	// Result
	dma_memset(RDS_Decoder_0A, 0x00, sizeof(RDS_Decoder_0A));
	dma_memset(RDS_Decoder_2A, 0x00, sizeof(RDS_Decoder_2A));
	dma_memset(RDS_Decoder_2B, 0x00, sizeof(RDS_Decoder_2B));
	sprintf(RDS_Decoder_Text, " RDS: -");
	RDS_Stereo = false;
	addSymbols(RDS_Decoder_Text, RDS_Decoder_Text, RDS_DECODER_STRLEN, " ", true);
	LCD_UpdateQuery.TextBar = true;
}

void RDSDecoder_Process(float32_t *bufferIn) {
	// reinit?
	if (RDS_decoder_samplerate != TRX_GetRXSampleRate || RDS_decoder_mainfreq != CurrentVFO->Freq) {
		RDSDecoder_Init();
	}

	// no rds in signal
	if (RDS_decoder_samplerate < 192000) {
		return;
	}

	// get pilot tone
	arm_biquad_cascade_df2T_f32_single(&RDS_Pilot_Filter, bufferIn, RDS_pilot_buff, RDS_DECODER_PACKET_SIZE);

	// multiply pilot tone
	for (uint_fast16_t i = 0; i < RDS_DECODER_PACKET_SIZE; i++) {
		RDS_pilot_buff[i] = RDS_pilot_buff[i] * RDS_pilot_buff[i] * RDS_pilot_buff[i];
	}

	// filter rds nco
	arm_biquad_cascade_df2T_f32_single(&RDS_57kPilot_Filter, RDS_pilot_buff, RDS_pilot_buff, RDS_DECODER_PACKET_SIZE);

	// filter RDS signal
	arm_biquad_cascade_df2T_f32_single(&RDS_Signal_Filter, bufferIn, RDS_buff, RDS_DECODER_PACKET_SIZE);

	// move signal to low freq
	for (uint_fast16_t i = 0; i < RDS_DECODER_PACKET_SIZE; i++) {
		RDS_buff[i] *= RDS_pilot_buff[i];
	}

	// LPF filter
	arm_biquad_cascade_df2T_f32_single(&RDS_LPF_Filter, RDS_buff, RDS_buff, RDS_DECODER_PACKET_SIZE);

	// decimate
	arm_fir_decimate_f32(&DECIMATE_FIR, RDS_buff, RDS_buff, RDS_DECODER_PACKET_SIZE);

	// get bits data
	static uint32_t raw_block1 = 0;
	static uint32_t raw_block2 = 0;
	static uint32_t raw_block3 = 0;
	static uint32_t raw_block4 = 0;
	static bool signal_state_prev = false;
	static bool signal_state_retry = false;
	static uint8_t bit_sample_counter = 0;
	for (uint32_t i = 0; i < (RDS_DECODER_PACKET_SIZE / DECIMATE_FIR.M); i++) {
		// get data
		bool signal_state = (RDS_buff[i] > 0.0f) ? true : false;
		static bool filtered_state = false;
		uint8_t process_bits = 0;
		if (signal_state_prev == signal_state) {
			bit_sample_counter++;
		} else {
			if (!signal_state_retry) {
				signal_state_retry = true;
				bit_sample_counter++;
				continue;
			}
			signal_state_retry = false;

			// print(bit_sample_counter, " ");

			if (bit_sample_counter >= 8) {
				process_bits = 2;
			} else if (bit_sample_counter >= 4) {
				process_bits = 1;
			}

			filtered_state = signal_state_prev;
			bit_sample_counter = 2;
			signal_state_prev = signal_state;
		}

		// get bits
		static bool bit1_ready = false;
		static bool bit1_state = false;
		static bool bit2_ready = false;
		static bool bit2_state = false;
		bool bit_out_state = false;
		bool bit_out_ready = false;
		static bool filtered_state_prev = false;
		if (process_bits > 0) {
			// print(process_bits);
			for (uint8_t b = 0; b < process_bits; b++) {
				if (!bit1_ready) {
					bit1_state = filtered_state_prev;
					bit1_ready = true;
				} else if (!bit2_ready) {
					bit2_state = filtered_state_prev;
					bit2_ready = true;

					// shift error
					if (bit1_state == bit2_state) {
						bit1_state = filtered_state_prev;
						bit1_ready = true;
						bit2_ready = false;
					}
				}

				if (bit1_ready && bit2_ready) {
					bit_out_state = bit1_state;
					bit1_ready = false;
					bit2_ready = false;
					bit_out_ready = true;
				}
				filtered_state_prev = filtered_state;
			}
		}
		if (!bit_out_ready) {
			continue;
		}

		// shift data
		raw_block4 <<= 1;
		raw_block4 |= (raw_block3 >> 25) & 0x1;
		raw_block3 <<= 1;
		raw_block3 |= (raw_block2 >> 25) & 0x1;
		raw_block2 <<= 1;
		raw_block2 |= (raw_block1 >> 25) & 0x1;
		raw_block1 <<= 1;

		// do diff
		static bool prev_bit = false;
		if (bit_out_state != prev_bit) {
			raw_block1 |= 1;
		}
		prev_bit = bit_out_state;

#define MaxCorrectableBits 2 // 5
#define CheckwordBitsCount 10

		// wait block A
		bool gotA = false;
		uint32_t block4 = raw_block4;
		uint16_t _syndrome = RDS_BuildSyndrome(block4);
		_syndrome ^= 0x3d8;
		gotA = _syndrome == 0 ? true : false;
		if (!gotA && RDS_ApplyFEC(&block4, _syndrome) <= MaxCorrectableBits) {
			gotA = true;
		}

		if (gotA) {
			block4 = (uint16_t)((block4 >> CheckwordBitsCount) & 0xffff);
			// print("A");

			// wait block B
			bool gotB = false;
			uint32_t block3 = raw_block3;
			_syndrome = RDS_BuildSyndrome(block3);
			_syndrome ^= 0x3d4;
			gotB = _syndrome == 0 ? true : false;
			if (!gotB && RDS_ApplyFEC(&block3, _syndrome) <= MaxCorrectableBits) {
				gotB = true;
			}

			if (gotB) {
				block3 = (uint16_t)((block3 >> CheckwordBitsCount) & 0xffff);
				// print("B");

				// wait block C
				bool gotC = false;
				uint32_t block2 = raw_block2;
				_syndrome = RDS_BuildSyndrome(block2);
				_syndrome ^= (uint16_t)((block3 & 0x800) == 0 ? 0x25c : 0x3cc);
				gotC = _syndrome == 0 ? true : false;
				if (!gotC && RDS_ApplyFEC(&block2, _syndrome) <= MaxCorrectableBits) {
					gotC = true;
				}

				if (gotC) {
					block2 = (uint16_t)((block2 >> CheckwordBitsCount) & 0xffff);
					// print("C");

					// wait block D
					bool gotD = false;
					uint32_t block1 = raw_block1;
					_syndrome = RDS_BuildSyndrome(block1);
					_syndrome ^= 0x258;
					gotD = _syndrome == 0 ? true : false;
					if (!gotD && RDS_ApplyFEC(&block1, _syndrome) <= MaxCorrectableBits) {
						gotD = true;
					}

					if (gotD) {
						block1 = (uint16_t)((block1 >> CheckwordBitsCount) & 0xffff);
						// println("D");
						RDS_AnalyseFrames(block4, block3, block2, block1);
						// write string
						memset(RDS_Decoder_Text, 0x00, sizeof(RDS_Decoder_Text));
						strcat(RDS_Decoder_Text, " RDS: ");
						if ((strlen(RDS_Decoder_Text) + strlen(RDS_Decoder_0A) + 1) < RDS_DECODER_STRLEN) {
							strcat(RDS_Decoder_Text, RDS_Decoder_0A);
							strcat(RDS_Decoder_Text, " ");
						}
						if ((strlen(RDS_Decoder_Text) + strlen(RDS_Decoder_2A) + 1) < RDS_DECODER_STRLEN) {
							strcat(RDS_Decoder_Text, RDS_Decoder_2A);
							strcat(RDS_Decoder_Text, " ");
						}
						if ((strlen(RDS_Decoder_Text) + strlen(RDS_Decoder_2B) + 1) < RDS_DECODER_STRLEN) {
							strcat(RDS_Decoder_Text, RDS_Decoder_2B);
							strcat(RDS_Decoder_Text, " ");
						}
						addSymbols(RDS_Decoder_Text, RDS_Decoder_Text, RDS_DECODER_STRLEN, " ", true);
						LCD_UpdateQuery.TextBar = true;
						println("RDS 0A: ", RDS_Decoder_0A, " 2A: ", RDS_Decoder_2A, " 2B: ", RDS_Decoder_2B);
					}
				}
			}
		}
	}
}

static void RDS_AnalyseFrames(uint32_t groupA, uint32_t groupB, uint32_t groupC, uint32_t groupD) {
	// 0A or 0B group radio text
	if ((groupB & 0xf800) == 0b0000000000000000) {
		int index = (groupB & 0x3) * 2; // text segment and DI segment address

		if (index == (3 * 2)) { // DI mono/stereo information
			RDS_Stereo = groupB & 0x4;
		}

		if (index > (RDS_STR_MAXLEN - 5)) {
			return;
		}

		char chr_c = cleanASCIIgarbage((char)(groupD >> 8));
		char chr_d = cleanASCIIgarbage((char)(groupD & 0xff));

		if (chr_c == 0 || chr_d == 0) {
			return;
		}

		RDS_Decoder_0A[index] = chr_c;
		RDS_Decoder_0A[index + 1] = chr_d;

		for (uint8_t i = 0; i < index; i++) {
			if (RDS_Decoder_0A[i] == 0) {
				RDS_Decoder_0A[i] = ' ';
			}
		}

		// println("0A ", index, ": ", str);
	}

	// 2A group radio text
	if ((groupB & 0xf800) == 0b0010000000000000) {
		int index = (groupB & 0xf) * 4; // text segment
		bool abFlag = ((groupB >> 4) & 0x1) == 1;

		if (index > (RDS_STR_MAXLEN - 5)) {
			return;
		}

		char chr_a = cleanASCIIgarbage((char)(groupC >> 8));
		char chr_b = cleanASCIIgarbage((char)(groupC & 0xff));
		char chr_c = cleanASCIIgarbage((char)(groupD >> 8));
		char chr_d = cleanASCIIgarbage((char)(groupD & 0xff));

		if (chr_a == 0 || chr_b == 0 || chr_c == 0 || chr_d == 0) {
			return;
		}

		if (abFlag) {
			RDS_Decoder_2A[index] = chr_a;
			RDS_Decoder_2A[index + 1] = chr_b;
			RDS_Decoder_2A[index + 2] = chr_c;
			RDS_Decoder_2A[index + 3] = chr_d;

			for (uint8_t i = 0; i < index; i++) {
				if (RDS_Decoder_2A[i] == 0) {
					RDS_Decoder_2A[i] = ' ';
				}
			}
		} else {
			RDS_Decoder_2B[index] = chr_a;
			RDS_Decoder_2B[index + 1] = chr_b;
			RDS_Decoder_2B[index + 2] = chr_c;
			RDS_Decoder_2B[index + 3] = chr_d;

			for (uint8_t i = 0; i < index; i++) {
				if (RDS_Decoder_2B[i] == 0) {
					RDS_Decoder_2B[i] = ' ';
				}
			}
		}

		// println("2A ", abFlag?" A ":" B ", index, ": ", str);
	}
}

static uint32_t RDS_ApplyFEC(uint32_t *block, uint32_t _syndrome) {
	const uint16_t poly = 0x5b9;
	const int errorMask = (1 << 5);

	uint32_t correction = (uint32_t)(1 << 25);
	uint8_t correctedBitsCount = 0;

	for (uint8_t i = 0; i < 16; i++) {
		uint32_t st = ((_syndrome & 0x200) == 0x200);
		uint32_t bitError = (_syndrome & errorMask) == 0;
		*block ^= (st && bitError) ? correction : 0;
		_syndrome <<= 1;
		_syndrome ^= ((st && !bitError) ? poly : (uint16_t)0);
		correctedBitsCount += (st && bitError) ? 1 : 0;
		correction >>= 1;
	}
	_syndrome &= 0x3ff;

	if (correctedBitsCount > 0 && _syndrome != 0) { // corrected wrong
		correctedBitsCount += 50;
	}

	return correctedBitsCount;
}

static uint16_t RDS_BuildSyndrome(uint32_t raw) {
	uint16_t Parity[] = {0x2DC, 0x16E, 0x0B7, 0x287, 0x39F, 0x313, 0x355, 0x376, 0x1BB, 0x201, 0x3DC, 0x1EE, 0x0F7, 0x2A7, 0x38F, 0x31B};

	uint32_t block = raw & 0x3ffffff;
	uint16_t syndrome = (uint16_t)(block >> 16);
	for (uint8_t i = 0; i < 16; i++) {
		syndrome ^= ((block & 0x8000) == 0x8000) ? Parity[i] : (uint16_t)0;
		block <<= 1;
	}

	return syndrome;
}
