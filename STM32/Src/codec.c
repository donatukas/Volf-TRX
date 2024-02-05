#include "codec.h"
#include "agc.h"
#include "hardware.h"
#include "i2c.h"
#include "lcd.h"
#include "trx_manager.h"
#include "usbd_audio_if.h"

#define I2C_ADDRESS_WM8731 0x34 // audio codec address

// Public variables
uint32_t CODEC_DMA_samples = 0;                                    // count the number of samples passed to the audio codec
bool CODEC_DMA_state = true;                                       // what part of the buffer we are working with, true - compleate; false - half
bool CODEC_Buffer_underrun = false;                                // lack of data in the buffer from the audio processor
SRAM int32_t CODEC_Audio_Buffer_RX[CODEC_AUDIO_BUFFER_SIZE] = {0}; // audio codec ring buffers
SRAM int32_t CODEC_Audio_Buffer_TX[CODEC_AUDIO_BUFFER_SIZE] = {0};
bool CODEC_Beeping;            // Beeping flag
bool CODEC_Beeping_Left;       // Beeping flag in left channel
bool CODEC_Beeping_Right;      // Beeping flag in left channel
bool CODEC_Muting;             // Muting flag
bool CODEC_test_result = true; // self-test flag

// Private variables

// Prototypes
static HAL_StatusTypeDef HAL_I2S_TXRX_DMA(I2S_HandleTypeDef *hi2s, uint16_t *txData, uint16_t *rxData, uint16_t txSize,
                                          uint16_t rxSize); // Full-duplex implementation of I2S startup
static void I2S_DMATxCplt(DMA_HandleTypeDef *hdma);         // RX Buffer is fully sent to the codec
static void I2S_DMATxHalfCplt(DMA_HandleTypeDef *hdma);     // RX Buffer half sent to the codec
static void I2S_DMARxCplt(DMA_HandleTypeDef *hdma);         // TX Buffer is completely taken from the codec
static void I2S_DMARxHalfCplt(DMA_HandleTypeDef *hdma);     // TX Buffer half received from the codec
static void I2S_DMAError(DMA_HandleTypeDef *hdma);          // DMA I2S error

// start the I2S bus
void CODEC_start_i2s_and_dma(void) {
	CODEC_CleanBuffer();
	if (HAL_I2S_GetState(&HRDW_AUDIO_CODEC_I2S) == HAL_I2S_STATE_READY) {
		HAL_I2S_TXRX_DMA(&HRDW_AUDIO_CODEC_I2S, (uint16_t *)&CODEC_Audio_Buffer_RX[0], (uint16_t *)&CODEC_Audio_Buffer_TX[0], CODEC_AUDIO_BUFFER_SIZE * 2,
		                 CODEC_AUDIO_BUFFER_SIZE); // 32bit rx spi, 16bit tx spi
	}
}

// clear the audio codec and USB audio buffer
void CODEC_CleanBuffer(void) {
	dma_memset(CODEC_Audio_Buffer_RX, 0x00, sizeof CODEC_Audio_Buffer_RX);
	dma_memset(CODEC_Audio_Buffer_TX, 0x00, sizeof CODEC_Audio_Buffer_TX);
	Aligned_CleanDCache_by_Addr((uint32_t *)&CODEC_Audio_Buffer_RX[0], sizeof(CODEC_Audio_Buffer_RX));
	Aligned_CleanDCache_by_Addr((uint32_t *)&CODEC_Audio_Buffer_TX[0], sizeof(CODEC_Audio_Buffer_TX));
	dma_memset(USB_AUDIO_rx_buffer_a, 0x00, sizeof USB_AUDIO_rx_buffer_a);
	dma_memset(USB_AUDIO_rx_buffer_b, 0x00, sizeof USB_AUDIO_rx_buffer_a);
	dma_memset(USB_AUDIO_tx_buffer, 0x00, sizeof USB_AUDIO_tx_buffer);
	ResetAGC();
}

void CODEC_Mute(void) {
	CODEC_Muting = true;
	CODEC_Mute_AF_AMP();
}

