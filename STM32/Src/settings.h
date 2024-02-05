#ifndef SETTINGS_h
#define SETTINGS_h

#include "bands.h"
#include "functions.h"
#include "hardware.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define STM32_VERSION_STR "9.3.1" // current STM32 version

#if defined(FRONTPANEL_LITE)
#define FPGA_VERSION_STR "8.0.0" // needed FPGA version Wolf-Lite
#else
#define FPGA_VERSION_STR "8.2.0" // needed FPGA version Wolf/Wolf-2/Wolf-X1
#endif

#define SETT_VERSION 149        // Settings config version
#define CALIB_VERSION 91        // Calibration config version
#define WIFI_SETTINGS_VERSION 5 // WiFi config version

#define TRX_SAMPLERATE 48000                 // audio stream sampling rate during processing and TX (NOT RX!)
#define MAX_TX_AMPLITUDE_MULT 0.85f          // Maximum amplitude when transmitting to FPGA
#define AGC_CLIPPING 6.0f                    // Limit over target in AGC, dB
#define TOUCHPAD_DELAY 200                   // Anti-bounce time for pressing the touchpad
#define AUTOGAIN_TARGET_AMPLITUDE 20000.0f   // target autocorrector amplitude
#define AUTOGAIN_MAX_AMPLITUDE 30000.0f      // maximum autocorrector amplitude
#define AUTOGAIN_CORRECTOR_WAITSTEP 5        // waiting for the averaging of the results when the auto-corrector of the input circuits is running
#define KEY_HOLD_TIME 500                    // time of long pressing of the keyboard button for triggering, ms
#define SHOW_LOGO true                       // Show logo on boot (from images.h)
#define POWERDOWN_TIMEOUT 1000               // time of pressing the shutdown button, for operation, ms
#define POWERDOWN_FORCE_TIMEOUT 2000         // force time
#define USB_RESTART_TIMEOUT 5000             // time after which USB restart occurs if there are no packets
#define SNTP_SYNC_INTERVAL (60 * 2)          // Time synchronization interval via NTP, sec
#define SCANNER_NOSIGNAL_TIME 50             // time to continue sweeping if signal too low
#define SCANNER_NOSIGNAL_TIME_MEM 500        // time to continue sweeping if signal too low (MEM channels scan mode)
#define SCANNER_SIGNAL_TIME_FM 5000          // time to continue sweeping if signal founded for FM
#define SCANNER_SIGNAL_TIME_OTHER 1000       // time to continue sweeping if signal founded for SSB
#define SCANNER_FREQ_STEP_WFM 100000         // step for freq scanner for WFM
#define SCANNER_FREQ_STEP_NFM 25000          // step for freq scanner for NFM
#define SCANNER_FREQ_STEP_OTHER 500          // step for freq scanner for SSB
#define ENCODER_MIN_RATE_ACCELERATION 2.0f   // encoder enable rounding if lower than value
#define DXCLUSTER_UPDATE_TIME (1000 * 60)    // interval to get cluster info, 1min
#define WOLF_CLUSTER_UPDATE_TIME (1000 * 30) // interval to get WOLF cluster info, 30sec
#define SAT_UPDATE_TIME (1000 * 1)           // interval to calculate SAT info, 1sec
#define NORMAL_SWR_SAVED 1.5f                // ATU SWR target for saved settings
#define NORMAL_SWR_TUNE 1.2f                 // ATU SWR target for new tune
#define IDLE_LCD_BRIGHTNESS 5                // Low brightness for IDLE mode (dimmer)
#define CW_ADD_GAIN_IF 18.0f                 // additional IF gain in CW
#define TX_LPF_TIMEOUT (180 * 1000)          // TX LPF On Timeout, millisec (3 min)
#define SWR_PROTECTOR_MAX_POWER 20.0f        // drop down to PWR %, if SWR high
#define FREE_TUNE_CENTER_ON_IDLE_SEC 20      // center spectrum on idle while in free tune mode

#define FULL_DUPLEX TRX.Full_Duplex             // Enable duplex RX and TX
#define SHOW_RX_FFT_ON_TX FULL_DUPLEX           // Show RX FFT channel on TX
#define LISTEN_RX_AUDIO_ON_TX SHOW_RX_FFT_ON_TX // Process audio RX channel for TX mode

#define CODEC_BITS_FULL_SCALE 4294967296 // maximum signal amplitude in the bus // powf (2, FPGA_BUS_BITS)
#define USB_DEBUG_ENABLED true           // allow using USB as a console
#define SWD_DEBUG_ENABLED false          // enable SWD as a console
#define LCD_DEBUG_ENABLED false          // enable LCD as a console

#define AUTOGAINER_TAGET (ADC_FULL_SCALE / 3)
#define AUTOGAINER_HYSTERESIS (ADC_FULL_SCALE / 13)

#define MAX_WIFIPASS_LENGTH 32
#define MAX_CALLSIGN_LENGTH 16
#define MAX_CW_MACROS_LENGTH 61
#define MAX_CW_MACROS_NAME_LENGTH 6
#define ALLQSO_TOKEN_SIZE 16

#if defined(FRONTPANEL_MINI)
#define MAX_CHANNEL_MEMORY_NAME_LENGTH 6
#else
#define MAX_CHANNEL_MEMORY_NAME_LENGTH 10
#endif

#define SAT_NAME_MAXLEN 20
#define SAT_TLE_MAXCOUNT 28
#define SAT_TLE_LINE_MAXLEN 70
#define SAT_QTH_LINE_MAXLEN 12

#define ATU_MAX_FREQ_KHZ 60000
#define ATU_MEM_STEP_MIN_KHZ 50
#define ATU_MEM_COUNT (ATU_MAX_FREQ_KHZ / ATU_MEM_STEP_MIN_KHZ) // 1200 positions * 3byte = 3600 bytes for each ANT

