#include "wspr.h"
#include "fonts.h"
#include "fpga.h"
#include "functions.h"
#include "lcd.h"
#include "lcd_driver.h"
#include "main.h"
#include "trx_manager.h"

// Private variables
static char tmp_buff[64] = {0};
static WSPRState wspr_status = WSPR_WAIT;
static uint8_t wspr_band = 160;
static float64_t WSPR2_OffsetFreq[4] = {0, 1.4648, 2.9296, 4.3944};
static uint8_t WSPR2_encMessage[11];
static uint8_t WSPR2_symTable[170];     // symbol table 162
static uint8_t WSPR2_symTableTemp[170]; // symbol table temp
static const uint8_t WSPR2_SyncVec[162] = {1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0,
                                           0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0,
                                           1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0,
                                           0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0};

static float64_t WSPR_bands_freq[11] = {0};
static uint8_t WSPR2_count = 0;
static uint8_t WSPR2_BeginDelay = 0;

// Saved variables
static uint32_t Lastfreq = 0;
static uint_fast8_t Lastmode = 0;
static bool LastAutoGain = false;
static bool LastBandMapEnabled = false;
static bool LastRF_Filters = false;
static bool LastManualNotch = false;
static bool LastAutoNotch = false;
static uint8_t LastDNR = 0;
static bool LastRIT = false;
static bool LastXIT = false;
static bool LastSPLIT = false;
static bool LastNB1 = false;
static bool LastNB2 = false;
static bool LastMute = false;

// Public variables
bool SYSMENU_wspr_opened = false;

// Prototypes
static uint8_t WSPR_GetNextBand(void);
static float64_t WSPR_GetFreqFromBand(uint8_t band);
static void WSPR_Encode(void);
static void WSPR_Encode_call(void);
static char WSPR_chr_normf(char bc);
static void WSPR_Encode_locator(void);
static void WSPR_Encode_conv(void);
static uint8_t WSPR_parity(uint32_t li);
static void WSPR_Interleave_sync(void);
static void WSPR_StartTransmit(void);
static void WSPR_StopTransmit(void);

// start
void WSPR_Start(void) {
	LCD_busy = true;

	// save settings
	Lastfreq = CurrentVFO->Freq;
	Lastmode = CurrentVFO->Mode;
	LastAutoGain = TRX.AutoGain;
	LastBandMapEnabled = TRX.BandMapEnabled;
	LastRF_Filters = TRX.RF_Filters;
	LastManualNotch = CurrentVFO->ManualNotchFilter;
	LastAutoNotch = CurrentVFO->AutoNotchFilter;
	LastDNR = CurrentVFO->DNR_Type;
	LastRIT = TRX.RIT_Enabled;
	LastXIT = TRX.XIT_Enabled;
	LastSPLIT = TRX.SPLIT_Enabled;
	LastNB1 = TRX.NOISE_BLANKER1;
	LastNB2 = TRX.NOISE_BLANKER2;
	LastMute = TRX.Mute;

	// prepare trx
	TRX.Mute = true;
	TRX.TWO_SIGNAL_TUNE = false;
	TRX.BandMapEnabled = false;
	TRX_setMode(TRX_MODE_CW, CurrentVFO);
	wspr_band = WSPR_GetNextBand();
	WSPR_Encode();

	// prepare bands
	WSPR_bands_freq[0] = 1838100.0 + (float64_t)TRX.WSPR_FREQ_OFFSET;
	WSPR_bands_freq[1] = 3570100.0 + (float64_t)TRX.WSPR_FREQ_OFFSET;
	WSPR_bands_freq[2] = 7040100.0 + (float64_t)TRX.WSPR_FREQ_OFFSET;
	WSPR_bands_freq[3] = 10140200.0 + (float64_t)TRX.WSPR_FREQ_OFFSET;
	WSPR_bands_freq[4] = 14097100.0 + (float64_t)TRX.WSPR_FREQ_OFFSET;
	WSPR_bands_freq[5] = 18106100.0 + (float64_t)TRX.WSPR_FREQ_OFFSET;
	WSPR_bands_freq[6] = 21096100.0 + (float64_t)TRX.WSPR_FREQ_OFFSET;
	WSPR_bands_freq[7] = 24926100.0 + (float64_t)TRX.WSPR_FREQ_OFFSET;
	WSPR_bands_freq[8] = 28126100.0 + (float64_t)TRX.WSPR_FREQ_OFFSET;
	WSPR_bands_freq[9] = 50294500.0 + (float64_t)TRX.WSPR_FREQ_OFFSET;
	WSPR_bands_freq[10] = 144489000.0 + (float64_t)TRX.WSPR_FREQ_OFFSET;

	// draw the GUI
	LCDDriver_Fill(BG_COLOR);

#ifdef LCD_SMALL_INTERFACE
	LCDDriver_printText("WSPR Beacon", 5, 5, FG_COLOR, BG_COLOR, 1);
#else
	LCDDriver_printTextFont("WSPR Beacon", 10, 30, FG_COLOR, BG_COLOR, &FreeSans9pt7b);
#endif

	LCD_busy = false;
	LCD_UpdateQuery.SystemMenu = true;
}

