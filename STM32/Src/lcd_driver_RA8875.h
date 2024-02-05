#ifndef LCDDRIVER_RA8875_H_
#include "settings.h"
#if (defined(LCD_RA8875))
#define LCDDRIVER_RA8875_H_

#define LCD_FSMC_COMM_ADDR 0xC0080000 // remapped
#define LCD_FSMC_DATA_ADDR 0xC0000000

#define LCD_WIDTH 800
#define LCD_HEIGHT 480
// #define LCD_WIDTH 480
// #define LCD_HEIGHT 320
#define LCD_TYPE_FSMC true
#define HAS_BTE true
#define HAS_BRIGHTNESS_CONTROL true

// Registers & bits
#define LCD_RA8875_PWRR 0x01
#define LCD_RA8875_PWRR_DISPON 0x80
#define LCD_RA8875_PWRR_DISPOFF 0x00
#define LCD_RA8875_PWRR_SLEEP 0x02
#define LCD_RA8875_PWRR_NORMAL 0x00
#define LCD_RA8875_PWRR_SOFTRESET 0x01

#define LCD_RA8875_MRWC 0x02

#define LCD_RA8875_GPIOX 0xC7

#define LCD_RA8875_PLLC1 0x88
#define LCD_RA8875_PLLC1_PLLDIV2 0x80
#define LCD_RA8875_PLLC1_PLLDIV1 0x00

#define LCD_RA8875_PLLC2 0x89
#define LCD_RA8875_PLLC2_DIV1 0x00
#define LCD_RA8875_PLLC2_DIV2 0x01
#define LCD_RA8875_PLLC2_DIV4 0x02
#define LCD_RA8875_PLLC2_DIV8 0x03
#define LCD_RA8875_PLLC2_DIV16 0x04
#define LCD_RA8875_PLLC2_DIV32 0x05
#define LCD_RA8875_PLLC2_DIV64 0x06
#define LCD_RA8875_PLLC2_DIV128 0x07

#define LCD_RA8875_SYSR 0x10
#define LCD_RA8875_SYSR_8BPP 0x00
#define LCD_RA8875_SYSR_16BPP 0x0C
#define LCD_RA8875_SYSR_MCU8 0x00
#define LCD_RA8875_SYSR_MCU16 0x03

#define LCD_RA8875_PCSR 0x04
#define LCD_RA8875_PCSR_PDATR 0x00
#define LCD_RA8875_PCSR_PDATL 0x80
#define LCD_RA8875_PCSR_CLK 0x00
#define LCD_RA8875_PCSR_2CLK 0x01
#define LCD_RA8875_PCSR_4CLK 0x02
#define LCD_RA8875_PCSR_8CLK 0x03

#define LCD_RA8875_HDWR 0x14

#define LCD_RA8875_HNDFTR 0x15
#define LCD_RA8875_HNDFTR_DE_HIGH 0x00
#define LCD_RA8875_HNDFTR_DE_LOW 0x80

#define LCD_RA8875_HNDR 0x16
#define LCD_RA8875_HSTR 0x17
#define LCD_RA8875_HPWR 0x18
#define LCD_RA8875_HPWR_LOW 0x00
#define LCD_RA8875_HPWR_HIGH 0x80

#define LCD_RA8875_VDHR0 0x19
#define LCD_RA8875_VDHR1 0x1A
#define LCD_RA8875_VNDR0 0x1B
#define LCD_RA8875_VNDR1 0x1C
#define LCD_RA8875_VSTR0 0x1D
#define LCD_RA8875_VSTR1 0x1E
#define LCD_RA8875_VPWR 0x1F
#define LCD_RA8875_VPWR_LOW 0x00
#define LCD_RA8875_VPWR_HIGH 0x80

#define LCD_RA8875_HSAW0 0x30
#define LCD_RA8875_HSAW1 0x31
#define LCD_RA8875_VSAW0 0x32
#define LCD_RA8875_VSAW1 0x33

#define LCD_RA8875_HEAW0 0x34
#define LCD_RA8875_HEAW1 0x35
#define LCD_RA8875_VEAW0 0x36
#define LCD_RA8875_VEAW1 0x37

#define LCD_RA8875_MCLR 0x8E
#define LCD_RA8875_MCLR_START 0x80
#define LCD_RA8875_MCLR_STOP 0x00
#define LCD_RA8875_MCLR_READSTATUS 0x80
#define LCD_RA8875_MCLR_FULL 0x00
#define LCD_RA8875_MCLR_ACTIVE 0x40

