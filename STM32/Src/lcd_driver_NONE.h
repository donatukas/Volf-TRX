#ifndef _LCDDRIVER_NONE1_H_
#include "settings.h"
#if defined(LCD_NONE)
#define _LCDDRIVER_NONE_H_

#define LCD_FSMC_COMM_ADDR 0xC0000000 // remapped
#define LCD_FSMC_DATA_ADDR 0xC0080000

// LCD dimensions defines
#define LCD_WIDTH 480
#define LCD_HEIGHT 320
#define LCD_TYPE_FSMC true

#endif
#endif
