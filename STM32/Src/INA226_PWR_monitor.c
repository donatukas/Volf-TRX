#include "INA226_PWR_monitor.h"
#include "i2c.h"
#include "lcd.h"

static float32_t Voltage = 0;
static float32_t Current = 0;

uint16_t INA226_Read2Byte(uint8_t reg_addr) {
#ifdef HAS_TOUCHPAD
	uint16_t reg_data = 0;

	i2c_beginTransmission_u8(&I2C_SHARED_BUS, INA226_ADDR);
	i2c_write_u8(&I2C_SHARED_BUS, reg_addr);
	uint8_t res = i2c_endTransmission(&I2C_SHARED_BUS);

	if (res == 0) {
		if (i2c_beginReceive_u8(&I2C_SHARED_BUS, INA226_ADDR)) {
			reg_data = i2c_Read_HalfWord(&I2C_SHARED_BUS);
			i2c_stop(&I2C_SHARED_BUS);
		}
	} else if (TRX.Debug_Type == TRX_DEBUG_I2C) {
		println("I2C INA226 error: ", res);
	}

	return reg_data;
#else
	return 0;
#endif
}

uint8_t INA226_Write2Byte(uint8_t reg_addr, uint16_t reg_data) {
#ifdef HAS_TOUCHPAD
	uint8_t data_high = (uint8_t)((reg_data & 0xFF00) >> 8);
	uint8_t data_low = (uint8_t)reg_data & 0x00FF;

	i2c_beginTransmission_u8(&I2C_SHARED_BUS, INA226_ADDR);
	i2c_write_u8(&I2C_SHARED_BUS, reg_addr);
	i2c_write_u8(&I2C_SHARED_BUS, data_high);
	i2c_write_u8(&I2C_SHARED_BUS, data_low);

	uint8_t res = i2c_endTransmission(&I2C_SHARED_BUS);

	return res;
#else
	return 0;
#endif
}

void INA226_Init(void) {
#ifdef HAS_TOUCHPAD
	if (I2C_SHARED_BUS.locked) {
		return;
	}
	I2C_SHARED_BUS.locked = true;

	// 16 times average, 1.1ms, 1.1ms, continuous measurement of shunt voltage and bus voltage
	// INA226_Write2Byte(Config_Reg, 0x4527);//0100_010_100_100_111

	// 0100_101_011_010_111 //256 times average, 588ms, 332ms, continuous measurement of shunt voltage and bus voltage
	INA226_Write2Byte(Config_Reg, 0x4AD7);
	// New VBUS data is ready every 150,5ms
	// New VSHC (current) data is ready every 85ms

	// 0100_111_100_100_111 //1024 times average, 1.1ms, 1.1ms, continuous measurement of shunt voltage and bus voltage
	// INA226_Write2Byte(Config_Reg, 0x4F27);

	// 0100_001_100_100_111 //1 times average, 1.1ms, 1.1ms, continuous measurement of shunt voltage and bus voltage
	// INA226_Write2Byte(Config_Reg, 0x4127);
	float64_t INA226_Shunt = (float64_t)CALIBRATE.INA226_Shunt_mOhm / 1000.0;
	float64_t INA226_MaximumExpectedCurrent = 30.0 * 1000000.0; // 30 Amp
	float64_t INA226_CurrentLSB = INA226_MaximumExpectedCurrent / pow(2, 15);
	float64_t INA226_Calibration = 0.00512 / ((INA226_CurrentLSB / 1000.0 / 1000.0) * INA226_Shunt);

	// Write the calibration bytes (used for the current calculation)
	INA226_Write2Byte(Calib_Reg, (uint16_t)INA226_Calibration);

	I2C_SHARED_BUS.locked = false;
#endif
}

// Read the INA226 Voltage and Current data
void Read_INA226_Data(void) {
#ifdef HAS_TOUCHPAD
	if (I2C_SHARED_BUS.locked) {
		return;
	}
	I2C_SHARED_BUS.locked = true;

	const uint8_t INA226_Read_Tm = 6;
	static uint8_t Rd_Count;

	Rd_Count++;

	// not ot overload the I2C bus, take the data oproximately every 210ms
	if (Rd_Count > INA226_Read_Tm) {
		Rd_Count = 0;

		Voltage = INA226_Read2Byte(Bus_V_Reg) * 0.00125f + CALIBRATE.INA226_VoltageOffset;
		Current = INA226_Read2Byte(Current_Reg) * 0.001f; // multiply the Current register value with the calibration coefficient (mA/Bit)
	}

	I2C_SHARED_BUS.locked = false;
#endif
}

// Return the INA226 Bus Voltage
float32_t Get_INA226_Voltage(void) { return Voltage; }

// Return the INA226 Schunt Current
float32_t Get_INA226_Current(void) { return Current; }
