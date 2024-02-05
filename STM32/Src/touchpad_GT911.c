#include "i2c.h"
#include "lcd.h"
#include "touchpad.h"
#if (defined(TOUCHPAD_GT911))

GT911_Dev GT911 = {0};
static uint8_t gt911_i2c_addr = GT911_I2C_ADDR_1;

static uint8_t GT911_Config[186] = {
    0x5B, 0x20, 0x03, 0xE0, 0x01, 0x05, 0x3D, 0x20, 0x22, 0x08, 0x28, 0x08, 0x64, 0x46, 0x03, 0x05, 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x00, 0x18, 0x1A, 0x1E, 0x14, 0x8C, 0x2E, 0x0E, 0xB1,
    0xB3, 0xB8, 0x08, 0x00, 0x00, 0x00, 0xD9, 0x02, 0x11, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9B, 0xD0, 0x9E, 0xD5, 0xF4, 0x08, 0x00, 0x00, 0x04, 0x83, 0x9F,
    0x00, 0x81, 0xA9, 0x00, 0x7F, 0xB3, 0x00, 0x7E, 0xBE, 0x00, 0x7E, 0xCA, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C, 0x0E, 0x10, 0x12, 0x14, 0x16, 0x18,
    0x1A, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2A, 0x29, 0x28, 0x26, 0x24, 0x22, 0x21, 0x20, 0x1F, 0x1E, 0x1D, 0x1C, 0x18,
    0x16, 0x14, 0x13, 0x12, 0x10, 0x0F, 0x0C, 0x0A, 0x08, 0x06, 0x04, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x56, 0x00,
};

uint8_t GT911_WR_Reg(uint16_t reg, uint8_t *buf, uint8_t len) {
	i2c_beginTransmission_u8(&I2C_SHARED_BUS, gt911_i2c_addr);
	i2c_write_u8(&I2C_SHARED_BUS, (reg >> 8) & 0xFF);
	i2c_write_u8(&I2C_SHARED_BUS, reg & 0xFF);
	for (uint8_t i = 0; i < len; i++) {
		i2c_write_u8(&I2C_SHARED_BUS, buf[i]);
	}
	uint8_t ret = i2c_endTransmission(&I2C_SHARED_BUS);
	if (ret != SUCCESS && TRX.Debug_Type == TRX_DEBUG_I2C) {
		println("I2C GT911 Write error: ", ret);
	}
	return ret;
}

void GT911_RD_RegOneByte(uint16_t reg, uint8_t *buf) {
	i2c_beginTransmission_u8(&I2C_SHARED_BUS, gt911_i2c_addr);
	i2c_write_u8(&I2C_SHARED_BUS, (reg >> 8) & 0xFF);
	i2c_write_u8(&I2C_SHARED_BUS, reg & 0xFF);
	uint8_t res = i2c_endTransmission(&I2C_SHARED_BUS);
	if (res == 0) {
		if (i2c_beginReceive_u8(&I2C_SHARED_BUS, gt911_i2c_addr)) {
			*buf = i2c_Read_Byte(&I2C_SHARED_BUS, 0);
			i2c_stop(&I2C_SHARED_BUS);
		}
	} else {
		println("[ERR] No touchpad found on i2c bus");

		// try new i2c addr if failed
		if (gt911_i2c_addr == GT911_I2C_ADDR_1) {
			gt911_i2c_addr = GT911_I2C_ADDR_2;
		} else {
			gt911_i2c_addr = GT911_I2C_ADDR_1;
		}
	}
}

void GT911_RD_Reg(uint16_t reg, uint8_t *buf, uint8_t len) {
	for (uint8_t i = 0; i < len; i++) {
		GT911_RD_RegOneByte(reg + i, &buf[i]);
	}
}

void GT911_ReadStatus(void) {
	if (I2C_SHARED_BUS.locked) {
		return;
	}
	I2C_SHARED_BUS.locked = true;

	uint8_t buf[4] = {0};
	GT911_RD_Reg(GT911_PRODUCT_ID_REG, (uint8_t *)&buf[0], 3);
	GT911_RD_Reg(GT911_CONFIG_REG, (uint8_t *)&buf[3], 1);

	static IRAM2 char str[64] = {0};
	sprintf(str, "TouchPad_ID:%d,%d,%d\r\nTouchPad_Config_Version:%2x", buf[0], buf[1], buf[2], buf[3]);
	println(str);

	I2C_SHARED_BUS.locked = false;
}

void GT911_ReadFirmwareVersion(void) {
	if (I2C_SHARED_BUS.locked) {
		return;
	}
	I2C_SHARED_BUS.locked = true;

	uint8_t buf[2] = {0};
	GT911_RD_Reg(GT911_FIRMWARE_VERSION_REG, buf, 2);

	static IRAM2 char str[128] = {0};
	sprintf(str, "FirmwareVersion:%2x", (((uint16_t)buf[1] << 8) + buf[0]));
	println(str);

	I2C_SHARED_BUS.locked = false;
}

