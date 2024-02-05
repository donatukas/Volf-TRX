#ifndef SELF_TEST_H
#define SELF_TEST_H

#include "functions.h"
#include "hardware.h"
#include "main.h"
#include "stdbool.h"
#include "stdint.h"

#define SELF_TEST_pages 7
#define SELF_TEST_frequency_HF 14000000
#define SELF_TEST_frequency_VHF 145000000
#define SELF_TEST_adc_test_latency 500

// Public variabled
extern bool SYSMENU_selftest_opened;

// Public methods
extern void SELF_TEST_Start(void);                 // launch
extern void SELF_TEST_Stop(void);                  // stop session
extern void SELF_TEST_Draw(void);                  // drawing
extern void SELF_TEST_EncRotate(int8_t direction); // events per encoder tick

#endif
