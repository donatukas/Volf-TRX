/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    stm32f4xx_it.c
 * @brief   Interrupt Service Routines.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

// EXTI0 - KEY DASH
// EXTI1 - KEY DOT
// EXTI3 - ENC_CLK
// EXTI2 - PTT_IN
// EXTI10 - 48K_Clock
// EXTI13 - ENC2_CLK

// TIM3 - WIFI
// TIM4 - FFT calculation
// TIM5 - audio processor
// TIM6 - every 10ms, different actions
// TIM7 - USB FIFO
// TIM8 - EEPROM / front panel
// TIM1 - ENC2
// TIM2 - Digital decoding, preprocessing

// DMA1-0 - Audio Codec RX
// DMA1-5 - Audio Codec TX
// DMA1-3 - Peripheral SPI2 RX
// DMA1-4 - Peripheral SPI2 TX

// DMA2-0 - copy buffers at 32bit
// DMA2-1 - send audio processor buffer to codec buffer - A
// DMA2-2 - send audio processor buffer to codec buffer - B
// DMA2-4 - fill buffers at 32bit
// DMA2-5 - draw the fft at 16 bits, increment
// DMA2-7 - move waterfall down

/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_it.h"
#include "main.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "cw.h"
#include "events.h"
#include "fft.h"
#include "fpga.h"
#include "front_unit.h"
#include "functions.h"
#include "lcd.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern DMA_HandleTypeDef hdma_memtomem_dma2_stream4;
extern DMA_HandleTypeDef hdma_memtomem_dma2_stream1;
extern DMA_HandleTypeDef hdma_memtomem_dma2_stream0;
extern DMA_HandleTypeDef hdma_memtomem_dma2_stream2;
extern DMA_HandleTypeDef hdma_memtomem_dma2_stream5;
extern DMA_HandleTypeDef hdma_memtomem_dma2_stream7;
extern DMA_HandleTypeDef hdma_spi3_tx;
extern DMA_HandleTypeDef hdma_i2s3_ext_rx;
extern I2S_HandleTypeDef hi2s3;
extern SPI_HandleTypeDef hspi2;
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim5;
extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim7;
extern TIM_HandleTypeDef htim8;
extern PCD_HandleTypeDef hpcd_USB_OTG_FS;
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
 * @brief This function handles Non maskable interrupt.
 */