void CODEC_UnMute(void) {
	CODEC_Muting = false;
	if (!TRX.AFAmp_Mute) {
		CODEC_UnMute_AF_AMP();
	}
}

void CODEC_Mute_AF_AMP(void) {
#ifdef AF_AMP_MUTE_Pin
	HAL_GPIO_WritePin(AF_AMP_MUTE_GPIO_Port, AF_AMP_MUTE_Pin, GPIO_PIN_RESET);
#endif
}

void CODEC_UnMute_AF_AMP(void) {
#ifdef AF_AMP_MUTE_Pin
	HAL_GPIO_WritePin(AF_AMP_MUTE_GPIO_Port, AF_AMP_MUTE_Pin, GPIO_PIN_SET);
#endif
}

void CODEC_Beep(void) {
	if (TRX.Beeper) {
		CODEC_Beeping_Left = true;
		CODEC_Beeping_Right = true;
		CODEC_Beeping = true;
	}
}

void CODEC_TestBeep(bool channel) {
	CODEC_Beeping_Left = !channel;
	CODEC_Beeping_Right = channel;
	CODEC_Beeping = true;
}

// RX Buffer is fully sent to the codec
static void I2S_DMATxCplt(DMA_HandleTypeDef *hdma) {
	CPULOAD_WakeUp();
	if (((I2S_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent)->Instance == SPI3) {
		if (Processor_NeedRXBuffer) { // if the audio codec did not provide data to the buffer, raise the error flag
			CODEC_Buffer_underrun = true;
		}
		CODEC_DMA_state = true;
		Processor_NeedRXBuffer = true;
		if (CurrentVFO->Mode == TRX_MODE_LOOPBACK) {
			Processor_NeedTXBuffer = true;
		}
		CODEC_DMA_samples += (CODEC_AUDIO_BUFFER_SIZE / 2);
	}
}

// RX Buffer half sent to the codec
static void I2S_DMATxHalfCplt(DMA_HandleTypeDef *hdma) {
	CPULOAD_WakeUp();
	if (((I2S_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent)->Instance == SPI3) {
		if (Processor_NeedRXBuffer) { // if the audio codec did not provide data to the buffer, raise the error flag
			CODEC_Buffer_underrun = true;
		}
		CODEC_DMA_state = false;
		Processor_NeedRXBuffer = true;
		if (CurrentVFO->Mode == TRX_MODE_LOOPBACK) {
			Processor_NeedTXBuffer = true;
		}
		CODEC_DMA_samples += (CODEC_AUDIO_BUFFER_SIZE / 2);
	}
}

// TX Buffer is completely taken from the codec
static void I2S_DMARxCplt(DMA_HandleTypeDef *hdma) {
	CPULOAD_WakeUp();

	I2S_HandleTypeDef *hi2s = (I2S_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;
	HAL_I2S_RxCpltCallback(hi2s);
}

// TX Buffer half received from the codec
static void I2S_DMARxHalfCplt(DMA_HandleTypeDef *hdma) {
	CPULOAD_WakeUp();

	I2S_HandleTypeDef *hi2s = (I2S_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;
	HAL_I2S_RxHalfCpltCallback(hi2s);
}

// DMA I2S error
static void I2S_DMAError(DMA_HandleTypeDef *hdma) {
	CPULOAD_WakeUp();

#ifdef STM32H743xx
	I2S_HandleTypeDef *hi2s = (I2S_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent; /* Derogation MISRAC2012-Rule-11.5 */

	/* Disable Rx and Tx DMA Request */
	CLEAR_BIT(hi2s->Instance->CFG1, (SPI_CFG1_RXDMAEN | SPI_CFG1_TXDMAEN));
	hi2s->TxXferCount = (uint16_t)0UL;
	hi2s->RxXferCount = (uint16_t)0UL;

	hi2s->State = HAL_I2S_STATE_READY;

	/* Set the error code and execute error callback*/
	SET_BIT(hi2s->ErrorCode, HAL_I2S_ERROR_DMA);

	/* Call user error callback */
	HAL_I2S_ErrorCallback(hi2s);
#endif

#ifdef STM32F407xx
	I2S_HandleTypeDef *hi2s = (I2S_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent; /* Derogation MISRAC2012-Rule-11.5 */

	/* Disable Rx and Tx DMA Request */
	CLEAR_BIT(hi2s->Instance->CR2, (SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN));
	hi2s->TxXferCount = (uint16_t)0UL;
	hi2s->RxXferCount = (uint16_t)0UL;

	hi2s->State = HAL_I2S_STATE_READY;

	/* Set the error code and execute error callback*/
	SET_BIT(hi2s->ErrorCode, HAL_I2S_ERROR_DMA);

	/* Call user error callback */
	HAL_I2S_ErrorCallback(hi2s);
#endif
}

// Full-duplex implementation of I2S startup
static HAL_StatusTypeDef HAL_I2S_TXRX_DMA(I2S_HandleTypeDef *hi2s, uint16_t *txData, uint16_t *rxData, uint16_t txSize, uint16_t rxSize) {
#ifdef STM32H743xx
	if ((rxData == NULL) || (txData == NULL) || (rxSize == 0UL) || (txSize == 0UL)) {
		return HAL_ERROR;
	}

	/* Process Locked */
	__HAL_LOCK(hi2s);

	if (hi2s->State != HAL_I2S_STATE_READY) {
		__HAL_UNLOCK(hi2s);
		return HAL_BUSY;
	}

	/* Set state and reset error code */
	hi2s->State = HAL_I2S_STATE_BUSY;
	hi2s->ErrorCode = HAL_I2S_ERROR_NONE;
	hi2s->pRxBuffPtr = rxData;
	hi2s->RxXferSize = rxSize;
	hi2s->RxXferCount = rxSize;
	hi2s->pTxBuffPtr = txData;
	hi2s->TxXferSize = txSize;
	hi2s->TxXferCount = txSize;

	hi2s->hdmarx->XferHalfCpltCallback = I2S_DMARxHalfCplt;
	hi2s->hdmarx->XferCpltCallback = I2S_DMARxCplt;
	hi2s->hdmarx->XferErrorCallback = I2S_DMAError;
	hi2s->hdmatx->XferHalfCpltCallback = I2S_DMATxHalfCplt;
	hi2s->hdmatx->XferCpltCallback = I2S_DMATxCplt;
	hi2s->hdmatx->XferErrorCallback = I2S_DMAError;

	/* Enable the Rx DMA Stream/Channel */
	if (HAL_OK != HAL_DMA_Start_IT(hi2s->hdmarx, (uint32_t)&hi2s->Instance->RXDR, (uint32_t)hi2s->pRxBuffPtr, hi2s->RxXferSize)) {
		// Update SPI error code
		SET_BIT(hi2s->ErrorCode, HAL_I2S_ERROR_DMA);
		hi2s->State = HAL_I2S_STATE_READY;

		__HAL_UNLOCK(hi2s);
		return HAL_ERROR;
	}
	if (HAL_OK != HAL_DMA_Start_IT(hi2s->hdmatx, (uint32_t)hi2s->pTxBuffPtr, (uint32_t)&hi2s->Instance->TXDR, hi2s->TxXferSize)) {
		// Update SPI error code
		SET_BIT(hi2s->ErrorCode, HAL_I2S_ERROR_DMA);
		hi2s->State = HAL_I2S_STATE_READY;

		__HAL_UNLOCK(hi2s);
		return HAL_ERROR;
	}

	/* Check if the I2S Rx request is already enabled */
	if (HAL_IS_BIT_CLR(hi2s->Instance->CFG1, SPI_CFG1_RXDMAEN)) {
		// Enable Rx DMA Request
		SET_BIT(hi2s->Instance->CFG1, SPI_CFG1_RXDMAEN);
	}
	/* Check if the I2S Tx request is already enabled */
	if (HAL_IS_BIT_CLR(hi2s->Instance->CFG1, SPI_CFG1_TXDMAEN)) {
		/* Enable Tx DMA Request */
		SET_BIT(hi2s->Instance->CFG1, SPI_CFG1_TXDMAEN);
	}

	/* Check if the I2S is already enabled */
	if (HAL_IS_BIT_CLR(hi2s->Instance->CR1, SPI_CR1_SPE)) {
		/* Enable I2S peripheral */
		__HAL_I2S_ENABLE(hi2s);
	}

	/* Start the transfer */
	SET_BIT(hi2s->Instance->CR1, SPI_CR1_CSTART);

	__HAL_UNLOCK(hi2s);
#endif

#ifdef STM32F407xx
	if ((rxData == NULL) || (txData == NULL) || (rxSize == 0UL) || (txSize == 0UL)) {
		return HAL_ERROR;
	}

	/* Process Locked */
	__HAL_LOCK(hi2s);

	if (hi2s->State != HAL_I2S_STATE_READY) {
		__HAL_UNLOCK(hi2s);
		return HAL_BUSY;
	}

	/* Set state and reset error code */
	hi2s->pTxBuffPtr = txData;
	hi2s->pRxBuffPtr = rxData;

	hi2s->State = HAL_I2S_STATE_BUSY_TX_RX;
	hi2s->ErrorCode = HAL_I2S_ERROR_NONE;

	hi2s->TxXferSize = txSize;
	hi2s->TxXferCount = txSize;
	hi2s->RxXferSize = (rxSize << 1U);
	hi2s->RxXferCount = (rxSize << 1U);

	hi2s->hdmarx->XferHalfCpltCallback = I2S_DMARxHalfCplt;
	hi2s->hdmarx->XferCpltCallback = I2S_DMARxCplt;
	hi2s->hdmarx->XferErrorCallback = I2S_DMAError;
	hi2s->hdmatx->XferHalfCpltCallback = I2S_DMATxHalfCplt;
	hi2s->hdmatx->XferCpltCallback = I2S_DMATxCplt;
	hi2s->hdmatx->XferErrorCallback = I2S_DMAError;

	/* Enable the Rx DMA Stream/Channel */
	if (HAL_OK != HAL_DMA_Start_IT(hi2s->hdmarx, (uint32_t)&I2SxEXT(hi2s->Instance)->DR, (uint32_t)hi2s->pRxBuffPtr, hi2s->RxXferSize)) {
		// Update SPI error code
		SET_BIT(hi2s->ErrorCode, HAL_I2S_ERROR_DMA);
		hi2s->State = HAL_I2S_STATE_READY;

		__HAL_UNLOCK(hi2s);
		return HAL_ERROR;
	}
	if (HAL_OK != HAL_DMA_Start_IT(hi2s->hdmatx, (uint32_t)hi2s->pTxBuffPtr, (uint32_t)&hi2s->Instance->DR, hi2s->TxXferSize)) {
		// Update SPI error code
		SET_BIT(hi2s->ErrorCode, HAL_I2S_ERROR_DMA);
		hi2s->State = HAL_I2S_STATE_READY;

		__HAL_UNLOCK(hi2s);
		return HAL_ERROR;
	}

	/* Enable Rx DMA Request */
	SET_BIT(I2SxEXT(hi2s->Instance)->CR2, SPI_CR2_RXDMAEN);

	/* Enable Tx DMA Request */
	SET_BIT(hi2s->Instance->CR2, SPI_CR2_TXDMAEN);

	/* Check if the I2S is already enabled */
	if ((hi2s->Instance->I2SCFGR & SPI_I2SCFGR_I2SE) != SPI_I2SCFGR_I2SE) {
		/* Enable I2Sext(receiver) before enabling I2Sx peripheral */
		__HAL_I2SEXT_ENABLE(hi2s);

		/* Enable I2S peripheral after the I2Sext */
		__HAL_I2S_ENABLE(hi2s);
	}

	__HAL_UNLOCK(hi2s);
#endif

	return HAL_OK;
}
