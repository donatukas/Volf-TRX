#include "FT8_main.h"
#include "FT8_GUI.h"
#include "Process_DSP.h"
#include "arm_math.h"
#include "constants.h"
#include "decode_ft8.h"
#include "lcd.h"
#include "lcd_driver.h"
#include "locator_ft8.h"
#include "sd.h"
#include "traffic_manager.h"
#include <stdbool.h>

#define ft8_shift 6.25

int ft8_flag, FT_8_counter, ft8_marker, decode_flag, WF_counter;
int num_decoded_msg = 0;
int DSP_Flag;
uint16_t cursor_line = 100;
uint16_t FT8_DatBlockNum;
bool FT8_DecodeActiveFlg;
bool FT8_ColectDataFlg;
bool FT8_QSO_Count_needUpdate = true;
uint32_t FT8_QSO_Count = 0;

static uint64_t Lastfreq = 0;
static uint_fast8_t Lastmode = 0;

// Function prototypes
static void process_data(void);
static void update_synchronization(void);
static void FT8_getQSOCount(void);

void InitFT8_Decoder(void) {
	if (LCD_busy) {
		return;
	}
	LCD_busy = true; //

	// save prev TRX data
	if (Lastfreq == 0) {
		Lastfreq = CurrentVFO->Freq;
		Lastmode = CurrentVFO->Mode;
	}

	// draw the GUI
	LCDDriver_Fill(COLOR_BLACK);

	LCDDriver_printText("FT-8 Decoder", 100, 5, COLOR_GREEN, COLOR_BLACK, 2);

	init_DSP();
	initalize_constants();

	FT8_DatBlockNum = 0;
	FT8_DecodeActiveFlg = true;

	FT8_QSO_Count_needUpdate = true;
	update_synchronization();
	set_Station_Coordinates(TRX.LOCATOR);

	println("FT8-Initialised"); // Debug

	Unarm_FT8_Buttons(); // deactivate all buttons (if something is active and we reenter the "FT8 decode"- to start clear)
	Draw_FT8_Buttons();
	FT8_Menu_Idx = 0;         // index of the "CQ" button
	Update_FT8_Menu_Cursor(); // show the menu cursor

	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	if (band == BANDID_160m) { // 160m
		FT8_BND_Freq = FT8_Freq_160M;
	}
	if (band == BANDID_80m) { // 80m
		FT8_BND_Freq = FT8_Freq_80M;
	}
	if (band == BANDID_60m) { // 60m
		FT8_BND_Freq = FT8_Freq_60M;
	}
	if (band == BANDID_40m) { // 40m
		FT8_BND_Freq = FT8_Freq_40M;
	}
	if (band == BANDID_30m) { // 30m
		FT8_BND_Freq = FT8_Freq_30M;
	}
	if (band == BANDID_20m) { // 20m
		FT8_BND_Freq = FT8_Freq_20M;
	}
	if (band == BANDID_17m) { // 17m
		FT8_BND_Freq = FT8_Freq_17M;
	}
	if (band == BANDID_15m) { // 15m
		FT8_BND_Freq = FT8_Freq_15M;
	}
	if (band == BANDID_12m) { // 12m
		FT8_BND_Freq = FT8_Freq_12M;
	}
	if (band == BANDID_10m) { // 10m
		FT8_BND_Freq = FT8_Freq_10M;
	}
	if (band == BANDID_6m) { // 6m
		FT8_BND_Freq = FT8_Freq_6M;
	}
	if (band == BANDID_2m) { // 2m
		FT8_BND_Freq = FT8_Freq_2M;
	}
	if (band == BANDID_70cm) { // 70cm
		FT8_BND_Freq = FT8_Freq_70CM;
	}
	if (band == BANDID_QO100) { // QO-100
		FT8_BND_Freq = FT8_Freq_QO_100;
	}

	receive_sequence();
	cursor_freq = (uint16_t)((float)(cursor_line + ft8_min_bin) * (float32_t)ft8_shift);
	FT8_Print_Freq();

	Set_Data_Colection(0); // Disable the data colection
	LCD_busy = false;
}

void DeInitFT8_Decoder(void) {
	FT8_DecodeActiveFlg = false;

	TRX_setFrequency(Lastfreq, CurrentVFO);
	TRX_setMode(Lastmode, CurrentVFO);
	Lastfreq = 0;
}

