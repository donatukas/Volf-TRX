#ifndef CODEC_h
#define CODEC_h

#include "audio_processor.h"
#include "functions.h"
#include "hardware.h"
#include <stdio.h>

// Public variables
extern int32_t CODEC_Audio_Buffer_RX[CODEC_AUDIO_BUFFER_SIZE];
extern int32_t CODEC_Audio_Buffer_TX[CODEC_AUDIO_BUFFER_SIZE];
extern bool CODEC_DMA_state;       // what part of the buffer are we working with, true - complete; false - half
extern bool CODEC_Buffer_underrun; // lack of data in the buffer from the audio processor
extern uint32_t CODEC_DMA_samples; // count the number of samples transmitted to the audio codec
extern bool CODEC_Beeping;         // Beeping flag
extern bool CODEC_Beeping_Left;    // Beeping flag on left channel
extern bool CODEC_Beeping_Right;   // Beeping flag on left channel
extern bool CODEC_Muting;          // Muting flag
extern bool CODEC_test_result;

// Public methods
extern void CODEC_Init(void);              // I2C audio codec initialization
extern void CODEC_start_i2s_and_dma(void); // I2S bus start
extern void CODEC_CleanBuffer(void);       // clear the audio codec and USB audio buffer
extern void CODEC_TXRX_mode(void);         // switch to mixed mode RX-TX
extern void CODEC_Mute(void);              // mute audio out
extern void CODEC_UnMute(void);            // disable audio mute
extern void CODEC_Mute_AF_AMP(void);       // mute audio out (AF AMP only)
extern void CODEC_UnMute_AF_AMP(void);     // disable audio mute (AF AMP only)
extern void CODEC_Beep(void);              // beep on key press
extern void CODEC_TestBeep(bool channel);  // beep on self test
#endif
