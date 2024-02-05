/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : stm32h7xx_hal_msp.c
 * Description        : This file provides code for the MSP Initialization
 *                      and de-Initialization codes.
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
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
extern DMA_HandleTypeDef hdma_spi3_rx;
/* USER CODE END Includes */
extern DMA_HandleTypeDef hdma_spi2_rx;

extern DMA_HandleTypeDef hdma_spi2_tx;

extern DMA_HandleTypeDef hdma_spi4_tx;

extern DMA_HandleTypeDef hdma_usart6_rx;

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN Define */

/* USER CODE END Define */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN Macro */
/* USER CODE END Macro */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* External functions --------------------------------------------------------*/
/* USER CODE BEGIN ExternalFunctions */

/* USER CODE END ExternalFunctions */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
/**
 * Initializes the Global MSP.
 */
void HAL_MspInit(void) {
	/* USER CODE BEGIN MspInit 0 */

	/* USER CODE END MspInit 0 */

	__HAL_RCC_SYSCFG_CLK_ENABLE();

	/* System interrupt init*/

	/* USER CODE BEGIN MspInit 1 */

	/* USER CODE END MspInit 1 */
}

static uint32_t HAL_RCC_ADC12_CLK_ENABLED = 0;

/**
 * @brief ADC MSP Initialization
 * This function configures the hardware resources used in this example
 * @param hadc: ADC handle pointer
 * @retval None
 */
void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc) {
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
	if (hadc->Instance == ADC1) {
		/* USER CODE BEGIN ADC1_MspInit 0 */

		/* USER CODE END ADC1_MspInit 0 */

		/** Initializes the peripherals clock
		 */
		PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_ADC;
		PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_CLKP;
		if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
			Error_Handler();
		}

		/* Peripheral clock enable */
		HAL_RCC_ADC12_CLK_ENABLED++;
		if (HAL_RCC_ADC12_CLK_ENABLED == 1) {
			__HAL_RCC_ADC12_CLK_ENABLE();
		}

		__HAL_RCC_GPIOC_CLK_ENABLE();
		__HAL_RCC_GPIOB_CLK_ENABLE();
		/**ADC1 GPIO Configuration
		PC0     ------> ADC1_INP10
		PC1     ------> ADC1_INP11
		PC4     ------> ADC1_INP4
		PC5     ------> ADC1_INP8
		PB0     ------> ADC1_INP9
		PB1     ------> ADC1_INP5
		*/
		GPIO_InitStruct.Pin = SWR_BACKW_Pin | SWR_FORW_Pin | CURERNT_METER_Pin | PWR_VOLTMETER_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = TANG_SW1_Pin | TANG_SW2_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		/* USER CODE BEGIN ADC1_MspInit 1 */

		/* USER CODE END ADC1_MspInit 1 */
	} else if (hadc->Instance == ADC2) {
		/* USER CODE BEGIN ADC2_MspInit 0 */

		/* USER CODE END ADC2_MspInit 0 */

		/** Initializes the peripherals clock
		 */
		PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_ADC;
		PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_CLKP;
		if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
			Error_Handler();
		}

		/* Peripheral clock enable */
		HAL_RCC_ADC12_CLK_ENABLED++;
		if (HAL_RCC_ADC12_CLK_ENABLED == 1) {
			__HAL_RCC_ADC12_CLK_ENABLE();
		}

		__HAL_RCC_GPIOC_CLK_ENABLE();
		__HAL_RCC_GPIOB_CLK_ENABLE();
		/**ADC2 GPIO Configuration
		PC1     ------> ADC2_INP11
		PC5     ------> ADC2_INP8
		PB0     ------> ADC2_INP9
		PB1     ------> ADC2_INP5
		*/
		GPIO_InitStruct.Pin = SWR_FORW_Pin | PWR_VOLTMETER_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = TANG_SW1_Pin | TANG_SW2_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		/* USER CODE BEGIN ADC2_MspInit 1 */

		/* USER CODE END ADC2_MspInit 1 */
	} else if (hadc->Instance == ADC3) {
		/* USER CODE BEGIN ADC3_MspInit 0 */

		/* USER CODE END ADC3_MspInit 0 */

		/** Initializes the peripherals clock
		 */
		PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_ADC;
		PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_CLKP;
		if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
			Error_Handler();
		}

		/* Peripheral clock enable */
		__HAL_RCC_ADC3_CLK_ENABLE();
		/* USER CODE BEGIN ADC3_MspInit 1 */

		/* USER CODE END ADC3_MspInit 1 */
	}
}

/**
 * @brief ADC MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hadc: ADC handle pointer
 * @retval None
 */