void GT911_Init(void) {
	if (I2C_SHARED_BUS.locked) {
		return;
	}
	I2C_SHARED_BUS.locked = true;

	uint8_t buf[1] = {0};
	buf[0] = GOODIX_CMD_BASEUPDATE;
	GT911_WR_Reg(GT911_COMMAND_REG, buf, 1);
	HAL_Delay(100);
	buf[0] = GOODIX_CMD_CALIBRATE;
	GT911_WR_Reg(GT911_COMMAND_REG, buf, 1);
	HAL_Delay(100);

	println("Touchpad calibrated");

	I2C_SHARED_BUS.locked = false;
}

bool GT911_Flash(void) {
	if (I2C_SHARED_BUS.locked) {
		return false;
	}
	I2C_SHARED_BUS.locked = true;

	uint8_t config_Checksum = 0;
	uint8_t i = 0;
	uint16_t X_OUTPUT_MAX = LCD_WIDTH;
	uint16_t Y_OUTPUT_MAX = LCD_HEIGHT;

	GT911_RD_Reg(GT911_CONFIG_REG, (uint8_t *)&GT911_Config[0], 186);
	for (i = 0; i < sizeof(GT911_Config) - 2; i++) {
		config_Checksum += GT911_Config[i];

		printf("0x%02X  ", GT911_Config[i]);
		if ((i + 1) % 10 == 0) {
			printf("\r\n");
		}
	}
	printf("0x%02X  0x%02X\r\nconfig_Checksum=0x%2X\r\n", GT911_Config[184], GT911_Config[185], ((~config_Checksum) + 1) & 0xff);

	if (GT911_Config[184] == (((~config_Checksum) + 1) & 0xff)) {
		printf("READ CONFIG SUCCESS!\r\n");
		printf("%d*%d\r\n", GT911_Config[2] << 8 | GT911_Config[1], GT911_Config[4] << 8 | GT911_Config[3]);

		if ((X_OUTPUT_MAX != (GT911_Config[2] << 8 | GT911_Config[1])) || (Y_OUTPUT_MAX != (GT911_Config[4] << 8 | GT911_Config[3]))) {
			// GT911_Config[0] = 82; // change version
			GT911_Config[1] = X_OUTPUT_MAX & 0xff;
			GT911_Config[2] = X_OUTPUT_MAX >> 8;
			GT911_Config[3] = Y_OUTPUT_MAX & 0xff;
			GT911_Config[4] = Y_OUTPUT_MAX >> 8;
			GT911_Config[185] = 1;

			printf("%d*%d\r\n", GT911_Config[2] << 8 | GT911_Config[1], GT911_Config[4] << 8 | GT911_Config[3]);

			config_Checksum = 0;
			for (i = 0; i < sizeof(GT911_Config) - 2; i++) {
				config_Checksum += GT911_Config[i];
			}
			GT911_Config[184] = (~config_Checksum) + 1;

			printf("config_Checksum=0x%2X\r\n", GT911_Config[184]);

			printf("\r\n*************************\r\n");
			for (i = 0; i < sizeof(GT911_Config); i++) {
				printf("0x%02X  ", GT911_Config[i]);
				if ((i + 1) % 10 == 0) {
					printf("\r\n");
				}
			}

			GT911_WR_Reg(GT911_CONFIG_REG, (uint8_t *)GT911_Config, sizeof(GT911_Config));
			printf("\r\n*Config writed*\r\n");
			printf("\r\n*************************\r\n");

			GT911_RD_Reg(GT911_CONFIG_REG, (uint8_t *)&GT911_Config[0], 186);

			config_Checksum = 0;
			for (i = 0; i < sizeof(GT911_Config) - 2; i++) {
				config_Checksum += GT911_Config[i];

				printf("0x%02X  ", GT911_Config[i]);
				if ((i + 1) % 10 == 0) {
					printf("\r\n");
				}
			}
			printf("0x%02X  ", GT911_Config[184]);
			printf("0x%02X  ", GT911_Config[185]);
			printf("\r\n");
			printf("config_Checksum=0x%2X\r\n", ((~config_Checksum) + 1) & 0xff);
			printf("%d*%d\r\n", GT911_Config[2] << 8 | GT911_Config[1], GT911_Config[4] << 8 | GT911_Config[3]);

			I2C_SHARED_BUS.locked = false;
			return true;
		}
		I2C_SHARED_BUS.locked = false;
		return true;
	}

	I2C_SHARED_BUS.locked = false;
	return false;
}

