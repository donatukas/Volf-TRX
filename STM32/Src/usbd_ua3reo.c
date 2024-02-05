#include "usbd_ua3reo.h"
#include "codec.h"
#include "functions.h"
#include "hardware.h"
#include "lcd_driver.h"
#include "usb_device.h"
#include "usbd_audio_if.h"
#include "usbd_ctlreq.h"
#include "usbd_iq_if.h"

static uint8_t USBD_UA3REO_Init(USBD_HandleTypeDef *pdev);
static uint8_t USBD_UA3REO_DeInit(USBD_HandleTypeDef *pdev);
static uint8_t USBD_UA3REO_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static uint8_t USBD_UA3REO_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_UA3REO_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_UA3REO_EP0_RxReady(USBD_HandleTypeDef *pdev);
static uint8_t USBD_UA3REO_EP0_TxReady(void);
static uint8_t USBD_UA3REO_SOF(USBD_HandleTypeDef *pdev);
static uint8_t USBD_UA3REO_IsoINIncomplete(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_UA3REO_IsoOutIncomplete(USBD_HandleTypeDef *pdev, uint8_t epnum);
static void AUDIO_REQ_GetCurrent(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static void AUDIO_REQ_SetCurrent(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static void IQ_REQ_GetCurrent(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static void IQ_REQ_SetCurrent(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static const uint8_t *USBD_UA3REO_GetFSCfgDesc(uint16_t *length);
static const uint8_t *USBD_UA3REO_GetDeviceQualifierDescriptor(uint16_t *length);

static uint16_t rx_audio_buffer_head = 0;
static uint16_t rx_audio_buffer_step = 0;
static uint16_t rx_iq_buffer_head = 0;
static uint16_t rx_iq_buffer_step = 0;

volatile bool RX_USB_AUDIO_underrun = false;
volatile uint32_t RX_USB_AUDIO_SAMPLES = 0;
volatile uint32_t TX_USB_AUDIO_SAMPLES = 0;
volatile uint32_t RX_USB_IQ_SAMPLES = 0;
volatile uint32_t TX_USB_IQ_SAMPLES = 0;
volatile uint32_t USB_LastActiveTime = 0;

/* USB Standard Device Descriptor */
__ALIGN_BEGIN static const uint8_t USBD_UA3REO_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END = {
    USB_LEN_DEV_QUALIFIER_DESC, USB_DESC_TYPE_DEVICE_QUALIFIER, 0x00, 0x02, 0x00, 0x00, 0x00, 0x40, 0x01, 0x00,
};

/* UA3REO interface class callbacks structure */
USBD_ClassTypeDef USBD_UA3REO = {
    USBD_UA3REO_Init,
    USBD_UA3REO_DeInit,
    USBD_UA3REO_Setup,
    USBD_UA3REO_EP0_TxReady,
    USBD_UA3REO_EP0_RxReady,
    USBD_UA3REO_DataIn,
    USBD_UA3REO_DataOut,
    USBD_UA3REO_SOF,
    USBD_UA3REO_IsoINIncomplete,
    USBD_UA3REO_IsoOutIncomplete,
    NULL,
    USBD_UA3REO_GetFSCfgDesc,
    NULL,
    USBD_UA3REO_GetDeviceQualifierDescriptor,
};

/* USB CDC device Configuration Descriptor */
__ALIGN_BEGIN static const uint8_t USBD_UA3REO_CfgFSDesc[USB_CDC_CONFIG_DESC_SIZ] __ALIGN_END = {
    /*Configuration Descriptor*/
    0x09,                             /* bLength: Configuration Descriptor size */
    USB_DESC_TYPE_CONFIGURATION,      /* bDescriptorType: Configuration */
    (USB_CDC_CONFIG_DESC_SIZ & 0xFF), /* wTotalLength:no of returned bytes */
    USB_CDC_CONFIG_DESC_SIZ >> 8,
#ifdef SHORT_USB_DESCRIPTOR
    0x05, /* bNumInterfaces: count interface */
#else
    0x0A, /* bNumInterfaces: count interface */
#endif
    0x01, /* bConfigurationValue: Configuration value */
    0x00, /* iConfiguration: Index of string descriptor describing the configuration */
    0xC0, /* bmAttributes: self powered */
    0x32, /* MaxPower 100 mA */

#if HRDW_HAS_USB_DEBUG
    //---------------------------------------------------------------------------
    // DEBUG/KEY PORT
    // Interface Association Descriptor:
    //------------------------------
    0x08,                    // bLength
    0x0B,                    // bDescriptorType
    DEBUG_INTERFACE_IDX,     // bFirstInterface
    0x02,                    // bInterfaceCount
    0x02,                    // bFunctionClass      (Communication Device Class)
    0x02,                    // bFunctionSubClass   (Abstract Control Model - ACM)
    0x01,                    // bFunctionProtocol   (ITU-T V.250)
    USBD_IDX_INTERFACE1_STR, // iFunction   ""

    /*Interface Descriptor */
    0x09,                    /* bLength: Interface Descriptor size */
    USB_DESC_TYPE_INTERFACE, /* bDescriptorType: Interface */
    /* Interface descriptor type */
    DEBUG_INTERFACE_IDX,     /* bInterfaceNumber: Number of Interface */
    0x00,                    /* bAlternateSetting: Alternate setting */
    0x01,                    /* bNumEndpoints: One endpoints used */
    0x02,                    /* bInterfaceClass: Communication Interface Class */
    0x02,                    /* bInterfaceSubClass: Abstract Control Model */
    0x01,                    /* bInterfaceProtocol: Common AT commands */
    USBD_IDX_INTERFACE1_STR, /* iInterface: */

    /*Header Functional Descriptor*/
    0x05, /* bLength: Endpoint Descriptor size */
    0x24, /* bDescriptorType: CS_INTERFACE */
    0x00, /* bDescriptorSubtype: Header Func Desc */
    0x10, /* bcdCDC: spec release number */
    0x01,

    /*Call Management Functional Descriptor*/
    0x05,                      /* bFunctionLength */
    0x24,                      /* bDescriptorType: CS_INTERFACE */
    0x01,                      /* bDescriptorSubtype: Call Management Func Desc */
    0x00,                      /* bmCapabilities: D0+D1 */
    (DEBUG_INTERFACE_IDX + 1), /* bDataInterface: 1 */

    /*ACM Functional Descriptor*/
    0x04, /* bFunctionLength */
    0x24, /* bDescriptorType: CS_INTERFACE */
    0x02, /* bDescriptorSubtype: Abstract Control Management desc */
    0x02, /* bmCapabilities */

    /*Union Functional Descriptor*/
    0x05,                      /* bFunctionLength */
    0x24,                      /* bDescriptorType: CS_INTERFACE */
    0x06,                      /* bDescriptorSubtype: Union func desc */
    DEBUG_INTERFACE_IDX,       /* bMasterInterface: Communication class interface */
    (DEBUG_INTERFACE_IDX + 1), /* bSlaveInterface0: Data Class Interface */

    //---------------------------------------------------------------------------

    /*Endpoint CMD Descriptor*/
    0x07,                              /* bLength: Endpoint Descriptor size */
    USB_DESC_TYPE_ENDPOINT,            /* bDescriptorType: Endpoint */
    DEBUG_CMD_EP,                      /* bEndpointAddress */
    0x03,                              /* bmAttributes: Interrupt */
    LOBYTE(CDC_CMD_PACKET_SIZE),       /* wMaxPacketSize: */
    HIBYTE(CDC_CMD_PACKET_SIZE), 0xFF, /* bInterval: */

    /*Data class interface descriptor*/
    0x09,                      /* bLength: Endpoint Descriptor size */
    USB_DESC_TYPE_INTERFACE,   /* bDescriptorType: */
    (DEBUG_INTERFACE_IDX + 1), /* bInterfaceNumber: Number of Interface */
    0x00,                      /* bAlternateSetting: Alternate setting */
    0x02,                      /* bNumEndpoints: Two endpoints used */
    0x0A,                      /* bInterfaceClass: CDC */
    0x00,                      /* bInterfaceSubClass: */
    0x00,                      /* bInterfaceProtocol: */
    USBD_IDX_INTERFACE1_STR,   /* iInterface: */

    /*Endpoint OUT Descriptor*/
    0x07,                                      /* bLength: Endpoint Descriptor size */
    USB_DESC_TYPE_ENDPOINT,                    /* bDescriptorType: Endpoint */
    DEBUG_OUT_EP,                              /* bEndpointAddress */
    0x02,                                      /* bmAttributes: Bulk */
    LOBYTE(CDC_DATA_FS_MAX_PACKET_SIZE),       /* wMaxPacketSize: */
    HIBYTE(CDC_DATA_FS_MAX_PACKET_SIZE), 0x00, /* bInterval: ignore for Bulk transfer */

    /*Endpoint IN Descriptor*/
    0x07,                                      /* bLength: Endpoint Descriptor size */
    USB_DESC_TYPE_ENDPOINT,                    /* bDescriptorType: Endpoint */
    DEBUG_IN_EP,                               /* bEndpointAddress */
    0x02,                                      /* bmAttributes: Bulk */
    LOBYTE(CDC_DATA_FS_MAX_PACKET_SIZE),       /* wMaxPacketSize: */
    HIBYTE(CDC_DATA_FS_MAX_PACKET_SIZE), 0x00, /* bInterval: ignore for Bulk transfer */
#endif
//---------------------------------------------------------------------------
#if HRDW_HAS_USB_CAT
    // CAT PORT
    // Interface Association Descriptor:
    //------------------------------
    0x08,                    // bLength
    0x0B,                    // bDescriptorType
    CAT_INTERFACE_IDX,       // bFirstInterface
    0x02,                    // bInterfaceCount
    0x02,                    // bFunctionClass      (Communication Device Class)
    0x02,                    // bFunctionSubClass   (Abstract Control Model - ACM)
    0x01,                    // bFunctionProtocol   (ITU-T V.250)
    USBD_IDX_INTERFACE2_STR, // iFunction   ""

    /*Interface Descriptor */
    0x09,                    /* bLength: Interface Descriptor size */
    USB_DESC_TYPE_INTERFACE, /* bDescriptorType: Interface */
    /* Interface descriptor type */
    CAT_INTERFACE_IDX,       /* bInterfaceNumber: Number of Interface */
    0x00,                    /* bAlternateSetting: Alternate setting */
    0x01,                    /* bNumEndpoints: One endpoints used */
    0x02,                    /* bInterfaceClass: Communication Interface Class */
    0x02,                    /* bInterfaceSubClass: Abstract Control Model */
    0x01,                    /* bInterfaceProtocol: Common AT commands */
    USBD_IDX_INTERFACE2_STR, /* iInterface: */

    /*Header Functional Descriptor*/
    0x05, /* bLength: Endpoint Descriptor size */
    0x24, /* bDescriptorType: CS_INTERFACE */
    0x00, /* bDescriptorSubtype: Header Func Desc */
    0x10, /* bcdCDC: spec release number */
    0x01,

    /*Call Management Functional Descriptor*/
    0x05,                    /* bFunctionLength */
    0x24,                    /* bDescriptorType: CS_INTERFACE */
    0x01,                    /* bDescriptorSubtype: Call Management Func Desc */
    0x00,                    /* bmCapabilities: D0+D1 */
    (CAT_INTERFACE_IDX + 1), /* bDataInterface: 1 */

    /*ACM Functional Descriptor*/
    0x04, /* bFunctionLength */
    0x24, /* bDescriptorType: CS_INTERFACE */
    0x02, /* bDescriptorSubtype: Abstract Control Management desc */
    0x02, /* bmCapabilities */

    /*Union Functional Descriptor*/
    0x05,                    /* bFunctionLength */
    0x24,                    /* bDescriptorType: CS_INTERFACE */
    0x06,                    /* bDescriptorSubtype: Union func desc */
    CAT_INTERFACE_IDX,       /* bMasterInterface: Communication class interface */
    (CAT_INTERFACE_IDX + 1), /* bSlaveInterface0: Data Class Interface */

    //---------------------------------------------------------------------------

    /*Endpoint CMD Descriptor*/
    0x07,                              /* bLength: Endpoint Descriptor size */
    USB_DESC_TYPE_ENDPOINT,            /* bDescriptorType: Endpoint */
    CAT_CMD_EP,                        /* bEndpointAddress */
    0x03,                              /* bmAttributes: Interrupt */
    LOBYTE(CDC_CMD_PACKET_SIZE),       /* wMaxPacketSize: */
    HIBYTE(CDC_CMD_PACKET_SIZE), 0xFF, /* bInterval: */

    /*Data class interface descriptor*/
    0x09,                    /* bLength: Endpoint Descriptor size */
    USB_DESC_TYPE_INTERFACE, /* bDescriptorType: */
    (CAT_INTERFACE_IDX + 1), /* bInterfaceNumber: Number of Interface */
    0x00,                    /* bAlternateSetting: Alternate setting */
    0x02,                    /* bNumEndpoints: Two endpoints used */
    0x0A,                    /* bInterfaceClass: CDC */
    0x00,                    /* bInterfaceSubClass: */
    0x00,                    /* bInterfaceProtocol: */
    USBD_IDX_INTERFACE2_STR, /* iInterface: */

    /*Endpoint OUT Descriptor*/
    0x07,                                      /* bLength: Endpoint Descriptor size */
    USB_DESC_TYPE_ENDPOINT,                    /* bDescriptorType: Endpoint */
    CAT_OUT_EP,                                /* bEndpointAddress */
    0x02,                                      /* bmAttributes: Bulk */
    LOBYTE(CDC_DATA_FS_MAX_PACKET_SIZE),       /* wMaxPacketSize: */
    HIBYTE(CDC_DATA_FS_MAX_PACKET_SIZE), 0x00, /* bInterval: ignore for Bulk transfer */

    /*Endpoint IN Descriptor*/
    0x07,                                      /* bLength: Endpoint Descriptor size */
    USB_DESC_TYPE_ENDPOINT,                    /* bDescriptorType: Endpoint */
    CAT_IN_EP,                                 /* bEndpointAddress */
    0x02,                                      /* bmAttributes: Bulk */
    LOBYTE(CDC_DATA_FS_MAX_PACKET_SIZE),       /* wMaxPacketSize: */
    HIBYTE(CDC_DATA_FS_MAX_PACKET_SIZE), 0x00, /* bInterval: ignore for Bulk transfer */

//---------------------------------------------------------------------------
#endif
    // AUDIO RX PORT
    // Interface Association Descriptor:
    //------------------------------
    0x08,                    //     bLength
    0x0B,                    //    bDescriptorType
    AUDIO_INTERFACE_IDX,     //     bFirstInterface
    0x03,                    //     bInterfaceCount
    0x01,                    //     bFunctionClass      (Audio Device Class)
    0x00,                    //     bFunctionSubClass
    0x00,                    //     bFunctionProtocol
    USBD_IDX_INTERFACE3_STR, //     iFunction   "UA3REO Transceiver AUDIO RX"

    // Interface Descriptor (Control):
    //------------------------------
    0x09,                    //     bLength
    0x04,                    //     bDescriptorType
    AUDIO_INTERFACE_IDX,     //     bInterfaceNumber
    0x00,                    //     bAlternateSetting
    0x00,                    //     bNumEndPoints
    0x01,                    //     bInterfaceClass      (Audio Device Class)
    0x01,                    //     bInterfaceSubClass   (Audio Control Interface)
    0x00,                    //     bInterfaceProtocol
    USBD_IDX_INTERFACE3_STR, //     iInterface   ""

    // AC Interface Header Descriptor (Control):
    //------------------------------
    0x0A,                      //     bLength
    0x24,                      //     bDescriptorType
    0x01,                      //     bDescriptorSubtype
    0x00,                      //
    0x01,                      // bcdADC
    0x34,                      // wTotalLength   (52 bytes)
    0x00,                      // wTotalLength
    0x02,                      //     bInCollection
    (AUDIO_INTERFACE_IDX + 1), //     baInterfaceNr(1)
    (AUDIO_INTERFACE_IDX + 2), //     baInterfaceNr(2)

    // AC Input Terminal Descriptor (Microphone):
    //------------------------------
    0x0C, //    bLength
    0x24, //     bDescriptorType
    0x02, //     bDescriptorSubtype
    0x1E, //    bTerminalID
    0x10, // wTerminalType   (Radio Receiver)
    0x07, // wTerminalType   (Radio Receiver)
    0x00, //     bAssocTerminal
    0x02, //     bNrChannels   (2 channels)
    0x03, // wChannelConfig
    0x00, // wChannelConfig
    0x00, //    iChannelNames
    0x00, //     iTerminal

    // AC Output Terminal Descriptor (Microphone):
    //------------------------------
    0x09, //     bLength
    0x24, //     bDescriptorType
    0x03, //     bDescriptorSubtype
    0x23, //     bTerminalID
    0x01, // wTerminalType   (USB Streaming)
    0x01, // wTerminalType   (USB Streaming)
    0x00, //     bAssocTerminal
    0x1E, //    bSourceID
    0x00, //     iTerminal   "wwww 1"

    // AC Input Terminal Descriptor (Speaker):
    //------------------------------
    0x0C, //    bLength
    0x24, //     bDescriptorType
    0x02, //     bDescriptorSubtype
    0x0C, //    bTerminalID
    0x01, // wTerminalType   (USB Streaming)
    0x01, // wTerminalType   (USB Streaming)
    0x00, //     bAssocTerminal
    0x02, //     bNrChannels   (2 channels)
    0x03, // wChannelConfig
    0x00, // wChannelConfig
    0x1B, //    iChannelNames
    0x00, //    iTerminal   "eeee 3"

    // AC Output Terminal Descriptor (Speaker):
    //------------------------------
    0x09, //     bLength
    0x24, //     bDescriptorType
    0x03, //     bDescriptorSubtype
    0x11, //     bTerminalID
    0x11, // wTerminalType   (Radio Transmitter)
    0x07, // wTerminalType   (Radio Transmitter)
    0x00, //     bAssocTerminal
    0x0C, //    bSourceID
    0x00, //     iTerminal

    // Interface Descriptor (Microphone):
    //------------------------------
    0x09,                      //     bLength
    0x04,                      //     bDescriptorType
    (AUDIO_INTERFACE_IDX + 1), //     bInterfaceNumber
    0x00,                      //     bAlternateSetting
    0x00,                      //     bNumEndPoints
    0x01,                      //     bInterfaceClass      (Audio Device Class)
    0x02,                      //     bInterfaceSubClass   (Audio Streaming Interface)
    0x00,                      //     bInterfaceProtocol
    USBD_IDX_INTERFACE3_STR,   //     iInterface   "xxxx 1"

    // Interface Descriptor (Microphone):
    //------------------------------
    0x09,                      //     bLength
    0x04,                      //     bDescriptorType
    (AUDIO_INTERFACE_IDX + 1), //     bInterfaceNumber
    0x01,                      //     bAlternateSetting
    0x01,                      //     bNumEndPoints
    0x01,                      //     bInterfaceClass      (Audio Device Class)
    0x02,                      //     bInterfaceSubClass   (Audio Streaming Interface)
    0x00,                      //     bInterfaceProtocol
    USBD_IDX_INTERFACE3_STR,   //     iInterface   "xxxx 1"

    // AS Interface Descriptor (Microphone):
    //------------------------------
    0x07, //     bLength
    0x24, //     bDescriptorType
    0x01, //     bDescriptorSubtype
    0x23, //     bTerminalLink
    0x01, //     bDelay
    0x01, // wFormatTag   (PCM)
    0x00, // wFormatTag   (PCM)

    // AS Format Type 1 Descriptor (Microphone):
    //------------------------------
    0x0B,                             //    bLength
    0x24,                             //     bDescriptorType
    0x02,                             //     bDescriptorSubtype
    0x01,                             //     bFormatType   (FORMAT_TYPE_1)
    0x02,                             //     bNrChannels   (2 channels)
    BYTES_IN_SAMPLE_AUDIO_OUT_PACKET, //     bSubframeSize
    HRDW_USB_AUDIO_BITS,              //     bBitResolution   (16/24 bits per sample)
    0x01,                             //     bSamFreqType   (Discrete sampling frequencies)
    0x80,                             //         tSamFreq(1)   (48000 Hz)
    0xBB,                             //         tSamFreq(1)   (48000 Hz)
    0x00,                             //         tSamFreq(1)   (48000 Hz)

    // Endpoint Descriptor (Audio/MIDI 1.0) (Microphone):
    //------------------------------
    0x09,                     //     bLength
    0x05,                     //     bDescriptorType
    AUDIO_IN_EP,              //     bEndpointAddress  (IN endpoint 1)
    0x0D,                     //     bmAttributes      (Transfer: Isochronous / Synch: Synchronous / Usage: Data)
    LOBYTE(AUDIO_OUT_PACKET), /* wMaxPacketSize: */
    HIBYTE(AUDIO_OUT_PACKET),
    0x01, //     bInterval         (1 frames)
    0x00, //     bRefresh
    0x00, //     bSynchAddress

    // AS Isochronous Data Endpoint Descriptor (Microphone):
    //------------------------------
    0x07, //     bLength
    0x25, //     bDescriptorType
    0x01, //     bDescriptorSubtype
    0x00, //     bmAttributes
    0x02, //     bLockDelayUnits   (decoded PCM samples)
    0x00, // wLockDelay
    0x00, // wLockDelay

    // Interface Descriptor (Speaker):
    //------------------------------
    0x09,                      //     bLength
    0x04,                      //     bDescriptorType
    (AUDIO_INTERFACE_IDX + 2), //     bInterfaceNumber
    0x00,                      //     bAlternateSetting
    0x00,                      //     bNumEndPoints
    0x01,                      //     bInterfaceClass      (Audio Device Class)
    0x02,                      //     bInterfaceSubClass   (Audio Streaming Interface)
    0x00,                      //     bInterfaceProtocol
    USBD_IDX_INTERFACE3_STR,   //     iInterface   "yyyy 3"

    // Interface Descriptor (Speaker):
    //------------------------------
    0x09,                      //     bLength
    0x04,                      //     bDescriptorType
    (AUDIO_INTERFACE_IDX + 2), //     bInterfaceNumber
    0x01,                      //     bAlternateSetting
    0x01,                      //     bNumEndPoints
    0x01,                      //     bInterfaceClass      (Audio Device Class)
    0x02,                      //     bInterfaceSubClass   (Audio Streaming Interface)
    0x00,                      //     bInterfaceProtocol
    USBD_IDX_INTERFACE3_STR,   //    iInterface   "zzzz 3"

    // AS Interface Descriptor (Speaker):
    //------------------------------
    0x07, //     bLength
    0x24, //     bDescriptorType
    0x01, //     bDescriptorSubtype
    0x0C, //    bTerminalLink
    0x01, //     bDelay
    0x01, // wFormatTag   (PCM)
    0x00, // wFormatTag   (PCM)

    // AS Format Type 1 Descriptor (Speaker):
    //------------------------------
    0x0B,                             //    bLength
    0x24,                             //     bDescriptorType
    0x02,                             //     bDescriptorSubtype
    0x01,                             //     bFormatType   (FORMAT_TYPE_1)
    0x02,                             //     bNrChannels   (2 channels)
    BYTES_IN_SAMPLE_AUDIO_OUT_PACKET, //     bSubframeSize
    HRDW_USB_AUDIO_BITS,              //     bBitResolution   (16/24 bits per sample)
    0x01,                             //     bSamFreqType   (Discrete sampling frequencies)
    0x80,                             //         tSamFreq(1)   (48000 Hz)
    0xBB,                             //         tSamFreq(1)   (48000 Hz)
    0x00,                             //         tSamFreq(1)   (48000 Hz)

    // Endpoint Descriptor (Audio/MIDI 1.0) (Speaker):
    //------------------------------
    0x09,                     //     bLength
    0x05,                     //     bDescriptorType
    AUDIO_OUT_EP,             //     bEndpointAddress  (OUT endpoint 1)
    0x05,                     //     bmAttributes      (Transfer: Isochronous / Synch: Asynchronous / Usage: Data)
    LOBYTE(AUDIO_OUT_PACKET), /* wMaxPacketSize: */
    HIBYTE(AUDIO_OUT_PACKET),
    0x01, //     bInterval         (1 frames)
    0x00, //     bRefresh
    0x00, //     bSynchAddress

    // AS Isochronous Data Endpoint Descriptor (Speaker):
    //------------------------------
    0x07, //     bLength
    0x25, //     bDescriptorType
    0x01, //     bDescriptorSubtype
    0x00, //     bmAttributes
    0x00, //     bLockDelayUnits   (undefined)
    0x00, // wLockDelay
    0x00, // wLockDelay

//---------------------------------------------------------------------------

// AUDIO IQ PORT
#if HRDW_HAS_USB_IQ
    // Interface Association Descriptor:
    //------------------------------
    0x08,                    //     bLength
    0x0B,                    //    bDescriptorType
    IQ_INTERFACE_IDX,        //     bFirstInterface
    0x02,                    //     bInterfaceCount
    0x01,                    //     bFunctionClass      (Audio Device Class)
    0x00,                    //     bFunctionSubClass
    0x00,                    //     bFunctionProtocol
    USBD_IDX_INTERFACE4_STR, //     iFunction   "UA3REO Transceiver AUDIO IQ"

    // Interface Descriptor (Control):
    //------------------------------
    0x09,                    //     bLength
    0x04,                    //     bDescriptorType
    IQ_INTERFACE_IDX,        //     bInterfaceNumber
    0x00,                    //     bAlternateSetting
    0x00,                    //     bNumEndPoints
    0x01,                    //     bInterfaceClass      (Audio Device Class)
    0x01,                    //     bInterfaceSubClass   (Audio Control Interface)
    0x00,                    //     bInterfaceProtocol
    USBD_IDX_INTERFACE4_STR, //     iInterface   ""

    // AC Interface Header Descriptor (Control):
    //------------------------------
    0x09,                   //     bLength
    0x24,                   //     bDescriptorType
    0x01,                   //     bDescriptorSubtype
    0x00,                   //
    0x01,                   // bcdADC
    0x1E,                   // wTotalLength   (30 bytes)
    0x00,                   // wTotalLength
    0x01,                   //     bInCollection
    (IQ_INTERFACE_IDX + 1), //     baInterfaceNr(1)

    // AC Input Terminal Descriptor (Microphone):
    //------------------------------
    0x0C, //    bLength
    0x24, //     bDescriptorType
    0x02, //     bDescriptorSubtype
    0x1F, //    bTerminalID
    0x10, // wTerminalType   (Radio Receiver)
    0x07, // wTerminalType   (Radio Receiver)
    0x00, //     bAssocTerminal
    0x02, //     bNrChannels   (2 channels)
    0x03, // wChannelConfig
    0x00, // wChannelConfig
    0x00, //    iChannelNames
    0x00, //     iTerminal

    // AC Output Terminal Descriptor (Microphone):
    //------------------------------
    0x09, //     bLength
    0x24, //     bDescriptorType
    0x03, //     bDescriptorSubtype
    0x24, //     bTerminalID
    0x01, // wTerminalType   (USB Streaming)
    0x01, // wTerminalType   (USB Streaming)
    0x00, //     bAssocTerminal
    0x1F, //    bSourceID
    0x00, //     iTerminal   "wwww 1"

    // Interface Descriptor (Microphone):
    //------------------------------
    0x09,                    //     bLength
    0x04,                    //     bDescriptorType
    (IQ_INTERFACE_IDX + 1),  //     bInterfaceNumber
    0x00,                    //     bAlternateSetting
    0x00,                    //     bNumEndPoints
    0x01,                    //     bInterfaceClass      (Audio Device Class)
    0x02,                    //     bInterfaceSubClass   (Audio Streaming Interface)
    0x00,                    //     bInterfaceProtocol
    USBD_IDX_INTERFACE4_STR, //     iInterface   "xxxx 1"

    // Interface Descriptor (Microphone):
    //------------------------------
    0x09,                    //     bLength
    0x04,                    //     bDescriptorType
    (IQ_INTERFACE_IDX + 1),  //     bInterfaceNumber
    0x01,                    //     bAlternateSetting
    0x01,                    //     bNumEndPoints
    0x01,                    //     bInterfaceClass      (Audio Device Class)
    0x02,                    //     bInterfaceSubClass   (Audio Streaming Interface)
    0x00,                    //     bInterfaceProtocol
    USBD_IDX_INTERFACE4_STR, //     iInterface   "xxxx 1"

    // AS Interface Descriptor (Microphone):
    //------------------------------
    0x07, //     bLength
    0x24, //     bDescriptorType
    0x01, //     bDescriptorSubtype
    0x24, //     bTerminalLink
    0x01, //     bDelay
    0x01, // wFormatTag   (PCM)
    0x00, // wFormatTag   (PCM)

    // AS Format Type 1 Descriptor (Microphone):
    //------------------------------
    0x0B,                             //    bLength
    0x24,                             //     bDescriptorType
    0x02,                             //     bDescriptorSubtype
    0x01,                             //     bFormatType   (FORMAT_TYPE_1)
    0x02,                             //     bNrChannels   (2 channels)
    BYTES_IN_SAMPLE_AUDIO_OUT_PACKET, //     bSubframeSize
    HRDW_USB_AUDIO_BITS,              //     bBitResolution   (16/24 bits per sample)
    0x01,                             //     bSamFreqType   (Discrete sampling frequencies)
    0x80,                             //         tSamFreq(1)   (48000 Hz)
    0xBB,                             //         tSamFreq(1)   (48000 Hz)
    0x00,                             //         tSamFreq(1)   (48000 Hz)

    // Endpoint Descriptor (Audio/MIDI 1.0) (Microphone):
    //------------------------------
    0x09,                     //     bLength
    0x05,                     //     bDescriptorType
    IQ_IN_EP,                 //     bEndpointAddress  (IN endpoint 1)
    0x0D,                     //     bmAttributes      (Transfer: Isochronous / Synch: Synchronous / Usage: Data)
    LOBYTE(AUDIO_OUT_PACKET), /* wMaxPacketSize: */
    HIBYTE(AUDIO_OUT_PACKET),
    0x01, //     bInterval         (1 frames)
    0x00, //     bRefresh
    0x00, //     bSynchAddress

    // AS Isochronous Data Endpoint Descriptor (Microphone):
    //------------------------------
    0x07, //     bLength
    0x25, //     bDescriptorType
    0x01, //     bDescriptorSubtype
    0x00, //     bmAttributes
    0x02, //     bLockDelayUnits   (decoded PCM samples)
    0x00, // wLockDelay
    0x00, // wLockDelay
#endif
//---------------------------------------------------------------------------
#if HRDW_HAS_SD
    // Mass Storage interface
    0x09,                  /* bLength: Interface Descriptor size */
    0x04,                  /* bDescriptorType: */
    STORAGE_INTERFACE_IDX, /* bInterfaceNumber: Number of Interface */
    0x00,                  /* bAlternateSetting: Alternate setting */
    0x02,                  /* bNumEndpoints*/
    0x08,                  /* bInterfaceClass: MSC Class */
    0x06,                  /* bInterfaceSubClass : SCSI transparent*/
    0x50,                  /* nInterfaceProtocol */
    0x05,                  /* iInterface: */

    /********************  Mass Storage Endpoints ********************/
    0x07,                                                       /* Endpoint descriptor length = 7 */
    0x05,                                                       /* Endpoint descriptor type */
    MSC_EPIN_ADDR,                                              /* Endpoint address (IN, address 1) */
    0x02,                                                       /* Bulk endpoint type */
    LOBYTE(MSC_MAX_FS_PACKET), HIBYTE(MSC_MAX_FS_PACKET), 0x00, /* Polling interval in milliseconds */

    0x07,                                                      /* Endpoint descriptor length = 7 */
    0x05,                                                      /* Endpoint descriptor type */
    MSC_EPOUT_ADDR,                                            /* Endpoint address (OUT, address 1) */
    0x02,                                                      /* Bulk endpoint type */
    LOBYTE(MSC_MAX_FS_PACKET), HIBYTE(MSC_MAX_FS_PACKET), 0x00 /* Polling interval in milliseconds */
#endif
};

/**
 * @}
 */

/** @defgroup USBD_CDC_Private_Functions
 * @{
 */

/**
 * @brief  USBD_CDC_Init
 *         Initialize the CDC interface
 * @param  pdev: device instance
 * @param  cfgidx: Configuration index
 * @retval status
 */
static uint8_t USBD_UA3REO_Init(USBD_HandleTypeDef *pdev) {
	USB_LastActiveTime = HAL_GetTick();
	uint8_t ret = 0U;

/* Open EP IN */
#if HRDW_HAS_USB_DEBUG
	USBD_DEBUG_HandleTypeDef *hcdc_debug;
	USBD_LL_OpenEP(pdev, DEBUG_IN_EP, USBD_EP_TYPE_BULK, CDC_DATA_FS_IN_PACKET_SIZE);
	pdev->ep_in[DEBUG_IN_EP & 0xFU].is_used = 1U;
#endif

#if HRDW_HAS_USB_CAT
	USBD_CAT_HandleTypeDef *hcdc_cat;
	USBD_LL_OpenEP(pdev, CAT_IN_EP, USBD_EP_TYPE_BULK, CDC_DATA_FS_IN_PACKET_SIZE);
	pdev->ep_in[CAT_IN_EP & 0xFU].is_used = 1U;
#endif

	USBD_AUDIO_HandleTypeDef *haudio;
	USBD_LL_OpenEP(pdev, AUDIO_IN_EP, USBD_EP_TYPE_ISOC, AUDIO_OUT_PACKET);
	pdev->ep_in[AUDIO_IN_EP & 0xFU].is_used = 1U;

#if HRDW_HAS_USB_IQ
	USBD_IQ_HandleTypeDef *haudioiq;
	USBD_LL_OpenEP(pdev, IQ_IN_EP, USBD_EP_TYPE_ISOC, AUDIO_OUT_PACKET);
	pdev->ep_in[IQ_IN_EP & 0xFU].is_used = 1U;
#endif

#if HRDW_HAS_SD
	USBD_MSC_BOT_HandleTypeDef *hstorage;
	USBD_LL_OpenEP(pdev, MSC_EPIN_ADDR, USBD_EP_TYPE_BULK, MSC_MAX_FS_PACKET);
	pdev->ep_in[MSC_EPIN_ADDR & 0xFU].is_used = 1U;
#endif

/* Open EP OUT */
#if HRDW_HAS_USB_DEBUG
	USBD_LL_OpenEP(pdev, DEBUG_OUT_EP, USBD_EP_TYPE_BULK, CDC_DATA_FS_OUT_PACKET_SIZE);
	pdev->ep_out[DEBUG_OUT_EP & 0xFU].is_used = 1U;

	USBD_LL_OpenEP(pdev, DEBUG_CMD_EP, USBD_EP_TYPE_INTR, CDC_CMD_PACKET_SIZE);
	pdev->ep_in[DEBUG_CMD_EP & 0xFU].is_used = 1U;
#endif

#if HRDW_HAS_USB_CAT
	USBD_LL_OpenEP(pdev, CAT_OUT_EP, USBD_EP_TYPE_BULK, CDC_DATA_FS_OUT_PACKET_SIZE);
	pdev->ep_out[CAT_OUT_EP & 0xFU].is_used = 1U;

	USBD_LL_OpenEP(pdev, CAT_CMD_EP, USBD_EP_TYPE_INTR, CDC_CMD_PACKET_SIZE);
	pdev->ep_in[CAT_CMD_EP & 0xFU].is_used = 1U;
#endif

	USBD_LL_OpenEP(pdev, AUDIO_OUT_EP, USBD_EP_TYPE_ISOC, AUDIO_OUT_PACKET);
	pdev->ep_out[AUDIO_OUT_EP & 0xFU].is_used = 1U;

#if HRDW_HAS_SD
	USBD_LL_OpenEP(pdev, MSC_EPOUT_ADDR, USBD_EP_TYPE_BULK, MSC_MAX_FS_PACKET);
	pdev->ep_out[MSC_EPOUT_ADDR & 0xFU].is_used = 1U;
#endif

#if HRDW_HAS_USB_DEBUG
	static USBD_DEBUG_HandleTypeDef pClassDataDEBUG = {0};
	pdev->pClassDataDEBUG = &pClassDataDEBUG;
	dma_memset(pdev->pClassDataDEBUG, 0, sizeof(USBD_DEBUG_HandleTypeDef));
#endif

#if HRDW_HAS_USB_CAT
	static USBD_CAT_HandleTypeDef pClassDataCAT = {0};
	pdev->pClassDataCAT = &pClassDataCAT;
	dma_memset(pdev->pClassDataCAT, 0, sizeof(USBD_CAT_HandleTypeDef));
#endif

	static USBD_AUDIO_HandleTypeDef pClassDataAUDIO = {0};
	pdev->pClassDataAUDIO = &pClassDataAUDIO;
	dma_memset(pdev->pClassDataAUDIO, 0, sizeof(USBD_AUDIO_HandleTypeDef));

#if HRDW_HAS_USB_IQ
	static USBD_IQ_HandleTypeDef pClassDataIQ = {0};
	pdev->pClassDataIQ = &pClassDataIQ;
	dma_memset(pdev->pClassDataIQ, 0, sizeof(USBD_IQ_HandleTypeDef));
#endif

#if HRDW_HAS_SD
	static USBD_MSC_BOT_HandleTypeDef pClassDataSTORAGE = {0};
	pdev->pClassDataSTORAGE = &pClassDataSTORAGE;
	dma_memset(pdev->pClassDataSTORAGE, 0, sizeof(USBD_MSC_BOT_HandleTypeDef));
#endif

#if HRDW_HAS_USB_DEBUG
	hcdc_debug = (USBD_DEBUG_HandleTypeDef *)pdev->pClassDataDEBUG;

	/* Init  physical Interface components */
	((USBD_DEBUG_ItfTypeDef *)pdev->pUserDataDEBUG)->Init();

	/* Init Xfer states */
	hcdc_debug->TxState = 0U;
	hcdc_debug->RxState = 0U;

	/* Prepare Out endpoint to receive next packet */
	USBD_LL_PrepareReceive(pdev, DEBUG_OUT_EP, hcdc_debug->RxBuffer, CDC_DATA_FS_OUT_PACKET_SIZE);
#endif

#if HRDW_HAS_USB_CAT
	hcdc_cat = (USBD_CAT_HandleTypeDef *)pdev->pClassDataCAT;

	((USBD_CAT_ItfTypeDef *)pdev->pUserDataCAT)->Init();

	hcdc_cat->TxState = 0U;
	hcdc_cat->RxState = 0U;

	USBD_LL_PrepareReceive(pdev, CAT_OUT_EP, hcdc_cat->RxBuffer, CDC_DATA_FS_OUT_PACKET_SIZE);
#endif

	// Initialize the Audio output Hardware layer

	haudio = (USBD_AUDIO_HandleTypeDef *)pdev->pClassDataAUDIO;
	haudio->alt_setting = 0U;

	if (((USBD_AUDIO_ItfTypeDef *)pdev->pUserDataAUDIO)->Init() != 0) {
		return USBD_FAIL;
	}

#if HRDW_HAS_USB_IQ
	haudioiq = (USBD_IQ_HandleTypeDef *)pdev->pClassDataIQ;
	haudioiq->alt_setting = 0U;

	if (((USBD_IQ_ItfTypeDef *)pdev->pUserDataIQ)->Init() != 0) {
		return USBD_FAIL;
	}
#endif

#if HRDW_HAS_SD
	// MSC
	MSC_BOT_Init(pdev);
#endif

	return ret;
}

/**
 * @brief  USBD_CDC_Init
 *         DeInitialize the CDC layer
 * @param  pdev: device instance
 * @param  cfgidx: Configuration index
 * @retval status
 */
static uint8_t USBD_UA3REO_DeInit(USBD_HandleTypeDef *pdev) {
	uint8_t ret = 0U;

/* Close EP IN */
#if HRDW_HAS_USB_DEBUG
	USBD_LL_CloseEP(pdev, DEBUG_IN_EP);
	pdev->ep_in[DEBUG_IN_EP & 0xFU].is_used = 0U;
#endif

#if HRDW_HAS_USB_CAT
	USBD_LL_CloseEP(pdev, CAT_IN_EP);
	pdev->ep_in[CAT_IN_EP & 0xFU].is_used = 0U;
#endif

	USBD_LL_CloseEP(pdev, AUDIO_IN_EP);
	pdev->ep_in[AUDIO_IN_EP & 0xFU].is_used = 0U;

#if HRDW_HAS_USB_IQ
	USBD_LL_CloseEP(pdev, IQ_IN_EP);
	pdev->ep_in[IQ_IN_EP & 0xFU].is_used = 0U;
#endif

#if HRDW_HAS_SD
	USBD_LL_CloseEP(pdev, MSC_EPIN_ADDR);
	pdev->ep_in[MSC_EPIN_ADDR & 0xFU].is_used = 0U;
#endif

/* Close EP OUT */
#if HRDW_HAS_USB_DEBUG
	USBD_LL_CloseEP(pdev, DEBUG_OUT_EP);
	pdev->ep_out[DEBUG_OUT_EP & 0xFU].is_used = 0U;

	USBD_LL_CloseEP(pdev, DEBUG_CMD_EP);
	pdev->ep_in[DEBUG_CMD_EP & 0xFU].is_used = 0U;
#endif

#if HRDW_HAS_USB_CAT
	USBD_LL_CloseEP(pdev, CAT_OUT_EP);
	pdev->ep_out[CAT_OUT_EP & 0xFU].is_used = 0U;

	USBD_LL_CloseEP(pdev, CAT_CMD_EP);
	pdev->ep_in[CAT_CMD_EP & 0xFU].is_used = 0U;
#endif

	USBD_LL_CloseEP(pdev, AUDIO_OUT_EP);
	pdev->ep_out[AUDIO_OUT_EP & 0xFU].is_used = 0U;

#if HRDW_HAS_SD
	USBD_LL_CloseEP(pdev, MSC_EPOUT_ADDR);
	pdev->ep_out[MSC_EPOUT_ADDR & 0xFU].is_used = 0U;
#endif

/* DeInit  physical Interface components */
#if HRDW_HAS_USB_DEBUG
	if (pdev->pClassDataDEBUG != NULL) {
		((USBD_DEBUG_ItfTypeDef *)pdev->pUserDataDEBUG)->DeInit();
		pdev->pClassDataDEBUG = NULL;
	}
#endif

#if HRDW_HAS_USB_CAT
	if (pdev->pClassDataCAT != NULL) {
		((USBD_CAT_ItfTypeDef *)pdev->pUserDataCAT)->DeInit();
		pdev->pClassDataCAT = NULL;
	}
#endif

	if (pdev->pClassDataAUDIO != NULL) {
		((USBD_AUDIO_ItfTypeDef *)pdev->pUserDataAUDIO)->DeInit();
		pdev->pClassDataAUDIO = NULL;
	}

#if HRDW_HAS_USB_IQ
	if (pdev->pClassDataIQ != NULL) {
		((USBD_IQ_ItfTypeDef *)pdev->pUserDataIQ)->DeInit();
		pdev->pClassDataIQ = NULL;
	}
#endif

#if HRDW_HAS_SD
	if (pdev->pClassDataSTORAGE != NULL) {
		MSC_BOT_DeInit(pdev);
		pdev->pClassDataSTORAGE = NULL;
	}
#endif

	return ret;
}

/**
 * @brief  USBD_CDC_Setup
 *         Handle the CDC specific requests
 * @param  pdev: instance
 * @param  req: usb requests
 * @retval status
 */
#if HRDW_HAS_USB_DEBUG
static uint8_t USBD_DEBUG_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req) {
	USBD_DEBUG_HandleTypeDef *hcdc = (USBD_DEBUG_HandleTypeDef *)pdev->pClassDataDEBUG;
	uint16_t len;
	uint8_t ifalt = 0U;
	uint16_t status_info = 0U;
	USBD_StatusTypeDef ret = USBD_OK;

	if (hcdc == NULL) {
		return (uint8_t)USBD_FAIL;
	}

	switch (req->bmRequest & USB_REQ_TYPE_MASK) {
	case USB_REQ_TYPE_CLASS:
		if (req->wLength != 0U) {
			if ((req->bmRequest & 0x80U) != 0U) {
				((USBD_DEBUG_ItfTypeDef *)pdev->pUserDataDEBUG)->Control(req->bRequest, (uint8_t *)hcdc->data, req->wLength);

				len = MIN(CDC_REQ_MAX_DATA_SIZE, req->wLength);
				(void)USBD_CtlSendData(pdev, (uint8_t *)hcdc->data, len);
			} else {
				hcdc->CmdOpCode = req->bRequest;
				hcdc->CmdLength = (uint8_t)MIN(req->wLength, USB_MAX_EP0_SIZE);

				(void)USBD_CtlPrepareRx(pdev, (uint8_t *)hcdc->data, hcdc->CmdLength);
			}
		} else {
			((USBD_DEBUG_ItfTypeDef *)pdev->pUserDataDEBUG)->Control(req->bRequest, (uint8_t *)req, 0U);
		}
		break;

	case USB_REQ_TYPE_STANDARD:
		switch (req->bRequest) {
		case USB_REQ_GET_STATUS:
			if (pdev->dev_state == USBD_STATE_CONFIGURED) {
				(void)USBD_CtlSendData(pdev, (uint8_t *)&status_info, 2U);
			} else {
				USBD_CtlError(pdev, req);
				ret = USBD_FAIL;
			}
			break;

		case USB_REQ_GET_INTERFACE:
			if (pdev->dev_state == USBD_STATE_CONFIGURED) {
				(void)USBD_CtlSendData(pdev, &ifalt, 1U);
			} else {
				USBD_CtlError(pdev, req);
				ret = USBD_FAIL;
			}
			break;

		case USB_REQ_SET_INTERFACE:
			if (pdev->dev_state != USBD_STATE_CONFIGURED) {
				USBD_CtlError(pdev, req);
				ret = USBD_FAIL;
			}
			break;

		case USB_REQ_CLEAR_FEATURE:
			break;

		default:
			USBD_CtlError(pdev, req);
			ret = USBD_FAIL;
			break;
		}
		break;

	default:
		USBD_CtlError(pdev, req);
		ret = USBD_FAIL;
		break;
	}

	return (uint8_t)ret;
}
#endif

#if HRDW_HAS_USB_CAT
static uint8_t USBD_CAT_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req) {
	USBD_CAT_HandleTypeDef *hcdc = (USBD_CAT_HandleTypeDef *)pdev->pClassDataCAT;
	uint16_t len;
	uint8_t ifalt = 0U;
	uint16_t status_info = 0U;
	USBD_StatusTypeDef ret = USBD_OK;

	if (hcdc == NULL) {
		return (uint8_t)USBD_FAIL;
	}

	switch (req->bmRequest & USB_REQ_TYPE_MASK) {
	case USB_REQ_TYPE_CLASS:
		if (req->wLength != 0U) {
			if ((req->bmRequest & 0x80U) != 0U) {
				((USBD_CAT_ItfTypeDef *)pdev->pUserDataCAT)->Control(req->bRequest, (uint8_t *)hcdc->data, req->wLength);

				len = MIN(CDC_REQ_MAX_DATA_SIZE, req->wLength);
				(void)USBD_CtlSendData(pdev, (uint8_t *)hcdc->data, len);
			} else {
				hcdc->CmdOpCode = req->bRequest;
				hcdc->CmdLength = (uint8_t)MIN(req->wLength, USB_MAX_EP0_SIZE);

				(void)USBD_CtlPrepareRx(pdev, (uint8_t *)hcdc->data, hcdc->CmdLength);
			}
		} else {
			((USBD_CAT_ItfTypeDef *)pdev->pUserDataCAT)->Control(req->bRequest, (uint8_t *)req, 0U);
		}
		break;

	case USB_REQ_TYPE_STANDARD:
		switch (req->bRequest) {
		case USB_REQ_GET_STATUS:
			if (pdev->dev_state == USBD_STATE_CONFIGURED) {
				(void)USBD_CtlSendData(pdev, (uint8_t *)&status_info, 2U);
			} else {
				USBD_CtlError(pdev, req);
				ret = USBD_FAIL;
			}
			break;

		case USB_REQ_GET_INTERFACE:
			if (pdev->dev_state == USBD_STATE_CONFIGURED) {
				(void)USBD_CtlSendData(pdev, &ifalt, 1U);
			} else {
				USBD_CtlError(pdev, req);
				ret = USBD_FAIL;
			}
			break;

		case USB_REQ_SET_INTERFACE:
			if (pdev->dev_state != USBD_STATE_CONFIGURED) {
				USBD_CtlError(pdev, req);
				ret = USBD_FAIL;
			}
			break;

		case USB_REQ_CLEAR_FEATURE:
			break;

		default:
			USBD_CtlError(pdev, req);
			ret = USBD_FAIL;
			break;
		}
		break;

	default:
		USBD_CtlError(pdev, req);
		ret = USBD_FAIL;
		break;
	}

	return (uint8_t)ret;
}
#endif

static uint8_t USBD_AUDIO_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req) {
	USBD_AUDIO_HandleTypeDef *haudio;
	uint16_t status_info = 0U;
	uint8_t ret = USBD_OK;

	haudio = (USBD_AUDIO_HandleTypeDef *)pdev->pClassDataAUDIO;

	switch (req->bmRequest & USB_REQ_TYPE_MASK) {
	case USB_REQ_TYPE_CLASS:
		switch (req->bRequest) {
		case AUDIO_REQ_GET_CUR:
			AUDIO_REQ_GetCurrent(pdev, req);
			break;

		case AUDIO_REQ_SET_CUR:
			AUDIO_REQ_SetCurrent(pdev, req);
			break;

		default:
			USBD_CtlError(pdev, req);
			ret = USBD_FAIL;
			break;
		}
		break;

	case USB_REQ_TYPE_STANDARD:
		switch (req->bRequest) {
		case USB_REQ_GET_STATUS:
			if (pdev->dev_state == USBD_STATE_CONFIGURED) {
				USBD_CtlSendData(pdev, (uint8_t *)(void *)&status_info, 2U);
			} else {
				USBD_CtlError(pdev, req);
				ret = USBD_FAIL;
			}
			break;

		case USB_REQ_GET_DESCRIPTOR:
			if ((req->wValue >> 8) == AUDIO_DESCRIPTOR_TYPE) {
				// pbuf = USBD_AUDIO_CfgDesc + 18;
				// len = MIN(USB_AUDIO_DESC_SIZ , req->wLength);
				// USBD_CtlSendData (pdev, pbuf, len);
			}
			break;

		case USB_REQ_GET_INTERFACE:
			if (pdev->dev_state == USBD_STATE_CONFIGURED) {
				USBD_CtlSendData(pdev, (uint8_t *)(void *)&haudio->alt_setting, 1U);
			} else {
				USBD_CtlError(pdev, req);
				ret = USBD_FAIL;
			}
			break;

		case USB_REQ_SET_INTERFACE:
			if (pdev->dev_state == USBD_STATE_CONFIGURED) {
				if ((uint8_t)(req->wValue) <= USBD_MAX_NUM_INTERFACES) {
					haudio->alt_setting = (uint8_t)(req->wValue);
				} else {
					/* Call the error management function (command will be nacked */
					USBD_CtlError(pdev, req);
					ret = USBD_FAIL;
				}
			} else {
				USBD_CtlError(pdev, req);
				ret = USBD_FAIL;
			}
			break;

		default:
			USBD_CtlError(pdev, req);
			ret = USBD_FAIL;
			break;
		}
		break;
	default:
		USBD_CtlError(pdev, req);
		ret = USBD_FAIL;
		break;
	}
	return ret;
}

#if HRDW_HAS_USB_IQ
static uint8_t USBD_IQ_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req) {
	USBD_IQ_HandleTypeDef *haudio;
	uint16_t status_info = 0U;
	uint8_t ret = USBD_OK;

	haudio = (USBD_IQ_HandleTypeDef *)pdev->pClassDataIQ;

	switch (req->bmRequest & USB_REQ_TYPE_MASK) {
	case USB_REQ_TYPE_CLASS:
		switch (req->bRequest) {
		case AUDIO_REQ_GET_CUR:
			IQ_REQ_GetCurrent(pdev, req);
			break;

		case AUDIO_REQ_SET_CUR:
			IQ_REQ_SetCurrent(pdev, req);
			break;

		default:
			USBD_CtlError(pdev, req);
			ret = USBD_FAIL;
			break;
		}
		break;

	case USB_REQ_TYPE_STANDARD:
		switch (req->bRequest) {
		case USB_REQ_GET_STATUS:
			if (pdev->dev_state == USBD_STATE_CONFIGURED) {
				USBD_CtlSendData(pdev, (uint8_t *)(void *)&status_info, 2U);
			} else {
				USBD_CtlError(pdev, req);
				ret = USBD_FAIL;
			}
			break;

		case USB_REQ_GET_DESCRIPTOR:
			if ((req->wValue >> 8) == AUDIO_DESCRIPTOR_TYPE) {
				// pbuf = USBD_AUDIO_CfgDesc + 18;
				// len = MIN(USB_AUDIO_DESC_SIZ , req->wLength);
				// USBD_CtlSendData (pdev, pbuf, len);
			}
			break;

		case USB_REQ_GET_INTERFACE:
			if (pdev->dev_state == USBD_STATE_CONFIGURED) {
				USBD_CtlSendData(pdev, (uint8_t *)(void *)&haudio->alt_setting, 1U);
			} else {
				USBD_CtlError(pdev, req);
				ret = USBD_FAIL;
			}
			break;

		case USB_REQ_SET_INTERFACE:
			if (pdev->dev_state == USBD_STATE_CONFIGURED) {
				if ((uint8_t)(req->wValue) <= USBD_MAX_NUM_INTERFACES) {
					haudio->alt_setting = (uint8_t)(req->wValue);
				} else {
					/* Call the error management function (command will be nacked */
					USBD_CtlError(pdev, req);
					ret = USBD_FAIL;
				}
			} else {
				USBD_CtlError(pdev, req);
				ret = USBD_FAIL;
			}
			break;

		default:
			USBD_CtlError(pdev, req);
			ret = USBD_FAIL;
			break;
		}
		break;
	default:
		USBD_CtlError(pdev, req);
		ret = USBD_FAIL;
		break;
	}
	return ret;
}
#endif

#if HRDW_HAS_SD
uint8_t USBD_MSC_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req) {
	USBD_MSC_BOT_HandleTypeDef *hmsc = (USBD_MSC_BOT_HandleTypeDef *)pdev->pClassDataSTORAGE;
	USBD_StatusTypeDef ret = USBD_OK;
	uint16_t status_info = 0U;

	switch (req->bmRequest & USB_REQ_TYPE_MASK) {
		/* Class request */
	case USB_REQ_TYPE_CLASS:
		switch (req->bRequest) {
		case BOT_GET_MAX_LUN:
			if ((req->wValue == 0U) && (req->wLength == 1U) && ((req->bmRequest & 0x80U) == 0x80U)) {
				hmsc->max_lun = (uint32_t)((USBD_StorageTypeDef *)pdev->pUserDataSTORAGE)->GetMaxLun();
				(void)USBD_CtlSendData(pdev, (uint8_t *)&hmsc->max_lun, 1U);
			} else {
				USBD_CtlError(pdev, req);
				ret = USBD_FAIL;
			}
			break;

		case BOT_RESET:
			if ((req->wValue == 0U) && (req->wLength == 0U) && ((req->bmRequest & 0x80U) != 0x80U)) {
				MSC_BOT_Reset(pdev);
			} else {
				USBD_CtlError(pdev, req);
				ret = USBD_FAIL;
			}
			break;

		default:
			USBD_CtlError(pdev, req);
			ret = USBD_FAIL;
			break;
		}
		break;
		/* Interface & Endpoint request */
	case USB_REQ_TYPE_STANDARD:
		switch (req->bRequest) {
		case USB_REQ_GET_STATUS:
			if (pdev->dev_state == USBD_STATE_CONFIGURED) {
				(void)USBD_CtlSendData(pdev, (uint8_t *)&status_info, 2U);
			} else {
				USBD_CtlError(pdev, req);
				ret = USBD_FAIL;
			}
			break;

		case USB_REQ_GET_INTERFACE:
			if (pdev->dev_state == USBD_STATE_CONFIGURED) {
				(void)USBD_CtlSendData(pdev, (uint8_t *)&hmsc->interface, 1U);
			} else {
				USBD_CtlError(pdev, req);
				ret = USBD_FAIL;
			}
			break;

		case USB_REQ_SET_INTERFACE:
			if (pdev->dev_state == USBD_STATE_CONFIGURED) {
				hmsc->interface = (uint8_t)(req->wValue);
			} else {
				USBD_CtlError(pdev, req);
				ret = USBD_FAIL;
			}
			break;

		case USB_REQ_CLEAR_FEATURE:
			if (pdev->dev_state == USBD_STATE_CONFIGURED) {
				if (req->wValue == USB_FEATURE_EP_HALT) {
					/* Flush the FIFO */
					(void)USBD_LL_FlushEP(pdev, (uint8_t)req->wIndex);

					/* Handle BOT error */
					MSC_BOT_CplClrFeature(pdev, (uint8_t)req->wIndex);
				}
			}
			break;

		default:
			USBD_CtlError(pdev, req);
			ret = USBD_FAIL;
			break;
		}
		break;

	default:
		USBD_CtlError(pdev, req);
		ret = USBD_FAIL;
		break;
	}

	return (uint8_t)ret;
}
#endif