#define W25Q16_COMMAND_Write_Disable 0x04
#define W25Q16_COMMAND_Write_Enable 0x06
#define W25Q16_COMMAND_Erase_Chip 0xC7
#define W25Q16_COMMAND_Sector_Erase 0x20
#define W25Q16_COMMAND_32KBlock_Erase 0x52
#define W25Q16_COMMAND_Page_Program 0x02
#define W25Q16_COMMAND_Read_Data 0x03
#define W25Q16_COMMAND_FastRead_Data 0x0B
#define W25Q16_COMMAND_Power_Down 0xB9
#define W25Q16_COMMAND_Power_Up 0xAB
#define W25Q16_COMMAND_GetStatus 0x05
#define W25Q16_COMMAND_WriteStatus 0x01
#define W25Q16_PAGE_SIZE 256
#define W25Q16_SECTOR_SIZE 4096

#define EEPROM_SECTOR_CALIBRATION 0
#define EEPROM_SECTOR_SETTINGS 4
#define EEPROM_SECTOR_WIFI 8
#define EEPROM_SECTOR_ATU_1 10
#define EEPROM_SECTOR_ATU_2 11
#define EEPROM_SECTOR_ATU_3 12
#define EEPROM_SECTOR_ATU_4 13
#define EEPROM_SECTOR_DPD 16   // + BANDS_COUNT sectors
#define EEPROM_REPEAT_TRYES 10 // command tryes

#if defined(FRONTPANEL_NONE) || defined(FRONTPANEL_SMALL_V1) || defined(FRONTPANEL_X1) // || defined(FRONTPANEL_MINI)
#define MEMORY_CHANNELS_COUNT 0
#elif defined(LCD_RA8875)
#define MEMORY_CHANNELS_COUNT 35
#else
#define MEMORY_CHANNELS_COUNT 20
#endif

#define BANDS_MEMORIES_COUNT 3
#define ANT_MAX_COUNT 4

#define ATU_MAXLENGTH 7
#define ATU_0x0_MAXPOS B8(00000000)
#define ATU_5x5_MAXPOS B8(00011111)
#define ATU_6x6_MAXPOS B8(00111111)
#define ATU_7x7_MAXPOS B8(01111111)

#ifdef FRONTPANEL_MINI
static float32_t ATU_I_VALS[ATU_MAXLENGTH + 1] = {0.0, 0.1, 0.22, 0.45, 1.0, 2.2, 4.5, 4.5};
static float32_t ATU_C_VALS[ATU_MAXLENGTH + 1] = {0.0, 22.0, 47.0, 100.0, 220.0, 470.0, 1000.0, 1000.0};
#else
static float32_t ATU_I_VALS[ATU_MAXLENGTH + 1] = {0.0, 0.05, 0.1, 0.22, 0.45, 1.0, 2.2, 4.4};
static float32_t ATU_C_VALS[ATU_MAXLENGTH + 1] = {0.0, 10.0, 22.0, 47.0, 100.0, 220.0, 470.0, 1000.0};
#endif

// FRONT PANELS
#ifdef FRONTPANEL_NONE
#define MAX_VOLUME_VALUE 1024.0f
#define FUNCBUTTONS_COUNT 1
#define FUNCBUTTONS_ON_PAGE 1
#define FUNCBUTTONS_PAGES 1
static char ota_config_frontpanel[] = "NONE";
#define ATU_MAXPOS ATU_0x0_MAXPOS
#endif

#ifdef FRONTPANEL_SMALL_V1
#define HRDW_MCP3008_1 true
#define HRDW_MCP3008_2 true
#define HRDW_MCP3008_3 true
#define MAX_VOLUME_VALUE 1024.0f
#define FUNCBUTTONS_COUNT 1
#define FUNCBUTTONS_ON_PAGE 1
#define FUNCBUTTONS_PAGES 1
static char ota_config_frontpanel[] = "SMALL";
#define ATU_MAXPOS                                                                                                                 \
	((CALIBRATE.RF_unit_type == RF_UNIT_BIG || CALIBRATE.RF_unit_type == RF_UNIT_RU4PN || CALIBRATE.RF_unit_type == RF_UNIT_KT_100S) \
	     ? ATU_5x5_MAXPOS                                                                                                            \
	     : ((CALIBRATE.RF_unit_type == RF_UNIT_SPLIT) ? ATU_7x7_MAXPOS : ATU_0x0_MAXPOS))
#endif

#ifdef FRONTPANEL_LITE
#define HRDW_MCP3008_1 true
#define HRDW_HAS_FUNCBUTTONS true
#define MAX_VOLUME_VALUE 100.0f
#define FUNCBUTTONS_ON_PAGE 5
#define FUNCBUTTONS_PAGES 7
#define FUNCBUTTONS_COUNT (FUNCBUTTONS_PAGES * FUNCBUTTONS_ON_PAGE + 1)
static char ota_config_frontpanel[] = "LITE";
#define ATU_MAXPOS ATU_0x0_MAXPOS
#endif

#ifdef FRONTPANEL_BIG_V1
#define HRDW_MCP3008_1 true
#define HRDW_HAS_FUNCBUTTONS true
#define MAX_VOLUME_VALUE 1024.0f
#define FUNCBUTTONS_ON_PAGE 8
#define FUNCBUTTONS_PAGES 5
#define FUNCBUTTONS_COUNT (FUNCBUTTONS_PAGES * FUNCBUTTONS_ON_PAGE + 6)
static char ota_config_frontpanel[] = "BIG";
#define ATU_MAXPOS                                                                                                                 \
	((CALIBRATE.RF_unit_type == RF_UNIT_BIG || CALIBRATE.RF_unit_type == RF_UNIT_RU4PN || CALIBRATE.RF_unit_type == RF_UNIT_KT_100S) \
	     ? ATU_5x5_MAXPOS                                                                                                            \
	     : ((CALIBRATE.RF_unit_type == RF_UNIT_SPLIT) ? ATU_7x7_MAXPOS : ATU_0x0_MAXPOS))
#endif

#ifdef FRONTPANEL_WF_100D
#define HRDW_MCP3008_1 true
#define HRDW_HAS_FUNCBUTTONS true
#define MAX_VOLUME_VALUE 1024.0f
#define FUNCBUTTONS_ON_PAGE 9
#define FUNCBUTTONS_PAGES 4
#define FUNCBUTTONS_COUNT (FUNCBUTTONS_PAGES * FUNCBUTTONS_ON_PAGE + 7)
static char ota_config_frontpanel[] = "WF_100D";
#define ATU_MAXPOS ATU_7x7_MAXPOS
#endif