void HAL_ADC_MspDeInit(ADC_HandleTypeDef *hadc) {
	if (hadc->Instance == ADC1) {
		/* USER CODE BEGIN ADC1_MspDeInit 0 */

		/* USER CODE END ADC1_MspDeInit 0 */
		/* Peripheral clock disable */
		HAL_RCC_ADC12_CLK_ENABLED--;
		if (HAL_RCC_ADC12_CLK_ENABLED == 0) {
			__HAL_RCC_ADC12_CLK_DISABLE();
		}

		/**ADC1 GPIO Configuration
		PC0     ------> ADC1_INP10
		PC1     ------> ADC1_INP11
		PC4     ------> ADC1_INP4
		PC5     ------> ADC1_INP8
		PB0     ------> ADC1_INP9
		PB1     ------> ADC1_INP5
		*/
		HAL_GPIO_DeInit(GPIOC, SWR_BACKW_Pin | SWR_FORW_Pin | CURERNT_METER_Pin | PWR_VOLTMETER_Pin);

		HAL_GPIO_DeInit(GPIOB, TANG_SW1_Pin | TANG_SW2_Pin);

		/* USER CODE BEGIN ADC1_MspDeInit 1 */

		/* USER CODE END ADC1_MspDeInit 1 */
	} else if (hadc->Instance == ADC2) {
		/* USER CODE BEGIN ADC2_MspDeInit 0 */

		/* USER CODE END ADC2_MspDeInit 0 */
		/* Peripheral clock disable */
		HAL_RCC_ADC12_CLK_ENABLED--;
		if (HAL_RCC_ADC12_CLK_ENABLED == 0) {
			__HAL_RCC_ADC12_CLK_DISABLE();
		}

		/**ADC2 GPIO Configuration
		PC1     ------> ADC2_INP11
		PC5     ------> ADC2_INP8
		PB0     ------> ADC2_INP9
		PB1     ------> ADC2_INP5
		*/
		HAL_GPIO_DeInit(GPIOC, SWR_FORW_Pin | PWR_VOLTMETER_Pin);

		HAL_GPIO_DeInit(GPIOB, TANG_SW1_Pin | TANG_SW2_Pin);

		/* USER CODE BEGIN ADC2_MspDeInit 1 */

		/* USER CODE END ADC2_MspDeInit 1 */
	} else if (hadc->Instance == ADC3) {
		/* USER CODE BEGIN ADC3_MspDeInit 0 */

		/* USER CODE END ADC3_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_ADC3_CLK_DISABLE();
		/* USER CODE BEGIN ADC3_MspDeInit 1 */

		/* USER CODE END ADC3_MspDeInit 1 */
	}
}

/**
 * @brief CRC MSP Initialization
 * This function configures the hardware resources used in this example
 * @param hcrc: CRC handle pointer
 * @retval None
 */
void HAL_CRC_MspInit(CRC_HandleTypeDef *hcrc) {
	if (hcrc->Instance == CRC) {
		/* USER CODE BEGIN CRC_MspInit 0 */

		/* USER CODE END CRC_MspInit 0 */
		/* Peripheral clock enable */
		__HAL_RCC_CRC_CLK_ENABLE();
		/* USER CODE BEGIN CRC_MspInit 1 */

		/* USER CODE END CRC_MspInit 1 */
	}
}

/**
 * @brief CRC MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hcrc: CRC handle pointer
 * @retval None
 */
void HAL_CRC_MspDeInit(CRC_HandleTypeDef *hcrc) {
	if (hcrc->Instance == CRC) {
		/* USER CODE BEGIN CRC_MspDeInit 0 */

		/* USER CODE END CRC_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_CRC_CLK_DISABLE();
		/* USER CODE BEGIN CRC_MspDeInit 1 */

		/* USER CODE END CRC_MspDeInit 1 */
	}
}

/**
 * @brief I2S MSP Initialization
 * This function configures the hardware resources used in this example
 * @param hi2s: I2S handle pointer
 * @retval None
 */
