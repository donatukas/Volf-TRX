#ifndef _LCDDRIVER_ILI9341_H_
#include "settings.h"
#if defined(LCD_ILI9341)
#define _LCDDRIVER_ILI9341_H_

// LCD dimensions defines
#define LCD_WIDTH 320
#define LCD_HEIGHT 240
#define LCD_TYPE_SPI true
#define LCD_SMALL_INTERFACE true

// ILI9341 LCD commands
#define LCD_CONFIG_MADCTL_MY 0x80  ///< Bottom to top
#define LCD_CONFIG_MADCTL_MX 0x40  ///< Right to left
#define LCD_CONFIG_MADCTL_MV 0x20  ///< Reverse Mode
#define LCD_CONFIG_MADCTL_ML 0x10  ///< LCD refresh Bottom to top
#define LCD_CONFIG_MADCTL_RGB 0x00 ///< Red-Green-Blue pixel order
#define LCD_CONFIG_MADCTL_BGR 0x08 ///< Blue-Green-Red pixel order
#define LCD_CONFIG_MADCTL_MH 0x04  ///< LCD refresh right to left

#define LCD_COMMAND_NOP 0x00     ///< No-op register
#define LCD_COMMAND_SWRESET 0x01 ///< Software reset register
#define LCD_COMMAND_RDDID 0x04   ///< Read display identification information
#define LCD_COMMAND_RDDST 0x09   ///< Read Display Status

#define LCD_COMMAND_SLPIN 0x10  ///< Enter Sleep Mode
#define LCD_COMMAND_SLPOUT 0x11 ///< Sleep Out
#define LCD_COMMAND_PTLON 0x12  ///< Partial Mode ON
#define LCD_COMMAND_NORON 0x13  ///< Normal Display Mode ON

#define LCD_COMMAND_RDMODE 0x0A     ///< Read Display Power Mode
#define LCD_COMMAND_RDMADCTL 0x0B   ///< Read Display MADCTL
#define LCD_COMMAND_RDPIXFMT 0x0C   ///< Read Display Pixel Format
#define LCD_COMMAND_RDIMGFMT 0x0D   ///< Read Display Image Format
#define LCD_COMMAND_RDSELFDIAG 0x0F ///< Read Display Self-Diagnostic Result

#define LCD_COMMAND_INVOFF 0x20   ///< Display Inversion OFF
#define LCD_COMMAND_INVON 0x21    ///< Display Inversion ON
#define LCD_COMMAND_GAMMASET 0x26 ///< Gamma Set
#define LCD_COMMAND_DISPOFF 0x28  ///< Display OFF
#define LCD_COMMAND_DISPON 0x29   ///< Display ON

#define LCD_COMMAND_CASET 0x2A ///< Column Address Set
#define LCD_COMMAND_PASET 0x2B ///< Page Address Set
#define LCD_COMMAND_RAMWR 0x2C ///< Memory Write
#define LCD_COMMAND_RAMRD 0x2E ///< Memory Read

#define LCD_COMMAND_PTLAR 0x30    ///< Partial Area
#define LCD_COMMAND_VSCRDEF 0x33  ///< Vertical Scrolling Definition
#define LCD_COMMAND_MADCTL 0x36   ///< Memory Access Control
#define LCD_COMMAND_VSCRSADD 0x37 ///< Vertical Scrolling Start Address
#define LCD_COMMAND_PIXFMT 0x3A   ///< COLMOD: Pixel Format Set

#define LCD_COMMAND_FRMCTR1 0xB1 ///< Frame Rate Control (In Normal Mode/Full Colors)
#define LCD_COMMAND_FRMCTR2 0xB2 ///< Frame Rate Control (In Idle Mode/8 colors)
#define LCD_COMMAND_FRMCTR3 0xB3 ///< Frame Rate control (In Partial Mode/Full Colors)
#define LCD_COMMAND_INVCTR 0xB4  ///< Display Inversion Control
#define LCD_COMMAND_DFUNCTR 0xB6 ///< Display Function Control

#define LCD_COMMAND_PWCTR1 0xC0 ///< Power Control 1
#define LCD_COMMAND_PWCTR2 0xC1 ///< Power Control 2
#define LCD_COMMAND_PWCTR3 0xC2 ///< Power Control 3
#define LCD_COMMAND_PWCTR4 0xC3 ///< Power Control 4
#define LCD_COMMAND_PWCTR5 0xC4 ///< Power Control 5
#define LCD_COMMAND_VMCTR1 0xC5 ///< VCOM Control 1
#define LCD_COMMAND_VMCTR2 0xC7 ///< VCOM Control 2

#define LCD_COMMAND_RDID1 0xDA ///< Read ID 1
#define LCD_COMMAND_RDID2 0xDB ///< Read ID 2
#define LCD_COMMAND_RDID3 0xDC ///< Read ID 3
#define LCD_COMMAND_RDID4 0xDD ///< Read ID 4

#define LCD_COMMAND_GMCTRP1 0xE0 ///< Positive Gamma Correction
#define LCD_COMMAND_GMCTRN1 0xE1 ///< Negative Gamma Correction
#define LCD_COMMAND_PWCTR6 0xFC

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