#ifdef FRONTPANEL_KT_100S
#define HRDW_MCP3008_1 true
#define HRDW_HAS_FUNCBUTTONS true
#define MAX_VOLUME_VALUE 1024.0f
#define FUNCBUTTONS_ON_PAGE 8
#define FUNCBUTTONS_PAGES 5
#define FUNCBUTTONS_COUNT (FUNCBUTTONS_PAGES * FUNCBUTTONS_ON_PAGE + 6)
static char ota_config_frontpanel[] = "KT_100S";
#define ATU_MAXPOS ATU_5x5_MAXPOS
#endif

#ifdef FRONTPANEL_WOLF_2
#define HRDW_MCP3008_1 true
#define HRDW_HAS_FUNCBUTTONS true
#define MAX_VOLUME_VALUE 1024.0f
#define FUNCBUTTONS_ON_PAGE 9
#define FUNCBUTTONS_PAGES 4
#define FUNCBUTTONS_COUNT (FUNCBUTTONS_PAGES * FUNCBUTTONS_ON_PAGE + 7)
static char ota_config_frontpanel[] = "WOLF-2";
#define ATU_MAXPOS ATU_7x7_MAXPOS
#endif

#ifdef FRONTPANEL_X1
#define HRDW_MCP3008_1 true
#define HRDW_HAS_FUNCBUTTONS true
#define MAX_VOLUME_VALUE 100.0f
#define FUNCBUTTONS_ON_PAGE 4
#define FUNCBUTTONS_PAGES 9
#define FUNCBUTTONS_COUNT (FUNCBUTTONS_PAGES * FUNCBUTTONS_ON_PAGE + 3)
static char ota_config_frontpanel[] = "X1";
#define ATU_MAXPOS ATU_0x0_MAXPOS
#endif

#ifdef FRONTPANEL_MINI
#define HRDW_HAS_FUNCBUTTONS true
#define MAX_VOLUME_VALUE 100.0f
#define FUNCBUTTONS_ON_PAGE 4
#define FUNCBUTTONS_PAGES 9
#define FUNCBUTTONS_COUNT (FUNCBUTTONS_PAGES * FUNCBUTTONS_ON_PAGE + 10)
static char ota_config_frontpanel[] = "Mini";
#define ATU_MAXPOS ATU_6x6_MAXPOS
#endif

// LCDs
#if defined(LCD_ILI9481)
static char ota_config_lcd[] = "ILI9481";
#ifdef STM32H743xx
#define FT8_SUPPORT true
#endif
#endif

#if defined(LCD_ILI9481_IPS)
static char ota_config_lcd[] = "ILI9481_IPS";
#ifdef STM32H743xx
#define FT8_SUPPORT true
#endif
#endif

#if defined(LCD_HX8357B)
static char ota_config_lcd[] = "HX8357B";
#ifdef STM32H743xx
#define FT8_SUPPORT true
#endif
#endif

#if defined(LCD_HX8357C) && !defined(LCD_SLOW)
static char ota_config_lcd[] = "HX8357C";
#ifdef STM32H743xx
#define FT8_SUPPORT true
#endif
#endif

#if defined(LCD_HX8357C) && defined(LCD_SLOW)
static char ota_config_lcd[] = "HX8357C-SLOW";
#ifdef STM32H743xx
#define FT8_SUPPORT true
#endif
#endif

#if defined(LCD_ILI9486)
static char ota_config_lcd[] = "ILI9486";
#ifdef STM32H743xx
#define FT8_SUPPORT true
#endif
#endif

#if defined(LCD_ST7796S)
static char ota_config_lcd[] = "ST7796S";
#ifdef STM32H743xx
#define FT8_SUPPORT true
#endif
#endif

#if defined(LCD_ILI9341)
#if defined(FRONTPANEL_MINI)
static char ota_config_lcd[] = "WOLF-Mini-ILI9341";
#else
static char ota_config_lcd[] = "ILI9341";
#endif
#ifdef STM32H743xx
#define FT8_SUPPORT true
#endif
#endif

#if defined(LCD_ST7789)
static char ota_config_lcd[] = "LCD_ST7789";
#ifdef STM32H743xx
#define FT8_SUPPORT true
#endif
#endif

#if defined(LCD_ST7735S)
#if defined(FRONTPANEL_MINI)
static char ota_config_lcd[] = "WOLF-Mini-ST7735S";
#else
static char ota_config_lcd[] = "ST7735S";
#endif
#ifdef STM32H743xx
#define FT8_SUPPORT false
#endif
#endif

#if defined(LCD_RA8875)
#if defined(FRONTPANEL_WF_100D)
static char ota_config_lcd[] = "WF-100D";
#elif defined(FRONTPANEL_KT_100S)
static char ota_config_lcd[] = "KT-100S";
#elif defined(FRONTPANEL_WOLF_2)
static char ota_config_lcd[] = "WOLF-2";
#else
static char ota_config_lcd[] = "RA8875";
#endif

#ifdef STM32H743xx
#define FT8_SUPPORT true
#endif
#endif

#if defined(LCD_NONE)
static char ota_config_lcd[] = "NONE";
#ifdef STM32H743xx
#define FT8_SUPPORT true
#endif
#endif

// TOUCHPADs
#if defined(TOUCHPAD_GT911)
static char ota_config_touchpad[] = "GT911";
#else
static char ota_config_touchpad[] = "NONE";
#endif

typedef enum {
	TRX_MODE_LSB,
	TRX_MODE_USB,
	TRX_MODE_CW,
	TRX_MODE_NFM,
	TRX_MODE_WFM,
	TRX_MODE_AM,
	TRX_MODE_SAM_STEREO,
	TRX_MODE_SAM_LSB,
	TRX_MODE_SAM_USB,
	TRX_MODE_DIGI_L,
	TRX_MODE_DIGI_U,
	TRX_MODE_IQ,
	TRX_MODE_LOOPBACK,
	TRX_MODE_RTTY,
} TRX_MODE;
#define TRX_MODE_COUNT 14

#if HRDW_HAS_DUAL_RX
// dual receiver operating mode
typedef enum {
	VFO_A_AND_B,
	VFO_A_PLUS_B,
} DUAL_RX_TYPE;
#endif