void HAL_I2S_MspInit(I2S_HandleTypeDef *hi2s) {
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
	if (hi2s->Instance == SPI3) {
		/* USER CODE BEGIN SPI3_MspInit 0 */

		/* USER CODE END SPI3_MspInit 0 */

		/** Initializes the peripherals clock
		 */
		PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SPI3;
		PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PIN;
		if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
			Error_Handler();
		}

		/* Peripheral clock enable */
		__HAL_RCC_SPI3_CLK_ENABLE();

		__HAL_RCC_GPIOA_CLK_ENABLE();
		__HAL_RCC_GPIOC_CLK_ENABLE();
		/**I2S3 GPIO Configuration
		PA15 (JTDI)     ------> I2S3_WS
		PC10     ------> I2S3_CK
		PC11     ------> I2S3_SDI
		PC12     ------> I2S3_SDO
		*/
		GPIO_InitStruct.Pin = WM8731_WS_LRC_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
		HAL_GPIO_Init(WM8731_WS_LRC_GPIO_Port, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = WM8731_BCLK_Pin | WM8731_ADC_SD_Pin | WM8731_DAC_SD_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

		/* USER CODE BEGIN SPI3_MspInit 1 */

		/* I2S3 DMA Init */

		// SPI3_TX Init
		hdma_spi3_tx.Instance = DMA1_Stream5;
		hdma_spi3_tx.Init.Request = DMA_REQUEST_SPI3_TX;
		hdma_spi3_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
		hdma_spi3_tx.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_spi3_tx.Init.MemInc = DMA_MINC_ENABLE;
		hdma_spi3_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
		hdma_spi3_tx.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
		hdma_spi3_tx.Init.Mode = DMA_CIRCULAR;
		hdma_spi3_tx.Init.Priority = DMA_PRIORITY_VERY_HIGH;
		hdma_spi3_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
		if (HAL_DMA_Init(&hdma_spi3_tx) != HAL_OK) {
			Error_Handler();
		}
		HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 1, 0);
		HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
		__HAL_LINKDMA(hi2s, hdmatx, hdma_spi3_tx);

		// SPI3_RX Init
		hdma_spi3_rx.Instance = DMA1_Stream0;
		hdma_spi3_rx.Init.Request = DMA_REQUEST_SPI3_RX;
		hdma_spi3_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
		hdma_spi3_rx.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_spi3_rx.Init.MemInc = DMA_MINC_ENABLE;
		hdma_spi3_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
		hdma_spi3_rx.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
		hdma_spi3_rx.Init.Mode = DMA_CIRCULAR;
		hdma_spi3_rx.Init.Priority = DMA_PRIORITY_VERY_HIGH;
		hdma_spi3_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
		if (HAL_DMA_Init(&hdma_spi3_rx) != HAL_OK) {
			Error_Handler();
		}
		HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 1, 0);
		HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);
		__HAL_LINKDMA(hi2s, hdmarx, hdma_spi3_rx);

		/* USER CODE END SPI3_MspInit 1 */
	}
}

/**
 * @brief I2S MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hi2s: I2S handle pointer
 * @retval None
 */
void HAL_I2S_MspDeInit(I2S_HandleTypeDef *hi2s) {
	if (hi2s->Instance == SPI3) {
		/* USER CODE BEGIN SPI3_MspDeInit 0 */

		/* USER CODE END SPI3_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_SPI3_CLK_DISABLE();

		/**I2S3 GPIO Configuration
		PA15 (JTDI)     ------> I2S3_WS
		PC10     ------> I2S3_CK
		PC11     ------> I2S3_SDI
		PC12     ------> I2S3_SDO
		*/
		HAL_GPIO_DeInit(WM8731_WS_LRC_GPIO_Port, WM8731_WS_LRC_Pin);

		HAL_GPIO_DeInit(GPIOC, WM8731_BCLK_Pin | WM8731_ADC_SD_Pin | WM8731_DAC_SD_Pin);

		/* USER CODE BEGIN SPI3_MspDeInit 1 */
		HAL_DMA_DeInit(hi2s->hdmarx);
		/* USER CODE END SPI3_MspDeInit 1 */
	}
}

/**
 * @brief RTC MSP Initialization
 * This function configures the hardware resources used in this example
 * @param hrtc: RTC handle pointer
 * @retval None
 */
void HAL_RTC_MspInit(RTC_HandleTypeDef *hrtc) {
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
	if (hrtc->Instance == RTC) {
		/* USER CODE BEGIN RTC_MspInit 0 */

		/* USER CODE END RTC_MspInit 0 */

		/** Initializes the peripherals clock
		 */
		PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
		PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
		if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
			Error_Handler();
		}

		/* Peripheral clock enable */
		__HAL_RCC_RTC_ENABLE();
		/* USER CODE BEGIN RTC_MspInit 1 */

		/* USER CODE END RTC_MspInit 1 */
	}
}

/**
 * @brief RTC MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hrtc: RTC handle pointer
 * @retval None
 */
void HAL_RTC_MspDeInit(RTC_HandleTypeDef *hrtc) {
	if (hrtc->Instance == RTC) {
		/* USER CODE BEGIN RTC_MspDeInit 0 */

		/* USER CODE END RTC_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_RTC_DISABLE();
		/* USER CODE BEGIN RTC_MspDeInit 1 */

		/* USER CODE END RTC_MspDeInit 1 */
	}
}

/**
 * @brief SPI MSP Initialization
 * This function configures the hardware resources used in this example
 * @param hspi: SPI handle pointer
 * @retval None
 */
