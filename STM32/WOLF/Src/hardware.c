#include "hardware.h"
#include "functions.h"
#include "main.h"

bool HRDW_SPI_Internal_STM32_Clock = false;
bool HRDW_SPI_Periph_busy = false;
volatile bool HRDW_SPI_Locked = false;
bool dma_memset32_busy = false;
bool dma_memcpy32_busy = false;
bool HRDW_DAC_PLL_selected = true;

CPULOAD_t CPU_LOAD = {0};
static uint32_t CPULOAD_startWorkTime = 0;
static uint32_t CPULOAD_startSleepTime = 0;
static uint32_t CPULOAD_WorkingTime = 0;
static uint32_t CPULOAD_SleepingTime = 0;
static uint32_t CPULOAD_SleepCounter = 0;
static bool CPULOAD_status = true; // true - wake up ; false - sleep

void CPULOAD_Init(void) {
	DBGMCU->CR |= (DBGMCU_CR_DBG_SLEEPD1_Msk | DBGMCU_CR_DBG_STOPD1_Msk | DBGMCU_CR_DBG_STANDBYD1_Msk);
	// allow using the counter
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	// start the counter
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
	// zero the value of the counting register
	DWT->CYCCNT = 0;
	CPULOAD_status = true;
}

void CPULOAD_GoToSleepMode(void) {
	// Add to working time
	CPULOAD_WorkingTime += DWT->CYCCNT - CPULOAD_startWorkTime;
	// Save count cycle time
	CPULOAD_SleepCounter++;
	CPULOAD_startSleepTime = DWT->CYCCNT;
	CPULOAD_status = false;
	// Go to sleep mode Wait for wake up interrupt
	__WFI();
}

void CPULOAD_WakeUp(void) {
	if (CPULOAD_status) {
		return;
	}
	CPULOAD_status = true;
	// Increase number of sleeping time in CPU cycles
	CPULOAD_SleepingTime += DWT->CYCCNT - CPULOAD_startSleepTime;
	// Save current time to get number of working CPU cycles
	CPULOAD_startWorkTime = DWT->CYCCNT;
}

void CPULOAD_Calc(void) {
	// Save values
	CPU_LOAD.SCNT = CPULOAD_SleepingTime;
	CPU_LOAD.WCNT = CPULOAD_WorkingTime;
	CPU_LOAD.SINC = CPULOAD_SleepCounter;
	CPU_LOAD.Load = ((float)CPULOAD_WorkingTime / (float)(CPULOAD_SleepingTime + CPULOAD_WorkingTime) * 100);
	if (CPU_LOAD.SCNT == 0) {
		CPU_LOAD.Load = 100;
	}
	if (CPU_LOAD.SCNT == 0 && CPU_LOAD.WCNT == 0) {
		CPU_LOAD.Load = 255;
		CPULOAD_Init();
	}
	// Reset time
	CPULOAD_SleepingTime = 0;
	CPULOAD_WorkingTime = 0;
	CPULOAD_SleepCounter = 0;
}

void HRDW_Init(void) {
	HAL_ADCEx_InjectedStart(&hadc1); // ADC RF-UNIT'а
#if defined(FRONTPANEL_X1) || defined(FRONTPANEL_MINI)
	HAL_ADCEx_InjectedStart(&hadc2); // ADC Tangent (some versions)
#endif
	HAL_ADCEx_InjectedStart(&hadc3); // ADC CPU temperature
#if HRDW_HAS_VHF_MIXER
	RFMIXER_Init();
#endif
}

float32_t HRDW_getCPUTemperature(void) {
	// STM32H743 Temperature
	uint16_t TS_CAL1 = *((uint16_t *)0x1FF1E820); // TS_CAL1 Temperature sensor raw data acquired value at 30 °C, VDDA=3.3 V //-V566
	uint16_t TS_CAL2 = *((uint16_t *)0x1FF1E840); // TS_CAL2 Temperature sensor raw data acquired value at 110 °C, VDDA=3.3 V //-V566
	uint32_t TS_DATA = HAL_ADCEx_InjectedGetValue(&hadc3, ADC_INJECTED_RANK_1);
	float32_t result = ((110.0f - 30.0f) / ((float32_t)TS_CAL2 - (float32_t)TS_CAL1)) * ((float32_t)TS_DATA - (float32_t)TS_CAL1) + 30; // from reference
	return result;
}