static uint8_t USBD_UA3REO_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req) {
	USB_LastActiveTime = HAL_GetTick();
	uint8_t ret = 0;
	// Route requests to MSC interface or its endpoints to MSC class implementaion
	if (((req->bmRequest & USB_REQ_RECIPIENT_MASK) == USB_REQ_RECIPIENT_INTERFACE && req->wIndex == AUDIO_INTERFACE_IDX) ||
	    ((req->bmRequest & USB_REQ_RECIPIENT_MASK) == USB_REQ_RECIPIENT_ENDPOINT && ((req->wIndex & 0x7F) == AUDIO_EP_IDX))) {
		ret = USBD_AUDIO_Setup(pdev, req);
	}

#if HRDW_HAS_USB_DEBUG
	else if (((req->bmRequest & USB_REQ_RECIPIENT_MASK) == USB_REQ_RECIPIENT_INTERFACE && req->wIndex == DEBUG_INTERFACE_IDX) ||
	         ((req->bmRequest & USB_REQ_RECIPIENT_MASK) == USB_REQ_RECIPIENT_ENDPOINT && ((req->wIndex & 0x7F) == DEBUG_EP_IDX))) {
		ret = USBD_DEBUG_Setup(pdev, req);
	}
#endif

#if HRDW_HAS_USB_IQ
	else if (((req->bmRequest & USB_REQ_RECIPIENT_MASK) == USB_REQ_RECIPIENT_INTERFACE && req->wIndex == IQ_INTERFACE_IDX) ||
	         ((req->bmRequest & USB_REQ_RECIPIENT_MASK) == USB_REQ_RECIPIENT_ENDPOINT && ((req->wIndex & 0x7F) == IQ_EP_IDX))) {
		ret = USBD_IQ_Setup(pdev, req);
	}
#endif

#if HRDW_HAS_SD
	else if (((req->bmRequest & USB_REQ_RECIPIENT_MASK) == USB_REQ_RECIPIENT_INTERFACE && req->wIndex == STORAGE_INTERFACE_IDX) ||
	         ((req->bmRequest & USB_REQ_RECIPIENT_MASK) == USB_REQ_RECIPIENT_ENDPOINT && ((req->wIndex & 0x7F) == STORAGE_EP_IDX))) {
		ret = USBD_MSC_Setup(pdev, req);
	}
#endif

#if HRDW_HAS_USB_CAT
	else
		ret = USBD_CAT_Setup(pdev, req);
#endif

	return ret;
}