// CAT type
typedef enum {
	CAT_FT450,
	CAT_TS2000,
} CAT_TYPE;

// trx input
typedef enum {
	TRX_INPUT_MIC,
	TRX_INPUT_LINE,
	TRX_INPUT_USB,
} TRX_INPUT_TYPE;

// debug level
typedef enum {
	TRX_DEBUG_OFF,
	TRX_DEBUG_SYSTEM,
	TRX_DEBUG_WIFI,
	TRX_DEBUG_BUTTONS,
	TRX_DEBUG_TOUCH,
	TRX_DEBUG_CAT,
	TRX_DEBUG_I2C,
} TRX_DEBUG_TYPE;

// IQ SAMPLERATE
typedef enum {
	TRX_SAMPLERATE_K48,
	TRX_SAMPLERATE_K96,
	TRX_SAMPLERATE_K192,
	TRX_SAMPLERATE_K384,
} TRX_IQ_SAMPLERATE_VALUE;

// RF UNIT TYPE
#ifdef FRONTPANEL_MINI
typedef enum {
	RF_UNIT_HF,
	RF_UNIT_VHF,
} TRX_RF_UNIT_TYPE;
#else
typedef enum {
	RF_UNIT_NONE,
	RF_UNIT_QRP,
	RF_UNIT_BIG,
	RF_UNIT_SPLIT,
	RF_UNIT_RU4PN,
	RF_UNIT_KT_100S,
	RF_UNIT_WF_100D,
} TRX_RF_UNIT_TYPE;
#endif

// TANGENT TYPE
typedef enum {
	TANGENT_MH36,
	TANGENT_MH48,
} TRX_TANGENT_TYPE;

// CW PTT TYPE
typedef enum {
	KEY_PTT,
	EXT_PTT,
	KEY_AND_EXT_PTT,
} CW_PTT_TYPE;

// DX Cluster Type
typedef enum {
	DX_CLUSTER_RBN,
	DX_CLUSTER_DX_SUMMIT,
} DX_CLUSTER_TYPE;

// ENC2 FUNC MODE
typedef enum {
	ENC_FUNC_PAGER,
	ENC_FUNC_FAST_STEP,
	ENC_FUNC_SET_WPM,
	ENC_FUNC_SET_RIT,
	ENC_FUNC_SET_NOTCH,
	ENC_FUNC_SET_LPF,
	ENC_FUNC_SET_HPF,
	ENC_FUNC_SET_SQL,
	ENC_FUNC_SET_VOLUME,
	ENC_FUNC_SET_IF,
	ENC_FUNC_SET_MEM,
} ENC2_FUNC_MODE;

// COM port RTS/DTR lines mode
typedef enum {
	COM_LINE_MODE_DISABLED,
	COM_LINE_MODE_PTT,
	COM_LINE_MODE_KEYER,
} COM_LINE_MODE;

// Audio lines mode
typedef enum {
	RX_AUDIO_MODE_STEREO,
	RX_AUDIO_MODE_LEFT,
	RX_AUDIO_MODE_RIGHT,
} RX_AUDIO_MODE;

// FFT Automatic type
typedef enum {
	FFT_AUTOMATIC_NONE,
	FFT_AUTOMATIC_HALF,
	FFT_AUTOMATIC_FULL,
} FFT_AUTOMATIC_TYPE;

// ANT select
typedef enum {
	TRX_ANT_1,
	TRX_ANT_2,
	TRX_ANT_3,
	TRX_ANT_4,
} TRX_ANT;

// Save settings by band
typedef struct {
	uint64_t Freq;
	float32_t ATT_DB;
	uint16_t VFO_A_CW_LPF_Filter;
	uint16_t VFO_A_SSB_LPF_RX_Filter;
	uint16_t VFO_A_AM_LPF_RX_Filter;
	uint16_t VFO_A_FM_LPF_RX_Filter;
	uint16_t VFO_B_CW_LPF_Filter;
	uint16_t VFO_B_SSB_LPF_RX_Filter;
	uint16_t VFO_B_AM_LPF_RX_Filter;
	uint16_t VFO_B_FM_LPF_RX_Filter;
	int16_t FM_SQL_threshold_dBm;
	uint8_t ANT_RX;
	uint8_t ANT_TX;
	uint8_t Mode;
	uint8_t DNR_Type;
	uint8_t IF_Gain;
	uint8_t RF_Gain;
	uint8_t RF_Gain_By_Mode_CW;
	uint8_t RF_Gain_By_Mode_SSB;
	uint8_t RF_Gain_By_Mode_FM;
	uint8_t RF_Gain_By_Mode_AM;
	uint8_t RF_Gain_By_Mode_DIGI;
	bool LNA;
	bool ATT;
	bool ADC_Driver;
	bool ADC_PGA;
	bool AGC;
	bool RepeaterMode;
	bool SQL;
	bool Fast;
	TRX_IQ_SAMPLERATE_VALUE SAMPLERATE;
} BAND_SAVED_SETTINGS_TYPE;

// Save memory channels
typedef struct {
	uint64_t freq;
	float32_t CTCSS_Freq;
	uint8_t mode;
	bool RepeaterMode;
	char name[MAX_CHANNEL_MEMORY_NAME_LENGTH];
} CHANNEL_SAVED_SETTINGS_TYPE;

// VFO structure
typedef struct {
	uint64_t Freq;
	uint64_t SpectrumCenterFreq;
	uint64_t RXFreqAfterTransverters;
	int64_t SpectrumAndRXDiff;
	uint16_t HPF_RX_Filter_Width; // current width
	uint16_t HPF_TX_Filter_Width; // current width
	uint16_t LPF_RX_Filter_Width; // current width
	uint16_t LPF_TX_Filter_Width; // current width
	uint16_t CW_LPF_Filter;
	uint16_t DIGI_LPF_Filter;
	uint16_t DIGI_HPF_Filter;
	uint16_t SSB_LPF_RX_Filter;
	uint16_t SSB_LPF_TX_Filter;
	uint16_t SSB_HPF_RX_Filter;
	uint16_t SSB_HPF_TX_Filter;
	uint16_t AM_LPF_RX_Filter;
	uint16_t AM_LPF_TX_Filter;
	uint16_t FM_LPF_RX_Filter;
	uint16_t FM_LPF_TX_Filter;
	uint16_t FM_HPF_RX_Filter;
	uint16_t NotchFC;
	int16_t FM_SQL_threshold_dBm;
	uint8_t DNR_Type; // 0-disabled 1-dnr 2-dnr2
	TRX_MODE Mode;
	bool ManualNotchFilter;
	bool AutoNotchFilter;
	bool AGC;
	bool SQL;
	bool RepeaterMode;
} VFO;

