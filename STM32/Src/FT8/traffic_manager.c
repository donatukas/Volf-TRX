#include "traffic_manager.h"
#include "FT8_GUI.h"
#include "FT8_main.h"
#include "atu.h"
#include "audio_processor.h"
#include "decode_ft8.h"
#include "gen_ft8.h"
#include "lcd.h"
#include "lcd_driver.h"
#include "rf_unit.h"
#include "sd.h"
#include "trx_manager.h"
#include "wifi.h"
#include <stdint.h>

uint16_t cursor_freq;  // the AF frequency wich will be tansmited now (roughly from 0 to 3kHz)
uint64_t FT8_BND_Freq; // frequency for the FT8 on the current Band
int xmit_flag, ft8_xmit_counter;

#define FT8_TONE_SPACING 6.25f

extern uint16_t cursor_line;

int Beacon_State; //
extern int num_decoded_msg;
uint32_t F_Offset;
char QSODate[9];    // string containing the potential QSO date
char QSOOnTime[7];  // potential QSO Start time
char QSOOffTime[7]; // potential QSO Stop time

void transmit_sequence(void) {
	Set_Data_Colection(0);                    // Disable the data colection
	set_Xmit_Freq(FT8_BND_Freq, cursor_freq); // Set band frequency and the frequency in the FT8 (cursor freq.)
	ATU_SetCompleted();
	TRX.Full_Duplex = false;
	TRX.TWO_SIGNAL_TUNE = false;
	TRX_Tune = true;
	TRX_Restart_Mode();
}

void receive_sequence(void) {
	Set_Data_Colection(0);          // Disable the data colection (it will be enabled by next 15s marker)
	xmit_flag = 0;                  // disable if transmit was activeted
	set_Xmit_Freq(FT8_BND_Freq, 0); // Set band frequency and the frequency in the FT8 (cursor freq.)
	TRX_Tune = false;
	TRX_Restart_Mode();
}

void tune_On_sequence(void) {
	Set_Data_Colection(0);                    // Disable the data colection
	set_Xmit_Freq(FT8_BND_Freq, cursor_freq); // Set band frequency and the frequency in the FT8 (cursor freq.)
	BUTTONHANDLER_TUNE(0);
}

void tune_Off_sequence(void) {
	Set_Data_Colection(0);          // Disable the data colection (it will be enabled by next 15s marker)
	set_Xmit_Freq(FT8_BND_Freq, 0); // Set band frequency and the frequency in the FT8 (cursor freq.)
	BUTTONHANDLER_TUNE(255);
}

void set_Xmit_Freq(uint64_t BandFreq, float32_t Freq) {
	uint64_t F_Long = BandFreq * 1000; // BandFreq is in kHz
	APROC_TUNE_DigiTone_Freq = Freq;
	if (CurrentVFO->Freq != F_Long) {
		TRX_setFrequency(F_Long, CurrentVFO);
	}
	if (CurrentVFO->Mode != TRX_MODE_DIGI_U) {
		TRX_setMode(TRX_MODE_DIGI_U, CurrentVFO);
	}
}

void set_FT8_Tone(char ft8_tone) {
	float32_t F_FT8 = ft8_tone * FT8_TONE_SPACING;
	set_Xmit_Freq(FT8_BND_Freq, (float32_t)cursor_freq + F_FT8);
}

void setup_to_transmit_on_next_DSP_Flag(void) {
	ft8_xmit_counter = 0;
	transmit_sequence();
	// Set band frequency and the frequency in the FT8 (cursor freq.)
	xmit_flag = 1;
}

