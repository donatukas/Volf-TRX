#include "cw.h"
#include "agc.h"
#include "audio_filters.h"
#include "bands.h"
#include "codec.h"
#include "cw_decoder.h"
#include "fpga.h"
#include "front_unit.h"
#include "functions.h"
#include "lcd.h"
#include "main.h"
#include "rf_unit.h"
#include "settings.h"
#include "swr_analyzer.h"
#include "system_menu.h"
#include "trx_manager.h"
#include "usbd_audio_if.h"
#include "vad.h"

volatile bool CW_key_serial = false;
volatile bool CW_old_key_serial = false;
volatile bool CW_key_dot_hard = false;
volatile bool CW_key_dash_hard = false;
volatile uint_fast8_t KEYER_symbol_status = 0; // status (signal or period) of the automatic key symbol
volatile bool CW_Process_Macros = false;
volatile uint8_t CW_In_SSB_applyed = 0; // 0 - unapplied, 1 - old LSB, 2 - old USB
static bool CW_first_symbol_processed = true;

static uint32_t KEYER_symbol_counter = 0; // samples counter of the automatic key character

static float32_t current_cw_power = 0.0f;        // current amplitude (for rise/fall)
static bool iambic_first_button_pressed = false; // start symbol | false - dot, true - dash
static bool iambic_last_symbol = false;          // last Iambic symbol | false - dot, true - dash
static bool iambic_sequence_started = false;

static char *CW_Macros_Message;
static uint32_t CW_current_message_index = 0;
static uint32_t CW_current_symbol_index = 0;
static uint64_t CW_wait_until = 0;
static uint8_t CW_EncodeStatus = 0; // 0 - wait symbol, 1 - transmit symbol, 2 - wait after char, 3 - wait after word

static uint8_t CW_SymbolMemory = 0; // 0 - no char, 1 - dot, 2 - dash

static char *CW_CharToDots(char chr);
static void CW_do_Process_Macros(void);
static float32_t CW_GenerateKeyer(float32_t power, bool prepareBeforeDelay);
static void CW_updateEstimateTimeout(void);

