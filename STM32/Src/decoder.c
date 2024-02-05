#include "decoder.h"
#include "cw_decoder.h"
#include "rds_decoder.h"
#include "rtty_decoder.h"

// Private variables
static SRAM float32_t DECODER_Buffer[DECODER_BUFF_SIZE] = {0};
static uint32_t DECODER_head = 0; // index of adding new data to the buffer
static uint32_t DECODER_tail = 0; // index of reading data from the buffer

void DECODER_Init(void) {
#ifndef STM32F407xx
	RDSDecoder_Init();
#endif
	CWDecoder_Init();
	RTTYDecoder_Init();
}

void DECODER_PutSamples(float32_t *bufferIn, uint32_t size) {
	if ((DECODER_head + size) <= DECODER_BUFF_SIZE) {
		dma_memcpy32((float32_t *)&DECODER_Buffer[DECODER_head], (float32_t *)bufferIn, size);
		DECODER_head += size;
		if (DECODER_head >= DECODER_BUFF_SIZE) {
			DECODER_head = 0;
		}
	} else {
		uint32_t firstpart = DECODER_BUFF_SIZE - DECODER_head;
		dma_memcpy32((float32_t *)&DECODER_Buffer[DECODER_head], (float32_t *)bufferIn, firstpart);
		DECODER_head = 0;
		dma_memcpy32((float32_t *)&DECODER_Buffer[DECODER_head], (float32_t *)bufferIn, (size - firstpart));
		DECODER_head += (size - firstpart);
		// if (DECODER_tail == DECODER_head)
		// print("o");
	}
}

void DECODER_Process(void) {
	if (DECODER_tail == DECODER_head) // overrun
	{
		// print("o");
		return;
	}
	// get data from the buffer
	float32_t *bufferOut = &DECODER_Buffer[DECODER_tail];

	// CW Decoder
	if (TRX.CW_Decoder && (CurrentVFO->Mode == TRX_MODE_CW || CurrentVFO->Mode == TRX_MODE_LOOPBACK)) {
		DECODER_tail += DECODER_PACKET_SIZE;
		CWDecoder_Process(bufferOut);
	}
#ifndef STM32F407xx
	// RDS Decoder
	if (TRX.RDS_Decoder && CurrentVFO->Mode == TRX_MODE_WFM) { // RDS_DECODER_PACKET_SIZE
		DECODER_tail += RDS_DECODER_PACKET_SIZE;
		RDSDecoder_Process(bufferOut);
	}
#endif

	// RTTY Decoder
	if (CurrentVFO->Mode == TRX_MODE_RTTY) {
		DECODER_tail += DECODER_PACKET_SIZE;
		RTTYDecoder_Process(bufferOut);
	}

	// move tail
	if (DECODER_tail >= DECODER_BUFF_SIZE) {
		DECODER_tail = 0;
	}
}