#define LCD_RA8875_DCR_LINESQUTRI_START 0x80
#define LCD_RA8875_DCR_LINESQUTRI_STOP 0x00
#define LCD_RA8875_DCR_LINESQUTRI_STATUS 0x80
#define LCD_RA8875_DCR_CIRCLE_START 0x40
#define LCD_RA8875_DCR_CIRCLE_STATUS 0x40
#define LCD_RA8875_DCR_CIRCLE_STOP 0x00
#define LCD_RA8875_DCR_FILL 0x20
#define LCD_RA8875_DCR_NOFILL 0x00
#define LCD_RA8875_DCR_DRAWLINE 0x00
#define LCD_RA8875_DCR_DRAWTRIANGLE 0x01
#define LCD_RA8875_DCR_DRAWSQUARE 0x10

#define LCD_RA8875_ELLIPSE 0xA0
#define LCD_RA8875_ELLIPSE_STATUS 0x80

#define LCD_RA8875_MWCR0 0x40
#define LCD_RA8875_MWCR0_GFXMODE 0x00
#define LCD_RA8875_MWCR0_TXTMODE 0x80
#define LCD_RA8875_MWCR0_CURSOR 0x40
#define LCD_RA8875_MWCR0_BLINK 0x20

#define LCD_RA8875_MWCR0_DIRMASK 0x0C ///< Bitmask for Write Direction
#define LCD_RA8875_MWCR0_LRTD 0x00    ///< Left->Right then Top->Down
#define LCD_RA8875_MWCR0_RLTD 0x04    ///< Right->Left then Top->Down
#define LCD_RA8875_MWCR0_TDLR 0x08    ///< Top->Down then Left->Right
#define LCD_RA8875_MWCR0_DTLR 0x0C    ///< Down->Top then Left->Right

#define LCD_RA8875_BTCR 0x44
#define LCD_RA8875_CURH0 0x46
#define LCD_RA8875_CURH1 0x47
#define LCD_RA8875_CURV0 0x48
#define LCD_RA8875_CURV1 0x49

#define LCD_RA8875_P1CR 0x8A
#define LCD_RA8875_P1CR_ENABLE 0x80
#define LCD_RA8875_P1CR_DISABLE 0x00
#define LCD_RA8875_P1CR_CLKOUT 0x10
#define LCD_RA8875_P1CR_PWMOUT 0x00

#define LCD_RA8875_P1DCR 0x8B

#define LCD_RA8875_P2CR 0x8C
#define LCD_RA8875_P2CR_ENABLE 0x80
#define LCD_RA8875_P2CR_DISABLE 0x00
#define LCD_RA8875_P2CR_CLKOUT 0x10
#define LCD_RA8875_P2CR_PWMOUT 0x00

#define LCD_RA8875_P2DCR 0x8D

#define LCD_RA8875_PWM_CLK_DIV1 0x00
#define LCD_RA8875_PWM_CLK_DIV2 0x01
#define LCD_RA8875_PWM_CLK_DIV4 0x02
#define LCD_RA8875_PWM_CLK_DIV8 0x03
#define LCD_RA8875_PWM_CLK_DIV16 0x04
#define LCD_RA8875_PWM_CLK_DIV32 0x05
#define LCD_RA8875_PWM_CLK_DIV64 0x06
#define LCD_RA8875_PWM_CLK_DIV128 0x07
#define LCD_RA8875_PWM_CLK_DIV256 0x08
#define LCD_RA8875_PWM_CLK_DIV512 0x09
#define LCD_RA8875_PWM_CLK_DIV1024 0x0A
#define LCD_RA8875_PWM_CLK_DIV2048 0x0B
#define LCD_RA8875_PWM_CLK_DIV4096 0x0C
#define LCD_RA8875_PWM_CLK_DIV8192 0x0D
#define LCD_RA8875_PWM_CLK_DIV16384 0x0E
#define LCD_RA8875_PWM_CLK_DIV32768 0x0F

