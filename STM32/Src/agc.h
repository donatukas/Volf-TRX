#ifndef AGC_H
#define AGC_H

#include "audio_processor.h"
#include <stdbool.h>

#define AGC_RINGBUFFER_TAPS_SIZE 3

typedef struct {
	float32_t need_gain_dB;
	float32_t need_gain_dB_old;
	float32_t AGC_RX_dBFS_old;
	uint32_t last_agc_peak_time;
#ifndef STM32F407xx
	float32_t agcBuffer_kw[AUDIO_BUFFER_HALF_SIZE];
#endif
	float32_t ringbuffer[AGC_RINGBUFFER_TAPS_SIZE * AUDIO_BUFFER_HALF_SIZE];
	uint16_t hold_time;
} AGC_RX_Instance;

typedef struct {
	float32_t need_gain_dB;
	float32_t need_gain_dB_old;
} AGC_TX_Instance;

extern float32_t AGC_SCREEN_maxGain;
extern float32_t AGC_SCREEN_currentGain;

// Public methods
extern void DoRxAGC(float32_t *agcbuffer_i, float32_t *agcbuffer_q, uint_fast16_t blockSize, AUDIO_PROC_RX_NUM rx_id, uint8_t nr_type, TRX_MODE mode,
                    bool stereo);                                                                      // start RX AGC on a data block
extern void DoTxAGC(float32_t *agcbuffer_i, uint_fast16_t blockSize, float32_t target, TRX_MODE mode); // start TX AGC on a data block
extern void ResetAGC(void);

#endif
