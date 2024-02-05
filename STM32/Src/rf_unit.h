#ifndef RF_UNIT_h
#define RF_UNIT_h

#include "hardware.h"
#include <stdbool.h>

#define SET_DATA_PIN HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET)

extern bool FAN_Active;

extern void RF_UNIT_UpdateState(bool clean);
extern void RF_UNIT_ProcessSensors(void);
extern void RF_UNIT_MeasureVoltage(void); // Tisho

#endif
