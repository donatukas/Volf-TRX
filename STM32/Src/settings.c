#include "settings.h"
#include "atu.h"
#include "audio_filters.h"
#include "bands.h"
#include "fpga.h"
#include "front_unit.h"
#include "functions.h"
#include "hardware.h"
#include "lcd.h"
#include "main.h"
#include "trx_manager.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

// W25Q16
IRAM2 static uint8_t Write_Enable = W25Q16_COMMAND_Write_Enable;
IRAM2 static uint8_t Power_Down = W25Q16_COMMAND_Power_Down;
IRAM2 static uint8_t Get_Status = W25Q16_COMMAND_GetStatus;
IRAM2 static uint8_t Power_Up = W25Q16_COMMAND_Power_Up;

IRAM2 static uint8_t CmdAddress[4];
IRAM2_ON_F407 struct TRX_SETTINGS TRX;
IRAM2_ON_F407 struct TRX_CALIBRATE CALIBRATE = {0};
IRAM2_ON_F407 struct TRX_WIFI WIFI = {0};
bool EEPROM_Enabled = true;

#define MAX_CLONE_SIZE_SETTINGS sizeof(CALIBRATE) > sizeof(TRX) ? sizeof(CALIBRATE) : sizeof(TRX)
#define MAX_CLONE_SIZE sizeof(ATU_MEMORY_TYPE) > MAX_CLONE_SIZE_SETTINGS ? sizeof(ATU_MEMORY_TYPE) : MAX_CLONE_SIZE_SETTINGS

static_assert(sizeof(TRX) < W25Q16_SECTOR_SIZE, "TRX Data structure doesn't match page size");
static_assert(sizeof(CALIBRATE) < W25Q16_SECTOR_SIZE, "CALIBRATE Data structure doesn't match page size");
static_assert(sizeof(ATU_MEMORY_TYPE) < W25Q16_SECTOR_SIZE, "ATU_MEMORY_TYPE Data structure doesn't match page size");

IRAM2 static uint8_t verify_clone[MAX_CLONE_SIZE] = {0};

volatile bool NeedSaveSettings = false;
volatile bool NeedSaveCalibration = false;
volatile bool NeedSaveWiFi = false;
volatile bool EEPROM_Busy = false;
VFO *CurrentVFO = &TRX.VFO_A;
VFO *SecondaryVFO = &TRX.VFO_B;

static void LoadSettingsFromEEPROM(void);
static bool EEPROM_Sector_Erase(uint8_t sector, bool force);
static bool EEPROM_Write_Data(uint8_t *Buffer, uint16_t size, uint8_t sector, bool verify, bool force);
static bool EEPROM_Read_Data(uint8_t *Buffer, uint16_t size, uint8_t sector, bool verif, bool force);
static void EEPROM_PowerDown(void);
static void EEPROM_PowerUp(void);
static void EEPROM_WaitWrite(void);
static uint8_t calculateCSUM(void);
static uint8_t calculateCSUM_EEPROM(void);
static uint8_t calculateCSUM_WIFI(void);

const char *MODE_DESCR[TRX_MODE_COUNT] = {
    "LSB", "USB", "CW", "NFM", "WFM", "AM", "SAM", "AM-L", "AM-U", "DIGL", "DIGU", "IQ", "LOOP", "RTTY",
};

