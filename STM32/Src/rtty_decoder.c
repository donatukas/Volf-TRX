#include "rtty_decoder.h"
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

// Ported from https://github.com/df8oe/UHSDR/blob/active-devel/mchf-eclipse/drivers/audio/rtty.c

char RTTY_Decoder_Text[RTTY_DECODER_STRLEN + 1] = {0}; // decoded string

static rtty_state_t RTTY_State = RTTY_STATE_WAIT_START;
static rtty_charSetMode_t RTTY_charSetMode = RTTY_MODE_LETTERS;
static uint16_t RTTY_oneBitSampleCount = 0;
static uint8_t RTTY_byteResult = 0;
static uint16_t RTTY_byteResult_bnum = 0;
static int32_t RTTY_DPLLBitPhase;
static int32_t RTTY_DPLLOldVal;

// lpf
static float32_t RTTY_LPF_Filter_Coeffs[BIQUAD_COEFF_IN_STAGE * RTTY_LPF_STAGES] = {0};
static float32_t RTTY_LPF_Filter_State[2 * RTTY_LPF_STAGES] = {0};
static arm_biquad_cascade_df2T_instance_f32 RTTY_LPF_Filter;

// mark
static float32_t RTTY_Mark_Filter_Coeffs[BIQUAD_COEFF_IN_STAGE * RTTY_BPF_STAGES] = {0};
static float32_t RTTY_Mark_Filter_State[2 * RTTY_BPF_STAGES] = {0};
static arm_biquad_cascade_df2T_instance_f32 RTTY_Mark_Filter;

// space
static float32_t RTTY_Space_Filter_Coeffs[BIQUAD_COEFF_IN_STAGE * RTTY_BPF_STAGES] = {0};
static float32_t RTTY_Space_Filter_State[2 * RTTY_BPF_STAGES] = {0};
static arm_biquad_cascade_df2T_instance_f32 RTTY_Space_Filter;

static const char RTTY_Letters[] = {'\0', 'E', '\n', 'A', ' ', 'S', 'I', 'U', '\r', 'D', 'R', 'J', 'N', 'F', 'C', 'K',
                                    'T',  'Z', 'L',  'W', 'H', 'Y', 'P', 'Q', 'O',  'B', 'G', ' ', 'M', 'X', 'V', ' '};

static const char RTTY_Symbols[32] = {'\0', '3', '\n', '-', ' ', '\a', '8', '7', '\r', '$', '4', '\'', ',', '!', ':', '(',
                                      '5',  '"', ')',  '2', '#', '6',  '0', '1', '9',  '?', '&', ' ',  '.', '/', ';', ' '};

static bool RTTYDecoder_waitForStartBit(float32_t sample);
static bool RTTYDecoder_getBitDPLL(float32_t sample, bool *val_p);
static int RTTYDecoder_demodulator(float32_t sample);
static float32_t RTTYDecoder_decayavg(float32_t average, float32_t input, int weight);

void RTTYDecoder_Init(void) {
	// speed
	if (TRX.RTTY_Speed == 45) {
		RTTY_oneBitSampleCount = (uint16_t)roundf((float32_t)TRX_SAMPLERATE / 45.45f);
	} else {
		RTTY_oneBitSampleCount = (uint16_t)roundf((float32_t)TRX_SAMPLERATE / (float32_t)TRX.RTTY_Speed);
	}

	// RTTY LPF Filter
	iir_filter_t *filter = biquad_create(RTTY_LPF_STAGES);
	biquad_init_lowpass(filter, TRX_SAMPLERATE, TRX.RTTY_Speed * 2);
	fill_biquad_coeffs(filter, RTTY_LPF_Filter_Coeffs, RTTY_LPF_STAGES);
	arm_biquad_cascade_df2T_init_f32(&RTTY_LPF_Filter, RTTY_LPF_STAGES, RTTY_LPF_Filter_Coeffs, RTTY_LPF_Filter_State);

	// RTTY mark filter
	filter = biquad_create(RTTY_BPF_STAGES);
	biquad_init_bandpass(filter, TRX_SAMPLERATE, (TRX.RTTY_Freq - TRX.RTTY_Shift / 2) - RTTY_BPF_WIDTH / 2, (TRX.RTTY_Freq - TRX.RTTY_Shift / 2) + RTTY_BPF_WIDTH / 2);
	fill_biquad_coeffs(filter, RTTY_Mark_Filter_Coeffs, RTTY_BPF_STAGES);
	arm_biquad_cascade_df2T_init_f32(&RTTY_Mark_Filter, RTTY_BPF_STAGES, RTTY_Mark_Filter_Coeffs, RTTY_Mark_Filter_State);

	// RTTY space filter
	filter = biquad_create(RTTY_BPF_STAGES);
	biquad_init_bandpass(filter, TRX_SAMPLERATE, (TRX.RTTY_Freq + TRX.RTTY_Shift / 2) - RTTY_BPF_WIDTH / 2, (TRX.RTTY_Freq + TRX.RTTY_Shift / 2) + RTTY_BPF_WIDTH / 2);
	fill_biquad_coeffs(filter, RTTY_Space_Filter_Coeffs, RTTY_BPF_STAGES);
	arm_biquad_cascade_df2T_init_f32(&RTTY_Space_Filter, RTTY_BPF_STAGES, RTTY_Space_Filter_Coeffs, RTTY_Space_Filter_State);

	// text
	sprintf(RTTY_Decoder_Text, " RTTY: -");
	addSymbols(RTTY_Decoder_Text, RTTY_Decoder_Text, RTTY_DECODER_STRLEN, " ", true);
	LCD_UpdateQuery.TextBar = true;
}

