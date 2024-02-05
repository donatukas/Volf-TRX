#ifndef _LCDDRIVER_ST7735S_H_
#include "settings.h"
#if defined(LCD_ST7735S)
#define _LCDDRIVER_ST7735S_H_

// LCD dimensions defines
#define LCD_WIDTH 160
#define LCD_HEIGHT 128
#define LCD_TYPE_SPI true
#define LCD_SMALL_INTERFACE true

// ST7735S LCD commands
#define LCD_COMMAND_COLUMN_ADDR 0x2A
#define LCD_COMMAND_PAGE_ADDR 0x2B
#define LCD_COMMAND_GRAM 0x2C

#ifndef _swap_int16_t
#define _swap_int16_t(a, b) \
	{                         \
		int16_t t = a;          \
		a = b;                  \
		b = t;                  \
	}
#endif

#endif
#endif
