/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "INA226_PWR_monitor.h"
#include "audio_processor.h"
#include "bootloader.h"
#include "codec.h"
#include "fft.h"
#include "fpga.h"
#include "front_unit.h"
#include "functions.h"
#include "images.h"
#include "jpeg_utils.h"
#include "lcd.h"
#include "lcd_driver.h"
#include "profiler.h"
#include "rf_unit.h"
#include "sd.h"
#include "settings.h"
#include "trx_manager.h"
#include "usb_device.h"
#include "usbd_cat_if.h"
#include "usbd_debug_if.h"
#include "wifi.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc3;

CRC_HandleTypeDef hcrc;

DMA2D_HandleTypeDef hdma2d;

I2S_HandleTypeDef hi2s3;

IWDG_HandleTypeDef hiwdg1;

JPEG_HandleTypeDef hjpeg;

RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi2;
DMA_HandleTypeDef hdma_spi2_rx;
DMA_HandleTypeDef hdma_spi2_tx;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim5;
TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim7;
TIM_HandleTypeDef htim15;
TIM_HandleTypeDef htim16;
TIM_HandleTypeDef htim17;

UART_HandleTypeDef huart6;
DMA_HandleTypeDef hdma_usart6_rx;

PCD_HandleTypeDef hpcd_USB_OTG_FS;

DMA_HandleTypeDef hdma_memtomem_dma2_stream5;
MDMA_HandleTypeDef hmdma_mdma_channel0_sw_0;
MDMA_HandleTypeDef hmdma_mdma_channel1_sw_0;
MDMA_HandleTypeDef hmdma_mdma_channel2_sw_0;
MDMA_HandleTypeDef hmdma_mdma_channel3_sw_0;
MDMA_HandleTypeDef hmdma_mdma_channel4_sw_0;
SRAM_HandleTypeDef hsram1;

/* USER CODE BEGIN PV */
DMA_HandleTypeDef hdma_spi3_rx;
DMA_HandleTypeDef hdma_spi3_tx;
static char greetings_buff[32] = {0};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MPU_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_MDMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_FMC_Init(void);
static void MX_I2S3_Init(void);
static void MX_RTC_Init(void);
static void MX_SPI2_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
static void MX_TIM5_Init(void);
static void MX_TIM6_Init(void);
static void MX_TIM7_Init(void);
static void MX_USART6_UART_Init(void);
static void MX_USB_OTG_FS_PCD_Init(void);
static void MX_ADC3_Init(void);
static void MX_TIM15_Init(void);
static void MX_TIM16_Init(void);
static void MX_TIM17_Init(void);
static void MX_DMA2D_Init(void);
static void MX_TIM2_Init(void);
static void MX_CRC_Init(void);
static void MX_IWDG1_Init(void);
static void MX_JPEG_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
	/* USER CODE BEGIN 1 */
	MX_GPIO_Init(); // pwr hold
	/* USER CODE END 1 */

	/* MPU Configuration--------------------------------------------------------*/
	MPU_Config();
	/* Enable the CPU Cache */

	/* Enable I-Cache---------------------------------------------------------*/
	SCB_EnableICache();

	/* Enable D-Cache---------------------------------------------------------*/
	SCB_EnableDCache();

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */
	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* Configure the peripherals common clocks */
	PeriphCommonClock_Config();

	/* USER CODE BEGIN SysInit */
	// System stabilization
	uint8_t tryes = 0;
	// while (tryes < 3 && (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY) || (RCC->CR & RCC_CR_HSERDY) == 0 || (RCC->CR & RCC_CR_PLL1RDY) == 0 || (RCC->CR &
	// RCC_CR_PLL2RDY) == 0 || (RCC->CR & RCC_CR_PLL3RDY) == 0 || (RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL1))
	while (tryes < 3 && (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY) || (RCC->CR & RCC_CR_HSERDY) == 0 || (RCC->BDCR & RCC_BDCR_LSERDY) == 0 || (RCC->CR & RCC_CR_PLL1RDY) == 0 ||
	                     (RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL1)) {
		SystemClock_Config();
		tryes++;
	}
	if (tryes == 3) {
		MX_FMC_Init();
		LCD_Init();
		LCD_showError("STM32 crystals error", false);
	}

// Stack protection
#define stack_addr 0x20000000
	MPU_Region_InitTypeDef MPU_InitStruct = {0};
	MPU_InitStruct.Enable = MPU_REGION_ENABLE;
	MPU_InitStruct.Number = MPU_REGION_NUMBER4;
	MPU_InitStruct.BaseAddress = stack_addr + 256; // growning down
	MPU_InitStruct.Size = MPU_REGION_SIZE_32B;
	MPU_InitStruct.SubRegionDisable = 0x0;
	MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
	MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
	MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
	MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
	MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
	MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
	HAL_MPU_ConfigRegion(&MPU_InitStruct);

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_DMA_Init();
	MX_MDMA_Init();
	MX_ADC1_Init();
	MX_FMC_Init();
	MX_I2S3_Init();
	MX_RTC_Init();
	MX_SPI2_Init();
	MX_TIM3_Init();
	MX_TIM4_Init();
	MX_TIM5_Init();
	MX_TIM6_Init();
	MX_TIM7_Init();
	MX_USART6_UART_Init();
	MX_ADC3_Init();
	MX_TIM15_Init();
	MX_TIM16_Init();
	MX_TIM17_Init();
	MX_FATFS_Init();
	MX_DMA2D_Init();
	MX_TIM2_Init();
	MX_CRC_Init();
	MX_JPEG_Init();
	/* USER CODE BEGIN 2 */
	// HRDW_GoToInternalSPIClock();

#ifdef HAS_TOUCHPAD
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = ENC2SW_AND_TOUCHPAD_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(T_INT_GPIO_Port, &GPIO_InitStruct);
#endif
	__HAL_RCC_CSI_ENABLE();
	__HAL_RCC_SYSCFG_CLK_ENABLE();
	__HAL_RCC_BKPRAM_CLK_ENABLE();
	HAL_EnableCompensationCell();
	println("\r\n----------------------------------");
	println("Wolf Transceiver Initialization...");
	println("[OK] USB init");
	USBD_Restart();
	println("[OK] FIFO timer TIM7 init");
	HAL_TIM_Base_Start_IT(&htim7);
	println("[OK] ENC2 timer TIM16 init");
	HAL_TIM_Base_Start_IT(&htim16);
	println("[OK] Real Time Clock init");
	HAL_RTC_Init(&hrtc);
	println("[OK] Frontpanel init");
	FRONTPANEL_Init();

	println("[OK] Settings loading");
	bool reset_settings = false;
	bool reset_calibrations = false;
	bool go_dfu = false;

#ifdef FRONTPANEL_SMALL_V1
	if (PERIPH_FrontPanel_Buttons[15].state) { // soft reset (MENU)
		reset_settings = true;
	}
	if (PERIPH_FrontPanel_Buttons[15].state && PERIPH_FrontPanel_Buttons[0].state) { // Very hard reset (MENU+PRE)
		reset_calibrations = true;
	}
#endif
#if defined(FRONTPANEL_BIG_V1) || defined(FRONTPANEL_KT_100S)
	if (PERIPH_FrontPanel_Buttons[15].state) { // soft reset (F1)
		reset_settings = true;
	}
	if (PERIPH_FrontPanel_Buttons[15].state && PERIPH_FrontPanel_Buttons[5].state) { // Very hard reset (F1+F8)
		reset_calibrations = true;
	}
#endif
#ifdef FRONTPANEL_WF_100D
	if (PERIPH_FrontPanel_Buttons[10].state) { // go DFU (MENU)
		go_dfu = true;
	}
	if (PERIPH_FrontPanel_Buttons[13].state) { // soft reset (F1)
		reset_settings = true;
	}
	if (PERIPH_FrontPanel_Buttons[13].state && PERIPH_FrontPanel_Buttons[3].state) { // Very hard reset (F1+F8)
		reset_calibrations = true;
	}
