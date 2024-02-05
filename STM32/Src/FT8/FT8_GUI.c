#include "FT8_GUI.h"
#include "FT8_main.h"
#include "decode_ft8.h"
#include "gen_ft8.h"
#include "lcd_driver.h"
#include "traffic_manager.h"

uint8_t FT8_Menu_Idx;
static uint8_t Sel_Mess_Idx; // Selected recieved mesage index (used when we want to answer somebodey)

static ButtonStruct sButtonData[] = {{/*text*/ " CQ ", // button 0
                                      /*state*/ false},

                                     {/*text*/ "TUNE", // button 1
                                      /*state*/ false},

                                     {/*text*/ "RT_C", // button 2
                                      /*state*/ false}

};

void Draw_FT8_Buttons(void) {
	drawButton(0);
	drawButton(1);
	drawButton(2);
}

// used to deactivate all buttons (if something is active and we reenter the "FT8 decode"- to start clear)
void Unarm_FT8_Buttons(void) {
	for (uint8_t i = 0; i <= FT8_Menu_Max_Idx; i++) {
		sButtonData[i].state = false;
	}
}

void drawButton(uint16_t i) {
	// char ctmp[5] = {0};

	uint16_t color;
	if (sButtonData[i].state) {
		color = COLOR_RED;
	} else {
		color = COLOR_GREEN;
	}

#if (defined(LAY_320x240))
	LCDDriver_printText(sButtonData[i].text, (FT8_button_spac_x * i) + 7, FT8_button_line + 7, COLOR_WHITE, COLOR_BLACK, 1);
#else
	LCDDriver_printText(sButtonData[i].text, (FT8_button_spac_x * i) + 7, FT8_button_line + 7, COLOR_WHITE, COLOR_BLACK, 2);
#endif
	LCDDriver_drawRectXY(FT8_button_spac_x * i, FT8_button_line, (FT8_button_spac_x * i) + FT8_button_width, FT8_button_line + FT8_button_height, color);
}

void Update_FT8_Menu_Cursor(void) {
	static uint8_t old_Sel_Mess_Idx;

	// Clear old Button cursor (all posible cursors)
	LCDDriver_drawLine(0, FT8_button_line - 5, (FT8_Menu_Max_Idx * FT8_button_spac_x) + FT8_button_width, FT8_button_line - 5, COLOR_BLACK);

	// Clear the old recieved mesage cursor
#if (defined(LAY_320x240))
	LCDDriver_drawLine(10, 112 + (old_Sel_Mess_Idx)*18, 100, 112 + (old_Sel_Mess_Idx)*18, COLOR_BLACK);
#else
	LCDDriver_drawLine(10, 115 + (old_Sel_Mess_Idx)*25, 70, 115 + (old_Sel_Mess_Idx)*25, COLOR_BLACK);
#endif

	if (FT8_Menu_Idx <= FT8_Menu_Max_Idx) // cursor is in the range of the buttons
	{
		// LCDDriver_drawLine(10, 115, 70, 115, COLOR_BLACK);		//clear the cursor of the first recieved mesage (it is needed by transition between
		// the two fields)
		LCDDriver_drawLine(10, 115, 70, 115,
		                   COLOR_BLACK); // clear the cursor of the last recieved mesage (it is needed by transition between the two fields)
		// Draw the new position of the cursor (for buttons)
		LCDDriver_drawLine(FT8_Menu_Idx * FT8_button_spac_x, FT8_button_line - 5, (FT8_Menu_Idx * FT8_button_spac_x) + FT8_button_width, FT8_button_line - 5, COLOR_WHITE);
	} else // cursor is in the rage of the recieved mesages
	{
		Sel_Mess_Idx = FT8_Menu_Idx - (FT8_Menu_Max_Idx + 1);
		/*
		      //Clear the old recieved mesage cursor
		      if(Sel_Mess_Idx > 0)
		        LCDDriver_drawLine(10, 115 + (Sel_Mess_Idx-1) *25, 70, 115 + (Sel_Mess_Idx-1) *25, COLOR_BLACK);
		      if(Sel_Mess_Idx < 5)				//max number of messages - 1 => we are on the before last position
		        LCDDriver_drawLine(10, 115 + (Sel_Mess_Idx+1) *25, 70, 115 + (Sel_Mess_Idx+1) *25, COLOR_BLACK);
		*/
		// Draw the new position of the cursor (recieved messages)
#if (defined(LAY_320x240))
		LCDDriver_drawLine(10, 112 + Sel_Mess_Idx * 18, 100, 112 + Sel_Mess_Idx * 18, COLOR_WHITE);
#else
		LCDDriver_drawLine(10, 115 + Sel_Mess_Idx * 25, 70, 115 + Sel_Mess_Idx * 25, COLOR_WHITE);
#endif
	}

	old_Sel_Mess_Idx = Sel_Mess_Idx;
}