void CW_key_change(void) {
	bool TRX_new_key_dot_hard = !HAL_GPIO_ReadPin(KEY_IN_DOT_GPIO_Port, KEY_IN_DOT_Pin);
	bool TRX_new_key_dash_hard = !HAL_GPIO_ReadPin(KEY_IN_DASH_GPIO_Port, KEY_IN_DASH_Pin);
	if (TRX.CW_Invert) {
		bool tmp = TRX_new_key_dot_hard;
		TRX_new_key_dot_hard = TRX_new_key_dash_hard;
		TRX_new_key_dash_hard = tmp;
	}

	TRX_Inactive_Time = 0;
	if (TRX_Tune || !TRX_Inited) {
		return;
	}

	bool onTx = TRX_on_TX;

	bool notx = TRX_TX_Disabled(CurrentVFO->Freq) && !TRX_REPEATER_Applied;
	if (notx) {
		TRX_ptt_soft = false;
		TRX_ptt_hard = false;
		return;
	}

	if (CW_key_dot_hard != TRX_new_key_dot_hard) {
		CW_key_dot_hard = TRX_new_key_dot_hard;

		// auto CW
		if (CW_key_dot_hard && (TRX.Auto_CW_Mode || TRX.CW_In_SSB) && CurrentVFO->Mode != TRX_MODE_CW && TRX_Inited) {
			if (TRX.CW_In_SSB && CurrentVFO->Mode == TRX_MODE_LSB) {
				TRX_setFrequency(CurrentVFO->Freq - TRX.CW_Pitch, CurrentVFO);
				TRX_setMode(TRX_MODE_CW, CurrentVFO);
				CW_In_SSB_applyed = 1;
			}
			if (TRX.CW_In_SSB && CurrentVFO->Mode == TRX_MODE_USB) {
				TRX_setFrequency(CurrentVFO->Freq + TRX.CW_Pitch, CurrentVFO);
				TRX_setMode(TRX_MODE_CW, CurrentVFO);
				CW_In_SSB_applyed = 2;
			}
			if (TRX.Auto_CW_Mode) {
				TRX_setMode(TRX_MODE_CW, CurrentVFO);
			}

			LCD_UpdateQuery.FreqInfoRedraw = true;
			LCD_UpdateQuery.TopButtons = true;
			LCD_UpdateQuery.StatusInfoGUIRedraw = true;
		}

		// like PTT
		if (CurrentVFO->Mode != TRX_MODE_CW && TRX_Inited) {
			CW_key_dash_hard = false;
			KEYER_symbol_status = 0;

			TRX_ptt_soft = TRX_new_key_dot_hard;
			LCD_UpdateQuery.StatusInfoGUIRedraw = true;
			FPGA_NeedSendParams = true;
			TRX_Restart_Mode();

			return;
		}

		// do tx
		if (CW_key_dot_hard == true && (KEYER_symbol_status == 0 || !TRX.CW_KEYER)) {
			CW_updateEstimateTimeout();
			FPGA_NeedSendParams = true;
			TRX_Restart_Mode();
			if (!onTx && (TRX.CW_PTT_Type == KEY_PTT || TRX.CW_PTT_Type == KEY_AND_EXT_PTT)) {
				CW_GenerateKeyer(0, true);
			}
		}
	}

	if (CW_key_dash_hard != TRX_new_key_dash_hard) {
		CW_key_dash_hard = TRX_new_key_dash_hard;

		// auto CW
		if (CW_key_dash_hard && (TRX.Auto_CW_Mode || TRX.CW_In_SSB) && CurrentVFO->Mode != TRX_MODE_CW && TRX_Inited) {
			if (TRX.CW_In_SSB && CurrentVFO->Mode == TRX_MODE_LSB) {
				TRX_setFrequency(CurrentVFO->Freq - TRX.CW_Pitch, CurrentVFO);
				TRX_setMode(TRX_MODE_CW, CurrentVFO);
				CW_In_SSB_applyed = 1;
			}
			if (TRX.CW_In_SSB && CurrentVFO->Mode == TRX_MODE_USB) {
				TRX_setFrequency(CurrentVFO->Freq + TRX.CW_Pitch, CurrentVFO);
				TRX_setMode(TRX_MODE_CW, CurrentVFO);
				CW_In_SSB_applyed = 2;
			}
			if (TRX.Auto_CW_Mode) {
				TRX_setMode(TRX_MODE_CW, CurrentVFO);
			}

			LCD_UpdateQuery.FreqInfoRedraw = true;
			LCD_UpdateQuery.TopButtons = true;
			LCD_UpdateQuery.StatusInfoGUIRedraw = true;
		}

		// like PTT
		if (CurrentVFO->Mode != TRX_MODE_CW && TRX_Inited) {
			CW_key_dot_hard = false;
			KEYER_symbol_status = 0;

			TRX_ptt_soft = TRX_new_key_dash_hard;
			LCD_UpdateQuery.StatusInfoGUIRedraw = true;
			FPGA_NeedSendParams = true;
			TRX_Restart_Mode();

			return;
		}

		// do tx
		if (CW_key_dash_hard == true && (KEYER_symbol_status == 0 || !TRX.CW_KEYER)) {
			CW_updateEstimateTimeout();
			FPGA_NeedSendParams = true;
			TRX_Restart_Mode();
			if (!onTx && (TRX.CW_PTT_Type == KEY_PTT || TRX.CW_PTT_Type == KEY_AND_EXT_PTT)) {
				CW_GenerateKeyer(0, true);
			}
		}
	}

	// USB\Macros mode
	if (CW_key_serial != CW_old_key_serial) {
		CW_old_key_serial = CW_key_serial;
		CW_updateEstimateTimeout();
		FPGA_NeedSendParams = true;
		TRX_Restart_Mode();
	}
}

static void CW_updateEstimateTimeout(void) {
	if (CW_Key_Timeout_est < TRX.CW_Key_timeout) {
		CW_Key_Timeout_est = TRX.CW_Key_timeout;
	}
}

