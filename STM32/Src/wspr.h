#ifndef WSPR_H
#define WSPR_H

#include "hardware.h"
#include "main.h"
#include "stdbool.h"
#include "stdint.h"

typedef enum {
	WSPR_WAIT = 0x00U,
	WSPR_TRANSMIT = 0x01U,
} WSPRState;

// Public variabled
extern bool SYSMENU_wspr_opened;

// Public methods
extern void WSPR_Start(void);                 // launch
extern void WSPR_Stop(void);                  // stop session
extern void WSPR_DoEvents(void);              // drawing
extern void WSPR_DoFastEvents(void);          // transmitting
extern void WSPR_EncRotate(int8_t direction); // events per encoder tick

#endif