void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi) {
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
	if (hspi->Instance == SPI2) {
		/* USER CODE BEGIN SPI2_MspInit 0 */

		/* USER CODE END SPI2_MspInit 0 */

		/** Initializes the peripherals clock
		 */
		PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SPI2;
		PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PIN;
		if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
			Error_Handler();
		}

		/* Peripheral clock enable */
		__HAL_RCC_SPI2_CLK_ENABLE();

		__HAL_RCC_GPIOB_CLK_ENABLE();
		/**SPI2 GPIO Configuration
		PB13     ------> SPI2_SCK
		PB14     ------> SPI2_MISO
		PB15     ------> SPI2_MOSI
		*/
		GPIO_InitStruct.Pin = PERI_SCK_Pin | PERI_MISO_Pin | PERI_MOSI_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		/* SPI2 DMA Init */
		/* SPI2_RX Init */
		hdma_spi2_rx.Instance = DMA1_Stream2;
		hdma_spi2_rx.Init.Request = DMA_REQUEST_SPI2_RX;
		hdma_spi2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
		hdma_spi2_rx.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_spi2_rx.Init.MemInc = DMA_MINC_ENABLE;
		hdma_spi2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
		hdma_spi2_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
		hdma_spi2_rx.Init.Mode = DMA_NORMAL;
		hdma_spi2_rx.Init.Priority = DMA_PRIORITY_MEDIUM;
		hdma_spi2_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
		if (HAL_DMA_Init(&hdma_spi2_rx) != HAL_OK) {
			Error_Handler();
		}

		__HAL_LINKDMA(hspi, hdmarx, hdma_spi2_rx);

		/* SPI2_TX Init */
		hdma_spi2_tx.Instance = DMA1_Stream3;
		hdma_spi2_tx.Init.Request = DMA_REQUEST_SPI2_TX;
		hdma_spi2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
		hdma_spi2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_spi2_tx.Init.MemInc = DMA_MINC_ENABLE;
		hdma_spi2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
		hdma_spi2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
		hdma_spi2_tx.Init.Mode = DMA_NORMAL;
		hdma_spi2_tx.Init.Priority = DMA_PRIORITY_MEDIUM;
		hdma_spi2_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
		if (HAL_DMA_Init(&hdma_spi2_tx) != HAL_OK) {
			Error_Handler();
		}

		__HAL_LINKDMA(hspi, hdmatx, hdma_spi2_tx);

		/* SPI2 interrupt Init */
		HAL_NVIC_SetPriority(SPI2_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(SPI2_IRQn);
		/* USER CODE BEGIN SPI2_MspInit 1 */

		/* USER CODE END SPI2_MspInit 1 */
	} else if (hspi->Instance == SPI4) {
		/* USER CODE BEGIN SPI4_MspInit 0 */

		/* USER CODE END SPI4_MspInit 0 */

		/** Initializes the peripherals clock
		 */
		PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SPI4;
		PeriphClkInitStruct.Spi45ClockSelection = RCC_SPI45CLKSOURCE_D2PCLK1;
		if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
			Error_Handler();
		}

		/* Peripheral clock enable */
		__HAL_RCC_SPI4_CLK_ENABLE();

		__HAL_RCC_GPIOE_CLK_ENABLE();
		/**SPI4 GPIO Configuration
		PE11     ------> SPI4_NSS
		PE12     ------> SPI4_SCK
		PE14     ------> SPI4_MOSI
		*/
		GPIO_InitStruct.Pin = LCD_CS_Pin | LCD_SCK_Pin | LCD_MOSI_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		GPIO_InitStruct.Alternate = GPIO_AF5_SPI4;
		HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

		/* SPI4 DMA Init */
		/* SPI4_TX Init */
		hdma_spi4_tx.Instance = DMA2_Stream5;
		hdma_spi4_tx.Init.Request = DMA_REQUEST_SPI4_TX;
		hdma_spi4_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
		hdma_spi4_tx.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_spi4_tx.Init.MemInc = DMA_MINC_ENABLE;
		hdma_spi4_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
		hdma_spi4_tx.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
		hdma_spi4_tx.Init.Mode = DMA_NORMAL;
		hdma_spi4_tx.Init.Priority = DMA_PRIORITY_LOW;
		hdma_spi4_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
		if (HAL_DMA_Init(&hdma_spi4_tx) != HAL_OK) {
			Error_Handler();
		}

		__HAL_LINKDMA(hspi, hdmatx, hdma_spi4_tx);

		/* SPI4 interrupt Init */
		HAL_NVIC_SetPriority(SPI4_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(SPI4_IRQn);
		/* USER CODE BEGIN SPI4_MspInit 1 */

		/* USER CODE END SPI4_MspInit 1 */
	}
}

/**
 * @brief SPI MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hspi: SPI handle pointer
 * @retval None
 */
