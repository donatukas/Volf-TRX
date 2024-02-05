#include "gen_ft8.h"
#include "FT8_GUI.h"
#include "arm_math.h"
#include "constants.h"
#include "decode_ft8.h"
#include "encode.h"
#include "lcd.h"        //For debug
#include "lcd_driver.h" //For debug
#include "locator_ft8.h"
#include "pack.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char message[20];

void set_message(uint16_t index) {
	uint8_t packed[K_BYTES];
	const char blank[] = "                   ";
	const char seventy_three[] = "RR73";
	const char seventy_three2[] = "73";

	strcpy(message, blank);

	switch (index) {

	case 0:
		sprintf(message, "%s %s %s", "CQ", TRX.CALLSIGN, TRX.LOCATOR);
		break;

	case 1:
		sprintf(message, "%s %s %s", Target_Call, TRX.CALLSIGN, TRX.LOCATOR); // Answer a "CQ"
		break;

	case 2:
		sprintf(message, "%s %s %3i", Target_Call, TRX.CALLSIGN, Target_RSL);
		// case 2:  sprintf(message,"%s %s", Target_Call,Station_Call);
		break;

	case 3:
		sprintf(message, "%s %s %3s", Target_Call, TRX.CALLSIGN, seventy_three);
		break;

	case 4:
		sprintf(message, "%s %s R%3i", Target_Call, TRX.CALLSIGN,
		        Target_RSL); // Send a raport for the - Answer "CQ" case (diffrence from "2" is the "R")
		break;

	case 5:
		sprintf(message, "%s %s %s", Target_Call, TRX.CALLSIGN,
		        seventy_three2); // Send a "73" for the - Answer "CQ" case (diffrence from "3" is the missing "RR")
		break;
	}

	// Debug
	FT8_Print_TX_Mess(message);

	pack77_1(message, packed);
	genft8(packed, tones);
}
