////////////////////////////////////////////////////////////////////////////
//                           **** ADPCM-XQ ****                           //
//                  Xtreme Quality ADPCM Encoder/Decoder                  //
//                    Copyright (c) 2015 David Bryant.                    //
//                          All Rights Reserved.                          //
//      Distributed under the BSD Software License (see license.txt)      //
////////////////////////////////////////////////////////////////////////////

#ifndef ADPCMLIB_H_
#define ADPCMLIB_H_

#include <stdint.h>

void *adpcm_create_context(int num_channels, int lookahead, int noise_shaping, int32_t initial_deltas[2]);
int adpcm_encode_block(void *p, uint8_t *outbuf, size_t *outbufsize, const int16_t *inbuf, int inbufcount);
int adpcm_decode_block(int16_t *outbuf, const uint8_t *inbuf, size_t inbufsize, int channels);

#define NOISE_SHAPING_OFF 0     // flat noise (no shaping)
#define NOISE_SHAPING_STATIC 1  // first-order highpass shaping
#define NOISE_SHAPING_DYNAMIC 2 // dynamically tilted noise based on signal

#endif /* ADPCMLIB_H_ */
