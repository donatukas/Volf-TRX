#include "hardware.h"

#if HRDW_HAS_SD

#include "functions.h"
#include "lcd.h"
#include "sd.h"
#include "vocoder.h"

IRAM2 int16_t VOCODER_Buffer[SIZE_ADPCM_BLOCK] = {0};
uint16_t VOCODER_Buffer_Index = 0;
uint32_t VOCODER_ProcessedSamples = 0;
uint32_t VOCODER_SecondsElapsedPrev = 0;
uint32_t VOCODER_SecondsElapsed = 0;
uint32_t VOCODER_SecondsTotal = 0;
void *ADPCM_cnxt = NULL;

void ADPCM_Init(void) {
	int32_t average_deltas[2] = {0, 0};
	ADPCM_cnxt = adpcm_create_context(1, 2, NOISE_SHAPING_DYNAMIC, average_deltas); // num_channels, lookahead, noise_shaping, average_deltas
}

void VOCODER_Process(void) {
	// encode audio
	uint32_t outbuff_size = 0;
	if (!SD_workbuffer_current) {
		adpcm_encode_block(ADPCM_cnxt, (uint8_t *)&SD_workbuffer_A[SD_RecordBufferIndex], &outbuff_size, VOCODER_Buffer, SIZE_ADPCM_BLOCK);
	} else {
		adpcm_encode_block(ADPCM_cnxt, (uint8_t *)&SD_workbuffer_B[SD_RecordBufferIndex], &outbuff_size, VOCODER_Buffer, SIZE_ADPCM_BLOCK);
	}
	SD_RecordBufferIndex += SIZE_ADPCM_COMPRESSED_BLOCK; // outbuff_size;

	VOCODER_ProcessedSamples += SIZE_ADPCM_BLOCK;
	VOCODER_SecondsElapsed = VOCODER_ProcessedSamples / TRX_SAMPLERATE;
	if (VOCODER_SecondsElapsedPrev != VOCODER_SecondsElapsed) {
		LCD_UpdateQuery.SystemMenuRedraw = true;
	}
	VOCODER_SecondsElapsedPrev = VOCODER_SecondsElapsed;

	if (SD_RecordBufferIndex == FF_MAX_SS) {
		SD_RecordBufferIndex = 0;
		SD_workbuffer_current = !SD_workbuffer_current;
		SD_doCommand(SDCOMM_PROCESS_RECORD, false);
	}
}

bool VODECODER_Process(void) {
	static uint16_t VOCODER_PLAYER_SD_BUFFER_INDEX = 0;
	if (SD_Play_Buffer_Ready || VOCODER_PLAYER_SD_BUFFER_INDEX > 0) {
		if (VOCODER_PLAYER_SD_BUFFER_INDEX == 0) {
			// println("Readed from SD: ", SD_Play_Buffer_Size);
			SD_workbuffer_current = !SD_workbuffer_current;
			SD_Play_Buffer_Ready = false;
			SD_doCommand(SDCOMM_PROCESS_PLAY, false);
		}

		if (!SD_workbuffer_current) {
			adpcm_decode_block(VOCODER_Buffer, (uint8_t *)&SD_workbuffer_A[VOCODER_PLAYER_SD_BUFFER_INDEX], SIZE_ADPCM_COMPRESSED_BLOCK, 1);
		} else {
			adpcm_decode_block(VOCODER_Buffer, (uint8_t *)&SD_workbuffer_B[VOCODER_PLAYER_SD_BUFFER_INDEX], SIZE_ADPCM_COMPRESSED_BLOCK, 1);
		}

		VOCODER_PLAYER_SD_BUFFER_INDEX += SIZE_ADPCM_COMPRESSED_BLOCK;

		if (VOCODER_PLAYER_SD_BUFFER_INDEX >= FF_MAX_SS) {
			VOCODER_PLAYER_SD_BUFFER_INDEX = 0;
		}

		VOCODER_ProcessedSamples += SIZE_ADPCM_BLOCK;
		VOCODER_SecondsElapsed = VOCODER_ProcessedSamples / TRX_SAMPLERATE;
		if (VOCODER_SecondsElapsedPrev != VOCODER_SecondsElapsed) {
			LCD_UpdateQuery.SystemMenuRedraw = true;
		}
		VOCODER_SecondsElapsedPrev = VOCODER_SecondsElapsed;

		return true;
	}
	if (!SD_CommandInProcess) {
		SD_doCommand(SDCOMM_PROCESS_PLAY, false);
	}
	return false;
}

#endif
