#include "events.h"
#include "FT8/FT8_main.h"
#include "INA226_PWR_monitor.h"
#include "agc.h"
#include "atu.h"
#include "audio_filters.h"
#include "audio_processor.h"
#include "bootloader.h"
#include "codec.h"
#include "cw.h"
#include "decoder.h"
#include "fft.h"
#include "fpga.h"
#include "front_unit.h"
#include "functions.h"
#include "lcd.h"
#include "main.h"
#include "profiler.h"
#include "rf_unit.h"
#include "satellite.h"
#include "sd.h"
#include "settings.h"
#include "snap.h"
#include "swr_analyzer.h"
#include "system_menu.h"
#include "trx_manager.h"
#include "usbd_audio_if.h"
#include "usbd_cat_if.h"
#include "usbd_debug_if.h"
#include "usbd_ua3reo.h"
#include "vocoder.h"
#include "wifi.h"
#include "wspr.h"

void EVENTS_do_WSPR(void) // 1,4648 Hz
{
	if (SYSMENU_wspr_opened) {
		WSPR_DoFastEvents();
	}
}

void EVENTS_do_WIFI(void) // 1000 Hz
{
	static uint16_t wifi_start_timeout = 0;

	// process wifi
	if (wifi_start_timeout < 10) {
		wifi_start_timeout++;
	} else {
#if HRDW_HAS_WIFI
		// we work with WiFi by timer, or send it if it is turned off (to turn it on, we need a restart)
		if (WIFI.Enabled) {
			WIFI_Process();
		} else {
			WIFI_GoSleep();
		}
#endif
	}
}

void EVENTS_do_FFT(void) // 1000 Hz
{
	if (SYSMENU_spectrum_opened || SYSMENU_swr_opened) {
		SYSMENU_drawSystemMenu(false, false);
		return;
	}

	if (FFT_need_fft) {
		FFT_doFFT();
	}

#if HRDW_HAS_USB_CAT
	ua3reo_dev_cat_parseCommand();
#endif
}

void EVENTS_do_AUDIO_PROCESSOR(void) // 20 000 Hz
{
	if (!Processor_NeedTXBuffer && !Processor_NeedRXBuffer) {
		return;
	}

	if (TRX_on_TX) {
		processTxAudio();
	}

	if (TRX_on_RX) {
		processRxAudio();
	}

	// in the spectrum analyzer mode, we raise its processing to priority, performing together with the audio processor
	if (SYSMENU_spectrum_opened) {
		LCD_doEvents();
	}

#if FT8_SUPPORT
	if (FT8_DecodeActiveFlg) {
		TRX_Inactive_Time = 0;
		MenagerFT8();
	}
#endif
}

void EVENTS_do_USB_FIFO(void) // 1000 Hz
{
	print_flush(); // send data to debug from the buffer

	// unmute after transition process end
	if (TRX_Temporary_Mute_StartTime > 0 && (HAL_GetTick() - TRX_Temporary_Mute_StartTime) > 100) {
		CODEC_UnMute();
		TRX_Temporary_Mute_StartTime = 0;
	}
}

void EVENTS_do_PERIPHERAL(void) // 1000 Hz
{
#if HRDW_HAS_SD
	if (SD_BusyByUSB) {
		return;
	}
#endif

	// FRONT PANEL SPI
	static uint16_t front_slowler = 0;
	front_slowler++;
	if (front_slowler > 20) {
		FRONTPANEL_Process();
		front_slowler = 0;
	}

	// EEPROM SPI
	if (NeedSaveCalibration) { // save calibration data to EEPROM
		SaveCalibration();
	}
	if (NeedSaveWiFi) { // save WiFi settings data to EEPROM
		SaveWiFiSettings();
	}

#if HRDW_HAS_SD
	// SD-Card SPI
	SD_Process();
#endif
}

static int8_t EVENTS_get_encoderDirection(uint8_t state) {
	switch (state) {
	// base process
	case 0b0001:
	case 0b1000:
	case 0b0111:
	case 0b1110:
	// for fast interrupts
	case 0b0110:
		// case 0b1001:
		return -1;
	// base process
	case 0b0010:
	case 0b1011:
	case 0b0100:
	case 0b1101:
	// for fast interrupts
	case 0b1100:
		// case 0b0011:
		return 1;
	}

	return 0;
}

