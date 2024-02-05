#include "settings.h"
#if defined(LCD_NONE)

// Header files
#include "fonts.h"
#include "functions.h"
#include "lcd_driver.h"
#include "lcd_driver_NONE.h"
#include "main.h"

//***** Functions prototypes *****//
static inline void LCDDriver_SetCursorPosition(uint16_t x, uint16_t y);
inline void LCDDriver_SetCursorAreaPosition(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

// Write command to LCD
inline void LCDDriver_SendCommand(uint16_t com) {}

// Write data to LCD
inline void LCDDriver_SendData(uint16_t data) {}

// Write data to LCD
inline void LCDDriver_SendData16(uint16_t data) {}

// Write pair command-data
inline void LCDDriver_writeReg(uint16_t reg, uint16_t val) {}

// Read command from LCD
inline uint16_t LCDDriver_ReadStatus(void) { return 0; }
// Read data from LCD
inline uint16_t LCDDriver_ReadData(void) { return 0; }

// Read Register
inline uint16_t LCDDriver_readReg(uint16_t reg) { return 0; }

// Initialise function
void LCDDriver_Init(void) {}

// Set screen rotation
void LCDDriver_setRotation(uint8_t rotate) {}

// Set cursor position
inline void LCDDriver_SetCursorAreaPosition(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {}

static inline void LCDDriver_SetCursorPosition(uint16_t x, uint16_t y) {}

// Write data to a single pixel
void LCDDriver_drawPixel(uint16_t x, uint16_t y, uint16_t color) {}

// Fill the entire screen with a background color
void LCDDriver_Fill(uint16_t color) {}

// Rectangle drawing functions
void LCDDriver_Fill_RectXY(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {}

void LCDDriver_Fill_RectWH(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {}

// Line drawing functions
void LCDDriver_drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {}

void LCDDriver_drawFastHLine(uint16_t x, uint16_t y, int16_t w, uint16_t color) {}

void LCDDriver_drawFastVLine(uint16_t x, uint16_t y, int16_t h, uint16_t color) {}

void LCDDriver_drawRectXY(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {}

void LCDDriver_Fill_Triangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color) {}

uint32_t LCDDriver_readScreenPixelsToBMP(uint8_t *buffer, uint32_t *current_index, uint32_t max_count, uint32_t paddingSize) { return 0; }

#endif
