#include "atu.h"
#include "lcd.h"
#include "settings.h"
#include "trx_manager.h"

static bool ATU_Finished = false;
static bool ATU_InProcess = false;
static bool ATU_BestValsProbed = false;
static float32_t ATU_MinSWR = 1.0;
static uint8_t ATU_MinSWR_I = 0;
static uint8_t ATU_MinSWR_C = 0;
static bool ATU_MinSWR_T = false;
static uint8_t ATU_Stage = 0;
bool ATU_TunePowerStabilized = false;

#if !defined(FRONTPANEL_LITE) && !defined(FRONTPANEL_X1)
SRAM4 ATU_MEMORY_TYPE ATU_MEMORY_1 = {.loaded_ant = 99, .saved = true};
SRAM4 ATU_MEMORY_TYPE ATU_MEMORY_2 = {.loaded_ant = 99, .saved = true};
SRAM4 ATU_MEMORY_TYPE ATU_MEMORY_3 = {.loaded_ant = 99, .saved = true};
SRAM4 ATU_MEMORY_TYPE ATU_MEMORY_4 = {.loaded_ant = 99, .saved = true};
#endif

void ATU_Invalidate(void) {
	ATU_Finished = false;
	ATU_InProcess = false;
	ATU_TunePowerStabilized = false;
	ATU_BestValsProbed = false;
}

void ATU_SetCompleted(void) { ATU_Finished = true; }