extern struct TRX_SETTINGS {
	uint8_t flash_id; // version check

	VFO VFO_A;
	VFO VFO_B;

	float32_t ATT_DB;
	float32_t NOTCH_STEP_Hz;
	float32_t CTCSS_Freq;
	float32_t MIC_Gain_SSB_DB;
	float32_t MIC_Gain_AM_DB;
	float32_t MIC_Gain_FM_DB;
	float32_t CW_DotToDashRate;
	float32_t TX_CESSB_COMPRESS_DB;

	uint32_t FRQ_STEP_CW_Hz;
	uint32_t FRQ_STEP_SSB_Hz;
	uint32_t FRQ_STEP_DIGI_Hz;
	uint32_t FRQ_STEP_AM_Hz;
	uint32_t FRQ_STEP_FM_Hz;
	uint32_t FRQ_STEP_WFM_Hz;
	uint32_t SWR_CUSTOM_Begin;
	uint32_t SWR_CUSTOM_End;
	uint32_t SPEC_Begin;
	uint32_t SPEC_End;

	int16_t REPEATER_Offset;
	int16_t FFT_ManualBottom;
	int16_t FFT_ManualTop;
	int16_t SPEC_TopDBM;
	int16_t SPEC_BottomDBM;
	int16_t WSPR_FREQ_OFFSET;
	int16_t FM_SQL_threshold_dBm_shadow;

	uint16_t RIT_INTERVAL;
	uint16_t XIT_INTERVAL;
	uint16_t Volume;
	uint16_t RX_AGC_Hold_Time;
	uint16_t RX_AGC_Hold_Limiter;
	uint16_t RX_AGC_Hold_Step_Up;
	uint16_t RX_AGC_Hold_Step_Down;
	uint16_t VOX_TIMEOUT;
	uint16_t CW_Pitch;
	uint16_t CW_Key_timeout;
	uint16_t CW_SelfHear;
	uint16_t CW_KEYER_WPM;
	uint16_t LCD_SleepTimeout;
	uint16_t RTTY_Speed;
	uint16_t RTTY_Shift;
	uint16_t RTTY_Freq;
	uint16_t CW_LPF_Filter_shadow;
	uint16_t DIGI_LPF_Filter_shadow;
	uint16_t DIGI_HPF_Filter_shadow;
	uint16_t SSB_LPF_RX_Filter_shadow;
	uint16_t SSB_LPF_TX_Filter_shadow;
	uint16_t SSB_HPF_RX_Filter_shadow;
	uint16_t SSB_HPF_TX_Filter_shadow;
	uint16_t AM_LPF_RX_Filter_shadow;
	uint16_t AM_LPF_TX_Filter_shadow;
	uint16_t FM_LPF_RX_Filter_shadow;
	uint16_t FM_LPF_TX_Filter_shadow;
	uint16_t FM_HPF_RX_Filter_shadow;
	uint16_t NOTCH_Filter_width;
	uint16_t ATU_MEM_STEP_KHZ;

	int8_t MIC_NOISE_GATE;
	int8_t RX_EQ_P1;
	int8_t RX_EQ_P2;
	int8_t RX_EQ_P3;
	int8_t RX_EQ_P4;
	int8_t RX_EQ_P5;
	int8_t RX_EQ_P6;
	int8_t RX_EQ_P1_WFM;
	int8_t RX_EQ_P2_WFM;
	int8_t RX_EQ_P3_WFM;
	int8_t RX_EQ_P4_WFM;
	int8_t RX_EQ_P5_WFM;
	int8_t RX_EQ_P6_WFM;
	int8_t MIC_EQ_P1_SSB;
	int8_t MIC_EQ_P2_SSB;
	int8_t MIC_EQ_P3_SSB;
	int8_t MIC_EQ_P4_SSB;
	int8_t MIC_EQ_P5_SSB;
	int8_t MIC_EQ_P6_SSB;
	int8_t MIC_EQ_P1_AMFM;
	int8_t MIC_EQ_P2_AMFM;
	int8_t MIC_EQ_P3_AMFM;
	int8_t MIC_EQ_P4_AMFM;
	int8_t MIC_EQ_P5_AMFM;
	int8_t MIC_EQ_P6_AMFM;
	int8_t AGC_Gain_target_SSB;
	int8_t AGC_Gain_target_CW;
	int8_t VOX_THRESHOLD;
	int8_t FFT_FreqGrid;
	int8_t Dual_RX_AB_Balance;

	uint8_t ATT_STEP;
	uint8_t RF_Gain;
	uint8_t RF_Gain_By_Mode_CW;
	uint8_t RF_Gain_By_Mode_SSB;
	uint8_t RF_Gain_By_Mode_FM;
	uint8_t RF_Gain_By_Mode_AM;
	uint8_t RF_Gain_By_Mode_DIGI;
	uint8_t FAST_STEP_Multiplier;
	uint8_t ENC2_STEP_Multiplier;
	uint8_t ATU_I;
	uint8_t ATU_C;
	uint8_t CW_LPF_Stages;
	uint8_t SSB_LPF_Stages;
	uint8_t AMFM_LPF_Stages;
	uint8_t Volume_Step;
	uint8_t IF_Gain;
	uint8_t MIC_REVERBER;
	uint8_t DNR1_SNR_THRESHOLD;
	uint8_t DNR2_SNR_THRESHOLD;
	uint8_t DNR_AVERAGE;
	uint8_t DNR_MINIMAL;
	uint8_t VAD_THRESHOLD;
	uint8_t NOISE_BLANKER1_THRESHOLD;
	uint8_t NOISE_BLANKER2_THRESHOLD;
	uint8_t RX_AGC_SSB_speed;
	uint8_t RX_AGC_CW_speed;
	uint8_t RX_AGC_Max_gain;
	uint8_t TX_Compressor_speed_SSB;
	uint8_t TX_Compressor_maxgain_SSB;
	uint8_t TX_Compressor_speed_AMFM;
	uint8_t TX_Compressor_maxgain_AMFM;
	uint8_t SELFHEAR_Volume;
	uint8_t LINE_Volume;
	uint8_t CODEC_Out_Volume;
	uint8_t FM_Stereo_Modulation;
	uint8_t TROPO_Region;
	uint8_t RX_AUDIO_MODE;
	uint8_t CW_Iambic_Type;
	uint8_t ColorThemeId;
	uint8_t LayoutThemeId;
	uint8_t FFT_Zoom;
	uint8_t FFT_ZoomCW;
	uint8_t LCD_Brightness;
	uint8_t FFT_Sensitivity;
	uint8_t FFT_Speed;
	uint8_t FFT_Averaging;
	uint8_t FFT_Window;
	uint8_t FFT_Height;
	uint8_t FFT_Style;
	uint8_t FFT_BW_Style;
	uint8_t FFT_BW_Position;
	uint8_t FFT_Color;
	uint8_t WTF_Color;
	uint8_t FFT_3D;
	uint8_t FFT_DXCluster_Timeout;
	uint8_t FFT_Scale_Type;
	uint8_t RTTY_StopBits;
	uint8_t CW_Decoder_Threshold;
	uint8_t DNR_shadow;
	uint8_t FRONTPANEL_funcbuttons_page;
	uint8_t ANT_RX;
	uint8_t ANT_TX;
	uint8_t CW_EDGES_SMOOTH_MS;
	uint8_t FuncButtons[(FUNCBUTTONS_PAGES * FUNCBUTTONS_ON_PAGE)];

	DX_CLUSTER_TYPE DXCluster_Type;
	TRX_DEBUG_TYPE Debug_Type;
	TRX_IQ_SAMPLERATE_VALUE SAMPLERATE_MAIN;
	TRX_IQ_SAMPLERATE_VALUE SAMPLERATE_FM;
	TRX_INPUT_TYPE InputType_MAIN;
	TRX_INPUT_TYPE InputType_DIGI;
	CW_PTT_TYPE CW_PTT_Type;
	ENC2_FUNC_MODE ENC2_func_mode;
	FFT_AUTOMATIC_TYPE FFT_Automatic_Type;
#if HRDW_HAS_DUAL_RX
	DUAL_RX_TYPE Dual_RX_Type;
#endif

	bool NeedGoToBootloader;
	bool selected_vfo; // false - A; true - B
	bool Fast;
	bool LNA;
	bool ATT;
	bool RF_Filters;
	bool RF_Gain_For_Each_Band;
	bool RF_Gain_For_Each_Mode;
	bool ChannelMode;
	bool RIT_Enabled;
	bool XIT_Enabled;
	bool SPLIT_Enabled;
	bool FineRITTune;
	bool TWO_SIGNAL_TUNE;
	bool BandMapEnabled;
	bool AutoGain;
	bool Locked;
	bool Encoder_Accelerate;
	bool TUNER_Enabled;
	bool ATU_Enabled;
	bool ATU_T;
	bool Custom_Transverter_Enabled;
	bool Transverter_2m;
	bool Transverter_70cm;
	bool Transverter_23cm;
	bool Transverter_13cm;
	bool Transverter_6cm;
	bool Transverter_3cm;
	bool Transverter_QO100;
	bool Transverter_1_2cm;
	bool Auto_Input_Switch;
	bool Auto_Snap;
	bool Full_Duplex;
	bool Digital_Pre_Distortion;
	bool Split_Mode_Sync_Freq;
	bool FT8_Auto_CQ;
	bool Mute;
	bool AFAmp_Mute;
	bool MIC_Boost;
	bool BluetoothAudio_Enabled;
	bool NOISE_BLANKER1;
	bool NOISE_BLANKER2;
	bool Beeper;
	bool FM_Stereo;
	bool AGC_Threshold;
	bool VOX;
	bool TX_CESSB;
	bool AGC_Spectral;
	bool CW_KEYER;
	bool CW_OneSymbolMemory;
	bool CW_GaussFilter;
	bool CW_Iambic;
	bool CW_Invert;
	bool FFT_Enabled;
	bool WTF_Moving;
	bool FFT_Compressor;
	bool FFT_dBmGrid;
	bool FFT_Background;
	bool FFT_Lens;
	bool FFT_HoldPeaks;
	bool FFT_DXCluster;
	bool FFT_DXCluster_Azimuth;
	bool Show_Sec_VFO;
	bool AnalogMeterShowPWR;
	bool EnableBottomNavigationButtons;
	bool CW_Decoder;
	bool RDS_Decoder;
	bool RTTY_InvertBits;
	bool ADC_Driver;
	bool ADC_PGA;
	bool ADC_RAND;
	bool ADC_SHDN;
	bool ADC_DITH;
	bool WSPR_BANDS_160;
	bool WSPR_BANDS_80;
	bool WSPR_BANDS_40;
	bool WSPR_BANDS_30;
	bool WSPR_BANDS_20;
	bool WSPR_BANDS_17;
	bool WSPR_BANDS_15;
	bool WSPR_BANDS_12;
	bool WSPR_BANDS_10;
	bool WSPR_BANDS_6;
	bool WSPR_BANDS_2;
	bool SQL_shadow;
	bool AGC_shadow;
	bool Notch_on_shadow;
	bool RepeaterMode_shadow;
	bool WOLF_Cluster;
	bool FREE_Tune;
	bool CenterSpectrumAfterIdle;
	bool Auto_CW_Mode;
	bool CW_In_SSB;
	bool SatMode;
#if HRDW_HAS_DUAL_RX
	bool Dual_RX;
#endif

	char CALLSIGN[MAX_CALLSIGN_LENGTH + 1];
	char LOCATOR[MAX_CALLSIGN_LENGTH + 1];
	char URSI_CODE[MAX_CALLSIGN_LENGTH + 1];
	char CW_Macros_1[MAX_CW_MACROS_LENGTH + 1];
	char CW_Macros_2[MAX_CW_MACROS_LENGTH + 1];
	char CW_Macros_3[MAX_CW_MACROS_LENGTH + 1];
	char CW_Macros_4[MAX_CW_MACROS_LENGTH + 1];
	char CW_Macros_5[MAX_CW_MACROS_LENGTH + 1];
	char CW_Macros_Name_1[MAX_CW_MACROS_NAME_LENGTH + 1];
	char CW_Macros_Name_2[MAX_CW_MACROS_NAME_LENGTH + 1];
	char CW_Macros_Name_3[MAX_CW_MACROS_NAME_LENGTH + 1];
	char CW_Macros_Name_4[MAX_CW_MACROS_NAME_LENGTH + 1];
	char CW_Macros_Name_5[MAX_CW_MACROS_NAME_LENGTH + 1];
	char SAT_Name[SAT_NAME_MAXLEN + 1];
	char SAT_TLE_Line1[SAT_TLE_LINE_MAXLEN + 1];
	char SAT_TLE_Line2[SAT_TLE_LINE_MAXLEN + 1];
	char SAT_QTH_Lat[SAT_QTH_LINE_MAXLEN + 1];
	char SAT_QTH_Lon[SAT_QTH_LINE_MAXLEN + 1];
	char SAT_QTH_Alt[SAT_QTH_LINE_MAXLEN + 1];

	BAND_SAVED_SETTINGS_TYPE BANDS_SAVED_SETTINGS[BANDS_COUNT];

	uint8_t csum;   // check sum
	uint8_t ENDBit; // end bit
} TRX;

