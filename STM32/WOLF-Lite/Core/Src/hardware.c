#include "hardware.h"
#include "functions.h"
#include "main.h"

bool HRDW_SPI_Periph_busy = false;
volatile bool HRDW_SPI_Locked = false;
bool dma_memset32_busy = false;
bool dma_memcpy32_busy = false;

CPULOAD_t CPU_LOAD = {0};
static uint32_t CPULOAD_startWorkTime = 0;
static uint32_t CPULOAD_startSleepTime = 0;
static uint32_t CPULOAD_WorkingTime = 0;
static uint32_t CPULOAD_SleepingTime = 0;
static uint32_t CPULOAD_SleepCounter = 0;
static bool CPULOAD_status = true; // true - wake up ; false - sleep

void CPULOAD_Init(void) {
	// DBGMCU->CR |= (DBGMCU_CR_DBG_SLEEPD1_Msk | DBGMCU_CR_DBG_STOPD1_Msk | DBGMCU_CR_DBG_STANDBYD1_Msk);
	//  allow using the counter
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
	HAL_ADCEx_InjectedStart(&hadc1);
	HAL_ADCEx_InjectedStart(&hadc2);
	HAL_ADCEx_InjectedStart(&hadc3);
}

float32_t HRDW_getCPUTemperature(void) {
	// STM32F407 Temperature
	float32_t cpu_temperature = (float32_t)(HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_1)) * 3.3f / 4096.0f;
	float32_t cpu_temperature_result = (cpu_temperature - 0.760f) / 0.0025f + 25.0f;
	return cpu_temperature_result;
}

float32_t HRDW_getCPUVref(void) {
	// STM32F407 VREF
	uint16_t VREFINT_CAL = *VREFINT_CAL_ADDR;
	float32_t cpu_vref = (float32_t)(HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_2));
	float32_t cpu_vref_result = 3.3f * (float32_t)VREFINT_CAL / (float32_t)cpu_vref;
	return cpu_vref_result;
}

inline uint32_t HRDW_getAudioCodecRX_DMAIndex(void) { return CODEC_AUDIO_BUFFER_SIZE - (uint16_t)__HAL_DMA_GET_COUNTER(HRDW_AUDIO_CODEC_I2S.hdmarx) / 2; }

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

	bool result = SPI_Transmit(&hspi2, out_data, in_data, count, W25Q16_CS_GPIO_Port, W25Q16_CS_Pin, hold_cs, SPI_EEPROM_PRESCALER, false);

	HRDW_SPI_Periph_busy = false;
	return result;
}

inline bool HRDW_SD_SPI(uint8_t *out_data, uint8_t *in_data, uint32_t count, bool hold_cs) { return false; }

static uint32_t dma_memset32_reg = 0;
void dma_memset32(void *dest, uint32_t val, uint32_t size) {
	if (val == 0) {
		memset(dest, val, size * 4);
	} else {
		uint32_t *buf = dest;
		while (size--) {
			*buf++ = val;
		}
	}
}

void dma_memcpy32(void *dest, void *src, uint32_t size) { memcpy(dest, src, size * 4); }

void dma_memset(void *dest, uint8_t val, uint32_t size) { memset(dest, val, size); }

void dma_memcpy(void *dest, void *src, uint32_t size) { memcpy(dest, src, size); }

void SCB_CleanDCache_by_Addr(uint32_t *addr, uint32_t size) {}
void SCB_InvalidateDCache_by_Addr(uint32_t *addr, uint32_t size) {}
void SCB_CleanInvalidateDCache_by_Addr(uint32_t *addr, uint32_t size) {}

uint8_t HARDW_GetAntCount(void) { return 1; }