/**
 * @brief  USBD_CDC_DataIn
 *         Data sent on non-control IN endpoint
 * @param  pdev: device instance
 * @param  epnum: endpoint number
 * @retval status
 */
#if HRDW_HAS_USB_DEBUG
static uint8_t USBD_DEBUG_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum) {
	USBD_DEBUG_HandleTypeDef *hcdc_debug = (USBD_DEBUG_HandleTypeDef *)pdev->pClassDataDEBUG;
	PCD_HandleTypeDef *hpcd_debug = pdev->pUserDataDEBUG;

	if (pdev->pClassDataDEBUG != NULL) {
		if ((pdev->ep_in[epnum].total_length > 0U) && ((pdev->ep_in[epnum].total_length % hpcd_debug->IN_ep[epnum].maxpacket) == 0U)) {
			/* Update the packet total length */
			pdev->ep_in[epnum].total_length = 0U;

			/* Send ZLP */
			USBD_LL_Transmit(pdev, epnum, NULL, 0U);
		} else {
			hcdc_debug->TxState = 0U;
		}
		return USBD_OK;
	} else {
		return USBD_FAIL;
	}
}
#endif

#if HRDW_HAS_USB_CAT
static uint8_t USBD_CAT_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum) {
	USBD_CAT_HandleTypeDef *hcdc_cat = (USBD_CAT_HandleTypeDef *)pdev->pClassDataCAT;
	PCD_HandleTypeDef *hpcd_cat = pdev->pUserDataCAT;

	if (pdev->pClassDataCAT != NULL) {
		if ((pdev->ep_in[epnum].total_length > 0U) && ((pdev->ep_in[epnum].total_length % hpcd_cat->IN_ep[epnum].maxpacket) == 0U)) {
			/* Update the packet total length */
			pdev->ep_in[epnum].total_length = 0U;

			/* Send ZLP */
			USBD_LL_Transmit(pdev, epnum, NULL, 0U);
		} else {
			hcdc_cat->TxState = 0U;
		}
		return USBD_OK;
	} else {
		return USBD_FAIL;
	}
}
#endif