#endif

	if (reset_settings) {
		LoadSettings(true);
	} else {
		LoadSettings(false);
	}

	// DFU bootloader
	if (TRX.NeedGoToBootloader || go_dfu) {
		TRX.NeedGoToBootloader = false;
		SaveSettings();
		JumpToBootloader();
	} else {
		MX_IWDG1_Init();
	}

	println("[OK] Calibration & WiFi loading");
	if (reset_calibrations) {
		LoadCalibration(true);
	} else {
		LoadCalibration(false);
	}

#ifdef FRONTPANEL_WF_100D
	if (CALIBRATE.RF_unit_type != RF_UNIT_WF_100D) {
		LoadCalibration(true);
	}
#endif
#ifdef FRONTPANEL_KT_100S
	if (CALIBRATE.RF_unit_type != RF_UNIT_KT_100S) {
		LoadCalibration(true);
	}
#endif

	TRX.Locked = false;
	println("[OK] LCD init");
	LCD_busy = true;
	LCD_Init();
	if (SHOW_LOGO) {
		strcpy(greetings_buff, "ver. ");
		strcat(greetings_buff, STM32_VERSION_STR);

#if (defined(LAY_800x480))
		LCDDriver_Fill(rgb888torgb565(255, 255, 255));
		LCDDriver_printImage_JPEGCompressed(0, 0, IMAGES_logo800_NY_jpeg, sizeof(IMAGES_logo800_NY_jpeg));
		LCDDriver_printTextFont(greetings_buff, 30, (LCD_HEIGHT - 30), COLOR_BLACK, rgb888torgb565(255, 255, 255), &FreeSans12pt7b);
#else
		LCDDriver_Fill(rgb888torgb565(243, 243, 243));
		LCDDriver_printImage_RLECompressed(((LCD_WIDTH - IMAGES_logo480.width) / 2), ((LCD_HEIGHT - IMAGES_logo480.height) / 2), &IMAGES_logo480, BG_COLOR, BG_COLOR);
		LCDDriver_printText(greetings_buff, 10, (LCD_HEIGHT - 10 - 8), COLOR_RED, rgb888torgb565(243, 243, 243), 1);
#endif

		// show callsign greetings
		uint16_t x1, y1, w, h;
		strcpy(greetings_buff, "Hello, ");
		strcat(greetings_buff, TRX.CALLSIGN);
		strcat(greetings_buff, " !");
		LCDDriver_getTextBoundsFont(greetings_buff, LAYOUT->GREETINGS_X, LAYOUT->GREETINGS_Y, &x1, &y1, &w, &h, &FreeSans9pt7b);
#if (defined(LAY_800x480))
		LCDDriver_printTextFont(greetings_buff, LAYOUT->GREETINGS_X - (w / 2), LAYOUT->GREETINGS_Y, COLOR->GREETINGS, rgb888torgb565(255, 255, 255), &FreeSans9pt7b);
#else
		LCDDriver_printTextFont(greetings_buff, LAYOUT->GREETINGS_X - (w / 2), LAYOUT->GREETINGS_Y, COLOR->GREETINGS, rgb888torgb565(243, 243, 243), &FreeSans9pt7b);
#endif
	}
	println("[OK] Profiler init");
	InitProfiler();

	println("[OK] RTC calibration");
	RTC_Calibration();

	println("[OK] RF-Unit init");
	RF_UNIT_UpdateState(false);
	println("[OK] FFT/Waterfall & ADC & TIM4 init");
	FFT_PreInit();
	FFT_Init();
	HAL_TIM_Base_Start_IT(&htim4);
	println("[OK] STM32-ADC Calibration");
	HAL_ADCEx_Calibration_Start(&hadc1, LL_ADC_CALIB_OFFSET_LINEARITY, ADC_SINGLE_ENDED);
	HAL_ADCEx_Calibration_Start(&hadc3, LL_ADC_CALIB_OFFSET_LINEARITY, ADC_SINGLE_ENDED);
	println("[OK] AudioCodec init");
	CODEC_Init();
	println("[OK] TRX init");
	TRX_Init();
	println("[OK] Audioprocessor & TIM5 init");
	initAudioProcessor();
	HAL_TIM_Base_Start_IT(&htim5);
	LCD_busy = false;
	LCD_redraw(true);
	println("[OK] Misc timer TIM6 init");
	HAL_TIM_Base_Start_IT(&htim6);
	println("[OK] CPU Load init");
	CPULOAD_Init();
	println("[OK] PERIPHERAL timer TIM15 init");
	dma_memset(&SDFatFs, 0, sizeof(SDFatFs));
	HAL_TIM_Base_Start_IT(&htim15);
	println("[OK] Digital decoder timer TIM17 init");
	HAL_TIM_Base_Start_IT(&htim17);
	println("[OK] FPGA init");
#ifdef FRONTPANEL_SMALL_V1
	if (PERIPH_FrontPanel_Buttons[19].state) { // fpga bus test (MODE+)
		FPGA_Init(true, false);
	}
	if (PERIPH_FrontPanel_Buttons[20].state) { // fpga firmware test (MODE-)
		FPGA_Init(false, true);
	} else
#endif
		FPGA_Init(false, false);
	println("[OK] WIFI timer TIM3 init");
	HAL_TIM_Base_Start_IT(&htim3);
	println("UA3REO Transceiver started!\r\n");
	TRX_Inited = true;

	if (CALIBRATE.INA226_EN) { //	if INA226 is activated then initialise
		INA226_Init();
	}

	// while(true){HAL_Delay(3000); SCB->AIRCR = 0x05FA0004; } //debug restart
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
		CPULOAD_GoToSleepMode();
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

	/** Supply configuration update enable
	 */
	HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

	/** Configure the main internal regulator output voltage
	 */
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {
	}

	__HAL_RCC_SYSCFG_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

	while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {
	}

	/** Configure LSE Drive Capability
	 */
	HAL_PWR_EnableBkUpAccess();
	__HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_HIGH);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_LSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.LSEState = RCC_LSE_ON;
	RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
	RCC_OscInitStruct.HSICalibrationValue = 32;
	RCC_OscInitStruct.LSIState = RCC_LSI_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 2;
	RCC_OscInitStruct.PLL.PLLN = 240;
	RCC_OscInitStruct.PLL.PLLP = 2;
	RCC_OscInitStruct.PLL.PLLQ = 2;
	RCC_OscInitStruct.PLL.PLLR = 2;
	RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
	RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
	RCC_OscInitStruct.PLL.PLLFRACN = 0;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_D3PCLK1 | RCC_CLOCKTYPE_D1PCLK1;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
	RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief Peripherals Common Clock Configuration
 * @retval None
 */