static float32_t CW_generateRiseSignal(float32_t power) {
	if (current_cw_power < power) {
		current_cw_power += power * CW_EDGES_SMOOTH;
	}
	if (current_cw_power > power) {
		current_cw_power = power;
	}
	return current_cw_power;
}
static float32_t CW_generateFallSignal(float32_t power) {
	if (current_cw_power > 0.0f) {
		current_cw_power -= power * CW_EDGES_SMOOTH;
	}
	if (current_cw_power < 0.0f) {
		current_cw_power = 0.0f;
	}
	return current_cw_power;
}

float32_t CW_GenerateSignal(float32_t power) {
	// Do no signal before start TX delay
	if ((HAL_GetTick() - TRX_TX_StartTime) < CALIBRATE.TX_StartDelay) {
		return 0.0f;
	}

	// Keyer disabled
	if (!TRX.CW_KEYER) {
		KEYER_symbol_status = 0;

		if (!CW_key_serial && !CW_key_dot_hard && !CW_key_dash_hard) {
			return CW_generateFallSignal(power);
		}
		return CW_generateRiseSignal(power);
	}

	// CW Macros
	if (CW_Process_Macros) {
		CW_do_Process_Macros();
	}

	// USB CW (Serial)
	if (CW_key_serial) {
		return CW_generateRiseSignal(power);
	}

	// Keyer
	return CW_GenerateKeyer(power, false);
}

