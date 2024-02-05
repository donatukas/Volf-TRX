/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    stm32h7xx_it.c
 * @brief   Interrupt Service Routines.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */

// EXTI0 - KEY DASH
// EXTI1 - KEY DOT
// EXTI2 - ENC_CLK
// EXTI4 - PTT_IN
// EXTI10 - 48K_Clock
// EXTI11 - PWR_button
// EXTI13 - ENC2_CLK

// TIM2 - WSPR
// TIM3 - WIFI
// TIM4 - FFT calculation
// TIM5 - audio processor
// TIM6 - every 10ms, different actions
// TIM7 - USB FIFO
// TIM15 - EEPROM / front panel
// TIM16 - Interrogation of the auxiliary encoder, because it hangs on the same interrupt as the FPGA
// TIM17 - Digital CW decoding, ...

// DMA1-0 - receiving data from the audio codec
// DMA1-1 - receiving data from WiFi via UART
// DMA1-2 - Peripheral SPI2 RX
// DMA1-3 - Peripheral SPI2 TX
// DMA1-5 - sending data to audio codec

// DMA2-5 - draw the fft at 16 bits, increment

// MDMA-0 - copy buffers at 32bit
// MDMA-1 - send audio processor buffer to codec buffer - A
// MDMA-2 - send audio processor buffer to codec buffer - B
// MDMA-3 - move the waterfall down
// MDMA-4 - fill buffers at 32bit

/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_it.h"
#include "main.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
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
#include "cw.h"
#include "events.h"
#include "fft.h"
#include "fpga.h"
#include "front_unit.h"
#include "lcd.h"

static uint32_t ms10_counter = 0;
/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern DMA_HandleTypeDef hdma_memtomem_dma2_stream5;
extern DMA2D_HandleTypeDef hdma2d;
extern DMA_HandleTypeDef hdma_spi2_rx;
extern DMA_HandleTypeDef hdma_spi2_tx;
extern SPI_HandleTypeDef hspi2;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim5;
extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim7;
extern TIM_HandleTypeDef htim15;
extern TIM_HandleTypeDef htim16;
extern TIM_HandleTypeDef htim17;
extern DMA_HandleTypeDef hdma_usart6_rx;
extern UART_HandleTypeDef huart6;
extern PCD_HandleTypeDef hpcd_USB_OTG_FS;
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
 * @brief This function handles Non maskable interrupt.
 */
void NMI_Handler(void) {
	/* USER CODE BEGIN NonMaskableInt_IRQn 0 */
	CPULOAD_WakeUp();
	/* USER CODE END NonMaskableInt_IRQn 0 */
	/* USER CODE BEGIN NonMaskableInt_IRQn 1 */

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
/* STM32H7xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32h7xx.s).                    */
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
	HAL_GPIO_EXTI_IRQHandler(ENC_CLK_Pin);
	/* USER CODE BEGIN EXTI2_IRQn 1 */

	/* USER CODE END EXTI2_IRQn 1 */
}

/**
 * @brief This function handles EXTI line4 interrupt.
 */
void EXTI4_IRQHandler(void) {
	/* USER CODE BEGIN EXTI4_IRQn 0 */
	CPULOAD_WakeUp();
	/* USER CODE END EXTI4_IRQn 0 */
	HAL_GPIO_EXTI_IRQHandler(PTT_IN_Pin);
	/* USER CODE BEGIN EXTI4_IRQn 1 */

	/* USER CODE END EXTI4_IRQn 1 */
}

/**
 * @brief This function handles DMA1 stream1 global interrupt.
 */
void DMA1_Stream1_IRQHandler(void) {
	/* USER CODE BEGIN DMA1_Stream1_IRQn 0 */
	CPULOAD_WakeUp();
	/* USER CODE END DMA1_Stream1_IRQn 0 */
	HAL_DMA_IRQHandler(&hdma_usart6_rx);
	/* USER CODE BEGIN DMA1_Stream1_IRQn 1 */

	/* USER CODE END DMA1_Stream1_IRQn 1 */
}

/**
 * @brief This function handles DMA1 stream2 global interrupt.
 */
void DMA1_Stream2_IRQHandler(void) {
	/* USER CODE BEGIN DMA1_Stream2_IRQn 0 */
	CPULOAD_WakeUp();
	/* USER CODE END DMA1_Stream2_IRQn 0 */
	HAL_DMA_IRQHandler(&hdma_spi2_rx);
	/* USER CODE BEGIN DMA1_Stream2_IRQn 1 */

	/* USER CODE END DMA1_Stream2_IRQn 1 */
}

/**
 * @brief This function handles DMA1 stream3 global interrupt.
 */
void DMA1_Stream3_IRQHandler(void) {
	/* USER CODE BEGIN DMA1_Stream3_IRQn 0 */
	CPULOAD_WakeUp();
	/* USER CODE END DMA1_Stream3_IRQn 0 */
	HAL_DMA_IRQHandler(&hdma_spi2_tx);
	/* USER CODE BEGIN DMA1_Stream3_IRQn 1 */

	/* USER CODE END DMA1_Stream3_IRQn 1 */
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
	EVENTS_do_WSPR();
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
	HAL_GPIO_EXTI_IRQHandler(PWR_ON_Pin);
	/* USER CODE BEGIN EXTI15_10_IRQn 1 */

	/* USER CODE END EXTI15_10_IRQn 1 */
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
	// StartProfilerUs();
	EVENTS_do_AUDIO_PROCESSOR();
	// EndProfilerUs(true);
	/* USER CODE END TIM5_IRQn 1 */
}