float32_t HRDW_getCPUVref(void) {
	// STM32H743 VREF
	uint16_t VREFINT_CAL = *VREFINT_CAL_ADDR; // VREFIN_CAL Raw data acquired at temperature of 30 °C, VDDA = 3.3 V //-V566
	uint32_t VREFINT_DATA = HAL_ADCEx_InjectedGetValue(&hadc3, ADC_INJECTED_RANK_2);
	float32_t result = 3.3f * (float32_t)VREFINT_CAL / (float32_t)VREFINT_DATA; // from reference
	return result;
}

inline uint32_t HRDW_getAudioCodecRX_DMAIndex(void) { return CODEC_AUDIO_BUFFER_SIZE - (uint16_t)__HAL_DMA_GET_COUNTER(HRDW_AUDIO_CODEC_I2S.hdmarx); }

inline uint32_t HRDW_getAudioCodecTX_DMAIndex(void) { return CODEC_AUDIO_BUFFER_SIZE * 2 - (uint16_t)__HAL_DMA_GET_COUNTER(HRDW_AUDIO_CODEC_I2S.hdmatx); }

#ifdef HRDW_MCP3008_1
inline bool HRDW_FrontUnit_SPI(uint8_t *out_data, uint8_t *in_data, uint32_t count, bool hold_cs) {
	if (HRDW_SPI_Periph_busy) {
		println("SPI Busy");
		return false;
	}
	HRDW_SPI_Periph_busy = true;

	bool result = SPI_Transmit(&hspi2, out_data, in_data, count, AD1_CS_GPIO_Port, AD1_CS_Pin, hold_cs, SPI_FRONT_UNIT_PRESCALER, false);

	HRDW_SPI_Periph_busy = false;
	return result;
}
#endif

#ifdef HRDW_MCP3008_2
inline bool HRDW_FrontUnit2_SPI(uint8_t *out_data, uint8_t *in_data, uint32_t count, bool hold_cs) {
	if (HRDW_SPI_Periph_busy) {
		println("SPI Busy");
		return false;
	}
	HRDW_SPI_Periph_busy = true;

	bool result = SPI_Transmit(&hspi2, out_data, in_data, count, AD2_CS_GPIO_Port, AD2_CS_Pin, hold_cs, SPI_FRONT_UNIT_PRESCALER, false);

	HRDW_SPI_Periph_busy = false;
	return result;
}
#endif

#ifdef HRDW_MCP3008_3
inline bool HRDW_FrontUnit3_SPI(uint8_t *out_data, uint8_t *in_data, uint32_t count, bool hold_cs) {
	if (HRDW_SPI_Periph_busy) {
		println("SPI Busy");
		return false;
	}
	HRDW_SPI_Periph_busy = true;

	bool result = SPI_Transmit(&hspi2, out_data, in_data, count, AD3_CS_GPIO_Port, AD3_CS_Pin, hold_cs, SPI_FRONT_UNIT_PRESCALER, false);

	HRDW_SPI_Periph_busy = false;
	return result;
}
#endif

inline bool HRDW_EEPROM_SPI(uint8_t *out_data, uint8_t *in_data, uint32_t count, bool hold_cs) {
	if (HRDW_SPI_Periph_busy) {
		println("SPI Busy");
		return false;
	}
	HRDW_SPI_Periph_busy = true;

	bool result = SPI_Transmit(&hspi2, out_data, in_data, count, W25Q16_CS_GPIO_Port, W25Q16_CS_Pin, hold_cs, SPI_EEPROM_PRESCALER, true);

	HRDW_SPI_Periph_busy = false;
	return result;
}

inline bool HRDW_SD_SPI(uint8_t *out_data, uint8_t *in_data, uint32_t count, bool hold_cs) {
	if (HRDW_SPI_Periph_busy) {
		println("SPI Busy");
		return false;
	}
	HRDW_SPI_Periph_busy = true;

	bool result = SPI_Transmit(&hspi2, out_data, in_data, count, SD_CS_GPIO_Port, SD_CS_Pin, hold_cs, SPI_SD_PRESCALER, true);

	HRDW_SPI_Periph_busy = false;
	return result;
}

