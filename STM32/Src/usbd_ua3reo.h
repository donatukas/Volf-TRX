#ifndef __USB_CDC_H
#define __USB_CDC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "audio_processor.h"
#include "usbd_ioreq.h"
#include "usbd_msc_bot.h"
#include "usbd_msc_data.h"
#include "usbd_msc_scsi.h"

#define IN_EP_DIR 0x80 // Adds a direction bit

#if !HRDW_HAS_USB_IQ && !HRDW_HAS_SD && (!HRDW_HAS_USB_CAT || !HRDW_HAS_USB_DEBUG)
#define SHORT_USB_DESCRIPTOR true
#endif

#if !SHORT_USB_DESCRIPTOR
#define DEBUG_INTERFACE_IDX 0x0 // Index of DEBUG interface
#define DEBUG_EP_IDX 0x01
#define DEBUG_CMD_IDX 0x06
#define DEBUG_OUT_EP DEBUG_EP_IDX
#define DEBUG_CMD_EP (DEBUG_CMD_IDX | IN_EP_DIR)
#define DEBUG_IN_EP (DEBUG_EP_IDX | IN_EP_DIR)

#define CAT_INTERFACE_IDX 0x2 // Index of CAT interface
#define CAT_EP_IDX 0x02
#define CAT_CMD_IDX 0x07
#define CAT_OUT_EP CAT_EP_IDX
#define CAT_CMD_EP (CAT_CMD_IDX | IN_EP_DIR)
#define CAT_IN_EP (CAT_EP_IDX | IN_EP_DIR)

#define AUDIO_INTERFACE_IDX 0x4 // Index of AUDIO interface
#define AUDIO_EP_IDX 0x03
#define AUDIO_OUT_EP AUDIO_EP_IDX
#define AUDIO_IN_EP (AUDIO_EP_IDX | IN_EP_DIR)

#if HRDW_HAS_USB_IQ
#define IQ_INTERFACE_IDX 0x7 // Index of IQ interface
#define IQ_EP_IDX 0x04
#define IQ_IN_EP (IQ_EP_IDX | IN_EP_DIR)
#endif

#if HRDW_HAS_SD
#define STORAGE_INTERFACE_IDX 0x9 // Index of STORAGE interface
#define STORAGE_EP_IDX 0x05
#define MSC_EPIN_ADDR (STORAGE_EP_IDX | IN_EP_DIR)
#define MSC_EPOUT_ADDR STORAGE_EP_IDX
#define MSC_MEDIA_PACKET 512U
#define MSC_MAX_FS_PACKET 0x40U
#endif
#else // SHORT_USB_DESCRIPTOR
#if HRDW_HAS_USB_DEBUG
#define DEBUG_INTERFACE_IDX 0x0 // Index of DEBUG interface
#define DEBUG_EP_IDX 0x01
#define DEBUG_CMD_IDX 0x02
#define DEBUG_OUT_EP DEBUG_EP_IDX
#define DEBUG_CMD_EP (DEBUG_CMD_IDX | IN_EP_DIR)
#define DEBUG_IN_EP (DEBUG_EP_IDX | IN_EP_DIR)
#endif

#if HRDW_HAS_USB_CAT
#define CAT_INTERFACE_IDX 0x0 // Index of CAT interface
#define CAT_EP_IDX 0x01
#define CAT_CMD_IDX 0x02
#define CAT_OUT_EP CAT_EP_IDX
#define CAT_CMD_EP (CAT_CMD_IDX | IN_EP_DIR)
#define CAT_IN_EP (CAT_EP_IDX | IN_EP_DIR)
#endif

#define AUDIO_INTERFACE_IDX 0x2 // Index of AUDIO interface
#define AUDIO_EP_IDX 0x03
#define AUDIO_OUT_EP AUDIO_EP_IDX
#define AUDIO_IN_EP (AUDIO_EP_IDX | IN_EP_DIR)
#endif

#ifndef CDC_HS_BINTERVAL
#define CDC_HS_BINTERVAL 0x10U
#endif /* CDC_HS_BINTERVAL */

#ifndef CDC_FS_BINTERVAL
#define CDC_FS_BINTERVAL 0x10U
#endif /* CDC_FS_BINTERVAL */

/* CDC Endpoints parameters: you can fine tune these values depending on the needed baudrates and performance. */
#define CDC_DATA_FS_MAX_PACKET_SIZE 16U /* Endpoint IN & OUT Packet size */
#define CDC_CMD_PACKET_SIZE 16U         /* Control Endpoint Packet size */

#if SHORT_USB_DESCRIPTOR
#define USB_CDC_CONFIG_DESC_SIZ 248U
#else
#define USB_CDC_CONFIG_DESC_SIZ 436U
#endif

#define CDC_DATA_FS_IN_PACKET_SIZE CDC_DATA_FS_MAX_PACKET_SIZE
#define CDC_DATA_FS_OUT_PACKET_SIZE CDC_DATA_FS_MAX_PACKET_SIZE