void GT911_Scan(void) {
	if (I2C_SHARED_BUS.locked) {
		return;
	}
	I2C_SHARED_BUS.locked = true;

	uint8_t buf[41] = {0};
	uint8_t Clearbuf = 0;

	if (GT911.Touch == 1) {
		GT911.Touch = 0;
		GT911_RD_Reg(GT911_READ_XY_REG, buf, 1);

		if ((buf[0] & 0x80) == 0x00) {
			GT911_WR_Reg(GT911_READ_XY_REG, (uint8_t *)&Clearbuf, 1);

			// sprintf(str,"%d",buf[0]);
			// sendToDebug_strln(str);
			// HAL_Delay(10);
		} else {
			// sprintf(str, "bufstat:%d",(buf[0]&0x80));
			// sendToDebug_strln(str);

			GT911.TouchpointFlag = buf[0];
			GT911.TouchCount = buf[0] & 0x0f;
			if (GT911.TouchCount > GT911_MAX_TOUCH || GT911.TouchCount == 0) {
				GT911_WR_Reg(GT911_READ_XY_REG, (uint8_t *)&Clearbuf, 1);
				I2C_SHARED_BUS.locked = false;
				return;
			}
			GT911_RD_Reg(GT911_READ_XY_REG + 1, &buf[1], GT911.TouchCount * 8);
			GT911_WR_Reg(GT911_READ_XY_REG, (uint8_t *)&Clearbuf, 1);

			GT911.Touchkeytrackid[0] = buf[1];
			GT911.X[0] = (uint16_t)((uint16_t)buf[3] << 8) + buf[2];
			GT911.Y[0] = (uint16_t)(480 - (((uint16_t)buf[5] << 8) + buf[4]));
			GT911.S[0] = (uint16_t)((uint16_t)buf[7] << 8) + buf[6];

			GT911.Touchkeytrackid[1] = buf[9];
			GT911.X[1] = (uint16_t)((uint16_t)buf[11] << 8) + buf[10];
			GT911.Y[1] = (uint16_t)(480 - (((uint16_t)buf[13] << 8) + buf[12]));
			GT911.S[1] = (uint16_t)((uint16_t)buf[15] << 8) + buf[14];

			GT911.Touchkeytrackid[2] = buf[17];
			GT911.X[2] = (uint16_t)((uint16_t)buf[19] << 8) + buf[18];
			GT911.Y[2] = (uint16_t)(480 - (((uint16_t)buf[21] << 8) + buf[20]));
			GT911.S[2] = (uint16_t)((uint16_t)buf[23] << 8) + buf[22];

			GT911.Touchkeytrackid[3] = buf[25];
			GT911.X[3] = (uint16_t)((uint16_t)buf[27] << 8) + buf[26];
			GT911.Y[3] = (uint16_t)(480 - (((uint16_t)buf[29] << 8) + buf[28]));
			GT911.S[3] = (uint16_t)((uint16_t)buf[31] << 8) + buf[30];

			GT911.Touchkeytrackid[4] = buf[33];
			GT911.X[4] = (uint16_t)((uint16_t)buf[35] << 8) + buf[34];
			GT911.Y[4] = (uint16_t)(480 - (((uint16_t)buf[37] << 8) + buf[36]));
			GT911.S[4] = (uint16_t)((uint16_t)buf[39] << 8) + buf[38];

			for (uint8_t touch_id = 0; touch_id < GT911.TouchCount; touch_id++) {
				GT911.Y[touch_id] = 480 - GT911.Y[touch_id];

				if (GT911.Y[touch_id] < 5) {
					GT911.Y[touch_id] = 5;
				}
				if (GT911.Y[touch_id] > 475) {
					GT911.Y[touch_id] = 475;
				}

				if (GT911.X[touch_id] < 5) {
					GT911.X[touch_id] = 5;
				}
				if (GT911.X[touch_id] > 795) {
					GT911.X[touch_id] = 795;
				}

				if (CALIBRATE.LCD_Rotate) {
					GT911.X[touch_id] = LCD_WIDTH - GT911.X[touch_id];
					GT911.Y[touch_id] = LCD_HEIGHT - GT911.Y[touch_id];
				}

				if (CALIBRATE.TOUCHPAD_horizontal_flip) {
					GT911.X[touch_id] = LCD_WIDTH - GT911.X[touch_id];
				}

				if (CALIBRATE.TOUCHPAD_vertical_flip) {
					GT911.Y[touch_id] = LCD_HEIGHT - GT911.Y[touch_id];
				}

				if (TRX.Debug_Type == TRX_DEBUG_TOUCH) {
					print("Touch X:", GT911.X[touch_id]);
					print(" Y:", GT911.Y[touch_id]);
					print_flush();
					print(" ID:", GT911.Touchkeytrackid[touch_id]);
					println(" COUNT:", GT911.TouchCount);
					print_flush();
				}
			}
			if (GT911.TouchCount == 1) {
				TOUCHPAD_processTouch(GT911.X[0], GT911.Y[0], 0, 0);
			}
			if (GT911.TouchCount == 2) {
				TOUCHPAD_processTouch(GT911.X[0], GT911.Y[0], GT911.X[1], GT911.Y[1]);
			}
		}
	}
	I2C_SHARED_BUS.locked = false;
}

#endif
