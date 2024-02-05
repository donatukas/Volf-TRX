#include "usbd_audio_if.h"
#include "codec.h"
#include "functions.h"
#include "trx_manager.h"

static int8_t AUDIO_Init_FS(void);
static int8_t AUDIO_DeInit_FS(void);

IRAM2 uint8_t USB_AUDIO_rx_buffer_a[USB_AUDIO_RX_BUFFER_SIZE] = {0};
IRAM2 uint8_t USB_AUDIO_rx_buffer_b[USB_AUDIO_RX_BUFFER_SIZE] = {0};
IRAM2 uint8_t USB_AUDIO_tx_buffer[USB_AUDIO_TX_BUFFER_SIZE] = {0};

volatile bool USB_AUDIO_current_rx_buffer = false; // a-false b-true
volatile bool USB_AUDIO_need_rx_buffer = false;    // a-false b-true
static bool USB_AUDIO_Inited = false;

USBD_AUDIO_ItfTypeDef USBD_AUDIO_fops_FS = {
    AUDIO_Init_FS,
    AUDIO_DeInit_FS,
};

/* Private functions ---------------------------------------------------------*/
/**
 * @brief  Initializes the AUDIO media low layer over USB FS IP
 * @param  AudioFreq: Audio frequency used to play the audio stream.
 * @param  Volume: Initial volume level (from 0 (Mute) to 100 (Max))
 * @param  options: Reserved for future use
 * @retval USBD_OK if all operations are OK else USBD_FAIL
 */

static int8_t AUDIO_Init_FS(void) {
	dma_memset(USB_AUDIO_rx_buffer_a, 0x00, sizeof(USB_AUDIO_rx_buffer_a));
	dma_memset(USB_AUDIO_rx_buffer_b, 0x00, sizeof(USB_AUDIO_rx_buffer_b));
	dma_memset(USB_AUDIO_tx_buffer, 0x00, sizeof(USB_AUDIO_tx_buffer));

	USBD_AUDIO_HandleTypeDef *haudio = (USBD_AUDIO_HandleTypeDef *)hUsbDeviceFS.pClassDataAUDIO;
	haudio->RxBuffer = (uint8_t *)&USB_AUDIO_rx_buffer_a;
	haudio->TxBuffer = (uint8_t *)&USB_AUDIO_tx_buffer;
	haudio->TxBufferIndex = 0;

	USBD_AUDIO_StartTransmit(&hUsbDeviceFS);
	USBD_AUDIO_StartReceive(&hUsbDeviceFS);
	USB_AUDIO_Inited = true;

	return (USBD_OK);
}

uint32_t USB_AUDIO_GetTXBufferIndex_FS(void) {
	if (!USB_AUDIO_Inited) {
		return 0;
	}
	const USBD_AUDIO_HandleTypeDef *haudio = (USBD_AUDIO_HandleTypeDef *)hUsbDeviceFS.pClassDataAUDIO;
	return haudio->TxBufferIndex;
}

/**
 * @brief  De-Initializes the AUDIO media low layer
 * @param  options: Reserved for future use
 * @retval USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t AUDIO_DeInit_FS(void) {
	/* USER CODE BEGIN 1 */
	return (USBD_OK);
	/* USER CODE END 1 */
}
