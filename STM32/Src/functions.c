#include "functions.h"
#include "arm_math.h"
#include "fpga.h"
#include "hardware.h"
#include "lcd.h"
#include "trx_manager.h"
#include "usbd_cat_if.h"
#include "usbd_debug_if.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

volatile bool SPI_DMA_TXRX_ready_callback = false;

void readFromCircleBuffer32(uint32_t *source, uint32_t *dest, uint32_t index, uint32_t length, uint32_t words_to_read) {
	if (index >= words_to_read) {
		dma_memcpy32(&dest[0], &source[index - words_to_read], words_to_read);
	} else {
		uint_fast16_t prev_part = words_to_read - index;
		dma_memcpy32(&dest[0], &source[length - prev_part], prev_part);
		dma_memcpy32(&dest[prev_part], &source[0], (words_to_read - prev_part));
	}
}

void readHalfFromCircleUSBBuffer16Bit(uint8_t *source, int32_t *dest, uint32_t index, uint32_t length) {
	uint_fast16_t halflen = length / 2;
	uint_fast16_t readed_index = 0;
	if (index >= halflen) {
		for (uint_fast16_t i = (index - halflen); i < index; i++) {
			dest[readed_index] = (source[i * 2 + 0] << 16) | (source[i * 2 + 1] << 24);
			readed_index++;
		}
	} else {
		uint_fast16_t prev_part = halflen - index;
		for (uint_fast16_t i = (length - prev_part); i < length; i++) {
			dest[readed_index] = (source[i * 2 + 0] << 16) | (source[i * 2 + 1] << 24);
			readed_index++;
		}
		for (uint_fast16_t i = 0; i < (halflen - prev_part); i++) {
			dest[readed_index] = (source[i * 2 + 0] << 16) | (source[i * 2 + 1] << 24);
			readed_index++;
		}
	}
}

void readHalfFromCircleUSBBuffer24Bit(uint8_t *source, int32_t *dest, uint32_t index, uint32_t length) {
	uint_fast16_t halflen = length / 2;
	uint_fast16_t readed_index = 0;
	if (index >= halflen) {
		for (uint_fast16_t i = (index - halflen); i < index; i++) {
			dest[readed_index] = (source[i * 3 + 0] << 8) | (source[i * 3 + 1] << 16) | (source[i * 3 + 2] << 24);
			readed_index++;
		}
	} else {
		uint_fast16_t prev_part = halflen - index;
		for (uint_fast16_t i = (length - prev_part); i < length; i++) {
			dest[readed_index] = (source[i * 3 + 0] << 8) | (source[i * 3 + 1] << 16) | (source[i * 3 + 2] << 24);
			readed_index++;
		}
		for (uint_fast16_t i = 0; i < (halflen - prev_part); i++) {
			dest[readed_index] = (source[i * 3 + 0] << 8) | (source[i * 3 + 1] << 16) | (source[i * 3 + 2] << 24);
			readed_index++;
		}
	}
}

static uint16_t dbg_lcd_y = 0;
static uint16_t dbg_lcd_x = 0;
void print_chr_LCDOnly(char chr) {
	if (LCD_DEBUG_ENABLED) {
		if (chr == '\r') {
			return;
		}

		if (chr == '\n') {
			dbg_lcd_y += 9;
			dbg_lcd_x = 0;
			if (dbg_lcd_y >= LCD_HEIGHT) {
				dbg_lcd_y = 0;
				LCDDriver_Fill(BG_COLOR);
			}
			return;
		}

		LCDDriver_drawChar(dbg_lcd_x, dbg_lcd_y, chr, COLOR_RED, BG_COLOR, COLOR_RED, BG_COLOR, 1);
		dbg_lcd_x += 6;
	}
}

void print_flush(void) {
#if HRDW_HAS_USB_DEBUG
	uint_fast16_t debug_tryes = 0;
	while (DEBUG_Transmit_FIFO_Events() == USBD_BUSY && debug_tryes < 512) {
		debug_tryes++;
	}
#endif
#if HRDW_DEBUG_ON_CAT_PORT
	uint_fast16_t cat_tryes = 0;
	while (CAT_Transmit_FIFO_Events() == USBD_BUSY && cat_tryes < 512) {
		cat_tryes++;
	}
#endif
}

void print_hex(uint8_t data, bool _inline) {
	char tmp[50] = ""; //-V808
	if (_inline) {
		sprintf(tmp, "%02X", data);
	} else {
		sprintf(tmp, "%02X\n", data);
	}
	print(tmp);
}

void print_bin8(uint8_t data, bool _inline) {
	char tmp[50] = ""; //-V808
	if (_inline) {
		sprintf(tmp, "%c%c%c%c%c%c%c%c", BYTE_TO_BINARY(data));
	} else {
		sprintf(tmp, "%c%c%c%c%c%c%c%c\n", BYTE_TO_BINARY(data));
	}
	print(tmp);
}

