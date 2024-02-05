#ifndef __HARDWARE_H
#define __HARDWARE_H

#include "arm_math_types.h"
#include "main.h"
#include "stm32f4xx.h"
#include <stdbool.h>

// features
#define HRDW_HAS_MDMA false
#define HRDW_HAS_SD false
#define HRDW_HAS_DMA2D false
#define HRDW_HAS_JPEG false
#define HRDW_HAS_WIFI false
#define HRDW_HAS_USB_DEBUG false
#define HRDW_HAS_USB_CAT true
#define HRDW_HAS_USB_IQ false
#define HRDW_HAS_DUAL_RX false
#define HRDW_HAS_FULL_FFT_BUFFER false
#define HRDW_USB_AUDIO_BITS 16
#define HRDW_AUDIO_CODEC_WM8731 true
#define HRDW_HAS_LNA_HF false
#define HRDW_HAS_LNA_VHF false

#define HRDW_DEBUG_ON_CAT_PORT false //********************************************************************

// settings
#define ADC_CLOCK 61440000LL                // ADC generator frequency
#define DAC_CLOCK (ADC_CLOCK * 85LL / 32LL) // DAC generator frequency (163.200.000)
#define MAX_TX_FREQ_HZ (DAC_CLOCK / 2)      // Maximum transmission frequency
#define ADC_FULL_SCALE 4096                 // maximum signal amplitude in the ADC // powf (2, ADC_BITS)
#define FLOAT_FULL_SCALE_POW 4
#define DCDC_FREQ_0 960000
#define DCDC_FREQ_1 1200000
#define ADC_INPUT_IMPEDANCE 200.0f // 50ohm -> 1:4 trans
#define ADC_RANGE 2.25f
#define ADC_RANGE_PGA 1.5f
#define ADC_LNA_GAIN_DB 20.0f
#define ADC_DRIVER_GAIN_DB 25.5f
#define ADC_PGA_GAIN_DB 3.522f

// instances
#define HRDW_AUDIO_CODEC_I2S hi2s3
// #define HRDW_WSPR_TIMER
// #define HRDW_WIFI_UART
#define HRDW_FSMC_LCD hsram1
#define HRDW_CRC_HANDLE hcrc
// #define HRDW_JPEG_HANDLE
// #define HRDW_LCD_SPI
#define HRDW_IWDG hiwdg
#define HRDW_LCD_FSMC_COPY_DMA hdma_memtomem_dma2_stream5
#define HRDW_MEMCPY_DMA hdma_memtomem_dma2_stream0
#define HRDW_MEMSET_DMA hdma_memtomem_dma2_stream4
#define HRDW_AUDIO_COPY_DMA hdma_memtomem_dma2_stream1
#define HRDW_AUDIO_COPY2_DMA hdma_memtomem_dma2_stream2

// SPI Speed
#define SPI_FRONT_UNIT_PRESCALER SPI_BAUDRATEPRESCALER_16
#define SPI_EEPROM_PRESCALER SPI_BAUDRATEPRESCALER_16

// buffers
#define CODEC_AUDIO_BUFFER_SIZE (AUDIO_BUFFER_SIZE * 2)          // the size of the circular buffer is 2 times larger than the FPGA buffer, we work in the first half, then on the other
#define CODEC_AUDIO_BUFFER_HALF_SIZE AUDIO_BUFFER_SIZE           // half buffer
#define AUDIO_BUFFER_SIZE (192 * 2)                              // the size of the buffer for working with sound 48kHz
#define AUDIO_BUFFER_HALF_SIZE (AUDIO_BUFFER_SIZE / 2)           // buffer size for working with sound 48kHz
#define FPGA_TX_IQ_BUFFER_SIZE AUDIO_BUFFER_SIZE                 // size of TX data buffer for FPGA
#define FPGA_TX_IQ_BUFFER_HALF_SIZE (FPGA_TX_IQ_BUFFER_SIZE / 2) // half the size of the TX data buffer for FPGA
#define FPGA_RX_IQ_BUFFER_SIZE FPGA_TX_IQ_BUFFER_SIZE            // size of the RX data buffer from the PGA
#define FPGA_RX_IQ_BUFFER_HALF_SIZE (FPGA_RX_IQ_BUFFER_SIZE / 2) // half the size of the RX data buffer from the PGA

// macros
#define FPGA_setGPIOBusInput GPIOA->MODER = -1431764992
#define FPGA_setGPIOBusOutput GPIOA->MODER = -1431743147

// types
typedef struct {
	float32_t Load; /*!< CPU load percentage */
	uint32_t WCNT;  /*!< Number of working cycles in one period. Meant for private use */
	uint32_t SCNT;  /*!< Number of sleeping cycles in one period. Meant for private use */
	uint32_t SINC;
} CPULOAD_t;

// variables
extern bool HRDW_SPI_Periph_busy;
extern volatile bool HRDW_SPI_Locked;
extern bool dma_memset32_busy;
extern bool dma_memcpy32_busy;
extern CPULOAD_t CPU_LOAD;

// methods
extern void CPULOAD_Init(void);
extern void CPULOAD_GoToSleepMode(void);
extern void CPULOAD_WakeUp(void);
extern void CPULOAD_Calc(void);
extern void HRDW_Init(void);
extern float32_t HRDW_getCPUTemperature(void);
extern float32_t HRDW_getCPUVref(void);
extern uint32_t HRDW_getAudioCodecTX_DMAIndex(void);
extern uint32_t HRDW_getAudioCodecRX_DMAIndex(void);
extern bool HRDW_FrontUnit_SPI(uint8_t *out_data, uint8_t *in_data, uint32_t count, bool hold_cs);
extern bool HRDW_FrontUnit2_SPI(uint8_t *out_data, uint8_t *in_data, uint32_t count, bool hold_cs);
extern bool HRDW_FrontUnit3_SPI(uint8_t *out_data, uint8_t *in_data, uint32_t count, bool hold_cs);
extern bool HRDW_EEPROM_SPI(uint8_t *out_data, uint8_t *in_data, uint32_t count, bool hold_cs);
extern bool HRDW_SD_SPI(uint8_t *out_data, uint8_t *in_data, uint32_t count, bool hold_cs);
extern void SCB_CleanDCache_by_Addr(uint32_t *addr, uint32_t size);
extern void SCB_InvalidateDCache_by_Addr(uint32_t *addr, uint32_t size);
extern void SCB_CleanInvalidateDCache_by_Addr(uint32_t *addr, uint32_t size);
extern uint8_t HARDW_GetAntCount(void);

#endif