void HAL_SPI_MspDeInit(SPI_HandleTypeDef *hspi) {
	if (hspi->Instance == SPI2) {
		/* USER CODE BEGIN SPI2_MspDeInit 0 */

		/* USER CODE END SPI2_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_SPI2_CLK_DISABLE();

		/**SPI2 GPIO Configuration
		PB13     ------> SPI2_SCK
		PB14     ------> SPI2_MISO
		PB15     ------> SPI2_MOSI
		*/
		HAL_GPIO_DeInit(GPIOB, PERI_SCK_Pin | PERI_MISO_Pin | PERI_MOSI_Pin);

		/* SPI2 DMA DeInit */
		HAL_DMA_DeInit(hspi->hdmarx);
		HAL_DMA_DeInit(hspi->hdmatx);

		/* SPI2 interrupt DeInit */
		HAL_NVIC_DisableIRQ(SPI2_IRQn);
		/* USER CODE BEGIN SPI2_MspDeInit 1 */

		/* USER CODE END SPI2_MspDeInit 1 */
	} else if (hspi->Instance == SPI4) {
		/* USER CODE BEGIN SPI4_MspDeInit 0 */

		/* USER CODE END SPI4_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_SPI4_CLK_DISABLE();

		/**SPI4 GPIO Configuration
		PE11     ------> SPI4_NSS
		PE12     ------> SPI4_SCK
		PE14     ------> SPI4_MOSI
		*/
		HAL_GPIO_DeInit(GPIOE, LCD_CS_Pin | LCD_SCK_Pin | LCD_MOSI_Pin);

		/* SPI4 DMA DeInit */
		HAL_DMA_DeInit(hspi->hdmatx);

		/* SPI4 interrupt DeInit */
		HAL_NVIC_DisableIRQ(SPI4_IRQn);
		/* USER CODE BEGIN SPI4_MspDeInit 1 */

		/* USER CODE END SPI4_MspDeInit 1 */
	}
}

/**
 * @brief TIM_Base MSP Initialization
 * This function configures the hardware resources used in this example
 * @param htim_base: TIM_Base handle pointer
 * @retval None
 */
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim_base) {
	if (htim_base->Instance == TIM2) {
		/* USER CODE BEGIN TIM2_MspInit 0 */

		/* USER CODE END TIM2_MspInit 0 */
		/* Peripheral clock enable */
		__HAL_RCC_TIM2_CLK_ENABLE();
		/* TIM2 interrupt Init */
		HAL_NVIC_SetPriority(TIM2_IRQn, 1, 0);
		HAL_NVIC_EnableIRQ(TIM2_IRQn);
		/* USER CODE BEGIN TIM2_MspInit 1 */

		/* USER CODE END TIM2_MspInit 1 */
	} else if (htim_base->Instance == TIM3) {
		/* USER CODE BEGIN TIM3_MspInit 0 */

		/* USER CODE END TIM3_MspInit 0 */
		/* Peripheral clock enable */
		__HAL_RCC_TIM3_CLK_ENABLE();
		/* TIM3 interrupt Init */
		HAL_NVIC_SetPriority(TIM3_IRQn, 12, 0);
		HAL_NVIC_EnableIRQ(TIM3_IRQn);
		/* USER CODE BEGIN TIM3_MspInit 1 */

		/* USER CODE END TIM3_MspInit 1 */
	} else if (htim_base->Instance == TIM4) {
		/* USER CODE BEGIN TIM4_MspInit 0 */

		/* USER CODE END TIM4_MspInit 0 */
		/* Peripheral clock enable */
		__HAL_RCC_TIM4_CLK_ENABLE();
		/* TIM4 interrupt Init */
		HAL_NVIC_SetPriority(TIM4_IRQn, 9, 0);
		HAL_NVIC_EnableIRQ(TIM4_IRQn);
		/* USER CODE BEGIN TIM4_MspInit 1 */

		/* USER CODE END TIM4_MspInit 1 */
	} else if (htim_base->Instance == TIM5) {
		/* USER CODE BEGIN TIM5_MspInit 0 */

		/* USER CODE END TIM5_MspInit 0 */
		/* Peripheral clock enable */
		__HAL_RCC_TIM5_CLK_ENABLE();
		/* TIM5 interrupt Init */
		HAL_NVIC_SetPriority(TIM5_IRQn, 5, 0);
		HAL_NVIC_EnableIRQ(TIM5_IRQn);
		/* USER CODE BEGIN TIM5_MspInit 1 */

		/* USER CODE END TIM5_MspInit 1 */
	} else if (htim_base->Instance == TIM6) {
		/* USER CODE BEGIN TIM6_MspInit 0 */

		/* USER CODE END TIM6_MspInit 0 */
		/* Peripheral clock enable */
		__HAL_RCC_TIM6_CLK_ENABLE();
		/* TIM6 interrupt Init */
		HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 8, 0);
		HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
		/* USER CODE BEGIN TIM6_MspInit 1 */

		/* USER CODE END TIM6_MspInit 1 */
	} else if (htim_base->Instance == TIM7) {
		/* USER CODE BEGIN TIM7_MspInit 0 */

		/* USER CODE END TIM7_MspInit 0 */
		/* Peripheral clock enable */
		__HAL_RCC_TIM7_CLK_ENABLE();
		/* TIM7 interrupt Init */
		HAL_NVIC_SetPriority(TIM7_IRQn, 10, 0);
		HAL_NVIC_EnableIRQ(TIM7_IRQn);
		/* USER CODE BEGIN TIM7_MspInit 1 */

		/* USER CODE END TIM7_MspInit 1 */
	} else if (htim_base->Instance == TIM15) {
		/* USER CODE BEGIN TIM15_MspInit 0 */

		/* USER CODE END TIM15_MspInit 0 */
		/* Peripheral clock enable */
		__HAL_RCC_TIM15_CLK_ENABLE();
		/* TIM15 interrupt Init */
		HAL_NVIC_SetPriority(TIM15_IRQn, 11, 0);
		HAL_NVIC_EnableIRQ(TIM15_IRQn);
		/* USER CODE BEGIN TIM15_MspInit 1 */

		/* USER CODE END TIM15_MspInit 1 */
	} else if (htim_base->Instance == TIM16) {
		/* USER CODE BEGIN TIM16_MspInit 0 */

		/* USER CODE END TIM16_MspInit 0 */
		/* Peripheral clock enable */
		__HAL_RCC_TIM16_CLK_ENABLE();
		/* TIM16 interrupt Init */
		HAL_NVIC_SetPriority(TIM16_IRQn, 2, 0);
		HAL_NVIC_EnableIRQ(TIM16_IRQn);
		/* USER CODE BEGIN TIM16_MspInit 1 */

		/* USER CODE END TIM16_MspInit 1 */
	} else if (htim_base->Instance == TIM17) {
		/* USER CODE BEGIN TIM17_MspInit 0 */

		/* USER CODE END TIM17_MspInit 0 */
		/* Peripheral clock enable */
		__HAL_RCC_TIM17_CLK_ENABLE();
		/* TIM17 interrupt Init */
		HAL_NVIC_SetPriority(TIM17_IRQn, 4, 0);
		HAL_NVIC_EnableIRQ(TIM17_IRQn);
		/* USER CODE BEGIN TIM17_MspInit 1 */

		/* USER CODE END TIM17_MspInit 1 */
	}
}