static float32_t CW_GenerateKeyer(float32_t power, bool prepareBeforeDelay) {
	// Keyer
	uint32_t fall_length_samples = TRX_SAMPLERATE / 1000 * TRX.CW_EDGES_SMOOTH_MS;
	uint32_t dot_length_samples = (float32_t)TRX_SAMPLERATE / 1000.0f * (1200.0f / (float32_t)TRX.CW_KEYER_WPM);
	uint32_t dash_length_samples = dot_length_samples * TRX.CW_DotToDashRate;
	uint32_t sim_space_length_samples = dot_length_samples;
	KEYER_symbol_counter++;
	if (prepareBeforeDelay) {
		CW_Key_Timeout_est = TRX.CW_Key_timeout + CALIBRATE.TX_StartDelay;
	}

	// Iambic keyer start mode
	if (!iambic_sequence_started) {
		if (CW_key_dot_hard && !CW_key_dash_hard) {
			iambic_first_button_pressed = false;
		}
		if (!CW_key_dot_hard && CW_key_dash_hard) {
			iambic_first_button_pressed = true;
		}
		if (CW_key_dot_hard && CW_key_dash_hard) {
			iambic_sequence_started = true;
		}
	}

	// Symbol memory
	static bool cw_memory_prev_dot_status = false;
	static bool cw_memory_prev_dash_status = false;

	if (TRX.CW_OneSymbolMemory && KEYER_symbol_status != 0) {
		if (CW_key_dot_hard != cw_memory_prev_dot_status) {
			if (CW_SymbolMemory == 0 && CW_key_dot_hard) { // add dot to memory
				CW_SymbolMemory = 1;
			}
		}

		if (CW_key_dash_hard != cw_memory_prev_dash_status) {
			if (CW_SymbolMemory == 0 && CW_key_dash_hard) { // add dash to memory
				CW_SymbolMemory = 2;
			}
		}
	}

	cw_memory_prev_dot_status = CW_key_dot_hard;
	cw_memory_prev_dash_status = CW_key_dash_hard;

	// DOT .
	if (KEYER_symbol_status == 0 && CW_key_dot_hard) {
		KEYER_symbol_counter = 0;
		KEYER_symbol_status = 1;
	}
	if (KEYER_symbol_status == 1 && KEYER_symbol_counter <= dot_length_samples && KEYER_symbol_counter > (dot_length_samples - fall_length_samples)) {
		CW_updateEstimateTimeout();
		return CW_generateFallSignal(power);
	}
	if (KEYER_symbol_status == 1 && KEYER_symbol_counter <= dot_length_samples) {
		CW_updateEstimateTimeout();
		return CW_generateRiseSignal(power);
	}
	if (KEYER_symbol_status == 1 && KEYER_symbol_counter > dot_length_samples) {
		iambic_last_symbol = false;
		KEYER_symbol_counter = 0;
		KEYER_symbol_status = 3;
	}

	// DASH -
	if (KEYER_symbol_status == 0 && CW_key_dash_hard) {
		KEYER_symbol_counter = 0;
		KEYER_symbol_status = 2;
	}
	if (KEYER_symbol_status == 2 && KEYER_symbol_counter <= dash_length_samples && KEYER_symbol_counter > (dash_length_samples - fall_length_samples)) {
		CW_updateEstimateTimeout();
		return CW_generateFallSignal(power);
	}
	if (KEYER_symbol_status == 2 && KEYER_symbol_counter <= dash_length_samples) {
		CW_updateEstimateTimeout();
		return CW_generateRiseSignal(power);
	}
	if (KEYER_symbol_status == 2 && KEYER_symbol_counter > dash_length_samples) {
		iambic_last_symbol = true;
		KEYER_symbol_counter = 0;
		KEYER_symbol_status = 3;
	}

	// SPACE
	if (KEYER_symbol_status == 3 && KEYER_symbol_counter <= sim_space_length_samples) {
		CW_updateEstimateTimeout();
		return CW_generateFallSignal(power);
	}
	if (KEYER_symbol_status == 3 && KEYER_symbol_counter > sim_space_length_samples) {
		if (!TRX.CW_Iambic) { // classic keyer

			if (TRX.CW_OneSymbolMemory && CW_SymbolMemory != 0) { // memory symbol sequence
				KEYER_symbol_counter = 0;
				KEYER_symbol_status = CW_SymbolMemory;
				CW_SymbolMemory = 0;
			} else { // no symbol in memory, classic mode
				KEYER_symbol_status = 0;
			}

		} else { // iambic keyer

			// stop sequence on mode A
			if (TRX.CW_Iambic_Type == 0 && (!CW_key_dot_hard || !CW_key_dash_hard)) {
				KEYER_symbol_status = 0;
				iambic_sequence_started = false;
			}

			// start iambic sequence
			if (iambic_sequence_started) {
				if (!iambic_last_symbol) // iambic dot . , next dash -
				{
					KEYER_symbol_counter = 0;
					KEYER_symbol_status = 2;
					if (iambic_first_button_pressed && (!CW_key_dot_hard || !CW_key_dash_hard)) // iambic dash-dot sequence compleated
					{
						iambic_sequence_started = false;
						KEYER_symbol_status = 0;
					}
				} else // iambic dash - , next dot .
				{
					KEYER_symbol_counter = 0;
					KEYER_symbol_status = 1;
					if (!iambic_first_button_pressed && (!CW_key_dot_hard || !CW_key_dash_hard)) // iambic dot-dash sequence compleated
					{
						KEYER_symbol_status = 0;
						iambic_sequence_started = false;
					}
				}
			} else { // no sequence, return to classic mode
				KEYER_symbol_status = 0;
			}
		}
	}

	// Else generate falling
	return CW_generateFallSignal(power);
}

void CW_InitMacros(char *new_message) {
	CW_Macros_Message = new_message;

	CW_current_message_index = 0;
	CW_current_symbol_index = 0;
	CW_wait_until = 0;
	CW_EncodeStatus = 0;
}

void CW_do_Process_Macros(void) {
	if (CurrentVFO->Mode != TRX_MODE_CW) {
		CW_key_serial = false;
		CW_Process_Macros = false;
	}

	uint64_t current_time = HAL_GetTick();
	if (current_time < CW_wait_until) {
		return;
	}

	const char *chr = CW_CharToDots(*(CW_Macros_Message + CW_current_message_index));
	char symbol = chr[CW_current_symbol_index];

	if (CW_EncodeStatus == 1) {
		CW_current_symbol_index++;

		if (chr[CW_current_symbol_index] == 0) { // end of word
			CW_current_symbol_index = 0;
			CW_current_message_index++;

			CW_wait_until = current_time + CW_CHAR_SPACE_LENGTH_MS;

			if (CW_Macros_Message[CW_current_message_index] == 0) { // end of message
				CW_wait_until = current_time + CW_WORD_SPACE_LENGTH_MS;
				CW_current_message_index = 0;

				CW_Process_Macros = false;
				CW_key_serial = false;
				TRX_ptt_soft = false;
			}
		} else {
			CW_wait_until = current_time + CW_SYMBOL_SPACE_LENGTH_MS;
		}

		CW_EncodeStatus = 0;
		CW_key_serial = false;

		return;
	}

	if (symbol == '.') {
		CW_wait_until = current_time + CW_DOT_LENGTH_MS;
		CW_key_serial = true;
	}

	if (symbol == '-') {
		CW_wait_until = current_time + CW_DASH_LENGTH_MS;
		CW_key_serial = true;
	}

	if (symbol == ' ') {
		CW_wait_until = current_time + CW_WORD_SPACE_LENGTH_MS;
		CW_key_serial = false;
	}

	if (symbol == 0) {
		CW_key_serial = false;
	}

	CW_EncodeStatus = 1;
}