#define CDC_REQ_MAX_DATA_SIZE 0x7U
/*---------------------------------------------------------------------*/
/*  CDC definitions                                                    */
/*---------------------------------------------------------------------*/
#define CDC_SEND_ENCAPSULATED_COMMAND 0x00U
#define CDC_GET_ENCAPSULATED_RESPONSE 0x01U
#define CDC_SET_COMM_FEATURE 0x02U
#define CDC_GET_COMM_FEATURE 0x03U
#define CDC_CLEAR_COMM_FEATURE 0x04U
#define CDC_SET_LINE_CODING 0x20U
#define CDC_GET_LINE_CODING 0x21U
#define CDC_SET_CONTROL_LINE_STATE 0x22U
#define CDC_SEND_BREAK 0x23U

// AUDIO
#define USBD_AUDIO_FREQ 48000U

#define BYTES_IN_SAMPLE_AUDIO_OUT_PACKET (HRDW_USB_AUDIO_BITS / 8) // 16/24 bit

#define AUDIO_OUT_PACKET (BYTES_IN_SAMPLE_AUDIO_OUT_PACKET * 2 * (USBD_AUDIO_FREQ / 1000))  // 2/3bytes (16/24bit) * 2 channel * 48 packet per second
#define USB_AUDIO_RX_BUFFER_SIZE (AUDIO_BUFFER_SIZE * BYTES_IN_SAMPLE_AUDIO_OUT_PACKET)     // 16/24 bit
#define USB_AUDIO_TX_BUFFER_SIZE (AUDIO_BUFFER_SIZE * BYTES_IN_SAMPLE_AUDIO_OUT_PACKET * 2) // 16/24 bit x2 size

#define AUDIO_REQ_GET_CUR 0x81U
#define AUDIO_REQ_SET_CUR 0x01U
#define AUDIO_OUT_STREAMING_CTRL 0x02U
#define USB_AUDIO_DESC_SIZ 0x09U
#define AUDIO_DESCRIPTOR_TYPE 0x21U

extern volatile uint32_t RX_USB_AUDIO_SAMPLES;
extern volatile uint32_t TX_USB_AUDIO_SAMPLES;
extern volatile uint32_t RX_USB_IQ_SAMPLES;
extern volatile uint32_t TX_USB_IQ_SAMPLES;
extern volatile bool RX_USB_AUDIO_underrun;
extern volatile uint32_t USB_LastActiveTime;

typedef struct {
	uint32_t bitrate;
	uint8_t format;
	uint8_t paritytype;
	uint8_t datatype;
} USBD_CDC_LineCodingTypeDef;

typedef enum {
	AUDIO_CMD_START = 1,
	AUDIO_CMD_PLAY,
	AUDIO_CMD_STOP,
} AUDIO_CMD_TypeDef;

typedef enum {
	AUDIO_OFFSET_NONE = 0,
	AUDIO_OFFSET_HALF,
	AUDIO_OFFSET_FULL,
	AUDIO_OFFSET_UNKNOWN,
} AUDIO_OffsetTypeDef;

typedef struct {
	uint8_t cmd;
	uint8_t data[USB_MAX_EP0_SIZE];
	uint8_t len;
	uint8_t unit;
} USBD_AUDIO_ControlTypeDef;

#if HRDW_HAS_USB_DEBUG
typedef struct _USBD_DEBUG_Itf {
	int8_t (*Init)(void);
	int8_t (*DeInit)(void);
	int8_t (*Control)(uint8_t cmd, uint8_t *pbuf, uint32_t len);
	int8_t (*Receive)(uint8_t *Buf);

} USBD_DEBUG_ItfTypeDef;
#endif

#if HRDW_HAS_USB_CAT
typedef struct _USBD_CAT_Itf {
	int8_t (*Init)(void);
	int8_t (*DeInit)(void);
	int8_t (*Control)(uint8_t cmd, uint8_t *pbuf, uint32_t len);
	int8_t (*Receive)(uint8_t *Buf, uint32_t *Len);

} USBD_CAT_ItfTypeDef;
#endif

typedef struct {
	int8_t (*Init)(void);
	int8_t (*DeInit)(void);
} USBD_AUDIO_ItfTypeDef;

#if HRDW_HAS_USB_IQ
typedef struct {
	int8_t (*Init)(void);
	int8_t (*DeInit)(void);
} USBD_IQ_ItfTypeDef;
#endif

typedef struct {
	uint32_t alt_setting;
	USBD_AUDIO_ControlTypeDef control;
	uint8_t *RxBuffer;
	uint8_t *TxBuffer;
	uint32_t TxBufferIndex;
} USBD_AUDIO_HandleTypeDef;

#if HRDW_HAS_USB_IQ
typedef struct {
	uint32_t alt_setting;
	USBD_AUDIO_ControlTypeDef control;
	uint8_t *RxBuffer;
} USBD_IQ_HandleTypeDef;
#endif