void service_CQ(void) {
#define MaxAttempt_Tries 3 // The Attempt tries to send the same mesage (if reached => jump to "CQ" call)
	int receive_index;
	static uint8_t Attempt_Count = 0; // Count the attempts to send the same message

	char ctmp[20] = {0}; // Debug

	receive_index = Check_Calling_Stations(num_decoded_msg); // index of the station that called us (last if we got more calls)

	if (receive_index == -1) // if we didn't recieved mesage calling us
	{
		if (Beacon_State > 1) // if conversation was initiated
		{
			if (Beacon_State == 5) { // we just started the "CQ Answer" call
				Attempt_Count = 0;     // set the Attempt_Count (the call is already done once - initialised by pressing the button in "FT8_GUI.c")
			}

			Attempt_Count++;
			if (Attempt_Count < MaxAttempt_Tries) {
				if (Beacon_State == 2) {
					Beacon_State = 20; // Repeat the "report signal" call
				} else if (Beacon_State == 3) {
					Beacon_State = 30; // Repeat the "73" call
				} else if (Beacon_State == 5) {
					Beacon_State = 50; // Repeat the "CQ Answer" call
				}
			} else // We tried enough without answer => go back to "CQ" call
			{
				FT8_Clear_TargetCall(); // Clear the place on the display for the "TargetCall"
				Attempt_Count = 0;

				if (TRX.FT8_Auto_CQ) {
					Beacon_State = 1; // Set Call - "CQ"
				} else {
					Beacon_State = 0;
					FT8_Menu_Idx = 0;      // index of the "CQ" button
					FT8_Menu_Pos_Toggle(); // deactivate the "CQ" button -> set green
				}
			}
		} else {            // The conversation was not initiated => then we just call CQ
			Beacon_State = 1; // Set Call - "CQ"
		}
	} // if(receive_index == -1) - we didn't recieved mesage calling us

	else if (receive_index >= 0) // we did recieve a message calling us
	{
		if (Beacon_State == 1) // if previous state was "call CQ" -> then we got an answer -> report the signal level
		{
			GetQSOTime(1);                                        // Record the QSO Start Time
			Beacon_State = 2;                                     // Set call - report signal
			Attempt_Count = 0;                                    // zero the attempt counter
		} else if ((Beacon_State == 2) || (Beacon_State == 20)) // if previous state was "report signal" ->
		{
			receive_index = FindPartnerIDX(num_decoded_msg); // find the index of the station that we were talking till now
			if (receive_index >= 0) {
				Attempt_Count = 0;                              // zero the attempt counter
				if (CheckRecievedRaportRSL(receive_index, 0)) { // check if the oposite side answered corespondingly
					Beacon_State = 3;                             // Set call - "RR73"
				} else if (CheckRecieved73(receive_index, 1))   // if RR73 after raport
				{
					Beacon_State = 7; // Set call - "73"
					LogQSO();
					FT8_Clear_TargetCall(); // Clear the place on the display for the "TargetCall"
				}
			} else { // The answer we got it was not from the station we were talking till now
				Beacon_State = 20;
			}
		} else if ((Beacon_State == 3) || (Beacon_State == 30)) // if previous state was call - "RR73"
		{
			receive_index = FindPartnerIDX(num_decoded_msg); // find the index of the station that we were talking till now
			if (receive_index >= 0) {
				Attempt_Count = 0;                     // zero the attempt counter
				if (CheckRecieved73(receive_index, 0)) // check if the oposite side answered corespondingly
				{
					LogQSO();
					FT8_Clear_TargetCall(); // Clear the place on the display for the "TargetCall"

					if (TRX.FT8_Auto_CQ) {
						Beacon_State = 1; // if yes then we are done and can go further (call next "CQ")
					} else {
						Beacon_State = 0;
						FT8_Menu_Idx = 0;      // index of the "CQ" button
						FT8_Menu_Pos_Toggle(); // deactivate the "CQ" button -> set green
					}
				}
			} else { // The answer we got it was not from the station we were talking till now
				Beacon_State = 30;
			}
		}
		// end the CALL initiated by us
		//------------------------------------------------------
		// The CALL initiated by oposite side (we answer their CQ)
		else if ((Beacon_State == 5) || (Beacon_State == 50)) // if previous state was "answer CQ"
		{
			receive_index = FindPartnerIDX(num_decoded_msg); // find the index of the station that we were talking till now
			if (receive_index >= 0) {
				Attempt_Count = 0;                            // zero the attempt counter
				if (CheckRecievedRaportRSL(receive_index, 1)) // check if the oposite side answered corespondingly
				{
					Beacon_State = 6; // Send RSL
					GetQSOTime(1);    // Record the QSO Start Time
				}
			} else {             // The answer we got it was not from the station we were talking till now
				Beacon_State = 50; // Try again
			}
		} else if (Beacon_State == 6) // if previous state was send RSL
		{
			receive_index = FindPartnerIDX(num_decoded_msg); // find the index of the station that we were talking till now
			if (receive_index >= 0) {
				Attempt_Count = 0;                     // zero the attempt counter
				if (CheckRecieved73(receive_index, 1)) // check if the oposite side answered corespondingly
				{
					Beacon_State = 7; // Set call - "73"
					LogQSO();
				}
			} else {            // The answer we got it was not from the station we were talking till now
				Beacon_State = 6; // Try again
			}
		}
	}

	//!
	//! Need to check all loops if everything makes sence
	//!

	// Debug
	sprintf(ctmp, "Beacon_State: %d ", Beacon_State);
#if (defined(LAY_320x240))
	LCDDriver_printText(ctmp, 241, 260, COLOR_GREEN, COLOR_BLACK, 1);
#else
	LCDDriver_printText(ctmp, 241, 260, COLOR_GREEN, COLOR_BLACK, 2);
#endif

	switch (Beacon_State) {
	// CALL initiated by us
	case 1:           // Send CQ
		set_message(0); // send CQ
		setup_to_transmit_on_next_DSP_Flag();
		break;

	case 2:                             // send RSL
		SetNew_TargetCall(receive_index); // put in to the buffers the "Target_Call" and the "Target_Grid" (the index of the message who answered us)
		FT8_Print_TargetCall();
		set_message(2); // send RSL
		setup_to_transmit_on_next_DSP_Flag();
		break;

	case 3:           // send 73
		set_message(3); // send 73
		setup_to_transmit_on_next_DSP_Flag();
		break;

	case 20:          // repat send RSL
		set_message(2); // send RSL
		setup_to_transmit_on_next_DSP_Flag();
		break;

	case 30:          // repeat send 73
		set_message(3); // send 73
		setup_to_transmit_on_next_DSP_Flag();
		break;
		// end CALL initiated by us

		//------------------------------------------

		// The CALL initiated by oposite side

	case 6: // send a raport to a "CQ answer"
		// SetNew_TargetCall(receive_index);
		set_message(4); // send a raport to a "CQ answer"
		setup_to_transmit_on_next_DSP_Flag();
		break;

	case 7:           // send a "73" to a "CQ answer"
		set_message(5); // send a 73 to a "CQ answer"
		setup_to_transmit_on_next_DSP_Flag();
		Beacon_State = 8; // Disable the CQ after the mesage is transmited - see in the "FT8_main.c"
		break;

	case 50:          // repat the send "answer CQ"
		set_message(1); // send answer a "CQ"
		setup_to_transmit_on_next_DSP_Flag();
		break;
	}
}