#define LCD_RA8875_TPCR0 0x70
#define LCD_RA8875_TPCR0_ENABLE 0x80
#define LCD_RA8875_TPCR0_DISABLE 0x00
#define LCD_RA8875_TPCR0_WAIT_512CLK 0x00
#define LCD_RA8875_TPCR0_WAIT_1024CLK 0x10
#define LCD_RA8875_TPCR0_WAIT_2048CLK 0x20
#define LCD_RA8875_TPCR0_WAIT_4096CLK 0x30
#define LCD_RA8875_TPCR0_WAIT_8192CLK 0x40
#define LCD_RA8875_TPCR0_WAIT_16384CLK 0x50
#define LCD_RA8875_TPCR0_WAIT_32768CLK 0x60
#define LCD_RA8875_TPCR0_WAIT_65536CLK 0x70
#define LCD_RA8875_TPCR0_WAKEENABLE 0x08
#define LCD_RA8875_TPCR0_WAKEDISABLE 0x00
#define LCD_RA8875_TPCR0_ADCCLK_DIV1 0x00
#define LCD_RA8875_TPCR0_ADCCLK_DIV2 0x01
#define LCD_RA8875_TPCR0_ADCCLK_DIV4 0x02
#define LCD_RA8875_TPCR0_ADCCLK_DIV8 0x03
#define LCD_RA8875_TPCR0_ADCCLK_DIV16 0x04
#define LCD_RA8875_TPCR0_ADCCLK_DIV32 0x05
#define LCD_RA8875_TPCR0_ADCCLK_DIV64 0x06
#define LCD_RA8875_TPCR0_ADCCLK_DIV128 0x07

#define LCD_RA8875_TPCR1 0x71
#define LCD_RA8875_TPCR1_AUTO 0x00
#define LCD_RA8875_TPCR1_MANUAL 0x40
#define LCD_RA8875_TPCR1_VREFINT 0x00
#define LCD_RA8875_TPCR1_VREFEXT 0x20
#define LCD_RA8875_TPCR1_DEBOUNCE 0x04
#define LCD_RA8875_TPCR1_NODEBOUNCE 0x00
#define LCD_RA8875_TPCR1_IDLE 0x00
#define LCD_RA8875_TPCR1_WAIT 0x01
#define LCD_RA8875_TPCR1_LATCHX 0x02
#define LCD_RA8875_TPCR1_LATCHY 0x03

#define LCD_RA8875_DPCR 0x20
#define LCD_RA8875_DPCR_HDIR 0x08
#define LCD_RA8875_DPCR_VDIR 0x04

#define LCD_RA8875_TPXH 0x72
#define LCD_RA8875_TPYH 0x73
#define LCD_RA8875_TPXYL 0x74

#define LCD_RA8875_INTC1 0xF0
#define LCD_RA8875_INTC1_KEY 0x10
#define LCD_RA8875_INTC1_DMA 0x08
#define LCD_RA8875_INTC1_TP 0x04
#define LCD_RA8875_INTC1_BTE 0x02

#define LCD_RA8875_INTC2 0xF1
#define LCD_RA8875_INTC2_KEY 0x10
#define LCD_RA8875_INTC2_DMA 0x08
#define LCD_RA8875_INTC2_TP 0x04
#define LCD_RA8875_INTC2_BTE 0x02

#define LCD_RA8875_SCROLL_BOTH 0x00
#define LCD_RA8875_SCROLL_LAYER1 0x40
#define LCD_RA8875_SCROLL_LAYER2 0x80
#define LCD_RA8875_SCROLL_BUFFER 0xC0

#define LCD_RA8875_BGCR0 (0x60) /* Background Color Register 0 */
#define LCD_RA8875_BGCR1 (0x61) /* Background Color Register 1 */
#define LCD_RA8875_BGCR2 (0x62) /* Background Color Register 2 */
#define LCD_RA8875_FGCR0 (0x63) /* Foreground Color Register 0 */
#define LCD_RA8875_FGCR1 (0x64) /* Foreground Color Register 1 */
#define LCD_RA8875_FGCR2 (0x65) /* Foreground Color Register 2 */
#define LCD_RA8875_PTNO (0x66)  /* Pattern Set No for BTE */
#define LCD_RA8875_BGTR0 (0x67) /* Background Color Register for Transparent 0 */
#define LCD_RA8875_BGTR1 (0x68) /* Background Color Register for Transparent 1 */
#define LCD_RA8875_BGTR2 (0x69) /* Background Color Register for Transparent 2 */

/* Drawing Control Registers */
#define LCD_RA8875_DCR (0x90)    /* Draw Line/Circle/Square Control Register */
#define LCD_RA8875_DLHSR0 (0x91) /* Draw Line/Square Horizontal Start Address Register0 */
#define LCD_RA8875_DLHSR1 (0x92) /* Draw Line/Square Horizontal Start Address Register1 */
#define LCD_RA8875_DLVSR0 (0x93) /* Draw Line/Square Vertical Start Address Register0 */
#define LCD_RA8875_DLVSR1 (0x94) /* Draw Line/Square Vertical Start Address Register1 */
#define LCD_RA8875_DLHER0 (0x95) /* Draw Line/Square Horizontal End Address Register0 */
#define LCD_RA8875_DLHER1 (0x96) /* Draw Line/Square Horizontal End Address Register1 */
#define LCD_RA8875_DLVER0 (0x97) /* Draw Line/Square Vertical End Address Register0 */
#define LCD_RA8875_DLVER1 (0x98) /* Draw Line/Square Vertical End Address Register1 */

