#ifndef LOCATOR_H
#define LOCATOR_H

#include "functions.h"
#include "hardware.h"
#include "main.h"
#include "stdbool.h"
#include "stdint.h"

// Public variabled
extern bool SYSMENU_locator_info_opened;

// Public methods
extern void LOCINFO_Start(void);                 // launch
extern void LOCINFO_Stop(void);                  // stop session
extern void LOCINFO_Draw(void);                  // drawing
extern void LOCINFO_EncRotate(int8_t direction); // events per encoder tick

extern float32_t LOCINFO_get_latlon_from_locator(char *locator, bool return_lat);
extern float32_t LOCINFO_distanceInKmBetweenEarthCoordinates(float32_t lat1, float32_t lon1, float32_t lat2, float32_t lon2);
extern float32_t LOCINFO_azimuthFromCoordinates(float32_t lat1, float32_t lon1, float32_t lat2, float32_t lon2);

#endif