void ATU_Process(void) {
	if (!TRX.ATU_Enabled || !TRX.TUNER_Enabled) {
		ATU_Finished = true;
		return;
	}

	if (TRX.TWO_SIGNAL_TUNE) {
		ATU_Finished = true;
		return;
	}

	if (!TRX_Tune) {
		return;
	}

	if (ATU_Finished) {
		return;
	}

	if (TRX.RF_Gain == 0) {
		ATU_Finished = true;
		return;
	}

	if (!ATU_TunePowerStabilized) {
		return;
	}

	if (!ATU_BestValsProbed) {
		ATU_Load_Memory(TRX_on_TX ? TRX.ANT_TX : TRX.ANT_RX, CurrentVFO->Freq);
		LCD_UpdateQuery.StatusInfoBar = true;
	}

#define delay_stages_saved (CALIBRATE.ATU_AVERAGING * 2)
#define delay_stages_tune CALIBRATE.ATU_AVERAGING
	static float32_t TRX_SWR_val = 1.0f;
	static uint8_t delay_stages_count = 0;
	if ((!ATU_BestValsProbed && delay_stages_count < delay_stages_saved) || (ATU_BestValsProbed && delay_stages_count < delay_stages_tune)) {
		if (delay_stages_count == 0) {
			TRX_SWR_val = TRX_SWR;
		}

		// if(TRX_SWR < TRX_SWR_val)
		// TRX_SWR_val = TRX_SWR;
		TRX_SWR_val = TRX_SWR_val * 0.5f + TRX_SWR * 0.5f;

		delay_stages_count++;
		return;
	} else {
		delay_stages_count = 0;
	}

	char buff[64] = {0};
	// float32_t TRX_PWR = TRX_PWR_Forward - TRX_PWR_Backward;
	float32_t TRX_PWR = TRX_PWR_Forward;

	if (TRX_PWR >= 0.0f) { // if (TRX_PWR >= 5.0f) {
		if ((!ATU_BestValsProbed && TRX_SWR_val <= NORMAL_SWR_SAVED) || (ATU_BestValsProbed && TRX_SWR_val <= NORMAL_SWR_TUNE)) {
			println("Normal SWR, stop!");
			sprintf(buff, "Best SWR: %.1f", (double)TRX_SWR_val);
			LCD_showTooltip(buff);
			ATU_Finished = true;
			delay_stages_count = 0;

			ATU_Save_Memory(TRX_on_TX ? TRX.ANT_TX : TRX.ANT_RX, CurrentVFO->Freq, TRX.ATU_I, TRX.ATU_C, TRX.ATU_T);
			BUTTONHANDLER_TUNE(0);
			ATU_Flush_Memory();
			return;
		}
	}

	if (!ATU_BestValsProbed) {
		ATU_BestValsProbed = true;
	}

	static float32_t ATU_MinSWR_Slider = 9.9f;
	static float32_t ATU_MinSWR_prev_1 = 9.9f;
	static float32_t ATU_MinSWR_prev_2 = 9.9f;
	static float32_t ATU_MinSWR_prev_3 = 9.9f;

	if (!ATU_Finished && !ATU_InProcess) {
		TRX.ATU_I = 0;
		TRX.ATU_C = 0;
		TRX.ATU_T = 0;
		ATU_MinSWR = 99.9f;
		ATU_MinSWR_prev_1 = ATU_MinSWR;
		ATU_MinSWR_prev_2 = ATU_MinSWR;
		ATU_MinSWR_prev_3 = ATU_MinSWR;
		ATU_MinSWR_I = 0;
		ATU_MinSWR_C = 0;
		ATU_MinSWR_T = false;
		ATU_Stage = 0;
		ATU_InProcess = true;
		LCD_UpdateQuery.StatusInfoBar = true;
	} else {
		// best result
		if (ATU_MinSWR > TRX_SWR_val) { //  && TRX_PWR >= (float32_t)CALIBRATE.TUNE_MAX_POWER / 2.0f
			ATU_MinSWR = TRX_SWR_val;
			ATU_MinSWR_I = TRX.ATU_I;
			ATU_MinSWR_C = TRX.ATU_C;
			ATU_MinSWR_T = TRX.ATU_T;
		}
		// wrong way?
		bool wrong_way = false;
		if (ATU_MinSWR_prev_3 < ATU_MinSWR_prev_2 && ATU_MinSWR_prev_2 < ATU_MinSWR_prev_1 && ATU_MinSWR_prev_1 < TRX_SWR_val) {
			if (fabsf(ATU_MinSWR_prev_3 - ATU_MinSWR) > 1.0f) {
				wrong_way = true;
			}
		}
		// println("PREV PREV: ", ATU_MinSWR_prev_prev, " PREV: ", ATU_MinSWR_prev, " NOW: ", TRX_SWR);
		ATU_MinSWR_prev_3 = ATU_MinSWR_prev_2;
		ATU_MinSWR_prev_2 = ATU_MinSWR_prev_1;
		ATU_MinSWR_prev_1 = TRX_SWR_val;

		// debug
		if (ATU_Stage == 0) {
			print("INDS ");
		}
		if (ATU_Stage == 1) {
			print("CAPS ");
		}
		if (ATU_Stage == 2) {
			print("CAPS T ");
		}
		if (ATU_Stage == 3) {
			print("I-1 ");
		}
		if (ATU_Stage == 4) {
			print("I+1 ");
		}
		if (wrong_way) {
			print("Wr Way ");
		}
		println("Stage: ", ATU_Stage, " I: ", TRX.ATU_I, " C: ", TRX.ATU_C, " T: ", (uint8_t)TRX.ATU_T, " SWR: ", (double)TRX_SWR_val, " PWR: ", (double)TRX_PWR);
		// iteration block

		if (ATU_Stage == 0) // iterate inds
		{
			if (TRX.ATU_I < ATU_MAXPOS && !wrong_way) {
				TRX.ATU_I++;
			} else {
				ATU_Stage = 1;
				TRX.ATU_I = ATU_MinSWR_I;
				TRX.ATU_C = 1;
				ATU_MinSWR_prev_3 = ATU_MinSWR_prev_2 = ATU_MinSWR_prev_1 = 9.9f;
			}
		} else if (ATU_Stage == 1) // iterate caps
		{
			if (TRX.ATU_C < ATU_MAXPOS && !wrong_way) {
				TRX.ATU_C++;
			} else {
				ATU_Stage = 2;
				TRX.ATU_C = 0;
				TRX.ATU_T = true;
				ATU_MinSWR_prev_3 = ATU_MinSWR_prev_2 = ATU_MinSWR_prev_1 = 9.9f;
			}
		} else if (ATU_Stage == 2) // iterate caps with other T
		{
			if (TRX.ATU_C < ATU_MAXPOS && !wrong_way) {
				TRX.ATU_C++;
			} else {
				TRX.ATU_C = 0;
				TRX.ATU_T = ATU_MinSWR_T;
				ATU_MinSWR_Slider = ATU_MinSWR;
				if (TRX.ATU_I > 0) {
					TRX.ATU_I = ATU_MinSWR_I - 1;
					ATU_Stage = 3;
				} else {
					ATU_Stage = 4;
				}
				ATU_MinSWR_prev_3 = ATU_MinSWR_prev_2 = ATU_MinSWR_prev_1 = 9.9f;
			}
		} else if (ATU_Stage == 3) // iterate caps with i-1
		{
			if (TRX.ATU_C < ATU_MAXPOS && !wrong_way) {
				TRX.ATU_C++;
			} else {
				// slide more?
				if (ATU_MinSWR < ATU_MinSWR_Slider && TRX.ATU_I > 0) {
					ATU_MinSWR_Slider = ATU_MinSWR;
					TRX.ATU_I = ATU_MinSWR_I - 1;
					TRX.ATU_C = 0;
				} else // go slide right
				{
					TRX.ATU_C = 0;
					ATU_MinSWR_Slider = ATU_MinSWR;
					if (TRX.ATU_I < ATU_MAXPOS) {
						TRX.ATU_I = ATU_MinSWR_I + 1;
						ATU_Stage = 4;
					} else {
						ATU_Stage = 5;
					}
				}
				ATU_MinSWR_prev_3 = ATU_MinSWR_prev_2 = ATU_MinSWR_prev_1 = 9.9f;
			}
		} else if (ATU_Stage == 4) // iterate caps with i+1
		{
			if (TRX.ATU_C < ATU_MAXPOS && !wrong_way) {
				TRX.ATU_C++;
			} else {
				// slide more?
				if (ATU_MinSWR < ATU_MinSWR_Slider && TRX.ATU_I < ATU_MAXPOS) {
					ATU_MinSWR_Slider = ATU_MinSWR;
					TRX.ATU_I = ATU_MinSWR_I + 1;
					TRX.ATU_C = 0;
				} else // enough
				{
					ATU_Stage = 5;
				}
				ATU_MinSWR_prev_3 = ATU_MinSWR_prev_2 = ATU_MinSWR_prev_1 = 9.9f;
			}
		}

		if (ATU_Stage == 5) // finish tune
		{
			ATU_InProcess = false;
			ATU_Finished = true;
			TRX.ATU_I = ATU_MinSWR_I;
			TRX.ATU_C = ATU_MinSWR_C;
			TRX.ATU_T = ATU_MinSWR_T;
			println("ATU best I: ", TRX.ATU_I, " C: ", TRX.ATU_C, " T: ", (uint8_t)TRX.ATU_T, " SWR: ", (double)ATU_MinSWR, " PWR: ", (double)TRX_PWR);
			sprintf(buff, "Best SWR: %.1f", (double)ATU_MinSWR);
			LCD_showTooltip(buff);
			delay_stages_count = 0;

			ATU_Save_Memory(TRX_on_TX ? TRX.ANT_TX : TRX.ANT_RX, CurrentVFO->Freq, TRX.ATU_I, TRX.ATU_C, TRX.ATU_T);
			BUTTONHANDLER_TUNE(0);
			ATU_Flush_Memory();
		}

		LCD_UpdateQuery.StatusInfoBar = true;
	}

	ATU_Save_Memory(TRX_on_TX ? TRX.ANT_TX : TRX.ANT_RX, CurrentVFO->Freq, TRX.ATU_I, TRX.ATU_C, TRX.ATU_T);
}