void PeriphCommonClock_Config(void) {
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

	/** Initializes the peripherals clock
	 */
	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_CKPER;
	PeriphClkInitStruct.CkperClockSelection = RCC_CLKPSOURCE_HSI;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief ADC1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_ADC1_Init(void) {

	/* USER CODE BEGIN ADC1_Init 0 */

	/* USER CODE END ADC1_Init 0 */

	ADC_MultiModeTypeDef multimode = {0};
	ADC_InjectionConfTypeDef sConfigInjected = {0};

	/* USER CODE BEGIN ADC1_Init 1 */

	/* USER CODE END ADC1_Init 1 */

	/** Common config
	 */
	hadc1.Instance = ADC1;
	hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV256;
	hadc1.Init.Resolution = ADC_RESOLUTION_16B;
	hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
	hadc1.Init.EOCSelection = ADC_EOC_SEQ_CONV;
	hadc1.Init.LowPowerAutoWait = DISABLE;
	hadc1.Init.ContinuousConvMode = DISABLE;
	hadc1.Init.NbrOfConversion = 1;
	hadc1.Init.DiscontinuousConvMode = DISABLE;
	hadc1.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;
	hadc1.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
	hadc1.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
	hadc1.Init.OversamplingMode = ENABLE;
	if (HAL_ADC_Init(&hadc1) != HAL_OK) {
		Error_Handler();
	}

	/** Configure the ADC multi-mode
	 */
	multimode.Mode = ADC_MODE_INDEPENDENT;
	if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK) {
		Error_Handler();
	}

	/** Disable Injected Queue
	 */
	HAL_ADCEx_DisableInjectedQueue(&hadc1);

	/** Configure Injected Channel
	 */
	sConfigInjected.InjectedChannel = ADC_CHANNEL_11;
	sConfigInjected.InjectedRank = ADC_INJECTED_RANK_1;
	sConfigInjected.InjectedSamplingTime = ADC_SAMPLETIME_32CYCLES_5;
	sConfigInjected.InjectedSingleDiff = ADC_SINGLE_ENDED;
	sConfigInjected.InjectedOffsetNumber = ADC_OFFSET_NONE;
	sConfigInjected.InjectedOffset = 0;
	sConfigInjected.InjectedOffsetSignedSaturation = DISABLE;
	sConfigInjected.InjectedNbrOfConversion = 4;
	sConfigInjected.InjectedDiscontinuousConvMode = DISABLE;
	sConfigInjected.AutoInjectedConv = DISABLE;
	sConfigInjected.QueueInjectedContext = DISABLE;
	sConfigInjected.ExternalTrigInjecConv = ADC_INJECTED_SOFTWARE_START;
	sConfigInjected.ExternalTrigInjecConvEdge = ADC_EXTERNALTRIGINJECCONV_EDGE_NONE;
	sConfigInjected.InjecOversamplingMode = DISABLE;
	if (HAL_ADCEx_InjectedConfigChannel(&hadc1, &sConfigInjected) != HAL_OK) {
		Error_Handler();
	}

	/** Configure Injected Channel
	 */
	sConfigInjected.InjectedChannel = ADC_CHANNEL_10;
	sConfigInjected.InjectedRank = ADC_INJECTED_RANK_2;
	if (HAL_ADCEx_InjectedConfigChannel(&hadc1, &sConfigInjected) != HAL_OK) {
		Error_Handler();
	}

	/** Configure Injected Channel
	 */
	sConfigInjected.InjectedChannel = ADC_CHANNEL_8;
	sConfigInjected.InjectedRank = ADC_INJECTED_RANK_3;
	if (HAL_ADCEx_InjectedConfigChannel(&hadc1, &sConfigInjected) != HAL_OK) {
		Error_Handler();
	}

	/** Configure Injected Channel
	 */
	sConfigInjected.InjectedChannel = ADC_CHANNEL_5;
	sConfigInjected.InjectedRank = ADC_INJECTED_RANK_4;
	if (HAL_ADCEx_InjectedConfigChannel(&hadc1, &sConfigInjected) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN ADC1_Init 2 */

	/* USER CODE END ADC1_Init 2 */
}

/**
 * @brief ADC3 Initialization Function
 * @param None
 * @retval None
 */
static void MX_ADC3_Init(void) {

	/* USER CODE BEGIN ADC3_Init 0 */

	/* USER CODE END ADC3_Init 0 */

	ADC_InjectionConfTypeDef sConfigInjected = {0};

	/* USER CODE BEGIN ADC3_Init 1 */

	/* USER CODE END ADC3_Init 1 */

	/** Common config
	 */
	hadc3.Instance = ADC3;
	hadc3.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV256;
	hadc3.Init.Resolution = ADC_RESOLUTION_16B;
	hadc3.Init.ScanConvMode = ADC_SCAN_ENABLE;
	hadc3.Init.EOCSelection = ADC_EOC_SEQ_CONV;
	hadc3.Init.LowPowerAutoWait = DISABLE;
	hadc3.Init.ContinuousConvMode = ENABLE;
	hadc3.Init.NbrOfConversion = 1;
	hadc3.Init.DiscontinuousConvMode = DISABLE;
	hadc3.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;
	hadc3.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
	hadc3.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
	hadc3.Init.OversamplingMode = DISABLE;
	if (HAL_ADC_Init(&hadc3) != HAL_OK) {
		Error_Handler();
	}

	/** Disable Injected Queue
	 */
	HAL_ADCEx_DisableInjectedQueue(&hadc3);

	/** Configure Injected Channel
	 */
	sConfigInjected.InjectedChannel = ADC_CHANNEL_TEMPSENSOR;
	sConfigInjected.InjectedRank = ADC_INJECTED_RANK_1;
	sConfigInjected.InjectedSamplingTime = ADC_SAMPLETIME_810CYCLES_5;
	sConfigInjected.InjectedSingleDiff = ADC_SINGLE_ENDED;
	sConfigInjected.InjectedOffsetNumber = ADC_OFFSET_NONE;
	sConfigInjected.InjectedOffset = 0;
	sConfigInjected.InjectedOffsetSignedSaturation = DISABLE;
	sConfigInjected.InjectedNbrOfConversion = 3;
	sConfigInjected.InjectedDiscontinuousConvMode = DISABLE;
	sConfigInjected.AutoInjectedConv = DISABLE;
	sConfigInjected.QueueInjectedContext = DISABLE;
	sConfigInjected.ExternalTrigInjecConv = ADC_EXTERNALTRIGINJEC_T4_TRGO;
	sConfigInjected.ExternalTrigInjecConvEdge = ADC_EXTERNALTRIGINJECCONV_EDGE_RISING;
	sConfigInjected.InjecOversamplingMode = DISABLE;
	if (HAL_ADCEx_InjectedConfigChannel(&hadc3, &sConfigInjected) != HAL_OK) {
		Error_Handler();
	}

	/** Configure Injected Channel
	 */
	sConfigInjected.InjectedChannel = ADC_CHANNEL_VREFINT;
	sConfigInjected.InjectedRank = ADC_INJECTED_RANK_2;
	if (HAL_ADCEx_InjectedConfigChannel(&hadc3, &sConfigInjected) != HAL_OK) {
		Error_Handler();
	}

	/** Configure Injected Channel
	 */
	sConfigInjected.InjectedChannel = ADC_CHANNEL_VBAT;
	sConfigInjected.InjectedRank = ADC_INJECTED_RANK_3;
	if (HAL_ADCEx_InjectedConfigChannel(&hadc3, &sConfigInjected) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN ADC3_Init 2 */

	/* USER CODE END ADC3_Init 2 */
}

/**
 * @brief CRC Initialization Function
 * @param None
 * @retval None
 */
static void MX_CRC_Init(void) {

	/* USER CODE BEGIN CRC_Init 0 */

	/* USER CODE END CRC_Init 0 */

	/* USER CODE BEGIN CRC_Init 1 */

	/* USER CODE END CRC_Init 1 */
	hcrc.Instance = CRC;
	hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
	hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;
	hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
	hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
	hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
	if (HAL_CRC_Init(&hcrc) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN CRC_Init 2 */

	/* USER CODE END CRC_Init 2 */
}

/**
 * @brief DMA2D Initialization Function
 * @param None
 * @retval None
 */
static void MX_DMA2D_Init(void) {

	/* USER CODE BEGIN DMA2D_Init 0 */

	/* USER CODE END DMA2D_Init 0 */

	/* USER CODE BEGIN DMA2D_Init 1 */

	/* USER CODE END DMA2D_Init 1 */
	hdma2d.Instance = DMA2D;
	hdma2d.Init.Mode = DMA2D_M2M;
	hdma2d.Init.ColorMode = DMA2D_OUTPUT_RGB565;
	hdma2d.Init.OutputOffset = 0;
	hdma2d.LayerCfg[1].InputOffset = 0;
	hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB565;
	hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
	hdma2d.LayerCfg[1].InputAlpha = 0;
	hdma2d.LayerCfg[1].AlphaInverted = DMA2D_REGULAR_ALPHA;
	hdma2d.LayerCfg[1].RedBlueSwap = DMA2D_RB_REGULAR;
	hdma2d.LayerCfg[1].ChromaSubSampling = DMA2D_NO_CSS;
	if (HAL_DMA2D_Init(&hdma2d) != HAL_OK) {
		Error_Handler();
	}
	if (HAL_DMA2D_ConfigLayer(&hdma2d, 1) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN DMA2D_Init 2 */

	/* USER CODE END DMA2D_Init 2 */
}

/**
 * @brief I2S3 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2S3_Init(void) {

	/* USER CODE BEGIN I2S3_Init 0 */

	/* USER CODE END I2S3_Init 0 */

	/* USER CODE BEGIN I2S3_Init 1 */

	/* USER CODE END I2S3_Init 1 */
	hi2s3.Instance = SPI3;
	hi2s3.Init.Mode = I2S_MODE_MASTER_FULLDUPLEX;
	hi2s3.Init.Standard = I2S_STANDARD_PHILIPS;
	hi2s3.Init.DataFormat = I2S_DATAFORMAT_32B;
	hi2s3.Init.MCLKOutput = I2S_MCLKOUTPUT_DISABLE;
	hi2s3.Init.AudioFreq = I2S_AUDIOFREQ_48K;
	hi2s3.Init.CPOL = I2S_CPOL_LOW;
	hi2s3.Init.FirstBit = I2S_FIRSTBIT_MSB;
	hi2s3.Init.WSInversion = I2S_WS_INVERSION_DISABLE;
	hi2s3.Init.Data24BitAlignment = I2S_DATA_24BIT_ALIGNMENT_RIGHT;
	hi2s3.Init.MasterKeepIOState = I2S_MASTER_KEEP_IO_STATE_ENABLE;
	if (HAL_I2S_Init(&hi2s3) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN I2S3_Init 2 */
	// перевод I2S в режим full-duplex
	hi2s3.Init.Mode = I2S_MODE_MASTER_FULLDUPLEX;
	if (HAL_I2S_Init(&hi2s3) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE END I2S3_Init 2 */
}

/**
 * @brief IWDG1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_IWDG1_Init(void) {

	/* USER CODE BEGIN IWDG1_Init 0 */

	/* USER CODE END IWDG1_Init 0 */

	/* USER CODE BEGIN IWDG1_Init 1 */

	/* USER CODE END IWDG1_Init 1 */
	hiwdg1.Instance = IWDG1;
	hiwdg1.Init.Prescaler = IWDG_PRESCALER_128;
	hiwdg1.Init.Window = 1500;
	hiwdg1.Init.Reload = 1500;
	if (HAL_IWDG_Init(&hiwdg1) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN IWDG1_Init 2 */

	/* USER CODE END IWDG1_Init 2 */
}

/**
 * @brief JPEG Initialization Function
 * @param None
 * @retval None
 */
static void MX_JPEG_Init(void) {

	/* USER CODE BEGIN JPEG_Init 0 */

	/* USER CODE END JPEG_Init 0 */

	/* USER CODE BEGIN JPEG_Init 1 */

	/* USER CODE END JPEG_Init 1 */
	hjpeg.Instance = JPEG;
	if (HAL_JPEG_Init(&hjpeg) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN JPEG_Init 2 */

	/* USER CODE END JPEG_Init 2 */
}

/**
 * @brief RTC Initialization Function
 * @param None
 * @retval None
 */
static void MX_RTC_Init(void) {

	/* USER CODE BEGIN RTC_Init 0 */

	/* USER CODE END RTC_Init 0 */

	/* USER CODE BEGIN RTC_Init 1 */

	/* USER CODE END RTC_Init 1 */

	/** Initialize RTC Only
	 */
	hrtc.Instance = RTC;
	hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
	hrtc.Init.AsynchPrediv = 127;
	hrtc.Init.SynchPrediv = 255;
	hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
	hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
	hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
	hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
	if (HAL_RTC_Init(&hrtc) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN RTC_Init 2 */

	/* USER CODE END RTC_Init 2 */
}

/**
 * @brief SPI2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_SPI2_Init(void) {

	/* USER CODE BEGIN SPI2_Init 0 */

	/* USER CODE END SPI2_Init 0 */

	/* USER CODE BEGIN SPI2_Init 1 */

	/* USER CODE END SPI2_Init 1 */
	/* SPI2 parameter configuration*/
	hspi2.Instance = SPI2;
	hspi2.Init.Mode = SPI_MODE_MASTER;
	hspi2.Init.Direction = SPI_DIRECTION_2LINES;
	hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
	hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi2.Init.NSS = SPI_NSS_SOFT;
	hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
	hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi2.Init.CRCPolynomial = 0x0;
	hspi2.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
	hspi2.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
	hspi2.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
	hspi2.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
	hspi2.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
	hspi2.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
	hspi2.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
	hspi2.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
	hspi2.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
	hspi2.Init.IOSwap = SPI_IO_SWAP_DISABLE;
	if (HAL_SPI_Init(&hspi2) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN SPI2_Init 2 */

	/* USER CODE END SPI2_Init 2 */
}

/**
 * @brief TIM2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM2_Init(void) {

	/* USER CODE BEGIN TIM2_Init 0 */

	/* USER CODE END TIM2_Init 0 */

	TIM_ClockConfigTypeDef sClockSourceConfig = {0};
	TIM_MasterConfigTypeDef sMasterConfig = {0};

	/* USER CODE BEGIN TIM2_Init 1 */

	/* USER CODE END TIM2_Init 1 */
	htim2.Instance = TIM2;
	htim2.Init.Prescaler = 16341;
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim2.Init.Period = 10025;
	htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim2) != HAL_OK) {
		Error_Handler();
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM2_Init 2 */

	/* USER CODE END TIM2_Init 2 */
}

/**
 * @brief TIM3 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM3_Init(void) {

	/* USER CODE BEGIN TIM3_Init 0 */

	/* USER CODE END TIM3_Init 0 */

	TIM_ClockConfigTypeDef sClockSourceConfig = {0};
	TIM_MasterConfigTypeDef sMasterConfig = {0};

	/* USER CODE BEGIN TIM3_Init 1 */

	/* USER CODE END TIM3_Init 1 */
	htim3.Instance = TIM3;
	htim3.Init.Prescaler = 1199;
	htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim3.Init.Period = 199;
	htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim3) != HAL_OK) {
		Error_Handler();
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM3_Init 2 */

	/* USER CODE END TIM3_Init 2 */
}

/**
 * @brief TIM4 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM4_Init(void) {

	/* USER CODE BEGIN TIM4_Init 0 */

	/* USER CODE END TIM4_Init 0 */

	TIM_ClockConfigTypeDef sClockSourceConfig = {0};
	TIM_MasterConfigTypeDef sMasterConfig = {0};

	/* USER CODE BEGIN TIM4_Init 1 */

	/* USER CODE END TIM4_Init 1 */
	htim4.Instance = TIM4;
	htim4.Init.Prescaler = 1199;
	htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim4.Init.Period = 199;
	htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim4) != HAL_OK) {
		Error_Handler();
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM4_Init 2 */

	/* USER CODE END TIM4_Init 2 */
}

/**
 * @brief TIM5 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM5_Init(void) {

	/* USER CODE BEGIN TIM5_Init 0 */

	/* USER CODE END TIM5_Init 0 */

	TIM_ClockConfigTypeDef sClockSourceConfig = {0};
	TIM_MasterConfigTypeDef sMasterConfig = {0};

	/* USER CODE BEGIN TIM5_Init 1 */

	/* USER CODE END TIM5_Init 1 */
	htim5.Instance = TIM5;
	htim5.Init.Prescaler = 119;
	htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim5.Init.Period = 99;
	htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim5.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim5) != HAL_OK) {
		Error_Handler();
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim5, &sClockSourceConfig) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM5_Init 2 */

	/* USER CODE END TIM5_Init 2 */
}