void FT8_Menu_Pos_Toggle(void) {

	if (FT8_Menu_Idx <= FT8_Menu_Max_Idx) // cursor is in the range of the buttons
	{
		if (sButtonData[FT8_Menu_Idx].state) {
			sButtonData[FT8_Menu_Idx].state = false;
		} else {
			sButtonData[FT8_Menu_Idx].state = true;
		}

		switch (FT8_Menu_Idx) {
		case 0:
			if (sButtonData[0].state) // CQ button pressed
			{
				CQ_Flag = 1;
				Beacon_State = 0;
			} else {
				CQ_Flag = 0;
				if (xmit_flag > 0) { // if we are transmitting -> go to receive
					receive_sequence();
				}
				FT8_Clear_TargetCall();
				FT8_Clear_TX_Mess();
			}
			break;

		case 1:
			if (sButtonData[1].state) { // Tune button pressed
				tune_On_sequence();
			} else {
				tune_Off_sequence();
				sButtonData[0].state = false; // if the "CQ" button was active disable it as well
				receive_sequence();

				drawButton(0); // update the "CQ" button
				FT8_Clear_TX_Mess();
			}
			break;
		}
		drawButton(FT8_Menu_Idx);

	}      // end if (FT8_Menu_Idx <= FT8_Menu_Max_Idx)			//cursor is in the range of the buttons
	else { // cursor is in the range of the recieved messages
		if (sButtonData[0].state) {
			sButtonData[0].state = false;
		} else {
			sButtonData[0].state = true;
		}

		if (sButtonData[0].state) // CQ button pressed
		{
			CQ_Flag = 1;
			Beacon_State = 5; // we answer the oposite side CQ
			SetNew_TargetCall(Sel_Mess_Idx);
			FT8_Print_TargetCall();
			set_message(1); // answer a "CQ" message
			setup_to_transmit_on_next_DSP_Flag();

			// trick to force the send imidiately
			Set_Data_Colection(1); // Set new data colection
		} else {
			CQ_Flag = 0;
			if (xmit_flag > 0) { // if we are transmitting -> go to receive
				receive_sequence();
			}
			FT8_Clear_TargetCall();
			FT8_Clear_TX_Mess();
		}
		drawButton(0); // update the "CQ" button
	}
}

void FT8_Print_Freq(void) {
	char ctmp[10] = {0};

	if (FT8_BND_Freq < 1000000) {
		sprintf(ctmp, "%llu kHz", FT8_BND_Freq);
	} else {
		sprintf(ctmp, "%llu", FT8_BND_Freq);
	}
#if (defined(LAY_800x480))
	LCDDriver_printText(ctmp, 680, 30, COLOR_WHITE, COLOR_BLACK, 2);
#elif (defined(LAY_320x240))
	LCDDriver_printText(ctmp, 260, 20, COLOR_WHITE, COLOR_BLACK, 1);
#else
	LCDDriver_printText(ctmp, 360, 30, COLOR_WHITE, COLOR_BLACK, 2);
#endif

	sprintf(ctmp, "%d Hz ", cursor_freq);
#if (defined(LAY_800x480))
	LCDDriver_printText(ctmp, 680, 55, COLOR_WHITE, COLOR_BLACK, 2);
#elif (defined(LAY_320x240))
	LCDDriver_printText(ctmp, 260, 35, COLOR_WHITE, COLOR_BLACK, 1);
#else
	LCDDriver_printText(ctmp, 360, 55, COLOR_WHITE, COLOR_BLACK, 2);
#endif
}

void FT8_Print_TargetCall(void) {
#if (defined(LAY_800x480))
	LCDDriver_printText(Target_Call, 680, 80, COLOR_YELLOW, COLOR_BLACK,
	                    2); // Display in the upper right corner (under the clock) the call sign of the partner

#elif (defined(LAY_320x240))
	LCDDriver_printText(Target_Call, 200, 100, COLOR_YELLOW, COLOR_BLACK, 1); // Display in the upper right corner (under the clock) the call sign of the partner

#else
	LCDDriver_printText(Target_Call, 360, 80, COLOR_YELLOW, COLOR_BLACK,
	                    2);                                // Display in the upper right corner (under the clock) the call sign of the partner
#endif
}