void LoadSettings(bool clear) {
	BKPSRAM_Enable();
	memcpy(&TRX, BACKUP_SRAM_BANK_ADDR, sizeof(TRX));
	// Check, the data in the backup sram is correct, otherwise we use the second bank
	if (TRX.ENDBit != 100 || TRX.flash_id != SETT_VERSION || TRX.csum != calculateCSUM()) {
		println("[ERR] BACKUP SRAM data incorrect");

		LoadSettingsFromEEPROM();
		if (TRX.ENDBit != 100 || TRX.flash_id != SETT_VERSION || TRX.csum != calculateCSUM()) {
			println("[ERR] EEPROM Settings data incorrect");
		} else {
			println("[OK] Settings data succesfully loaded from EEPROM");
		}
	} else {
		println("[OK] Settings data succesfully loaded from BACKUP SRAM");
	}
	BKPSRAM_Disable();

	if (TRX.flash_id != SETT_VERSION || clear || TRX.ENDBit != 100 || TRX.csum != calculateCSUM()) // code to trace new clean flash
	{
		if (clear) {
			println("[OK] Soft reset TRX");
		}
		memset(&TRX, 0x00, sizeof(TRX));
		//
		TRX.flash_id = SETT_VERSION; // Firmware ID in SRAM, if it doesn't match, use the default
		TRX.NeedGoToBootloader = false;
		// TRX
		TRX.selected_vfo = false;                                        // current VFO (false - A)
		TRX.VFO_A.Freq = 7100000;                                        // stored VFO-A frequency
		TRX.VFO_A.SpectrumCenterFreq = TRX.VFO_A.Freq;                   // FFT spectrum center frequency
		TRX.VFO_A.Mode = TRX_MODE_LSB;                                   // saved VFO-A mode
		TRX.VFO_A.LPF_RX_Filter_Width = 2700;                            // saved bandwidth for VFO-A
		TRX.VFO_A.LPF_TX_Filter_Width = 2700;                            // saved bandwidth for VFO-A
		TRX.VFO_A.HPF_RX_Filter_Width = 200;                             // saved bandwidth for VFO-A
		TRX.VFO_A.HPF_TX_Filter_Width = 200;                             // saved bandwidth for VFO-A
		TRX.VFO_A.ManualNotchFilter = false;                             // notch filter to cut out noise
		TRX.VFO_A.AutoNotchFilter = false;                               // notch filter to cut out noise
		TRX.VFO_A.NotchFC = 1000;                                        // cutoff frequency of the notch filter
		TRX.VFO_A.DNR_Type = 0;                                          // digital noise reduction
		TRX.VFO_A.AGC = true;                                            // AGC
		TRX.VFO_A.SQL = false;                                           // SSB/FM Squelch
		TRX.VFO_A.RepeaterMode = false;                                  // Repeater mode
		TRX.VFO_A.FM_SQL_threshold_dBm = -90;                            // FM noise squelch
		TRX.VFO_A.CW_LPF_Filter = 600;                                   // default value of CW filter width
		TRX.VFO_A.DIGI_LPF_Filter = 3000;                                // default value of DIGI filter width
		TRX.VFO_A.DIGI_HPF_Filter = 0;                                   // default value of DIGI filter width
		TRX.VFO_A.SSB_LPF_RX_Filter = 2700;                              // default value of SSB filter width
		TRX.VFO_A.SSB_LPF_TX_Filter = 2700;                              // default value of SSB filter width
		TRX.VFO_A.SSB_HPF_RX_Filter = 200;                               // default value of SSB filter width
		TRX.VFO_A.SSB_HPF_TX_Filter = 200;                               // default value of SSB filter width
		TRX.VFO_A.AM_LPF_RX_Filter = 8000;                               // default value of AM filter width
		TRX.VFO_A.AM_LPF_TX_Filter = 8000;                               // default value of AM filter width
		TRX.VFO_A.FM_LPF_RX_Filter = 8000;                               // default value of the FM filter width
		TRX.VFO_A.FM_LPF_TX_Filter = 8000;                               // default value of the FM filter width
		TRX.VFO_A.FM_HPF_RX_Filter = 0;                                  // default value of the FM filter width
		TRX.VFO_B.Freq = 14150000;                                       // stored VFO-B frequency
		TRX.VFO_B.SpectrumCenterFreq = TRX.VFO_B.Freq;                   // FFT spectrum center frequency
		TRX.VFO_B.Mode = TRX_MODE_USB;                                   // saved VFO-B mode
		TRX.VFO_B.LPF_RX_Filter_Width = TRX.VFO_A.LPF_RX_Filter_Width;   // saved bandwidth for VFO-B
		TRX.VFO_B.LPF_TX_Filter_Width = TRX.VFO_A.LPF_TX_Filter_Width;   // saved bandwidth for VFO-B
		TRX.VFO_B.HPF_RX_Filter_Width = TRX.VFO_A.HPF_RX_Filter_Width;   // saved bandwidth for VFO-B
		TRX.VFO_B.HPF_TX_Filter_Width = TRX.VFO_A.HPF_TX_Filter_Width;   // saved bandwidth for VFO-B
		TRX.VFO_B.ManualNotchFilter = TRX.VFO_A.ManualNotchFilter;       // notch filter to cut out noise
		TRX.VFO_B.AutoNotchFilter = TRX.VFO_A.AutoNotchFilter;           // notch filter to cut out noise
		TRX.VFO_B.NotchFC = TRX.VFO_A.NotchFC;                           // cutoff frequency of the notch filter
		TRX.VFO_B.DNR_Type = TRX.VFO_A.DNR_Type;                         // digital noise reduction
		TRX.VFO_B.AGC = TRX.VFO_A.AGC;                                   // AGC
		TRX.VFO_B.SQL = TRX.VFO_A.SQL;                                   // SSB/FM Squelch
		TRX.VFO_B.FM_SQL_threshold_dBm = TRX.VFO_A.FM_SQL_threshold_dBm; // FM noise squelch
		TRX.VFO_B.RepeaterMode = TRX.VFO_A.RepeaterMode;                 // Repeater mode
		TRX.VFO_B.CW_LPF_Filter = TRX.VFO_A.CW_LPF_Filter;               // default value of CW filter width
		TRX.VFO_B.DIGI_LPF_Filter = TRX.VFO_A.DIGI_LPF_Filter;           // default value of DIGI filter width
		TRX.VFO_B.DIGI_HPF_Filter = TRX.VFO_A.DIGI_HPF_Filter;           // default value of DIGI filter width
		TRX.VFO_B.SSB_LPF_RX_Filter = TRX.VFO_A.SSB_LPF_RX_Filter;       // default value of SSB filter width
		TRX.VFO_B.SSB_LPF_TX_Filter = TRX.VFO_A.SSB_LPF_TX_Filter;       // default value of SSB filter width
		TRX.VFO_B.SSB_HPF_RX_Filter = TRX.VFO_A.SSB_HPF_RX_Filter;       // default value of SSB filter width
		TRX.VFO_B.SSB_HPF_TX_Filter = TRX.VFO_A.SSB_HPF_TX_Filter;       // default value of SSB filter width
		TRX.VFO_B.AM_LPF_RX_Filter = TRX.VFO_A.AM_LPF_RX_Filter;         // default value of AM filter width
		TRX.VFO_B.AM_LPF_TX_Filter = TRX.VFO_A.AM_LPF_TX_Filter;         // default value of AM filter width
		TRX.VFO_B.FM_LPF_RX_Filter = TRX.VFO_A.FM_LPF_RX_Filter;         // default value of the FM filter width
		TRX.VFO_B.FM_LPF_TX_Filter = TRX.VFO_A.FM_LPF_TX_Filter;         // default value of the FM filter width
		TRX.VFO_B.FM_HPF_RX_Filter = TRX.VFO_A.FM_HPF_RX_Filter;         // default value of the FM filter width
		TRX.Fast = true;                                                 // accelerated frequency change when the encoder rotates
		TRX.LNA = false;                                                 // LNA (Low Noise Amplifier)
		TRX.ATT = false;                                                 // attenuator
		TRX.ATT_DB = 12.0f;                                              // suppress the attenuator
		TRX.ATT_STEP = 6.0f;                                             // step of tuning the attenuator
		TRX.RF_Filters = true;                                           // LPF / HPF / BPF
		TRX.ANT_RX = TRX_ANT_1;                                          // ANT-1 RX
		TRX.ANT_TX = TRX_ANT_1;                                          // ANT-1 TX
		TRX.RF_Gain = 20;                                                // output power (%)
		TRX.RF_Gain_For_Each_Band = false;                               // save RF Gain for each band separatly
		TRX.RF_Gain_For_Each_Mode = false;                               // save RF Gain for each mode separatly
		TRX.ChannelMode = false;                                         // enable channel mode on VFO
		TRX.RIT_Enabled = false;                                         // activate the SHIFT mode
		TRX.XIT_Enabled = false;                                         // activate the SPLIT mode
		TRX.RIT_INTERVAL = 1000;                                         // Detune range with the SHIFT knob (5000 = -5000Hz / + 5000Hz)
		TRX.XIT_INTERVAL = 1000;                                         // Detune range with the SPLIT knob (5000 = -5000Hz / + 5000Hz)
		TRX.TWO_SIGNAL_TUNE = false;                                     // Two-signal generator in TUNE mode (1 + 2kHz)
#ifdef LAY_160x128
		TRX.SAMPLERATE_MAIN = TRX_SAMPLERATE_K48; // Samplerate for ssb/cw/digi/nfm/etc modes
		TRX.SAMPLERATE_FM = TRX_SAMPLERATE_K192;  // Samplerate for FM mode
#endif
#ifdef LAY_480x320
		TRX.SAMPLERATE_MAIN = TRX_SAMPLERATE_K48; // Samplerate for ssb/cw/digi/nfm/etc modes
		TRX.SAMPLERATE_FM = TRX_SAMPLERATE_K192;  // Samplerate for FM mode
#endif
#ifdef LAY_320x240
		TRX.SAMPLERATE_MAIN = TRX_SAMPLERATE_K48; // Samplerate for ssb/cw/digi/nfm/etc modes
		TRX.SAMPLERATE_FM = TRX_SAMPLERATE_K192;  // Samplerate for FM mode
#endif
#ifdef LAY_800x480
		TRX.SAMPLERATE_MAIN = TRX_SAMPLERATE_K96; // Samplerate for ssb/cw/digi/nfm/etc modes
		TRX.SAMPLERATE_FM = TRX_SAMPLERATE_K192;  // Samplerate for FM mode
#endif
#ifdef STM32F407xx
		TRX.SAMPLERATE_MAIN = TRX_SAMPLERATE_K48; // Samplerate for ssb/cw/digi/nfm/etc modes
		TRX.SAMPLERATE_FM = TRX_SAMPLERATE_K48;   // Samplerate for FM mode
#endif
		TRX.FRQ_STEP_CW_Hz = 2;             // frequency tuning step CW
		TRX.FRQ_STEP_SSB_Hz = 10;           // frequency tuning step SSB
		TRX.FRQ_STEP_DIGI_Hz = 10;          // frequency tuning step DIGI
		TRX.FRQ_STEP_AM_Hz = 20;            // frequency tuning step AM
		TRX.FRQ_STEP_FM_Hz = 25;            // frequency tuning step FM
		TRX.FRQ_STEP_WFM_Hz = 1000;         // frequency tuning step WFM
		TRX.FAST_STEP_Multiplier = 10;      // Multiplier for FAST mode
		TRX.ENC2_STEP_Multiplier = 50;      // Multiplier for ENC2 rotate
		TRX.NOTCH_STEP_Hz = 50;             // Manual NOTCH tuning step
		TRX.Debug_Type = TRX_DEBUG_OFF;     // Debug output to DEBUG / UART port
		TRX.BandMapEnabled = true;          // automatic change of mode according to the range map
		TRX.InputType_MAIN = TRX_INPUT_MIC; // type of input to transfer (SSB/FM/AM)
		TRX.InputType_DIGI = TRX_INPUT_USB; // type of input to transfer (DIGI)
#ifdef FRONTPANEL_KT_100S
		TRX.AutoGain = false; // auto-control preamp and attenuator
#else
		TRX.AutoGain = true;    // auto-control preamp and attenuator
#endif
		TRX.Locked = false;        // Lock control
		TRX.SPLIT_Enabled = false; // Split frequency mode (receive one VFO, transmit another)
#if HRDW_HAS_DUAL_RX
		TRX.Dual_RX = false;            // Dual RX feature
		TRX.Dual_RX_Type = VFO_A_AND_B; // dual receiver mode
		TRX.Dual_RX_AB_Balance = 0;     // A/B Balance
#endif
		TRX.Encoder_Accelerate = true;             // Accelerate Encoder on fast rate
		strcpy(TRX.CALLSIGN, "HamRad");            // Callsign
		strcpy(TRX.LOCATOR, "LO02RR");             // Locator
		strcpy(TRX.URSI_CODE, "SO148");            // URSI Ionogramm location CODE https://digisonde.com/index.html#stationmap-section
		TRX.TROPO_Region = 0;                      // Eastern europe by default
		TRX.Custom_Transverter_Enabled = false;    // Enable transverter mode
		TRX.ATU_I = 0;                             // ATU default state
		TRX.ATU_C = 0;                             // ATU default state
		TRX.ATU_T = false;                         // ATU default state
		TRX.ATU_Enabled = true;                    // ATU enabled state
		TRX.TUNER_Enabled = true;                  // TUNER enabled state
		TRX.ATU_MEM_STEP_KHZ = 50;                 // ATU save step
		TRX.Transverter_2m = false;                // Transvertrs enable
		TRX.Transverter_70cm = false;              // Transvertrs enable
		TRX.Transverter_23cm = false;              // Transvertrs enable
		TRX.Transverter_13cm = false;              // Transvertrs enable
		TRX.Transverter_6cm = false;               // Transvertrs enable
		TRX.Transverter_3cm = false;               // Transvertrs enable
		TRX.Transverter_QO100 = false;             // Transvertrs enable
		TRX.Transverter_1_2cm = false;             // Transvertrs enable
		TRX.FineRITTune = true;                    // Fine or coarse tune for split/shift
		TRX.Auto_Input_Switch = false;             // Auto Mic/USB Switch
		TRX.Auto_Snap = false;                     // Auto track and snap to signal frequency
		TRX.Full_Duplex = false;                   // Full Duplex TX+RX Mode
		TRX.Digital_Pre_Distortion = false;        // enable digital pre-distortion
		TRX.Split_Mode_Sync_Freq = false;          // enable VFO freq syncronization in SPLIT mode
		TRX.FT8_Auto_CQ = false;                   // Auto-CQ mode for FT8
		TRX.DXCluster_Type = DX_CLUSTER_DX_SUMMIT; // DX cluster type
		TRX.REPEATER_Offset = -600;                // repeater mode custom offset
		TRX.WOLF_Cluster = true;                   // Enable WOLF cluster
		TRX.FREE_Tune = false;                     // Enable free tune on spectrum bandwidth
		// AUDIO
		TRX.Volume = 25;                                               // AF Volume
		TRX.Volume_Step = 5;                                           // AF Volume step by sec encoder
		TRX.Mute = false;                                              // AF Mute
		TRX.AFAmp_Mute = false;                                        // AF Amp Mute
		TRX.IF_Gain = 20;                                              // IF gain, dB (before all processing and AGC)
		TRX.AGC_Gain_target_SSB = -30;                                 // Maximum (target) AGC gain SSB
		TRX.AGC_Gain_target_CW = -35;                                  // Maximum (target) AGC gain CW
		TRX.MIC_Gain_SSB_DB = 9.0f;                                    // Microphone gain, dB SSB modes
		TRX.MIC_Gain_AM_DB = 9.0f;                                     // Microphone gain, dB AM mode
		TRX.MIC_Gain_FM_DB = 9.0f;                                     // Microphone gain, dB FM mode
		TRX.MIC_Boost = false;                                         // +20dB mic amplifier
		TRX.LINE_Volume = 23;                                          // Line input level
		TRX.CODEC_Out_Volume = 121;                                    // Codec headphone level
		TRX.BluetoothAudio_Enabled = false;                            // Bluetooth audio on/off
		TRX.MIC_NOISE_GATE = -120;                                     // Mic noise gate
		TRX.RX_EQ_P1 = 0;                                              // Receiver Equalizer 300Hz
		TRX.RX_EQ_P2 = 0;                                              // Receiver Equalizer 700Hz
		TRX.RX_EQ_P3 = 0;                                              // Receiver Equalizer 1200Hz
		TRX.RX_EQ_P4 = 0;                                              // Receiver Equalizer 1800Hz
		TRX.RX_EQ_P5 = 0;                                              // Receiver Equalizer 2000Hz
		TRX.RX_EQ_P6 = 0;                                              // Receiver Equalizer 2500Hz
		TRX.RX_EQ_P1_WFM = 0;                                          // Receiver WFM Equalizer 50Hz
		TRX.RX_EQ_P2_WFM = 0;                                          // Receiver WFM Equalizer 300Hz
		TRX.RX_EQ_P3_WFM = 0;                                          // Receiver WFM Equalizer 1500Hz
		TRX.RX_EQ_P4_WFM = 0;                                          // Receiver WFM Equalizer 5000Hz
		TRX.RX_EQ_P5_WFM = 0;                                          // Receiver WFM Equalizer 8000Hz
		TRX.RX_EQ_P6_WFM = 0;                                          // Receiver WFM Equalizer 12000Hz
		TRX.MIC_EQ_P1_SSB = 0;                                         // Mic EQ SSB
		TRX.MIC_EQ_P2_SSB = 0;                                         // Mic EQ SSB
		TRX.MIC_EQ_P3_SSB = 0;                                         // Mic EQ SSB
		TRX.MIC_EQ_P4_SSB = 0;                                         // Mic EQ SSB
		TRX.MIC_EQ_P5_SSB = 0;                                         // Mic EQ SSB
		TRX.MIC_EQ_P6_SSB = 0;                                         // Mic EQ SSB
		TRX.MIC_EQ_P1_AMFM = 0;                                        // Mic EQ AM/FM
		TRX.MIC_EQ_P2_AMFM = 0;                                        // Mic EQ AM/FM
		TRX.MIC_EQ_P3_AMFM = 0;                                        // Mic EQ AM/FM
		TRX.MIC_EQ_P4_AMFM = 0;                                        // Mic EQ AM/FM
		TRX.MIC_EQ_P5_AMFM = 0;                                        // Mic EQ AM/FM
		TRX.MIC_EQ_P6_AMFM = 0;                                        // Mic EQ AM/FM
		TRX.MIC_REVERBER = 0;                                          // Mic Reveerber
		TRX.DNR1_SNR_THRESHOLD = 50;                                   // Digital noise reduction 1 level
		TRX.DNR2_SNR_THRESHOLD = 35;                                   // Digital noise reduction 2 level
		TRX.DNR_AVERAGE = 2;                                           // DNR averaging when looking for average magnitude
		TRX.DNR_MINIMAL = 99;                                          // DNR averaging when searching for minimum magnitude
		TRX.NOISE_BLANKER1 = false;                                    // suppressor of short impulse noise NOISE BLANKER 1
		TRX.NOISE_BLANKER2 = false;                                    // suppressor of short impulse noise NOISE BLANKER 2
		TRX.AGC_Spectral = false;                                      // Spectral AGC mode
		TRX.NOISE_BLANKER1_THRESHOLD = 6;                              // threshold for noise blanker 1
		TRX.NOISE_BLANKER2_THRESHOLD = 16;                             // threshold for noise blanker 2
		TRX.TX_CESSB = true;                                           // Controlled-envelope single-sideband modulation
		TRX.TX_CESSB_COMPRESS_DB = 0.1f;                               // CESSB additional gain
		TRX.RX_AGC_SSB_speed = 10;                                     // AGC receive rate on SSB
		TRX.RX_AGC_CW_speed = 1;                                       // AGC receive rate on CW
		TRX.RX_AGC_Max_gain = 30;                                      // Maximum AGC gain
		TRX.RX_AGC_Hold_Time = 300;                                    // AGC Hold time on peaks
		TRX.RX_AGC_Hold_Limiter = 5;                                   // AGC Hold time limiter for skip peaks
		TRX.RX_AGC_Hold_Step_Up = 95;                                  // AGC Hold step for increase holding
		TRX.RX_AGC_Hold_Step_Down = 60;                                // AGC Hold step for decrease holding
		TRX.TX_Compressor_speed_SSB = 3;                               // TX compressor speed SSB
		TRX.TX_Compressor_maxgain_SSB = 10;                            // TX compressor max gain SSB
		TRX.TX_Compressor_speed_AMFM = 3;                              // TX compressor speed AM/FM
		TRX.TX_Compressor_maxgain_AMFM = 10;                           // TX compressor max gain AM/FM
		TRX.CW_LPF_Stages = IIR_LPF_STAGES < 10 ? IIR_LPF_STAGES : 10; // stages for CW LPF filter
		TRX.SSB_LPF_Stages = IIR_LPF_STAGES;                           // stages for SSB LPF filter
		TRX.AMFM_LPF_Stages = 3;                                       // stages for NFM LPF filter
		TRX.Beeper = true;                                             // Keyboard beeper
		TRX.CTCSS_Freq = 0;                                            // CTCSS FM Frequency
		TRX.SELFHEAR_Volume = 40;                                      // Selfhearing volume
		TRX.FM_Stereo = false;                                         // Stereo FM Mode
		TRX.FM_Stereo_Modulation = 40;                                 // Stereo FM Sub-carrier modulation
		TRX.VAD_THRESHOLD = 150;                                       // Threshold of SSB/SCAN squelch
		TRX.VOX = false;                                               // TX by voice activation
		TRX.VOX_TIMEOUT = 300;                                         // VOX timeout in ms
		TRX.VOX_THRESHOLD = -27;                                       // VOX threshold in dbFS
		TRX.RX_AUDIO_MODE = RX_AUDIO_MODE_STEREO;                      // OUT Lines mode stereo/left/right
		TRX.AGC_Threshold = false;                                     // Disable AGC on noise signals
		TRX.NOTCH_Filter_width = 100;                                  // Manual notch filter width
		// CW
		TRX.CW_Pitch = 600;                                             // LO offset in CW mode
		TRX.CW_Key_timeout = 200;                                       // time of releasing transmission after the last character on the key
		TRX.CW_SelfHear = true;                                         // self-control CW
		TRX.CW_KEYER = true;                                            // Automatic key
		TRX.CW_OneSymbolMemory = true;                                  // One symbol memory for automatic key
		TRX.CW_KEYER_WPM = 25;                                          // Automatic key speed
		TRX.CW_GaussFilter = true;                                      // Gauss responce LPF filter
		TRX.CW_DotToDashRate = 3.0f;                                    // Dot To Dash length rate
		TRX.CW_Iambic = false;                                          // CW Iambic Keyer
		TRX.CW_Iambic_Type = 1;                                         // CW Iambic Keyer type 0 - A, 1 - B
		TRX.CW_Invert = false;                                          // CW dash/dot inversion
		TRX.Auto_CW_Mode = false;                                       // Switch to CW mode if key pressed
		TRX.CW_In_SSB = false;                                          // CW manipulation on SSB mode
		TRX.CW_PTT_Type = KEY_AND_EXT_PTT;                              // CW PTT type (Key / External tangent ptt)
		TRX.CW_EDGES_SMOOTH_MS = 5;                                     // CW Signal edges smooting
		strcpy(TRX.CW_Macros_1, "CQ CQ CQ");                            // CW Macros 1
		strcpy(TRX.CW_Macros_2, "CQ TEST");                             // CW Macros 2
		strcpy(TRX.CW_Macros_3, "TNX RST 599 599 NAME QTH K");          // CW Macros 3
		strcpy(TRX.CW_Macros_4, "73 73 SK K");                          // CW Macros 4
		strcpy(TRX.CW_Macros_5, "TNX FOR NICE QSO DR OM CU AGN GL 73"); // CW Macros 5
		strcpy(TRX.CW_Macros_Name_1, "MACR1");                          // CW Macros 1 name
		strcpy(TRX.CW_Macros_Name_2, "MACR2");                          // CW Macros 2 name
		strcpy(TRX.CW_Macros_Name_3, "MACR3");                          // CW Macros 3 name
		strcpy(TRX.CW_Macros_Name_4, "MACR4");                          // CW Macros 4 name
		strcpy(TRX.CW_Macros_Name_5, "MACR5");                          // CW Macros 5 name
		// SCREEN
		TRX.ColorThemeId = 0;  // Selected Color theme
		TRX.LayoutThemeId = 0; // Selected Layout theme
#ifdef LAY_800x480
		TRX.ColorThemeId = 3;
		TRX.LayoutThemeId = 7;
#endif
		TRX.FFT_Enabled = true; // use FFT spectrum
#ifdef LAY_160x128
		TRX.FFT_Zoom = 1;   // approximation of the FFT spectrum
		TRX.FFT_ZoomCW = 8; // zoomfft for cw mode
#endif
#ifdef LAY_480x320
		TRX.FFT_Zoom = 1;   // approximation of the FFT spectrum
		TRX.FFT_ZoomCW = 8; // zoomfft for cw mode
#endif
#ifdef LAY_320x240
		TRX.FFT_Zoom = 1;   // approximation of the FFT spectrum
		TRX.FFT_ZoomCW = 8; // zoomfft for cw mode
#endif
#ifdef LAY_800x480
		TRX.FFT_Zoom = 1;   // approximation of the FFT spectrum
		TRX.FFT_ZoomCW = 8; // zoomfft for cw mode
#endif
#ifdef STM32F407xx
		TRX.FFT_Zoom = 1;   // approximation of the FFT spectrum
		TRX.FFT_ZoomCW = 8; // zoomfft for cw mode
#endif
		TRX.LCD_Brightness = 60;                     // LCD Brightness
		TRX.LCD_SleepTimeout = 0;                    // LCD Sleep Timeout berfore idle
		TRX.WTF_Moving = true;                       // move WTF with frequency encoder
		TRX.FFT_Automatic_Type = FFT_AUTOMATIC_FULL; // Automatic FFT Scale type
		TRX.FFT_Sensitivity = 8;                     // Threshold of FFT autocalibrate
		TRX.FFT_Speed = 3;                           // FFT Speed
		TRX.FFT_Averaging = 10;                      // averaging the FFT to make it smoother
#ifdef STM32F407xx
		TRX.FFT_Averaging = 6;
#endif
		TRX.FFT_Window = 1;        // FFT Window
		TRX.FFT_Style = 5;         // FFT style
		TRX.FFT_BW_Style = 2;      // FFT BW style
		TRX.FFT_BW_Position = 0;   // FFT BW position
		TRX.FFT_Color = 0;         // FFT display color
		TRX.WTF_Color = 1;         // WTF display color
		TRX.FFT_Compressor = true; // Compress FFT Peaks
		TRX.FFT_Background = true; // FFT gradient background
#ifdef LAY_160x128
		TRX.FFT_FreqGrid = 0;       // FFT freq grid style
		TRX.FFT_Height = 4;         // FFT display height
		TRX.FFT_Background = false; // FFT gradient background
#elif defined LAY_320x240
		TRX.FFT_FreqGrid = 0;   // FFT freq grid style
		TRX.FFT_Height = 3;     // FFT display height
#else
		TRX.FFT_FreqGrid = 1;                          // FFT freq grid style
		TRX.FFT_Height = 2;                            // FFT display height
#endif
		TRX.FFT_dBmGrid = false;                  // FFT power grid
		TRX.FFT_Lens = false;                     // FFT lens effect
		TRX.FFT_HoldPeaks = false;                // Show FFT Peaks
		TRX.FFT_3D = 0;                           // FFT 3D mode
		TRX.FFT_ManualBottom = -130;              // Minimal threshold for manual FFT scale
		TRX.FFT_ManualTop = -90;                  // Maximum threshold for manual FFT scale
		TRX.FFT_DXCluster = true;                 // Show DX cluster over FFT
		TRX.FFT_DXCluster_Azimuth = false;        // Add azimut to callsign
		TRX.FFT_DXCluster_Timeout = 5;            // DXCluser timeout in minutes
		TRX.CenterSpectrumAfterIdle = false;      // Center FFT spectrum in free tune after idle delay
		TRX.Show_Sec_VFO = false;                 // Show secondary VFO on FFT
		TRX.FFT_Scale_Type = 0;                   // Scale type (0 - amplitude, 1 - squared, 2 - dBm)
		TRX.AnalogMeterShowPWR = false;           // false - SWR, true - PWR
		TRX.EnableBottomNavigationButtons = true; // enable bottom menu navigation buttons
		for (uint8_t i = 0; i < (FUNCBUTTONS_PAGES * FUNCBUTTONS_ON_PAGE); i++) {
			TRX.FuncButtons[i] = i;
		}
		// DECODERS
		TRX.CW_Decoder = false;       // automatic telegraph decoder
		TRX.CW_Decoder_Threshold = 8; // CW Decoder sensivity
#ifdef STM32F407xx
		TRX.RDS_Decoder = false; // RDS Decoder panel
#else
		TRX.RDS_Decoder = true; // RDS Decoder panel
#endif
		TRX.RTTY_Speed = 45;         // RTTY decoder speed
		TRX.RTTY_Shift = 170;        // RTTY decoder shift
		TRX.RTTY_Freq = 1000;        // RTTY decoder center frequency
		TRX.RTTY_StopBits = 1;       // RTTY 1, 1.5, 2 stop bits
		TRX.RTTY_InvertBits = false; // RTTY 0 and 1 reverse
		// ADC
		TRX.ADC_Driver = true; // preamplifier (ADC driver)
		TRX.ADC_PGA = true;    // ADC preamp
		TRX.ADC_RAND = false;  // ADC encryption (xor randomizer)
		TRX.ADC_SHDN = false;  // ADC disable
		TRX.ADC_DITH = false;  // ADC dither
		// SERVICES
		TRX.SWR_CUSTOM_Begin = 6500; // start spectrum analyzer range
		TRX.SWR_CUSTOM_End = 7500;   // end of spectrum analyzer range
		TRX.SPEC_Begin = 1;          // start spectrum analyzer range
		TRX.SPEC_End = 150;          // end of spectrum analyzer range
		TRX.SPEC_TopDBM = -60;       // chart thresholds
		TRX.SPEC_BottomDBM = -130;   // chart thresholds
		TRX.WSPR_FREQ_OFFSET = 0;    // offset beacon from freq center
		TRX.WSPR_BANDS_160 = false;  // enabled WSPR bands
		TRX.WSPR_BANDS_80 = true;
		TRX.WSPR_BANDS_40 = true;
		TRX.WSPR_BANDS_30 = true;
		TRX.WSPR_BANDS_20 = true;
		TRX.WSPR_BANDS_17 = true;
		TRX.WSPR_BANDS_15 = true;
		TRX.WSPR_BANDS_12 = true;
		TRX.WSPR_BANDS_10 = true;
		TRX.WSPR_BANDS_6 = false;
		TRX.WSPR_BANDS_2 = false;
		// SAT
		strcpy(TRX.SAT_Name, "QO-100");
		strcpy(TRX.SAT_TLE_Line1, "1 43700U 18090A   23320.50049146  .00000163  00000-0  00000-0 0  9991");
		strcpy(TRX.SAT_TLE_Line2, "2 43700   0.0142 283.2286 0001422 345.1641 352.9695  1.00273483 18216");
		strcpy(TRX.SAT_QTH_Lat, "52.7");
		strcpy(TRX.SAT_QTH_Lon, "41.4");
		strcpy(TRX.SAT_QTH_Alt, "174.0");
		//
		TRX.ENDBit = 100; // Bit for the end of a successful write to eeprom

		// Default Bands settings
		for (uint8_t i = 0; i < BANDS_COUNT; i++) { // saved frequencies by bands
			TRX.BANDS_SAVED_SETTINGS[i].Freq = BANDS[i].defaultFreq;
			TRX.BANDS_SAVED_SETTINGS[i].Mode = (uint8_t)getModeFromFreq(TRX.BANDS_SAVED_SETTINGS[i].Freq);
			TRX.BANDS_SAVED_SETTINGS[i].IF_Gain = TRX.IF_Gain;
			TRX.BANDS_SAVED_SETTINGS[i].RF_Gain = TRX.RF_Gain;
			TRX.BANDS_SAVED_SETTINGS[i].RF_Gain_By_Mode_CW = TRX.RF_Gain;
			TRX.BANDS_SAVED_SETTINGS[i].RF_Gain_By_Mode_SSB = TRX.RF_Gain;
			TRX.BANDS_SAVED_SETTINGS[i].RF_Gain_By_Mode_FM = TRX.RF_Gain;
			TRX.BANDS_SAVED_SETTINGS[i].RF_Gain_By_Mode_AM = TRX.RF_Gain;
			TRX.BANDS_SAVED_SETTINGS[i].RF_Gain_By_Mode_DIGI = TRX.RF_Gain;
			if (TRX.BANDS_SAVED_SETTINGS[i].Freq > 30000000) {
				TRX.BANDS_SAVED_SETTINGS[i].LNA = true;
			} else {
				TRX.BANDS_SAVED_SETTINGS[i].LNA = false;
			}
			TRX.BANDS_SAVED_SETTINGS[i].ATT = TRX.ATT;
			TRX.BANDS_SAVED_SETTINGS[i].ATT_DB = TRX.ATT_DB;
			TRX.BANDS_SAVED_SETTINGS[i].ANT_RX = TRX.ANT_RX;
			TRX.BANDS_SAVED_SETTINGS[i].ANT_TX = TRX.ANT_TX;
			TRX.BANDS_SAVED_SETTINGS[i].ADC_Driver = TRX.ADC_Driver;
			TRX.BANDS_SAVED_SETTINGS[i].SQL = false;
			TRX.BANDS_SAVED_SETTINGS[i].FM_SQL_threshold_dBm = TRX.VFO_A.FM_SQL_threshold_dBm;
			TRX.BANDS_SAVED_SETTINGS[i].ADC_PGA = TRX.ADC_PGA;
			TRX.BANDS_SAVED_SETTINGS[i].DNR_Type = 0;
			TRX.BANDS_SAVED_SETTINGS[i].AGC = true;
			TRX.BANDS_SAVED_SETTINGS[i].RepeaterMode = false;
			TRX.BANDS_SAVED_SETTINGS[i].Fast = TRX.Fast;
			TRX.BANDS_SAVED_SETTINGS[i].SAMPLERATE = TRX.SAMPLERATE_MAIN;
			TRX.BANDS_SAVED_SETTINGS[i].VFO_A_CW_LPF_Filter = TRX.VFO_A.CW_LPF_Filter;
			TRX.BANDS_SAVED_SETTINGS[i].VFO_A_SSB_LPF_RX_Filter = TRX.VFO_A.SSB_LPF_RX_Filter;
			TRX.BANDS_SAVED_SETTINGS[i].VFO_A_AM_LPF_RX_Filter = TRX.VFO_A.AM_LPF_RX_Filter;
			TRX.BANDS_SAVED_SETTINGS[i].VFO_A_FM_LPF_RX_Filter = TRX.VFO_A.FM_LPF_RX_Filter;
			TRX.BANDS_SAVED_SETTINGS[i].VFO_B_CW_LPF_Filter = TRX.VFO_B.CW_LPF_Filter;
			TRX.BANDS_SAVED_SETTINGS[i].VFO_B_SSB_LPF_RX_Filter = TRX.VFO_B.SSB_LPF_RX_Filter;
			TRX.BANDS_SAVED_SETTINGS[i].VFO_B_AM_LPF_RX_Filter = TRX.VFO_B.AM_LPF_RX_Filter;
			TRX.BANDS_SAVED_SETTINGS[i].VFO_B_FM_LPF_RX_Filter = TRX.VFO_B.FM_LPF_RX_Filter;
		}

#if defined(FRONTPANEL_MINI)
		TRX.IF_Gain = 15;
		TRX.FFT_Background = false;
		TRX.FFT_BW_Style = 3;
		TRX.FFT_Height = 4;
		TRX.ColorThemeId = 2;
#endif

		// Shadow variables
		TRX.SQL_shadow = TRX.VFO_A.SQL;
		TRX.AGC_shadow = TRX.VFO_A.AGC;
		TRX.DNR_shadow = TRX.VFO_A.DNR_Type;
		TRX.RepeaterMode_shadow = TRX.VFO_A.RepeaterMode;
		TRX.Notch_on_shadow = false;
		TRX.FM_SQL_threshold_dBm_shadow = TRX.VFO_A.FM_SQL_threshold_dBm;
		TRX.FRONTPANEL_funcbuttons_page = 0;
#if defined(FRONTPANEL_LITE)
		TRX.ENC2_func_mode = ENC_FUNC_SET_VOLUME;
#else
		TRX.ENC2_func_mode = ENC_FUNC_FAST_STEP;
#endif
		TRX.CW_LPF_Filter_shadow = TRX.VFO_A.CW_LPF_Filter;
		TRX.DIGI_LPF_Filter_shadow = TRX.VFO_A.DIGI_LPF_Filter;
		TRX.DIGI_HPF_Filter_shadow = TRX.VFO_A.DIGI_HPF_Filter;
		TRX.SSB_LPF_RX_Filter_shadow = TRX.VFO_A.SSB_LPF_RX_Filter;
		TRX.SSB_LPF_TX_Filter_shadow = TRX.VFO_A.SSB_LPF_TX_Filter;
		TRX.SSB_HPF_RX_Filter_shadow = TRX.VFO_A.SSB_HPF_RX_Filter;
		TRX.SSB_HPF_TX_Filter_shadow = TRX.VFO_A.SSB_HPF_TX_Filter;
		TRX.AM_LPF_RX_Filter_shadow = TRX.VFO_A.AM_LPF_RX_Filter;
		TRX.AM_LPF_TX_Filter_shadow = TRX.VFO_A.AM_LPF_TX_Filter;
		TRX.FM_LPF_RX_Filter_shadow = TRX.VFO_A.FM_LPF_RX_Filter;
		TRX.FM_LPF_TX_Filter_shadow = TRX.VFO_A.FM_LPF_TX_Filter;
		TRX.FM_HPF_RX_Filter_shadow = TRX.VFO_A.FM_HPF_RX_Filter;

		LCD_showError("Loaded default settings", true);

		SaveSettings(); // save to primary bank
		SaveSettings(); // save to second bank
		SaveSettingsToEEPROM();
	}

	// VFO settings
	if (!TRX.selected_vfo) {
		CurrentVFO = &TRX.VFO_A;
		SecondaryVFO = &TRX.VFO_B;
	} else {
		CurrentVFO = &TRX.VFO_B;
		SecondaryVFO = &TRX.VFO_A;
	}

	BAND_SELECTABLE[BANDID_2m] = CALIBRATE.ENABLE_2m_band || TRX.Transverter_2m;
	BAND_SELECTABLE[BANDID_70cm] = CALIBRATE.ENABLE_70cm_band || TRX.Transverter_70cm;
	BAND_SELECTABLE[BANDID_23cm] = CALIBRATE.ENABLE_23cm_band || TRX.Transverter_23cm;
	BAND_SELECTABLE[BANDID_13cm] = TRX.Transverter_13cm;
	BAND_SELECTABLE[BANDID_6cm] = TRX.Transverter_6cm;
	BAND_SELECTABLE[BANDID_3cm] = TRX.Transverter_3cm;
	BAND_SELECTABLE[BANDID_QO100] = TRX.Transverter_QO100;
	BAND_SELECTABLE[BANDID_1_2cm] = TRX.Transverter_1_2cm;
}