#if HRDW_HAS_USB_DEBUG
typedef struct {
	uint32_t data[CDC_DATA_FS_MAX_PACKET_SIZE / 4U]; /* Force 32bits alignment */
	uint8_t CmdOpCode;
	uint8_t CmdLength;
	uint8_t *RxBuffer;
	uint8_t *TxBuffer;
	uint32_t RxLength;
	uint32_t TxLength;

	__IO uint32_t TxState;
	__IO uint32_t RxState;
} USBD_DEBUG_HandleTypeDef;
#endif

#if HRDW_HAS_USB_CAT
typedef struct {
	uint32_t data[CDC_DATA_FS_MAX_PACKET_SIZE / 4U]; /* Force 32bits alignment */
	uint8_t CmdOpCode;
	uint8_t CmdLength;
	uint8_t *RxBuffer;
	uint8_t *TxBuffer;
	uint32_t RxLength;
	uint32_t TxLength;

	__IO uint32_t TxState;
	__IO uint32_t RxState;
} USBD_CAT_HandleTypeDef;
#endif

#if HRDW_HAS_SD
typedef struct _USBD_STORAGE {
	int8_t (*Init)(uint8_t lun);
	int8_t (*GetCapacity)(uint8_t lun, uint32_t *block_num, uint16_t *block_size);
	int8_t (*IsReady)(uint8_t lun);
	int8_t (*IsWriteProtected)(uint8_t lun);
	int8_t (*Read)(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
	int8_t (*Write)(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
	int8_t (*GetMaxLun)(void);
	int8_t *pInquiry;

} USBD_StorageTypeDef;

typedef struct {
	uint32_t max_lun;
	uint32_t interface;
	uint8_t bot_state;
	uint8_t bot_status;
	uint32_t bot_data_length;
	uint8_t bot_data[MSC_MEDIA_PACKET];
	USBD_MSC_BOT_CBWTypeDef cbw;
	USBD_MSC_BOT_CSWTypeDef csw;

	USBD_SCSI_SenseTypeDef scsi_sense[SENSE_LIST_DEEPTH];
	uint8_t scsi_sense_head;
	uint8_t scsi_sense_tail;
	uint8_t scsi_medium_state;

	uint16_t scsi_blk_size;
	uint32_t scsi_blk_nbr;

	uint32_t scsi_blk_addr;
	uint32_t scsi_blk_len;
} USBD_MSC_BOT_HandleTypeDef;
#endif

extern USBD_ClassTypeDef USBD_UA3REO;
extern USBD_HandleTypeDef hUsbDeviceFS;

#define USBD_UA3REO_CLASS &USBD_UA3REO

#if HRDW_HAS_USB_DEBUG
extern uint8_t USBD_DEBUG_RegisterInterface(USBD_HandleTypeDef *pdev, USBD_DEBUG_ItfTypeDef *fops);
extern uint8_t USBD_DEBUG_SetTxBuffer(USBD_HandleTypeDef *pdev, uint8_t *pbuff, uint16_t length);
extern uint8_t USBD_DEBUG_SetRxBuffer(USBD_HandleTypeDef *pdev, uint8_t *pbuff);
extern uint8_t USBD_DEBUG_ReceivePacket(USBD_HandleTypeDef *pdev);
extern uint8_t USBD_DEBUG_TransmitPacket(USBD_HandleTypeDef *pdev);
#endif

#if HRDW_HAS_USB_CAT
extern uint8_t USBD_CAT_RegisterInterface(USBD_HandleTypeDef *pdev, USBD_CAT_ItfTypeDef *fops);
extern uint8_t USBD_CAT_SetTxBuffer(USBD_HandleTypeDef *pdev, uint8_t *pbuff, uint16_t length);
extern uint8_t USBD_CAT_SetRxBuffer(USBD_HandleTypeDef *pdev, uint8_t *pbuff);
extern uint8_t USBD_CAT_ReceivePacket(USBD_HandleTypeDef *pdev);
extern uint8_t USBD_CAT_TransmitPacket(USBD_HandleTypeDef *pdev);
#endif

extern uint8_t USBD_AUDIO_RegisterInterface(USBD_HandleTypeDef *pdev, USBD_AUDIO_ItfTypeDef *fops);
extern uint8_t USBD_AUDIO_StartTransmit(USBD_HandleTypeDef *pdev);
extern uint8_t USBD_AUDIO_StartReceive(USBD_HandleTypeDef *pdev);

#if HRDW_HAS_USB_IQ
extern uint8_t USBD_IQ_RegisterInterface(USBD_HandleTypeDef *pdev, USBD_IQ_ItfTypeDef *fops);
extern uint8_t USBD_IQ_StartTransmit(USBD_HandleTypeDef *pdev);
#endif

#if HRDW_HAS_SD
uint8_t USBD_MSC_RegisterStorage(USBD_HandleTypeDef *pdev, USBD_StorageTypeDef *fops);
#endif

extern void USBD_Restart(void);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
