#ifndef __INA226_PWR_MONITOR_H
#define __INA226_PWR_MONITOR_H
#include "stdint.h"

#define INA226_ADDR 0x40 // A1=GND，A2=GND

#define Config_Reg 0x00
#define Shunt_V_Reg 0x01
#define Bus_V_Reg 0x02
#define Power_Reg 0x03
#define Current_Reg 0x04
#define Calib_Reg 0x05
#define Mask_En_Reg 0x06
#define Alert_Reg 0x07
#define Man_ID_Reg 0xFE // 0x5449
#define ID_Reg 0xFF     // 0x2260

uint16_t INA226_Read2Byte(uint8_t reg_addr);
uint8_t INA226_Write2Byte(uint8_t reg_addr, uint16_t reg_data);
void INA226_Init(void);
void Read_INA226_Data(void);
float Get_INA226_Voltage(void);
float Get_INA226_Current(void);

#endif