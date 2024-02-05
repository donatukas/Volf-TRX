#ifndef __USBD_CDC_DEBUG_IF_H__
#define __USBD_CDC_DEBUG_IF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "usbd_ua3reo.h"

#if HRDW_HAS_USB_DEBUG
extern USBD_DEBUG_ItfTypeDef USBD_DEBUG_fops_FS;
uint8_t DEBUG_Transmit_FS(uint8_t *Buf, uint16_t Len);

void DEBUG_Transmit_FIFO(uint8_t *data, uint16_t length);
extern uint8_t DEBUG_Transmit_FIFO_Events(void);
#endif

#ifdef __cplusplus
}
#endif

#endif