static void LoadSettingsFromEEPROM(void) {
	EEPROM_PowerUp();
	uint8_t tryes = 0;
	while (tryes < EEPROM_REPEAT_TRYES && !EEPROM_Read_Data((uint8_t *)&TRX, sizeof(TRX), EEPROM_SECTOR_SETTINGS, true, false)) {
		tryes++;
	}
	if (tryes >= EEPROM_REPEAT_TRYES) {
		println("[ERR] Read EEPROM SETTINGS multiple errors");
	}
	EEPROM_PowerDown();
}

void LoadCalibration(bool clear) {
	EEPROM_PowerUp();
	uint8_t tryes = 0;
	while (tryes < EEPROM_REPEAT_TRYES && !EEPROM_Read_Data((uint8_t *)&CALIBRATE, sizeof(CALIBRATE), EEPROM_SECTOR_CALIBRATION, true, false)) {
		tryes++;
	}
	if (tryes >= EEPROM_REPEAT_TRYES) {
		println("[ERR] Read EEPROM CALIBRATE multiple errors");
		LCD_showError("EEPROM Error", true);
	}

	if (CALIBRATE.ENDBit != 100 || CALIBRATE.flash_id != CALIB_VERSION || clear ||
	    CALIBRATE.csum != calculateCSUM_EEPROM()) // code for checking the firmware in the eeprom, if it does not match, we use the default
	{
		memset(&CALIBRATE, 0x00, sizeof(CALIBRATE));

		println("[ERR] CALIBRATE Flash check CODE:", CALIBRATE.flash_id);
		CALIBRATE.flash_id = CALIB_VERSION; // code for checking the firmware in the eeprom, if it does not match, we use the default

		CALIBRATE.ENCODER_INVERT = false;      // invert left-right rotation of the main encoder
		CALIBRATE.ENCODER2_INVERT = false;     // invert left-right rotation of the optional encoder
		CALIBRATE.ENCODER_DEBOUNCE = 0;        // time to eliminate contact bounce at the main encoder, ms
		CALIBRATE.ENCODER2_DEBOUNCE = 1;       // time to eliminate contact bounce at the additional encoder, ms
		CALIBRATE.ENCODER2_ON_FALLING = true;  // skip one step on ENC2
		CALIBRATE.ENCODER_SLOW_RATE = 25;      // slow down the encoder for high resolutions
		CALIBRATE.ENCODER_ACCELERATION = 75;   // acceleration rate if rotate
		CALIBRATE.TangentType = TANGENT_MH48;  // Tangent type
		CALIBRATE.CICFIR_GAINER_48K_val = 7;   // Offset from the output of the CIC compensator
		CALIBRATE.CICFIR_GAINER_96K_val = 12;  // Offset from the output of the CIC compensator
		CALIBRATE.CICFIR_GAINER_192K_val = 17; // Offset from the output of the CIC compensator
		CALIBRATE.CICFIR_GAINER_384K_val = 22; // Offset from the output of the CIC compensator
		CALIBRATE.TXCICFIR_GAINER_val = 5;     // Offset from the TX-CIC output of the compensator
		CALIBRATE.DAC_GAINER_val = 5;          // DAC shift
		// Calibrate the maximum output power for each band
		CALIBRATE.DAC_driver_mode = 2;            // OPA2673 bias mode
		CALIBRATE.rf_out_power_2200m = 29;        // 2200m
		CALIBRATE.rf_out_power_630m = 29;         // 630m
		CALIBRATE.rf_out_power_160m = 29;         // 160m
		CALIBRATE.rf_out_power_80m = 27;          // 80m
		CALIBRATE.rf_out_power_60m = 27;          // 60m
		CALIBRATE.rf_out_power_40m = 26;          // 40m
		CALIBRATE.rf_out_power_30m = 26;          // 30m
		CALIBRATE.rf_out_power_20m = 24;          // 20m
		CALIBRATE.rf_out_power_17m = 24;          // 17m
		CALIBRATE.rf_out_power_15m = 24;          // 15m
		CALIBRATE.rf_out_power_12m = 24;          // 12m
		CALIBRATE.rf_out_power_cb = 24;           // 27MHz
		CALIBRATE.rf_out_power_10m = 24;          // 10m
		CALIBRATE.rf_out_power_6m = 13;           // 6m
		CALIBRATE.rf_out_power_4m = 13;           // 4m
		CALIBRATE.rf_out_power_2m = 100;          // 2m
		CALIBRATE.rf_out_power_70cm = 100;        // 70cm
		CALIBRATE.rf_out_power_23cm = 100;        // 23cm
		CALIBRATE.rf_out_power_13cm = 100;        // 13cm
		CALIBRATE.rf_out_power_6cm = 100;         // 6cm
		CALIBRATE.rf_out_power_3cm = 100;         // 3cm
		CALIBRATE.rf_out_power_QO100 = 100;       // QO-100
		CALIBRATE.rf_out_power_1_2cm = 100;       // 1.2cm
		CALIBRATE.smeter_calibration_hf = 10;     // S-Meter calibration, set when calibrating the transceiver to S9 (ATT, PREAMP off) HF
		CALIBRATE.smeter_calibration_vhf = 10;    // S-Meter calibration, set when calibrating the transceiver to S9 (ATT, PREAMP off) VHF
		CALIBRATE.SWR_FWD_Calibration_HF = 11.0f; // SWR Transormator rate forward
		CALIBRATE.SWR_BWD_Calibration_HF = 11.0f; // SWR Transormator rate return
		CALIBRATE.SWR_FWD_Calibration_6M = 10.0f; // SWR Transormator rate forward
		CALIBRATE.SWR_BWD_Calibration_6M = 10.0f; // SWR Transormator rate return
		CALIBRATE.SWR_FWD_Calibration_VHF = 3.6f; // SWR Transormator rate forward
		CALIBRATE.SWR_BWD_Calibration_VHF = 3.6f; // SWR Transormator rate return
		CALIBRATE.TUNE_MAX_POWER = 2;             // Maximum RF power in Tune mode
		CALIBRATE.MAX_RF_POWER_ON_METER = 7;      // Max TRX Power for indication
		CALIBRATE.PWR_VLT_Calibration = 1000.0f;  // VLT meter calibration
#if defined(FRONTPANEL_X1)
		CALIBRATE.ENCODER_INVERT = true;
		CALIBRATE.ENCODER_SLOW_RATE = 10;
		CALIBRATE.RFU_HPF_START = 32000 * 1000; // HPF
		CALIBRATE.RFU_BPF_0_START = 1500 * 1000;
		CALIBRATE.RFU_BPF_0_END = 2500 * 1000;
		CALIBRATE.RFU_BPF_1_START = 2500 * 1000;
		CALIBRATE.RFU_BPF_1_END = 4700 * 1000;
		CALIBRATE.RFU_BPF_2_START = 4700 * 1000;
		CALIBRATE.RFU_BPF_2_END = 8000 * 1000;
		CALIBRATE.RFU_BPF_3_START = 8000 * 1000;
		CALIBRATE.RFU_BPF_3_END = 14700 * 1000;
		CALIBRATE.RFU_BPF_4_START = 14700 * 1000;
		CALIBRATE.RFU_BPF_4_END = 22100 * 1000;
		CALIBRATE.RFU_BPF_5_START = 22100 * 1000;
		CALIBRATE.RFU_BPF_5_END = 32000 * 1000;
		CALIBRATE.RFU_BPF_6_START = 138000 * 1000;
		CALIBRATE.RFU_BPF_6_END = 150000 * 1000;
		CALIBRATE.rf_out_power_2200m = 20;        // 2200m
		CALIBRATE.rf_out_power_630m = 20;         // 630m
		CALIBRATE.rf_out_power_160m = 41;         // 160m
		CALIBRATE.rf_out_power_80m = 29;          // 80m
		CALIBRATE.rf_out_power_60m = 29;          // 60m
		CALIBRATE.rf_out_power_40m = 26;          // 40m
		CALIBRATE.rf_out_power_30m = 26;          // 30m
		CALIBRATE.rf_out_power_20m = 32;          // 20m
		CALIBRATE.rf_out_power_17m = 36;          // 17m
		CALIBRATE.rf_out_power_15m = 42;          // 15m
		CALIBRATE.rf_out_power_12m = 39;          // 12m
		CALIBRATE.rf_out_power_cb = 39;           // 27MHz
		CALIBRATE.rf_out_power_10m = 42;          // 10m
		CALIBRATE.rf_out_power_6m = 20;           // 6m
		CALIBRATE.rf_out_power_4m = 13;           // 4m
		CALIBRATE.rf_out_power_2m = 20;           // 2m
		CALIBRATE.rf_out_power_70cm = 20;         // 70cm
		CALIBRATE.rf_out_power_23cm = 20;         // 23cm
		CALIBRATE.rf_out_power_13cm = 20;         // 13cm
		CALIBRATE.rf_out_power_6cm = 20;          // 6cm
		CALIBRATE.rf_out_power_3cm = 20;          // 3cm
		CALIBRATE.rf_out_power_QO100 = 20;        // QO-100
		CALIBRATE.rf_out_power_1_2cm = 20;        // 1.2cm
		CALIBRATE.smeter_calibration_hf = 12;     // S-Meter calibration, set when calibrating the transceiver to S9 (ATT, PREAMP off) HF
		CALIBRATE.smeter_calibration_vhf = 12;    // S-Meter calibration, set when calibrating the transceiver to S9 (ATT, PREAMP off) VHF
		CALIBRATE.SWR_FWD_Calibration_HF = 10.0f; // SWR Transormator rate forward
		CALIBRATE.SWR_BWD_Calibration_HF = 10.0f; // SWR Transormator rate return
		CALIBRATE.SWR_FWD_Calibration_6M = 10.0f; // SWR Transormator rate forward
		CALIBRATE.SWR_BWD_Calibration_6M = 10.0f; // SWR Transormator rate return
		CALIBRATE.SWR_FWD_Calibration_VHF = 8.5f; // SWR Transormator rate forward
		CALIBRATE.SWR_BWD_Calibration_VHF = 8.5f; // SWR Transormator rate return
		CALIBRATE.TUNE_MAX_POWER = 5;             // Maximum RF power in Tune mode
		CALIBRATE.MAX_RF_POWER_ON_METER = 15;     // Max TRX Power for indication
		CALIBRATE.RF_unit_type = RF_UNIT_NONE;    // RF-unit type
#elif defined(FRONTPANEL_BIG_V1)
		CALIBRATE.RF_unit_type = RF_UNIT_QRP;          // RF-unit type
		CALIBRATE.rf_out_power_2200m = 29;             // 2200m
		CALIBRATE.rf_out_power_630m = 29;              // 630m
		CALIBRATE.rf_out_power_160m = 29;              // 160m
		CALIBRATE.rf_out_power_80m = 27;               // 80m
		CALIBRATE.rf_out_power_60m = 27;               // 60m
		CALIBRATE.rf_out_power_40m = 26;               // 40m
		CALIBRATE.rf_out_power_30m = 26;               // 30m
		CALIBRATE.rf_out_power_20m = 24;               // 20m
		CALIBRATE.rf_out_power_17m = 24;               // 17m
		CALIBRATE.rf_out_power_15m = 24;               // 15m
		CALIBRATE.rf_out_power_12m = 24;               // 12m
		CALIBRATE.rf_out_power_cb = 24;                // 27MHz
		CALIBRATE.rf_out_power_10m = 24;               // 10m
		CALIBRATE.rf_out_power_6m = 13;                // 6m
		CALIBRATE.rf_out_power_4m = 13;                // 4m
		CALIBRATE.rf_out_power_2m = 100;               // 2m
		CALIBRATE.rf_out_power_70cm = 100;             // 70cm
		CALIBRATE.rf_out_power_23cm = 100;             // 23cm
		CALIBRATE.rf_out_power_13cm = 100;             // 13cm
		CALIBRATE.rf_out_power_6cm = 100;              // 6cm
		CALIBRATE.rf_out_power_3cm = 100;              // 3cm
		CALIBRATE.rf_out_power_QO100 = 100;            // QO100
		CALIBRATE.rf_out_power_1_2cm = 100;            // 1.2cm
		CALIBRATE.RFU_LPF_END = 60000 * 1000;          // LPF
		CALIBRATE.RFU_HPF_START = 60000 * 1000;        // HPF U14-RF1
		CALIBRATE.RFU_BPF_0_START = 138 * 1000 * 1000; // 2m U14-RF3
		CALIBRATE.RFU_BPF_0_END = 150 * 1000 * 1000;   // 2m
		CALIBRATE.RFU_BPF_1_START = 1500 * 1000;       // 160m U16-RF2
		CALIBRATE.RFU_BPF_1_END = 2400 * 1000;         // 160m
		CALIBRATE.RFU_BPF_2_START = 2400 * 1000;       // 80m U16-RF4
		CALIBRATE.RFU_BPF_2_END = 4700 * 1000;         // 80m
		CALIBRATE.RFU_BPF_3_START = 4700 * 1000;       // 40m U16-RF1
		CALIBRATE.RFU_BPF_3_END = 7200 * 1000;         // 40m
		CALIBRATE.RFU_BPF_4_START = 7200 * 1000;       // 30m U16-RF3
		CALIBRATE.RFU_BPF_4_END = 11500 * 1000;        // 30m
		CALIBRATE.RFU_BPF_5_START = 11500 * 1000;      // 20,17m U14-RF2
		CALIBRATE.RFU_BPF_5_END = 21000 * 1000;        // 20,17m
		CALIBRATE.RFU_BPF_6_START = 21000 * 1000;      // 15,12,10,6m U14-RF4
		CALIBRATE.RFU_BPF_6_END = 64000 * 1000;        // 15,12,10,6m
		CALIBRATE.RFU_BPF_7_START = 0;                 // disabled on qrp version
		CALIBRATE.RFU_BPF_7_END = 0;                   // disabled on qrp version
		CALIBRATE.RFU_BPF_8_START = 0;                 // disabled on qrp version
		CALIBRATE.RFU_BPF_8_END = 0;                   // disabled on qrp version
		CALIBRATE.SWR_FWD_Calibration_HF = 11.0f;      // SWR Transormator rate forward
		CALIBRATE.SWR_BWD_Calibration_HF = 11.0f;      // SWR Transormator rate return
		CALIBRATE.SWR_FWD_Calibration_6M = 10.0f;      // SWR Transormator rate forward
		CALIBRATE.SWR_BWD_Calibration_6M = 10.0f;      // SWR Transormator rate return
		CALIBRATE.SWR_FWD_Calibration_VHF = 3.6f;      // SWR Transormator rate forward
		CALIBRATE.SWR_BWD_Calibration_VHF = 3.6f;      // SWR Transormator rate return
		CALIBRATE.TUNE_MAX_POWER = 2;                  // Maximum RF power in Tune mode
		CALIBRATE.MAX_RF_POWER_ON_METER = 7;           // Max TRX Power for indication
		CALIBRATE.RF_unit_type = RF_UNIT_NONE;         // RF-unit type
#elif defined(FRONTPANEL_KT_100S)
		CALIBRATE.RF_unit_type = RF_UNIT_KT_100S;      // RF-unit type
		CALIBRATE.rf_out_power_2200m = 40;             // 2200m
		CALIBRATE.rf_out_power_630m = 40;              // 630m
		CALIBRATE.rf_out_power_160m = 40;              // 160m
		CALIBRATE.rf_out_power_80m = 40;               // 80m
		CALIBRATE.rf_out_power_60m = 40;               // 60m
		CALIBRATE.rf_out_power_40m = 40;               // 40m
		CALIBRATE.rf_out_power_30m = 40;               // 30m
		CALIBRATE.rf_out_power_20m = 40;               // 20m
		CALIBRATE.rf_out_power_17m = 40;               // 17m
		CALIBRATE.rf_out_power_15m = 40;               // 15m
		CALIBRATE.rf_out_power_12m = 40;               // 12m
		CALIBRATE.rf_out_power_cb = 40;                // 27MHz
		CALIBRATE.rf_out_power_10m = 40;               // 10m
		CALIBRATE.rf_out_power_6m = 40;                // 6m
		CALIBRATE.rf_out_power_4m = 40;                // 4m
		CALIBRATE.rf_out_power_2m = 50;                // 2m
		CALIBRATE.rf_out_power_70cm = 50;              // 70cm
		CALIBRATE.rf_out_power_23cm = 50;              // 23cm
		CALIBRATE.rf_out_power_13cm = 50;              // 13cm
		CALIBRATE.rf_out_power_6cm = 50;               // 6cm
		CALIBRATE.rf_out_power_3cm = 50;               // 3cm
		CALIBRATE.rf_out_power_QO100 = 50;             // QO100
		CALIBRATE.rf_out_power_1_2cm = 50;             // 1.2cm
		CALIBRATE.RFU_LPF_END = 60000 * 1000;          // LPF
		CALIBRATE.RFU_HPF_START = 60000 * 1000;        // HPF
		CALIBRATE.RFU_BPF_0_START = 138 * 1000 * 1000; // 2m U14-RF3
		CALIBRATE.RFU_BPF_0_END = 150 * 1000 * 1000;   // 2m
		CALIBRATE.RFU_BPF_1_START = 1500 * 1000;       // 160m U16-RF2
		CALIBRATE.RFU_BPF_1_END = 2400 * 1000;         // 160m
		CALIBRATE.RFU_BPF_2_START = 2400 * 1000;       // 80m U16-RF4
		CALIBRATE.RFU_BPF_2_END = 4700 * 1000;         // 80m
		CALIBRATE.RFU_BPF_3_START = 4700 * 1000;       // 40m U16-RF1
		CALIBRATE.RFU_BPF_3_END = 7200 * 1000;         // 40m
		CALIBRATE.RFU_BPF_4_START = 7200 * 1000;       // 30m U16-RF3
		CALIBRATE.RFU_BPF_4_END = 11500 * 1000;        // 30m
		CALIBRATE.RFU_BPF_5_START = 11500 * 1000;      // 20,17m U14-RF2
		CALIBRATE.RFU_BPF_5_END = 21000 * 1000;        // 20,17m
		CALIBRATE.RFU_BPF_6_START = 21000 * 1000;      // 15,12,10,6m U14-RF4
		CALIBRATE.RFU_BPF_6_END = 64000 * 1000;        // 15,12,10,6m
		CALIBRATE.RFU_BPF_7_START = 0;                 // disabled on qrp version
		CALIBRATE.RFU_BPF_7_END = 0;                   // disabled on qrp version
		CALIBRATE.RFU_BPF_8_START = 0;                 // disabled on qrp version
		CALIBRATE.RFU_BPF_8_END = 0;                   // disabled on qrp version
		CALIBRATE.SWR_FWD_Calibration_HF = 22.0f;      // SWR Transormator rate forward
		CALIBRATE.SWR_BWD_Calibration_HF = 22.0f;      // SWR Transormator rate return
		CALIBRATE.SWR_FWD_Calibration_6M = 22.0f;      // SWR Transormator rate forward
		CALIBRATE.SWR_BWD_Calibration_6M = 22.0f;      // SWR Transormator rate return
		CALIBRATE.SWR_FWD_Calibration_VHF = 22.0f;     // SWR Transormator rate forward
		CALIBRATE.SWR_BWD_Calibration_VHF = 22.0f;     // SWR Transormator rate return
		CALIBRATE.TUNE_MAX_POWER = 10;                 // Maximum RF power in Tune mode
		CALIBRATE.MAX_RF_POWER_ON_METER = 100;         // Max TRX Power for indication
		CALIBRATE.RF_unit_type = RF_UNIT_KT_100S;      // RF-unit type
#elif defined(FRONTPANEL_WF_100D)
		CALIBRATE.ENCODER2_INVERT = true; // invert left-right rotation of the optional encoder
		CALIBRATE.RF_unit_type = RF_UNIT_WF_100D;
		CALIBRATE.rf_out_power_2200m = 20;             // 2200m
		CALIBRATE.rf_out_power_630m = 20;              // 630m
		CALIBRATE.rf_out_power_160m = 27;              // 160m
		CALIBRATE.rf_out_power_80m = 28;               // 80m
		CALIBRATE.rf_out_power_60m = 29;               // 60m
		CALIBRATE.rf_out_power_40m = 30;               // 40m
		CALIBRATE.rf_out_power_30m = 34;               // 30m
		CALIBRATE.rf_out_power_20m = 35;               // 20m
		CALIBRATE.rf_out_power_17m = 44;               // 17m
		CALIBRATE.rf_out_power_15m = 54;               // 15m
		CALIBRATE.rf_out_power_12m = 49;               // 12m
		CALIBRATE.rf_out_power_cb = 50;                // 27MHz
		CALIBRATE.rf_out_power_10m = 90;               // 10m
		CALIBRATE.rf_out_power_6m = 90;                // 6m
		CALIBRATE.rf_out_power_4m = 90;                // 4m
		CALIBRATE.rf_out_power_2m = 80;                // 2m
		CALIBRATE.rf_out_power_70cm = 15;              // 70cm
		CALIBRATE.rf_out_power_23cm = 80;              // 23cm
		CALIBRATE.rf_out_power_13cm = 15;              // 13cm
		CALIBRATE.rf_out_power_6cm = 16;               // 6cm
		CALIBRATE.rf_out_power_3cm = 17;               // 3cm
		CALIBRATE.rf_out_power_QO100 = 15;             // QO-100
		CALIBRATE.rf_out_power_1_2cm = 17;             // 1.2cm
		CALIBRATE.RFU_LPF_END = 53 * 1000 * 1000;      // LPF
		CALIBRATE.RFU_HPF_START = 60 * 1000 * 1000;    // HPF
		CALIBRATE.RFU_BPF_0_START = 1600 * 1000;       // 1.6-2.5MHz
		CALIBRATE.RFU_BPF_0_END = 2500 * 1000;         //
		CALIBRATE.RFU_BPF_1_START = 2500 * 1000;       // 2.5-4MHz
		CALIBRATE.RFU_BPF_1_END = 4000 * 1000;         //
		CALIBRATE.RFU_BPF_2_START = 6000 * 1000;       // 6-7.3MHz
		CALIBRATE.RFU_BPF_2_END = 7300 * 1000;         //
		CALIBRATE.RFU_BPF_3_START = 7300 * 1000;       // 7-12MHz
		CALIBRATE.RFU_BPF_3_END = 12000 * 1000;        //
		CALIBRATE.RFU_BPF_4_START = 12000 * 1000;      // 12-14.5MHz
		CALIBRATE.RFU_BPF_4_END = 14500 * 1000;        //
		CALIBRATE.RFU_BPF_5_START = 14500 * 1000;      // 14.5-21.5MHz
		CALIBRATE.RFU_BPF_5_END = 21500 * 1000;        //
		CALIBRATE.RFU_BPF_6_START = 21500 * 1000;      // 21.5-30 MHz
		CALIBRATE.RFU_BPF_6_END = 30000 * 1000;        //
		CALIBRATE.RFU_BPF_7_START = 138 * 1000 * 1000; // 138-150MHz
		CALIBRATE.RFU_BPF_7_END = 150 * 1000 * 1000;   //
		CALIBRATE.RFU_BPF_8_START = 0;                 // disabled
		CALIBRATE.RFU_BPF_8_END = 0;                   // disabled
		CALIBRATE.SWR_FWD_Calibration_HF = 23.5f;      // SWR Transormator rate forward
		CALIBRATE.SWR_BWD_Calibration_HF = 23.5f;      // SWR Transormator rate return
		CALIBRATE.SWR_FWD_Calibration_6M = 23.0f;      // SWR Transormator rate forward
		CALIBRATE.SWR_BWD_Calibration_6M = 23.0f;      // SWR Transormator rate return
		CALIBRATE.SWR_FWD_Calibration_VHF = 16.6f;     // SWR Transormator rate forward
		CALIBRATE.SWR_BWD_Calibration_VHF = 8.5f;      // SWR Transormator rate return
		CALIBRATE.TUNE_MAX_POWER = 15;                 // Maximum RF power in Tune mode
		CALIBRATE.MAX_RF_POWER_ON_METER = 100;         // Max TRX Power for indication
		CALIBRATE.RF_unit_type = RF_UNIT_WF_100D;      // RF-unit type
#elif defined(FRONTPANEL_WOLF_2)
		CALIBRATE.ENCODER2_INVERT = true;          // invert left-right rotation of the optional encoder
		CALIBRATE.rf_out_power_2200m = 20;         // 2200m
		CALIBRATE.rf_out_power_630m = 20;          // 630m
		CALIBRATE.rf_out_power_160m = 27;          // 160m
		CALIBRATE.rf_out_power_80m = 28;           // 80m
		CALIBRATE.rf_out_power_60m = 29;           // 60m
		CALIBRATE.rf_out_power_40m = 30;           // 40m
		CALIBRATE.rf_out_power_30m = 34;           // 30m
		CALIBRATE.rf_out_power_20m = 35;           // 20m
		CALIBRATE.rf_out_power_17m = 44;           // 17m
		CALIBRATE.rf_out_power_15m = 54;           // 15m
		CALIBRATE.rf_out_power_12m = 49;           // 12m
		CALIBRATE.rf_out_power_cb = 50;            // 27MHz
		CALIBRATE.rf_out_power_10m = 90;           // 10m
		CALIBRATE.rf_out_power_6m = 90;            // 6m
		CALIBRATE.rf_out_power_4m = 90;            // 4m
		CALIBRATE.rf_out_power_2m = 80;            // 2m
		CALIBRATE.rf_out_power_70cm = 15;          // 70cm
		CALIBRATE.rf_out_power_23cm = 15;          // 23cm
		CALIBRATE.rf_out_power_13cm = 15;          // 13cm
		CALIBRATE.rf_out_power_6cm = 15;           // 6cm
		CALIBRATE.rf_out_power_3cm = 15;           // 3cm
		CALIBRATE.rf_out_power_QO100 = 15;         // QO-100
		CALIBRATE.rf_out_power_1_2cm = 15;         // 1.2cm
		CALIBRATE.RFU_LPF_END = 60 * 1000 * 1000;  // LPF
		CALIBRATE.RFU_HPF_START = 0;               // HPF disabled
		CALIBRATE.RFU_BPF_0_START = 1700 * 1000;   // 1.6-2.5MHz
		CALIBRATE.RFU_BPF_0_END = 2800 * 1000;     //
		CALIBRATE.RFU_BPF_1_START = 2800 * 1000;   // 2.5-4MHz
		CALIBRATE.RFU_BPF_1_END = 4400 * 1000;     //
		CALIBRATE.RFU_BPF_2_START = 6600 * 1000;   // 6-7.3MHz
		CALIBRATE.RFU_BPF_2_END = 8500 * 1000;     //
		CALIBRATE.RFU_BPF_3_START = 8500 * 1000;   // 7-12MHz
		CALIBRATE.RFU_BPF_3_END = 12500 * 1000;    //
		CALIBRATE.RFU_BPF_4_START = 12500 * 1000;  // 12-14.5MHz
		CALIBRATE.RFU_BPF_4_END = 16000 * 1000;    //
		CALIBRATE.RFU_BPF_5_START = 16000 * 1000;  // 14.5-21.5MHz
		CALIBRATE.RFU_BPF_5_END = 23000 * 1000;    //
		CALIBRATE.RFU_BPF_6_START = 23000 * 1000;  // 21.5-30 MHz
		CALIBRATE.RFU_BPF_6_END = 32000 * 1000;    //
		CALIBRATE.RFU_BPF_7_START = 0;             // disabled
		CALIBRATE.RFU_BPF_7_END = 0;               // disabled
		CALIBRATE.RFU_BPF_8_START = 0;             // disabled
		CALIBRATE.RFU_BPF_8_END = 0;               // disabled
		CALIBRATE.SWR_FWD_Calibration_HF = 23.5f;  // SWR Transormator rate forward
		CALIBRATE.SWR_BWD_Calibration_HF = 23.5f;  // SWR Transormator rate return
		CALIBRATE.SWR_FWD_Calibration_6M = 23.0f;  // SWR Transormator rate forward
		CALIBRATE.SWR_BWD_Calibration_6M = 23.0f;  // SWR Transormator rate return
		CALIBRATE.SWR_FWD_Calibration_VHF = 16.6f; // SWR Transormator rate forward
		CALIBRATE.SWR_BWD_Calibration_VHF = 9.5f;  // SWR Transormator rate return
		CALIBRATE.TUNE_MAX_POWER = 15;             // Maximum RF power in Tune mode
		CALIBRATE.MAX_RF_POWER_ON_METER = 100;     // Max TRX Power for indication
		CALIBRATE.RF_unit_type = RF_UNIT_NONE;     // RF-unit type
#elif defined(FRONTPANEL_LITE)
		CALIBRATE.ENCODER_SLOW_RATE = 10;
		CALIBRATE.ENCODER2_ON_FALLING = false;
		CALIBRATE.smeter_calibration_hf = 15;
		CALIBRATE.TUNE_MAX_POWER = 5;          // Maximum RF power in Tune mode
		CALIBRATE.MAX_RF_POWER_ON_METER = 15;  // Max TRX Power for indication
		CALIBRATE.RF_unit_type = RF_UNIT_NONE; // RF-unit type
#elif defined(FRONTPANEL_MINI)
		CALIBRATE.ENCODER_SLOW_RATE = 15;
		CALIBRATE.RFU_LPF_END = 60000 * 1000;          // LPF
		CALIBRATE.RFU_HPF_START = 40000 * 1000;        // HPF
		CALIBRATE.RFU_BPF_0_START = 138 * 1000 * 1000; // 2m
		CALIBRATE.RFU_BPF_0_END = 150 * 1000 * 1000;   // 2m
		CALIBRATE.RFU_BPF_1_START = 1500 * 1000;       // 160m
		CALIBRATE.RFU_BPF_1_END = 2500 * 1000;         // 160m
		CALIBRATE.RFU_BPF_2_START = 2500 * 1000;       // 80m
		CALIBRATE.RFU_BPF_2_END = 4700 * 1000;         // 80m
		CALIBRATE.RFU_BPF_3_START = 4700 * 1000;       // 40m
		CALIBRATE.RFU_BPF_3_END = 8000 * 1000;         // 40m
		CALIBRATE.RFU_BPF_4_START = 8000 * 1000;       // 30m,20m
		CALIBRATE.RFU_BPF_4_END = 14700 * 1000;        // 30m,20m
		CALIBRATE.RFU_BPF_5_START = 14700 * 1000;      // 20,17m
		CALIBRATE.RFU_BPF_5_END = 22100 * 1000;        // 20,17m
		CALIBRATE.RFU_BPF_6_START = 22100 * 1000;      // 15,12,10
		CALIBRATE.RFU_BPF_6_END = 40000 * 1000;        // 15,12,10
		CALIBRATE.RFU_BPF_7_START = 0;                 // disabled
		CALIBRATE.RFU_BPF_7_END = 0;                   // disabled
		CALIBRATE.RFU_BPF_8_START = 0;                 // disabled
		CALIBRATE.RFU_BPF_8_END = 0;                   // disabled
		CALIBRATE.smeter_calibration_hf = 16;
		CALIBRATE.TUNE_MAX_POWER = 5;             // Maximum RF power in Tune mode
		CALIBRATE.MAX_RF_POWER_ON_METER = 30;     // Max TRX Power for indication
		CALIBRATE.SWR_FWD_Calibration_HF = 21.0f; // SWR Transormator rate forward
		CALIBRATE.SWR_BWD_Calibration_HF = 21.0f; // SWR Transormator rate return
		CALIBRATE.PWR_VLT_Calibration = 1100.0f;  // VLT meter calibration
		CALIBRATE.RF_unit_type = RF_UNIT_HF;      // RF-unit type
#else
		CALIBRATE.RFU_LPF_END = 60000 * 1000;          // LPF
		CALIBRATE.RFU_HPF_START = 60000 * 1000;        // HPF U14-RF1
		CALIBRATE.RFU_BPF_0_START = 138 * 1000 * 1000; // 2m U14-RF3
		CALIBRATE.RFU_BPF_0_END = 150 * 1000 * 1000;   // 2m
		CALIBRATE.RFU_BPF_1_START = 1500 * 1000;       // 160m U16-RF2
		CALIBRATE.RFU_BPF_1_END = 2400 * 1000;         // 160m
		CALIBRATE.RFU_BPF_2_START = 2400 * 1000;       // 80m U16-RF4
		CALIBRATE.RFU_BPF_2_END = 4700 * 1000;         // 80m
		CALIBRATE.RFU_BPF_3_START = 4700 * 1000;       // 40m U16-RF1
		CALIBRATE.RFU_BPF_3_END = 7200 * 1000;         // 40m
		CALIBRATE.RFU_BPF_4_START = 7200 * 1000;       // 30m U16-RF3
		CALIBRATE.RFU_BPF_4_END = 11500 * 1000;        // 30m
		CALIBRATE.RFU_BPF_5_START = 11500 * 1000;      // 20,17m U14-RF2
		CALIBRATE.RFU_BPF_5_END = 21000 * 1000;        // 20,17m
		CALIBRATE.RFU_BPF_6_START = 21000 * 1000;      // 15,12,10,6m U14-RF4
		CALIBRATE.RFU_BPF_6_END = 64000 * 1000;        // 15,12,10,6m
		CALIBRATE.RFU_BPF_7_START = 0;                 // disabled on qrp version
		CALIBRATE.RFU_BPF_7_END = 0;                   // disabled on qrp version
		CALIBRATE.RFU_BPF_8_START = 0;                 // disabled on qrp version
		CALIBRATE.RFU_BPF_8_END = 0;                   // disabled on qrp version
		CALIBRATE.RF_unit_type = RF_UNIT_NONE;         // RF-unit type
#endif
#if defined(FRONTPANEL_MINI)
		CALIBRATE.TCXO_frequency = 20000; // TCXO Frequency x1000
#else
		CALIBRATE.TCXO_frequency = 12288;              // TCXO Frequency x1000
#endif
		CALIBRATE.MAX_ChargePump_Freq = 200;    // Maximum frequency for charge pump pwm
		CALIBRATE.VCXO_correction = 0;          // VCXO Frequency offset
		CALIBRATE.FAN_MEDIUM_START = 45;        // Temperature at which the fan starts at half power
		CALIBRATE.FAN_MEDIUM_STOP = 40;         // Temperature at which the fan stops
		CALIBRATE.FAN_FULL_START = 55;          // Temperature at which the fan starts at full power
		CALIBRATE.FAN_Medium_speed = 50;        // FAN PWM rate at medium speed
		CALIBRATE.TRX_MAX_RF_TEMP = 80;         // Maximum RF unit themperature to enable protect
		CALIBRATE.TRX_MAX_SWR = 3;              // Maximum SWR to enable protect on TX (NOT IN TUNE MODE!)
		CALIBRATE.FM_DEVIATION_SCALE = 4;       // FM Deviation scale
		CALIBRATE.SSB_POWER_ADDITION = 0;       // Additional power in SSB mode
		CALIBRATE.AM_MODULATION_INDEX = 100;    // AM Modulation Index
		CALIBRATE.RTC_Coarse_Calibration = 127; // Coarse RTC calibration
		CALIBRATE.RTC_Calibration = 0;          // Real Time Clock calibration
		CALIBRATE.EXT_2200m = 0;                // External port by band
		CALIBRATE.EXT_630m = 0;                 // External port by band
		CALIBRATE.EXT_160m = 0;                 // External port by band
		CALIBRATE.EXT_80m = 1;                  // External port by band
		CALIBRATE.EXT_60m = 1;                  // External port by band
		CALIBRATE.EXT_40m = 2;                  // External port by band
		CALIBRATE.EXT_30m = 3;                  // External port by band
		CALIBRATE.EXT_20m = 4;                  // External port by band
		CALIBRATE.EXT_17m = 5;                  // External port by band
		CALIBRATE.EXT_15m = 6;                  // External port by band
		CALIBRATE.EXT_12m = 7;                  // External port by band
		CALIBRATE.EXT_CB = 10;                  // External port by band
		CALIBRATE.EXT_10m = 10;                 // External port by band
		CALIBRATE.EXT_6m = 12;                  // External port by band
		CALIBRATE.EXT_4m = 12;                  // External port by band
		CALIBRATE.EXT_FM = 11;                  // External port by band
		CALIBRATE.EXT_2m = 11;                  // External port by band
		CALIBRATE.EXT_70cm = 8;                 // External port by band
		CALIBRATE.EXT_23cm = 9;                 // External port by band
		CALIBRATE.EXT_13cm = 13;                // External port by band
		CALIBRATE.EXT_6cm = 14;                 // External port by band
		CALIBRATE.EXT_3cm = 15;                 // External port by band
		CALIBRATE.EXT_QO100 = 15;               // External port by band
		CALIBRATE.EXT_1_2cm = 12;               // External port by band
		CALIBRATE.NOTX_NOTHAM = true;           // disable TX on non-HAM bands
		CALIBRATE.NOTX_2200m = false;           // disable TX on some bands
		CALIBRATE.NOTX_630m = false;
		CALIBRATE.NOTX_160m = false;
		CALIBRATE.NOTX_80m = false;
		CALIBRATE.NOTX_60m = false;
		CALIBRATE.NOTX_40m = false;
		CALIBRATE.NOTX_30m = false;
		CALIBRATE.NOTX_20m = false;
		CALIBRATE.NOTX_17m = false;
		CALIBRATE.NOTX_15m = false;
		CALIBRATE.NOTX_12m = false;
		CALIBRATE.NOTX_CB = false;
		CALIBRATE.NOTX_10m = false;
		CALIBRATE.NOTX_6m = false;
		CALIBRATE.NOTX_4m = false;
		CALIBRATE.NOTX_2m = false;
		CALIBRATE.NOTX_70cm = false;
		CALIBRATE.NOTX_23cm = false;
		CALIBRATE.ENABLE_2200m_band = false; // enable hidden bands
		CALIBRATE.ENABLE_630m_band = false;
		CALIBRATE.ENABLE_60m_band = false;
		CALIBRATE.ENABLE_6m_band = true;
		CALIBRATE.ENABLE_4m_band = false;
		CALIBRATE.ENABLE_FM_band = true;
		CALIBRATE.ENABLE_2m_band = true;
		CALIBRATE.ENABLE_AIR_band = false;
		CALIBRATE.ENABLE_marine_band = false;
		CALIBRATE.ENABLE_70cm_band = false;
		CALIBRATE.ENABLE_23cm_band = false;
		CALIBRATE.Transverter_Custom_Offset_MHz = 100; // Offset from VFO
		CALIBRATE.Transverter_2m_RF_MHz = 144;
		CALIBRATE.Transverter_2m_IF_MHz = 28;
		CALIBRATE.Transverter_70cm_RF_MHz = 432;
		CALIBRATE.Transverter_70cm_IF_MHz = 144;
		CALIBRATE.Transverter_23cm_RF_MHz = 1296;
		CALIBRATE.Transverter_23cm_IF_MHz = 144;
		CALIBRATE.Transverter_13cm_RF_MHz = 2320;
		CALIBRATE.Transverter_13cm_IF_MHz = 144;
		CALIBRATE.Transverter_6cm_RF_MHz = 5760;
		CALIBRATE.Transverter_6cm_IF_MHz = 144;
		CALIBRATE.Transverter_3cm_RF_MHz = 10368;
		CALIBRATE.Transverter_3cm_IF_MHz = 144;
		CALIBRATE.Transverter_QO100_RF_kHz = 10489500;
		CALIBRATE.Transverter_QO100_IF_RX_kHz = 739500;
		CALIBRATE.Transverter_QO100_IF_TX_MHz = 144;
		CALIBRATE.Transverter_1_2cm_RF_MHz = 24048;
		CALIBRATE.Transverter_1_2cm_IF_MHz = 144;
		CALIBRATE.KTY81_Calibration = 2000;
#ifdef FRONTPANEL_LITE
		CALIBRATE.ENABLE_6m_band = false;
		CALIBRATE.ENABLE_FM_band = false;
		CALIBRATE.ENABLE_2m_band = false;
		CALIBRATE.ENABLE_70cm_band = false;
		CALIBRATE.Transverter_2m_IF_MHz = 28;
		CALIBRATE.Transverter_70cm_IF_MHz = 28;
		CALIBRATE.Transverter_23cm_IF_MHz = 28;
		CALIBRATE.Transverter_13cm_IF_MHz = 28;
		CALIBRATE.Transverter_6cm_IF_MHz = 28;
		CALIBRATE.Transverter_3cm_IF_MHz = 28;
		CALIBRATE.Transverter_QO100_IF_RX_kHz = 28500;
		CALIBRATE.Transverter_QO100_IF_TX_MHz = 28;
		CALIBRATE.Transverter_1_2cm_IF_MHz = 28;
#endif
		CALIBRATE.OTA_update = true;           // enable OTA FW update over WiFi
		CALIBRATE.TX_StartDelay = 5;           // Relay switch delay before RF signal ON, ms
		CALIBRATE.LCD_Rotate = false;          // LCD 180 degree rotation
		CALIBRATE.INA226_EN = false;           // INA226 enabled
		CALIBRATE.INA226_Shunt_mOhm = 100.0f;  // INA226 current shunt (mOhms)
		CALIBRATE.INA226_VoltageOffset = 0.0f; // INA226 voltage offset
#ifdef FRONTPANEL_WOLF_2
		CALIBRATE.INA226_EN = true;
		CALIBRATE.INA226_Shunt_mOhm = 25.0f;
#endif
		CALIBRATE.PWR_CUR_Calibration = 2.5f; // CUR meter calibration
		CALIBRATE.ATU_AVERAGING = 3;          // Tuner averaging stages
		CALIBRATE.CAT_Type = CAT_TS2000;
		CALIBRATE.LNA_compensation = 0;             // Compensation for LNA, db
		CALIBRATE.TwoSignalTune_Balance = 50;       // balance of signals on twosignal-tune
		CALIBRATE.LinearPowerControl = false;       // linear or logrithmic power control
		CALIBRATE.ALC_Port_Enabled = false;         // enable ALC port to set TRX power from external amplifier
		CALIBRATE.ALC_Inverted_Logic = false;       // invert voltage logic
		CALIBRATE.IF_GAIN_MIN = 0;                  // min limit for if gain regulator
		CALIBRATE.IF_GAIN_MAX = 40;                 // max limit for if gain regulator
		CALIBRATE.TOUCHPAD_horizontal_flip = false; // Touchpad harozontal flip
		CALIBRATE.TOUCHPAD_vertical_flip = false;   // Touchpad vertical flip
		CALIBRATE.TOUCHPAD_TIMEOUT = 50;            // Touchpad timings calibrations
		CALIBRATE.TOUCHPAD_CLICK_THRESHOLD = 5;
		CALIBRATE.TOUCHPAD_CLICK_TIMEOUT = 400;
		CALIBRATE.TOUCHPAD_HOLD_TIMEOUT = 400;
		CALIBRATE.TOUCHPAD_SWIPE_THRESHOLD_PX = 5;
		CALIBRATE.COM_CAT_DTR_Mode = COM_LINE_MODE_DISABLED; // COM Ports control lines modes
		CALIBRATE.COM_CAT_RTS_Mode = COM_LINE_MODE_DISABLED;
		CALIBRATE.COM_DEBUG_DTR_Mode = COM_LINE_MODE_KEYER;
		CALIBRATE.COM_DEBUG_RTS_Mode = COM_LINE_MODE_PTT;
#if !HRDW_HAS_USB_DEBUG
		CALIBRATE.COM_CAT_DTR_Mode = COM_LINE_MODE_KEYER;
		CALIBRATE.COM_CAT_RTS_Mode = COM_LINE_MODE_PTT;
#endif
		CALIBRATE.Swap_USB_IQ = false; // Swap IQ for USB output

		// Default memory channels
		for (uint8_t i = 0; i < MEMORY_CHANNELS_COUNT; i++) {
			CALIBRATE.MEMORY_CHANNELS[i].freq = 0;
			CALIBRATE.MEMORY_CHANNELS[i].mode = TRX_MODE_LSB;
			CALIBRATE.MEMORY_CHANNELS[i].CTCSS_Freq = 0;
			CALIBRATE.MEMORY_CHANNELS[i].RepeaterMode = false;
			sprintf(CALIBRATE.MEMORY_CHANNELS[i].name, "Ch %d", i + 1);
		}
		for (uint8_t i = 0; i < BANDS_COUNT; i++) {
			CALIBRATE.BAND_MEMORIES[i][0] = TRX.BANDS_SAVED_SETTINGS[i].Freq;

			for (uint8_t j = 1; j < BANDS_MEMORIES_COUNT; j++) {
				CALIBRATE.BAND_MEMORIES[i][j] = 0;
			}
		}

		CALIBRATE.ENDBit = 100; // Bit for the end of a successful write to eeprom

#ifdef STM32F407xx
		volatile bool NeedRot = false;
		uint8_t ii = 99;
		char buf_rot[10];
		LCD_showError("Push POWER to ROTATE or wait to SKIP", false);
		while (ii > 0) {
			sprintf(buf_rot, "%2d", ii);
			LCDDriver_printTextFont(buf_rot, 240, 200, COLOR_WHITE, COLOR_RED, (GFXfont *)&FreeSans12pt7b);
			if (HAL_GPIO_ReadPin(PWR_ON_GPIO_Port, GPIO_PIN_7) == GPIO_PIN_RESET) {
				NeedRot = !NeedRot;
				CALIBRATE.LCD_Rotate = NeedRot;
				LCD_Init();
				LCD_showError("Push POWER to ROTATE or wait to SKIP", false);
				ii = 99;
			}
			HAL_Delay(100);
			ii--;
		}
#endif

#ifdef LAY_160x128
		LCD_showError("Load default calibrate", true);
#else
		LCD_showError("Loaded default calibrations", true);
#endif
		SaveCalibration();
	}
	EEPROM_PowerDown();
	// enable bands
	BAND_SELECTABLE[BANDID_2200m] = CALIBRATE.ENABLE_2200m_band;
	BAND_SELECTABLE[BANDID_630m] = CALIBRATE.ENABLE_630m_band;
	BAND_SELECTABLE[BANDID_60m] = CALIBRATE.ENABLE_60m_band;
	BAND_SELECTABLE[BANDID_6m] = CALIBRATE.ENABLE_6m_band;
	BAND_SELECTABLE[BANDID_4m] = CALIBRATE.ENABLE_4m_band;
	BAND_SELECTABLE[BANDID_FM] = CALIBRATE.ENABLE_FM_band;
	BAND_SELECTABLE[BANDID_2m] = CALIBRATE.ENABLE_2m_band || TRX.Transverter_2m;
	BAND_SELECTABLE[BANDID_AIR] = CALIBRATE.ENABLE_AIR_band;
	BAND_SELECTABLE[BANDID_Marine] = CALIBRATE.ENABLE_marine_band;
	BAND_SELECTABLE[BANDID_70cm] = CALIBRATE.ENABLE_70cm_band || TRX.Transverter_70cm;
	BAND_SELECTABLE[BANDID_23cm] = CALIBRATE.ENABLE_23cm_band || TRX.Transverter_23cm;

	// load WiFi settings after calibrations
	LoadWiFiSettings(false);

	// reset ATU on Hard Reset
	if (clear) {
		ResetATUBanks();
	}

	// load ATU
	ATU_Load_ANT_Banks();
}