static uint8_t USBD_AUDIO_DataIn(USBD_HandleTypeDef *pdev) {
	// Send audio to Host
	USBD_AUDIO_HandleTypeDef *hcdc_audio = (USBD_AUDIO_HandleTypeDef *)pdev->pClassDataAUDIO;
	if (rx_audio_buffer_head >= USB_AUDIO_RX_BUFFER_SIZE) {
		if (USB_AUDIO_need_rx_buffer == true) {
			RX_USB_AUDIO_underrun = true;
		} else {
			if (USB_AUDIO_current_rx_buffer) {
				hcdc_audio->RxBuffer = (uint8_t *)&USB_AUDIO_rx_buffer_b;
			} else {
				hcdc_audio->RxBuffer = (uint8_t *)&USB_AUDIO_rx_buffer_a;
			}
			USB_AUDIO_current_rx_buffer = !USB_AUDIO_current_rx_buffer;
			USB_AUDIO_need_rx_buffer = true;
			rx_audio_buffer_head = 0;
		}
	}
	if ((USB_AUDIO_RX_BUFFER_SIZE - rx_audio_buffer_head) >= AUDIO_OUT_PACKET) { //-V658
		rx_audio_buffer_step = AUDIO_OUT_PACKET;
	} else {
		rx_audio_buffer_step = (USB_AUDIO_RX_BUFFER_SIZE - rx_audio_buffer_head);
	}

	pdev->ep_in[AUDIO_IN_EP & 0xFU].total_length = rx_audio_buffer_step;
	HAL_PCD_EP_Transmit(pdev->pData, AUDIO_IN_EP, hcdc_audio->RxBuffer + rx_audio_buffer_head, rx_audio_buffer_step);
	RX_USB_AUDIO_SAMPLES += rx_audio_buffer_step / (BYTES_IN_SAMPLE_AUDIO_OUT_PACKET * 2); // 2/3 byte (16/24 bit) * 2 channel
	rx_audio_buffer_head += rx_audio_buffer_step;
	return USBD_OK;
}