/**
 * @brief TIM6 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM6_Init(void) {

	/* USER CODE BEGIN TIM6_Init 0 */

	/* USER CODE END TIM6_Init 0 */

	TIM_MasterConfigTypeDef sMasterConfig = {0};

	/* USER CODE BEGIN TIM6_Init 1 */

	/* USER CODE END TIM6_Init 1 */
	htim6.Instance = TIM6;
	htim6.Init.Prescaler = 11999;
	htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim6.Init.Period = 199;
	htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim6) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM6_Init 2 */

	/* USER CODE END TIM6_Init 2 */
}

/**
 * @brief TIM7 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM7_Init(void) {

	/* USER CODE BEGIN TIM7_Init 0 */

	/* USER CODE END TIM7_Init 0 */

	TIM_MasterConfigTypeDef sMasterConfig = {0};

	/* USER CODE BEGIN TIM7_Init 1 */

	/* USER CODE END TIM7_Init 1 */
	htim7.Instance = TIM7;
	htim7.Init.Prescaler = 1199;
	htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim7.Init.Period = 199;
	htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim7) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM7_Init 2 */

	/* USER CODE END TIM7_Init 2 */
}

/**
 * @brief TIM15 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM15_Init(void) {

	/* USER CODE BEGIN TIM15_Init 0 */

	/* USER CODE END TIM15_Init 0 */

	TIM_ClockConfigTypeDef sClockSourceConfig = {0};
	TIM_MasterConfigTypeDef sMasterConfig = {0};

	/* USER CODE BEGIN TIM15_Init 1 */

	/* USER CODE END TIM15_Init 1 */
	htim15.Instance = TIM15;
	htim15.Init.Prescaler = 1199;
	htim15.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim15.Init.Period = 199;
	htim15.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim15.Init.RepetitionCounter = 0;
	htim15.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim15) != HAL_OK) {
		Error_Handler();
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim15, &sClockSourceConfig) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim15, &sMasterConfig) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM15_Init 2 */

	/* USER CODE END TIM15_Init 2 */
}