extern struct TRX_CALIBRATE {
	uint8_t flash_id; // version check

	float32_t SWR_FWD_Calibration_HF;
	float32_t SWR_BWD_Calibration_HF;
	float32_t SWR_FWD_Calibration_6M;
	float32_t SWR_BWD_Calibration_6M;
	float32_t SWR_FWD_Calibration_VHF;
	float32_t SWR_BWD_Calibration_VHF;
	float32_t INA226_VoltageOffset;
	float32_t INA226_Shunt_mOhm;
	float32_t PWR_VLT_Calibration;
	float32_t PWR_CUR_Calibration;
	uint32_t RFU_LPF_END;
	uint32_t RFU_HPF_START;
	uint32_t RFU_BPF_0_START; // UHF
	uint32_t RFU_BPF_0_END;   // UHF
	uint32_t RFU_BPF_1_START;
	uint32_t RFU_BPF_1_END;
	uint32_t RFU_BPF_2_START;
	uint32_t RFU_BPF_2_END;
	uint32_t RFU_BPF_3_START;
	uint32_t RFU_BPF_3_END;
	uint32_t RFU_BPF_4_START;
	uint32_t RFU_BPF_4_END;
	uint32_t RFU_BPF_5_START;
	uint32_t RFU_BPF_5_END;
	uint32_t RFU_BPF_6_START;
	uint32_t RFU_BPF_6_END;
	uint32_t RFU_BPF_7_START;
	uint32_t RFU_BPF_7_END;
	uint32_t RFU_BPF_8_START;
	uint32_t RFU_BPF_8_END;
	int16_t RTC_Calibration;
	int16_t VCXO_correction;
	int16_t smeter_calibration_hf;
	int16_t smeter_calibration_vhf;
	uint16_t TCXO_frequency;
	uint16_t MAX_ChargePump_Freq;
	uint16_t TX_StartDelay;
	uint16_t Transverter_Custom_Offset_MHz;
	uint16_t Transverter_2m_RF_MHz;
	uint16_t Transverter_2m_IF_MHz;
	uint16_t Transverter_70cm_RF_MHz;
	uint16_t Transverter_70cm_IF_MHz;
	uint16_t Transverter_23cm_RF_MHz;
	uint16_t Transverter_23cm_IF_MHz;
	uint16_t Transverter_13cm_RF_MHz;
	uint16_t Transverter_13cm_IF_MHz;
	uint16_t Transverter_6cm_RF_MHz;
	uint16_t Transverter_6cm_IF_MHz;
	uint16_t Transverter_3cm_RF_MHz;
	uint16_t Transverter_3cm_IF_MHz;
	uint16_t Transverter_1_2cm_RF_MHz;
	uint16_t Transverter_1_2cm_IF_MHz;
	uint32_t Transverter_QO100_RF_kHz;
	uint32_t Transverter_QO100_IF_RX_kHz;
	uint16_t Transverter_QO100_IF_TX_MHz;
	uint16_t KTY81_Calibration;
	uint16_t TOUCHPAD_TIMEOUT;
	uint16_t TOUCHPAD_CLICK_THRESHOLD;
	uint16_t TOUCHPAD_CLICK_TIMEOUT;
	uint16_t TOUCHPAD_HOLD_TIMEOUT;
	uint16_t TOUCHPAD_SWIPE_THRESHOLD_PX;
	int8_t LNA_compensation;
	uint8_t DAC_driver_mode;
	uint8_t rf_out_power_2200m;
	uint8_t rf_out_power_630m;
	uint8_t rf_out_power_160m;
	uint8_t rf_out_power_80m;
	uint8_t rf_out_power_60m;
	uint8_t rf_out_power_40m;
	uint8_t rf_out_power_30m;
	uint8_t rf_out_power_20m;
	uint8_t rf_out_power_17m;
	uint8_t rf_out_power_15m;
	uint8_t rf_out_power_12m;
	uint8_t rf_out_power_cb;
	uint8_t rf_out_power_10m;
	uint8_t rf_out_power_6m;
	uint8_t rf_out_power_4m;
	uint8_t rf_out_power_2m;
	uint8_t rf_out_power_70cm;
	uint8_t rf_out_power_23cm;
	uint8_t rf_out_power_13cm;
	uint8_t rf_out_power_6cm;
	uint8_t rf_out_power_3cm;
	uint8_t rf_out_power_QO100;
	uint8_t rf_out_power_1_2cm;
	uint8_t ENCODER_DEBOUNCE;
	uint8_t ENCODER2_DEBOUNCE;
	uint8_t ENCODER_SLOW_RATE;
	uint8_t ENCODER2_ON_FALLING;
	uint8_t CICFIR_GAINER_48K_val;
	uint8_t CICFIR_GAINER_96K_val;
	uint8_t CICFIR_GAINER_192K_val;
	uint8_t CICFIR_GAINER_384K_val;
	uint8_t TXCICFIR_GAINER_val;
	uint8_t DAC_GAINER_val;
	uint8_t MAX_RF_POWER_ON_METER;
	uint8_t ENCODER_ACCELERATION;
	uint8_t FAN_MEDIUM_START;
	uint8_t FAN_MEDIUM_STOP;
	uint8_t FAN_FULL_START;
	uint8_t TRX_MAX_RF_TEMP;
	uint8_t TRX_MAX_SWR;
	uint8_t FM_DEVIATION_SCALE;
	uint8_t SSB_POWER_ADDITION;
	uint8_t AM_MODULATION_INDEX;
	uint8_t TUNE_MAX_POWER;
	uint8_t RTC_Coarse_Calibration;
	uint8_t EXT_2200m;
	uint8_t EXT_630m;
	uint8_t EXT_160m;
	uint8_t EXT_80m;
	uint8_t EXT_60m;
	uint8_t EXT_40m;
	uint8_t EXT_30m;
	uint8_t EXT_20m;
	uint8_t EXT_17m;
	uint8_t EXT_15m;
	uint8_t EXT_12m;
	uint8_t EXT_CB;
	uint8_t EXT_10m;
	uint8_t EXT_6m;
	uint8_t EXT_4m;
	uint8_t EXT_FM;
	uint8_t EXT_2m;
	uint8_t EXT_70cm;
	uint8_t EXT_23cm;
	uint8_t EXT_13cm;
	uint8_t EXT_6cm;
	uint8_t EXT_3cm;
	uint8_t EXT_QO100;
	uint8_t EXT_1_2cm;
	uint8_t ATU_AVERAGING;
	uint8_t TwoSignalTune_Balance;
	uint8_t IF_GAIN_MIN;
	uint8_t IF_GAIN_MAX;
	uint8_t FAN_Medium_speed;
	TRX_RF_UNIT_TYPE RF_unit_type;
	TRX_TANGENT_TYPE TangentType;
	CAT_TYPE CAT_Type;
	COM_LINE_MODE COM_CAT_DTR_Mode;
	COM_LINE_MODE COM_CAT_RTS_Mode;
	COM_LINE_MODE COM_DEBUG_DTR_Mode;
	COM_LINE_MODE COM_DEBUG_RTS_Mode;
	bool ENCODER_INVERT;
	bool ENCODER2_INVERT;
	bool NOTX_NOTHAM;
	bool NOTX_2200m;
	bool NOTX_630m;
	bool NOTX_160m;
	bool NOTX_80m;
	bool NOTX_60m;
	bool NOTX_40m;
	bool NOTX_30m;
	bool NOTX_20m;
	bool NOTX_17m;
	bool NOTX_15m;
	bool NOTX_12m;
	bool NOTX_CB;
	bool NOTX_10m;
	bool NOTX_6m;
	bool NOTX_4m;
	bool NOTX_FM;
	bool NOTX_2m;
	bool NOTX_70cm;
	bool NOTX_23cm;
	bool ENABLE_2200m_band;
	bool ENABLE_630m_band;
	bool ENABLE_60m_band;
	bool ENABLE_6m_band;
	bool ENABLE_4m_band;
	bool ENABLE_FM_band;
	bool ENABLE_2m_band;
	bool ENABLE_AIR_band;
	bool ENABLE_marine_band;
	bool ENABLE_70cm_band;
	bool ENABLE_23cm_band;
	bool OTA_update;
	bool LCD_Rotate;
	bool TOUCHPAD_horizontal_flip;
	bool TOUCHPAD_vertical_flip;
	bool INA226_EN;
	bool LinearPowerControl;
	bool ALC_Port_Enabled;
	bool ALC_Inverted_Logic;
	bool Swap_USB_IQ;
	bool ATT_compensation;
	CHANNEL_SAVED_SETTINGS_TYPE MEMORY_CHANNELS[MEMORY_CHANNELS_COUNT];
	uint32_t BAND_MEMORIES[BANDS_COUNT][BANDS_MEMORIES_COUNT];

	uint8_t csum;   // check sum
	uint8_t ENDBit; // end bit
} CALIBRATE;