void LoadWiFiSettings(bool clear) {
	EEPROM_PowerUp();
	uint8_t tryes = 0;
	while (tryes < EEPROM_REPEAT_TRYES && !EEPROM_Read_Data((uint8_t *)&WIFI, sizeof(WIFI), EEPROM_SECTOR_WIFI, true, false)) {
		tryes++;
	}
	if (tryes >= EEPROM_REPEAT_TRYES) {
		println("[ERR] Read EEPROM WIFI multiple errors");
		LCD_showError("EEPROM Error", true);
	}

	if (WIFI.ENDBit != 100 || WIFI.flash_id != WIFI_SETTINGS_VERSION || clear ||
	    WIFI.csum != calculateCSUM_WIFI()) // code for checking the firmware in the eeprom, if it does not match, we use the default
	{
		memset(&WIFI, 0x00, sizeof(WIFI));

		println("[ERR] WIFI Flash check CODE:", WIFI.flash_id);
		WIFI.flash_id = WIFI_SETTINGS_VERSION; // code for checking the firmware in the eeprom, if it does not match, we use the default

		WIFI.Enabled = true;                      // activate WiFi
		strcpy(WIFI.AP_1, "WIFI-AP");             // WiFi access point 1
		strcpy(WIFI.Password_1, "WIFI-PASSWORD"); // password to the WiFi point 1
		strcpy(WIFI.AP_2, "WIFI-AP");             // WiFi access point 2
		strcpy(WIFI.Password_2, "WIFI-PASSWORD"); // password to the WiFi point 2
		strcpy(WIFI.AP_3, "WIFI-AP");             // WiFi access point 3
		strcpy(WIFI.Password_3, "WIFI-PASSWORD"); // password to the WiFi point 3
		WIFI.Timezone = 3;                        // time zone (for time synchronization)
		WIFI.CAT_Server = false;                  // Server for receiving CAT commands via WIFI
		strcpy(WIFI.ALLQSO_TOKEN, "");            // Token from AllQSO.ru
		strcpy(WIFI.ALLQSO_LOGID, "");            // LogId from AllQSO.ru

		WIFI.ENDBit = 100; // Bit for the end of a successful write to eeprom

		SaveWiFiSettings();
	}
	EEPROM_PowerDown();
}