/**
 * @brief TIM_Base MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param htim_base: TIM_Base handle pointer
 * @retval None
 */
void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef *htim_base) {
	if (htim_base->Instance == TIM2) {
		/* USER CODE BEGIN TIM2_MspDeInit 0 */

		/* USER CODE END TIM2_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_TIM2_CLK_DISABLE();

		/* TIM2 interrupt DeInit */
		HAL_NVIC_DisableIRQ(TIM2_IRQn);
		/* USER CODE BEGIN TIM2_MspDeInit 1 */

		/* USER CODE END TIM2_MspDeInit 1 */
	} else if (htim_base->Instance == TIM3) {
		/* USER CODE BEGIN TIM3_MspDeInit 0 */

		/* USER CODE END TIM3_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_TIM3_CLK_DISABLE();

		/* TIM3 interrupt DeInit */
		HAL_NVIC_DisableIRQ(TIM3_IRQn);
		/* USER CODE BEGIN TIM3_MspDeInit 1 */

		/* USER CODE END TIM3_MspDeInit 1 */
	} else if (htim_base->Instance == TIM4) {
		/* USER CODE BEGIN TIM4_MspDeInit 0 */

		/* USER CODE END TIM4_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_TIM4_CLK_DISABLE();

		/* TIM4 interrupt DeInit */
		HAL_NVIC_DisableIRQ(TIM4_IRQn);
		/* USER CODE BEGIN TIM4_MspDeInit 1 */

		/* USER CODE END TIM4_MspDeInit 1 */
	} else if (htim_base->Instance == TIM5) {
		/* USER CODE BEGIN TIM5_MspDeInit 0 */

		/* USER CODE END TIM5_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_TIM5_CLK_DISABLE();

		/* TIM5 interrupt DeInit */
		HAL_NVIC_DisableIRQ(TIM5_IRQn);
		/* USER CODE BEGIN TIM5_MspDeInit 1 */

		/* USER CODE END TIM5_MspDeInit 1 */
	} else if (htim_base->Instance == TIM6) {
		/* USER CODE BEGIN TIM6_MspDeInit 0 */

		/* USER CODE END TIM6_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_TIM6_CLK_DISABLE();

		/* TIM6 interrupt DeInit */
		HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);
		/* USER CODE BEGIN TIM6_MspDeInit 1 */

		/* USER CODE END TIM6_MspDeInit 1 */
	} else if (htim_base->Instance == TIM7) {
		/* USER CODE BEGIN TIM7_MspDeInit 0 */

		/* USER CODE END TIM7_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_TIM7_CLK_DISABLE();

		/* TIM7 interrupt DeInit */
		HAL_NVIC_DisableIRQ(TIM7_IRQn);
		/* USER CODE BEGIN TIM7_MspDeInit 1 */

		/* USER CODE END TIM7_MspDeInit 1 */
	} else if (htim_base->Instance == TIM15) {
		/* USER CODE BEGIN TIM15_MspDeInit 0 */

		/* USER CODE END TIM15_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_TIM15_CLK_DISABLE();

		/* TIM15 interrupt DeInit */
		HAL_NVIC_DisableIRQ(TIM15_IRQn);
		/* USER CODE BEGIN TIM15_MspDeInit 1 */

		/* USER CODE END TIM15_MspDeInit 1 */
	} else if (htim_base->Instance == TIM16) {
		/* USER CODE BEGIN TIM16_MspDeInit 0 */

		/* USER CODE END TIM16_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_TIM16_CLK_DISABLE();

		/* TIM16 interrupt DeInit */
		HAL_NVIC_DisableIRQ(TIM16_IRQn);
		/* USER CODE BEGIN TIM16_MspDeInit 1 */

		/* USER CODE END TIM16_MspDeInit 1 */
	} else if (htim_base->Instance == TIM17) {
		/* USER CODE BEGIN TIM17_MspDeInit 0 */

		/* USER CODE END TIM17_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_TIM17_CLK_DISABLE();

		/* TIM17 interrupt DeInit */
		HAL_NVIC_DisableIRQ(TIM17_IRQn);
		/* USER CODE BEGIN TIM17_MspDeInit 1 */

		/* USER CODE END TIM17_MspDeInit 1 */
	}
}