#if HRDW_HAS_USB_IQ
static uint8_t USBD_IQ_DataIn(USBD_HandleTypeDef *pdev) {
	// Send iq to Host
	USBD_IQ_HandleTypeDef *hcdc_audio = (USBD_IQ_HandleTypeDef *)pdev->pClassDataIQ;
	if (rx_iq_buffer_head >= USB_AUDIO_RX_BUFFER_SIZE) {
		if (USB_IQ_need_rx_buffer == true) {
			RX_USB_AUDIO_underrun = true;
		} else {
			if (USB_IQ_current_rx_buffer) {
				hcdc_audio->RxBuffer = (uint8_t *)&USB_IQ_rx_buffer_b;
			} else {
				hcdc_audio->RxBuffer = (uint8_t *)&USB_IQ_rx_buffer_a;
			}
			USB_IQ_current_rx_buffer = !USB_IQ_current_rx_buffer;
			USB_IQ_need_rx_buffer = true;
			rx_iq_buffer_head = 0;
		}
	}
	if ((USB_AUDIO_RX_BUFFER_SIZE - rx_iq_buffer_head) >= AUDIO_OUT_PACKET) { //-V658
		rx_iq_buffer_step = AUDIO_OUT_PACKET;
	} else {
		rx_iq_buffer_step = (USB_AUDIO_RX_BUFFER_SIZE - rx_iq_buffer_head);
	}

	pdev->ep_in[IQ_IN_EP & 0xFU].total_length = rx_iq_buffer_step;
	HAL_PCD_EP_Transmit(pdev->pData, IQ_IN_EP, hcdc_audio->RxBuffer + rx_iq_buffer_head, rx_iq_buffer_step);
	RX_USB_IQ_SAMPLES += rx_iq_buffer_step / (BYTES_IN_SAMPLE_AUDIO_OUT_PACKET * 2); // 3 byte (24 bit) * 2 channel
	rx_iq_buffer_head += rx_iq_buffer_step;

	return USBD_OK;
}
#endif