void SaveSettings(void) {
	BKPSRAM_Enable();
	TRX.csum = calculateCSUM();
	Aligned_CleanDCache_by_Addr((uint32_t *)&TRX, sizeof(TRX));

	memcpy(BACKUP_SRAM_BANK_ADDR, &TRX, sizeof(TRX));
	Aligned_CleanDCache_by_Addr(BACKUP_SRAM_BANK_ADDR, sizeof(TRX));

	BKPSRAM_Disable();
	NeedSaveSettings = false;
	// sendToDebug_str("[OK] Settings Saved to bank ");
	// sendToDebug_uint8(settings_bank, false);
	// sendToDebug_uint32(sizeof(TRX), false);
}

void SaveSettingsToEEPROM(void) {
	if (EEPROM_Busy || HRDW_SPI_Locked) {
		return;
	}
	EEPROM_PowerUp();
	EEPROM_Busy = true;
	TRX.csum = calculateCSUM();
	uint8_t tryes = 0;
	while (tryes < EEPROM_REPEAT_TRYES && !EEPROM_Sector_Erase(EEPROM_SECTOR_SETTINGS, false)) {
		println("[ERR] Erase EEPROM Settings error");
		print_flush();
		tryes++;
	}
	if (tryes >= EEPROM_REPEAT_TRYES) {
		println("[ERR] Erase EEPROM Settings multiple errors");
		print_flush();
		// LCD_showError("EEPROM Error", true);
		EEPROM_Busy = false;
		return;
	}
	tryes = 0;
	while (tryes < EEPROM_REPEAT_TRYES && !EEPROM_Write_Data((uint8_t *)&TRX, sizeof(TRX), EEPROM_SECTOR_SETTINGS, true, false)) {
		println("[ERR] Write EEPROM Settings error");
		print_flush();
		EEPROM_Sector_Erase(EEPROM_SECTOR_SETTINGS, false);
		tryes++;
	}
	if (tryes >= EEPROM_REPEAT_TRYES) {
		println("[ERR] Write EEPROM Settings multiple errors");
		print_flush();
		// LCD_showError("EEPROM Error", true);
		EEPROM_Busy = false;
		return;
	}

	EEPROM_Busy = false;
	EEPROM_PowerDown();
	println("[OK] EEPROM Settings Saved");
	print_flush();
}

