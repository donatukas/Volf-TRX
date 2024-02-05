#include "settings.h"
#if defined(LCD_ST7735S)

// Header files
#include "fonts.h"
#include "functions.h"
#include "lcd_driver.h"
#include "lcd_driver_ST7735S.h"
#include "main.h"

//***** Functions prototypes *****//
static inline void LCDDriver_SetCursorPosition(uint16_t x, uint16_t y);
inline void LCDDriver_SetCursorAreaPosition(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
static bool DC_mode = false;

// Write command to LCD
inline void LCDDriver_SendCommand(uint16_t com) {
	if (hspi4.Init.DataSize != SPI_DATASIZE_8BIT) {
		hspi4.Init.DataSize = SPI_DATASIZE_8BIT;
		HAL_SPI_Init(&hspi4);
	}

	// if(DC_mode)
	{
		LCD_DC_GPIO_Port->BSRR = (LCD_DC_Pin << 16U);
		// DC_mode = false;
	}

	/* Set the number of data at current transfer */
	MODIFY_REG(hspi4.Instance->CR2, SPI_CR2_TSIZE, 1);

	/* Enable SPI peripheral */
	__HAL_SPI_ENABLE(&hspi4);

	/* Master transfer start */
	SET_BIT(hspi4.Instance->CR1, SPI_CR1_CSTART);

	// Set data
	*((__IO uint8_t *)&hspi4.Instance->TXDR) = com;

	// Wait until TXP flag is set to send data
	while (!__HAL_SPI_GET_FLAG(&hspi4, SPI_FLAG_TXP)) {
	}

	// Wait for Tx (and CRC) data to be sent
	while (!__HAL_SPI_GET_FLAG(&hspi4, SPI_FLAG_EOT)) {
	}

	__HAL_SPI_CLEAR_EOTFLAG(&hspi4);
	__HAL_SPI_CLEAR_TXTFFLAG(&hspi4);

	__HAL_SPI_DISABLE(&hspi4);
}

// Write 8bit data to LCD
inline void LCDDriver_SendData8(uint8_t data) {
	if (hspi4.Init.DataSize != SPI_DATASIZE_8BIT) {
		hspi4.Init.DataSize = SPI_DATASIZE_8BIT;
		HAL_SPI_Init(&hspi4);
	}

	// if(!DC_mode)
	{
		LCD_DC_GPIO_Port->BSRR = LCD_DC_Pin;
		// DC_mode = true;
	}

	/* Set the number of data at current transfer */
	MODIFY_REG(hspi4.Instance->CR2, SPI_CR2_TSIZE, 1);

	/* Enable SPI peripheral */
	__HAL_SPI_ENABLE(&hspi4);

	/* Master transfer start */
	SET_BIT(hspi4.Instance->CR1, SPI_CR1_CSTART);

	// Set data
	*((__IO uint8_t *)&hspi4.Instance->TXDR) = data;

	// Wait until TXP flag is set to send data
	while (!__HAL_SPI_GET_FLAG(&hspi4, SPI_FLAG_TXP)) {
	}

	// Wait for Tx (and CRC) data to be sent
	while (!__HAL_SPI_GET_FLAG(&hspi4, SPI_FLAG_EOT)) {
	}

	__HAL_SPI_CLEAR_EOTFLAG(&hspi4);
	__HAL_SPI_CLEAR_TXTFFLAG(&hspi4);

	__HAL_SPI_DISABLE(&hspi4);
}

// Write 16 bit data to LCD
inline void LCDDriver_SendData16(uint16_t data) {
	if (hspi4.Init.DataSize != SPI_DATASIZE_8BIT) {
		hspi4.Init.DataSize = SPI_DATASIZE_8BIT;
		HAL_SPI_Init(&hspi4);
	}

	// if(!DC_mode)
	{
		LCD_DC_GPIO_Port->BSRR = LCD_DC_Pin;
		// DC_mode = true;
	}

	/* Set the number of data at current transfer */
	MODIFY_REG(hspi4.Instance->CR2, SPI_CR2_TSIZE, 2);

	/* Enable SPI peripheral */
	__HAL_SPI_ENABLE(&hspi4);

	/* Master transfer start */
	SET_BIT(hspi4.Instance->CR1, SPI_CR1_CSTART);

	// Set data
	*((__IO uint8_t *)&hspi4.Instance->TXDR) = data >> 8;

	// Wait until TXP flag is set to send data
	while (!__HAL_SPI_GET_FLAG(&hspi4, SPI_FLAG_TXP)) {
	}

	// Set data
	*((__IO uint8_t *)&hspi4.Instance->TXDR) = data;

	// Wait until TXP flag is set to send data
	while (!__HAL_SPI_GET_FLAG(&hspi4, SPI_FLAG_TXP)) {
	}

	// Wait for Tx (and CRC) data to be sent
	while (!__HAL_SPI_GET_FLAG(&hspi4, SPI_FLAG_EOT)) {
	}

	__HAL_SPI_CLEAR_EOTFLAG(&hspi4);
	__HAL_SPI_CLEAR_TXTFFLAG(&hspi4);

	__HAL_SPI_DISABLE(&hspi4);
}

// Initialise function
void LCDDriver_Init(void) {
	//------------------------------------ST7735R Reset Sequence-----------------------------------------//
	HAL_Delay(150);

	LCDDriver_SendCommand(0x01); // SWRESET Software reset
	HAL_Delay(150);
	//--------------------------------End ST7735S Reset Sequence ---------------------------------------//
	LCDDriver_SendCommand(0x11); // Sleep out
	HAL_Delay(150);              // Delay 120ms
	//------------------------------------ST7735S Frame rate-------------------------------------------------//
	LCDDriver_SendCommand(0xB1); // Frame rate 80Hz
	LCDDriver_SendData8(0x02);
	LCDDriver_SendData8(0x35);
	LCDDriver_SendData8(0x36);
	LCDDriver_SendCommand(0xB2); // Frame rate 80Hz
	LCDDriver_SendData8(0x02);
	LCDDriver_SendData8(0x35);
	LCDDriver_SendData8(0x36);
	LCDDriver_SendCommand(0xB3); // Frame rate 80Hz
	LCDDriver_SendData8(0x02);
	LCDDriver_SendData8(0x35);
	LCDDriver_SendData8(0x36);
	LCDDriver_SendData8(0x02);
	LCDDriver_SendData8(0x35);
	LCDDriver_SendData8(0x36);
	//------------------------------------End ST7735S Frame rate-------------------------------------------//
	LCDDriver_SendCommand(0xB4); // Dot inversion
	LCDDriver_SendData8(0x03);
	//------------------------------------ST7735S Power Sequence-----------------------------------------//
	LCDDriver_SendCommand(0xC0);
	LCDDriver_SendData8(0xA2);
	LCDDriver_SendData8(0x02);
	LCDDriver_SendData8(0x84);
	LCDDriver_SendCommand(0xC1);
	LCDDriver_SendData8(0xC5);
	LCDDriver_SendCommand(0xC2);
	LCDDriver_SendData8(0x0D);
	LCDDriver_SendData8(0x00);
	LCDDriver_SendCommand(0xC3);
	LCDDriver_SendData8(0x8D);
	LCDDriver_SendData8(0x2A);
	LCDDriver_SendCommand(0xC4);
	LCDDriver_SendData8(0x8D);
	LCDDriver_SendData8(0xEE);
	//---------------------------------End ST7735S Power Sequence---------------------------------------//
	LCDDriver_SendCommand(0xC5); // VCOM
	LCDDriver_SendData8(0x0a);
	LCDDriver_SendCommand(0x36); // MX, MY, RGB mode
	LCDDriver_SendData8(0xC8);
	//------------------------------------ST7735S Gamma Sequence-----------------------------------------//
	LCDDriver_SendCommand(0XE0);
	LCDDriver_SendData8(0x12);
	LCDDriver_SendData8(0x1C);
	LCDDriver_SendData8(0x10);
	LCDDriver_SendData8(0x18);
	LCDDriver_SendData8(0x33);
	LCDDriver_SendData8(0x2C);
	LCDDriver_SendData8(0x25);
	LCDDriver_SendData8(0x28);
	LCDDriver_SendData8(0x28);
	LCDDriver_SendData8(0x27);
	LCDDriver_SendData8(0x2F);
	LCDDriver_SendData8(0x3C);
	LCDDriver_SendData8(0x00);
	LCDDriver_SendData8(0x03);
	LCDDriver_SendData8(0x03);
	LCDDriver_SendData8(0x10);
	LCDDriver_SendCommand(0XE1);
	LCDDriver_SendData8(0x12);
	LCDDriver_SendData8(0x1C);
	LCDDriver_SendData8(0x10);
	LCDDriver_SendData8(0x18);
	LCDDriver_SendData8(0x2D);
	LCDDriver_SendData8(0x28);
	LCDDriver_SendData8(0x23);
	LCDDriver_SendData8(0x28);
	LCDDriver_SendData8(0x28);
	LCDDriver_SendData8(0x26);
	LCDDriver_SendData8(0x2F);
	LCDDriver_SendData8(0x3B);
	LCDDriver_SendData8(0x00);
	LCDDriver_SendData8(0x03);
	LCDDriver_SendData8(0x03);
	LCDDriver_SendData8(0x10);
	//------------------------------------End ST7735S Gamma Sequence-----------------------------------------//
	LCDDriver_SendCommand(0x3A); // 65k mode
	LCDDriver_SendData8(0x05);
	LCDDriver_SendCommand(0x29); // Display on
	HAL_Delay(150);
}

// Set screen rotation
void LCDDriver_setRotation(uint8_t rotate) {
	LCDDriver_SendCommand(0x36); /* Memory Data Access Control */

	switch (rotate) {
	case 1:
		LCDDriver_SendData8(0x60); /* MX + MV */
		break;

	case 2:
		LCDDriver_SendData8(0xC0); /* MY + MX */
		break;

	case 3:
		LCDDriver_SendData8(0xA0); /* MY + MV */
		break;

	default:
		LCDDriver_SendData8(0x00); /* None */
		break;
	}
}

// Set cursor position
inline void LCDDriver_SetCursorAreaPosition(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
	if (x1 > LCD_WIDTH - 1) {
		x1 > LCD_WIDTH - 1;
	}
	if (x2 > LCD_WIDTH - 1) {
		x2 > LCD_WIDTH - 1;
	}

	if (y1 > LCD_HEIGHT - 1) {
		y1 > LCD_HEIGHT - 1;
	}
	if (y2 > LCD_HEIGHT - 1) {
		y2 > LCD_HEIGHT - 1;
	}

	LCDDriver_SendCommand(LCD_COMMAND_COLUMN_ADDR);
	LCDDriver_SendData16(x1);
	LCDDriver_SendData16(x2);
	LCDDriver_SendCommand(LCD_COMMAND_PAGE_ADDR);
	LCDDriver_SendData16(y1);
	LCDDriver_SendData16(y2);
	LCDDriver_SendCommand(LCD_COMMAND_GRAM);
}

static inline void LCDDriver_SetCursorPosition(uint16_t x, uint16_t y) { LCDDriver_SetCursorAreaPosition(x, y, x, y); }

// Write data to a single pixel
void LCDDriver_drawPixel(uint16_t x, uint16_t y, uint16_t color) {
	LCDDriver_SetCursorPosition(x, y);
	LCDDriver_SendData16(color);
}

// Fill the entire screen with a background color
void LCDDriver_Fill(uint16_t color) { LCDDriver_Fill_RectXY(0, 0, LCD_WIDTH, LCD_HEIGHT, color); }

// Rectangle drawing functions
void LCDDriver_Fill_RectXY(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {
	if (x1 > (LCD_WIDTH - 1)) { // Set fill area
		x1 = LCD_WIDTH - 1;
	}
	if (y1 > (LCD_HEIGHT - 1)) {
		y1 = LCD_HEIGHT - 1;
	}

	// Set fill area
	LCDDriver_SetCursorAreaPosition(x0, y0, x1, y1);

	uint32_t n = ((x1 + 1) - x0) * ((y1 + 1) - y0);
	while (n--) {
		LCDDriver_SendData16(color);
	}
}

void LCDDriver_Fill_RectWH(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) { LCDDriver_Fill_RectXY(x, y, x + w, y + h, color); }

// Line drawing functions
void LCDDriver_drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {
	int16_t steep = abs(y1 - y0) > abs(x1 - x0);
	if (steep) {
		uswap(x0, y0) uswap(x1, y1)
	}

	if (x0 > x1) {
		uswap(x0, x1) uswap(y0, y1)
	}

	int16_t dx, dy;
	dx = (int16_t)(x1 - x0);
	dy = (int16_t)abs(y1 - y0);

	int16_t err = dx / 2;
	int16_t ystep;

	if (y0 < y1) {
		ystep = 1;
	} else {
		ystep = -1;
	}

	for (; x0 <= x1; x0++) {
		if (steep) {
			LCDDriver_drawPixel(y0, x0, color);
		} else {
			LCDDriver_drawPixel(x0, y0, color);
		}
		err -= dy;
		if (err < 0) {
			y0 += ystep;
			err += dx;
		}
	}
}

void LCDDriver_drawFastHLine(uint16_t x, uint16_t y, int16_t w, uint16_t color) {
	int16_t x2 = x + w;
	if (x2 < 0) {
		x2 = 0;
	}
	if (x2 > (LCD_WIDTH - 1)) {
		x2 = LCD_WIDTH - 1;
	}
	if (y < 0) {
		y = 0;
	}
	if (y > (LCD_HEIGHT - 1)) {
		y = LCD_HEIGHT - 1;
	}

	if (x2 < x) {
		LCDDriver_Fill_RectXY((uint16_t)x2, y, x, y, color);
	} else {
		LCDDriver_Fill_RectXY(x, y, (uint16_t)x2, y, color);
	}
}

void LCDDriver_drawFastVLine(uint16_t x, uint16_t y, int16_t h, uint16_t color) {
	int16_t y2 = y + h - 1;
	if (x < 0) {
		x = 0;
	}
	if (x > (LCD_WIDTH - 1)) {
		x = LCD_WIDTH - 1;
	}
	if (y2 < 0) {
		y2 = 0;
	}
	if (y2 > (LCD_HEIGHT - 1)) {
		y2 = LCD_HEIGHT - 1;
	}

	if (y2 < y) {
		LCDDriver_Fill_RectXY(x, (uint16_t)y2, x, y, color);
	} else {
		LCDDriver_Fill_RectXY(x, y, x, (uint16_t)y2, color);
	}
}

void LCDDriver_drawRectXY(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {
	LCDDriver_drawFastHLine(x0, y0, (int16_t)(x1 - x0), color);
	LCDDriver_drawFastHLine(x0, y1, (int16_t)(x1 - x0), color);
	LCDDriver_drawFastVLine(x0, y0, (int16_t)(y1 - y0), color);
	LCDDriver_drawFastVLine(x1, y0, (int16_t)(y1 - y0), color);
}

void LCDDriver_Fill_Triangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color) {
	int16_t a, b, y, last;

	// Sort coordinates by Y order (y2 >= y1 >= y0)
	if (y0 > y1) {
		_swap_int16_t(y0, y1) _swap_int16_t(x0, x1)
	}
	if (y1 > y2) {
		_swap_int16_t(y2, y1) _swap_int16_t(x2, x1)
	}
	if (y0 > y1) {
		_swap_int16_t(y0, y1) _swap_int16_t(x0, x1)
	}

	if (y0 == y2) { // Handle awkward all-on-same-line case as its own thing
		a = b = x0;
		if (x1 < a) {
			a = x1;
		} else if (x1 > b) {
			b = x1;
		}
		if (x2 < a) {
			a = x2;
		} else if (x2 > b) {
			b = x2;
		}
		LCDDriver_drawFastHLine(a, y0, b - a + 1, color);
		return;
	}

	int16_t dx01 = x1 - x0, dy01 = y1 - y0, dx02 = x2 - x0, dy02 = y2 - y0, dx12 = x2 - x1, dy12 = y2 - y1;
	int32_t sa = 0, sb = 0;

	// For upper part of triangle, find scanline crossings for segments
	// 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
	// is included here (and second loop will be skipped, avoiding a /0
	// error there), otherwise scanline y1 is skipped here and handled
	// in the second loop...which also avoids a /0 error here if y0=y1
	// (flat-topped triangle).
	if (y1 == y2) {
		last = y1; // Include y1 scanline
	} else {
		last = y1 - 1; // Skip it
	}

	for (y = y0; y <= last; y++) {
		a = x0 + sa / dy01;
		b = x0 + sb / dy02;
		sa += dx01;
		sb += dx02;
		/* longhand:
		a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
		b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
		*/
		if (a > b) {
			_swap_int16_t(a, b) LCDDriver_drawFastHLine(a, y, b - a + 1, color);
		}
	}

	// For lower part of triangle, find scanline crossings for segments
	// 0-2 and 1-2.  This loop is skipped if y1=y2.
	sa = (int32_t)dx12 * (y - y1);
	sb = (int32_t)dx02 * (y - y0);
	for (; y <= y2; y++) {
		a = x1 + sa / dy12;
		b = x0 + sb / dy02;
		sa += dx12;
		sb += dx02;
		/* longhand:
		a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
		b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
		*/
		if (a > b) {
			_swap_int16_t(a, b) LCDDriver_drawFastHLine(a, y, b - a + 1, color);
		}
	}
}

uint32_t LCDDriver_readScreenPixelsToBMP(uint8_t *buffer, uint32_t *current_index, uint32_t max_count, uint32_t paddingSize) { return 0; }

#endif