void GetQSODate(void) {
	RTC_DateTypeDef sDate = {0};
	RTC_TimeTypeDef sTime = {0};
	getUTCDateTime(&sDate, &sTime);

	sprintf(QSODate, "20%02d%02d%02d", sDate.Year, sDate.Month, sDate.Date);
}

// "QSO_Start" flag showig to take the QSO "Start" or "Stop" time
void GetQSOTime(uint8_t QSO_Start) {
	RTC_DateTypeDef sDate = {0};
	RTC_TimeTypeDef sTime = {0};
	getUTCDateTime(&sDate, &sTime);

	if (sTime.Seconds == 60) { // Fix the seconds
		sTime.Seconds = 0;
	}

	if (QSO_Start == 1) {
		sprintf(QSOOnTime, "%02d%02d%02d", sTime.Hours, sTime.Minutes, sTime.Seconds);
	} else {
		sprintf(QSOOffTime, "%02d%02d%02d", sTime.Hours, sTime.Minutes, sTime.Seconds);
	}
}

void LogQSO(void) {
	static char StrToLog[260];
	char cBND[5]; // for the strng containing the current band

	switch (FT8_BND_Freq) {
	case FT8_Freq_160M:
		strcpy(cBND, "160m");
		break;
	case FT8_Freq_80M:
		strcpy(cBND, "80m");
		break;
	case FT8_Freq_60M:
		strcpy(cBND, "60m");
		break;
	case FT8_Freq_40M:
		strcpy(cBND, "40m");
		break;
	case FT8_Freq_30M:
		strcpy(cBND, "30m");
		break;
	case FT8_Freq_20M:
		strcpy(cBND, "20m");
		break;
	case FT8_Freq_17M:
		strcpy(cBND, "17m");
		break;
	case FT8_Freq_15M:
		strcpy(cBND, "15m");
		break;
	case FT8_Freq_12M:
		strcpy(cBND, "12m");
		break;
	case FT8_Freq_10M:
		strcpy(cBND, "10m");
		break;
	case FT8_Freq_6M:
		strcpy(cBND, "6m");
		break;
	case FT8_Freq_2M:
		strcpy(cBND, "2m");
		break;
	case FT8_Freq_70CM:
		strcpy(cBND, "70cm");
		break;
	case BANDID_QO100:
		strcpy(cBND, "3cm");
		break;
	}

	float64_t QSO_Freq = ((float64_t)FT8_BND_Freq + (float64_t)cursor_freq / 1000) / 1000; // Calculate the QSO Frequency in MHz (for example 7.075500)
	char sQSO_Freq[12];
	sprintf(sQSO_Freq, "%1.6f", (float64_t)QSO_Freq);

	GetQSODate();  // Get the date to be able to put it in the Log later
	GetQSOTime(0); // End Time

	// example message
	//<call:5>M0JJF <gridsquare:4>IO91 <mode:3>FT8 <rst_sent:3>-21 <rst_rcvd:3>-18 <qso_date:8>20210403 <time_on:6>090500
	//<qso_date_off:8>20210403 <time_off:6>090821 <band:3>40m <freq:8>7.075500 <station_callsign:5>DB5AT <my_gridsquare:6>JN48ov <eor>

	char RapRcv_RSL_filtered[5] = {0};
	if (RapRcv_RSL[0] == 'R') {
		strcpy(RapRcv_RSL_filtered, RapRcv_RSL + 1);
	} else {
		strcpy(RapRcv_RSL_filtered, RapRcv_RSL);
	}

	if (SD_Present) {
		sprintf(StrToLog,
		        "<call:%d>%s <gridsquare:4>%s <mode:3>FT8 <rst_sent:3>%3i <rst_rcvd:%d>%s <qso_date:8>%s <time_on:6>%s <qso_date_off:8>%s "
		        "<time_off:6>%s <band:%d>%s <freq:%d>%s <station_callsign:%d>%s <my_gridsquare:6>%s <eor>\r\n",
		        strlen(Target_Call), Target_Call, Target_Grid, Target_RSL, strlen(RapRcv_RSL_filtered), RapRcv_RSL_filtered, QSODate, QSOOnTime, QSODate, QSOOffTime, strlen(cBND), cBND,
		        strlen(sQSO_Freq), sQSO_Freq, strlen(TRX.CALLSIGN), TRX.CALLSIGN, TRX.LOCATOR);

		strcpy((char *)SD_workbuffer_A, "FT8_QSO_Log.adi"); // File name
		strcpy((char *)SD_workbuffer_B, (char *)StrToLog);  // Data to write
		SDCOMM_WRITE_TO_FILE_partsize = strlen(StrToLog);

		SD_doCommand(SDCOMM_WRITE_TO_FILE, false);
		FT8_QSO_Count_needUpdate = true;
	}

	static char cNote[32] = {0};
	sprintf(cNote, "%1.6f Loc: %s", (float64_t)QSO_Freq, Target_Grid);
	static char cTarget_RSL[16] = {0};
	sprintf(cTarget_RSL, "%3i", Target_RSL);
	WIFI_postQSOtoAllQSO(Target_Call, cNote, QSODate, QSOOffTime, cTarget_RSL, RapRcv_RSL_filtered, "FT8", cBND, "", TRX.LOCATOR);
}