/**
 * @brief This function handles TIM6 global interrupt, DAC1_CH1 and DAC1_CH2 underrun error interrupts.
 */
void TIM6_DAC_IRQHandler(void) {
	/* USER CODE BEGIN TIM6_DAC_IRQn 0 */
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
 * @brief This function handles DMA2 stream5 global interrupt.
 */
void DMA2_Stream5_IRQHandler(void) {
	/* USER CODE BEGIN DMA2_Stream5_IRQn 0 */
	CPULOAD_WakeUp();
	/* USER CODE END DMA2_Stream5_IRQn 0 */
	HAL_DMA_IRQHandler(&hdma_memtomem_dma2_stream5);
	/* USER CODE BEGIN DMA2_Stream5_IRQn 1 */
#if HRDW_HAS_FULL_FFT_BUFFER
	FFT_afterPrintFFT();
#else
	FFT_ShortBufferPrintFFT();
#endif
	/* USER CODE END DMA2_Stream5_IRQn 1 */
}

/**
 * @brief This function handles USART6 global interrupt.
 */
void USART6_IRQHandler(void) {
	/* USER CODE BEGIN USART6_IRQn 0 */
	CPULOAD_WakeUp();
	/* USER CODE END USART6_IRQn 0 */
	HAL_UART_IRQHandler(&huart6);
	/* USER CODE BEGIN USART6_IRQn 1 */

	/* USER CODE END USART6_IRQn 1 */
}

/**
 * @brief This function handles DMA2D global interrupt.
 */
void DMA2D_IRQHandler(void) {
	/* USER CODE BEGIN DMA2D_IRQn 0 */
	CPULOAD_WakeUp();
	/* USER CODE END DMA2D_IRQn 0 */
	HAL_DMA2D_IRQHandler(&hdma2d);
	/* USER CODE BEGIN DMA2D_IRQn 1 */

	/* USER CODE END DMA2D_IRQn 1 */
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
 * @brief This function handles TIM15 global interrupt.
 */
void TIM15_IRQHandler(void) {
	/* USER CODE BEGIN TIM15_IRQn 0 */
	CPULOAD_WakeUp();
	/* USER CODE END TIM15_IRQn 0 */
	HAL_TIM_IRQHandler(&htim15);
	/* USER CODE BEGIN TIM15_IRQn 1 */
	EVENTS_do_PERIPHERAL();
	/* USER CODE END TIM15_IRQn 1 */
}

/**
 * @brief This function handles TIM16 global interrupt.
 */
void TIM16_IRQHandler(void) {
	/* USER CODE BEGIN TIM16_IRQn 0 */
	CPULOAD_WakeUp();
	/* USER CODE END TIM16_IRQn 0 */
	HAL_TIM_IRQHandler(&htim16);
	/* USER CODE BEGIN TIM16_IRQn 1 */
	EVENTS_do_ENC();
	/* USER CODE END TIM16_IRQn 1 */
}

/**
 * @brief This function handles TIM17 global interrupt.
 */
void TIM17_IRQHandler(void) {
	/* USER CODE BEGIN TIM17_IRQn 0 */
	CPULOAD_WakeUp();
	/* USER CODE END TIM17_IRQn 0 */
	HAL_TIM_IRQHandler(&htim17);
	/* USER CODE BEGIN TIM17_IRQn 1 */
	EVENTS_do_PREPROCESS();
	/* USER CODE END TIM17_IRQn 1 */
}

/* USER CODE BEGIN 1 */

void DMA1_Stream0_IRQHandler(void) {
	CPULOAD_WakeUp();
	HAL_DMA_IRQHandler(&hdma_spi3_rx);
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
	CPULOAD_WakeUp();
	if (hspi->Instance == SPI2) {
		SPI_DMA_TXRX_ready_callback = true;
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == GPIO_PIN_10) {       // FPGA BUS
		FPGA_fpgadata_iqclock();           // IQ data
		FPGA_fpgadata_stuffclock();        // parameters and other services
	} else if (GPIO_Pin == GPIO_PIN_2) { // Main encoder
		EVENTS_do_ENC();
	} else if (GPIO_Pin == GPIO_PIN_4) { // PTT
		if (TRX_Inited) {
			TRX_ptt_change();
		}
	} else if (GPIO_Pin == GPIO_PIN_1) { // KEY DOT
		CW_key_change();
	} else if (GPIO_Pin == GPIO_PIN_0) { // KEY DASH
		CW_key_change();
	}
}

void DMA1_Stream5_IRQHandler(void) {
	CPULOAD_WakeUp();
	HAL_DMA_IRQHandler(&hdma_spi3_tx);
}

/* USER CODE END 1 */