// stop
void WSPR_Stop(void) {
	WSPR_StopTransmit();

	TRX_setFrequency(Lastfreq, CurrentVFO);
	TRX_setMode(Lastmode, CurrentVFO);
	TRX.AutoGain = LastAutoGain;
	TRX.BandMapEnabled = LastBandMapEnabled;
	TRX.RF_Filters = LastRF_Filters;
	CurrentVFO->ManualNotchFilter = LastManualNotch;
	CurrentVFO->AutoNotchFilter = LastAutoNotch;
	CurrentVFO->DNR_Type = LastDNR;
	TRX.RIT_Enabled = LastRIT;
	TRX.XIT_Enabled = LastXIT;
	TRX.SPLIT_Enabled = LastSPLIT;
	TRX.NOISE_BLANKER1 = LastNB1;
	TRX.NOISE_BLANKER2 = LastNB2;
	TRX.Mute = LastMute;
	LCD_UpdateQuery.StatusInfoBarRedraw = true;
}

// draw
void WSPR_DoEvents(void) {
	if (LCD_busy) {
		LCD_UpdateQuery.SystemMenuRedraw = true;
		return;
	}
	LCD_busy = true;

#ifdef LCD_SMALL_INTERFACE
	uint16_t y = 30;
	const uint16_t y_step = 10;
#else
	uint16_t y = 50;
	const uint16_t y_step = 20;
#endif

	RTC_TimeTypeDef sTime = {0};
	RTC_DateTypeDef sDate = {0};
	getLocalDateTime(&sDate, &sTime);
	uint8_t Hours = sTime.Hours;
	uint8_t Minutes = sTime.Minutes;
	uint8_t Seconds = sTime.Seconds;
	static uint8_t OLD_Seconds = 0;

	// Start timeslot
	if (bitRead(Minutes, 0) == 0 && Seconds == 0 && OLD_Seconds != Seconds) {
		wspr_band = WSPR_GetNextBand();
		TRX_setTXFrequencyFloat(WSPR_GetFreqFromBand(wspr_band), CurrentVFO);
		WSPR_StartTransmit();
	}
	OLD_Seconds = Seconds;

	// show time
	sprintf(tmp_buff, "Time: %02d:%02d:%02d", Hours, Minutes, Seconds);

#ifdef LCD_SMALL_INTERFACE
	LCDDriver_printText(tmp_buff, 5, y, FG_COLOR, BG_COLOR, 1);
#else
	LCDDriver_printText(tmp_buff, 10, y, FG_COLOR, BG_COLOR, 2);
#endif

	y += y_step;

	// next slot time
	uint8_t Next_Hours = Hours;
	uint8_t Next_Minutes = Minutes + 1;
	uint8_t Next_Seconds = 0;
	if ((Next_Minutes & 0x1) == 1) {
		Next_Minutes++;
	}
	if (Next_Minutes >= 60) {
		Next_Minutes = 0;
		Next_Hours++;
	}
	if (Next_Hours >= 24) {
		Next_Hours = 0;
	}
	sprintf(tmp_buff, "Next slot: %02d:%02d:%02d", Next_Hours, Next_Minutes, Next_Seconds);

#ifdef LCD_SMALL_INTERFACE
	LCDDriver_printText(tmp_buff, 5, y, FG_COLOR, BG_COLOR, 1);
#else
	LCDDriver_printText(tmp_buff, 10, y, FG_COLOR, BG_COLOR, 2);
#endif

	y += y_step;

// status
#ifdef LCD_SMALL_INTERFACE
	if (wspr_status == WSPR_WAIT) {
		LCDDriver_printText("STATUS: WAIT    ", 5, y, FG_COLOR, BG_COLOR, 1);
	}
	if (wspr_status == WSPR_TRANSMIT) {
		LCDDriver_printText("STATUS: TRANSMIT", 5, y, FG_COLOR, BG_COLOR, 1);
	}
#else
	if (wspr_status == WSPR_WAIT) {
		LCDDriver_printText("STATUS: WAIT    ", 10, y, FG_COLOR, BG_COLOR, 2);
	}
	if (wspr_status == WSPR_TRANSMIT) {
		LCDDriver_printText("STATUS: TRANSMIT", 10, y, FG_COLOR, BG_COLOR, 2);
	}
#endif
	y += y_step;

// Band
#ifdef LCD_SMALL_INTERFACE
	sprintf(tmp_buff, "Current band: % 2dm    ", wspr_band);
	LCDDriver_printText(tmp_buff, 5, y, FG_COLOR, BG_COLOR, 1);
#else
	sprintf(tmp_buff, "Current band: % 2dm (%dHz)", wspr_band, (uint32_t)(WSPR_GetFreqFromBand(wspr_band)));
	LCDDriver_printText(tmp_buff, 10, y, FG_COLOR, BG_COLOR, 2);
#endif
	y += y_step;

// Next band
#ifdef LCD_SMALL_INTERFACE
	sprintf(tmp_buff, "Next band: % 2dm    ", WSPR_GetNextBand());
	LCDDriver_printText(tmp_buff, 5, y, FG_COLOR, BG_COLOR, 1);
#else
	sprintf(tmp_buff, "Next band: % 2dm (%dHz)", WSPR_GetNextBand(), (uint32_t)(WSPR_GetFreqFromBand(WSPR_GetNextBand())));
	LCDDriver_printText(tmp_buff, 10, y, FG_COLOR, BG_COLOR, 2);
#endif
	y += y_step;

// TX parameters
#ifdef LCD_SMALL_INTERFACE
	sprintf(tmp_buff, "SWR: %.1f, PWR: %.1fW     ", (double)TRX_SWR, ((double)TRX_PWR_Forward - (double)TRX_PWR_Backward));
	LCDDriver_printText(tmp_buff, 5, y, FG_COLOR, BG_COLOR, 1);
#else
	sprintf(tmp_buff, "SWR: %.1f, PWR: %.1fW, TEMP: % 2d     ", (double)TRX_SWR, ((double)TRX_PWR_Forward - (double)TRX_PWR_Backward), (int16_t)TRX_RF_Temperature);
	LCDDriver_printText(tmp_buff, 10, y, FG_COLOR, BG_COLOR, 2);
#endif

	LCD_busy = false;
	LCD_UpdateQuery.SystemMenuRedraw = true;
}