void SaveCalibration(void) {
	if (EEPROM_Busy || HRDW_SPI_Locked) {
		return;
	}
	EEPROM_PowerUp();
	EEPROM_Busy = true;

	CALIBRATE.csum = calculateCSUM_EEPROM();
	uint8_t tryes = 0;
	while (tryes < EEPROM_REPEAT_TRYES && !EEPROM_Sector_Erase(EEPROM_SECTOR_CALIBRATION, false)) {
		println("[ERR] Erase EEPROM calibrate error");
		print_flush();
		tryes++;
	}
	if (tryes >= EEPROM_REPEAT_TRYES) {
		println("[ERR] Erase EEPROM calibrate multiple errors");
		print_flush();
		// LCD_showError("EEPROM Error", true);
		EEPROM_Busy = false;
		return;
	}
	tryes = 0;
	while (tryes < EEPROM_REPEAT_TRYES && !EEPROM_Write_Data((uint8_t *)&CALIBRATE, sizeof(CALIBRATE), EEPROM_SECTOR_CALIBRATION, true, false)) {
		println("[ERR] Write EEPROM calibrate error");
		print_flush();
		EEPROM_Sector_Erase(EEPROM_SECTOR_CALIBRATION, false);
		tryes++;
	}
	if (tryes >= EEPROM_REPEAT_TRYES) {
		println("[ERR] Write EEPROM calibrate multiple errors");
		print_flush();
		// LCD_showError("EEPROM Error", true);
		EEPROM_Busy = false;
		return;
	}

	EEPROM_Busy = false;
	EEPROM_PowerDown();
	println("[OK] EEPROM Calibrations Saved");
	print_flush();
	NeedSaveCalibration = false;
}