static char *CW_CharToDots(char chr) {
	if (chr == ' ') {
		return " ";
	}
	if (chr == '_') {
		return " ";
	}
	if (chr == 'A') {
		return ".-";
	}
	if (chr == 'B') {
		return "-...";
	}
	if (chr == 'C') {
		return "-.-.";
	}
	if (chr == 'D') {
		return "-..";
	}
	if (chr == 'E') {
		return ".";
	}
	if (chr == 'F') {
		return "..-.";
	}
	if (chr == 'G') {
		return "--.";
	}
	if (chr == 'H') {
		return "....";
	}
	if (chr == 'I') {
		return "..";
	}
	if (chr == 'J') {
		return ".---";
	}
	if (chr == 'K') {
		return "-.-";
	}
	if (chr == 'L') {
		return ".-..";
	}
	if (chr == 'M') {
		return "--";
	}
	if (chr == 'N') {
		return "-.";
	}
	if (chr == 'O') {
		return "---";
	}
	if (chr == 'P') {
		return ".--.";
	}
	if (chr == 'Q') {
		return "--.-";
	}
	if (chr == 'R') {
		return ".-.";
	}
	if (chr == 'S') {
		return "...";
	}
	if (chr == 'T') {
		return "-";
	}
	if (chr == 'U') {
		return "..-";
	}
	if (chr == 'V') {
		return "...-";
	}
	if (chr == 'W') {
		return ".--";
	}
	if (chr == 'X') {
		return "-..-";
	}
	if (chr == 'Y') {
		return "-.--";
	}
	if (chr == 'Z') {
		return "--..";
	}
	if (chr == '1') {
		return ".----";
	}
	if (chr == '2') {
		return "..---";
	}
	if (chr == '3') {
		return "...--";
	}
	if (chr == '4') {
		return "....-";
	}
	if (chr == '5') {
		return ".....";
	}
	if (chr == '6') {
		return "-....";
	}
	if (chr == '7') {
		return "--...";
	}
	if (chr == '8') {
		return "---..";
	}
	if (chr == '9') {
		return "----.";
	}
	if (chr == '0') {
		return "-----";
	}
	if (chr == '?') {
		return "..--..";
	}
	if (chr == '.') {
		return ".-.-.-";
	}
	if (chr == ',') {
		return "--..--";
	}
	if (chr == '!') {
		return "-.-.--";
	}
	if (chr == '@') {
		return ".--.-.";
	}
	if (chr == ':') {
		return "---...";
	}
	if (chr == ';') {
		return "-.-.-.";
	}
	if (chr == '-') {
		return "-....-";
	}
	if (chr == '/') {
		return "-..-.";
	}
	if (chr == '(') {
		return "-.--.";
	}
	if (chr == ')') {
		return "-.--.-";
	}
	if (chr == '$') {
		return "...-..-";
	}
	if (chr == '+') {
		return ".-.-.";
	}
	if (chr == '~') {
		return "...-.";
	}
	if (chr == '\'') {
		return ".----.";
	}
	if (chr == '"') {
		return ".-..-.";
	}
	if (chr == '&') {
		return ".-...";
	}
	if (chr == '=') {
		return "-...-";
	}

	return " ";
}