static int8_t EVENTS_get_encoder2Direction(uint8_t state) {
	switch (state) {
	// case 0b0001:
	case 0b1000:
		if (CALIBRATE.ENCODER2_ON_FALLING) {
			return 0;
		}
	// case 0b1110:
	case 0b0111:
		return CALIBRATE.ENCODER2_INVERT ? 1 : -1;
	// case 0b0010:
	case 0b0100:
		if (CALIBRATE.ENCODER2_ON_FALLING) {
			return 0;
		}
	// case 0b1101:
	case 0b1011:
		return CALIBRATE.ENCODER2_INVERT ? -1 : 1;
	}

	return 0;
}

void EVENTS_do_ENC(void) // 20 0000 Hz
{
	// Update watchdog
	HAL_IWDG_Refresh(&HRDW_IWDG);

	if (!TRX_Inited) {
		return;
	}

	uint32_t currentTick = HAL_GetTick();

	// Poll encoders by timer
	// Main Encoder
	static uint8_t ENCODER_state = 0;
	static uint32_t ENCODER_AValDeb = 0;
	static int32_t ENCODER_slowler = 0;
	static int16_t ENCticksInInterval = 0;
	static uint32_t ENCstartMeasureTime = 0;
	static float32_t ENCAcceleration = 0;
	static uint8_t ENCODER_DTVal_prevState = 0;
	static uint8_t ENCODER_CLKVal_prevState = 0;

	if ((currentTick - ENCODER_AValDeb) >= CALIBRATE.ENCODER_DEBOUNCE) {
		uint8_t ENCODER_DTVal = HAL_GPIO_ReadPin(ENC_DT_GPIO_Port, ENC_DT_Pin);
		uint8_t ENCODER_CLKVal = HAL_GPIO_ReadPin(ENC_CLK_GPIO_Port, ENC_CLK_Pin);
		if (ENCODER_DTVal == ENCODER_DTVal_prevState && ENCODER_CLKVal == ENCODER_CLKVal_prevState || CALIBRATE.ENCODER_DEBOUNCE == 0) {
			ENCODER_state = ((ENCODER_state << 2) | (ENCODER_DTVal << 1) | ENCODER_CLKVal) & 0xF;
			int8_t direction = EVENTS_get_encoderDirection(ENCODER_state);
			uint8_t ENCODER_SLOW_RATE = LCD_systemMenuOpened ? (CALIBRATE.ENCODER_SLOW_RATE * 5) : CALIBRATE.ENCODER_SLOW_RATE;

			if (direction < 0) {
				ENCODER_slowler--;
				if (ENCODER_slowler <= -ENCODER_SLOW_RATE) {
					// acceleration
					ENCticksInInterval++;
					if ((currentTick - ENCstartMeasureTime) > CALIBRATE.ENCODER_ACCELERATION) {
						ENCstartMeasureTime = currentTick;
						ENCAcceleration = (10.0f + ENCticksInInterval - 1.0f) / 10.0f;
						ENCticksInInterval = 0;
					}
					// do rotate
					FRONTPANEL_ENCODER_Rotated(CALIBRATE.ENCODER_INVERT ? ENCAcceleration : -ENCAcceleration);
					ENCODER_slowler = 0;
					TRX_ScanMode = false;
				}
			}
			if (direction > 0) {
				ENCODER_slowler++;
				if (ENCODER_slowler >= ENCODER_SLOW_RATE) {
					// acceleration
					ENCticksInInterval++;
					if ((currentTick - ENCstartMeasureTime) > CALIBRATE.ENCODER_ACCELERATION) {
						ENCstartMeasureTime = currentTick;
						ENCAcceleration = (10.0f + ENCticksInInterval - 1.0f) / 10.0f;
						ENCticksInInterval = 0;
					}
					// do rotate
					FRONTPANEL_ENCODER_Rotated(CALIBRATE.ENCODER_INVERT ? -ENCAcceleration : ENCAcceleration);
					ENCODER_slowler = 0;
					TRX_ScanMode = false;
				}
			}
		}
		ENCODER_AValDeb = currentTick;
		ENCODER_DTVal_prevState = ENCODER_DTVal;
		ENCODER_CLKVal_prevState = ENCODER_CLKVal;
	}

	// ENC2
	static uint8_t ENCODER2_state = 0;
	static uint32_t ENCODER2_AValDeb = 0;
	static uint8_t ENCODER2_DTVal_prevState = 0;
	static uint8_t ENCODER2_CLKVal_prevState = 0;

	if ((currentTick - ENCODER2_AValDeb) >= CALIBRATE.ENCODER2_DEBOUNCE) {
		uint8_t ENCODER2_DTVal = HAL_GPIO_ReadPin(ENC2_DT_GPIO_Port, ENC2_DT_Pin);
		uint8_t ENCODER2_CLKVal = HAL_GPIO_ReadPin(ENC2_CLK_GPIO_Port, ENC2_CLK_Pin);
		if (ENCODER2_DTVal == ENCODER2_DTVal_prevState && ENCODER2_CLKVal == ENCODER2_CLKVal_prevState || CALIBRATE.ENCODER2_DEBOUNCE == 0) {
			ENCODER2_state = ((ENCODER2_state << 2) | (ENCODER2_DTVal << 1) | ENCODER2_CLKVal) & 0xF;
			int8_t direction = EVENTS_get_encoder2Direction(ENCODER2_state);
			if (direction != 0) {
				FRONTPANEL_ProcessEncoder2 = direction;
			}
		}
		ENCODER2_AValDeb = currentTick;
		ENCODER2_DTVal_prevState = ENCODER2_DTVal;
		ENCODER2_CLKVal_prevState = ENCODER2_CLKVal;
	}

#ifdef HRDW_HAS_ENC3
	static uint8_t ENCODER3_state = 0;
	static uint32_t ENCODER3_AValDeb = 0;
	static uint8_t ENCODER3_DTVal_prevState = 0;
	static uint8_t ENCODER3_CLKVal_prevState = 0;

	if ((currentTick - ENCODER3_AValDeb) >= CALIBRATE.ENCODER2_DEBOUNCE) {
		uint8_t ENCODER3_DTVal = HAL_GPIO_ReadPin(ENC3_DT_GPIO_Port, ENC3_DT_Pin);
		uint8_t ENCODER3_CLKVal = HAL_GPIO_ReadPin(ENC3_CLK_GPIO_Port, ENC3_CLK_Pin);
		if (ENCODER3_DTVal == ENCODER3_DTVal_prevState && ENCODER3_CLKVal == ENCODER3_CLKVal_prevState || CALIBRATE.ENCODER2_DEBOUNCE == 0) {
			ENCODER3_state = ((ENCODER3_state << 2) | (ENCODER3_DTVal << 1) | ENCODER3_CLKVal) & 0xF;
			int8_t direction = EVENTS_get_encoder2Direction(ENCODER3_state);
			if (direction != 0) {
				FRONTPANEL_ProcessEncoder3 = direction;
			}
		}
		ENCODER3_AValDeb = currentTick;
		ENCODER3_DTVal_prevState = ENCODER3_DTVal;
		ENCODER3_CLKVal_prevState = ENCODER3_CLKVal;
	}
#endif

#ifdef HRDW_HAS_ENC4
	static uint8_t ENCODER4_state = 0;
	static uint32_t ENCODER4_AValDeb = 0;
	static uint8_t ENCODER4_DTVal_prevState = 0;
	static uint8_t ENCODER4_CLKVal_prevState = 0;

	if ((currentTick - ENCODER4_AValDeb) >= CALIBRATE.ENCODER2_DEBOUNCE) {
		uint8_t ENCODER4_DTVal = HAL_GPIO_ReadPin(ENC4_DT_GPIO_Port, ENC4_DT_Pin);
		uint8_t ENCODER4_CLKVal = HAL_GPIO_ReadPin(ENC4_CLK_GPIO_Port, ENC4_CLK_Pin);
		if (ENCODER4_DTVal == ENCODER4_DTVal_prevState && ENCODER4_CLKVal == ENCODER4_CLKVal_prevState || CALIBRATE.ENCODER2_DEBOUNCE == 0) {
			ENCODER4_state = ((ENCODER4_state << 2) | (ENCODER4_DTVal << 1) | ENCODER4_CLKVal) & 0xF;
			int8_t direction = EVENTS_get_encoder2Direction(ENCODER4_state);
			if (direction != 0) {
				FRONTPANEL_ProcessEncoder4 = direction;
			}
		}
		ENCODER4_AValDeb = currentTick;
		ENCODER4_DTVal_prevState = ENCODER4_DTVal;
		ENCODER4_CLKVal_prevState = ENCODER4_CLKVal;
	}
#endif

#ifdef HAS_TOUCHPAD
	static bool TOUCH_Int_Last = true;
	bool TOUCH_Int_Now = HAL_GPIO_ReadPin(T_INT_GPIO_Port, T_INT_Pin);
	if (TOUCH_Int_Last != TOUCH_Int_Now) {
		TOUCH_Int_Last = TOUCH_Int_Now;
		if (TOUCH_Int_Now) {
			TOUCHPAD_reserveInterrupt();
		}
	}

	// TOUCHPAD_reserveInterrupt();
#endif
}