extern struct TRX_WIFI {
	uint8_t flash_id; // version check
	// WIFI
	char AP_1[MAX_WIFIPASS_LENGTH + 1];
	char Password_1[MAX_WIFIPASS_LENGTH + 1];
	char AP_2[MAX_WIFIPASS_LENGTH + 1];
	char Password_2[MAX_WIFIPASS_LENGTH + 1];
	char AP_3[MAX_WIFIPASS_LENGTH + 1];
	char Password_3[MAX_WIFIPASS_LENGTH + 1];
	char ALLQSO_TOKEN[ALLQSO_TOKEN_SIZE + 1];
	char ALLQSO_LOGID[ALLQSO_TOKEN_SIZE + 1];
	float32_t Timezone;
	bool Enabled;
	bool CAT_Server;
	uint8_t csum;   // check sum
	uint8_t ENDBit; // end bit
} WIFI;

extern volatile bool NeedSaveSettings;
extern volatile bool NeedSaveCalibration;
extern volatile bool NeedSaveWiFi;
extern volatile bool EEPROM_Busy;
extern VFO *CurrentVFO;
extern VFO *SecondaryVFO;
extern bool EEPROM_Enabled;

extern void LoadSettings(bool clear);
extern void LoadCalibration(bool clear);
extern void LoadWiFiSettings(bool clear);
extern void SaveSettings(void);
extern void SaveCalibration(void);
extern void SaveWiFiSettings(void);
extern void SaveSettingsToEEPROM(void);
extern void BKPSRAM_Enable(void);
extern void BKPSRAM_Disable(void);
extern void RTC_Calibration(void);
extern bool LoadDPDSettings(uint8_t *out, uint32_t size, uint32_t sector_offset);
extern bool SaveDPDSettings(uint8_t *in, uint32_t size, uint32_t sector_offset);
extern bool LoadATUSettings(uint8_t *out, uint32_t size, uint32_t sector);
extern bool SaveATUSettings(uint8_t *in, uint32_t size, uint32_t sector);

#endif