void MenagerFT8(void) {
	char ctmp[20] = {0};

	if (decode_flag == 0) {
		process_data();
	}

	if (LCD_busy) {
		return;
	}
	LCD_busy = true;

	if (DSP_Flag == 1) {
		if (!TRX_Tune) {
			// println("process_FT8_FFT");
			process_FT8_FFT();
		}

		if (xmit_flag == 1) {
			//__disable_irq(); // Disable all interrupts
			int offset_index = 5;
			// 79
			if (ft8_xmit_counter >= offset_index && ft8_xmit_counter < 79 + offset_index) {
				set_FT8_Tone(tones[ft8_xmit_counter - offset_index]);
			}

			ft8_xmit_counter++;

			// Debug
			sprintf(ctmp, "ft8_xmit_c: %d ", ft8_xmit_counter);
#if (defined(LAY_320x240))
			LCDDriver_printText(ctmp, 10, 65, COLOR_GREEN, COLOR_BLACK, 1);
#else
			LCDDriver_printText(ctmp, 10, 65, COLOR_GREEN, COLOR_BLACK, 2);
#endif

			bool send_message_done = false;
			// 80
			if (ft8_xmit_counter == 80 + offset_index) {
				send_message_done = true;
			}

			uint32_t Time = RTC->TR;
			uint8_t Seconds = ((Time >> 4) & 0x07) * 10 + ((Time >> 0) & 0x0f);
			if (ft8_xmit_counter > offset_index && (Seconds == 14 || Seconds == 29 || Seconds == 44 || Seconds == 59)) { // 15s marker
				send_message_done = true;
			}

			if (send_message_done) // send mesage is done!
			{
				// xmit_flag = 0;
				receive_sequence();

				if (Beacon_State == 8) // if we are on the end of answering a "CQ" (we just send "73")
				{
					if (TRX.FT8_Auto_CQ) {
						Beacon_State = 0;
					} else {
						FT8_Menu_Idx = 0;      // index of the "CQ" button
						FT8_Menu_Pos_Toggle(); // deactivate the "CQ" button -> set green
					}
				}
			}
			//__enable_irq(); // Re-enable all interrupts
		}

		DSP_Flag = 0;
	}

	else if (decode_flag == 1) {
		//__disable_irq(); // Disable all interrupts

		FT8_Clear_Mess_Field(); // Clear the recieved mesages field

		num_decoded_msg = 0;
		//			FT8_ColectDataFlg = false;		//Stop Data colection
		//			FT8_Bussy = true;							//FT8 Decode => busy
		Set_Data_Colection(0); // Disable the data colection

		uint32_t decode_srat_time = HAL_GetTick();
		num_decoded_msg = ft8_decode();

		// Debug
		sprintf(ctmp, "Decoded: %d ", num_decoded_msg);
#if (defined(LAY_320x240))
		LCDDriver_printText(ctmp, 10, 45, COLOR_GREEN, COLOR_BLACK, 1);
#else
		LCDDriver_printText(ctmp, 10, 45, COLOR_GREEN, COLOR_BLACK, 2);
#endif
		println("Decode time, ms: ", HAL_GetTick() - decode_srat_time)

		    decode_flag = 0;

		Service_FT8();
		//			FT8_Bussy = false;

		// __enable_irq(); // Re-enable all interrupts
	}

	update_synchronization();
	LCD_busy = false; //
}

static void process_data(void) {
	if (FT8_DatBlockNum >= num_que_blocks) {
		//    for (int i = 0; i<block_size*(FT8_DatBlockNum/8); i++) {
		//			input_gulp[i] = AudioBuffer_for_FT8[i];		//coppy to FFT buffer
		//			AudioBuffer_for_FT8[i] = 0; 		//	and empty the buffer
		//		}
		FT8_DatBlockNum = 0;

		// println("Prepare new Data!");	//Debug
		for (int i = 0; i < input_gulp_size; i++) {
			dsp_buffer[i] = dsp_buffer[i + input_gulp_size];
			dsp_buffer[i + input_gulp_size] = dsp_buffer[i + 2 * input_gulp_size];
			dsp_buffer[i + 2 * input_gulp_size] = input_gulp[i];
		}
		DSP_Flag = 1;
	}
}

