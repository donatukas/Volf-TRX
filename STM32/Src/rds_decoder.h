#ifndef RDS_DECODER_h
#define RDS_DECODER_h

#include "functions.h"
#include "hardware.h"
#include "lcd.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#ifndef STM32F407xx

#if (defined(LAY_800x480))
#define RDS_DECODER_STRLEN 57 // length of decoded string
#else
#define RDS_DECODER_STRLEN 30 // length of decoded string
#endif

#define RDS_DECODER_PACKET_SIZE 64
#define RDS_PILOT_TONE_MAX_ERROR 500
#define RDS_FILTER_STAGES 5
#define RDS_FREQ (3.0f * SWFM_PILOT_TONE_FREQ) // 57kHz
#define RDS_LOW_FREQ 1187.5f                   // RDS signal baseband breq
#define RDS_FILTER_WIDTH 2400
// #define RDS_DECIMATOR 16 // and x8 for filtering
#define RDS_STR_MAXLEN 34

// Public variables
extern char RDS_Decoder_Text[RDS_DECODER_STRLEN + 1];
extern bool RDS_Stereo;

// Public methods
extern void RDSDecoder_Init(void);                   // initialize the CW decoder
extern void RDSDecoder_Process(float32_t *bufferIn); // start CW decoder for the data block

#endif
#endif
