#include "settings.h"
#if defined(LCD_ST7789)

// Header files
#include "fonts.h"
#include "functions.h"
#include "lcd_driver.h"
#include "lcd_driver_ST7789.h"
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

void ST7789_WriteData(uint8_t *buff, size_t buff_size) {
	if (hspi2.Init.DataSize != SPI_DATASIZE_8BIT) {
		hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
		HAL_SPI_Init(&hspi2);
	}
	/* SPI_CR1 reg, bit 11 DFF, 0: 8-bit / 1: 16-bit
	 * This bit should be written only when SPI is disabled (SPE = ‘0’) for correct operation.
	 * SPI_CR1, bit 6 SPE */
	CLEAR_BIT(hspi2.Instance->CR1, SPI_CR1_SPE);
	CLEAR_BIT(hspi2.Instance->CR1, SPI_DATASIZE_16BIT);
	SET_BIT(hspi2.Instance->CR1, SPI_DATASIZE_8BIT);
	SET_BIT(hspi2.Instance->CR1, SPI_CR1_SPE);

	ST7789_Select();
	ST7789_DC_Set();

	// split data in small chunks because HAL can't send more than 64K at once

	while (buff_size > 0) {
		uint16_t chunk_size = buff_size > 65535 ? 65535 : buff_size;
		HAL_SPI_Transmit(&ST7789_SPI_PORT, buff, chunk_size, HAL_MAX_DELAY);
		buff += chunk_size;
		buff_size -= chunk_size;
	}

	ST7789_UnSelect();
}

// Initialise function
void LCDDriver_Init(void) {

	HAL_Delay(150);
	//    ST7789_RST_Clr();
	//    HAL_Delay(25);
	//    ST7789_RST_Set();
	//    HAL_Delay(50);

	LCDDriver_SendCommand(ST7789_COLMOD); //	Set color mode
	LCDDriver_SendData8(ST7789_COLOR_MODE_16bit);
	LCDDriver_SendCommand(0xB2); //	Porch control
	{
		uint8_t data[] = {0x0C, 0x0C, 0x00, 0x33, 0x33};
		ST7789_WriteData(data, sizeof(data));
	}
	//	LCDDriver_setRotation(SCREEN_ROTATE);

	/* Internal LCD Voltage generator settings */
	LCDDriver_SendCommand(0XB7); //	Gate Control
	LCDDriver_SendData8(0x35);   //	Default value
	LCDDriver_SendCommand(0xBB); //	VCOM setting
	LCDDriver_SendData8(0x19);   //	0.725v (default 0.75v for 0x20)
	LCDDriver_SendCommand(0xC0); //	LCMCTRL
	LCDDriver_SendData8(0x2C);   //	Default value
	LCDDriver_SendCommand(0xC2); //	VDV and VRH command Enable
	LCDDriver_SendData8(0x01);   //	Default value
	LCDDriver_SendCommand(0xC3); //	VRH set
	LCDDriver_SendData8(0x12);   //	+-4.45v (defalut +-4.1v for 0x0B)
	LCDDriver_SendCommand(0xC4); //	VDV set
	LCDDriver_SendData8(0x20);   //	Default value
	LCDDriver_SendCommand(0xC6); //	Frame rate control in normal mode
	LCDDriver_SendData8(0x0F);   //	Default value (60HZ)
	LCDDriver_SendCommand(0xD0); //	Power control
	LCDDriver_SendData8(0xA4);   //	Default value
	LCDDriver_SendData8(0xA1);   //	Default value
	/**************** Division line ****************/

	LCDDriver_SendCommand(0xE0);
	{
		uint8_t data[] = {0xD0, 0x04, 0x0D, 0x11, 0x13, 0x2B, 0x3F, 0x54, 0x4C, 0x18, 0x0D, 0x0B, 0x1F, 0x23};
		ST7789_WriteData(data, sizeof(data));
	}

	LCDDriver_SendCommand(0xE1);
	{
		uint8_t data[] = {0xD0, 0x04, 0x0C, 0x11, 0x13, 0x2C, 0x3F, 0x44, 0x51, 0x2F, 0x1F, 0x1F, 0x20, 0x23};
		ST7789_WriteData(data, sizeof(data));
	}
	LCDDriver_SendCommand(ST7789_INVOFF); //	Inversion ON
	LCDDriver_SendCommand(ST7789_SLPOUT); //	Out of sleep mode
	LCDDriver_SendCommand(ST7789_NORON);  //	Normal Display on
	LCDDriver_SendCommand(ST7789_DISPON); //	Main screen turned on

	HAL_Delay(50);
}