// Transmit
void WSPR_DoFastEvents(void) {
	if (WSPR2_BeginDelay < 1) { // Begin delay - actually 0.682mSec
		TRX_setTXFrequencyFloat(WSPR_GetFreqFromBand(wspr_band), CurrentVFO);
		WSPR2_BeginDelay++;
	} else {
		// Begin 162 WSPR symbol transmission
		if (WSPR2_count < 162) {
			TRX_setTXFrequencyFloat(WSPR_GetFreqFromBand(wspr_band) + WSPR2_OffsetFreq[WSPR2_symTable[WSPR2_count]], CurrentVFO);
			WSPR2_count++; // Increments the interrupt counter
		} else {
			WSPR_StopTransmit();
			TRX_SNTP_Synced = 0; // request time sync
		}
	}
}

static void WSPR_StartTransmit(void) {
	// Start WSPR transmit process
	TRX_Tune = 1;
	TRX_ptt_hard = TRX_Tune;
	TRX_Restart_Mode();
	wspr_status = WSPR_TRANSMIT;
	WSPR2_BeginDelay = 0;
	WSPR2_count = 0;
#ifdef HRDW_WSPR_TIMER
	HAL_TIM_Base_Start_IT(&HRDW_WSPR_TIMER);
#endif
}

static void WSPR_StopTransmit(void) {
	// Stop WSPR transmit process
	TRX_Tune = 0;
	TRX_ptt_hard = TRX_Tune;
	TRX_Restart_Mode();
	wspr_status = WSPR_WAIT;
	WSPR2_BeginDelay = 0;
	WSPR2_count = 0;
#ifdef HRDW_WSPR_TIMER
	HAL_TIM_Base_Stop_IT(&HRDW_WSPR_TIMER);
#endif
}