void FT8_Clear_TargetCall(void) {
#if (defined(LAY_800x480))
	LCDDriver_Fill_RectXY(680, 80, 799, 100, COLOR_BLACK); // Clear the old target call mesage

#elif (defined(LAY_320x240))
	LCDDriver_Fill_RectXY(220, 100, 320, 110, COLOR_BLACK);                   // Clear the old target call mesage

#else
	LCDDriver_Fill_RectXY(360, 80, 480, 100, COLOR_BLACK); // Clear the old target call mesage
#endif
}

void FT8_Clear_Mess_Field(void) {
#if (defined(LAY_800x480))
	LCDDriver_Fill_RectXY(0, 100, 480, 380, COLOR_BLACK); // Clear the old mesages

#elif (defined(LAY_320x240))
	LCDDriver_Fill_RectXY(0, 100, 320, 200, COLOR_BLACK);                     // Clear the old mesages

#else
	LCDDriver_Fill_RectXY(0, 100, 480, 240, COLOR_BLACK);  // Clear the old mesages
#endif
}

void FT8_Print_TX_Mess(char *message) {
#if (defined(LAY_800x480))
	LCDDriver_Fill_RectXY(0, 380, 240, 394, COLOR_BLACK); // Clear the old TX mesage
	LCDDriver_printText(message, 0, 380, COLOR_RED, COLOR_BLACK, 2);

#elif (defined(LAY_320x240))
	LCDDriver_Fill_RectXY(200, 130, 320, 145, COLOR_BLACK);                   // Clear the old TX mesage
	LCDDriver_printText(message, 200, 130, COLOR_RED, COLOR_BLACK, 1);

#else
	LCDDriver_Fill_RectXY(0, 260, 240, 274, COLOR_BLACK);  // Clear the old TX mesage
	LCDDriver_printText(message, 0, 260, COLOR_RED, COLOR_BLACK, 2);
#endif
}

void FT8_Clear_TX_Mess(void) {
#if (defined(LAY_800x480))
	LCDDriver_Fill_RectXY(0, 380, 240, 394, COLOR_BLACK); // Clear the old TX mesage

#elif (defined(LAY_320x240))
	LCDDriver_Fill_RectXY(200, 100, 240, 120, COLOR_BLACK); // Clear the old TX mesage

#else
	LCDDriver_Fill_RectXY(0, 260, 240, 274, COLOR_BLACK); // Clear the old TX mesage
#endif
}

void Enc2Rotate_Menager(int8_t direction, uint8_t decoded_msg) {
	char MessIdx;

	if (sButtonData[2].state) // if "Time correction" button is pressed
	{
		RTC_TimeTypeDef sTime = {0};
		RTC_DateTypeDef sDate = {0};
		getUTCDateTime(&sDate, &sTime);
		uint8_t Hours = sTime.Hours;
		uint8_t Minutes = sTime.Minutes;
		uint8_t Seconds = sTime.Seconds;

		if (Seconds == 0 && direction < 0) {
			return;
		}
		Seconds = (uint8_t)(Seconds + direction);

		if (Seconds >= 60) {
			Seconds = Seconds - 60;
			Minutes++;
		}
		sTime.TimeFormat = RTC_HOURFORMAT12_PM;
		sTime.SubSeconds = 0;
		sTime.SecondFraction = 0;
		sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
		sTime.StoreOperation = RTC_STOREOPERATION_SET;
		sTime.Hours = Hours;
		sTime.Minutes = Minutes;
		sTime.Seconds = Seconds;
		BKPSRAM_Enable();
		HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);

		drawButton(2);
	} else {
		FT8_Menu_Idx += direction;

#if (defined(LAY_800x480))
		if (decoded_msg > 11) { // message_limit it is 6
			MessIdx = 11;
		} else {
			MessIdx = decoded_msg;
		}

#elif (defined(LAY_320x240))
		if (decoded_msg > 6) {                                // message_limit it is 6
			MessIdx = 6;
		} else {
			MessIdx = decoded_msg;
		}

#else
		if (decoded_msg > 6) {                              // message_limit it is 6
			MessIdx = 6;
		} else {
			MessIdx = decoded_msg;
		}
#endif

		if (FT8_Menu_Idx > (FT8_Menu_Max_Idx + MessIdx)) {
			FT8_Menu_Idx = 0;
		}

		Update_FT8_Menu_Cursor();
	}
}