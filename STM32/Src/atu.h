#ifndef ATU_H
#define ATU_H

#include "hardware.h"
#include "settings.h"

typedef struct {
	uint8_t I;
	uint8_t C;
	bool T;
} ATU_MEMORY_STATE;

typedef struct {
	uint8_t loaded_ant;
	bool saved;
	ATU_MEMORY_STATE state[ATU_MEM_COUNT];
} ATU_MEMORY_TYPE;

extern bool ATU_TunePowerStabilized;

extern void ATU_Process(void);
extern void ATU_Invalidate(void);
extern void ATU_SetCompleted(void);

extern void ATU_Flush_Memory();
extern void ATU_Load_ANT_Banks(void);
extern void ATU_Save_Memory(uint8_t ant, uint64_t frequency, uint8_t I, uint8_t C, bool T);
extern void ATU_Load_Memory(uint8_t ant, uint64_t frequency);
extern void ResetATUBanks();

#endif
