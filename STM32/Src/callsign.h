#ifndef CALLSIGN_H
#define CALLSIGN_H

#include "functions.h"
#include "hardware.h"
#include "main.h"
#include "stdbool.h"
#include "stdint.h"

#define CALLSIGN_DB_GLOBAL_COUNT 342
#define CALLSIGN_DB_RUS_COUNT 89

typedef struct // description of the call database
{
	const float32_t lat;
	const float32_t lon;
	const uint8_t cq_zone;
	const uint8_t itu_zone;
	const char *continent;
	const char *country;
	const char *prefixes;
} CALLSIGN_INFO_LINE;

// Public variabled
extern bool SYSMENU_callsign_info_opened;

// Public methods
extern void CALLSIGN_INFO_Start(void);                 // launch
extern void CALLSIGN_INFO_Stop(void);                  // stop session
extern void CALLSIGN_INFO_Draw(void);                  // drawing
extern void CALLSIGN_INFO_EncRotate(int8_t direction); // events per encoder tick

extern void CALLSIGN_getInfoByCallsign(CALLSIGN_INFO_LINE **ret, char *call);

#endif