void EVENTS_do_PREPROCESS(void) // 1000 Hz
{
	// audio buffer RX preprocessor
	if (TRX_on_RX) {
		preProcessRxAudio();
	}

	if (FFT_new_buffer_ready) {
		FFT_bufferPrepare();
	}

	if (NeedProcessDecoder) {
		DECODER_Process();
	}
}

void EVENTS_do_EVERY_10ms(void) // 100 Hz
{
	static uint32_t powerdown_start_delay = 0;
	static bool prev_pwr_state = true;
	static uint32_t ms10_10_counter = 0;
	static uint32_t ms10_30_counter = 0;
	ms10_10_counter++;
	ms10_30_counter++;

	// power off sequence
	if (prev_pwr_state == true && HAL_GPIO_ReadPin(PWR_ON_GPIO_Port, PWR_ON_Pin) == GPIO_PIN_RESET) {
		powerdown_start_delay = HAL_GetTick();
	}
	prev_pwr_state = HAL_GPIO_ReadPin(PWR_ON_GPIO_Port, PWR_ON_Pin);

	if ((HAL_GPIO_ReadPin(PWR_ON_GPIO_Port, PWR_ON_Pin) == GPIO_PIN_RESET) && ((HAL_GetTick() - powerdown_start_delay) > POWERDOWN_TIMEOUT) &&
	    ((!NeedSaveCalibration && !NeedSaveWiFi && !HRDW_SPI_Locked && !EEPROM_Busy && !LCD_busy) || ((HAL_GetTick() - powerdown_start_delay) > POWERDOWN_FORCE_TIMEOUT))) {
		LCD_busy = true;
		SaveSettings();
		SaveSettingsToEEPROM();
		ATU_Flush_Memory();
		TRX_Inited = false;
		CODEC_Mute();
		CODEC_CleanBuffer();
		LCDDriver_Fill(COLOR_BLACK);
		LCD_showInfo("GOOD BYE, 73", false);
		print_flush();
		while (HAL_GPIO_ReadPin(PWR_ON_GPIO_Port, PWR_ON_Pin) == GPIO_PIN_RESET) {
		}
		HAL_Delay(500);
		HAL_GPIO_WritePin(PWR_HOLD_GPIO_Port, PWR_HOLD_Pin, GPIO_PIN_RESET);
		// SCB->AIRCR = 0x05FA0004; // software reset
		while (true) {
		}
	}

	// Process SWR, Power meter, ALC, Thermal sensors, Fan, ...
	RF_UNIT_ProcessSensors();

	if (TRX_on_TX) {
		sprintf(TRX_SWR_SMOOTHED_STR, "SWR: %.1f", (double)TRX_SWR_SMOOTHED);
	} else {
		sprintf(TRX_SWR_SMOOTHED_STR, "SWR: -RX-");
	}

	// TRX protector
	if (TRX_on_TX) {
		TRX_TX_EndTime = HAL_GetTick();

		if (TRX_RF_Temperature > CALIBRATE.TRX_MAX_RF_TEMP) {
			TRX_Tune = false;
			TRX_ptt_hard = false;
			TRX_ptt_soft = false;
			LCD_UpdateQuery.StatusInfoGUIRedraw = true;
			LCD_UpdateQuery.StatusInfoBarRedraw = true;
			LCD_UpdateQuery.TopButtons = true;
			NeedSaveSettings = true;
			TRX_Restart_Mode();
			println("RF temperature too HIGH!");
			LCD_showTooltip("RF temperature too HIGH!");
		}
		/*if (TRX_SWR_SMOOTHED > CALIBRATE.TRX_MAX_SWR && !TRX_Tune && TRX_PWR_Forward > CALIBRATE.TUNE_MAX_POWER)
		{
		  TRX_Tune = false;
		  TRX_ptt_hard = false;
		  TRX_ptt_soft = false;
		  LCD_UpdateQuery.StatusInfoGUIRedraw = true;
		  LCD_UpdateQuery.StatusInfoBarRedraw = true;
		  LCD_UpdateQuery.TopButtons = true;
		  NeedSaveSettings = true;
		  TRX_Restart_Mode();
		  println("SWR too HIGH!");
		  LCD_showTooltip("SWR too HIGH!");
		}*/
		if (TRX_SWR_SMOOTHED > CALIBRATE.TRX_MAX_SWR && !TRX_Tune && TRX_PWR_Forward > 1.0f && !TRX_SWR_PROTECTOR) {
			TRX_SWR_PROTECTOR = true;
			LCD_showTooltip("SWR HIGH!");
		}
	}

	// transmission release time after key signal
	if (CW_Key_Timeout_est > 0 && !CW_key_serial && !CW_key_dot_hard && !CW_key_dash_hard) {
		CW_Key_Timeout_est -= 10;
		if (CW_Key_Timeout_est < 10) {
			CW_Key_Timeout_est = 0;
			FPGA_NeedSendParams = true;
			TRX_Restart_Mode();
		}
	}

	// VOX
	APROC_doVOX();

	// if the settings have changed, update the parameters in the FPGA
	if (NeedSaveSettings) {
		FPGA_NeedSendParams = true;
	}

	// there was a request to reinitialize audio and notch filters
	if (NeedReinitNotch) {
		InitNotchFilter();
	}
	if (NeedReinitAudioFilters) {
		ReinitAudioFilters();
	}

	// Process touchpad frequency changing
	TRX_setFrequencySlowly_Process();

	// emulate PTT over CAT/Software
	if (TRX_ptt_soft != TRX_old_ptt_soft) {
		TRX_ptt_change();
	}

	// emulate the key via the COM port
	if (CW_key_serial != CW_old_key_serial) {
		CW_key_change();
	}

	// update the state of the RF-Unit board
	RF_UNIT_UpdateState(false);

// INA226 current and voltage
#ifdef HAS_TOUCHPAD
	if (CALIBRATE.INA226_EN) {
		Read_INA226_Data();
	}
#endif

// check touchpad events
#ifdef HAS_TOUCHPAD
	TOUCHPAD_ProcessInterrupt();
#endif

	// Process LCD dimmer
#ifdef HAS_BRIGHTNESS_CONTROL
	static bool LCD_Dimmer_State = false;
	if (!LCD_busy) {
		LCD_busy = true;
		if (!LCD_Dimmer_State && TRX.LCD_SleepTimeout > 0 && TRX_Inactive_Time >= TRX.LCD_SleepTimeout) {
			LCDDriver_setBrightness(IDLE_LCD_BRIGHTNESS);
			LCD_Dimmer_State = true;
		}
		if (LCD_Dimmer_State && TRX_Inactive_Time < TRX.LCD_SleepTimeout) {
			LCDDriver_setBrightness(TRX.LCD_Brightness);
			LCD_Dimmer_State = false;
		}
		LCD_busy = false;
	}
#endif

	static bool needLCDDoEvents = true;
	if (ms10_30_counter >= 3) // every 30ms
	{
		ms10_30_counter = 0;
		LCD_UpdateQuery.StatusInfoBar = true;
		// update information on LCD
		needLCDDoEvents = true;
	} else if (LCD_UpdateQuery.FreqInfo) { // Redraw freq fast
		needLCDDoEvents = true;
	}

	if (needLCDDoEvents && LCD_doEvents()) {
		needLCDDoEvents = false;
	}

	static uint8_t needPrintFFT = 0;
#if defined(FRONTPANEL_LITE)
	if (needPrintFFT < 10 && (ms10_10_counter >= (6 - TRX.FFT_Speed) * 2)) // every x msec
	{
		ms10_10_counter = 0;
		needPrintFFT++;
	}
#else
	if (needPrintFFT < 10 && (ms10_10_counter % (6 - TRX.FFT_Speed)) == 0) // every x msec
	{
		ms10_10_counter = 0;
		needPrintFFT++;
	}
#endif

	if (needPrintFFT > 0 && !LCD_UpdateQuery.Background && FFT_printFFT()) { // draw FFT
		needPrintFFT--;
	}

	// restart USB if there is no activity (off) to find a new connection
	if (TRX_Inited && ((USB_LastActiveTime + USB_RESTART_TIMEOUT < HAL_GetTick()))) { // || (USB_LastActiveTime == 0)
		USBD_Restart();
	}
}