void ATU_Flush_Memory() {
#if !defined(FRONTPANEL_LITE) && !defined(FRONTPANEL_X1)
	if (!ATU_MEMORY_1.saved && ATU_MEMORY_1.loaded_ant == 0) {
		ATU_MEMORY_1.saved = SaveATUSettings((uint8_t *)&ATU_MEMORY_1, sizeof(ATU_MEMORY_1), EEPROM_SECTOR_ATU_1);
	}
	if (!ATU_MEMORY_2.saved && ATU_MEMORY_2.loaded_ant == 1) {
		ATU_MEMORY_2.saved = SaveATUSettings((uint8_t *)&ATU_MEMORY_2, sizeof(ATU_MEMORY_2), EEPROM_SECTOR_ATU_2);
	}
	if (!ATU_MEMORY_3.saved && ATU_MEMORY_3.loaded_ant == 2) {
		ATU_MEMORY_3.saved = SaveATUSettings((uint8_t *)&ATU_MEMORY_3, sizeof(ATU_MEMORY_3), EEPROM_SECTOR_ATU_3);
	}
	if (!ATU_MEMORY_4.saved && ATU_MEMORY_4.loaded_ant == 3) {
		ATU_MEMORY_4.saved = SaveATUSettings((uint8_t *)&ATU_MEMORY_4, sizeof(ATU_MEMORY_4), EEPROM_SECTOR_ATU_4);
	}
#endif
}