static uint32_t dma_memset32_reg = 0;
void dma_memset32(void *dest, uint32_t val, uint32_t size) {
	if (size == 0) {
		return;
	}

	if (dma_memset32_busy) // for async calls
	{
		if (val == 0) {
			memset(dest, val, size * 4);
		} else {
			uint32_t *buf = dest;
			while (size--) {
				*buf++ = val;
			}
		}
		return;
	}

	dma_memset32_busy = true;
	dma_memset32_reg = val;
	Aligned_CleanDCache_by_Addr(&dma_memset32_reg, sizeof(dma_memset32_reg));
	Aligned_CleanDCache_by_Addr(dest, size * 4);

	uint32_t max_block = DMA_MAX_BLOCK / 4;
	uint32_t *current_dest = (uint32_t *)dest;
	uint32_t estimated = size;
	while (estimated > 0) {
		uint32_t block_size = (estimated > max_block) ? max_block : estimated;
		HAL_MDMA_Start(&HRDW_MEMSET_MDMA, (uint32_t)&dma_memset32_reg, (uint32_t)current_dest, 4 * block_size, 1);
		SLEEPING_MDMA_PollForTransfer(&HRDW_MEMSET_MDMA);
		estimated -= block_size;
		current_dest += block_size;
	}

	Aligned_CleanInvalidateDCache_by_Addr(dest, size * 4);
	dma_memset32_busy = false;

	/*uint32_t *pDst = (uint32_t *)dest;
	uint8_t errors = 0;
	for(uint32_t i = 0; i < size; i++)
	  if(pDst[i] != val && errors < 3)
	  {
	    println(i);
	    errors++;
	  }*/
}

void dma_memcpy32(void *dest, void *src, uint32_t size) {
	if (size == 0) {
		return;
	}

	if (dma_memcpy32_busy) // for async calls
	{
		memcpy(dest, src, size * 4);
		return;
	}

	dma_memcpy32_busy = true;
	Aligned_CleanDCache_by_Addr(src, size * 4);
	Aligned_CleanDCache_by_Addr(dest, size * 4);

	HAL_MDMA_Start(&HRDW_MEMCPY_MDMA, (uint32_t)src, (uint32_t)dest, size * 4, 1);
	SLEEPING_MDMA_PollForTransfer(&HRDW_MEMCPY_MDMA);

	Aligned_CleanInvalidateDCache_by_Addr(dest, size * 4);
	dma_memcpy32_busy = false;

	/*char *pSrc = (char *)src;
	char *pDst = (char *)dest;
	for(uint32_t i = 0; i < size * 4; i++)
	  if(pSrc[i] != pDst[i])
	    println(size * 4, " ", i);*/
}

void HRDW_GoToInternalSPIClock(void) {
	HRDW_SPI_Internal_STM32_Clock = true;

	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SPI3 | RCC_PERIPHCLK_SPI2 | RCC_PERIPHCLK_CKPER;
	PeriphClkInitStruct.PLL2.PLL2M = 1;
	PeriphClkInitStruct.PLL2.PLL2N = 48;
	PeriphClkInitStruct.PLL2.PLL2P = 5;
	PeriphClkInitStruct.PLL2.PLL2Q = 2;
	PeriphClkInitStruct.PLL2.PLL2R = 2;
	PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_3;
	PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
	PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
	PeriphClkInitStruct.CkperClockSelection = RCC_CLKPSOURCE_HSI;
	PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL2;

	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
		Error_Handler();
	}
}

uint8_t HARDW_GetAntCount(void) {
#if defined(FRONTPANEL_WOLF_2)
	return 2;
#elif defined(FRONTPANEL_MINI)
	return 1;
#else
	switch (CALIBRATE.RF_unit_type) {
	case RF_UNIT_NONE:
	case RF_UNIT_RU4PN:
	case RF_UNIT_QRP:
	case RF_UNIT_WF_100D:
	case RF_UNIT_BIG:
	case RF_UNIT_SPLIT:
		return 2;
	case RF_UNIT_KT_100S:
		return 4;
	}

	return 1;
#endif
}