void RTTYDecoder_Process(float32_t *bufferIn) {
	for (uint32_t buf_pos = 0; buf_pos < DECODER_PACKET_SIZE; buf_pos++) {
		switch (RTTY_State) {
		case RTTY_STATE_WAIT_START: // not synchronized, need to wait for start bit
			if (RTTYDecoder_waitForStartBit(bufferIn[buf_pos])) {
				RTTY_State = RTTY_STATE_BIT;
				RTTY_byteResult_bnum = 1;
				RTTY_byteResult = 0;
			}
			break;
		case RTTY_STATE_BIT:
			// reading 7 more bits
			if (RTTY_byteResult_bnum < 8) {
				bool bitResult = false;
				if (RTTYDecoder_getBitDPLL(bufferIn[buf_pos], &bitResult)) {
					switch (RTTY_byteResult_bnum) {
					case 6: // stop bit 1
					case 7: // stop bit 2
						if (bitResult == false) {
							// not in sync
							RTTY_State = RTTY_STATE_WAIT_START;
						}
						if (TRX.RTTY_StopBits != RTTY_STOP_2 && RTTY_byteResult_bnum == 6) {
							// we pretend to be at the 7th bit after receiving the first stop bit if we have less than 2 stop bits
							// this omits check for 1.5 bit condition but we should be more or less safe here, may cause
							// a little more unaligned receive but without that shortcut we simply cannot receive these configurations
							// so it is worth it
							RTTY_byteResult_bnum = 7;
						}
						break;
					default:
						RTTY_byteResult |= (bitResult ? 1 : 0) << (RTTY_byteResult_bnum - 1);
					}
					RTTY_byteResult_bnum++;
				}
			}
			if (RTTY_byteResult_bnum == 8 && RTTY_State == RTTY_STATE_BIT) {
				char charResult;

				switch (RTTY_byteResult) {
				case RTTY_LETTER_CODE:
					RTTY_charSetMode = RTTY_MODE_LETTERS;
					// println(" ^L^");
					break;
				case RTTY_SYMBOL_CODE:
					RTTY_charSetMode = RTTY_MODE_SYMBOLS;
					// println(" ^F^");
					break;
				default:
					switch (RTTY_charSetMode) {
					case RTTY_MODE_SYMBOLS:
						charResult = RTTY_Symbols[RTTY_byteResult];
						break;
					case RTTY_MODE_LETTERS:
					default:
						charResult = RTTY_Letters[RTTY_byteResult];
						break;
					}
					// RESULT !!!!
					// print(charResult);
					char str[2] = {0};
					str[0] = charResult;
					if (strlen(RTTY_Decoder_Text) >= RTTY_DECODER_STRLEN) {
						shiftTextLeft(RTTY_Decoder_Text, 1);
					}
					strcat(RTTY_Decoder_Text, str);
					LCD_UpdateQuery.TextBar = true;
					break;
				}
				RTTY_State = RTTY_STATE_WAIT_START;
			}
		}
	}
}

// this function returns only true when the start bit is successfully received
static bool RTTYDecoder_waitForStartBit(float32_t sample) {
	bool retval = false;
	int bitResult;
	static int16_t wait_for_start_state = 0;
	static int16_t wait_for_half = 0;

	bitResult = RTTYDecoder_demodulator(sample);

	switch (wait_for_start_state) {
	case 0:
		// waiting for a falling edge
		if (bitResult != 0) {
			wait_for_start_state++;
		}
		break;
	case 1:
		if (bitResult != 1) {
			wait_for_start_state++;
		}
		break;
	case 2:
		wait_for_half = RTTY_oneBitSampleCount / 2;
		wait_for_start_state++;
		/* no break */
	case 3:
		wait_for_half--;
		if (wait_for_half == 0) {
			retval = (bitResult == 0);
			wait_for_start_state = 0;
		}
		break;
	}
	return retval;
}

