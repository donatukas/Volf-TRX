#ifndef TOUCHPAD_h
#define TOUCHPAD_h

#include "settings.h"

#if (defined(TOUCHPAD_GT911))
#include "touchpad_GT911.h"
#endif

extern void TOUCHPAD_Init(void);
extern void TOUCHPAD_ProcessInterrupt(void);
extern void TOUCHPAD_reserveInterrupt(void);
extern void TOUCHPAD_processTouch(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

#endif