void print_bin16(uint16_t data, bool _inline) {
	char tmp[50] = ""; //-V808
	if (_inline) {
		sprintf(tmp, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c", BIT16_TO_BINARY(data));
	} else {
		sprintf(tmp, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n", BIT16_TO_BINARY(data));
	}
	print(tmp);
}

void print_bin26(uint32_t data, bool _inline) {
	char tmp[50] = ""; //-V808
	if (_inline) {
		sprintf(tmp, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c", BIT26_TO_BINARY(data));
	} else {
		sprintf(tmp, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n", BIT26_TO_BINARY(data));
	}
	print(tmp);
}

uint32_t getRXPhraseFromFrequency(int64_t freq, uint8_t rx_num, bool invert_iq_by_mixer) // calculate the frequency from the phrase for FPGA (RX1 / RX2)
{
	if (freq < 0) {
		return 0;
	}
	bool inverted = false;
	int64_t _freq = freq;
	if (_freq > ADC_CLOCK / 2) // Go Nyquist
	{
		while (_freq > (ADC_CLOCK / 2)) {
			_freq -= (ADC_CLOCK / 2);
			inverted = !inverted;
		}
		if (inverted) {
			_freq = (ADC_CLOCK / 2) - _freq;
		}
	}
	if (rx_num == 1) {
		TRX_RX1_IQ_swap = invert_iq_by_mixer ? !inverted : inverted;
	}
	if (rx_num == 2) {
		TRX_RX2_IQ_swap = invert_iq_by_mixer ? !inverted : inverted;
	}
	float64_t res = round(((float64_t)_freq / (float64_t)ADC_CLOCK) * (float64_t)4294967296); // freq in Hz/oscil in Hz*2^bits (32 now);
	return (uint32_t)res;
}

uint32_t getTXPhraseFromFrequency(float64_t freq) // calculate the frequency from the phrase for FPGA (TX)
{
	if (freq < 0) {
		return 0;
	}

	bool inverted = false;
	float64_t _freq = freq;

	uint8_t TRX_TX_Harmonic_new = 0;
	if (_freq > MAX_TX_FREQ_HZ) { // harmonics mode
		while (_freq > MAX_TX_FREQ_HZ) {
			_freq /= 3; // third-harmonics
			TRX_TX_Harmonic_new += 3;
		}
	}
	if (TRX_TX_Harmonic_new == 0) {
		TRX_TX_Harmonic = 1;
	} else {
		TRX_TX_Harmonic = TRX_TX_Harmonic_new;
	}

	FPGA_choise_DAC_PLL(freq); // calculate better PLL for DAC

	TRX_DAC_X4 = true;
	uint8_t nyquist = _freq / (DAC_CLOCK / 2);
	if (nyquist == 0) // 0-0.5 dac freq
	{
		TRX_DAC_HP1 = false; // HP1 low-pass
		TRX_DAC_HP2 = false; // HP2 low-pass
	}
	if (nyquist == 1) // 0.5-1.0 dac freq
	{
		TRX_DAC_HP1 = true;  // HP1 high-pass
		TRX_DAC_HP2 = false; // HP2 low-pass
	}
	if (nyquist == 2) // 1.0-1.5 dac freq
	{
		TRX_DAC_HP1 = true; // HP1 high-pass
		TRX_DAC_HP2 = true; // HP2 high-pass
	}
	if (nyquist == 3) // 1.5-2.0 dac freq
	{
		TRX_DAC_HP1 = false; // HP1 low-pass
		TRX_DAC_HP2 = true;  // HP2 high-pass
	}

	if (_freq > (DAC_CLOCK / 2)) // Go Nyquist
	{
		while (_freq > (DAC_CLOCK / 2)) {
			_freq -= (DAC_CLOCK / 2);
			inverted = !inverted;
		}
		if (inverted) {
			_freq = (DAC_CLOCK / 2) - _freq;
		}
	}
	TRX_TX_IQ_swap = inverted;

	float64_t res = round((float64_t)_freq / (float64_t)DAC_CLOCK * (float64_t)4294967296); // freq in Hz/oscil in Hz*2^NCO phase bits (32 now)
	return (uint32_t)res;
}

void addSymbols(char *dest, char *str, uint_fast8_t length, char *symbol, bool toEnd) // add zeroes
{
	char res[70] = "";
	strcpy(res, str);
	while (strlen(res) < length) {
		if (toEnd) {
			strcat(res, symbol);
		} else {
			char tmp[50] = "";
			strcat(tmp, symbol);
			strcat(tmp, res);
			strcpy(res, tmp);
		}
	}
	strcpy(dest, res);
}

float32_t log10f_fast(float32_t X) {
	float32_t Y, F;
	int32_t E;
	F = frexpf(fabsf(X), &E);
	Y = 1.23149591368684f;
	Y *= F;
	Y += -4.11852516267426f;
	Y *= F;
	Y += 6.02197014179219f;
	Y *= F;
	Y += -3.13396450166353f;
	Y += E;
	return (Y * 0.3010299956639812f);
}

float32_t db2rateV(float32_t i) // from decibels to times (for voltage)
{
	return powf(10.0f, (i / 20.0f));
}

float32_t db2rateP(float32_t i) // from decibels to times (for power)
{
	return powf(10.0f, (i / 10.0f));
}

float32_t rate2dbV(float32_t i) // times to decibels (for voltage)
{
	return 20 * log10f_fast(i);
}

float32_t rate2dbP(float32_t i) // from times to decibels (for power)
{
	return 10 * log10f_fast(i);
}

#define VOLUME_LOW_DB (-50.0f)
#define VOLUME_EPSILON powf(10.0f, (VOLUME_LOW_DB / 20.0f))
float32_t volume2rate(float32_t i) // from the position of the volume knob to the gain
{
	float32_t mute_zone = MAX_VOLUME_VALUE * 0.01f;
	if (MAX_VOLUME_VALUE == 100.0f) {
		mute_zone = 1.0f;
	}

	if (i <= 0.0f) {
		return 0.0f;
	}

#if !defined(FRONTPANEL_LITE) && !defined(FRONTPANEL_X1)
	if (i < (mute_zone / MAX_VOLUME_VALUE)) { // mute zone
		return 0.0f;
	}
#endif

	return powf(VOLUME_EPSILON, (1.0f - i));
}

void shiftTextLeft(char *string, uint_fast16_t shiftLength) {
	uint_fast16_t size = strlen(string);
	if (shiftLength >= size) {
		dma_memset(string, '\0', size);
		return;
	}
	for (uint_fast16_t i = 0; i < size - shiftLength; i++) {
		string[i] = string[i + shiftLength];
		string[i + shiftLength] = '\0';
	}
}

float32_t getMaxTXAmplitudeOnFreq(uint64_t freq) {
	uint16_t calibrate_level = 0;

	if (freq < 0.2 * 1000000) {
		calibrate_level = CALIBRATE.rf_out_power_2200m;
	}
	if (freq < 1.0 * 1000000) {
		calibrate_level = CALIBRATE.rf_out_power_630m;
	} else if (freq < 2.5 * 1000000) {
		calibrate_level = CALIBRATE.rf_out_power_160m;
	} else if (freq < 4.0 * 1000000) {
		calibrate_level = CALIBRATE.rf_out_power_80m;
	} else if (freq < 6.0 * 1000000) {
		calibrate_level = CALIBRATE.rf_out_power_60m;
	} else if (freq < 8.5 * 1000000) {
		calibrate_level = CALIBRATE.rf_out_power_40m;
	} else if (freq < 12.0 * 1000000) {
		calibrate_level = CALIBRATE.rf_out_power_30m;
	} else if (freq < 16.0 * 1000000) {
		calibrate_level = CALIBRATE.rf_out_power_20m;
	} else if (freq < 19.5 * 1000000) {
		calibrate_level = CALIBRATE.rf_out_power_17m;
	} else if (freq < 22.5 * 1000000) {
		calibrate_level = CALIBRATE.rf_out_power_15m;
	} else if (freq < 26.5 * 1000000) {
		calibrate_level = CALIBRATE.rf_out_power_12m;
	} else if (freq < 28.0 * 1000000) {
		calibrate_level = CALIBRATE.rf_out_power_cb;
	} else if (freq < 40.0 * 1000000) {
		calibrate_level = CALIBRATE.rf_out_power_10m;
	} else if (freq < 60.0 * 1000000) {
		calibrate_level = CALIBRATE.rf_out_power_6m;
	} else if (freq < 110.0 * 1000000) {
		calibrate_level = CALIBRATE.rf_out_power_4m;
	} else if (freq < 300.0 * 1000000) {
		calibrate_level = CALIBRATE.rf_out_power_2m;
	} else if (freq < 1000.0 * 1000000) {
		calibrate_level = CALIBRATE.rf_out_power_70cm;
	} else if (freq < 2000.0 * 1000000) {
		calibrate_level = CALIBRATE.rf_out_power_23cm;
	} else if (freq < 4000.0 * 1000000) {
		calibrate_level = CALIBRATE.rf_out_power_13cm;
	} else if (freq < 8000.0 * 1000000) {
		calibrate_level = CALIBRATE.rf_out_power_6cm;
	} else if (freq < 10489500000) {
		calibrate_level = CALIBRATE.rf_out_power_3cm;
	} else if (freq < 15000000000) {
		calibrate_level = CALIBRATE.rf_out_power_QO100;
	} else {
		calibrate_level = CALIBRATE.rf_out_power_1_2cm;
	}

	if (calibrate_level > 100) {
		calibrate_level = 100;
	}

	if (CALIBRATE.DAC_driver_mode == 2) // dac driver bias
	{
		TRX_DAC_DRV_A0 = false;
		TRX_DAC_DRV_A1 = false;
	} else if (CALIBRATE.DAC_driver_mode == 1) // dac driver bias 75%
	{
		TRX_DAC_DRV_A0 = true;
		TRX_DAC_DRV_A1 = false;
	} else if (CALIBRATE.DAC_driver_mode == 0) // dac driver bias 50%
	{
		TRX_DAC_DRV_A0 = false;
		TRX_DAC_DRV_A1 = true;
	} else if (calibrate_level == 0) // dac driver off
	{
		TRX_DAC_DRV_A0 = true;
		TRX_DAC_DRV_A1 = true;
	}

	return (float32_t)calibrate_level / 100.0f;
}

float32_t generateSin(float32_t amplitude, float32_t *index, float32_t samplerate, float32_t freq) {
	float32_t ret = amplitude * fast_sin(*index * F_2PI);
	*index += freq / samplerate;
	while (*index >= 1.0f) {
		*index -= 1.0f;
	}
	return ret;
}

float32_t generateSinWithZeroCrossing(float32_t amplitude, float32_t *index, float32_t samplerate, float32_t *prev_freq, float32_t freq) {
	float32_t ret = amplitude * fast_sin(*index * F_2PI);

	*index += *prev_freq / samplerate;

	while (*index >= 1.0f) {
		*index -= 1.0f;
		*prev_freq = freq;
	}

	return ret;
}

inline int32_t convertToSPIBigEndian(int32_t in) { return (int32_t)(0xFFFF0000 & (uint32_t)(in << 16)) | (int32_t)(0x0000FFFF & (uint32_t)(in >> 16)); }

inline uint8_t rev8(uint8_t data) {
	uint32_t tmp = data;
	return (uint8_t)(__RBIT(tmp) >> 24);
}

IRAM2 uint8_t SPI_tmp_buff[8] = {0};
bool SPI_Transmit(SPI_HandleTypeDef *hspi, uint8_t *out_data, uint8_t *in_data, uint32_t count, GPIO_TypeDef *CS_PORT, uint16_t CS_PIN, bool hold_cs, uint32_t prescaler, bool dma) {
	// SPI speed
	if (hspi->Init.BaudRatePrescaler != prescaler) {
		hspi->Init.BaudRatePrescaler = prescaler;
		HAL_SPI_Init(hspi);
	}

	const uint32_t SPI_timeout = 1000;
	const uint32_t SPI_DMA_timeout = 1000;

	HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_RESET);
	HAL_StatusTypeDef res = 0;

	if (count < 100) {
		dma = false;
	}

#ifdef STM32H743xx
	// non-DMA section
	if (dma && out_data == NULL && (uint32_t)in_data < 0x24000000) {
		dma = false;
	}
	if (dma && in_data == NULL && (uint32_t)out_data < 0x24000000) {
		dma = false;
	}
	if (dma && in_data != NULL && out_data != NULL) {
		if ((uint32_t)out_data < 0x24000000 || (uint32_t)in_data < 0x24000000) {
			dma = false;
		}
	}
#endif

	if (dma) {
		uint32_t startTime = HAL_GetTick();
		SPI_DMA_TXRX_ready_callback = false;
		if (in_data == NULL) {
			dma_memset(SPI_tmp_buff, 0x00, sizeof(SPI_tmp_buff));
			Aligned_CleanDCache_by_Addr((uint32_t)out_data, count);

			if (hspi->hdmarx->Init.MemInc != DMA_MINC_DISABLE) {
				hspi->hdmarx->Init.MemInc = DMA_MINC_DISABLE;
				HAL_DMA_Init(hspi->hdmarx);
			}
			if (hspi->hdmatx->Init.MemInc != DMA_MINC_ENABLE) {
				hspi->hdmatx->Init.MemInc = DMA_MINC_ENABLE;
				HAL_DMA_Init(hspi->hdmatx);
			}
			res = HAL_SPI_TransmitReceive_DMA(hspi, out_data, SPI_tmp_buff, count);

			while (!SPI_DMA_TXRX_ready_callback && ((HAL_GetTick() - startTime) < SPI_DMA_timeout)) {
				CPULOAD_GoToSleepMode();
			}
		} else if (out_data == NULL) {
			dma_memset(in_data, 0x00, count);

			/*if (hspi->hdmarx->Init.MemInc != DMA_MINC_ENABLE)
			{
			  hspi->hdmarx->Init.MemInc = DMA_MINC_ENABLE;
			  HAL_DMA_Init(hspi->hdmarx);
			}
			if (hspi->hdmatx->Init.MemInc != DMA_MINC_DISABLE)
			{
			  hspi->hdmatx->Init.MemInc = DMA_MINC_DISABLE;
			  HAL_DMA_Init(hspi->hdmatx);
			}*/

			res = HAL_SPI_Receive_IT(hspi, in_data, count);
			while (HAL_SPI_GetState(hspi) != HAL_SPI_STATE_READY && (HAL_GetTick() - startTime) < SPI_timeout) {
				CPULOAD_GoToSleepMode();
			}

			// res = HAL_SPI_TransmitReceive_DMA(hspi, SPI_tmp_buff, in_data, count);
			// res = HAL_SPI_Receive_DMA(hspi, in_data, count);
			// while (!SPI_DMA_TXRX_ready_callback && ((HAL_GetTick() - startTime) < SPI_DMA_timeout))
			// CPULOAD_GoToSleepMode();
		} else {
			dma_memset(in_data, 0x00, count);
			Aligned_CleanDCache_by_Addr((uint32_t)out_data, count);

			if (hspi->hdmarx->Init.MemInc != DMA_MINC_ENABLE) {
				hspi->hdmarx->Init.MemInc = DMA_MINC_ENABLE;
				HAL_DMA_Init(hspi->hdmarx);
			}
			if (hspi->hdmatx->Init.MemInc != DMA_MINC_ENABLE) {
				hspi->hdmatx->Init.MemInc = DMA_MINC_ENABLE;
				HAL_DMA_Init(hspi->hdmatx);
			}
			res = HAL_SPI_TransmitReceive_DMA(hspi, out_data, in_data, count);

			while (!SPI_DMA_TXRX_ready_callback && ((HAL_GetTick() - startTime) < SPI_DMA_timeout)) {
				CPULOAD_GoToSleepMode();
			}
		}

		Aligned_CleanInvalidateDCache_by_Addr((uint32_t)in_data, count);

		if ((HAL_GetTick() - startTime) > SPI_DMA_timeout) {
			res = HAL_TIMEOUT;
		}
	} else {
		__SPI2_CLK_ENABLE();
		if (in_data == NULL) {
			res = HAL_SPI_Transmit_IT(hspi, out_data, count);
		} else if (out_data == NULL) {
			dma_memset(in_data, 0x00, count);
			res = HAL_SPI_Receive_IT(hspi, in_data, count);
		} else {
			dma_memset(in_data, 0x00, count);
			res = HAL_SPI_TransmitReceive_IT(hspi, out_data, in_data, count);
		}
		uint32_t startTime = HAL_GetTick();
		while (HAL_SPI_GetState(hspi) != HAL_SPI_STATE_READY && (HAL_GetTick() - startTime) < SPI_timeout) {
			CPULOAD_GoToSleepMode();
		}
	}

	if (HAL_SPI_GetError(hspi) != 0) {
		res = HAL_ERROR;
	}

	if (!hold_cs) {
		HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_SET);
	}

	if (res == HAL_TIMEOUT) {
		println("[ERR] SPI timeout");

		// HAL_SPI_Abort(hspi);
		return false;
	}
	if (res == HAL_ERROR) {
		print("[ERR] SPI error, code: ", hspi->ErrorCode, " COUNT: ", count, " MODE: ");
		if (in_data == NULL) {
			println("TX");
		} else if (out_data == NULL) {
			println("RX");
		} else {
			println("TXRX");
		}

		// HAL_SPI_Abort(hspi);
		return false;
	}

	return true;
}

/*
 *  This Quickselect routine is based on the algorithm described in
 *  "Numerical recipes in C", Second Edition,
 *  Cambridge University Press, 1992, Section 8.5, ISBN 0-521-43108-5
 *  This code by Nicolas Devillard - 1998. Public domain.
 */
float32_t quick_median_select(float32_t *arr, int n) {
	int low, high;
	int median;
	int middle, ll, hh;

	low = 0;
	high = n - 1;
	median = (low + high) / 2;
	for (;;) {
		if (high <= low) { /* One element only */
			return arr[median];
		}

		if (high == low + 1) { /* Two elements only */
			if (arr[low] > arr[high]) {
				ELEM_SWAP_F32(arr[low], arr[high]);
			}
			return arr[median];
		}

		/* Find median of low, middle and high items; swap into position low */
		middle = (low + high) / 2;
		if (arr[middle] > arr[high]) {
			ELEM_SWAP_F32(arr[middle], arr[high]);
		}
		if (arr[low] > arr[high]) {
			ELEM_SWAP_F32(arr[low], arr[high]);
		}
		if (arr[middle] > arr[low]) {
			ELEM_SWAP_F32(arr[middle], arr[low]);
		}

		/* Swap low item (now in position middle) into position (low+1) */
		ELEM_SWAP_F32(arr[middle], arr[low + 1]);

		/* Nibble from each end towards middle, swapping items when stuck */
		ll = low + 1;
		hh = high;
		for (;;) {
			do {
				ll++;
			} while (arr[low] > arr[ll]);
			do {
				hh--;
			} while (arr[hh] > arr[low]);

			if (hh < ll) {
				break;
			}

			ELEM_SWAP_F32(arr[ll], arr[hh]);
		}

		/* Swap middle item (in position low) back into correct position */
		ELEM_SWAP_F32(arr[low], arr[hh]);

		/* Re-set active partition */
		if (hh <= median) {
			low = ll;
		}
		if (hh >= median) {
			high = hh - 1;
		}
	}
}

void memset16(void *dest, uint16_t val, uint32_t size) {
	if (size == 0) {
		return;
	}

	uint16_t *buf = dest;
	while (size--) {
		*buf++ = val;
	}

	/*char *buf = dest;
	union
	{
	    uint8_t d8[2];
	    uint16_t d16;
	}u16 = {.d16 = val};

	while(size--)
	{
	    *buf++ = u16.d8[0];
	    *buf++ = u16.d8[1];
	}*/
}

__WEAK void dma_memset(void *dest, uint8_t val, uint32_t size) {
	if (dma_memset32_busy || size < 128) // for async and fast calls
	{
		memset(dest, val, size);
		return;
	}

	// left align
	char *pDst = (char *)dest;
	while ((uint32_t)pDst != (((uint32_t)pDst) & ~(uint32_t)0x3) && size > 0) {
		*pDst++ = val;
		size--;
	}

	if (size > 0) {
		// center fills in 32bit
		uint32_t val32 = (val << 24) | (val << 16) | (val << 8) | (val << 0);
		uint32_t block32 = size / 4;
		uint32_t block8 = size % 4;
		dma_memset32(pDst, val32, block32);

		// right align
		if (block8 > 0) {
			pDst += block32 * 4;
			while (block8--) {
				*pDst++ = val;
			}
		}
	}
}

__WEAK void dma_memcpy(void *dest, void *src, uint32_t size) {
	if (dma_memcpy32_busy || size < 1024) // for async and fast calls
	{
		memcpy(dest, src, size);
		return;
	}

	// left align
	char *pSrc = (char *)src;
	char *pDst = (char *)dest;
	while (((uint32_t)pSrc != (((uint32_t)pSrc) & ~(uint32_t)0x3) || (uint32_t)pDst != (((uint32_t)pDst) & ~(uint32_t)0x3)) && size > 0) {
		*pDst++ = *pSrc++;
		size--;
	}

	if (size > 0) {
		// center copy in 32bit
		uint32_t block32 = size / 4;
		uint32_t block8 = size % 4;
		uint32_t max_block = DMA_MAX_BLOCK / 4;
		while (block32 > max_block) {
			dma_memcpy32(pDst, pSrc, max_block);
			block32 -= max_block;
			pDst += max_block * 4;
			pSrc += max_block * 4;
		}
		dma_memcpy32(pDst, pSrc, block32);

		// right align
		if (block8 > 0) {
			pDst += block32 * 4;
			pSrc += block32 * 4;
			while (block8--) {
				*pDst++ = *pSrc++;
			}
		}
	}
}

#if HRDW_HAS_MDMA
void SLEEPING_MDMA_PollForTransfer(MDMA_HandleTypeDef *hmdma) {
	const uint32_t Timeout = 100;
	uint32_t tickstart;

	if (HAL_MDMA_STATE_BUSY != hmdma->State) {
		return;
	}

	/* Get timeout */
	tickstart = HAL_GetTick();

	while (__HAL_MDMA_GET_FLAG(hmdma, MDMA_FLAG_CTC) == 0U) {
		if ((__HAL_MDMA_GET_FLAG(hmdma, MDMA_FLAG_TE) != 0U)) {
			(void)HAL_MDMA_Abort(hmdma); /* if error then abort the current transfer */
			return;
		}

		/* Check for the Timeout */
		if (((HAL_GetTick() - tickstart) > Timeout) || (Timeout == 0U)) {
			(void)HAL_MDMA_Abort(hmdma); /* if timeout then abort the current transfer */
			return;
		}

		// go sleep
		CPULOAD_GoToSleepMode();
	}

	/* Clear the transfer level flag */
	__HAL_MDMA_CLEAR_FLAG(hmdma, (MDMA_FLAG_BRT | MDMA_FLAG_BT | MDMA_FLAG_BFTC | MDMA_FLAG_CTC));

	/* Process unlocked */
	__HAL_UNLOCK(hmdma);

	hmdma->State = HAL_MDMA_STATE_READY;
}
#endif

typedef struct {
	__IO uint32_t ISR; /*!< DMA interrupt status register */
	__IO uint32_t Reserved0;
	__IO uint32_t IFCR; /*!< DMA interrupt flag clear register */
} DMA_Base_Registers;

void SLEEPING_DMA_PollForTransfer(DMA_HandleTypeDef *hdma) {
	const uint32_t Timeout = 100;

	HAL_StatusTypeDef status = HAL_OK;
	uint32_t mask_cpltlevel;
	uint32_t tickstart = HAL_GetTick();
	uint32_t tmpisr;

	/* calculate DMA base and stream number */
	DMA_Base_Registers *regs;

	if (HAL_DMA_STATE_BUSY != hdma->State) {
		return;
	}

	/* Transfer Complete flag */
	mask_cpltlevel = DMA_FLAG_TCIF0_4 << hdma->StreamIndex;

	regs = (DMA_Base_Registers *)hdma->StreamBaseAddress;
	tmpisr = regs->ISR;

	while (((tmpisr & mask_cpltlevel) == RESET) && ((hdma->ErrorCode & HAL_DMA_ERROR_TE) == RESET)) {
		/* Check for the Timeout (Not applicable in circular mode)*/
		if (((HAL_GetTick() - tickstart) > Timeout)) {
			/* Update error code */
			hdma->ErrorCode = HAL_DMA_ERROR_TIMEOUT;

			/* Change the DMA state */
			hdma->State = HAL_DMA_STATE_READY;

			/* Process Unlocked */
			__HAL_UNLOCK(hdma);

			println("[ERR] DMA Timeout");

			return;
		}

		/* Get the ISR register value */
		tmpisr = regs->ISR;

		if ((tmpisr & (DMA_FLAG_TEIF0_4 << hdma->StreamIndex)) != RESET) {
			/* Update error code */
			hdma->ErrorCode |= HAL_DMA_ERROR_TE;

			/* Clear the transfer error flag */
			regs->IFCR = DMA_FLAG_TEIF0_4 << hdma->StreamIndex;
		}

		if ((tmpisr & (DMA_FLAG_FEIF0_4 << hdma->StreamIndex)) != RESET) {
			/* Update error code */
			hdma->ErrorCode |= HAL_DMA_ERROR_FE;

			/* Clear the FIFO error flag */
			regs->IFCR = DMA_FLAG_FEIF0_4 << hdma->StreamIndex;
		}

		if ((tmpisr & (DMA_FLAG_DMEIF0_4 << hdma->StreamIndex)) != RESET) {
			/* Update error code */
			hdma->ErrorCode |= HAL_DMA_ERROR_DME;

			/* Clear the Direct Mode error flag */
			regs->IFCR = DMA_FLAG_DMEIF0_4 << hdma->StreamIndex;
		}

		// go sleep
		CPULOAD_GoToSleepMode();
	}

	if (hdma->ErrorCode != HAL_DMA_ERROR_NONE) {
		if ((hdma->ErrorCode & HAL_DMA_ERROR_TE) != RESET) {
			HAL_DMA_Abort(hdma);

			/* Clear the half transfer and transfer complete flags */
			regs->IFCR = (DMA_FLAG_HTIF0_4 | DMA_FLAG_TCIF0_4) << hdma->StreamIndex;

			/* Change the DMA state */
			hdma->State = HAL_DMA_STATE_READY;

			/* Process Unlocked */
			__HAL_UNLOCK(hdma);

			return;
		}
	}

	/* Clear the half transfer and transfer complete flags */
	regs->IFCR = (DMA_FLAG_HTIF0_4 | DMA_FLAG_TCIF0_4) << hdma->StreamIndex;

	hdma->State = HAL_DMA_STATE_READY;

	/* Process Unlocked */
	__HAL_UNLOCK(hdma);
}

uint8_t getInputType(void) {
	uint8_t type = TRX.InputType_MAIN;
	if (CurrentVFO->Mode == TRX_MODE_DIGI_L || CurrentVFO->Mode == TRX_MODE_DIGI_U || CurrentVFO->Mode == TRX_MODE_RTTY || CurrentVFO->Mode == TRX_MODE_IQ) {
		type = TRX.InputType_DIGI;
	}
	return type;
}

#if HRDW_HAS_SD
/* CRC16 table (for SD data) */
static unsigned int sd_crc16_table[256];
/* CRC7 table (for SD commands) */
static unsigned int sd_crc7_table[256];

/* Running CRC16 calculation for a byte. */
unsigned int sd_crc16_byte(unsigned int crcval, unsigned int byte) { return (sd_crc16_table[(byte ^ (crcval >> 8)) & 0xFFU] ^ (crcval << 8)) & 0xFFFFU; }

/* Running CRC7 calculation for a byte. */
unsigned int sd_crc7_byte(unsigned int crcval, unsigned int byte) { return sd_crc7_table[(byte ^ (crcval << 1)) & 0xFFU]; }

void sd_crc_generate_table(void) {
	static bool crc_table_generated = false;
	if (crc_table_generated) {
		return;
	}
	int crc = 0;
	/* Generate CRC16 table */
	for (uint32_t byt = 0U; byt < 256U; byt++) {
		crc = byt << 8;
		for (uint32_t bit = 0U; bit < 8U; bit++) {
			crc <<= 1;
			if ((crc & 0x10000U) != 0U) {
				crc ^= 0x1021U;
			}
		}
		sd_crc16_table[byt] = (crc & 0xFFFFU);
	}

	/* Generate CRC7 table */
	for (uint32_t byt = 0U; byt < 256U; byt++) {
		crc = byt;
		if ((crc & 0x80U) != 0U) {
			crc ^= 0x89U;
		}
		for (uint32_t bit = 1U; bit < 8U; bit++) {
			crc <<= 1;
			if ((crc & 0x80U) != 0U) {
				crc ^= 0x89U;
			}
		}
		sd_crc7_table[byt] = (crc & 0x7FU);
	}
	crc_table_generated = true;
}
#endif

void arm_biquad_cascade_df2T_f32_single(const arm_biquad_cascade_df2T_instance_f32 *S, const float32_t *pSrc, float32_t *pDst, uint32_t blockSize) {
	float32_t *pState = S->pState;
	const float32_t *pCoeffs = S->pCoeffs;
	const float32_t *pIn = pSrc;
	float32_t *pOut = pDst;

	for (uint32_t stage = 0; stage < S->numStages; stage++) {
		float32_t b0 = *pCoeffs++;
		float32_t b1 = *pCoeffs++;
		float32_t b2 = *pCoeffs++;
		float32_t a1 = *pCoeffs++;
		float32_t a2 = *pCoeffs++;

		float32_t d1 = pState[0];
		float32_t d2 = pState[1];

		for (uint32_t sample = 0; sample < blockSize; sample++) {
			float32_t Xn1 = *pIn++;

			float32_t acc1 = b0 * Xn1 + d1;
			d1 = b1 * Xn1 + d2 + a1 * acc1;
			d2 = b2 * Xn1 + a2 * acc1;

			*pOut++ = acc1;
		}

		*pState++ = d1;
		*pState++ = d2;

		pIn = pDst;
		pOut = pDst;
	}
}

void arm_biquad_cascade_df2T_f32_IQ(const arm_biquad_cascade_df2T_instance_f32 *I, const arm_biquad_cascade_df2T_instance_f32 *Q, const float32_t *pSrc_I, const float32_t *pSrc_Q,
                                    float32_t *pDst_I, float32_t *pDst_Q, uint32_t blockSize) {
	float32_t *pState_I = I->pState;
	float32_t *pState_Q = Q->pState;
	const float32_t *pCoeffs = I->pCoeffs;
	const float32_t *pIn_I = pSrc_I;
	const float32_t *pIn_Q = pSrc_Q;
	float32_t *pOut_I = pDst_I;
	float32_t *pOut_Q = pDst_Q;
	uint32_t stage = I->numStages;

	while (stage > 0) {
		float32_t b0 = *pCoeffs++;
		float32_t b1 = *pCoeffs++;
		float32_t b2 = *pCoeffs++;
		float32_t a1 = *pCoeffs++;
		float32_t a2 = *pCoeffs++;

		float32_t d1_I = pState_I[0];
		float32_t d2_I = pState_I[1];
		float32_t d1_Q = pState_Q[0];
		float32_t d2_Q = pState_Q[1];

		uint32_t sample = blockSize;
		while (sample > 0) {
			float32_t Xn1_I = *pIn_I++;
			float32_t acc1_I = b0 * Xn1_I + d1_I;
			d1_I = b1 * Xn1_I + d2_I + a1 * acc1_I;
			d2_I = b2 * Xn1_I + a2 * acc1_I;
			*pOut_I++ = acc1_I;

			float32_t Xn1_Q = *pIn_Q++;
			float32_t acc1_Q = b0 * Xn1_Q + d1_Q;
			d1_Q = b1 * Xn1_Q + d2_Q + a1 * acc1_Q;
			d2_Q = b2 * Xn1_Q + a2 * acc1_Q;
			*pOut_Q++ = acc1_Q;

			sample--;
		}

		*pState_I++ = d1_I;
		*pState_I++ = d2_I;
		*pState_Q++ = d1_Q;
		*pState_Q++ = d2_Q;

		pIn_I = pDst_I;
		pIn_Q = pDst_Q;
		pOut_I = pDst_I;
		pOut_Q = pDst_Q;

		stage--;
	}

	/* __asm volatile(
	    // -------- START STAGES WHILE LOOP
	    "1: \n"
	    // float32_t b0 = pCoeffs[0];
	    // float32_t b1 = pCoeffs[1];
	    // float32_t b2 = pCoeffs[2];
	    // float32_t a1 = pCoeffs[3];
	    // float32_t a2 = pCoeffs[4];
	    // pCoeffs += 5U;
	    "VLDMIA.F32 %[pCoeffs]!, {s8-s12} \n"
	    // float32_t d1_I = pState_I[0];
	    // float32_t d2_I = pState_I[1];
	    // float32_t d1_Q = pState_Q[0];
	    // float32_t d2_Q = pState_Q[1];
	    "VLDMIA.F32 %[pState_I], {s4-s5} \n"
	    "VLDMIA.F32 %[pState_Q], {s6-s7} \n"
	    // uint32_t sample = blockSize;
	    "MOV r0, %[blockSize] \n"
	    // -------- START SAMPLES WHILE LOOP
	    "2: \n"
	    // float32_t Xn1_I = *pIn_I++;
	    // float32_t Xn1_Q = *pIn_Q++;
	    "VLDMIA.F32 %[pIn_I]!, {s0} \n"
	    "VLDMIA.F32 %[pIn_Q]!, {s1} \n"
	    // float32_t acc1_I = b0 * Xn1_I + d1_I;
	    // float32_t acc1_Q = b0 * Xn1_Q + d1_Q;
	    "VMOV.F32 s2, s4 \n"
	    "VMOV.F32 s3, s6 \n"
	    "VMLA.F32 s2, s8, s0 \n"
	    "VMLA.F32 s3, s8, s1 \n"
	    // d1_I = b1 * Xn1_I + d2_I + a1 * acc1_I;
	    // d1_Q = b1 * Xn1_Q + d2_Q + a1 * acc1_Q;
	    "VMOV.F32 s4, s5 \n"
	    "VMOV.F32 s6, s7 \n"
	    "VMLA.F32 s4, s9, s0 \n"
	    "VMLA.F32 s6, s9, s1 \n"
	    "VMLA.F32 s4, s11, s2 \n"
	    "VMLA.F32 s6, s11, s3 \n"
	    // d2_I = b2 * Xn1_I + a2 * acc1_I;
	    // d2_Q = b2 * Xn1_Q + a2 * acc1_Q;
	    "VMUL.F32 s5, s10, s0 \n"
	    "VMUL.F32 s7, s10, s1 \n"
	    "VMLA.F32 s5, s12, s2 \n"
	    "VMLA.F32 s7, s12, s3 \n"
	    // *pOut_I++ = acc1_I;
	    // *pOut_Q++ = acc1_Q;
	    "VSTMIA.F32 %[pOut_I]!, {s2} \n"
	    "VSTMIA.F32 %[pOut_Q]!, {s3} \n"
	    // sample--;
	    "SUB r0, r0, #1 \n"
	    // while (sample > 0) {
	    "CMP r0, #0 \n"
	    "IT GT \n"
	    "BGT 2b \n"
	    // -------- END SAMPLES WHILE LOOP
	    // pState_I[0] = d1_I;
	    // pState_I[1] = d2_I;
	    // pState_I += 2U;
	    "VSTMIA.F32 %[pState_I]!, {s4-s5} \n"
	    // pState_Q[0] = d1_Q;
	    // pState_Q[1] = d2_Q;
	    // pState_Q += 2U;
	    "VSTMIA.F32 %[pState_Q]!, {s6-s7} \n"
	    // pIn_I = pDst_I;
	    // pIn_Q = pDst_Q;
	    // pOut_I = pDst_I;
	    // pOut_Q = pDst_Q;
	    "MOV %[pIn_I], %[pDst_I] \n"
	    "MOV %[pIn_Q], %[pDst_Q] \n"
	    "MOV %[pOut_I], %[pDst_I] \n"
	    "MOV %[pOut_Q], %[pDst_Q] \n"
	    // stage--;
	    "SUB %[stage], %[stage], #1 \n"
	    // 	while (stage > 0) {
	    "CMP %[stage], #0 \n"
	    "IT GT \n"
	    "BGT 1b \n"
	    // -------- END SAMPLES WHILE LOOP
	    : [pCoeffs] "+r"(pCoeffs), [pIn_I] "+r"(pIn_I), [pIn_Q] "+r"(pIn_Q), [pOut_I] "+r"(pOut_I), [pOut_Q] "+r"(pOut_Q), [pState_I] "+r"(pState_I), [pState_Q] "+r"(pState_Q),
	      [pDst_I] "+r"(pDst_I), [pDst_Q] "+r"(pDst_Q), [stage] "+r"(stage)
	    : [blockSize] "r"(blockSize)
	    : "r0", "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "s12" // "memory", "cc",
	); */
}

char cleanASCIIgarbage(char chr) {
	if ((chr < ' ') || (chr > 0x7f)) {
		return 0;
	}
	return chr;
}

bool textStartsWith(const char *a, const char *b) {
	if (strncmp(a, b, strlen(b)) == 0) {
		return 1;
	}
	return 0;
}

#if HRDW_HAS_FULL_FFT_BUFFER
void *alloc_to_wtf(uint32_t size, bool reset) {
	static uint32_t allocated = 0;
	if (reset) {
		allocated = 0;
		dma_memset(print_output_buffer, 0x00, sizeof(print_output_buffer));
	}
	size = (size + 0x03) & ~0x03;
	if ((allocated + size) > sizeof(print_output_buffer)) {
		LCD_showError("WTF ALLOC mem error", true);
	}
	void *p = (void *)((uint8_t *)print_output_buffer + allocated);
	allocated += size;
	return p;
}
#else
void *alloc_to_wtf(uint32_t size, bool reset) { return NULL; }
#endif

int8_t getPowerFromALC() {
	if (!CALIBRATE.ALC_Port_Enabled) {
		return -1;
	}

	float32_t volt = TRX_ALC_IN - 1.0f; // 0.0 - 1.0v - ALC disabled
	if (volt < 0.0f) {
		return -1;
	}

	float32_t power = 0;
	float32_t max_power_selected = (float32_t)TRX.RF_Gain;

	if (!CALIBRATE.ALC_Inverted_Logic) {
		power = volt * max_power_selected / 2.0f; // 1.0v - 3.0v - power 0-100%

		if (power < 0.0f) {
			power = 0.0f;
		}

		if (power > max_power_selected) {
			power = max_power_selected;
		}
	}

	if (CALIBRATE.ALC_Inverted_Logic) {
		power = volt * max_power_selected / 2.0f; // 1.0v - 3.0v - power 0-100%

		if (power < 0.0f) {
			power = 0.0f;
		}

		if (power > max_power_selected) {
			power = max_power_selected;
		}

		power = max_power_selected - power; // 1.0v - 3.0v - power 100-0%
	}

	return power;
}

void getUTCDateTime(RTC_DateTypeDef *sDate, RTC_TimeTypeDef *sTime) {
	HAL_RTC_GetTime(&hrtc, sTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, sDate, RTC_FORMAT_BIN);

	// println("UTC Date: ", sDate->Year, "-", sDate->Month, "-", sDate->Date);
	// println("UTC Time: ", sTime->Hours, "-", sTime->Minutes, "-", sTime->Seconds);
}

uint32_t getUTCTimestamp() {
	RTC_DateTypeDef sDate = {0};
	RTC_TimeTypeDef sTime = {0};
	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

	time_t timestamp;
	struct tm currTime;

	currTime.tm_year = sDate.Year + 100;
	currTime.tm_mon = sDate.Month - 1;
	currTime.tm_mday = sDate.Date;

	currTime.tm_hour = sTime.Hours;
	currTime.tm_min = sTime.Minutes;
	currTime.tm_sec = sTime.Seconds;

	return mktime(&currTime);
}

void getLocalDateTime(RTC_DateTypeDef *sDate, RTC_TimeTypeDef *sTime) {
	HAL_RTC_GetTime(&hrtc, sTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, sDate, RTC_FORMAT_BIN);

	// println("UTC Date: ", sDate->Year, "-", sDate->Month, "-", sDate->Date);
	// println("UTC Time: ", sTime->Hours, "-", sTime->Minutes, "-", sTime->Seconds);

	time_t timestamp;
	struct tm currTime;

	currTime.tm_year = sDate->Year + 100;
	currTime.tm_mon = sDate->Month - 1;
	currTime.tm_mday = sDate->Date;

	currTime.tm_hour = sTime->Hours;
	currTime.tm_min = sTime->Minutes;
	currTime.tm_sec = sTime->Seconds;

	timestamp = mktime(&currTime);

	timestamp += (int32_t)(WIFI.Timezone * 60.0f * 60.0f);

	currTime = *localtime(&timestamp);

	sDate->Year = currTime.tm_year - 100;
	sDate->Month = currTime.tm_mon + 1;
	sDate->Date = currTime.tm_mday;
	sTime->Hours = currTime.tm_hour;
	sTime->Minutes = currTime.tm_min;
	sTime->Seconds = currTime.tm_sec;

	// println("Local Date: ", sDate->Year, "-", sDate->Month, "-", sDate->Date);
	// println("Local Time: ", sTime->Hours, "-", sTime->Minutes, "-", sTime->Seconds);
}

const char *trim_front(const char *str) {
	while (*str == ' ') {
		str++;
	}
	return str;
}

static void trim_back(char *str) {
	int idx = strlen(str) - 1;
	while (idx >= 0 && str[idx] == ' ') {
		str[idx--] = '\0';
	}
}

char *trim(char *str) {
	str = (char *)trim_front(str);
	trim_back(str);
	return str;
}
