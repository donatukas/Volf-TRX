#ifndef CW_H
#define CW_H

#include "hardware.h"
#include "settings.h"
#include <stdbool.h>

#define CW_EDGES_SMOOTH (1.0f / (float32_t)(TRX_SAMPLERATE / 1000 * TRX.CW_EDGES_SMOOTH_MS))
#define CW_DOT_LENGTH_MS (1200.0f / (float32_t)TRX.CW_KEYER_WPM)
#define CW_DASH_LENGTH_MS (CW_DOT_LENGTH_MS * (float32_t)TRX.CW_DotToDashRate)
#define CW_SYMBOL_SPACE_LENGTH_MS CW_DOT_LENGTH_MS
#define CW_CHAR_SPACE_LENGTH_MS (3.0f * CW_DOT_LENGTH_MS)
#define CW_WORD_SPACE_LENGTH_MS (4.0f * CW_DOT_LENGTH_MS) // after char delay

extern void CW_key_change(void);
extern float32_t CW_GenerateSignal(float32_t power);
extern void CW_InitMacros(char *new_message);

volatile extern bool CW_key_serial;
volatile extern bool CW_old_key_serial;
volatile extern bool CW_key_dot_hard;
volatile extern bool CW_key_dash_hard;
volatile extern uint_fast8_t KEYER_symbol_status;
volatile extern bool CW_Process_Macros;
volatile extern uint8_t CW_In_SSB_applyed;

#endif