void NMI_Handler(void) {
	/* USER CODE BEGIN NonMaskableInt_IRQn 0 */
	CPULOAD_WakeUp();
	/* USER CODE END NonMaskableInt_IRQn 0 */
	/* USER CODE BEGIN NonMaskableInt_IRQn 1 */
	while (1) {
	}
	/* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
 * @brief This function handles Hard fault interrupt.
 */
void HardFault_Handler(void) {
	/* USER CODE BEGIN HardFault_IRQn 0 */
	CPULOAD_WakeUp();
	/* USER CODE END HardFault_IRQn 0 */
	while (1) {
		/* USER CODE BEGIN W1_HardFault_IRQn 0 */
		HAL_MPU_Disable();
		LCD_showError("Hard Fault", false);
		static uint32_t i = 0;
		while (i < 99999999) {
			i++;
			__asm("nop");
		}
		HAL_GPIO_WritePin(PWR_HOLD_GPIO_Port, PWR_HOLD_Pin, GPIO_PIN_RESET);
		/* USER CODE END W1_HardFault_IRQn 0 */
	}
}

/**
 * @brief This function handles Memory management fault.
 */
void MemManage_Handler(void) {
	/* USER CODE BEGIN MemoryManagement_IRQn 0 */
	CPULOAD_WakeUp();
	/* USER CODE END MemoryManagement_IRQn 0 */
	while (1) {
		/* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
		HAL_MPU_Disable();
		LCD_showError("Memory Fault", false);
		static uint32_t i = 0;
		while (i < 99999999) {
			i++;
			__asm("nop");
		}
		HAL_GPIO_WritePin(PWR_HOLD_GPIO_Port, PWR_HOLD_Pin, GPIO_PIN_RESET);
		/* USER CODE END W1_MemoryManagement_IRQn 0 */
	}
}

/**
 * @brief This function handles Pre-fetch fault, memory access fault.
 */
void BusFault_Handler(void) {
	/* USER CODE BEGIN BusFault_IRQn 0 */
	CPULOAD_WakeUp();
	/* USER CODE END BusFault_IRQn 0 */
	while (1) {
		/* USER CODE BEGIN W1_BusFault_IRQn 0 */
		LCD_showError("Bus Fault", false);
		static uint32_t i = 0;
		while (i < 99999999) {
			i++;
			__asm("nop");
		}
		HAL_GPIO_WritePin(PWR_HOLD_GPIO_Port, PWR_HOLD_Pin, GPIO_PIN_RESET);
		/* USER CODE END W1_BusFault_IRQn 0 */
	}
}

/**
 * @brief This function handles Undefined instruction or illegal state.
 */
void UsageFault_Handler(void) {
	/* USER CODE BEGIN UsageFault_IRQn 0 */
	CPULOAD_WakeUp();
	/* USER CODE END UsageFault_IRQn 0 */
	while (1) {
		/* USER CODE BEGIN W1_UsageFault_IRQn 0 */
		LCD_showError("Usage Fault", false);
		static uint32_t i = 0;
		while (i < 99999999) {
			i++;
			__asm("nop");
		}
		HAL_GPIO_WritePin(PWR_HOLD_GPIO_Port, PWR_HOLD_Pin, GPIO_PIN_RESET);
		/* USER CODE END W1_UsageFault_IRQn 0 */
	}
}

/**
 * @brief This function handles System service call via SWI instruction.
 */
void SVC_Handler(void) {
	/* USER CODE BEGIN SVCall_IRQn 0 */
	CPULOAD_WakeUp();
	/* USER CODE END SVCall_IRQn 0 */
	/* USER CODE BEGIN SVCall_IRQn 1 */

	/* USER CODE END SVCall_IRQn 1 */
}

/**
 * @brief This function handles Debug monitor.
 */
void DebugMon_Handler(void) {
	/* USER CODE BEGIN DebugMonitor_IRQn 0 */
	CPULOAD_WakeUp();
	/* USER CODE END DebugMonitor_IRQn 0 */
	/* USER CODE BEGIN DebugMonitor_IRQn 1 */

	/* USER CODE END DebugMonitor_IRQn 1 */
}

/**
 * @brief This function handles Pendable request for system service.
 */
void PendSV_Handler(void) {
	/* USER CODE BEGIN PendSV_IRQn 0 */
	CPULOAD_WakeUp();
	/* USER CODE END PendSV_IRQn 0 */
	/* USER CODE BEGIN PendSV_IRQn 1 */

	/* USER CODE END PendSV_IRQn 1 */
}

/**
 * @brief This function handles System tick timer.
 */
void SysTick_Handler(void) {
	/* USER CODE BEGIN SysTick_IRQn 0 */
	CPULOAD_WakeUp();
	/* USER CODE END SysTick_IRQn 0 */
	HAL_IncTick();
	/* USER CODE BEGIN SysTick_IRQn 1 */

	/* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f4xx.s).                    */
/******************************************************************************/

/**
 * @brief This function handles EXTI line0 interrupt.
 */
void EXTI0_IRQHandler(void) {
	/* USER CODE BEGIN EXTI0_IRQn 0 */
	CPULOAD_WakeUp();
	/* USER CODE END EXTI0_IRQn 0 */
	HAL_GPIO_EXTI_IRQHandler(KEY_IN_DASH_Pin);
	/* USER CODE BEGIN EXTI0_IRQn 1 */

	/* USER CODE END EXTI0_IRQn 1 */
}

/**
 * @brief This function handles EXTI line1 interrupt.
 */
void EXTI1_IRQHandler(void) {
	/* USER CODE BEGIN EXTI1_IRQn 0 */
	CPULOAD_WakeUp();
	/* USER CODE END EXTI1_IRQn 0 */
	HAL_GPIO_EXTI_IRQHandler(KEY_IN_DOT_Pin);
	/* USER CODE BEGIN EXTI1_IRQn 1 */

	/* USER CODE END EXTI1_IRQn 1 */
}

/**
 * @brief This function handles EXTI line2 interrupt.
 */
void EXTI2_IRQHandler(void) {
	/* USER CODE BEGIN EXTI2_IRQn 0 */
	CPULOAD_WakeUp();
	/* USER CODE END EXTI2_IRQn 0 */
	HAL_GPIO_EXTI_IRQHandler(PTT_IN_Pin);
	/* USER CODE BEGIN EXTI2_IRQn 1 */

	/* USER CODE END EXTI2_IRQn 1 */
}

/**
 * @brief This function handles EXTI line3 interrupt.
 */
void EXTI3_IRQHandler(void) {
	/* USER CODE BEGIN EXTI3_IRQn 0 */
	CPULOAD_WakeUp();
	/* USER CODE END EXTI3_IRQn 0 */
	HAL_GPIO_EXTI_IRQHandler(ENC_CLK_Pin);
	/* USER CODE BEGIN EXTI3_IRQn 1 */

	/* USER CODE END EXTI3_IRQn 1 */
}

/**
 * @brief This function handles DMA1 stream0 global interrupt.
 */
void DMA1_Stream0_IRQHandler(void) {
	/* USER CODE BEGIN DMA1_Stream0_IRQn 0 */
	CPULOAD_WakeUp();
	/* USER CODE END DMA1_Stream0_IRQn 0 */
	HAL_DMA_IRQHandler(&hdma_i2s3_ext_rx);
	/* USER CODE BEGIN DMA1_Stream0_IRQn 1 */

	/* USER CODE END DMA1_Stream0_IRQn 1 */
}

/**
 * @brief This function handles DMA1 stream5 global interrupt.
 */
void DMA1_Stream5_IRQHandler(void) {
	/* USER CODE BEGIN DMA1_Stream5_IRQn 0 */
	CPULOAD_WakeUp();
	/* USER CODE END DMA1_Stream5_IRQn 0 */
	HAL_DMA_IRQHandler(&hdma_spi3_tx);
	/* USER CODE BEGIN DMA1_Stream5_IRQn 1 */

	/* USER CODE END DMA1_Stream5_IRQn 1 */
}

/**
 * @brief This function handles TIM1 update interrupt and TIM10 global interrupt.
 */
void TIM1_UP_TIM10_IRQHandler(void) {
	/* USER CODE BEGIN TIM1_UP_TIM10_IRQn 0 */
	CPULOAD_WakeUp();
	/* USER CODE END TIM1_UP_TIM10_IRQn 0 */
	HAL_TIM_IRQHandler(&htim1);
	/* USER CODE BEGIN TIM1_UP_TIM10_IRQn 1 */
	EVENTS_do_ENC();
	/* USER CODE END TIM1_UP_TIM10_IRQn 1 */
}

/**
 * @brief This function handles TIM2 global interrupt.
 */
void TIM2_IRQHandler(void) {
	/* USER CODE BEGIN TIM2_IRQn 0 */
	CPULOAD_WakeUp();
	/* USER CODE END TIM2_IRQn 0 */
	HAL_TIM_IRQHandler(&htim2);
	/* USER CODE BEGIN TIM2_IRQn 1 */
	EVENTS_do_PREPROCESS();
	/* USER CODE END TIM2_IRQn 1 */
}

/**
 * @brief This function handles TIM3 global interrupt.
 */
void TIM3_IRQHandler(void) {
	/* USER CODE BEGIN TIM3_IRQn 0 */
	CPULOAD_WakeUp();
	/* USER CODE END TIM3_IRQn 0 */
	HAL_TIM_IRQHandler(&htim3);
	/* USER CODE BEGIN TIM3_IRQn 1 */
	EVENTS_do_WIFI();
	/* USER CODE END TIM3_IRQn 1 */
}

/**
 * @brief This function handles TIM4 global interrupt.
 */
void TIM4_IRQHandler(void) {
	/* USER CODE BEGIN TIM4_IRQn 0 */
	CPULOAD_WakeUp();
	/* USER CODE END TIM4_IRQn 0 */
	HAL_TIM_IRQHandler(&htim4);
	/* USER CODE BEGIN TIM4_IRQn 1 */
	EVENTS_do_FFT();
	/* USER CODE END TIM4_IRQn 1 */
}

/**
 * @brief This function handles SPI2 global interrupt.
 */
void SPI2_IRQHandler(void) {
	/* USER CODE BEGIN SPI2_IRQn 0 */
	CPULOAD_WakeUp();
	/* USER CODE END SPI2_IRQn 0 */
	HAL_SPI_IRQHandler(&hspi2);
	/* USER CODE BEGIN SPI2_IRQn 1 */

	/* USER CODE END SPI2_IRQn 1 */
}

/**
 * @brief This function handles EXTI line[15:10] interrupts.
 */
void EXTI15_10_IRQHandler(void) {
	/* USER CODE BEGIN EXTI15_10_IRQn 0 */
	CPULOAD_WakeUp();
	/* USER CODE END EXTI15_10_IRQn 0 */
	HAL_GPIO_EXTI_IRQHandler(AUDIO_48K_CLOCK_Pin);
	/* USER CODE BEGIN EXTI15_10_IRQn 1 */

	/* USER CODE END EXTI15_10_IRQn 1 */
}

/**
 * @brief This function handles TIM8 update interrupt and TIM13 global interrupt.
 */
void TIM8_UP_TIM13_IRQHandler(void) {
	/* USER CODE BEGIN TIM8_UP_TIM13_IRQn 0 */
	CPULOAD_WakeUp();
	/* USER CODE END TIM8_UP_TIM13_IRQn 0 */
	HAL_TIM_IRQHandler(&htim8);
	/* USER CODE BEGIN TIM8_UP_TIM13_IRQn 1 */
	EVENTS_do_PERIPHERAL();
	/* USER CODE END TIM8_UP_TIM13_IRQn 1 */
}

/**
 * @brief This function handles TIM5 global interrupt.
 */
void TIM5_IRQHandler(void) {
	/* USER CODE BEGIN TIM5_IRQn 0 */
	CPULOAD_WakeUp();
	/* USER CODE END TIM5_IRQn 0 */
	HAL_TIM_IRQHandler(&htim5);
	/* USER CODE BEGIN TIM5_IRQn 1 */
	EVENTS_do_AUDIO_PROCESSOR();
	/* USER CODE END TIM5_IRQn 1 */
}

/**
 * @brief This function handles SPI3 global interrupt.
 */
void SPI3_IRQHandler(void) {
	/* USER CODE BEGIN SPI3_IRQn 0 */
	CPULOAD_WakeUp();
	/* USER CODE END SPI3_IRQn 0 */
	HAL_I2S_IRQHandler(&hi2s3);
	/* USER CODE BEGIN SPI3_IRQn 1 */

	/* USER CODE END SPI3_IRQn 1 */
}

/**
 * @brief This function handles TIM6 global interrupt, DAC1 and DAC2 underrun error interrupts.
 */
void TIM6_DAC_IRQHandler(void) {
	/* USER CODE BEGIN TIM6_DAC_IRQn 0 */
	static uint32_t ms10_counter = 0;
	CPULOAD_WakeUp();
	/* USER CODE END TIM6_DAC_IRQn 0 */
	HAL_TIM_IRQHandler(&htim6);
	/* USER CODE BEGIN TIM6_DAC_IRQn 1 */
	ms10_counter++;

	EVENTS_do_EVERY_10ms();

	if ((ms10_counter % 10) == 0) // every 100ms
	{
		EVENTS_do_EVERY_100ms();
	}

	if (ms10_counter == 51) // every 0.5 sec
	{
		EVENTS_do_EVERY_500ms();
	}

	if (ms10_counter == 101) // every 1 sec
	{
		EVENTS_do_EVERY_1000ms();
		ms10_counter = 0;
	}
	/* USER CODE END TIM6_DAC_IRQn 1 */
}

/**
 * @brief This function handles TIM7 global interrupt.
 */
void TIM7_IRQHandler(void) {
	/* USER CODE BEGIN TIM7_IRQn 0 */
	CPULOAD_WakeUp();
	/* USER CODE END TIM7_IRQn 0 */
	HAL_TIM_IRQHandler(&htim7);
	/* USER CODE BEGIN TIM7_IRQn 1 */
	EVENTS_do_USB_FIFO();
	/* USER CODE END TIM7_IRQn 1 */
}

/**
 * @brief This function handles DMA2 stream1 global interrupt.
 */
void DMA2_Stream1_IRQHandler(void) {
	/* USER CODE BEGIN DMA2_Stream1_IRQn 0 */
	CPULOAD_WakeUp();
	/* USER CODE END DMA2_Stream1_IRQn 0 */
	HAL_DMA_IRQHandler(&hdma_memtomem_dma2_stream1);
	/* USER CODE BEGIN DMA2_Stream1_IRQn 1 */

	/* USER CODE END DMA2_Stream1_IRQn 1 */
}

/**
 * @brief This function handles DMA2 stream2 global interrupt.
 */
void DMA2_Stream2_IRQHandler(void) {
	/* USER CODE BEGIN DMA2_Stream2_IRQn 0 */
	CPULOAD_WakeUp();
	/* USER CODE END DMA2_Stream2_IRQn 0 */
	HAL_DMA_IRQHandler(&hdma_memtomem_dma2_stream2);
	/* USER CODE BEGIN DMA2_Stream2_IRQn 1 */

	/* USER CODE END DMA2_Stream2_IRQn 1 */
}

/**
 * @brief This function handles USB On The Go FS global interrupt.
 */
void OTG_FS_IRQHandler(void) {
	/* USER CODE BEGIN OTG_FS_IRQn 0 */
	CPULOAD_WakeUp();
	/* USER CODE END OTG_FS_IRQn 0 */
	HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
	/* USER CODE BEGIN OTG_FS_IRQn 1 */

	/* USER CODE END OTG_FS_IRQn 1 */
}

/**
 * @brief This function handles DMA2 stream5 global interrupt.
 */
void DMA2_Stream5_IRQHandler(void) {
	/* USER CODE BEGIN DMA2_Stream5_IRQn 0 */
	CPULOAD_WakeUp();
	/* USER CODE END DMA2_Stream5_IRQn 0 */
	HAL_DMA_IRQHandler(&hdma_memtomem_dma2_stream5);
	/* USER CODE BEGIN DMA2_Stream5_IRQn 1 */
	FFT_ShortBufferPrintFFT();
	/* USER CODE END DMA2_Stream5_IRQn 1 */
}

/* USER CODE BEGIN 1 */

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
	CPULOAD_WakeUp();
	if (hspi->Instance == SPI2) {
		SPI_DMA_TXRX_ready_callback = true;
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == AUDIO_48K_CLOCK_Pin) { // 2 - FPGA BUS
		FPGA_fpgadata_iqclock();             // IQ data
		FPGA_fpgadata_stuffclock();          // parameters and other services
	} else if (GPIO_Pin == ENC_CLK_Pin) {  // 3 - Main encoder
		EVENTS_do_ENC();
	} else if (GPIO_Pin == PTT_IN_Pin) { // PTT
		if (TRX_Inited) {
			TRX_ptt_change();
		}
	} else if (GPIO_Pin == KEY_IN_DOT_Pin) { // KEY DOT
		CW_key_change();
	} else if (GPIO_Pin == KEY_IN_DASH_Pin) { // KEY DASH
		CW_key_change();
	}
}

/* USER CODE END 1 */