void EVENTS_do_EVERY_100ms(void) // 10 Hz
{
	// every 100ms we receive data from FPGA (amplitude, ADC overload, etc.)
	FPGA_NeedGetParams = true;

	// Detect FPGA stuck error
	static float32_t old_FPGA_Audio_Buffer_RX1_I = 0;
	static float32_t old_FPGA_Audio_Buffer_RX1_Q = 0;
	const float32_t *FPGA_Audio_Buffer_RX1_I_current = !FPGA_RX_Buffer_Current ? (float32_t *)&FPGA_Audio_Buffer_RX1_I_A : (float32_t *)&FPGA_Audio_Buffer_RX1_I_B;
	const float32_t *FPGA_Audio_Buffer_RX1_Q_current = !FPGA_RX_Buffer_Current ? (float32_t *)&FPGA_Audio_Buffer_RX1_Q_A : (float32_t *)&FPGA_Audio_Buffer_RX1_Q_B;
	static uint16_t fpga_stuck_errors = 0;
	if (FPGA_Audio_Buffer_RX1_I_current[0] == old_FPGA_Audio_Buffer_RX1_I || FPGA_Audio_Buffer_RX1_Q_current[0] == old_FPGA_Audio_Buffer_RX1_Q) {
		fpga_stuck_errors++;
	} else {
		fpga_stuck_errors = 0;
	}
	if (fpga_stuck_errors > 5 && TRX_on_RX && !TRX.ADC_SHDN && !FPGA_bus_stop && CurrentVFO->Mode != TRX_MODE_WFM) // && !SD_PlayInProcess
	{
		// println("[ERR] IQ stuck error, restart disabled");
		fpga_stuck_errors = 0;
		// FPGA_NeedRestart_RX = true;
	}
	old_FPGA_Audio_Buffer_RX1_I = FPGA_Audio_Buffer_RX1_I_current[0];
	old_FPGA_Audio_Buffer_RX1_Q = FPGA_Audio_Buffer_RX1_Q_current[0];

	// Process AutoGain feature
	TRX_DoAutoGain();

	// Process Scaner
	if (TRX_ScanMode) {
		TRX_ProcessScanMode();
	}

	// reset error flags
	CODEC_Buffer_underrun = false;
	FPGA_Buffer_underrun = false;
	RX_USB_AUDIO_underrun = false;
	APROC_IFGain_Overflow = false;
	TRX_PWR_ALC_SWR_OVERFLOW = false;
	TRX_MIC_BELOW_NOISEGATE = false;
#if HRDW_HAS_SD
	SD_underrun = false;
#endif
}