#if HRDW_HAS_SD
uint8_t USBD_MSC_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum) {
	MSC_BOT_DataIn(pdev, epnum);

	return (uint8_t)USBD_OK;
}
#endif

static uint8_t USBD_UA3REO_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum) {
	if (epnum == AUDIO_EP_IDX) {
		return USBD_AUDIO_DataIn(pdev);
	}

#if HRDW_HAS_USB_IQ
	if (epnum == IQ_EP_IDX) {
		return USBD_IQ_DataIn(pdev);
	}
#endif

#if HRDW_HAS_USB_DEBUG
	if (epnum == DEBUG_EP_IDX) {
		return USBD_DEBUG_DataIn(pdev, epnum);
	}
#endif

#if HRDW_HAS_USB_CAT
	if (epnum == CAT_EP_IDX) {
		return USBD_CAT_DataIn(pdev, epnum);
	}
#endif

#if HRDW_HAS_SD
	if (epnum == STORAGE_EP_IDX) {
		return USBD_MSC_DataIn(pdev, epnum);
	}
#endif

	return USBD_FAIL;
}
/**
 * @brief  USBD_CDC_DataOut
 *         Data received on non-control Out endpoint
 * @param  pdev: device instance
 * @param  epnum: endpoint number
 * @retval status
 */