// events to the encoder
void WSPR_EncRotate(int8_t direction) {
#pragma unused(direction)
	if (LCD_busy) {
		return;
	}
	LCD_busy = true;

	LCD_busy = false;
}

static uint8_t WSPR_GetNextBand(void) {
	if (wspr_band >= 160 && TRX.WSPR_BANDS_80) {
		return 80;
	}
	if (wspr_band >= 80 && TRX.WSPR_BANDS_40) {
		return 40;
	}
	if (wspr_band >= 40 && TRX.WSPR_BANDS_30) {
		return 30;
	}
	if (wspr_band >= 30 && TRX.WSPR_BANDS_20) {
		return 20;
	}
	if (wspr_band >= 20 && TRX.WSPR_BANDS_17) {
		return 17;
	}
	if (wspr_band >= 17 && TRX.WSPR_BANDS_15) {
		return 15;
	}
	if (wspr_band >= 15 && TRX.WSPR_BANDS_12) {
		return 12;
	}
	if (wspr_band >= 12 && TRX.WSPR_BANDS_10) {
		return 10;
	}
	if (wspr_band >= 10 && TRX.WSPR_BANDS_6) {
		return 6;
	}
	if (wspr_band >= 6 && TRX.WSPR_BANDS_2) {
		return 2;
	}
	if (wspr_band >= 2 && TRX.WSPR_BANDS_160) {
		return 160;
	}
	if (TRX.WSPR_BANDS_160) {
		return 160;
	}
	if (TRX.WSPR_BANDS_80) {
		return 80;
	}
	if (TRX.WSPR_BANDS_40) {
		return 40;
	}
	if (TRX.WSPR_BANDS_30) {
		return 30;
	}
	if (TRX.WSPR_BANDS_20) {
		return 20;
	}
	if (TRX.WSPR_BANDS_17) {
		return 17;
	}
	if (TRX.WSPR_BANDS_15) {
		return 15;
	}
	if (TRX.WSPR_BANDS_12) {
		return 12;
	}
	if (TRX.WSPR_BANDS_10) {
		return 10;
	}
	if (TRX.WSPR_BANDS_6) {
		return 6;
	}
	if (TRX.WSPR_BANDS_2) {
		return 2;
	}
	return 20;
}

static float64_t WSPR_GetFreqFromBand(uint8_t band) {
	if (band == 160) {
		return WSPR_bands_freq[0];
	}
	if (band == 80) {
		return WSPR_bands_freq[1];
	}
	if (band == 40) {
		return WSPR_bands_freq[2];
	}
	if (band == 30) {
		return WSPR_bands_freq[3];
	}
	if (band == 20) {
		return WSPR_bands_freq[4];
	}
	if (band == 17) {
		return WSPR_bands_freq[5];
	}
	if (band == 15) {
		return WSPR_bands_freq[6];
	}
	if (band == 12) {
		return WSPR_bands_freq[7];
	}
	if (band == 10) {
		return WSPR_bands_freq[8];
	}
	if (band == 6) {
		return WSPR_bands_freq[9];
	}
	if (band == 2) {
		return WSPR_bands_freq[10];
	}
	return WSPR_bands_freq[4];
}

static void WSPR_Encode(void) {
	// Begin WSPR message calculation
	WSPR_Encode_call();
	WSPR_Encode_locator();
	WSPR_Encode_conv();
	WSPR_Interleave_sync();
}

// normalize characters 0..9 A..Z Space in order 0..36
static char WSPR_chr_normf(char bc) {
	char cc = 36;
	if (bc >= '0' && bc <= '9') {
		cc = bc - '0';
	}
	if (bc >= 'A' && bc <= 'Z') {
		cc = bc - 'A' + 10;
	}
	if (bc >= 'a' && bc <= 'z') {
		cc = bc - 'a' + 10;
	}
	if (bc == ' ') {
		cc = 36;
	}

	return (cc);
}

