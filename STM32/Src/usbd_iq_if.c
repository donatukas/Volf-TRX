#include "usbd_iq_if.h"
#include "codec.h"
#include "functions.h"
#include "trx_manager.h"
#include "usbd_ua3reo.h"

#if HRDW_HAS_USB_IQ
// extern USBD_HandleTypeDef hUsbDeviceFS;

static int8_t IQ_Init_FS(void);
static int8_t IQ_DeInit_FS(void);

SRAM4 uint8_t USB_IQ_rx_buffer_a[USB_AUDIO_RX_BUFFER_SIZE] = {0};
SRAM4 uint8_t USB_IQ_rx_buffer_b[USB_AUDIO_RX_BUFFER_SIZE] = {0};

volatile bool USB_IQ_current_rx_buffer = false; // a-false b-true
volatile bool USB_IQ_need_rx_buffer = false;    // a-false b-true
static bool USB_IQ_Inited = false;

USBD_IQ_ItfTypeDef USBD_IQ_fops_FS = {
    IQ_Init_FS,
    IQ_DeInit_FS,
};

/* Private functions ---------------------------------------------------------*/
/**
 * @brief  Initializes the AUDIO media low layer over USB FS IP
 * @param  AudioFreq: Audio frequency used to play the audio stream.
 * @param  Volume: Initial volume level (from 0 (Mute) to 100 (Max))
 * @param  options: Reserved for future use
 * @retval USBD_OK if all operations are OK else USBD_FAIL
 */

static int8_t IQ_Init_FS(void) {
	USBD_IQ_HandleTypeDef *haudio = (USBD_IQ_HandleTypeDef *)hUsbDeviceFS.pClassDataIQ;
	haudio->RxBuffer = (uint8_t *)&USB_IQ_rx_buffer_a;
	USBD_IQ_StartTransmit(&hUsbDeviceFS);
	USB_IQ_Inited = true;
	return (USBD_OK);
}

/**
 * @brief  De-Initializes the AUDIO media low layer
 * @param  options: Reserved for future use
 * @retval USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t IQ_DeInit_FS(void) {
	/* USER CODE BEGIN 1 */
	return (USBD_OK);
	/* USER CODE END 1 */
}

#endif