// Set screen rotation
void LCDDriver_setRotation(uint8_t rotate) {
	LCDDriver_SendCommand(ST7789_MADCTL); // MADCTL
	switch (rotate) {
	case 0:
		LCDDriver_SendData8(ST7789_MADCTL_MX | ST7789_MADCTL_MY | ST7789_MADCTL_RGB);
		break;
	case 1:
		LCDDriver_SendData8(ST7789_MADCTL_MY | ST7789_MADCTL_MV | ST7789_MADCTL_RGB);
		break;
	case 2:
		LCDDriver_SendData8(ST7789_MADCTL_RGB);
		break;
	case 3:
		LCDDriver_SendData8(ST7789_MADCTL_MX | ST7789_MADCTL_MV | ST7789_MADCTL_RGB);
		break;
	default:
		break;
	}
}

// Set cursor position
inline void LCDDriver_SetCursorAreaPosition(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {

	ST7789_Select();

	//
	//	/* Column Address set */
	//	LCDDriver_SendCommand(ST7789_CASET);
	//	{
	//		uint8_t data[] = {x_start >> 8, x_start & 0xFF, x_end >> 8, x_end & 0xFF};
	//		ST7789_WriteData(data, sizeof(data));
	//	}

	//	/* Row Address set */
	//	LCDDriver_SendCommand(ST7789_RASET);
	//	{
	//		uint8_t data[] = {y_start >> 8, y_start & 0xFF, y_end >> 8, y_end & 0xFF};
	//		ST7789_WriteData(data, sizeof(data));
	//	}
	//	/* Write to RAM */
	//	LCDDriver_SendCommand(ST7789_RAMWR);

	if (x1 > LCD_WIDTH - 1) {
		x1 = LCD_WIDTH;
	}
	if (x2 > LCD_WIDTH - 1) {
		x2 = LCD_WIDTH;
	}

	if (y1 > LCD_HEIGHT - 1) {
		y1 = LCD_HEIGHT;
	}
	if (y2 > LCD_HEIGHT - 1) {
		y2 = LCD_HEIGHT;
	}

	LCDDriver_SendCommand(LCD_COMMAND_COLUMN_ADDR);
	LCDDriver_SendData16(x1);
	LCDDriver_SendData16(x2);
	LCDDriver_SendCommand(LCD_COMMAND_PAGE_ADDR);
	LCDDriver_SendData16(y1);
	LCDDriver_SendData16(y2);
	LCDDriver_SendCommand(LCD_COMMAND_GRAM);

	ST7789_UnSelect();
}

static inline void LCDDriver_SetCursorPosition(uint16_t x, uint16_t y) { LCDDriver_SetCursorAreaPosition(x, y, x, y); }

// Write data to a single pixel
void LCDDriver_drawPixel(uint16_t x, uint16_t y, uint16_t color) {
	if (x > LCD_WIDTH - 1) {
		x = LCD_WIDTH - 1;
	}
	if (x < 0) {
		x = 0;
	}

	if (y > LCD_HEIGHT - 1) {
		y = LCD_HEIGHT - 1;
	}
	if (y < 0) {
		y = 0;
	}

	ST7789_Select();

	LCDDriver_SetCursorPosition(x, y);
	LCDDriver_SendData16(color);

	ST7789_UnSelect();
}

// Fill the entire screen with a background color
void LCDDriver_Fill(uint16_t color) { LCDDriver_Fill_RectXY(0, 0, LCD_WIDTH, LCD_HEIGHT, color); }

// Rectangle drawing functions
void LCDDriver_Fill_RectXY(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {
	//		if ((x0 < 0) || (x1 >= LCD_WIDTH) ||
	//		 (y0 < 0) || (y1 >= LCD_HEIGHT))	return;
	//
	//	if (x1 > (LCD_WIDTH - 1)) //Set fill area
	//		x1 = LCD_WIDTH - 1;
	//	if (y1 > (LCD_HEIGHT - 1))
	//		y1 = LCD_HEIGHT - 1;

	if (x1 > LCD_WIDTH - 1) {
		x1 = LCD_WIDTH - 1;
	}
	if (x0 < 0) {
		x0 = 0;
	}

	if (y0 > LCD_HEIGHT - 1) {
		y0 = LCD_HEIGHT - 1;
	}
	if (y1 < 0) {
		y1 = 0;
	}

	// Set fill area
	LCDDriver_SetCursorAreaPosition(x0, y0, x1, y1);
	ST7789_Select();

	uint32_t n = ((x1 + 1) - x0) * ((y1 + 1) - y0);
	while (n--) {
		LCDDriver_SendData16(color);
	}

	ST7789_UnSelect();
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

	if (x2 < x) {
		LCDDriver_Fill_RectXY((uint16_t)x2, y, x, y, color);
	} else {
		LCDDriver_Fill_RectXY(x, y, (uint16_t)x2, y, color);
	}
}

void LCDDriver_drawFastVLine(uint16_t x, uint16_t y, int16_t h, uint16_t color) {
	int16_t y2 = y + h - 1;
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