#define LCD_RA8875_DCHR0 (0x99) /* Draw Circle Center Horizontal Address Register0 */
#define LCD_RA8875_DCHR1 (0x9A) /* Draw Circle Center Horizontal Address Register1 */
#define LCD_RA8875_DCVR0 (0x9B) /* Draw Circle Center Vertical Address Register0 */
#define LCD_RA8875_DCVR1 (0x9C) /* Draw Circle Center Vertical Address Register1 */
#define LCD_RA8875_DCRR (0x9D)  /* Draw Circle Radius Register */

#define LCD_RA8875_ELLCR (0xA0)  /* Draw Ellipse/Ellipse Curve/Circle Square Control Register */
#define LCD_RA8875_ELL_A0 (0xA1) /* Draw Ellipse/Circle Square Long axis Setting Register0 */
#define LCD_RA8875_ELL_A1 (0xA2) /* Draw Ellipse/Circle Square Long axis Setting Register1 */
#define LCD_RA8875_ELL_B0 (0xA3) /* Draw Ellipse/Circle Square Short axis Setting Register0 */
#define LCD_RA8875_ELL_B1 (0xA4) /* Draw Ellipse/Circle Square Short axis Setting Register1 */
#define LCD_RA8875_DEHR0 (0xA5)  /* Draw Ellipse/Circle Square Center Horizontal Address Register0 */
#define LCD_RA8875_DEHR1 (0xA6)  /* Draw Ellipse/Circle Square Center Horizontal Address Register1 */
#define LCD_RA8875_DEVR0 (0xA7)  /* Draw Ellipse/Circle Square Center Vertical Address Register0 */
#define LCD_RA8875_DEVR1 (0xA8)  /* Draw Ellipse/Circle Square Center Vertical Address Register1 */

#define LCD_RA8875_DTPH0 (0xA9) /* Draw Triangle Point 2 Horizontal Address Register0 */
#define LCD_RA8875_DTPH1 (0xAA) /* Draw Triangle Point 2 Horizontal Address Register1 */
#define LCD_RA8875_DTPV0 (0xAB) /* Draw Triangle Point 2 Vertical Address Register0 */
#define LCD_RA8875_DTPV1 (0xAC) /* Draw Triangle Point 2 Vertical Address Register1 */

#define LCD_RA8875_BTE_BECR0 0x50 // BTE Function Control Register 0
#define LCD_RA8875_BTE_BECR0_BTEON 0x80
#define LCD_RA8875_BTE_BECR1 0x51     // BTE Function Control Register 1
#define LCD_RA8875_BTE_BECR1_MVP 0x02 // Move BTE in positive direction with ROP.
#define LCD_RA8875_BTE_BECR1_MVN 0x03 // Move BTE in negative direction with ROP.
#define LCD_RA8875_BTE_BECR1_DS 0xC0  // Setting register Destination = source
#define LCD_RA8875_BTE_HSBE0 0x54     // Horizontal Source Point 0 of BTE
#define LCD_RA8875_BTE_HSBE1 0x55     // Horizontal Source Point 1 of BTE
#define LCD_RA8875_BTE_VSBE0 0x56     // Vertical Source Point 0 of BTE
#define LCD_RA8875_BTE_VSBE1 0x57     // Vertical Source Point 1 of BTE
#define LCD_RA8875_BTE_HDBE0 0x58     // Horizontal Destination Point 0 of BTE
#define LCD_RA8875_BTE_HDBE1 0x59     // Horizontal Destination Point 1 of BTE
#define LCD_RA8875_BTE_VDBE0 0x5A     // Vertical Destination Point 0 of BTE
#define LCD_RA8875_BTE_VDBE1 0x5B     // Vertical Destination Point 1 of BTE
#define LCD_RA8875_BTE_BEWR0 0x5C     // BTE Width Register 0
#define LCD_RA8875_BTE_BEWR1 0x5D     // BTE Width Register 1
#define LCD_RA8875_BTE_BEHR0 0x5E     // BTE Height Register 0
#define LCD_RA8875_BTE_BEHR1 0x5F     // BTE Height Register 1

#endif
#endif