void SaveWiFiSettings(void) {
	if (EEPROM_Busy || HRDW_SPI_Locked) {
		return;
	}
	EEPROM_PowerUp();
	EEPROM_Busy = true;

	WIFI.csum = calculateCSUM_WIFI();
	uint8_t tryes = 0;
	while (tryes < EEPROM_REPEAT_TRYES && !EEPROM_Sector_Erase(EEPROM_SECTOR_WIFI, false)) {
		println("[ERR] Erase EEPROM WIFI error");
		print_flush();
		tryes++;
	}
	if (tryes >= EEPROM_REPEAT_TRYES) {
		println("[ERR] Erase EEPROM WIFI multiple errors");
		print_flush();
		// LCD_showError("EEPROM Error", true);
		EEPROM_Busy = false;
		return;
	}
	tryes = 0;
	while (tryes < EEPROM_REPEAT_TRYES && !EEPROM_Write_Data((uint8_t *)&WIFI, sizeof(WIFI), EEPROM_SECTOR_WIFI, true, false)) {
		println("[ERR] Write EEPROM WIFI error");
		print_flush();
		EEPROM_Sector_Erase(EEPROM_SECTOR_WIFI, false);
		tryes++;
	}
	if (tryes >= EEPROM_REPEAT_TRYES) {
		println("[ERR] Write EEPROM WIFI multiple errors");
		print_flush();
		// LCD_showError("EEPROM Error", true);
		EEPROM_Busy = false;
		return;
	}

	EEPROM_Busy = false;
	EEPROM_PowerDown();
	println("[OK] EEPROM WIFI Settings Saved");
	print_flush();
	NeedSaveWiFi = false;
}

bool LoadDPDSettings(uint8_t *out, uint32_t size, uint32_t sector_offset) {
	EEPROM_PowerUp();

	uint8_t tryes = 0;
	while (tryes < EEPROM_REPEAT_TRYES && !EEPROM_Read_Data(out, size, EEPROM_SECTOR_DPD + sector_offset, true, false)) {
		tryes++;
	}
	if (tryes >= EEPROM_REPEAT_TRYES) {
		println("[ERR] Load EEPROM DPD multiple errors");
		return false;
	}

	EEPROM_PowerDown();
	return true;
}