void ATU_Load_ANT_Banks(void) {
#if !defined(FRONTPANEL_LITE) && !defined(FRONTPANEL_X1)
	for (uint8_t ant = 0; ant < ANT_MAX_COUNT; ant++) {

		ATU_MEMORY_TYPE *memory = &ATU_MEMORY_1;
		uint8_t sector = EEPROM_SECTOR_ATU_1;
		if (ant == 1) {
			memory = &ATU_MEMORY_2;
			sector = EEPROM_SECTOR_ATU_2;
		}
		if (ant == 2) {
			memory = &ATU_MEMORY_3;
			sector = EEPROM_SECTOR_ATU_3;
		}
		if (ant == 3) {
			memory = &ATU_MEMORY_4;
			sector = EEPROM_SECTOR_ATU_4;
		}

		if (memory->loaded_ant != ant) {
			LoadATUSettings((uint8_t *)memory, sizeof(ATU_MEMORY_TYPE), sector);
			memory->saved = true;

			if (memory->loaded_ant != ant) {
				dma_memset(memory, 0x00, sizeof(ATU_MEMORY_TYPE));
				memory->loaded_ant = ant;
				memory->saved = false;
				ATU_Flush_Memory();
			}
		}
	}
#endif
}

void ResetATUBanks(void) {
#if !defined(FRONTPANEL_LITE) && !defined(FRONTPANEL_X1)
	for (uint8_t ant = 0; ant < ANT_MAX_COUNT; ant++) {
		ATU_MEMORY_TYPE *memory = &ATU_MEMORY_1;
		uint8_t sector = EEPROM_SECTOR_ATU_1;
		if (ant == 1) {
			memory = &ATU_MEMORY_2;
			sector = EEPROM_SECTOR_ATU_2;
		}
		if (ant == 2) {
			memory = &ATU_MEMORY_3;
			sector = EEPROM_SECTOR_ATU_3;
		}
		if (ant == 3) {
			memory = &ATU_MEMORY_4;
			sector = EEPROM_SECTOR_ATU_4;
		}

		println("[ATU] reset ", ant);
		dma_memset(memory, 0x00, sizeof(ATU_MEMORY_TYPE));
		memory->loaded_ant = ant;
		memory->saved = false;
		ATU_Flush_Memory();
	}
#endif
}

void ATU_Save_Memory(uint8_t ant, uint64_t frequency, uint8_t I, uint8_t C, bool T) {
#if !defined(FRONTPANEL_LITE) && !defined(FRONTPANEL_X1)
	if (frequency >= (ATU_MAX_FREQ_KHZ * 1000)) {
		return;
	}

	uint32_t blockIndex = frequency / (TRX.ATU_MEM_STEP_KHZ * 1000);
	if (blockIndex >= ATU_MEM_COUNT) {
		return;
	}

	ATU_MEMORY_TYPE *memory = &ATU_MEMORY_1;
	if (ant == 1) {
		memory = &ATU_MEMORY_2;
	}
	if (ant == 2) {
		memory = &ATU_MEMORY_3;
	}
	if (ant == 3) {
		memory = &ATU_MEMORY_4;
	}

	memory->state[blockIndex].I = I;
	memory->state[blockIndex].C = C;
	memory->state[blockIndex].T = T;
	memory->saved = false;
#endif
}

ATU_MEMORY_STATE ATU_Get_State(uint8_t ant, uint64_t frequency) {
#if !defined(FRONTPANEL_LITE) && !defined(FRONTPANEL_X1)
	if (frequency >= (ATU_MAX_FREQ_KHZ * 1000)) {
		ATU_MEMORY_STATE emptyState = {
		    .I = 0,
		    .C = 0,
		    .T = false,
		};
		return emptyState;
	}

	uint32_t blockIndex = frequency / (TRX.ATU_MEM_STEP_KHZ * 1000);
	if (blockIndex >= ATU_MEM_COUNT) {
		ATU_MEMORY_STATE emptyState = {
		    .I = 0,
		    .C = 0,
		    .T = false,
		};
		return emptyState;
	}

	ATU_MEMORY_TYPE *memory = &ATU_MEMORY_1;
	if (ant == 1) {
		memory = &ATU_MEMORY_2;
	}
	if (ant == 2) {
		memory = &ATU_MEMORY_3;
	}
	if (ant == 3) {
		memory = &ATU_MEMORY_4;
	}

	return memory->state[blockIndex];
#else
	ATU_MEMORY_STATE emptyState = {
	    .I = 0,
	    .C = 0,
	    .T = false,
	};
	return emptyState;
#endif
}

void ATU_Load_Memory(uint8_t ant, uint64_t frequency) {
	ATU_MEMORY_STATE ATU_state = ATU_Get_State(ant, frequency);
	TRX.ATU_I = ATU_state.I;
	TRX.ATU_C = ATU_state.C;
	TRX.ATU_T = ATU_state.T;
}