/**
 * @brief UART MSP Initialization
 * This function configures the hardware resources used in this example
 * @param huart: UART handle pointer
 * @retval None
 */
void HAL_UART_MspInit(UART_HandleTypeDef *huart) {
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
	if (huart->Instance == USART6) {
		/* USER CODE BEGIN USART6_MspInit 0 */

		/* USER CODE END USART6_MspInit 0 */

		/** Initializes the peripherals clock
		 */
		PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART6;
		PeriphClkInitStruct.Usart16ClockSelection = RCC_USART16CLKSOURCE_D2PCLK2;
		if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
			Error_Handler();
		}

		/* Peripheral clock enable */
		__HAL_RCC_USART6_CLK_ENABLE();

		__HAL_RCC_GPIOC_CLK_ENABLE();
		/**USART6 GPIO Configuration
		PC6     ------> USART6_TX
		PC7     ------> USART6_RX
		*/
		GPIO_InitStruct.Pin = WIFI_TX_Pin | WIFI_RX_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		GPIO_InitStruct.Alternate = GPIO_AF7_USART6;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

		/* USART6 DMA Init */
		/* USART6_RX Init */
		hdma_usart6_rx.Instance = DMA1_Stream1;
		hdma_usart6_rx.Init.Request = DMA_REQUEST_USART6_RX;
		hdma_usart6_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
		hdma_usart6_rx.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_usart6_rx.Init.MemInc = DMA_MINC_ENABLE;
		hdma_usart6_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
		hdma_usart6_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
		hdma_usart6_rx.Init.Mode = DMA_CIRCULAR;
		hdma_usart6_rx.Init.Priority = DMA_PRIORITY_HIGH;
		hdma_usart6_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
		if (HAL_DMA_Init(&hdma_usart6_rx) != HAL_OK) {
			Error_Handler();
		}

		__HAL_LINKDMA(huart, hdmarx, hdma_usart6_rx);

		/* USART6 interrupt Init */
		HAL_NVIC_SetPriority(USART6_IRQn, 1, 0);
		HAL_NVIC_EnableIRQ(USART6_IRQn);
		/* USER CODE BEGIN USART6_MspInit 1 */

		/* USER CODE END USART6_MspInit 1 */
	}
}

/**
 * @brief UART MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param huart: UART handle pointer
 * @retval None
 */