bool SaveDPDSettings(uint8_t *in, uint32_t size, uint32_t sector_offset) {
	EEPROM_PowerUp();

	uint8_t tryes = 0;
	while (tryes < EEPROM_REPEAT_TRYES && !EEPROM_Sector_Erase(EEPROM_SECTOR_DPD + sector_offset, false)) {
		tryes++;
	}
	if (tryes >= EEPROM_REPEAT_TRYES) {
		println("[ERR] Erase EEPROM DPD multiple errors");
		return false;
	}

	tryes = 0;
	while (tryes < EEPROM_REPEAT_TRYES && !EEPROM_Write_Data(in, size, EEPROM_SECTOR_DPD + sector_offset, true, false)) {
		tryes++;
	}
	if (tryes >= EEPROM_REPEAT_TRYES) {
		println("[ERR] Write EEPROM DPD multiple errors");
		return false;
	}
	EEPROM_PowerDown();
	return true;
}

bool LoadATUSettings(uint8_t *out, uint32_t size, uint32_t sector) {
	EEPROM_PowerUp();
	println("Load ATU memory ", sector);

	uint8_t tryes = 0;
	while (tryes < EEPROM_REPEAT_TRYES && !EEPROM_Read_Data(out, size, sector, true, false)) {
		tryes++;
	}
	if (tryes >= EEPROM_REPEAT_TRYES) {
		println("[ERR] Load EEPROM ATU multiple errors");
		return false;
	}

	EEPROM_PowerDown();
	return true;
}

bool SaveATUSettings(uint8_t *in, uint32_t size, uint32_t sector) {
	EEPROM_PowerUp();
	println("Save ATU memory ", sector);

	uint8_t tryes = 0;
	while (tryes < EEPROM_REPEAT_TRYES && !EEPROM_Sector_Erase(sector, false)) {
		tryes++;
	}
	if (tryes >= EEPROM_REPEAT_TRYES) {
		println("[ERR] Erase EEPROM ATU multiple errors");
		return false;
	}

	tryes = 0;
	while (tryes < EEPROM_REPEAT_TRYES && !EEPROM_Write_Data(in, size, sector, true, false)) {
		tryes++;
	}
	if (tryes >= EEPROM_REPEAT_TRYES) {
		println("[ERR] Write EEPROM ATU multiple errors");
		return false;
	}
	EEPROM_PowerDown();
	return true;
}

static bool EEPROM_Sector_Erase(uint8_t sector, bool force) {
	if (!force && !EEPROM_Enabled) {
		return true;
	}
	if (!force && HRDW_SPI_Locked) {
		return false;
	} else {
		HRDW_SPI_Locked = true;
	}

	uint32_t BigAddress = sector * W25Q16_SECTOR_SIZE;
	CmdAddress[0] = W25Q16_COMMAND_Sector_Erase;
	CmdAddress[1] = (BigAddress >> 16) & 0xFF;
	CmdAddress[2] = (BigAddress >> 8) & 0xFF;
	CmdAddress[3] = BigAddress & 0xFF;

	HRDW_EEPROM_SPI(&Write_Enable, NULL, 1, false); // Write Enable Command
	HRDW_EEPROM_SPI(CmdAddress, NULL, 4, false);    // Erase Command + Write Address ( The first address of flash module is 0x00000000 )
	EEPROM_WaitWrite();

	HRDW_SPI_Locked = false;
	return true;
}

static bool EEPROM_Write_Data(uint8_t *Buffer, uint16_t size, uint8_t sector, bool verify, bool force) {
	if (!force && !EEPROM_Enabled) {
		return true;
	}
	if (!force && HRDW_SPI_Locked) {
		return false;
	} else {
		HRDW_SPI_Locked = true;
	}
	if (size > sizeof(verify_clone)) {
		println("EEPROM WR buffer error");
		HRDW_SPI_Locked = false;
		return false;
	}
	Aligned_CleanDCache_by_Addr((uint32_t *)Buffer, size);

	for (uint16_t page = 0; page <= (size / W25Q16_PAGE_SIZE); page++) {
		uint32_t BigAddress = (page * W25Q16_PAGE_SIZE) + (sector * W25Q16_SECTOR_SIZE);
		CmdAddress[0] = W25Q16_COMMAND_Page_Program;
		CmdAddress[1] = (BigAddress >> 16) & 0xFF;
		CmdAddress[2] = (BigAddress >> 8) & 0xFF;
		CmdAddress[3] = BigAddress & 0xFF;
		uint16_t bsize = size - W25Q16_PAGE_SIZE * page;
		if (bsize > W25Q16_PAGE_SIZE) {
			bsize = W25Q16_PAGE_SIZE;
		}
		if (bsize == 0) {
			continue;
		}
		HRDW_EEPROM_SPI(&Write_Enable, NULL, 1, false);                                     // Write Enable Command
		HRDW_EEPROM_SPI(CmdAddress, NULL, 4, true);                                         // Write Command + Write Address ( The first address of flash module is 0x00000000 )
		HRDW_EEPROM_SPI((uint8_t *)(Buffer + W25Q16_PAGE_SIZE * page), NULL, bsize, false); // Write Data
		EEPROM_WaitWrite();
	}

	// verify
	if (verify) {
		int16_t last_verified_err = -2;
		int16_t prev_verified_err = -1;
		bool verify_succ = true;
		uint16_t verify_tryes = 0;
		while (last_verified_err != prev_verified_err && verify_tryes < 5) {
			verify_succ = true;
			EEPROM_Read_Data(verify_clone, size, sector, false, true);
			for (uint16_t i = 0; i < size; i++) {
				if (verify_clone[i] != Buffer[i]) {
					verify_tryes++;
					prev_verified_err = last_verified_err;
					last_verified_err = i;
					println("EEROM Verify error, pos:", i, " mem:", Buffer[i], " fla:", verify_clone[i]);
					print_flush();
					verify_succ = false;
					break;
				}
			}
			if (verify_succ) {
				break;
			}
		}
		if (!verify_succ) {
			HRDW_SPI_Locked = false;
			return false;
		}
	}
	HRDW_SPI_Locked = false;
	return true;
}

static bool EEPROM_Read_Data(uint8_t *Buffer, uint16_t size, uint8_t sector, bool verify, bool force) {
	if (!force && !EEPROM_Enabled) {
		return true;
	}
	if (!force && HRDW_SPI_Locked) {
		return false;
	} else {
		HRDW_SPI_Locked = true;
	}

	if (size > sizeof(verify_clone)) {
		println("EEPROM RD buffer error");
		HRDW_SPI_Locked = false;
		return false;
	}

	Aligned_CleanDCache_by_Addr((uint32_t *)Buffer, size);

	uint32_t BigAddress = sector * W25Q16_SECTOR_SIZE;
	CmdAddress[0] = W25Q16_COMMAND_Read_Data;
	CmdAddress[1] = (BigAddress >> 16) & 0xFF;
	CmdAddress[2] = (BigAddress >> 8) & 0xFF;
	CmdAddress[3] = BigAddress & 0xFF;

	bool read_ok = false;
	int8_t tryes = 0;
	while (!read_ok && tryes < 5) {
		bool res = HRDW_EEPROM_SPI(CmdAddress, NULL, 4, true); // Read Command + Write Address
		if (!res) {
			EEPROM_Enabled = false;
			println("[ERR] EEPROM not found...");
			if (!TRX_Inited) {
				LCD_showError("EEPROM init error", true);
			}
			HRDW_SPI_Locked = false;
			return true;
		}

		read_ok = HRDW_EEPROM_SPI(NULL, (uint8_t *)(Buffer), size, false); // Read data
		tryes++;
	}

	Aligned_CleanInvalidateDCache_by_Addr((uint32_t *)Buffer, size);

	// verify
	if (verify) {
		Aligned_CleanDCache_by_Addr((uint32_t *)verify_clone, size);

		BigAddress = sector * W25Q16_SECTOR_SIZE;
		CmdAddress[0] = W25Q16_COMMAND_Read_Data;
		CmdAddress[1] = (BigAddress >> 16) & 0xFF;
		CmdAddress[2] = (BigAddress >> 8) & 0xFF;
		CmdAddress[3] = BigAddress & 0xFF;

		bool res = HRDW_EEPROM_SPI(CmdAddress, NULL, 4, true); // Read Command + Write Address
		if (!res) {
			EEPROM_Enabled = false;
			println("[ERR] EEPROM not found...");
			if (!TRX_Inited) {
				LCD_showError("EEPROM init error", true);
			}
			HRDW_SPI_Locked = false;
			return true;
		}

		HRDW_EEPROM_SPI(NULL, (uint8_t *)verify_clone, size, false); // Read data

		Aligned_CleanInvalidateDCache_by_Addr((uint32_t *)verify_clone, size);

		for (uint16_t i = 0; i < size; i++) {
			if (verify_clone[i] != Buffer[i]) {
				// println("read err", verify_clone[i]);
				HRDW_SPI_Locked = false;
				return false;
			}
		}
	}
	if (!force) {
		HRDW_SPI_Locked = false;
	}
	return true;
}

static void EEPROM_WaitWrite(void) {
	if (!EEPROM_Enabled) {
		return;
	}
	uint8_t status = 0;
	uint8_t tryes = 0;
	do {
		tryes++;
		HRDW_EEPROM_SPI(&Get_Status, NULL, 1, true); // Get Status command
		HRDW_EEPROM_SPI(NULL, &status, 1, false);    // Read data
		if ((status & 0x01) == 0x01) {
			HAL_Delay(1);
		}
	} while ((status & 0x01) == 0x01 && (tryes < 200));
	if (tryes == 200) {
		println("[ERR]EEPROM Lock wait error");
		print_flush();
	}
}

static void EEPROM_PowerDown(void) {
	if (!EEPROM_Enabled) {
		return;
	}
	HRDW_EEPROM_SPI(&Power_Down, NULL, 1, false); // Power_Down Command
}

static void EEPROM_PowerUp(void) {
	if (!EEPROM_Enabled) {
		return;
	}
	HRDW_EEPROM_SPI(&Power_Up, NULL, 1, false); // Power_Up Command
}

void BKPSRAM_Enable(void) {
#ifdef STM32F407xx
	RCC->APB1ENR |= RCC_APB1ENR_PWREN;
	RCC->AHB1ENR |= RCC_AHB1ENR_BKPSRAMEN;
#endif

	HAL_PWREx_EnableBkUpReg();
	HAL_PWR_EnableBkUpAccess();

#ifdef STM32F407xx
	*(__IO uint32_t *)CSR_BRE_BB = (uint32_t)ENABLE;
	while (!(PWR->CSR & (PWR_FLAG_BRR))) {
		;
	}
#endif
}

void BKPSRAM_Disable(void) { HAL_PWR_DisableBkUpAccess(); }

static uint8_t calculateCSUM(void) {
#if HRDW_HAS_SD
	sd_crc_generate_table();
	uint8_t csum_old = TRX.csum;
	uint8_t csum_new = 0;
	TRX.csum = 0;
	uint8_t *TRX_addr = (uint8_t *)&TRX;
	for (uint16_t i = 0; i < sizeof(TRX); i++) {
		csum_new = sd_crc7_byte(csum_new, *(TRX_addr + i));
	}
	TRX.csum = csum_old;
	return csum_new;
#else
	return 100;
#endif
}

static uint8_t calculateCSUM_EEPROM(void) {
#if HRDW_HAS_SD
	sd_crc_generate_table();
	uint8_t csum_old = CALIBRATE.csum;
	uint8_t csum_new = 0;
	CALIBRATE.csum = 0;
	uint8_t *CALIBRATE_addr = (uint8_t *)&CALIBRATE;
	for (uint16_t i = 0; i < sizeof(CALIBRATE); i++) {
		csum_new = sd_crc7_byte(csum_new, *(CALIBRATE_addr + i));
	}
	CALIBRATE.csum = csum_old;
	return csum_new;
#else
	return 100;
#endif
}

static uint8_t calculateCSUM_WIFI(void) {
#if HRDW_HAS_SD
	sd_crc_generate_table();
	uint8_t csum_old = WIFI.csum;
	uint8_t csum_new = 0;
	WIFI.csum = 0;
	uint8_t *WIFI_addr = (uint8_t *)&WIFI;
	for (uint16_t i = 0; i < sizeof(WIFI); i++) {
		csum_new = sd_crc7_byte(csum_new, *(WIFI_addr + i));
	}
	WIFI.csum = csum_old;
	return csum_new;
#else
	return 100;
#endif
}

void RTC_Calibration(void) {
	if (hrtc.Init.AsynchPrediv != CALIBRATE.RTC_Coarse_Calibration) {
		HAL_PWR_EnableBkUpAccess();
		hrtc.Init.AsynchPrediv = CALIBRATE.RTC_Coarse_Calibration;
		HAL_RTC_Init(&hrtc);
	}

	HAL_RTCEx_SetSmoothCalib(&hrtc, RTC_SMOOTHCALIB_PERIOD_32SEC, RTC_SMOOTHCALIB_PLUSPULSES_RESET, 0);

	// 0.954 ppm (0.5 tick RTCCLK on 32 sec).
	// 1.908 ppm (0.5 tick RTCCLK on 16 sec).
	// 3.816 ppm (0.5 tick RTCCLK on 8 sec).

	// insert (clock TOO FAST, ADD cycles)
	if (CALIBRATE.RTC_Calibration > 0) {
		HAL_RTCEx_SetSmoothCalib(&hrtc, RTC_SMOOTHCALIB_PERIOD_32SEC, RTC_SMOOTHCALIB_PLUSPULSES_SET, 512 - CALIBRATE.RTC_Calibration);
	}

	// remove (clock TOO SLOW, REMOVE cycles)
	if (CALIBRATE.RTC_Calibration < 0) {
		uint32_t newval = -CALIBRATE.RTC_Calibration;
		HAL_RTCEx_SetSmoothCalib(&hrtc, RTC_SMOOTHCALIB_PERIOD_32SEC, RTC_SMOOTHCALIB_PLUSPULSES_RESET, newval);
	}
}
