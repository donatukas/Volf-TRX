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
extern ADC_HandleTypeDef hadc2;
extern ADC_HandleTypeDef hadc3;

extern I2S_HandleTypeDef hi2s3;
extern DMA_HandleTypeDef hdma_spi2_rx;
extern DMA_HandleTypeDef hdma_spi2_tx;
extern DMA_HandleTypeDef hdma_spi3_rx;
extern DMA_HandleTypeDef hdma_spi3_tx;
extern DMA_HandleTypeDef hdma_spi4_tx;

extern RTC_HandleTypeDef hrtc;

extern SPI_HandleTypeDef hspi2;
extern SPI_HandleTypeDef hspi4;

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

extern MDMA_HandleTypeDef hmdma_mdma_channel0_sw_0;
extern MDMA_HandleTypeDef hmdma_mdma_channel1_sw_0;
extern MDMA_HandleTypeDef hmdma_mdma_channel2_sw_0;
extern MDMA_HandleTypeDef hmdma_mdma_channel3_sw_0;
extern MDMA_HandleTypeDef hmdma_mdma_channel4_sw_0;

extern CRC_HandleTypeDef hcrc;
extern IWDG_HandleTypeDef hiwdg1;
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
#define ENC_DT_Pin GPIO_PIN_5
#define ENC_DT_GPIO_Port GPIOE
#define KEY_6_Pin GPIO_PIN_13
#define KEY_6_GPIO_Port GPIOC
#define SWR_BACKW_Pin GPIO_PIN_0
#define SWR_BACKW_GPIO_Port GPIOC
#define SWR_FORW_Pin GPIO_PIN_1
#define SWR_FORW_GPIO_Port GPIOC
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
#define PWR_VOLTMETER_Pin GPIO_PIN_5
#define PWR_VOLTMETER_GPIO_Port GPIOC
#define TANG_SW1_Pin GPIO_PIN_0
#define TANG_SW1_GPIO_Port GPIOB
#define TANG_SW2_Pin GPIO_PIN_1
#define TANG_SW2_GPIO_Port GPIOB
#define W25Q16_CS_Pin GPIO_PIN_2
#define W25Q16_CS_GPIO_Port GPIOB
#define PTT_IN_Pin GPIO_PIN_7
#define PTT_IN_GPIO_Port GPIOE
#define PTT_IN_EXTI_IRQn EXTI9_5_IRQn
#define KEY_4_Pin GPIO_PIN_8
#define KEY_4_GPIO_Port GPIOE
#define LCD_RST_Pin GPIO_PIN_9
#define LCD_RST_GPIO_Port GPIOE
#define LCD_DC_Pin GPIO_PIN_10
#define LCD_DC_GPIO_Port GPIOE
#define LCD_CS_Pin GPIO_PIN_11
#define LCD_CS_GPIO_Port GPIOE
#define LCD_SCK_Pin GPIO_PIN_12
#define LCD_SCK_GPIO_Port GPIOE
#define LCD_MISO_Pin GPIO_PIN_13
#define LCD_MISO_GPIO_Port GPIOE
#define LCD_MOSI_Pin GPIO_PIN_14
#define LCD_MOSI_GPIO_Port GPIOE
#define KEY_3_Pin GPIO_PIN_15
#define KEY_3_GPIO_Port GPIOE
#define AUDIO_48K_CLOCK_Pin GPIO_PIN_10
#define AUDIO_48K_CLOCK_GPIO_Port GPIOB
#define AUDIO_48K_CLOCK_EXTI_IRQn EXTI15_10_IRQn
#define SD_CS_Pin GPIO_PIN_11
#define SD_CS_GPIO_Port GPIOB
#define PERI_SCK_Pin GPIO_PIN_13
#define PERI_SCK_GPIO_Port GPIOB
#define PERI_MISO_Pin GPIO_PIN_14
#define PERI_MISO_GPIO_Port GPIOB
#define PERI_MOSI_Pin GPIO_PIN_15
#define PERI_MOSI_GPIO_Port GPIOB
#define KEY_2_Pin GPIO_PIN_9
#define KEY_2_GPIO_Port GPIOD
#define KEY_1_Pin GPIO_PIN_10
#define KEY_1_GPIO_Port GPIOD
#define PWR_ON_Pin GPIO_PIN_11
#define PWR_ON_GPIO_Port GPIOD
#define PWR_ON_EXTI_IRQn EXTI15_10_IRQn
#define PWR_HOLD_Pin GPIO_PIN_12
#define PWR_HOLD_GPIO_Port GPIOD
#define WIFI_TX_Pin GPIO_PIN_6
#define WIFI_TX_GPIO_Port GPIOC
#define WIFI_RX_Pin GPIO_PIN_7
#define WIFI_RX_GPIO_Port GPIOC
#define AUDIO_I2S_CLOCK_Pin GPIO_PIN_9
#define AUDIO_I2S_CLOCK_GPIO_Port GPIOC
#define ENC2_DT_Pin GPIO_PIN_9
#define ENC2_DT_GPIO_Port GPIOA
#define ENC2_CLK_Pin GPIO_PIN_10
#define ENC2_CLK_GPIO_Port GPIOA
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
#define KEY_5_Pin GPIO_PIN_4
#define KEY_5_GPIO_Port GPIOD
#define WM8731_SDA_Pin GPIO_PIN_6
#define WM8731_SDA_GPIO_Port GPIOD
#define ENC2_SW_Pin GPIO_PIN_7
#define ENC2_SW_GPIO_Port GPIOD
#define KEY_7_Pin GPIO_PIN_5
#define KEY_7_GPIO_Port GPIOB
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

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