#if HRDW_HAS_USB_DEBUG
static uint8_t USBD_DEBUG_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum) {
	USB_LastActiveTime = HAL_GetTick();
	USBD_DEBUG_HandleTypeDef *hcdc_debug = (USBD_DEBUG_HandleTypeDef *)pdev->pClassDataDEBUG;
	/* Get the received data length */
	hcdc_debug->RxLength = USBD_LL_GetRxDataSize(pdev, epnum);
	/* USB data will be immediately processed, this allow next USB traffic being NAKed till the end of the application Xfer */
	if (pdev->pClassDataDEBUG != NULL) {
		((USBD_DEBUG_ItfTypeDef *)pdev->pUserDataDEBUG)->Receive(hcdc_debug->RxBuffer);
		return USBD_OK;
	} else {
		return USBD_FAIL;
	}
}
#endif

#if HRDW_HAS_USB_CAT
static uint8_t USBD_CAT_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum) {
	USB_LastActiveTime = HAL_GetTick();
	USBD_CAT_HandleTypeDef *hcdc_cat = (USBD_CAT_HandleTypeDef *)pdev->pClassDataCAT;
	/* Get the received data length */
	hcdc_cat->RxLength = USBD_LL_GetRxDataSize(pdev, epnum);
	/* USB data will be immediately processed, this allow next USB traffic being
	NAKed till the end of the application Xfer */
	if (pdev->pClassDataCAT != NULL) {
		((USBD_CAT_ItfTypeDef *)pdev->pUserDataCAT)->Receive(hcdc_cat->RxBuffer, &hcdc_cat->RxLength);
		return USBD_OK;
	} else {
		return USBD_FAIL;
	}
}
#endif

static uint8_t USBD_AUDIO_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum) {
	USB_LastActiveTime = HAL_GetTick();
	USBD_AUDIO_HandleTypeDef *haudio = (USBD_AUDIO_HandleTypeDef *)pdev->pClassDataAUDIO;
	if (epnum == AUDIO_OUT_EP) {
		haudio->TxBufferIndex += AUDIO_OUT_PACKET;
		if (haudio->TxBufferIndex >= USB_AUDIO_TX_BUFFER_SIZE) {
			haudio->TxBufferIndex = 0;
		}
		USBD_LL_PrepareReceive(pdev, AUDIO_OUT_EP, haudio->TxBuffer + haudio->TxBufferIndex, AUDIO_OUT_PACKET);
		TX_USB_AUDIO_SAMPLES += AUDIO_OUT_PACKET / (BYTES_IN_SAMPLE_AUDIO_OUT_PACKET * 2); // 3 byte (24 bit) * 2 channel
	}
	return USBD_OK;
}

#if HRDW_HAS_SD
uint8_t USBD_MSC_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum) {
	MSC_BOT_DataOut(pdev, epnum);

	return (uint8_t)USBD_OK;
}
#endif

static uint8_t USBD_UA3REO_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum) {
#if HRDW_HAS_USB_DEBUG
	if (epnum == DEBUG_EP_IDX) {
		return USBD_DEBUG_DataOut(pdev, epnum);
	}
#endif

#if HRDW_HAS_USB_CAT
	if (epnum == CAT_EP_IDX) {
		return USBD_CAT_DataOut(pdev, epnum);
	}
#endif

	if (epnum == AUDIO_EP_IDX) {
		return USBD_AUDIO_DataOut(pdev, epnum);
	}

#if HRDW_HAS_SD
	if (epnum == STORAGE_EP_IDX) {
		return USBD_MSC_DataOut(pdev, epnum);
	}
#endif

	return USBD_FAIL;
}

/**
 * @brief  USBD_CDC_EP0_RxReady
 *         Handle EP0 Rx Ready event
 * @param  pdev: device instance
 * @retval status
 */

static uint8_t USBD_UA3REO_EP0_RxReady(USBD_HandleTypeDef *pdev) {
	USB_LastActiveTime = HAL_GetTick();

#if HRDW_HAS_USB_DEBUG
	// DEBUG
	USBD_DEBUG_HandleTypeDef *hcdc_debug = (USBD_DEBUG_HandleTypeDef *)pdev->pClassDataDEBUG;
	if ((pdev->pUserDataDEBUG != NULL) && (hcdc_debug->CmdOpCode != 0xFFU)) {
		((USBD_DEBUG_ItfTypeDef *)pdev->pUserDataDEBUG)->Control(hcdc_debug->CmdOpCode, (uint8_t *)(void *)hcdc_debug->data, 0);
		hcdc_debug->CmdOpCode = 0xFFU;
	}
#endif

#if HRDW_HAS_USB_CAT
	// CAT
	USBD_CAT_HandleTypeDef *hcdc_cat = (USBD_CAT_HandleTypeDef *)pdev->pClassDataCAT;
	if ((pdev->pUserDataCAT != NULL) && (hcdc_cat->CmdOpCode != 0xFFU)) {
		((USBD_CAT_ItfTypeDef *)pdev->pUserDataCAT)->Control(hcdc_cat->CmdOpCode, (uint8_t *)(void *)hcdc_cat->data, 0);
		hcdc_cat->CmdOpCode = 0xFFU;
	}
#endif

	// AUDIO
	USBD_AUDIO_HandleTypeDef *haudio;
	haudio = (USBD_AUDIO_HandleTypeDef *)pdev->pClassDataAUDIO;

	if (haudio->control.cmd == AUDIO_REQ_SET_CUR) {
		if (haudio->control.unit == AUDIO_OUT_STREAMING_CTRL) {
			haudio->control.cmd = 0U;
			haudio->control.len = 0U;
		}
	}

#if HRDW_HAS_USB_IQ
	// IQ
	USBD_IQ_HandleTypeDef *haudioiq = (USBD_IQ_HandleTypeDef *)pdev->pClassDataIQ;
#endif

	return USBD_OK;
}

static uint8_t USBD_UA3REO_EP0_TxReady(void) {
	/* Only OUT control data are processed */
	return USBD_OK;
}

static uint8_t USBD_UA3REO_IsoINIncomplete(USBD_HandleTypeDef *pdev, uint8_t epnum) { return USBD_OK; }

static uint8_t USBD_UA3REO_IsoOutIncomplete(USBD_HandleTypeDef *pdev, uint8_t epnum) { return USBD_OK; }

/**
 * @brief  USBD_CDC_GetFSCfgDesc
 *         Return configuration descriptor
 * @param  speed : current device speed
 * @param  length : pointer data length
 * @retval pointer to descriptor buffer
 */
static const uint8_t *USBD_UA3REO_GetFSCfgDesc(uint16_t *length) {
	*length = sizeof(USBD_UA3REO_CfgFSDesc);
	return USBD_UA3REO_CfgFSDesc;
}

/**
 * @brief  DeviceQualifierDescriptor
 *         return Device Qualifier descriptor
 * @param  length : pointer data length
 * @retval pointer to descriptor buffer
 */
static const uint8_t *USBD_UA3REO_GetDeviceQualifierDescriptor(uint16_t *length) {
	*length = sizeof(USBD_UA3REO_DeviceQualifierDesc);
	return USBD_UA3REO_DeviceQualifierDesc;
}

/**
 * @brief  USBD_CDC_RegisterInterface
 * @param  pdev: device instance
 * @param  fops: CD  Interface callback
 * @retval status
 */
#if HRDW_HAS_USB_DEBUG
uint8_t USBD_DEBUG_RegisterInterface(USBD_HandleTypeDef *pdev, USBD_DEBUG_ItfTypeDef *fops) {
	uint8_t ret = USBD_FAIL;
	if (fops != NULL) {
		pdev->pUserDataDEBUG = fops;
		ret = USBD_OK;
	}
	return ret;
}
#endif

#if HRDW_HAS_USB_CAT
uint8_t USBD_CAT_RegisterInterface(USBD_HandleTypeDef *pdev, USBD_CAT_ItfTypeDef *fops) {
	uint8_t ret = USBD_FAIL;
	if (fops != NULL) {
		pdev->pUserDataCAT = fops;
		ret = USBD_OK;
	}
	return ret;
}
#endif