static void WSPR_Encode_call(void) {
	// copy callsign
	char enc_call[7] = {0};
	for (int ci = 0; ci < 7; ci++) {
		enc_call[ci] = TRX.CALLSIGN[ci];
	}

	// coding of callsign
	if (WSPR_chr_normf(enc_call[2]) > 9) {
		enc_call[5] = enc_call[4];
		enc_call[4] = enc_call[3];
		enc_call[3] = enc_call[2];
		enc_call[2] = enc_call[1];
		enc_call[1] = enc_call[0];
		enc_call[0] = ' ';
	}

	unsigned long n1 = WSPR_chr_normf(enc_call[0]);
	n1 = n1 * 36 + WSPR_chr_normf(enc_call[1]);
	n1 = n1 * 10 + WSPR_chr_normf(enc_call[2]);
	n1 = n1 * 27 + WSPR_chr_normf(enc_call[3]) - 10;
	n1 = n1 * 27 + WSPR_chr_normf(enc_call[4]) - 10;
	n1 = n1 * 27 + WSPR_chr_normf(enc_call[5]) - 10;

	// merge coded callsign into message array c[]
	unsigned long t1 = n1;
	WSPR2_encMessage[0] = t1 >> 20;
	WSPR2_encMessage[1] = t1 >> 12;
	WSPR2_encMessage[2] = t1 >> 4;
	WSPR2_encMessage[3] = t1 << 4;
}

static void WSPR_Encode_locator(void) {
	// Min = 0 dBm, Max = 43 dBm, steps 0,3,7,10,13,17,20,23,27,30,33,37,40,43
	uint8_t power = 20;
	// from 7w out max
	if (TRX.RF_Gain >= 3) {
		power = 23;
	}
	if (TRX.RF_Gain >= 7) {
		power = 27;
	}
	if (TRX.RF_Gain >= 14) {
		power = 30;
	}
	if (TRX.RF_Gain >= 28) {
		power = 33;
	}
	if (TRX.RF_Gain >= 71) {
		power = 37;
	}

	// coding of locator
	unsigned long m1 = 179 - 10 * (WSPR_chr_normf(TRX.LOCATOR[0]) - 10) - WSPR_chr_normf(TRX.LOCATOR[2]);
	m1 = m1 * 180 + 10 * (WSPR_chr_normf(TRX.LOCATOR[1]) - 10) + WSPR_chr_normf(TRX.LOCATOR[3]);
	m1 = m1 * 128 + power + 64;

	// merge coded locator and power into message array c[]
	unsigned long t1 = m1;
	WSPR2_encMessage[3] = WSPR2_encMessage[3] + (0x0f & t1 >> 18);
	WSPR2_encMessage[4] = t1 >> 10;
	WSPR2_encMessage[5] = t1 >> 2;
	WSPR2_encMessage[6] = t1 << 6;
}

static uint8_t WSPR_parity(uint32_t li) {
	uint8_t po = 0;
	while (li != 0) {
		po++;
		li &= (li - 1);
	}
	return (po & 1);
}

// convolutional encoding of message array c[] into a 162 bit stream
static void WSPR_Encode_conv(void) {
	int bc = 0;
	int cnt = 0;
	int cc;
	unsigned long sh1 = 0;

	cc = WSPR2_encMessage[0];

	for (int i = 0; i < 81; i++) {
		if (i % 8 == 0) {
			cc = WSPR2_encMessage[bc];
			bc++;
		}
		if (cc & 0x80) {
			sh1 = sh1 | 1;
		}

		WSPR2_symTableTemp[cnt++] = WSPR_parity(sh1 & 0xF2D05351);
		WSPR2_symTableTemp[cnt++] = WSPR_parity(sh1 & 0xE4613C47);

		cc = cc << 1;
		sh1 = sh1 << 1;
	}
}

// interleave reorder the 162 data bits and and merge table with the sync vector
static void WSPR_Interleave_sync(void) {
	int ii, ij, b2, bis, ip;
	ip = 0;

	for (ii = 0; ii <= 255; ii++) {
		bis = 1;
		ij = 0;
		for (b2 = 0; b2 < 8; b2++) {
			if (ii & bis) {
				ij = ij | (0x80 >> b2);
			}
			bis = bis << 1;
		}
		if (ij < 162) {
			WSPR2_symTable[ij] = WSPR2_SyncVec[ij] + 2 * WSPR2_symTableTemp[ip];
			ip++;
		}
	}
}
