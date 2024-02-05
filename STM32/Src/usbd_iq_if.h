#ifndef __USBD_IQ_IF_H__
#define __USBD_IQ_IF_H__

#if HRDW_HAS_USB_IQ

#ifdef __cplusplus
extern "C" {
#endif

#include "functions.h"
#include "usbd_ua3reo.h"

extern USBD_IQ_ItfTypeDef USBD_IQ_fops_FS;

extern uint8_t SRAM4 USB_IQ_rx_buffer_a[USB_AUDIO_RX_BUFFER_SIZE];
extern uint8_t SRAM4 USB_IQ_rx_buffer_b[USB_AUDIO_RX_BUFFER_SIZE];
extern volatile bool USB_IQ_current_rx_buffer; // a-false b-true
extern volatile bool USB_IQ_need_rx_buffer;    // a-false b-true

void TransferComplete_CallBack_FS(void);
void HalfTransfer_CallBack_FS(void);
#ifdef __cplusplus
}
#endif

#endif

#endif
