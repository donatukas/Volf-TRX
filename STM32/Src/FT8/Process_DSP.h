#ifndef PROCESS_DSP_H_
#define PROCESS_DSP_H_

#include "arm_math.h"
#include "lcd.h"
#include <stdbool.h>

#define FFT_SIZE_FT8 2048   // 2048 arm_rfft_init_2048_q15
#define num_que_blocks 40   // 8
#define FT8_block_size 24   // 128
#define input_gulp_size 960 // 1024
#define WF_index_size 900   // 900

#if (defined(LAY_800x480))
#define ft8_buffer 700 // 700? arbitrary for 3 kc
#elif (defined(LAY_320x240))
#define ft8_buffer 300 // 400 arbitrary for 3 kc
#else
#define ft8_buffer 400 // 400 arbitrary for 3 kc
#endif

#define ft8_min_bin 48
#define FFT_Resolution 6.25
#define ft8_min_freq (FFT_Resolution * ft8_min_bin)

#define ft8_msg_samples 86 // 92

void init_DSP(void);
float ft_blackman_i(int i, int N);
void process_FT8_FFT(void);
void update_offset_waterfall(int offset);
void Service_FT8(void);
void Set_Data_Colection(bool enable);

extern int CQ_Flag;

#if (defined(LAY_800x480))
extern uint8_t *export_fft_power;
extern q15_t *dsp_buffer;
extern q15_t *dsp_output;
extern q15_t *input_gulp;
#else
extern uint8_t export_fft_power[ft8_msg_samples * ft8_buffer * 4];
extern q15_t dsp_buffer[] __attribute__((aligned(4)));
extern q15_t dsp_output[] __attribute__((aligned(4)));
extern q15_t input_gulp[input_gulp_size] __attribute__((aligned(4)));
#endif

#endif /* PROCESS_DSP_H_ */