void EVENTS_do_EVERY_500ms(void) // 2 Hz
{
	// Redraw menu infolines if needed
	LCD_UpdateQuery.SystemMenuInfolines = true;
}

void EVENTS_do_EVERY_1000ms(void) // 1 Hz
{
	static uint32_t tim6_delay = 0;
	TRX_Inactive_Time++;

	// Detect FPGA IQ phase error
	if (fabsf(TRX_IQ_phase_error) > 0.1f && TRX_on_RX && !TRX_phase_restarted && !TRX.ADC_SHDN && !FPGA_bus_stop && CurrentVFO->Mode != TRX_MODE_WFM) {
		println("[ERR] IQ phase error, restart disabled | ", (float64_t)TRX_IQ_phase_error);
		// FPGA_NeedRestart_RX = true;
		TRX_phase_restarted = true;
	}

#if HRDW_HAS_WIFI
	if (!WIFI_download_inprogress) {
		bool maySendIQ = true;
		if (!WIFI_IP_Gotted) { // Get resolved IP
			WIFI_GetIP(NULL);
			maySendIQ = false;
		}
		uint32_t mstime = HAL_GetTick();
		if (TRX_SNTP_Synced == 0 || (mstime > (SNTP_SYNC_INTERVAL * 1000) && TRX_SNTP_Synced < (mstime - SNTP_SYNC_INTERVAL * 1000))) { // Sync time from internet
			WIFI_GetSNTPTime(NULL);
			maySendIQ = false;
		}
		if (WIFI.CAT_Server && !WIFI_CAT_server_started) { // start WiFi CAT Server
			WIFI_StartCATServer(NULL);
			maySendIQ = false;
		}
		if (CALIBRATE.OTA_update && !WIFI_NewFW_checked) { // check OTA FW updates
			WIFI_checkFWUpdates();
			maySendIQ = false;
		}
		if (TRX.FFT_DXCluster && ((HAL_GetTick() - TRX_DXCluster_UpdateTime) > DXCLUSTER_UPDATE_TIME || TRX_DXCluster_UpdateTime == 0) && !SYSMENU_FT8_DECODER_opened) // get and show dx cluster
		{
			if (WIFI_getDXCluster_background()) {
				TRX_DXCluster_UpdateTime = HAL_GetTick();
			}
			maySendIQ = false;
		}
		if (TRX.WOLF_Cluster && ((HAL_GetTick() - TRX_WOLF_Cluster_UpdateTime) > WOLF_CLUSTER_UPDATE_TIME || TRX_WOLF_Cluster_UpdateTime == 0) &&
		    !SYSMENU_FT8_DECODER_opened) // get and show WOLF cluster
		{
			if (WIFI_getWOLFCluster_background()) {
				TRX_WOLF_Cluster_UpdateTime = HAL_GetTick();
			}
			maySendIQ = false;
		}
#if HRDW_HAS_SD
		if (TRX.SatMode && ((HAL_GetTick() - TRX_SAT_UpdateTime) > SAT_UPDATE_TIME || TRX_SAT_UpdateTime == 0)) // get satelite propagination
		{
			SAT_calc(SAT_unix2daynum(getUTCTimestamp(), 0));
			TRX_SAT_UpdateTime = HAL_GetTick();
			maySendIQ = false;
		}
#endif
		WIFI_maySendIQ = maySendIQ;
	}
#endif

	// Check vBAT
	static bool vbat_checked = false;
	if (!vbat_checked && TRX_Inited && !LCD_busy) {
		vbat_checked = true;
		if (TRX_VBAT_Voltage <= 2.5f) {
			LCD_showError("Replace BAT", true);
		}
	}

	// Check FPGA version
	static bool fpga_ver_checked = false;
	if (!fpga_ver_checked && TRX_Inited && !LCD_busy) {
		fpga_ver_checked = true;

		char out[10];
		sprintf(out, "%d.%d.%d", FPGA_FW_Version[2], FPGA_FW_Version[1], FPGA_FW_Version[0]);
		if (strcmp(out, FPGA_VERSION_STR) != NULL) {
			LCD_showError("FPGA version error", true);
		}
	}

	// Auto Snap
	if (TRX.Auto_Snap) {
		SNAP_DoSnap(true, 0);
	}

	// Free tune center spectrum on idle
	if (TRX.CenterSpectrumAfterIdle && TRX_Inactive_Time >= FREE_TUNE_CENTER_ON_IDLE_SEC && CurrentVFO->SpectrumCenterFreq != CurrentVFO->Freq) {
		CurrentVFO->SpectrumCenterFreq = CurrentVFO->Freq;
		TRX_setFrequency(CurrentVFO->Freq, CurrentVFO);
	}

	CPULOAD_Calc(); // Calculate CPU load
	TRX_STM32_TEMPERATURE = HRDW_getCPUTemperature();
	TRX_STM32_VREF = HRDW_getCPUVref();

	// Save Debug variables
	uint32_t dbg_tim6_delay = HAL_GetTick() - tim6_delay;
	float32_t dbg_coeff = 1000.0f / (float32_t)dbg_tim6_delay;
	dbg_FPGA_samples = (uint32_t)((float32_t)FPGA_samples * dbg_coeff);

	if (TRX.Debug_Type == TRX_DEBUG_SYSTEM) {
		// Print debug
		uint32_t dbg_WM8731_DMA_samples = (uint32_t)((float32_t)CODEC_DMA_samples / 2.0f * dbg_coeff);
		uint32_t dbg_AUDIOPROC_samples = (uint32_t)((float32_t)AUDIOPROC_samples * dbg_coeff);
		const float32_t *FPGA_Audio_Buffer_RX1_I_current = !FPGA_RX_Buffer_Current ? (float32_t *)&FPGA_Audio_Buffer_RX1_I_A : (float32_t *)&FPGA_Audio_Buffer_RX1_I_B;
		const float32_t *FPGA_Audio_Buffer_RX1_Q_current = !FPGA_RX_Buffer_Current ? (float32_t *)&FPGA_Audio_Buffer_RX1_Q_A : (float32_t *)&FPGA_Audio_Buffer_RX1_Q_B;
		float32_t dbg_FPGA_Audio_Buffer_I_tmp = FPGA_Audio_Buffer_RX1_I_current[0];
		float32_t dbg_FPGA_Audio_Buffer_Q_tmp = FPGA_Audio_Buffer_RX1_Q_current[0];
		if (TRX_on_TX) {
			dbg_FPGA_Audio_Buffer_I_tmp = FPGA_Audio_SendBuffer_I[0];
			dbg_FPGA_Audio_Buffer_Q_tmp = FPGA_Audio_SendBuffer_Q[0];
		}
		uint32_t dbg_RX_USB_AUDIO_SAMPLES = (uint32_t)((float32_t)RX_USB_AUDIO_SAMPLES * dbg_coeff);
		uint32_t dbg_TX_USB_AUDIO_SAMPLES = (uint32_t)((float32_t)TX_USB_AUDIO_SAMPLES * dbg_coeff);
		uint32_t cpu_load = (uint32_t)CPU_LOAD.Load;
		// Print Debug info
		println("FPGA Samples: ", dbg_FPGA_samples);            //~96000
		println("Audio DMA samples: ", dbg_WM8731_DMA_samples); //~48000
		print_flush();
		println("Audioproc blocks: ", dbg_AUDIOPROC_samples);
		println("CPU Load: ", cpu_load);
		print_flush();
		println("RF/STM32 Temperature: ", (int16_t)TRX_RF_Temperature, " / ", (int16_t)TRX_STM32_TEMPERATURE);
		println("STM32 Voltage: ", (double)TRX_STM32_VREF);
		print_flush();
		println("TIM6 delay: ", dbg_tim6_delay);
		println("RX1 dBm: ", (double)TRX_RX1_dBm_measurement);
		println("Noise floor: ", (double)TRX_NoiseFloor);
		println("FFT FPS: ", FFT_FPS);
		print_flush();
		println("First byte of RX-FPGA I/Q: ", (double)dbg_FPGA_Audio_Buffer_I_tmp, " / ", (double)dbg_FPGA_Audio_Buffer_Q_tmp); // first byte of IQ
		println("IQ Phase error: ", (double)TRX_IQ_phase_error);                                                                 // first byte of Q
		print_flush();
		println("USB Audio RX/TX samples: ", dbg_RX_USB_AUDIO_SAMPLES, " / ", dbg_TX_USB_AUDIO_SAMPLES); //~48000
		println("ADC MIN/MAX Amplitude: ", TRX_ADC_MINAMPLITUDE, " / ", TRX_ADC_MAXAMPLITUDE);
		print_flush();
#if HRDW_HAS_WIFI
		println("WIFI State: ", WIFI_State);
#endif
		println("");
		print_flush();
		PrintProfilerResult();
	}

	// Save Settings to Backup Memory
	if (NeedSaveSettings && (HAL_GPIO_ReadPin(PWR_ON_GPIO_Port, PWR_ON_Pin) == GPIO_PIN_SET)) {
		SaveSettings();
	}

	// ATU memory flush
	if (!HRDW_SPI_Locked) {
		ATU_Flush_Memory();
	}

	// Reset counters
	tim6_delay = HAL_GetTick();
	FPGA_samples = 0;
	AUDIOPROC_samples = 0;
	CODEC_DMA_samples = 0;
	FFT_FPS_Last = FFT_FPS;
	FFT_FPS = 0;
	RX_USB_AUDIO_SAMPLES = 0;
	TX_USB_AUDIO_SAMPLES = 0;
	FPGA_NeedSendParams = true;

// redraw lcd to fix problem
#ifdef LCD_HX8357B
	static uint8_t HX8357B_BUG_redraw_counter = 0;
	HX8357B_BUG_redraw_counter++;
	if (HX8357B_BUG_redraw_counter == 60) {
		LCD_UpdateQuery.TopButtonsRedraw = true;
		LCD_UpdateQuery.StatusInfoBarRedraw = true;
		LCD_UpdateQuery.StatusInfoGUI = true;
	} else if (HX8357B_BUG_redraw_counter == 120) {
		LCD_UpdateQuery.FreqInfoRedraw = true;
		LCD_UpdateQuery.StatusInfoGUI = true;
	} else if (HX8357B_BUG_redraw_counter >= 180) {
		LCD_UpdateQuery.StatusInfoGUI = true;
		LCD_UpdateQuery.StatusInfoBarRedraw = true;
		HX8357B_BUG_redraw_counter = 0;
	}
#endif
}
