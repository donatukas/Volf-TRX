#ifndef __USBD_CDC_CAT_IF_H__
#define __USBD_CDC_CAT_IF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "usbd_ua3reo.h"

#if HRDW_HAS_USB_CAT
extern USBD_CAT_ItfTypeDef USBD_CAT_fops_FS;
extern void ua3reo_dev_cat_parseCommand(void);
extern void CAT_SetWIFICommand(char *data, uint32_t length, uint32_t link_id);
extern void CAT_Transmit_FIFO(uint8_t *data, uint16_t length);
extern uint8_t CAT_Transmit_FIFO_Events(void);
#endif

#ifdef __cplusplus
}
#endif

#endif
