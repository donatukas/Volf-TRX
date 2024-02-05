#ifndef __EVENTS_H
#define __EVENTS_H

#include "arm_math_types.h"
#include "main.h"
#include <stdbool.h>

extern void EVENTS_do_WSPR(void);
extern void EVENTS_do_WIFI(void);
extern void EVENTS_do_FFT(void);
extern void EVENTS_do_AUDIO_PROCESSOR(void);
extern void EVENTS_do_USB_FIFO(void);
extern void EVENTS_do_PERIPHERAL(void);
extern void EVENTS_do_ENC(void);
extern void EVENTS_do_PREPROCESS(void);
extern void EVENTS_do_EVERY_10ms(void);
extern void EVENTS_do_EVERY_100ms(void);
extern void EVENTS_do_EVERY_500ms(void);
extern void EVENTS_do_EVERY_1000ms(void);

#endif
