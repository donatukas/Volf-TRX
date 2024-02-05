/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
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
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern ADC_HandleTypeDef hadc3;

extern I2S_HandleTypeDef hi2s3;
extern DMA_HandleTypeDef hdma_spi3_tx;
extern DMA_HandleTypeDef hdma_i2s3_ext_rx;

extern RTC_HandleTypeDef hrtc;
extern IWDG_HandleTypeDef hiwdg;
extern SPI_HandleTypeDef hspi2;

extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim5;
extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim7;
extern TIM_HandleTypeDef htim8;

extern PCD_HandleTypeDef hpcd_USB_OTG_FS;

extern DMA_HandleTypeDef hdma_memtomem_dma2_stream0;
extern DMA_HandleTypeDef hdma_memtomem_dma2_stream1;
extern DMA_HandleTypeDef hdma_memtomem_dma2_stream2;
extern DMA_HandleTypeDef hdma_memtomem_dma2_stream4;
extern DMA_HandleTypeDef hdma_memtomem_dma2_stream5;
extern DMA_HandleTypeDef hdma_memtomem_dma2_stream7;
extern SRAM_HandleTypeDef hsram1;
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define ENC_CLK_Pin GPIO_PIN_3
#define ENC_CLK_GPIO_Port GPIOE
#define ENC_CLK_EXTI_IRQn EXTI3_IRQn
#define ENC2_SW_Pin GPIO_PIN_4
#define ENC2_SW_GPIO_Port GPIOE
#define ENC_DT_Pin GPIO_PIN_5
#define ENC_DT_GPIO_Port GPIOE
#define ENC2_DT_Pin GPIO_PIN_6
#define ENC2_DT_GPIO_Port GPIOE
#define ENC2_CLK_Pin GPIO_PIN_13
#define ENC2_CLK_GPIO_Port GPIOC
#define SWR_FORW_Pin GPIO_PIN_0
#define SWR_FORW_GPIO_Port GPIOC
#define SWR_BACKW_Pin GPIO_PIN_1
#define SWR_BACKW_GPIO_Port GPIOC
#define FPGA_CLK_Pin GPIO_PIN_2
#define FPGA_CLK_GPIO_Port GPIOC
#define FPGA_SYNC_Pin GPIO_PIN_3
#define FPGA_SYNC_GPIO_Port GPIOC
#define FPGA_BUS_D0_Pin GPIO_PIN_0
#define FPGA_BUS_D0_GPIO_Port GPIOA
#define FPGA_BUS_D1_Pin GPIO_PIN_1
#define FPGA_BUS_D1_GPIO_Port GPIOA
#define FPGA_BUS_D2_Pin GPIO_PIN_2
#define FPGA_BUS_D2_GPIO_Port GPIOA
#define FPGA_BUS_D3_Pin GPIO_PIN_3
#define FPGA_BUS_D3_GPIO_Port GPIOA
#define FPGA_BUS_D4_Pin GPIO_PIN_4
#define FPGA_BUS_D4_GPIO_Port GPIOA
#define FPGA_BUS_D5_Pin GPIO_PIN_5
#define FPGA_BUS_D5_GPIO_Port GPIOA
#define FPGA_BUS_D6_Pin GPIO_PIN_6
#define FPGA_BUS_D6_GPIO_Port GPIOA
#define FPGA_BUS_D7_Pin GPIO_PIN_7
#define FPGA_BUS_D7_GPIO_Port GPIOA
#define Power_IN_Pin GPIO_PIN_4
#define Power_IN_GPIO_Port GPIOC
#define ALC_IN_Pin GPIO_PIN_5
#define ALC_IN_GPIO_Port GPIOC
#define PTT_SW1_Pin GPIO_PIN_0
#define PTT_SW1_GPIO_Port GPIOB
#define PTT_SW2_Pin GPIO_PIN_1
#define PTT_SW2_GPIO_Port GPIOB
#define PTT_IN_Pin GPIO_PIN_2
#define PTT_IN_GPIO_Port GPIOB
#define PTT_IN_EXTI_IRQn EXTI2_IRQn
#define AUDIO_48K_CLOCK_Pin GPIO_PIN_10
#define AUDIO_48K_CLOCK_GPIO_Port GPIOB
#define AUDIO_48K_CLOCK_EXTI_IRQn EXTI15_10_IRQn
#define W25Q16_CS_Pin GPIO_PIN_12
#define W25Q16_CS_GPIO_Port GPIOB
#define PERI_SCK_Pin GPIO_PIN_13
#define PERI_SCK_GPIO_Port GPIOB
#define PERI_MISO_Pin GPIO_PIN_14
#define PERI_MISO_GPIO_Port GPIOB
#define PERI_MOSI_Pin GPIO_PIN_15
#define PERI_MOSI_GPIO_Port GPIOB
#define AF_AMP_MUTE_Pin GPIO_PIN_7
#define AF_AMP_MUTE_GPIO_Port GPIOC
#define AD1_CS_Pin GPIO_PIN_8
#define AD1_CS_GPIO_Port GPIOC
#define WM8731_WS_LRC_Pin GPIO_PIN_15
#define WM8731_WS_LRC_GPIO_Port GPIOA
#define WM8731_BCLK_Pin GPIO_PIN_10
#define WM8731_BCLK_GPIO_Port GPIOC
#define WM8731_ADC_SD_Pin GPIO_PIN_11
#define WM8731_ADC_SD_GPIO_Port GPIOC
#define WM8731_DAC_SD_Pin GPIO_PIN_12
#define WM8731_DAC_SD_GPIO_Port GPIOC
#define WM8731_SCK_Pin GPIO_PIN_3
#define WM8731_SCK_GPIO_Port GPIOD
#define WM8731_SDA_Pin GPIO_PIN_6
#define WM8731_SDA_GPIO_Port GPIOD
#define PWR_HOLD_Pin GPIO_PIN_6
#define PWR_HOLD_GPIO_Port GPIOB
#define PWR_ON_Pin GPIO_PIN_7
#define PWR_ON_GPIO_Port GPIOB
#define LCD_BL_PWM_Pin GPIO_PIN_8
#define LCD_BL_PWM_GPIO_Port GPIOB
#define KEY_IN_DASH_Pin GPIO_PIN_0
#define KEY_IN_DASH_GPIO_Port GPIOE
#define KEY_IN_DASH_EXTI_IRQn EXTI0_IRQn
#define KEY_IN_DOT_Pin GPIO_PIN_1
#define KEY_IN_DOT_GPIO_Port GPIOE
#define KEY_IN_DOT_EXTI_IRQn EXTI1_IRQn

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
