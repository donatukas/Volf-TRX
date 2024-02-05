#include "agc.h"
#include "codec.h"
#include "hardware.h"
#include "i2c.h"
#include "lcd.h"
#include "trx_manager.h"
#include "usbd_audio_if.h"

#ifdef HRDW_AUDIO_CODEC_NAU8822

static uint8_t Codec_on_board(uint8_t reg, uint8_t value) {
	uint8_t st = 2;
	uint8_t repeats = 0;
	while (st != 0 && repeats < 10) {
		i2c_beginTransmission_u8(&I2C_CODEC, B8(0011010)); // I2C_ADDRESS_WM8731 00110100
		i2c_write_u8(&I2C_CODEC, reg);                     // MSB
		i2c_write_u8(&I2C_CODEC, value);                   // MSB
		st = i2c_endTransmission(&I2C_CODEC);
		if (st != 0) {
			repeats++;
			if (TRX.Debug_Type == TRX_DEBUG_I2C) {
				println("I2C codec error, send to register: ", reg, " value: ", value, " err_code: ", st, " repeat: ", repeats);
			}
		}
		HAL_Delay(1);
	}
	if (TRX.Debug_Type == TRX_DEBUG_I2C) {
		println("I2C codec end transmit, status: ", st);
	}
	return st;
}

void NAU8822_SendI2CCommand(uint8_t u8addr, uint16_t u16data) {
	uint8_t st = 2;
	uint8_t repeats = 0;
	while (st != 0 && repeats < 3) {
		i2c_begin(&I2C_CODEC);
		i2c_start(&I2C_CODEC);
		i2c_shift_out(&I2C_CODEC, B8(00110100)); // I2C_ADDRESS_WM8731 00110100
		if (!i2c_get_ack(&I2C_CODEC)) {
			println("No ACK1");
		}

		i2c_shift_out(&I2C_CODEC, (uint8_t)((u8addr << 1) | (u16data >> 8)));
		if (!i2c_get_ack(&I2C_CODEC)) {
			println("No ACK2");
		}
		i2c_shift_out(&I2C_CODEC, (uint8_t)(u16data & 0x00FF));
		if (!i2c_get_ack(&I2C_CODEC)) {
			println("No ACK3");
		}
		//	i2c_stop(&I2C_WM8731);

		st = i2c_endTransmission(&I2C_CODEC);
		if (st != 0) {
			repeats++;
		}
		HAL_Delay(1);
	}
}

// switch to mixed RX-TX mode (for LOOP)
void CODEC_TXRX_mode(void) // loopback
{
	NAU8822_SendI2CCommand(11, 0x0FF); /* DAC left digital volume control */
	NAU8822_SendI2CCommand(12, 0x1FF); /* DAC right digital volume control */
	NAU8822_SendI2CCommand(54, 0x03F);
	NAU8822_SendI2CCommand(55, 0x13F);

	// NAU8822_SendI2CCommand(52, 0x03F);
	// NAU8822_SendI2CCommand(53, 0x13F);

	if (TRX.BluetoothAudio_Enabled) {
		NAU8822_SendI2CCommand(52, 0x7F); //  OFF / LHP
		NAU8822_SendI2CCommand(53, 0x7F); //  OFF / RHP
		NAU8822_SendI2CCommand(3, 0x1EF); // AUX1 AUX2 ON
	} else {
		NAU8822_SendI2CCommand(52, 0x03F); //  ON / LHP
		NAU8822_SendI2CCommand(53, 0x13F); //  ON / RHP
		NAU8822_SendI2CCommand(3, 0xF);    // AUX1 AUX2 OF
	}

	if (getInputType() == TRX_INPUT_MIC || getInputType() == TRX_INPUT_LINE) // mic
	{
		NAU8822_SendI2CCommand(15, 0x000);
		NAU8822_SendI2CCommand(16, 0x1ff);
		NAU8822_SendI2CCommand(45, 0x000); // PGA 0x03f
		NAU8822_SendI2CCommand(46, 0x010); // PGA 0x13f
		NAU8822_SendI2CCommand(47, 0x000);
		if (TRX.MIC_Boost) {
			NAU8822_SendI2CCommand(48, 0x104); // 0x1ff
		} else {
			NAU8822_SendI2CCommand(48, 0x7); // 0x1ff
		}
		NAU8822_SendI2CCommand(44, 0x030);
	}
	if (getInputType() == TRX_INPUT_USB) {
		NAU8822_SendI2CCommand(15, 0x000);
		NAU8822_SendI2CCommand(16, 0x000);
		NAU8822_SendI2CCommand(45, 0x040); // PGA
		NAU8822_SendI2CCommand(46, 0x040); // PGA
		NAU8822_SendI2CCommand(47, 0x000);
		NAU8822_SendI2CCommand(48, 0x000);
		NAU8822_SendI2CCommand(44, 0x000);
	}
}

