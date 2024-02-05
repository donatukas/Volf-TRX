#ifndef TRAFFIC_MANAGER_H_
#define TRAFFIC_MANAGER_H_

#include <arm_math.h>
#include <stdint.h>

#define FT8_Freq_160M 1840 // in kHz
#define FT8_Freq_80M 3573
#define FT8_Freq_60M 5357
#define FT8_Freq_40M 7074
#define FT8_Freq_30M 10136
#define FT8_Freq_20M 14074
#define FT8_Freq_17M 18100
#define FT8_Freq_15M 21074
#define FT8_Freq_12M 24915
#define FT8_Freq_10M 28074
#define FT8_Freq_6M 50313
#define FT8_Freq_2M 144174
#define FT8_Freq_70CM 432174
#define FT8_Freq_QO_100 10489540

extern uint16_t cursor_freq;  // the AF frequency wich will be tansmited now (roughly from 0 to 3kHz)
extern uint64_t FT8_BND_Freq; // frequency for the FT8 on the current Band
extern int xmit_flag, ft8_xmit_counter;
extern int Beacon_State; //

void transmit_sequence(void);
void receive_sequence(void);
void set_Xmit_Freq(uint64_t BandFreq, float32_t Freq);
void set_FT8_Tone(char ft8_tone);
void setup_to_transmit_on_next_DSP_Flag(void);
void tune_On_sequence(void);
void tune_Off_sequence(void);
void service_CQ(void);

void GetQSOTime(uint8_t QSO_Start);
void GetQSODate(void);

void LogQSO(void);

#endif