#ifndef AUTO_CALIBRATION_H
#define AUTO_CALIBRATION_H

#include "hardware.h"

// Public variabled
extern bool SYSMENU_auto_calibration_opened;

// Public methods
extern void AUTO_CALIBRATION_Start_SWR(void);              // launch
extern void AUTO_CALIBRATION_Start_POWER(void);            // launch
extern void AUTO_CALIBRATION_Stop(void);                   // stop session
extern void AUTO_CALIBRATION_Draw(void);                   // drawing
extern void AUTO_CALIBRATION_EncRotate(int8_t direction);  // events per encoder tick
extern void AUTO_CALIBRATION_Enc2Rotate(int8_t direction); // events per encoder tick
extern void AUTO_CALIBRATION_Enc2Click();                  // events per encoder click

#endif