void NAU8822_Speaker_off(bool status) {
	if (status) {
		NAU8822_SendI2CCommand(43, 0x020); // Speaker OFF
		NAU8822_SendI2CCommand(54, 0x040);
		NAU8822_SendI2CCommand(55, 0x040);
	} else {
		NAU8822_SendI2CCommand(43, 0x010); // Speaker ON
		NAU8822_SendI2CCommand(54, 0x03F);
		NAU8822_SendI2CCommand(55, 0x13F);
	}
}

// initialize the audio codec over I2C
void CODEC_Init(void) {
	if (Codec_on_board(B8(00011110), B8(00000000)) != 0) // R15 Reset Chip
	{
		println("[ERR] Audio codec not found");
		LCD_showError("Audio codec init error", true);
		CODEC_test_result = false;
	}

	NAU8822_SendI2CCommand(0, 0x000); /* Reset all registers */
	HAL_Delay(0x200);

	NAU8822_SendI2CCommand(52, 0x000);
	NAU8822_SendI2CCommand(53, 0x100);
	NAU8822_SendI2CCommand(54, 0x000);
	NAU8822_SendI2CCommand(55, 0x100); // Vol

	NAU8822_SendI2CCommand(43, 0x020); // Speaker out 0x010, Mute 0x020

	NAU8822_SendI2CCommand(1, 0xFD); // 0x01D
	NAU8822_SendI2CCommand(2, 0x1BF);
	NAU8822_SendI2CCommand(3, 0x1EF);
	NAU8822_SendI2CCommand(56, B8(00000111)); // AUX

	HAL_Delay(250);

	NAU8822_SendI2CCommand(4, 0x078); /* Audio Interface, i2s, 32bit 0x070-i2s 0x078-mic work*/
	NAU8822_SendI2CCommand(5, 0x000); /*  */
	NAU8822_SendI2CCommand(6, 0x000); /* Deviders */
	NAU8822_SendI2CCommand(7, 0x000); /*  */

	NAU8822_SendI2CCommand(10, 0x008); /* DAC soft mute is disabled, DAC oversampling rate is 128x */
	NAU8822_SendI2CCommand(14, 0x108); /* ADC HP filter is disabled, ADC oversampling rate is 128x */

	NAU8822_SendI2CCommand(11, 0x0FF);
	NAU8822_SendI2CCommand(12, 0x1FF);

	NAU8822_SendI2CCommand(43, 0x010); // Speaker out 0x010, Mute 0x020

	NAU8822_SendI2CCommand(49, 0x006);

	NAU8822_SendI2CCommand(50, 0x000); // 0x001 /* Left DAC connected to LMIX 0x01f mik na dinamik */
	NAU8822_SendI2CCommand(51, 0x01d); // 0x001 /* Right DAC connected to RMIX 0x01f mik na dinamik !!! */

	NAU8822_SendI2CCommand(44, 0x030); /* LMICN/LMICP is connected to PGA */

	NAU8822_SendI2CCommand(45, 0x03f); // PGA
	NAU8822_SendI2CCommand(46, 0x03f); // PGA

	NAU8822_SendI2CCommand(65, 0x000);

	println("[OK] Audio codec inited");

	CODEC_UnMute();
}

#endif
