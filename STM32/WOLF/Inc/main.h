/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc3;

extern DMA2D_HandleTypeDef hdma2d;

extern I2S_HandleTypeDef hi2s3;
extern DMA_HandleTypeDef hdma_spi3_rx;
extern DMA_HandleTypeDef hdma_spi3_tx;
extern DMA_HandleTypeDef hdma_spi2_rx;
extern DMA_HandleTypeDef hdma_spi2_tx;

extern RTC_HandleTypeDef hrtc;
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

extern UART_HandleTypeDef huart6;
extern DMA_HandleTypeDef hdma_usart6_rx;

extern PCD_HandleTypeDef hpcd_USB_OTG_FS;

extern DMA_HandleTypeDef hdma_memtomem_dma2_stream5;
extern MDMA_HandleTypeDef hmdma_mdma_channel0_sw_0;
extern MDMA_HandleTypeDef hmdma_mdma_channel1_sw_0;
extern MDMA_HandleTypeDef hmdma_mdma_channel2_sw_0;
extern MDMA_HandleTypeDef hmdma_mdma_channel3_sw_0;
extern MDMA_HandleTypeDef hmdma_mdma_channel4_sw_0;
extern SRAM_HandleTypeDef hsram1;

extern CRC_HandleTypeDef hcrc;
extern IWDG_HandleTypeDef hiwdg1;
extern JPEG_HandleTypeDef hjpeg;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define ENC_CLK_Pin GPIO_PIN_2
#define ENC_CLK_GPIO_Port GPIOE
#define ENC_CLK_EXTI_IRQn EXTI2_IRQn
#define ENC2SW_AND_TOUCHPAD_Pin GPIO_PIN_4
#define ENC2SW_AND_TOUCHPAD_GPIO_Port GPIOE
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
#define PTT_IN_Pin GPIO_PIN_4
#define PTT_IN_GPIO_Port GPIOC
#define PTT_IN_EXTI_IRQn EXTI4_IRQn
#define RFUNT_THERM_Pin GPIO_PIN_5
#define RFUNT_THERM_GPIO_Port GPIOC
#define W25Q16_CS_Pin GPIO_PIN_0
#define W25Q16_CS_GPIO_Port GPIOB
#define RFUNT_ALC_Pin GPIO_PIN_1
#define RFUNT_ALC_GPIO_Port GPIOB
#define AUDIO_48K_CLOCK_Pin GPIO_PIN_10
#define AUDIO_48K_CLOCK_GPIO_Port GPIOB
#define AUDIO_48K_CLOCK_EXTI_IRQn EXTI15_10_IRQn
#define SD_CS_Pin GPIO_PIN_11
#define SD_CS_GPIO_Port GPIOB
#define AD3_CS_Pin GPIO_PIN_12
#define AD3_CS_GPIO_Port GPIOB
#define PERI_SCK_Pin GPIO_PIN_13
#define PERI_SCK_GPIO_Port GPIOB
#define PERI_MISO_Pin GPIO_PIN_14
#define PERI_MISO_GPIO_Port GPIOB
#define PERI_MOSI_Pin GPIO_PIN_15
#define PERI_MOSI_GPIO_Port GPIOB
#define PWR_ON_Pin GPIO_PIN_11
#define PWR_ON_GPIO_Port GPIOD
#define PWR_ON_EXTI_IRQn EXTI15_10_IRQn
#define PWR_HOLD_Pin GPIO_PIN_12
#define PWR_HOLD_GPIO_Port GPIOD
#define ESP_1_TX_Pin GPIO_PIN_6
#define ESP_1_TX_GPIO_Port GPIOC
#define ESP_1_RX_Pin GPIO_PIN_7
#define ESP_1_RX_GPIO_Port GPIOC
#define AD1_CS_Pin GPIO_PIN_8
#define AD1_CS_GPIO_Port GPIOC
#define AD2_CS_Pin GPIO_PIN_8
#define AD2_CS_GPIO_Port GPIOA
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
#define AF_AMP_MUTE_Pin GPIO_PIN_5
#define AF_AMP_MUTE_GPIO_Port GPIOB
#define RFUNIT_RCLK_Pin GPIO_PIN_6
#define RFUNIT_RCLK_GPIO_Port GPIOB
#define RFUNIT_CLK_Pin GPIO_PIN_7
#define RFUNIT_CLK_GPIO_Port GPIOB
#define RFUNIT_DATA_Pin GPIO_PIN_8
#define RFUNIT_DATA_GPIO_Port GPIOB
#define RFUNIT_OE_Pin GPIO_PIN_9
#define RFUNIT_OE_GPIO_Port GPIOB
#define KEY_IN_DASH_Pin GPIO_PIN_0
#define KEY_IN_DASH_GPIO_Port GPIOE
#define KEY_IN_DASH_EXTI_IRQn EXTI0_IRQn
#define KEY_IN_DOT_Pin GPIO_PIN_1
#define KEY_IN_DOT_GPIO_Port GPIOE
#define KEY_IN_DOT_EXTI_IRQn EXTI1_IRQn

/* USER CODE BEGIN Private defines */
#define T_INT_Pin ENC2SW_AND_TOUCHPAD_Pin
#define T_INT_GPIO_Port ENC2SW_AND_TOUCHPAD_GPIO_Port
#define T_I2C_SDA_Pin AD2_CS_Pin
#define T_I2C_SDA_GPIO_Port AD2_CS_GPIO_Port
#define T_I2C_SCL_Pin AD3_CS_Pin
#define T_I2C_SCL_GPIO_Port AD3_CS_GPIO_Port
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
