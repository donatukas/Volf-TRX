#ifndef RTTY_h
#define RTTY_h

#include "functions.h"
#include "hardware.h"
#include "lcd.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#if (defined(LAY_800x480))
#define RTTY_DECODER_STRLEN 66 // length of decoded string
#else
#define RTTY_DECODER_STRLEN 40 // length of decoded string
#endif

#define RTTY_LPF_STAGES 2
#define RTTY_BPF_STAGES 2
#define RTTY_BPF_WIDTH (TRX.RTTY_Shift / 4)

#define RTTY_SYMBOL_CODE (0b11011)
#define RTTY_LETTER_CODE (0b11111)

typedef enum {
	RTTY_STATE_WAIT_START,
	RTTY_STATE_BIT,
} rtty_state_t;

typedef enum { RTTY_MODE_LETTERS, RTTY_MODE_SYMBOLS } rtty_charSetMode_t;

typedef enum { RTTY_STOP_1, RTTY_STOP_1_5, RTTY_STOP_2 } rtty_stopbits_t;

// Public variables
extern char RTTY_Decoder_Text[RTTY_DECODER_STRLEN + 1];

// Public methods
extern void RTTYDecoder_Init(void);                   // initialize the CW decoder
extern void RTTYDecoder_Process(float32_t *bufferIn); // start CW decoder for the data block

#endif