uint8_t USBD_AUDIO_RegisterInterface(USBD_HandleTypeDef *pdev, USBD_AUDIO_ItfTypeDef *fops) {
	if (fops != NULL) {
		pdev->pUserDataAUDIO = fops;
	}
	return USBD_OK;
}

#if HRDW_HAS_USB_IQ
uint8_t USBD_IQ_RegisterInterface(USBD_HandleTypeDef *pdev, USBD_IQ_ItfTypeDef *fops) {
	if (fops != NULL) {
		pdev->pUserDataIQ = fops;
	}
	return USBD_OK;
}
#endif

/**
 * @brief  USBD_CDC_SetTxBuffer
 * @param  pdev: device instance
 * @param  pbuff: Tx Buffer
 * @retval status
 */
#if HRDW_HAS_USB_DEBUG
uint8_t USBD_DEBUG_SetTxBuffer(USBD_HandleTypeDef *pdev, uint8_t *pbuff, uint16_t length) {
	USBD_DEBUG_HandleTypeDef *hcdc = (USBD_DEBUG_HandleTypeDef *)pdev->pClassDataDEBUG;
	hcdc->TxBuffer = pbuff;
	hcdc->TxLength = length;
	return USBD_OK;
}
#endif

#if HRDW_HAS_USB_CAT
uint8_t USBD_CAT_SetTxBuffer(USBD_HandleTypeDef *pdev, uint8_t *pbuff, uint16_t length) {
	USBD_CAT_HandleTypeDef *hcdc = (USBD_CAT_HandleTypeDef *)pdev->pClassDataCAT;
	hcdc->TxBuffer = pbuff;
	hcdc->TxLength = length;
	return USBD_OK;
}
#endif

/**
 * @brief  USBD_CDC_SetRxBuffer
 * @param  pdev: device instance
 * @param  pbuff: Rx Buffer
 * @retval status
 */
#if HRDW_HAS_USB_DEBUG
uint8_t USBD_DEBUG_SetRxBuffer(USBD_HandleTypeDef *pdev, uint8_t *pbuff) {
	USBD_DEBUG_HandleTypeDef *hcdc = (USBD_DEBUG_HandleTypeDef *)pdev->pClassDataDEBUG;
	hcdc->RxBuffer = pbuff;
	return USBD_OK;
}
#endif

#if HRDW_HAS_USB_CAT
uint8_t USBD_CAT_SetRxBuffer(USBD_HandleTypeDef *pdev, uint8_t *pbuff) {
	USBD_CAT_HandleTypeDef *hcdc = (USBD_CAT_HandleTypeDef *)pdev->pClassDataCAT;
	hcdc->RxBuffer = pbuff;
	return USBD_OK;
}
#endif

/**
 * @brief  USBD_CDC_TransmitPacket
 *         Transmit packet on IN endpoint
 * @param  pdev: device instance
 * @retval status
 */
#if HRDW_HAS_USB_DEBUG
uint8_t USBD_DEBUG_TransmitPacket(USBD_HandleTypeDef *pdev) {
	USBD_DEBUG_HandleTypeDef *hcdc = (USBD_DEBUG_HandleTypeDef *)pdev->pClassDataDEBUG;
	if (pdev->pClassDataDEBUG != NULL) {
		if (hcdc->TxState == 0U) {
			/* Tx Transfer in progress */
			hcdc->TxState = 1U;
			/* Update the packet total length */
			pdev->ep_in[DEBUG_IN_EP & 0xFU].total_length = hcdc->TxLength;
			/* Transmit next packet */
			USBD_LL_Transmit(pdev, DEBUG_IN_EP, hcdc->TxBuffer, (uint16_t)hcdc->TxLength);
			return USBD_OK;
		} else {
			return USBD_BUSY;
		}
	} else {
		return USBD_FAIL;
	}
}
#endif

#if HRDW_HAS_USB_CAT
uint8_t USBD_CAT_TransmitPacket(USBD_HandleTypeDef *pdev) {
	USBD_CAT_HandleTypeDef *hcdc = (USBD_CAT_HandleTypeDef *)pdev->pClassDataCAT;
	if (pdev->pClassDataCAT != NULL) {
		if (hcdc->TxState == 0U) {
			hcdc->TxState = 1U;
			pdev->ep_in[CAT_IN_EP & 0xFU].total_length = hcdc->TxLength;
			USBD_LL_Transmit(pdev, CAT_IN_EP, hcdc->TxBuffer, (uint16_t)hcdc->TxLength);
			return USBD_OK;
		} else {
			return USBD_BUSY;
		}
	} else {
		return USBD_FAIL;
	}
}
#endif

uint8_t USBD_AUDIO_StartTransmit(USBD_HandleTypeDef *pdev) {
	if (pdev->pClassDataAUDIO != NULL) {
		println("[OK] Start Send USB Audio");

		USBD_AUDIO_DataIn(pdev);
		return USBD_OK;
	} else {
		return USBD_FAIL;
	}
}

#if HRDW_HAS_USB_IQ
uint8_t USBD_IQ_StartTransmit(USBD_HandleTypeDef *pdev) {
	if (pdev->pClassDataIQ != NULL) {
		println("[OK] Start Send USB IQ");

		USBD_IQ_DataIn(pdev);
		return USBD_OK;
	} else {
		return USBD_FAIL;
	}
}
#endif

uint8_t USBD_AUDIO_StartReceive(USBD_HandleTypeDef *pdev) {
	USBD_AUDIO_HandleTypeDef *haudio = (USBD_AUDIO_HandleTypeDef *)pdev->pClassDataAUDIO;
	if (pdev->pClassDataAUDIO != NULL) {
		println("[OK] Start Receive USB Audio");
		USBD_LL_PrepareReceive(pdev, AUDIO_OUT_EP, haudio->TxBuffer + haudio->TxBufferIndex, AUDIO_OUT_PACKET);
		return USBD_OK;
	} else {
		return USBD_FAIL;
	}
}

/**
 * @brief  USBD_CDC_ReceivePacket
 *         prepare OUT Endpoint for reception
 * @param  pdev: device instance
 * @retval status
 */
#if HRDW_HAS_USB_DEBUG
uint8_t USBD_DEBUG_ReceivePacket(USBD_HandleTypeDef *pdev) {
	USBD_DEBUG_HandleTypeDef *hcdc = (USBD_DEBUG_HandleTypeDef *)pdev->pClassDataDEBUG;
	/* Suspend or Resume USB Out process */
	if (pdev->pClassDataDEBUG != NULL) {
		/* Prepare Out endpoint to receive next packet */
		USBD_LL_PrepareReceive(pdev, DEBUG_OUT_EP, hcdc->RxBuffer, CDC_DATA_FS_OUT_PACKET_SIZE);
		return USBD_OK;
	} else {
		return USBD_FAIL;
	}
}
#endif

#if HRDW_HAS_USB_CAT
uint8_t USBD_CAT_ReceivePacket(USBD_HandleTypeDef *pdev) {
	USBD_CAT_HandleTypeDef *hcdc = (USBD_CAT_HandleTypeDef *)pdev->pClassDataCAT;
	/* Suspend or Resume USB Out process */
	if (pdev->pClassDataCAT != NULL) {
		/* Prepare Out endpoint to receive next packet */
		USBD_LL_PrepareReceive(pdev, CAT_OUT_EP, hcdc->RxBuffer, CDC_DATA_FS_OUT_PACKET_SIZE);
		return USBD_OK;
	} else {
		return USBD_FAIL;
	}
}
#endif

static void AUDIO_REQ_GetCurrent(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req) {
	USBD_AUDIO_HandleTypeDef *haudio;
	haudio = (USBD_AUDIO_HandleTypeDef *)pdev->pClassDataAUDIO;

	dma_memset(haudio->control.data, 0, sizeof(haudio->control.data));

	/* Send the current mute state */
	USBD_CtlSendData(pdev, haudio->control.data, req->wLength);
}

#if HRDW_HAS_USB_IQ
static void IQ_REQ_GetCurrent(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req) {
	USBD_IQ_HandleTypeDef *haudio;
	haudio = (USBD_IQ_HandleTypeDef *)pdev->pClassDataIQ;

	dma_memset(haudio->control.data, 0, sizeof(haudio->control.data));

	/* Send the current mute state */
	USBD_CtlSendData(pdev, haudio->control.data, req->wLength);
}
#endif

/**
 * @brief  AUDIO_Req_SetCurrent
 *         Handles the SET_CUR Audio control request.
 * @param  pdev: instance
 * @param  req: setup class request
 * @retval status
 */
static void AUDIO_REQ_SetCurrent(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req) {
	USBD_AUDIO_HandleTypeDef *haudio;
	haudio = (USBD_AUDIO_HandleTypeDef *)pdev->pClassDataAUDIO;

	if (req->wLength) {
		/* Prepare the reception of the buffer over EP0 */
		USBD_CtlPrepareRx(pdev, haudio->control.data, req->wLength);

		haudio->control.cmd = AUDIO_REQ_SET_CUR;     /* Set the request value */
		haudio->control.len = (uint8_t)req->wLength; /* Set the request data length */
		haudio->control.unit = HIBYTE(req->wIndex);  /* Set the request target unit */
	}
}

#if HRDW_HAS_USB_IQ
static void IQ_REQ_SetCurrent(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req) {
	USBD_IQ_HandleTypeDef *haudio;
	haudio = (USBD_IQ_HandleTypeDef *)pdev->pClassDataIQ;

	if (req->wLength) {
		/* Prepare the reception of the buffer over EP0 */
		USBD_CtlPrepareRx(pdev, haudio->control.data, req->wLength);

		haudio->control.cmd = AUDIO_REQ_SET_CUR;     /* Set the request value */
		haudio->control.len = (uint8_t)req->wLength; /* Set the request data length */
		haudio->control.unit = HIBYTE(req->wIndex);  /* Set the request target unit */
	}
}
#endif

static uint8_t USBD_UA3REO_SOF(USBD_HandleTypeDef *pdev) {
	USB_LastActiveTime = HAL_GetTick();
	return USBD_OK;
}

void USBD_Restart(void) {
	MX_USB_DevDisconnect();
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	// GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_12;
	GPIO_InitStruct.Pin = GPIO_PIN_12;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	// HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET);
	// HAL_Delay(1000);

	MX_USB_DEVICE_Init();
	USB_LastActiveTime = HAL_GetTick();
}

#if HRDW_HAS_SD
/**
 * @brief  USBD_MSC_RegisterStorage
 * @param  fops: storage callback
 * @retval status
 */
uint8_t USBD_MSC_RegisterStorage(USBD_HandleTypeDef *pdev, USBD_StorageTypeDef *fops) {
	if (fops == NULL) {
		return (uint8_t)USBD_FAIL;
	}

	pdev->pUserDataSTORAGE = fops;

	return (uint8_t)USBD_OK;
}
#endif