// update the syncronisation and show the time
static void update_synchronization(void) {
	char ctmp[30] = {0};

	static uint8_t Seconds_Old;

	RTC_TimeTypeDef sTime = {0};
	RTC_DateTypeDef sDate = {0};
	getLocalDateTime(&sDate, &sTime);
	uint8_t Hours = sTime.Hours;
	uint8_t Minutes = sTime.Minutes;
	uint8_t Seconds = sTime.Seconds;

	if (Seconds >= 60) {      // Fix the seconds
		Seconds = Seconds - 60; // it is efect of the time correction
	}

	if (Seconds_Old != Seconds) // update the time on screen only when change
	{
		sprintf(ctmp, "%02d:%02d:%02d", Hours, Minutes, Seconds);
#if (defined(LAY_800x480))
		LCDDriver_printText(ctmp, 680, 5, COLOR_WHITE, COLOR_BLACK, 2);

#elif (defined(LAY_320x240))
		LCDDriver_printText(ctmp, 260, 5, COLOR_WHITE, COLOR_BLACK, 1);

#else
		LCDDriver_printText(ctmp, 360, 5, COLOR_WHITE, COLOR_BLACK, 2);
#endif

		// TX parameters
		sprintf(ctmp, "SWR: %.1f, PWR: %.1fW    ", (double)TRX_SWR, ((double)TRX_PWR_Forward - (double)TRX_PWR_Backward));
#if (defined(LAY_800x480))
		LCDDriver_printText(ctmp, 235, 400, FG_COLOR, BG_COLOR, 2);
#elif (defined(LAY_320x240))
		LCDDriver_printText(ctmp, 180, 215, FG_COLOR, BG_COLOR, 1);
#else
		LCDDriver_printText(ctmp, 225, 280, FG_COLOR, BG_COLOR, 2);
#endif

		sprintf(ctmp, "TEMP:% 2d, QSO: %d  ", (int16_t)TRX_RF_Temperature, FT8_QSO_Count);
#if (defined(LAY_800x480))
		LCDDriver_printText(ctmp, 235, 420, FG_COLOR, BG_COLOR, 2);
#elif (defined(LAY_320x240))
		// LCDDriver_printText(ctmp, 180, 225, FG_COLOR, BG_COLOR, 1);
#else
		LCDDriver_printText(ctmp, 225, 300, FG_COLOR, BG_COLOR, 2);
#endif

		Seconds_Old = Seconds;
	}

	if ((ft8_flag == 0 && Seconds % 15 <= 1) && (decode_flag == 0)) // 15s marker
	{
		Set_Data_Colection(1); // Set new data colection
	}

	if (FT8_QSO_Count_needUpdate) {
		FT8_getQSOCount();
	}
}

// analyzer events to the encoder
void FT8_EncRotate(int8_t direction) {
	if (TRX_on_TX) {
		if (direction > 0 || TRX.RF_Gain > 0) {
			TRX.RF_Gain += direction;
		}
		if (TRX.RF_Gain > 100) {
			TRX.RF_Gain = 100;
		}

		int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
		if (band >= 0) {
			TRX.BANDS_SAVED_SETTINGS[band].RF_Gain = TRX.RF_Gain;
		}

		return;
	}

	if (LCD_busy) {
		return;
	}
	LCD_busy = true;

	cursor_line += direction;

	if (cursor_line < 1) {
		cursor_line = 1;
	}
	if (cursor_line > ft8_buffer - 50) {
		cursor_line = ft8_buffer - 50;
	}

	cursor_freq = (uint16_t)((float)(cursor_line + ft8_min_bin) * (float32_t)ft8_shift);

	if (TRX_Tune) {
		set_Xmit_Freq(FT8_BND_Freq, cursor_freq);
	}

	FT8_Print_Freq(); // Print the new frequency

	LCD_busy = false;
}

void FT8_Enc2Rotate(int8_t direction) {

	if (LCD_busy) {
		return;
	}
	LCD_busy = true;

	Enc2Rotate_Menager(direction, num_decoded_msg);

	LCD_busy = false;
}

void FT8_Enc2Click(void) {
	if (LCD_busy) {
		return;
	}
	LCD_busy = true;

	FT8_Menu_Pos_Toggle();

	LCD_busy = false;
}

void FT8_getQSOCount_callback(uint32_t count) { FT8_QSO_Count = count; }

static void FT8_getQSOCount(void) {
	if (SD_Present && !SD_CommandInProcess) {
		strcpy((char *)SD_workbuffer_A, "FT8_QSO_Log.adi"); // File name
		SDCOMM_GET_LINES_COUNT_callback = FT8_getQSOCount_callback;

		SD_doCommand(SDCOMM_GET_LINES_COUNT, false);
		FT8_QSO_Count_needUpdate = false;
	}
}
