#ifndef TOUCHPAD_GT911_h
#define TOUCHPAD_GT911_h

#include "settings.h"
#if (defined(TOUCHPAD_GT911))
#define HAS_TOUCHPAD true

#define GT911_I2C_ADDR_1 93 // 20/93 is shifted >> 1 touch addr
#define GT911_I2C_ADDR_2 20
#define GT911_MAX_TOUCH 5
#define GT911_COMMAND_REG 0x8040
#define GT911_CONFIG_REG 0x8047
#define GT911_PRODUCT_ID_REG 0x8140
#define GT911_FIRMWARE_VERSION_REG 0x8144
#define GT911_READ_XY_REG 0x814E

/* Commands for GT911_COMMAND_REG */
// 0: read coordinate state
#define GOODIX_CMD_READ 0x00
// 1: difference value original value
#define GOODIX_CMD_DIFFVAL 0x01
// 2: software reset
#define GOODIX_CMD_SOFTRESET 0x02
// 3: Baseline update
#define GOODIX_CMD_BASEUPDATE 0x03
// 4: Benchmark calibration
#define GOODIX_CMD_CALIBRATE 0x04
// 5: Off screen (send other invalid)
#define GOODIX_CMD_SCREEN_OFF 0x05

typedef struct {
	uint8_t Touch;
	uint8_t TouchpointFlag;
	uint8_t TouchCount;

	uint8_t Touchkeytrackid[GT911_MAX_TOUCH];
	uint16_t X[GT911_MAX_TOUCH];
	uint16_t Y[GT911_MAX_TOUCH];
	uint16_t S[GT911_MAX_TOUCH];
} GT911_Dev;

extern GT911_Dev GT911;

extern void GT911_RD_Reg(uint16_t reg, uint8_t *buf, uint8_t len);
extern uint8_t GT911_WR_Reg(uint16_t reg, uint8_t *buf, uint8_t len);
extern void GT911_ReadStatus(void);
extern void GT911_ReadFirmwareVersion(void);
extern void GT911_Scan(void);
extern void GT911_Init(void);
extern bool GT911_Flash(void);

#endif
#endif
