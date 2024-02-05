#ifndef __RFFC2071_PWR_MONITOR_H
#define __RFFC2071_PWR_MONITOR_H

#include "hardware.h"
#if HRDW_HAS_RFFC2071_MIXER

#define RFMIXER_MIN_FREQ_MHz 60
#define RFMIXER_MAX_FREQ_MHz 5000 // loss after 2.7
#define RFMIXER_IF_FREQ 29000000

#define RFFC2071_REGS_NUM 31
#define RFFC2071_LO_MAX 5400000000
#define RFFC2071_REF_FREQ 39000000

extern void RFMIXER_Init(void);
extern void RFMIXER_disable(void);
extern void RFMIXER_enable(void);
extern uint64_t RFMIXER_Freq_Set(uint64_t lo_freq_Hz);

#endif
#endif