// this function returns true once at the half of a bit with the bit's value
static bool RTTYDecoder_getBitDPLL(float32_t sample, bool *val_p) {
	static bool phaseChanged = false;
	bool retval = false;

	if (RTTY_DPLLBitPhase < RTTY_oneBitSampleCount) {
		*val_p = RTTYDecoder_demodulator(sample);

		if (!phaseChanged && *val_p != RTTY_DPLLOldVal) {
			if (RTTY_DPLLBitPhase < RTTY_oneBitSampleCount / 2) {
				RTTY_DPLLBitPhase += RTTY_oneBitSampleCount / 32; // early
			} else {
				RTTY_DPLLBitPhase -= RTTY_oneBitSampleCount / 32; // late
			}
			phaseChanged = true;
		}
		RTTY_DPLLOldVal = *val_p;
		RTTY_DPLLBitPhase++;
	}

	if (RTTY_DPLLBitPhase >= RTTY_oneBitSampleCount) {
		RTTY_DPLLBitPhase -= RTTY_oneBitSampleCount;
		retval = true;
	}

	return retval;
}

// adapted from https://github.com/ukhas/dl-fldigi/blob/master/src/include/misc.h
static float32_t RTTYDecoder_decayavg(float32_t average, float32_t input, int weight) {
	float32_t retval;
	if (weight <= 1) {
		retval = input;
	} else {
		retval = ((input - average) / (float32_t)weight) + average;
	}
	return retval;
}

// this function returns the bit value of the current sample
static int RTTYDecoder_demodulator(float32_t sample) {
	float32_t space_mag = 0;
	float32_t mark_mag = 0;
	arm_biquad_cascade_df2T_f32_single(&RTTY_Space_Filter, &sample, &space_mag, 1);
	arm_biquad_cascade_df2T_f32_single(&RTTY_Mark_Filter, &sample, &mark_mag, 1);

	float32_t v1 = 0.0;
	// calculating the RMS of the two lines (squaring them)
	space_mag *= space_mag;
	mark_mag *= mark_mag;

	// RTTY decoding with ATC = automatic threshold correction
	float32_t helper = space_mag;
	space_mag = mark_mag;
	mark_mag = helper;
	static float32_t mark_env = 0.0;
	static float32_t space_env = 0.0;
	static float32_t mark_noise = 0.0;
	static float32_t space_noise = 0.0;
	// experiment to implement an ATC (Automatic threshold correction), DD4WH, 2017_08_24
	// everything taken from FlDigi, licensed by GNU GPLv2 or later
	// https://github.com/ukhas/dl-fldigi/blob/master/src/cw_rtty/rtty.cxx
	// calculate envelope of the mark and space signals
	// uses fast attack and slow decay
	mark_env = RTTYDecoder_decayavg(mark_env, mark_mag, (mark_mag > mark_env) ? RTTY_oneBitSampleCount / 4 : RTTY_oneBitSampleCount * 16);
	space_env = RTTYDecoder_decayavg(space_env, space_mag, (space_mag > space_env) ? RTTY_oneBitSampleCount / 4 : RTTY_oneBitSampleCount * 16);
	// calculate the noise on the mark and space signals
	mark_noise = RTTYDecoder_decayavg(mark_noise, mark_mag, (mark_mag < mark_noise) ? RTTY_oneBitSampleCount / 4 : RTTY_oneBitSampleCount * 48);
	space_noise = RTTYDecoder_decayavg(space_noise, space_mag, (space_mag < space_noise) ? RTTY_oneBitSampleCount / 4 : RTTY_oneBitSampleCount * 48);
	// the noise floor is the lower signal of space and mark noise
	float32_t noise_floor = (space_noise < mark_noise) ? space_noise : mark_noise;

	// Linear ATC, section 3 of www.w7ay.net/site/Technical/ATC
	// v1 = space_mag - mark_mag - 0.5 * (space_env - mark_env);

	// Compensating for the noise floor by using clipping
	float32_t mclipped = 0.0, sclipped = 0.0;
	mclipped = mark_mag > mark_env ? mark_env : mark_mag;
	sclipped = space_mag > space_env ? space_env : space_mag;
	if (mclipped < noise_floor) {
		mclipped = noise_floor;
	}
	if (sclipped < noise_floor) {
		sclipped = noise_floor;
	}

	// Optimal ATC (Section 6 of of www.w7ay.net/site/Technical/ATC)
	v1 = (mclipped - noise_floor) * (mark_env - noise_floor) - (sclipped - noise_floor) * (space_env - noise_floor) -
	     0.25f * ((mark_env - noise_floor) * (mark_env - noise_floor) - (space_env - noise_floor) * (space_env - noise_floor));
	arm_biquad_cascade_df2T_f32_single(&RTTY_LPF_Filter, &v1, &v1, 1);

	// RTTY without ATC, which works very well too!
	// inverting line 1
	/*mark_mag *= -1;

	// summing the two lines
	v1 = mark_mag + space_mag;

	// lowpass filtering the summed line
	arm_biquad_cascade_df2T_f32_rolled(&RTTY_LPF_Filter, &v1, &v1, 1);*/

	if (TRX.RTTY_InvertBits) {
		return (v1 > 0) ? 1 : 0;
	} else {
		return (v1 > 0) ? 0 : 1;
	}
}
