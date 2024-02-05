#ifndef DECODER_h
#define DECODER_h

#include "functions.h"
#include "hardware.h"
#include "settings.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define DECODER_FREQ 1000                                   // decoder response frequency (1 msec)
#define DECODER_PACKET_SIZE (TRX_SAMPLERATE / DECODER_FREQ) // data in one packet (48 bytes)
#define DECODER_BUFF_SIZE (DECODER_PACKET_SIZE * 8 * 2)     // decoder buffer size

// Public methods
extern void DECODER_Init(void);                                     // initialization of the decoder and its child processes
extern void DECODER_Process(void);                                  // launch a suitable decoder
extern void DECODER_PutSamples(float32_t *bufferIn, uint32_t size); // add new data to the decoder buffer

#endif