/**
 * @brief TIM16 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM16_Init(void) {

	/* USER CODE BEGIN TIM16_Init 0 */

	/* USER CODE END TIM16_Init 0 */

	/* USER CODE BEGIN TIM16_Init 1 */

	/* USER CODE END TIM16_Init 1 */
	htim16.Instance = TIM16;
	htim16.Init.Prescaler = 1199;
	htim16.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim16.Init.Period = 199;
	htim16.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim16.Init.RepetitionCounter = 0;
	htim16.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim16) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM16_Init 2 */

	/* USER CODE END TIM16_Init 2 */
}

/**
 * @brief TIM17 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM17_Init(void) {

	/* USER CODE BEGIN TIM17_Init 0 */

	/* USER CODE END TIM17_Init 0 */

	/* USER CODE BEGIN TIM17_Init 1 */

	/* USER CODE END TIM17_Init 1 */
	htim17.Instance = TIM17;
	htim17.Init.Prescaler = 119;
	htim17.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim17.Init.Period = 99;
	htim17.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim17.Init.RepetitionCounter = 0;
	htim17.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim17) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM17_Init 2 */

	/* USER CODE END TIM17_Init 2 */
}

/**
 * @brief USART6 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART6_UART_Init(void) {

	/* USER CODE BEGIN USART6_Init 0 */

	/* USER CODE END USART6_Init 0 */

	/* USER CODE BEGIN USART6_Init 1 */

	/* USER CODE END USART6_Init 1 */
	huart6.Instance = USART6;
	huart6.Init.BaudRate = 115200;
	huart6.Init.WordLength = UART_WORDLENGTH_8B;
	huart6.Init.StopBits = UART_STOPBITS_1;
	huart6.Init.Parity = UART_PARITY_NONE;
	huart6.Init.Mode = UART_MODE_TX_RX;
	huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart6.Init.OverSampling = UART_OVERSAMPLING_16;
	huart6.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart6.Init.ClockPrescaler = UART_PRESCALER_DIV1;
	huart6.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(&huart6) != HAL_OK) {
		Error_Handler();
	}
	if (HAL_UARTEx_SetTxFifoThreshold(&huart6, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK) {
		Error_Handler();
	}
	if (HAL_UARTEx_SetRxFifoThreshold(&huart6, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK) {
		Error_Handler();
	}
	if (HAL_UARTEx_DisableFifoMode(&huart6) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN USART6_Init 2 */

	/* USER CODE END USART6_Init 2 */
}

/**
 * @brief USB_OTG_FS Initialization Function
 * @param None
 * @retval None
 */
static void MX_USB_OTG_FS_PCD_Init(void) {

	/* USER CODE BEGIN USB_OTG_FS_Init 0 */

	/* USER CODE END USB_OTG_FS_Init 0 */

	/* USER CODE BEGIN USB_OTG_FS_Init 1 */

	/* USER CODE END USB_OTG_FS_Init 1 */
	hpcd_USB_OTG_FS.Instance = USB_OTG_FS;
	hpcd_USB_OTG_FS.Init.dev_endpoints = 9;
	hpcd_USB_OTG_FS.Init.speed = PCD_SPEED_FULL;
	hpcd_USB_OTG_FS.Init.dma_enable = DISABLE;
	hpcd_USB_OTG_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
	hpcd_USB_OTG_FS.Init.Sof_enable = ENABLE;
	hpcd_USB_OTG_FS.Init.low_power_enable = DISABLE;
	hpcd_USB_OTG_FS.Init.lpm_enable = DISABLE;
	hpcd_USB_OTG_FS.Init.battery_charging_enable = ENABLE;
	hpcd_USB_OTG_FS.Init.vbus_sensing_enable = ENABLE;
	hpcd_USB_OTG_FS.Init.use_dedicated_ep1 = DISABLE;
	if (HAL_PCD_Init(&hpcd_USB_OTG_FS) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN USB_OTG_FS_Init 2 */

	/* USER CODE END USB_OTG_FS_Init 2 */
}

/**
 * Enable DMA controller clock
 * Configure DMA for memory to memory transfers
 *   hdma_memtomem_dma2_stream5
 */
static void MX_DMA_Init(void) {

	/* DMA controller clock enable */
	__HAL_RCC_DMA1_CLK_ENABLE();
	__HAL_RCC_DMA2_CLK_ENABLE();

	/* Configure DMA request hdma_memtomem_dma2_stream5 on DMA2_Stream5 */
	hdma_memtomem_dma2_stream5.Instance = DMA2_Stream5;
	hdma_memtomem_dma2_stream5.Init.Request = DMA_REQUEST_MEM2MEM;
	hdma_memtomem_dma2_stream5.Init.Direction = DMA_MEMORY_TO_MEMORY;
	hdma_memtomem_dma2_stream5.Init.PeriphInc = DMA_PINC_ENABLE;
	hdma_memtomem_dma2_stream5.Init.MemInc = DMA_MINC_DISABLE;
	hdma_memtomem_dma2_stream5.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
	hdma_memtomem_dma2_stream5.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
	hdma_memtomem_dma2_stream5.Init.Mode = DMA_NORMAL;
	hdma_memtomem_dma2_stream5.Init.Priority = DMA_PRIORITY_MEDIUM;
	hdma_memtomem_dma2_stream5.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
	hdma_memtomem_dma2_stream5.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
	hdma_memtomem_dma2_stream5.Init.MemBurst = DMA_MBURST_INC8;
	hdma_memtomem_dma2_stream5.Init.PeriphBurst = DMA_PBURST_INC8;
	if (HAL_DMA_Init(&hdma_memtomem_dma2_stream5) != HAL_OK) {
		Error_Handler();
	}

	/* DMA interrupt init */
	/* DMA1_Stream1_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
	/* DMA1_Stream2_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA1_Stream2_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DMA1_Stream2_IRQn);
	/* DMA1_Stream3_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA1_Stream3_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DMA1_Stream3_IRQn);
	/* DMA2_Stream5_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA2_Stream5_IRQn, 7, 0);
	HAL_NVIC_EnableIRQ(DMA2_Stream5_IRQn);
}

/**
 * Enable MDMA controller clock
 * Configure MDMA for global transfers
 *   hmdma_mdma_channel0_sw_0
 *   hmdma_mdma_channel1_sw_0
 *   hmdma_mdma_channel2_sw_0
 *   hmdma_mdma_channel3_sw_0
 *   hmdma_mdma_channel4_sw_0
 */
static void MX_MDMA_Init(void) {

	/* MDMA controller clock enable */
	__HAL_RCC_MDMA_CLK_ENABLE();
	/* Local variables */

	/* Configure MDMA channel MDMA_Channel0 */
	/* Configure MDMA request hmdma_mdma_channel0_sw_0 on MDMA_Channel0 */
	hmdma_mdma_channel0_sw_0.Instance = MDMA_Channel0;
	hmdma_mdma_channel0_sw_0.Init.Request = MDMA_REQUEST_SW;
	hmdma_mdma_channel0_sw_0.Init.TransferTriggerMode = MDMA_FULL_TRANSFER;
	hmdma_mdma_channel0_sw_0.Init.Priority = MDMA_PRIORITY_MEDIUM;
	hmdma_mdma_channel0_sw_0.Init.Endianness = MDMA_LITTLE_ENDIANNESS_PRESERVE;
	hmdma_mdma_channel0_sw_0.Init.SourceInc = MDMA_SRC_INC_WORD;
	hmdma_mdma_channel0_sw_0.Init.DestinationInc = MDMA_DEST_INC_WORD;
	hmdma_mdma_channel0_sw_0.Init.SourceDataSize = MDMA_SRC_DATASIZE_WORD;
	hmdma_mdma_channel0_sw_0.Init.DestDataSize = MDMA_DEST_DATASIZE_WORD;
	hmdma_mdma_channel0_sw_0.Init.DataAlignment = MDMA_DATAALIGN_PACKENABLE;
	hmdma_mdma_channel0_sw_0.Init.BufferTransferLength = 32;
	hmdma_mdma_channel0_sw_0.Init.SourceBurst = MDMA_SOURCE_BURST_SINGLE;
	hmdma_mdma_channel0_sw_0.Init.DestBurst = MDMA_DEST_BURST_SINGLE;
	hmdma_mdma_channel0_sw_0.Init.SourceBlockAddressOffset = 0;
	hmdma_mdma_channel0_sw_0.Init.DestBlockAddressOffset = 0;
	if (HAL_MDMA_Init(&hmdma_mdma_channel0_sw_0) != HAL_OK) {
		Error_Handler();
	}

	/* Configure MDMA channel MDMA_Channel1 */
	/* Configure MDMA request hmdma_mdma_channel1_sw_0 on MDMA_Channel1 */
	hmdma_mdma_channel1_sw_0.Instance = MDMA_Channel1;
	hmdma_mdma_channel1_sw_0.Init.Request = MDMA_REQUEST_SW;
	hmdma_mdma_channel1_sw_0.Init.TransferTriggerMode = MDMA_FULL_TRANSFER;
	hmdma_mdma_channel1_sw_0.Init.Priority = MDMA_PRIORITY_MEDIUM;
	hmdma_mdma_channel1_sw_0.Init.Endianness = MDMA_LITTLE_ENDIANNESS_PRESERVE;
	hmdma_mdma_channel1_sw_0.Init.SourceInc = MDMA_SRC_INC_WORD;
	hmdma_mdma_channel1_sw_0.Init.DestinationInc = MDMA_DEST_INC_WORD;
	hmdma_mdma_channel1_sw_0.Init.SourceDataSize = MDMA_SRC_DATASIZE_WORD;
	hmdma_mdma_channel1_sw_0.Init.DestDataSize = MDMA_DEST_DATASIZE_WORD;
	hmdma_mdma_channel1_sw_0.Init.DataAlignment = MDMA_DATAALIGN_PACKENABLE;
	hmdma_mdma_channel1_sw_0.Init.BufferTransferLength = 32;
	hmdma_mdma_channel1_sw_0.Init.SourceBurst = MDMA_SOURCE_BURST_SINGLE;
	hmdma_mdma_channel1_sw_0.Init.DestBurst = MDMA_DEST_BURST_SINGLE;
	hmdma_mdma_channel1_sw_0.Init.SourceBlockAddressOffset = 0;
	hmdma_mdma_channel1_sw_0.Init.DestBlockAddressOffset = 0;
	if (HAL_MDMA_Init(&hmdma_mdma_channel1_sw_0) != HAL_OK) {
		Error_Handler();
	}

	/* Configure MDMA channel MDMA_Channel2 */
	/* Configure MDMA request hmdma_mdma_channel2_sw_0 on MDMA_Channel2 */
	hmdma_mdma_channel2_sw_0.Instance = MDMA_Channel2;
	hmdma_mdma_channel2_sw_0.Init.Request = MDMA_REQUEST_SW;
	hmdma_mdma_channel2_sw_0.Init.TransferTriggerMode = MDMA_FULL_TRANSFER;
	hmdma_mdma_channel2_sw_0.Init.Priority = MDMA_PRIORITY_MEDIUM;
	hmdma_mdma_channel2_sw_0.Init.Endianness = MDMA_LITTLE_ENDIANNESS_PRESERVE;
	hmdma_mdma_channel2_sw_0.Init.SourceInc = MDMA_SRC_INC_WORD;
	hmdma_mdma_channel2_sw_0.Init.DestinationInc = MDMA_DEST_INC_WORD;
	hmdma_mdma_channel2_sw_0.Init.SourceDataSize = MDMA_SRC_DATASIZE_WORD;
	hmdma_mdma_channel2_sw_0.Init.DestDataSize = MDMA_DEST_DATASIZE_WORD;
	hmdma_mdma_channel2_sw_0.Init.DataAlignment = MDMA_DATAALIGN_PACKENABLE;
	hmdma_mdma_channel2_sw_0.Init.BufferTransferLength = 32;
	hmdma_mdma_channel2_sw_0.Init.SourceBurst = MDMA_SOURCE_BURST_SINGLE;
	hmdma_mdma_channel2_sw_0.Init.DestBurst = MDMA_DEST_BURST_SINGLE;
	hmdma_mdma_channel2_sw_0.Init.SourceBlockAddressOffset = 0;
	hmdma_mdma_channel2_sw_0.Init.DestBlockAddressOffset = 0;
	if (HAL_MDMA_Init(&hmdma_mdma_channel2_sw_0) != HAL_OK) {
		Error_Handler();
	}

	/* Configure MDMA channel MDMA_Channel3 */
	/* Configure MDMA request hmdma_mdma_channel3_sw_0 on MDMA_Channel3 */
	hmdma_mdma_channel3_sw_0.Instance = MDMA_Channel3;
	hmdma_mdma_channel3_sw_0.Init.Request = MDMA_REQUEST_SW;
	hmdma_mdma_channel3_sw_0.Init.TransferTriggerMode = MDMA_FULL_TRANSFER;
	hmdma_mdma_channel3_sw_0.Init.Priority = MDMA_PRIORITY_LOW;
	hmdma_mdma_channel3_sw_0.Init.Endianness = MDMA_LITTLE_ENDIANNESS_PRESERVE;
	hmdma_mdma_channel3_sw_0.Init.SourceInc = MDMA_SRC_DEC_BYTE;
	hmdma_mdma_channel3_sw_0.Init.DestinationInc = MDMA_DEST_DEC_BYTE;
	hmdma_mdma_channel3_sw_0.Init.SourceDataSize = MDMA_SRC_DATASIZE_BYTE;
	hmdma_mdma_channel3_sw_0.Init.DestDataSize = MDMA_DEST_DATASIZE_BYTE;
	hmdma_mdma_channel3_sw_0.Init.DataAlignment = MDMA_DATAALIGN_PACKENABLE;
	hmdma_mdma_channel3_sw_0.Init.BufferTransferLength = 32;
	hmdma_mdma_channel3_sw_0.Init.SourceBurst = MDMA_SOURCE_BURST_SINGLE;
	hmdma_mdma_channel3_sw_0.Init.DestBurst = MDMA_DEST_BURST_SINGLE;
	hmdma_mdma_channel3_sw_0.Init.SourceBlockAddressOffset = 0;
	hmdma_mdma_channel3_sw_0.Init.DestBlockAddressOffset = 0;
	if (HAL_MDMA_Init(&hmdma_mdma_channel3_sw_0) != HAL_OK) {
		Error_Handler();
	}

	/* Configure MDMA channel MDMA_Channel4 */
	/* Configure MDMA request hmdma_mdma_channel4_sw_0 on MDMA_Channel4 */
	hmdma_mdma_channel4_sw_0.Instance = MDMA_Channel4;
	hmdma_mdma_channel4_sw_0.Init.Request = MDMA_REQUEST_SW;
	hmdma_mdma_channel4_sw_0.Init.TransferTriggerMode = MDMA_FULL_TRANSFER;
	hmdma_mdma_channel4_sw_0.Init.Priority = MDMA_PRIORITY_MEDIUM;
	hmdma_mdma_channel4_sw_0.Init.Endianness = MDMA_LITTLE_ENDIANNESS_PRESERVE;
	hmdma_mdma_channel4_sw_0.Init.SourceInc = MDMA_SRC_INC_DISABLE;
	hmdma_mdma_channel4_sw_0.Init.DestinationInc = MDMA_DEST_INC_WORD;
	hmdma_mdma_channel4_sw_0.Init.SourceDataSize = MDMA_SRC_DATASIZE_WORD;
	hmdma_mdma_channel4_sw_0.Init.DestDataSize = MDMA_DEST_DATASIZE_WORD;
	hmdma_mdma_channel4_sw_0.Init.DataAlignment = MDMA_DATAALIGN_PACKENABLE;
	hmdma_mdma_channel4_sw_0.Init.BufferTransferLength = 32;
	hmdma_mdma_channel4_sw_0.Init.SourceBurst = MDMA_SOURCE_BURST_SINGLE;
	hmdma_mdma_channel4_sw_0.Init.DestBurst = MDMA_DEST_BURST_SINGLE;
	hmdma_mdma_channel4_sw_0.Init.SourceBlockAddressOffset = 0;
	hmdma_mdma_channel4_sw_0.Init.DestBlockAddressOffset = 0;
	if (HAL_MDMA_Init(&hmdma_mdma_channel4_sw_0) != HAL_OK) {
		Error_Handler();
	}
}

/* FMC initialization function */
static void MX_FMC_Init(void) {

	/* USER CODE BEGIN FMC_Init 0 */

	/* USER CODE END FMC_Init 0 */

	FMC_NORSRAM_TimingTypeDef Timing = {0};

	/* USER CODE BEGIN FMC_Init 1 */

	/* USER CODE END FMC_Init 1 */

	/** Perform the SRAM1 memory initialization sequence
	 */
	hsram1.Instance = FMC_NORSRAM_DEVICE;
	hsram1.Extended = FMC_NORSRAM_EXTENDED_DEVICE;
	/* hsram1.Init */
	hsram1.Init.NSBank = FMC_NORSRAM_BANK1;
	hsram1.Init.DataAddressMux = FMC_DATA_ADDRESS_MUX_DISABLE;
	hsram1.Init.MemoryType = FMC_MEMORY_TYPE_SRAM;
	hsram1.Init.MemoryDataWidth = FMC_NORSRAM_MEM_BUS_WIDTH_16;
	hsram1.Init.BurstAccessMode = FMC_BURST_ACCESS_MODE_DISABLE;
	hsram1.Init.WaitSignalPolarity = FMC_WAIT_SIGNAL_POLARITY_LOW;
	hsram1.Init.WaitSignalActive = FMC_WAIT_TIMING_BEFORE_WS;
	hsram1.Init.WriteOperation = FMC_WRITE_OPERATION_ENABLE;
	hsram1.Init.WaitSignal = FMC_WAIT_SIGNAL_DISABLE;
	hsram1.Init.ExtendedMode = FMC_EXTENDED_MODE_DISABLE;
	hsram1.Init.AsynchronousWait = FMC_ASYNCHRONOUS_WAIT_DISABLE;
	hsram1.Init.WriteBurst = FMC_WRITE_BURST_DISABLE;
	hsram1.Init.ContinuousClock = FMC_CONTINUOUS_CLOCK_SYNC_ONLY;
	hsram1.Init.WriteFifo = FMC_WRITE_FIFO_DISABLE;
	hsram1.Init.PageSize = FMC_PAGE_SIZE_NONE;
	/* Timing */
	Timing.AddressSetupTime = 5;
	Timing.AddressHoldTime = 15;
	Timing.DataSetupTime = 5;
	Timing.BusTurnAroundDuration = 3;
	Timing.CLKDivision = 16;
	Timing.DataLatency = 17;
	Timing.AccessMode = FMC_ACCESS_MODE_A;
	/* ExtTiming */

	if (HAL_SRAM_Init(&hsram1, &Timing, NULL) != HAL_OK) {
		Error_Handler();
	}

	HAL_SetFMCMemorySwappingConfig(FMC_SWAPBMAP_SDRAM_SRAM);

	/* USER CODE BEGIN FMC_Init 2 */

	// LCD timings
#if (defined(LCD_HX8357B))
	Timing.AddressSetupTime = 5;
	Timing.DataSetupTime = 5;
	Timing.BusTurnAroundDuration = 3;
	Timing.AccessMode = FMC_ACCESS_MODE_A;
#endif
#if (defined(LCD_HX8357C))
	Timing.AddressSetupTime = 10;
	Timing.DataSetupTime = 10;
	Timing.BusTurnAroundDuration = 8;
	Timing.AccessMode = FMC_ACCESS_MODE_A;
#if (defined(LCD_SLOW))
	Timing.AddressSetupTime = 20;
	Timing.DataSetupTime = 20;
	Timing.BusTurnAroundDuration = 16;
	Timing.AccessMode = FMC_ACCESS_MODE_A;
#endif
#endif
#if (defined(LCD_SSD1963))
	Timing.AddressSetupTime = 5;
	Timing.DataSetupTime = 5;
	Timing.BusTurnAroundDuration = 3;
	Timing.AccessMode = FMC_ACCESS_MODE_A;
#endif
#if (defined(LCD_ILI9486))
	Timing.AddressSetupTime = 5;
	Timing.DataSetupTime = 5;
	Timing.BusTurnAroundDuration = 3;
	Timing.AccessMode = FMC_ACCESS_MODE_A;
#endif
#if defined(LCD_ILI9481)
	Timing.AddressSetupTime = 5;
	Timing.DataSetupTime = 5;
	Timing.BusTurnAroundDuration = 3;
	Timing.AccessMode = FMC_ACCESS_MODE_A;
#endif
#if defined(LCD_ST7796S)
	Timing.AddressSetupTime = 5;
	Timing.DataSetupTime = 5;
	Timing.BusTurnAroundDuration = 3;
	Timing.AccessMode = FMC_ACCESS_MODE_A;
#endif
#if (defined(LCD_RA8875))
	Timing.AddressSetupTime = 30;
	Timing.DataSetupTime = 30;
	Timing.BusTurnAroundDuration = 20;
	Timing.AccessMode = FMC_ACCESS_MODE_A;
	// fast timings in lcd_driver_RA8875.c
#endif
	if (HAL_SRAM_Init(&hsram1, &Timing, NULL) != HAL_OK) {
		Error_Handler();
	}
	HAL_SetFMCMemorySwappingConfig(FMC_SWAPBMAP_SDRAM_SRAM);

	/* USER CODE END FMC_Init 2 */
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	/* USER CODE BEGIN MX_GPIO_Init_1 */
	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOC, FPGA_CLK_Pin | FPGA_SYNC_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOA, FPGA_BUS_D0_Pin | FPGA_BUS_D1_Pin | FPGA_BUS_D2_Pin | FPGA_BUS_D3_Pin | FPGA_BUS_D4_Pin | FPGA_BUS_D5_Pin | FPGA_BUS_D6_Pin | FPGA_BUS_D7_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, W25Q16_CS_Pin | SD_CS_Pin | AD3_CS_Pin | RFUNIT_OE_Pin, GPIO_PIN_SET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(PWR_HOLD_GPIO_Port, PWR_HOLD_Pin, GPIO_PIN_SET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(AD1_CS_GPIO_Port, AD1_CS_Pin, GPIO_PIN_SET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(AD2_CS_GPIO_Port, AD2_CS_Pin, GPIO_PIN_SET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOD, WM8731_SCK_Pin | WM8731_SDA_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, AF_AMP_MUTE_Pin | RFUNIT_RCLK_Pin | RFUNIT_CLK_Pin | RFUNIT_DATA_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pins : ENC_CLK_Pin KEY_IN_DASH_Pin KEY_IN_DOT_Pin */
	GPIO_InitStruct.Pin = ENC_CLK_Pin | KEY_IN_DASH_Pin | KEY_IN_DOT_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	/*Configure GPIO pin : PE3 */
	GPIO_InitStruct.Pin = GPIO_PIN_3;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	/*Configure GPIO pins : ENC2SW_AND_TOUCHPAD_Pin ENC_DT_Pin ENC2_DT_Pin */
	GPIO_InitStruct.Pin = ENC2SW_AND_TOUCHPAD_Pin | ENC_DT_Pin | ENC2_DT_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	/*Configure GPIO pin : ENC2_CLK_Pin */
	GPIO_InitStruct.Pin = ENC2_CLK_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(ENC2_CLK_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : SWR_BACKW_Pin */
	GPIO_InitStruct.Pin = SWR_BACKW_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(SWR_BACKW_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : FPGA_CLK_Pin FPGA_SYNC_Pin AD1_CS_Pin */
	GPIO_InitStruct.Pin = FPGA_CLK_Pin | FPGA_SYNC_Pin | AD1_CS_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/*Configure GPIO pins : FPGA_BUS_D0_Pin FPGA_BUS_D1_Pin FPGA_BUS_D2_Pin FPGA_BUS_D3_Pin
	                         FPGA_BUS_D4_Pin FPGA_BUS_D5_Pin FPGA_BUS_D6_Pin FPGA_BUS_D7_Pin
	                         AD2_CS_Pin */
	GPIO_InitStruct.Pin = FPGA_BUS_D0_Pin | FPGA_BUS_D1_Pin | FPGA_BUS_D2_Pin | FPGA_BUS_D3_Pin | FPGA_BUS_D4_Pin | FPGA_BUS_D5_Pin | FPGA_BUS_D6_Pin | FPGA_BUS_D7_Pin | AD2_CS_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pin : PTT_IN_Pin */
	GPIO_InitStruct.Pin = PTT_IN_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(PTT_IN_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : W25Q16_CS_Pin SD_CS_Pin AD3_CS_Pin RFUNIT_RCLK_Pin
	                         RFUNIT_CLK_Pin RFUNIT_DATA_Pin RFUNIT_OE_Pin */
	GPIO_InitStruct.Pin = W25Q16_CS_Pin | SD_CS_Pin | AD3_CS_Pin | RFUNIT_RCLK_Pin | RFUNIT_CLK_Pin | RFUNIT_DATA_Pin | RFUNIT_OE_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pins : PB2 PB4 */
	GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_4;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pin : AUDIO_48K_CLOCK_Pin */
	GPIO_InitStruct.Pin = AUDIO_48K_CLOCK_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(AUDIO_48K_CLOCK_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : PWR_ON_Pin */
	GPIO_InitStruct.Pin = PWR_ON_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(PWR_ON_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : PWR_HOLD_Pin WM8731_SCK_Pin WM8731_SDA_Pin */
	GPIO_InitStruct.Pin = PWR_HOLD_Pin | WM8731_SCK_Pin | WM8731_SDA_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	/*Configure GPIO pin : PC9 */
	GPIO_InitStruct.Pin = GPIO_PIN_9;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/*Configure GPIO pin : PD2 */
	GPIO_InitStruct.Pin = GPIO_PIN_2;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	/*Configure GPIO pin : AF_AMP_MUTE_Pin */
	GPIO_InitStruct.Pin = AF_AMP_MUTE_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(AF_AMP_MUTE_GPIO_Port, &GPIO_InitStruct);

	/* EXTI interrupt init*/
	HAL_NVIC_SetPriority(EXTI0_IRQn, 6, 0);
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);

	HAL_NVIC_SetPriority(EXTI1_IRQn, 6, 0);
	HAL_NVIC_EnableIRQ(EXTI1_IRQn);

	HAL_NVIC_SetPriority(EXTI2_IRQn, 2, 0);
	HAL_NVIC_EnableIRQ(EXTI2_IRQn);

	HAL_NVIC_SetPriority(EXTI4_IRQn, 6, 0);
	HAL_NVIC_EnableIRQ(EXTI4_IRQn);

	HAL_NVIC_SetPriority(EXTI15_10_IRQn, 3, 0);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

	/* USER CODE BEGIN MX_GPIO_Init_2 */
	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
// обработка вывода отладки
FILE __stdout;
FILE __stdin;

int fputc(int ch, FILE *f) {
#pragma unused(f)

	// SWD
	if (SWD_DEBUG_ENABLED) {
		ITM_SendChar((uint32_t)ch);
	}

	// USB
	if (USB_DEBUG_ENABLED) {
		char usb_char = (char)ch;
		DEBUG_Transmit_FIFO((uint8_t *)&usb_char, 1);
	}

	// LCD
	if (LCD_DEBUG_ENABLED) {
		print_chr_LCDOnly((char)ch);
	}

	return (ch);
}
/* USER CODE END 4 */

/* MPU Configuration */

void MPU_Config(void) {
	MPU_Region_InitTypeDef MPU_InitStruct = {0};

	/* Disables the MPU */
	HAL_MPU_Disable();

	/** Initializes and configures the Region and the memory to be protected
	 */
	MPU_InitStruct.Enable = MPU_REGION_ENABLE;
	MPU_InitStruct.Number = MPU_REGION_NUMBER0;
	MPU_InitStruct.BaseAddress = 0x60000000;
	MPU_InitStruct.Size = MPU_REGION_SIZE_1MB;
	MPU_InitStruct.SubRegionDisable = 0x0;
	MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
	MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
	MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
	MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

	HAL_MPU_ConfigRegion(&MPU_InitStruct);

	/** Initializes and configures the Region and the memory to be protected
	 */
	MPU_InitStruct.Number = MPU_REGION_NUMBER2;
	MPU_InitStruct.BaseAddress = 0x38800000;
	MPU_InitStruct.Size = MPU_REGION_SIZE_4KB;

	HAL_MPU_ConfigRegion(&MPU_InitStruct);
	/* Enables the MPU */
	HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	println("error handled");
	/*while (1)
	{
	  LCD_showError("Error handled", true);
	}*/
	// LCD_showError("Error handled", true);
	/* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line) {
	/* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
	   tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	printf("Wrong parameters value: file %s on line %u\r\n", file, line);
	/* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