void HAL_UART_MspDeInit(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART6) {
		/* USER CODE BEGIN USART6_MspDeInit 0 */

		/* USER CODE END USART6_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_USART6_CLK_DISABLE();

		/**USART6 GPIO Configuration
		PC6     ------> USART6_TX
		PC7     ------> USART6_RX
		*/
		HAL_GPIO_DeInit(GPIOC, WIFI_TX_Pin | WIFI_RX_Pin);

		/* USART6 DMA DeInit */
		HAL_DMA_DeInit(huart->hdmarx);

		/* USART6 interrupt DeInit */
		HAL_NVIC_DisableIRQ(USART6_IRQn);
		/* USER CODE BEGIN USART6_MspDeInit 1 */

		/* USER CODE END USART6_MspDeInit 1 */
	}
}

/**
 * @brief PCD MSP Initialization
 * This function configures the hardware resources used in this example
 * @param hpcd: PCD handle pointer
 * @retval None
 */
void HAL_PCD_MspInit(PCD_HandleTypeDef *hpcd) {
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
	if (hpcd->Instance == USB_OTG_FS) {
		/* USER CODE BEGIN USB_OTG_FS_MspInit 0 */

		/* USER CODE END USB_OTG_FS_MspInit 0 */

		/** Initializes the peripherals clock
		 */
		PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USB;
		PeriphClkInitStruct.PLL3.PLL3M = 4;
		PeriphClkInitStruct.PLL3.PLL3N = 96;
		PeriphClkInitStruct.PLL3.PLL3P = 2;
		PeriphClkInitStruct.PLL3.PLL3Q = 4;
		PeriphClkInitStruct.PLL3.PLL3R = 2;
		PeriphClkInitStruct.PLL3.PLL3RGE = RCC_PLL3VCIRANGE_1;
		PeriphClkInitStruct.PLL3.PLL3FRACN = 0;
		PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_PLL3;
		if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
			Error_Handler();
		}

		/** Enable USB Voltage detector
		 */
		HAL_PWREx_EnableUSBVoltageDetector();

		__HAL_RCC_GPIOA_CLK_ENABLE();
		/**USB_OTG_FS GPIO Configuration
		PA11     ------> USB_OTG_FS_DM
		PA12     ------> USB_OTG_FS_DP
		*/
		GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_12;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		GPIO_InitStruct.Alternate = GPIO_AF10_OTG1_FS;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		/* Peripheral clock enable */
		__HAL_RCC_USB_OTG_FS_CLK_ENABLE();
		/* USB_OTG_FS interrupt Init */
		HAL_NVIC_SetPriority(OTG_FS_IRQn, 1, 0);
		HAL_NVIC_EnableIRQ(OTG_FS_IRQn);
		/* USER CODE BEGIN USB_OTG_FS_MspInit 1 */

		// stop PCD, run CLK, set SLEEP, disable ULPI
		__HAL_PCD_DISABLE(hpcd);
		__HAL_RCC_USB1_OTG_HS_CLK_DISABLE();
		__HAL_RCC_USB1_OTG_HS_CLK_SLEEP_ENABLE();
		__HAL_RCC_USB1_OTG_HS_ULPI_CLK_SLEEP_DISABLE();
		__HAL_RCC_USB1_OTG_HS_ULPI_CLK_DISABLE();

		__HAL_RCC_USB2_OTG_FS_CLK_ENABLE();
		__HAL_RCC_USB2_OTG_FS_CLK_SLEEP_ENABLE();
		__HAL_RCC_USB2_OTG_FS_ULPI_CLK_SLEEP_DISABLE();
		__HAL_RCC_USB2_OTG_FS_ULPI_CLK_DISABLE();
		HAL_Delay(10);
		// Repeat disabling PCD (else may be error in USB_CoreReset)
		__HAL_PCD_DISABLE(hpcd);
		hpcd->Instance->GUSBCFG |= USB_OTG_GUSBCFG_PHYSEL;
		__HAL_RCC_USB2_OTG_FS_ULPI_CLK_DISABLE();
		HAL_Delay(10);

		/* USER CODE END USB_OTG_FS_MspInit 1 */
	}
}

/**
 * @brief PCD MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hpcd: PCD handle pointer
 * @retval None
 */
void HAL_PCD_MspDeInit(PCD_HandleTypeDef *hpcd) {
	if (hpcd->Instance == USB_OTG_FS) {
		/* USER CODE BEGIN USB_OTG_FS_MspDeInit 0 */

		/* USER CODE END USB_OTG_FS_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_USB_OTG_FS_CLK_DISABLE();

		/**USB_OTG_FS GPIO Configuration
		PA11     ------> USB_OTG_FS_DM
		PA12     ------> USB_OTG_FS_DP
		*/
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11 | GPIO_PIN_12);

		/* USB_OTG_FS interrupt DeInit */
		HAL_NVIC_DisableIRQ(OTG_FS_IRQn);
		/* USER CODE BEGIN USB_OTG_FS_MspDeInit 1 */

		/* USER CODE END USB_OTG_FS_MspDeInit 1 */
	}
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
