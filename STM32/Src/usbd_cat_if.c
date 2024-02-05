#include "usbd_cat_if.h"
#include "audio_filters.h"
#include "cw.h"
#include "fpga.h"
#include "functions.h"
#include "lcd.h"
#include "settings.h"
#include "trx_manager.h"
#include "vad.h"
#include "wifi.h"
#include <stdlib.h>

#define CAT_APP_RX_DATA_SIZE CDC_DATA_FS_MAX_PACKET_SIZE
#define CAT_APP_TX_DATA_SIZE CDC_DATA_FS_MAX_PACKET_SIZE

#define CAT_BUFFER_SIZE 64
static char cat_buffer[CAT_BUFFER_SIZE] = {0};
static uint8_t cat_buffer_head = 0;
static char command_to_parse1[CAT_BUFFER_SIZE] = {0};
static char command_to_parse2[CAT_BUFFER_SIZE] = {0};
static uint8_t CAT_UserRxBufferFS[CAT_APP_RX_DATA_SIZE];
static uint8_t CAT_UserTxBufferFS[CAT_APP_TX_DATA_SIZE];
static bool CAT_processingWiFiCommand = false;
static uint32_t CAT_processingWiFi_link_id = 0;
static uint8_t lineCoding[7] = {0x00, 0xC2, 0x01, 0x00, 0x00, 0x00, 0x08}; // 115200bps, 1stop, no parity, 8bit

extern USBD_HandleTypeDef hUsbDeviceFS;

static void getFT450Mode(uint8_t VFO_Mode, char *out);
static void getTS2000Mode(uint8_t VFO_Mode, char *out);
static uint8_t setFT450Mode(char *FT450_Mode);
static uint8_t setTS2000Mode(char *TS2000_Mode);
static int8_t CAT_Init_FS(void);
static int8_t CAT_DeInit_FS(void);
static int8_t CAT_Control_FS(uint8_t cmd, uint8_t *pbuf, uint32_t len);
static int8_t CAT_Receive_FS(uint8_t *pbuf, uint32_t *Len);
static void CAT_Transmit(char *data);
static uint8_t CAT_Transmit_FS(uint8_t *Buf, uint16_t Len);

#if HRDW_HAS_USB_CAT
USBD_CAT_ItfTypeDef USBD_CAT_fops_FS = {CAT_Init_FS, CAT_DeInit_FS, CAT_Control_FS, CAT_Receive_FS};
#endif

static int8_t CAT_Init_FS(void) {
/* USER CODE BEGIN 3 */
/* Set Application Buffers */
#if HRDW_HAS_USB_CAT
	USBD_CAT_SetTxBuffer(&hUsbDeviceFS, CAT_UserTxBufferFS, 0);
	USBD_CAT_SetRxBuffer(&hUsbDeviceFS, CAT_UserRxBufferFS);
#endif
	return (USBD_OK);
	/* USER CODE END 3 */
}

/**
 * @brief  DeInitializes the CDC media low layer
 * @retval USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t CAT_DeInit_FS(void) {
	/* USER CODE BEGIN 4 */
	return (USBD_OK);
	/* USER CODE END 4 */
}

/**
 * @brief  Manage the CDC class requests
 * @param  cmd: Command code
 * @param  pbuf: Buffer containing command data (request parameters)
 * @param  length: Number of data to be sent (in bytes)
 * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t CAT_Control_FS(uint8_t cmd, uint8_t *pbuf, uint32_t len) {
	/* USER CODE BEGIN 5 */
	bool dtr = false;
	bool rts = false;

	switch (cmd) {
	case CDC_SEND_ENCAPSULATED_COMMAND:
		break;

	case CDC_GET_ENCAPSULATED_RESPONSE:
		break;

	case CDC_SET_COMM_FEATURE:
		break;

	case CDC_GET_COMM_FEATURE:
		break;

	case CDC_CLEAR_COMM_FEATURE:
		break;

		/*******************************************************************************/
		/* Line Coding Structure                                                       */
		/*-----------------------------------------------------------------------------*/
		/* Offset | Field       | Size | Value  | Description                          */
		/* 0      | dwDTERate   |   4  | Number |Data terminal rate, in bits per second*/
		/* 4      | bCharFormat |   1  | Number | Stop bits                            */
		/*                                        0 - 1 Stop bit                       */
		/*                                        1 - 1.5 Stop bits                    */
		/*                                        2 - 2 Stop bits                      */
		/* 5      | bParityType |  1   | Number | Parity                               */
		/*                                        0 - None                             */
		/*                                        1 - Odd                              */
		/*                                        2 - Even                             */
		/*                                        3 - Mark                             */
		/*                                        4 - Space                            */
		/* 6      | bDataBits  |   1   | Number Data bits (5, 6, 7, 8 or 16).          */
		/*******************************************************************************/
	case CDC_SET_LINE_CODING:
		dma_memcpy(lineCoding, pbuf, sizeof(lineCoding));
		break;

	case CDC_GET_LINE_CODING:
		dma_memcpy(pbuf, lineCoding, sizeof(lineCoding));
		break;

	case CDC_SET_CONTROL_LINE_STATE:
		dtr = (pbuf[2] & 0x1) == 0x1;
		rts = (pbuf[2] & 0x2) == 0x2;

		if (CALIBRATE.COM_CAT_DTR_Mode == COM_LINE_MODE_KEYER) {
			CW_key_serial = dtr;
		}
		if (CALIBRATE.COM_CAT_RTS_Mode == COM_LINE_MODE_KEYER) {
			CW_key_serial = rts;
		}

		if (CALIBRATE.COM_CAT_DTR_Mode == COM_LINE_MODE_PTT) {
			if (dtr && !CW_key_serial && !TRX_ptt_soft) {
				TRX_ptt_soft = true;
			} else if (!dtr && !CW_key_serial && TRX_ptt_soft) {
				TRX_ptt_soft = false;
			}
		}
		if (CALIBRATE.COM_CAT_RTS_Mode == COM_LINE_MODE_PTT) {
			if (rts && !CW_key_serial && !TRX_ptt_soft) {
				TRX_ptt_soft = true;
			} else if (!rts && !CW_key_serial && TRX_ptt_soft) {
				TRX_ptt_soft = false;
			}
		}
		break;

	case CDC_SEND_BREAK:
		break;

	default:
		break;
	}

	return (USBD_OK);
	/* USER CODE END 5 */
}

/**
 * @brief  Data received over USB OUT endpoint are sent over CDC interface
 *         through this function.
 *
 *         @note
 *         This function will block any OUT packet reception on USB endpoint
 *         untill exiting this function. If you exit this function before transfer
 *         is complete on CDC interface (ie. using DMA controller) it will result
 *         in receiving more data while previous ones are still not sent.
 *
 * @param  Buf: Buffer of data to be received
 * @param  Len: Number of data received (in bytes)
 * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t CAT_Receive_FS(uint8_t *Buf, uint32_t *Len) {
	char charBuff[CAT_BUFFER_SIZE] = {0};
	strncpy(charBuff, (char *)Buf, Len[0]);
	dma_memset(&Buf, 0, Len[0]);
	// println((char *)charBuff);
	if (Len[0] <= CAT_BUFFER_SIZE) {
		for (uint16_t i = 0; i < Len[0]; i++) {
			if (charBuff[i] != 0) {
				cat_buffer[cat_buffer_head] = charBuff[i];
				if (cat_buffer[cat_buffer_head] == ';') {
					CAT_processingWiFiCommand = false;
					if (strlen(command_to_parse1) == 0) {
						dma_memset(command_to_parse1, 0, sizeof(command_to_parse1));
						dma_memcpy(command_to_parse1, cat_buffer, cat_buffer_head);
					} else if (strlen(command_to_parse2) == 0) {
						dma_memset(command_to_parse2, 0, sizeof(command_to_parse2));
						dma_memcpy(command_to_parse2, cat_buffer, cat_buffer_head);
					}

					cat_buffer_head = 0;
					dma_memset(cat_buffer, 0, CAT_BUFFER_SIZE);
					continue;
				}
				cat_buffer_head++;
				if (cat_buffer_head >= CAT_BUFFER_SIZE) {
					cat_buffer_head = 0;
					dma_memset(cat_buffer, 0, CAT_BUFFER_SIZE);
				}
			}
		}
	}
	return (USBD_OK);
}

/**
 * @brief  CDC_Transmit_FS
 *         Data to send over USB IN endpoint are sent over CDC interface
 *         through this function.
 *         @note
 *
 *
 * @param  Buf: Buffer of data to be sent
 * @param  Len: Number of data to be sent (in bytes)
 * @retval USBD_OK if all operations are OK else USBD_FAIL or USBD_BUSY
 */
static uint8_t CAT_Transmit_FS(uint8_t *Buf, uint16_t Len) {
	uint8_t result = USBD_OK;

#if HRDW_HAS_USB_CAT
	USBD_CAT_HandleTypeDef *hcdc = (USBD_CAT_HandleTypeDef *)hUsbDeviceFS.pClassDataCAT;
	if (hcdc->TxState != 0) {
		return USBD_BUSY;
	}
	USBD_CAT_SetTxBuffer(&hUsbDeviceFS, Buf, Len);
	result = USBD_CAT_TransmitPacket(&hUsbDeviceFS);
#endif

	return result;
}

static void CAT_Transmit(char *data) {
	CAT_Transmit_FS((uint8_t *)data, (uint16_t)strlen(data));
	if (TRX.Debug_Type == TRX_DEBUG_CAT) {
		println("CAT command answer: |", data, "|");
	}
	if (CAT_processingWiFiCommand) {
#if HRDW_HAS_WIFI
		WIFI_SendCatAnswer(data, CAT_processingWiFi_link_id, NULL);
#endif
		CAT_processingWiFiCommand = false;
	}
}

#if HRDW_DEBUG_ON_CAT_PORT

#ifdef STM32H743xx
#define CAT_TX_FIFO_BUFFER_SIZE 4096
#endif
#ifdef STM32F407xx
#define CAT_TX_FIFO_BUFFER_SIZE 256
#endif

static uint16_t CAT_tx_fifo_head = 0;
static uint16_t CAT_tx_fifo_tail = 0;
static bool FIFO_Events_busy = false;
static SRAM uint8_t CAT_tx_fifo[CAT_TX_FIFO_BUFFER_SIZE] = {0};
static SRAM uint8_t CAT_temp_buff[CAT_TX_FIFO_BUFFER_SIZE] = {0};

void CAT_Transmit_FIFO(uint8_t *data, uint16_t length) {
	if (length <= CAT_TX_FIFO_BUFFER_SIZE) {
		for (uint16_t i = 0; i < length; i++) {
			// buffer full
			if ((CAT_tx_fifo_head + 1) == CAT_tx_fifo_tail) {
				uint_fast16_t tryes = 0;
				while (CAT_Transmit_FIFO_Events() != USBD_OK && tryes < 512) {
					tryes++;
				}
				if (tryes >= 512) {
					return;
				}
			}

			// assign char buffer
			CAT_tx_fifo[CAT_tx_fifo_head] = data[i];
			CAT_tx_fifo_head++;

			if (CAT_tx_fifo_head >= CAT_TX_FIFO_BUFFER_SIZE) {
				CAT_tx_fifo_head = 0;
			}
		}
	}
}

uint8_t CAT_Transmit_FIFO_Events(void) {
	if (FIFO_Events_busy) {
		return USBD_FAIL;
	}

	if (CAT_tx_fifo_head == CAT_tx_fifo_tail) {
		return USBD_OK;
	}

	USBD_CAT_HandleTypeDef *hcdc = (USBD_CAT_HandleTypeDef *)hUsbDeviceFS.pClassDataCAT;
	if (hcdc->TxState != 0) {
		return USBD_FAIL;
	}

	FIFO_Events_busy = true;
	uint16_t count = 0;
	dma_memset(CAT_temp_buff, 0x00, CAT_TX_FIFO_BUFFER_SIZE);

	if (CAT_tx_fifo_tail > CAT_tx_fifo_head) {
		for (uint16_t i = CAT_tx_fifo_tail; i < CAT_TX_FIFO_BUFFER_SIZE; i++) {
			CAT_temp_buff[count] = CAT_tx_fifo[i];
			count++;
			CAT_tx_fifo_tail++;
			if (count == CAT_APP_TX_DATA_SIZE) {
				break;
			}
		}
		if (CAT_tx_fifo_tail == CAT_TX_FIFO_BUFFER_SIZE) {
			CAT_tx_fifo_tail = 0;
		}
	}

	if (CAT_tx_fifo_tail < CAT_tx_fifo_head && count < CAT_APP_TX_DATA_SIZE) {
		for (uint16_t i = CAT_tx_fifo_tail; i < CAT_tx_fifo_head; i++) {
			CAT_temp_buff[count] = CAT_tx_fifo[i];
			count++;
			CAT_tx_fifo_tail++;
			if (count == CAT_APP_TX_DATA_SIZE) {
				break;
			}
		}
	}

	CAT_Transmit_FS(CAT_temp_buff, count);
	FIFO_Events_busy = false;
	return USBD_BUSY;
}

#endif

#if HRDW_HAS_USB_CAT
void CAT_SetWIFICommand(char *data, uint32_t length, uint32_t link_id) {
	CAT_processingWiFiCommand = true;
	CAT_processingWiFi_link_id = link_id;
	dma_memset(&command_to_parse1, 0, CAT_BUFFER_SIZE);
	dma_memcpy(command_to_parse1, data, length);
	ua3reo_dev_cat_parseCommand();
}

void ua3reo_dev_cat_parseCommand(void) {
	USBD_CAT_ReceivePacket(&hUsbDeviceFS); // prepare next command
	if (command_to_parse1[0] == 0 && command_to_parse2[0] == 0) {
		return;
	}

	char _command_buffer[CAT_BUFFER_SIZE] = {0};
	char *_command = _command_buffer;
	if (strlen(command_to_parse1) > 0) {
		dma_memcpy(_command, command_to_parse1, CAT_BUFFER_SIZE);
		dma_memset(command_to_parse1, 0, CAT_BUFFER_SIZE);
	} else if (strlen(command_to_parse2) > 0) {
		dma_memcpy(_command, command_to_parse2, CAT_BUFFER_SIZE);
		dma_memset(command_to_parse2, 0, CAT_BUFFER_SIZE);
	}

	while (*_command == '\r' || *_command == '\n' || *_command == ' ') { // trim
		_command++;
	}
	if (strlen(_command) < 2) {
		return;
	}

	if (TRX.Debug_Type == TRX_DEBUG_CAT) {
		println("New CAT command: |", _command, "|");
	}

	char command[3] = {0};
	strncpy(command, _command, 2);
	bool has_args = false;
	char arguments[32] = {0};
	char ctmp[30] = {0};
	if (strlen(_command) > 2) {
		strncpy(arguments, _command + 2, strlen(_command) - 2);
		has_args = true;
	}
	// sendToDebug_str3("Arguments: |",arguments,"|\r\n");

	///////Yaesu FT-450 CAT///////
	// AI // AUTO INFORMATION
	// ID // IDENTIFICATION
	// FT // FUNCTION TX
	// VS // VFO SELECT
	// IF // INFORMATION
	// OI // OPPOSITE BAND INFORMATION
	// FA // FREQUENCY VFO-A
	// FB // FREQUENCY VFO-B
	// RA // RF ATTENUATOR  ??
	// PA // PRE-AMP        ??
	// PS // POWER-SWITCH
	// GT // AGC FUNCTION
	// MD // MODE
	// PC // POWER CONTROL
	// SH // WIDTH
	// NB // NOISE BLANKER   ??
	// NR // NOISE REDUCTION ??
	// VX // VOX STATUS
	// CT // CTCSS
	// ML // MONITOR LEVEL
	// BP // MANUAL NOTCH
	// BI // BREAK IN
	// OS // OFFSET
	// BS // BAND SELECT
	// NA // NARROW
	// SM // READ S-METER
	// KP // READ KEY PITCH
	// TX // TX SET
	//
	///////Kenwood TS-2000 CAT ///////
	// RX // Sets the receiver function status.
	// PS // Sets or reads the Power ON/ OFF status.
	// FW // filter width
	//

	if (strcmp(command, "AI") == 0) // AUTO INFORMATION
	{
		if (!has_args) {
			CAT_Transmit("AI0;");
		} else {
			// ничего не делаем, автоуведомления и так не работают
		}
		return;
	}

	if (strcmp(command, "ID") == 0) // IDENTIFICATION
	{
		if (!has_args) {
			if (CALIBRATE.CAT_Type == CAT_FT450) {
				CAT_Transmit("ID0241;");
			}

			if (CALIBRATE.CAT_Type == CAT_TS2000) {
				CAT_Transmit("ID019;");
			}
		} else {
			println("Unknown CAT arguments: ", _command);
		}
		return;
	}

	if (strcmp(command, "FT") == 0) // FUNCTION TX
	{
		if (!has_args) {
			if (!TRX.SPLIT_Enabled) {
				CAT_Transmit("FT0;");
			} else {
				CAT_Transmit("FT1;");
			}
		} else {
			if (strcmp(arguments, "0") == 0) {
				TRX.SPLIT_Enabled = false;
			} else if (strcmp(arguments, "1") == 0) {
				TRX.SPLIT_Enabled = true;
			} else {
				println("Unknown CAT arguments: ", _command);
			}
		}
		return;
	}

	if (strcmp(command, "VS") == 0) // VFO SELECT
	{
		if (!has_args) {
			if (!TRX.selected_vfo) {
				CAT_Transmit("VS0;");
			} else {
				CAT_Transmit("VS1;");
			}
		} else {
			uint8_t new_vfo = 0;
			if (strcmp(arguments, "0") == 0) {
				new_vfo = 0;
			} else if (strcmp(arguments, "1") == 0) {
				new_vfo = 1;
			}
			if (TRX.selected_vfo != new_vfo) {
				TRX.selected_vfo = new_vfo;
				if (!TRX.selected_vfo) {
					CurrentVFO = &TRX.VFO_A;
					SecondaryVFO = &TRX.VFO_B;
				} else {
					CurrentVFO = &TRX.VFO_B;
					SecondaryVFO = &TRX.VFO_A;
				}
				LCD_UpdateQuery.TopButtons = true;
				LCD_UpdateQuery.BottomButtons = true;
				LCD_UpdateQuery.FreqInfoRedraw = true;
				LCD_UpdateQuery.StatusInfoGUI = true;
				LCD_UpdateQuery.StatusInfoBarRedraw = true;
				NeedSaveSettings = true;
				NeedReinitAudioFiltersClean = true;
				NeedReinitAudioFilters = true;
				resetVAD();
				FFT_Init();
				TRX_ScanMode = false;
			}
			// println("CAT arguments: ", _command);
		}
		return;
	}

	if (strcmp(command, "IF") == 0) // INFORMATION
	{
		if (!has_args) {
			char answer[30] = {0};

			if (CALIBRATE.CAT_Type == CAT_FT450) {
				strcat(answer, "IF001"); // memory channel
				if (CurrentVFO->Freq < 10000000) {
					strcat(answer, "0");
				}
				sprintf(ctmp, "%llu", CurrentVFO->Freq);
				strcat(answer, ctmp); // freq

				if (TRX.XIT_Enabled) { // clirifier offset
					sprintf(ctmp, "%u", abs(TRX_XIT));
					addSymbols(ctmp, ctmp, 4, "0", false);
					if (TRX_XIT < 0) {
						strcat(answer, "-");
					} else {
						strcat(answer, "+");
					}
					strcat(answer, ctmp);
				} else if (TRX.RIT_Enabled) {
					sprintf(ctmp, "%u", abs(TRX_RIT));
					addSymbols(ctmp, ctmp, 4, "0", false);
					if (TRX_RIT < 0) {
						strcat(answer, "-");
					} else {
						strcat(answer, "+");
					}
					strcat(answer, ctmp);
				} else {
					strcat(answer, "+0000");
				}

				if (TRX.RIT_Enabled) { // RX clar
					strcat(answer, "1");
				} else {
					strcat(answer, "0");
				}
				if (TRX.XIT_Enabled) { // TX clar
					strcat(answer, "1");
				} else {
					strcat(answer, "0");
				}
				char mode[3] = {0};
				getFT450Mode((uint8_t)CurrentVFO->Mode, mode);
				strcat(answer, mode); // mode
				strcat(answer, "0");  // VFO Memory
				strcat(answer, "0");  // CTCSS OFF
				strcat(answer, "00"); // TONE NUMBER
				strcat(answer, "0;"); // Simplex
			}

			if (CALIBRATE.CAT_Type == CAT_TS2000) {
				strcat(answer, "IF"); // TRX status
				sprintf(ctmp, "%llu", CurrentVFO->Freq);
				addSymbols(ctmp, ctmp, 11, "0", false);
				strcat(answer, ctmp);   // freq
				strcat(answer, "0000"); // Frequency step size

				if (TRX.XIT_Enabled) { // RIT/ XIT frequency ±99999 in Hz
					sprintf(ctmp, "%u", abs(TRX_XIT));
					addSymbols(ctmp, ctmp, 5, "0", false);
					if (TRX_XIT < 0) {
						strcat(answer, "-");
					} else {
						strcat(answer, "+");
					}
					strcat(answer, ctmp);
				} else if (TRX.RIT_Enabled) {
					sprintf(ctmp, "%u", abs(TRX_RIT));
					addSymbols(ctmp, ctmp, 5, "0", false);
					if (TRX_RIT < 0) {
						strcat(answer, "-");
					} else {
						strcat(answer, "+");
					}
					strcat(answer, ctmp);
				} else {
					strcat(answer, "000000");
				}

				if (TRX.RIT_Enabled) { // 0: RIT OFF, 1: RIT ON
					strcat(answer, "1");
				} else {
					strcat(answer, "0");
				}
				if (TRX.XIT_Enabled) { // 0: XIT OFF, 1: XIT ON
					strcat(answer, "1");
				} else {
					strcat(answer, "0");
				}
				strcat(answer, "0");  // 0: Always 0 for the TS-480 (Memory channel bank number).
				strcat(answer, "00"); // Memory channel number (00 ~ 99).
				if (TRX_on_TX) {      // 0: RX, 1: TX
					strcat(answer, "1");
				} else {
					strcat(answer, "0");
				}
				char mode[3] = {0};
				getTS2000Mode((uint8_t)CurrentVFO->Mode, mode);
				strcat(answer, mode);    // Operating mode
				if (!TRX.selected_vfo) { // 0: VFO A 1: VFO B 2: M.CH
					strcat(answer, "0");
				} else {
					strcat(answer, "1");
				}
				strcat(answer, "0"); // Scan status. P1/ P2 0: Scan OFF 1: Scan ON 4: Tone Scan ON 5: CTCSS Scan ON
				if (TRX.SPLIT_Enabled) {
					strcat(answer, "1"); // 0: Simplex operation, 1: Split operation
				} else {
					strcat(answer, "0"); // 0: Simplex operation, 1: Split operation
				}
				strcat(answer, "0");  // 0: OFF, 1: TONE, 2: CTCSS
				strcat(answer, "00"); // Tone number (00 ~ 42). Refer to the TN and CN command.
				strcat(answer, "0;"); // Shift status
			}

			CAT_Transmit(answer);
		} else {
			println("Unknown CAT arguments: ", _command);
		}
		return;
	}

	if (strcmp(command, "OI") == 0) // OPPOSITE BAND INFORMATION
	{
		if (!has_args) {
			char answer[30] = {0};
			strcat(answer, "OI001"); // memory channel
			if (SecondaryVFO->Freq < 10000000) {
				strcat(answer, "0");
			}
			sprintf(ctmp, "%llu", SecondaryVFO->Freq);
			strcat(answer, ctmp);    // freq
			strcat(answer, "+0000"); // clirifier offset
			strcat(answer, "0");     // RX clar off
			strcat(answer, "0");     // TX clar off
			char mode[3] = {0};
			getFT450Mode((uint8_t)SecondaryVFO->Mode, mode);
			strcat(answer, mode); // mode
			strcat(answer, "0");  // VFO Memory
			strcat(answer, "0");  // CTCSS OFF
			strcat(answer, "00"); // TONE NUMBER
			strcat(answer, "0;"); // Simplex
			CAT_Transmit(answer);
		} else {
			println("Unknown CAT arguments: ", _command);
		}
		return;
	}

	if (strcmp(command, "FA") == 0) // FREQUENCY VFO-A
	{
		if (!has_args) {
			char answer[30] = {0};
			strcat(answer, "FA");

			if (CALIBRATE.CAT_Type == CAT_FT450) {
				if (TRX.VFO_A.Freq < 10000000) {
					strcat(answer, "0");
				}
				sprintf(ctmp, "%llu", TRX.VFO_A.Freq);
			}

			if (CALIBRATE.CAT_Type == CAT_TS2000) {
				sprintf(ctmp, "%llu", TRX.VFO_A.Freq);
				addSymbols(ctmp, ctmp, 11, "0", false);
			}

			strcat(answer, ctmp); // freq
			strcat(answer, ";");
			CAT_Transmit(answer);
		} else {
			TRX_setFrequency((uint64_t)atoll(arguments), &TRX.VFO_A);
			LCD_UpdateQuery.FreqInfo = true;
			LCD_UpdateQuery.TopButtons = true;
		}
		return;
	}

	if (strcmp(command, "FB") == 0) // FREQUENCY VFO-B
	{
		if (!has_args) {
			char answer[30] = {0};
			strcat(answer, "FB");

			if (CALIBRATE.CAT_Type == CAT_FT450) {
				if (TRX.VFO_B.Freq < 10000000) {
					strcat(answer, "0");
				}
				sprintf(ctmp, "%llu", TRX.VFO_B.Freq);
			}

			if (CALIBRATE.CAT_Type == CAT_TS2000) {
				sprintf(ctmp, "%llu", TRX.VFO_B.Freq);
				addSymbols(ctmp, ctmp, 11, "0", false);
			}

			strcat(answer, ctmp); // freq
			strcat(answer, ";");
			CAT_Transmit(answer);
		} else {
			TRX_setFrequency((uint64_t)atoll(arguments), &TRX.VFO_B);
			LCD_UpdateQuery.FreqInfo = true;
			LCD_UpdateQuery.TopButtons = true;
		}
		return;
	}

	if (strcmp(command, "PA") == 0) // PRE-AMP
	{
		if (!has_args) {
			if (!TRX.LNA) {
				CAT_Transmit("PA0;");
			} else {
				CAT_Transmit("PA1;");
			}
		} else {
			if (strcmp(arguments, "0") == 0) {
				if (!TRX.LNA) {
					CAT_Transmit("PA0;");
				} else {
					CAT_Transmit("PA0;");
					BUTTONHANDLER_PRE(0);
				}
			} else {
				if (TRX.LNA) {
					CAT_Transmit("PA1;");
				} else {
					CAT_Transmit("PA1;");
					BUTTONHANDLER_PRE(0);
				}
			}
		}
		return;
	}

	if (strcmp(command, "RA") == 0) // RF ATTENUATOR
	{
		if (!has_args) {
			if (!TRX.ATT) {
				CAT_Transmit("RA00;");
			}
			if (TRX.ATT) {
				CAT_Transmit("RA11;");
			}
		} else {
			if ((strcmp(arguments, "0") == 0) || (strcmp(arguments, "00") == 0)) {
				if (!TRX.ATT) {
					CAT_Transmit("RA00;");
				} else {
					CAT_Transmit("RA00;");
					BUTTONHANDLER_ATT(0);
				}
			} else {
				if (TRX.ATT) {
					CAT_Transmit("RA11;");
				} else {
					CAT_Transmit("RA11;");
					BUTTONHANDLER_ATT(0);
				}
			}
		}
		return;
	}

	if (strcmp(command, "UP") == 0) // Freq 1 step UP
	{
		if (!has_args) {
			CAT_Transmit("UP;");
		} else {
			if (strcmp(arguments, "0") == 0) {
				CAT_Transmit("UP0;");
			} else if (strcmp(arguments, "1") == 0) {
				CAT_Transmit("UP1;");
				BUTTONHANDLER_UP(0);
			}
			return;
		}
	}
	if (strcmp(command, "DN") == 0) // Freq 1 step DOWN
	{
		if (!has_args) {
			CAT_Transmit("DN;");
		} else {
			if (strcmp(arguments, "0") == 0) {
				CAT_Transmit("DN0;");
			} else if (strcmp(arguments, "1") == 0) {
				CAT_Transmit("DN1;");
				BUTTONHANDLER_DOWN(0);
			}
			return;
		}
	}
	if (strcmp(command, "PS") == 0) // POWER-SWITCH
	{
		if (!has_args) {
			CAT_Transmit("PS1;");
		} else {
			if (strcmp(arguments, "0") == 0) {
				// power off
			} else if (strcmp(arguments, "1") == 0) {
				// power on
			} else {
				println("Unknown CAT arguments: ", _command);
			}
		}
		return;
	}

	if (strcmp(command, "GT") == 0) // AGC FUNCTION
	{
		if (!has_args) {
			println("Unknown CAT arguments: ", _command);
		} else {
			if (strcmp(arguments, "0") == 0) {
				if (TRX.RX_AGC_SSB_speed == 0 || !CurrentVFO->AGC) {
					CAT_Transmit("GT00;");
				} else if (TRX.RX_AGC_SSB_speed == 1) {
					CAT_Transmit("GT04;");
				} else if (TRX.RX_AGC_SSB_speed == 2) {
					CAT_Transmit("GT03;");
				} else if (TRX.RX_AGC_SSB_speed == 3) {
					CAT_Transmit("GT02;");
				} else {
					CAT_Transmit("GT01;");
				}
			} else {
				println("Unknown CAT arguments: ", _command);
			}
		}
		return;
	}

	if (strcmp(command, "MD") == 0) // MODE
	{
		if (!has_args) {
			if (CALIBRATE.CAT_Type == CAT_FT450) {
				println("Unknown CAT arguments: ", _command);
			}

			if (CALIBRATE.CAT_Type == CAT_TS2000) {
				char answer[30] = {0};
				strcat(answer, "MD");
				char mode[3] = {0};
				getTS2000Mode((uint8_t)CurrentVFO->Mode, mode);
				strcat(answer, mode); // mode
				strcat(answer, ";");
				CAT_Transmit(answer);
			}
		} else {
			if (strcmp(arguments, "0") == 0) {
				char answer[30] = {0};
				strcat(answer, "MD0");
				char mode[3] = {0};
				getFT450Mode((uint8_t)CurrentVFO->Mode, mode);
				strcat(answer, mode); // mode
				strcat(answer, ";");
				CAT_Transmit(answer);
			} else {
				if (CALIBRATE.CAT_Type == CAT_FT450) {
					if (CurrentVFO->Mode != setFT450Mode(arguments)) {
						TRX_setMode(setFT450Mode(arguments), CurrentVFO);
						LCD_UpdateQuery.TopButtons = true;
					}
				}

				if (CALIBRATE.CAT_Type == CAT_TS2000) {
					if (CurrentVFO->Mode != setTS2000Mode(arguments)) {
						TRX_setMode(setTS2000Mode(arguments), CurrentVFO);
						LCD_UpdateQuery.TopButtons = true;
					}
				}
			}
		}
		return;
	}

	if (strcmp(command, "PC") == 0) // POWER CONTROL
	{
		if (!has_args) {
			char answer[30] = {0};
			strcat(answer, "PC");
			sprintf(ctmp, "%d", TRX.RF_Gain);
			strcat(answer, ctmp);
			strcat(answer, ";");
			CAT_Transmit(answer);
		} else {
			println("Unknown CAT arguments: ", _command);
		}
		return;
	}

	if (strcmp(command, "SH") == 0) // WIDTH
	{
		if (!has_args) {
			println("Unknown CAT arguments: ", _command);
		} else {
			if (strcmp(arguments, "0") == 0) {
				CAT_Transmit("SH031;");
			}
		}
		return;
	}

	if (strcmp(command, "NB") == 0) // NOISE BLANKER
	{
		if (!has_args) {
			if (!TRX.NOISE_BLANKER1 && !TRX.NOISE_BLANKER2) {
				CAT_Transmit("NB0;");
			} else {
				CAT_Transmit("NB1;");
			}
		} else {
			if (strcmp(arguments, "0") == 0) {
				CAT_Transmit("NB0;");
				TRX.NOISE_BLANKER1 = false;
				TRX.NOISE_BLANKER2 = false;
			} else {
				CAT_Transmit("NB1;");
				TRX.NOISE_BLANKER1 = true;
				TRX.NOISE_BLANKER2 = true;
			}
			LCD_UpdateQuery.TopButtons = true;
			NeedSaveSettings = true;
		}
		return;
	}

	if (strcmp(command, "NR") == 0) // NOISE REDUCTION
	{
		char answer[30] = {0};
		if (!has_args) { // querry about NOISE REDUCTION
			strcat(answer, "NR");
			sprintf(ctmp, "%d", CurrentVFO->DNR_Type);
			strcat(answer, ctmp); // type
			strcat(answer, ";");
			CAT_Transmit(answer);
		}

		if (strcmp(arguments, "0") == 0) {
			CAT_Transmit("NR0;");
			BUTTONHANDLER_DNR(1); // DNR0
		}

		if (strcmp(arguments, "1") == 0) {
			CAT_Transmit("NR1;");
			BUTTONHANDLER_DNR(2); // DNR1
		}

		if (strcmp(arguments, "2") == 0) {
			CAT_Transmit("NR2;");
			BUTTONHANDLER_DNR(3); // DNR2
		}
		return;
	}

	if (strcmp(command, "NT") == 0) // NOISE REDUCTION
	{
		if (!has_args) { // querry about NOISE REDUCTION
			if (CurrentVFO->AutoNotchFilter == true) {
				CAT_Transmit("NT1;");
			} else {
				CAT_Transmit("NT0;");
			}
		} else {
			if (strcmp(arguments, "0") == 0) {
				if (CurrentVFO->AutoNotchFilter == false) {
					CAT_Transmit("NT0;");
				}

				if (CurrentVFO->AutoNotchFilter == true) {
					CAT_Transmit("NT0;");
					BUTTONHANDLER_NOTCH(0);
				}
			}
			if (strcmp(arguments, "1") == 0) {
				if (CurrentVFO->AutoNotchFilter == true) {
					CAT_Transmit("NT1;");
				}

				if (CurrentVFO->AutoNotchFilter == false) {
					CAT_Transmit("NT1;");
					BUTTONHANDLER_NOTCH(0);
				}
			}
		}
		return;
	}

	if (strcmp(command, "VX") == 0) // VOX STATUS
	{
		if (!has_args) {
			CAT_Transmit("VX0;");
		} else {
			println("Unknown CAT arguments: ", _command);
		}
		return;
	}

	if (strcmp(command, "CT") == 0) // CTCSS
	{
		if (!has_args) {
			println("Unknown CAT arguments: ", _command);
		} else {
			if (strcmp(arguments, "0") == 0) {
				CAT_Transmit("CT00;");
			} else {
				println("Unknown CAT arguments: ", _command);
			}
		}
		return;
	}

	if (strcmp(command, "ML") == 0) // MONITOR LEVEL
	{
		if (!has_args) {
			println("Unknown CAT arguments: ", _command);
		} else {
			if (strcmp(arguments, "0") == 0) {
				CAT_Transmit("ML00;");
			} else {
				println("Unknown CAT arguments: ", _command);
			}
		}
		return;
	}

	if (strcmp(command, "BP") == 0) // MANUAL NOTCH
	{
		if (!has_args) {
			println("Unknown CAT arguments: ", _command);
		} else {
			if (strcmp(arguments, "00") == 0) {
				CAT_Transmit("BP00000;");
			} else {
				println("Unknown CAT arguments: ", _command);
			}
		}
		return;
	}

	if (strcmp(command, "BI") == 0) // BREAK IN
	{
		if (!has_args) {
			CAT_Transmit("BI0;");
		} else {
			println("Unknown CAT arguments: ", _command);
		}
		return;
	}

	if (strcmp(command, "OS") == 0) // OFFSET
	{
		if (!has_args) {
			println("Unknown CAT arguments: ", _command);
		} else {
			if (strcmp(arguments, "0") == 0) {
				CAT_Transmit("OS00;");
			} else {
				println("Unknown CAT arguments: ", _command);
			}
		}
		return;
	}

	if (strcmp(command, "BS") == 0) // BAND SELECT
	{
		if (!has_args) {
		} else {
			int8_t band = -1;
			if (strcmp(arguments, "00") == 0) {
				band = BANDID_160m;
			} else if (strcmp(arguments, "01") == 0) {
				band = BANDID_80m;
			} else if (strcmp(arguments, "03") == 0) {
				band = BANDID_40m;
			} else if (strcmp(arguments, "04") == 0) {
				band = BANDID_30m;
			} else if (strcmp(arguments, "05") == 0) {
				band = BANDID_20m;
			} else if (strcmp(arguments, "06") == 0) {
				band = BANDID_17m;
			} else if (strcmp(arguments, "07") == 0) {
				band = BANDID_15m;
			} else if (strcmp(arguments, "08") == 0) {
				band = BANDID_12m;
			} else if (strcmp(arguments, "09") == 0) {
				band = BANDID_10m;
			} else if (strcmp(arguments, "10") == 0) {
				band = BANDID_6m;
			} else {
				println("Unknown CAT arguments: ", _command);
			}

			if (band > -1) {
				TRX_setFrequency(TRX.BANDS_SAVED_SETTINGS[band].Freq, CurrentVFO);
				TRX_setMode(TRX.BANDS_SAVED_SETTINGS[band].Mode, CurrentVFO);
				TRX_RestoreBandSettings(band);

				LCD_UpdateQuery.TopButtons = true;
				LCD_UpdateQuery.FreqInfoRedraw = true;
				LCD_UpdateQuery.StatusInfoBarRedraw = true;
				LCD_UpdateQuery.StatusInfoGUI = true;
				resetVAD();
				TRX_ScanMode = false;
			}
		}
		return;
	}

	if (strcmp(command, "NA") == 0) // NARROW
	{
		if (!has_args) {
			println("Unknown CAT arguments: ", _command);
		} else {
			if (strcmp(arguments, "0") == 0) {
				CAT_Transmit("NA00;");
			}
		}
		return;
	}

	if (strcmp(command, "SM") == 0) // READ S-METER
	{
		if (!has_args) {
			println("Unknown CAT arguments: ", _command);
		} else {
			if (strcmp(arguments, "0") == 0) {
				CAT_Transmit("SM0100;");
			}
		}
		return;
	}

	if (strcmp(command, "KP") == 0) // READ KEY PITCH
	{
		if (!has_args) {
			CAT_Transmit("KP04;");
		} else {
			println("Unknown CAT arguments: ", _command);
		}
		return;
	}

	if (strcmp(command, "TX") == 0) // TX SET
	{
		if (!has_args) {
			if (CALIBRATE.CAT_Type == CAT_FT450) {
				if (TRX_ptt_soft) {
					CAT_Transmit("TX1;");
				} else if (TRX_ptt_hard) {
					CAT_Transmit("TX2;");
				} else {
					CAT_Transmit("TX0;");
				}
			}

			if (CALIBRATE.CAT_Type == CAT_TS2000) {
				TRX_ptt_soft = true;
				LCD_UpdateQuery.StatusInfoBarRedraw = true;
				LCD_UpdateQuery.StatusInfoGUI = true;
			}
		} else {
			if (CALIBRATE.CAT_Type == CAT_FT450) {
				if (strcmp(arguments, "0") == 0) {
					TRX_ptt_soft = false;
				}
				if (strcmp(arguments, "1") == 0) {
					TRX_ptt_soft = true;
				}
			}

			LCD_UpdateQuery.StatusInfoBarRedraw = true;
			LCD_UpdateQuery.StatusInfoGUI = true;
		}
		return;
	}

	if (strcmp(command, "RX") == 0) // RX SET
	{
		if (!has_args) {
			TRX_ptt_soft = false;
			LCD_UpdateQuery.StatusInfoBarRedraw = true;
			LCD_UpdateQuery.StatusInfoGUI = true;
			CAT_Transmit("RX0;");
		}
		return;
	}

	if (strcmp(command, "FW") == 0) // Filter width
	{
		if (!has_args) {
			char answer[30] = {0};

			if (CALIBRATE.CAT_Type == CAT_TS2000) {
				strcat(answer, "FW");
				sprintf(ctmp, "%u", CurrentVFO->LPF_RX_Filter_Width);
				addSymbols(ctmp, ctmp, 4, "0", false);
				strcat(answer, ctmp);
				strcat(answer, ";");
			}

			CAT_Transmit(answer);
		} else {
			println("Unknown CAT arguments: ", _command);
		}
		return;
	}

	if (strcmp(command, "SA") == 0) // Sets or reads the Satellite mode status.
	{
		CAT_Transmit("SA0;");
		return;
	}

	println("Unknown CAT command: ", _command);
}
#endif

static void getFT450Mode(uint8_t VFO_Mode, char *out) {
	if (VFO_Mode == TRX_MODE_LSB) {
		strcpy(out, "1");
	}
	if (VFO_Mode == TRX_MODE_USB) {
		strcpy(out, "2");
	}
	if (VFO_Mode == TRX_MODE_IQ) {
		strcpy(out, "8");
	}
	if (VFO_Mode == TRX_MODE_CW) {
		strcpy(out, "3");
	}
	if (VFO_Mode == TRX_MODE_DIGI_L) {
		strcpy(out, "8");
	}
	if (VFO_Mode == TRX_MODE_DIGI_U || VFO_Mode == TRX_MODE_RTTY) {
		strcpy(out, "C");
	}
	if (VFO_Mode == TRX_MODE_NFM) {
		strcpy(out, "4");
	}
	if (VFO_Mode == TRX_MODE_WFM) {
		strcpy(out, "4");
	}
	if (VFO_Mode == TRX_MODE_AM || VFO_Mode == TRX_MODE_SAM_STEREO || VFO_Mode == TRX_MODE_SAM_LSB || VFO_Mode == TRX_MODE_SAM_USB) {
		strcpy(out, "5");
	}
	if (VFO_Mode == TRX_MODE_LOOPBACK) {
		strcpy(out, "8");
	}
}

static void getTS2000Mode(uint8_t VFO_Mode, char *out) {
	if (VFO_Mode == TRX_MODE_LSB) {
		strcpy(out, "1");
	}
	if (VFO_Mode == TRX_MODE_USB) {
		strcpy(out, "2");
	}
	if (VFO_Mode == TRX_MODE_CW) {
		strcpy(out, "3");
	}
	if (VFO_Mode == TRX_MODE_NFM || VFO_Mode == TRX_MODE_WFM) {
		strcpy(out, "4");
	}
	if (VFO_Mode == TRX_MODE_AM || VFO_Mode == TRX_MODE_SAM_STEREO || VFO_Mode == TRX_MODE_SAM_LSB || VFO_Mode == TRX_MODE_SAM_USB) {
		strcpy(out, "5");
	}
	if (VFO_Mode == TRX_MODE_DIGI_L) {
		strcpy(out, "6");
	}
	if (VFO_Mode == TRX_MODE_DIGI_U || VFO_Mode == TRX_MODE_RTTY) {
		strcpy(out, "9");
	}
	if (VFO_Mode == TRX_MODE_IQ || VFO_Mode == TRX_MODE_LOOPBACK) {
		strcpy(out, "0");
	}
}

static uint8_t setFT450Mode(char *FT450_Mode) {
	if (strcmp(FT450_Mode, "01") == 0 || strcmp(FT450_Mode, "1") == 0) {
		return TRX_MODE_LSB;
	}
	if (strcmp(FT450_Mode, "02") == 0 || strcmp(FT450_Mode, "2") == 0) {
		return TRX_MODE_USB;
	}
	if (strcmp(FT450_Mode, "08") == 0 || strcmp(FT450_Mode, "8") == 0) {
		return TRX_MODE_IQ;
	}
	if (strcmp(FT450_Mode, "03") == 0 || strcmp(FT450_Mode, "3") == 0) {
		return TRX_MODE_CW;
	}
	if (strcmp(FT450_Mode, "06") == 0 || strcmp(FT450_Mode, "6") == 0) {
		return TRX_MODE_DIGI_L;
	}
	if (strcmp(FT450_Mode, "09") == 0 || strcmp(FT450_Mode, "9") == 0) {
		return TRX_MODE_DIGI_U;
	}
	if (strcmp(FT450_Mode, "0C") == 0 || strcmp(FT450_Mode, "C") == 0) {
		return TRX_MODE_DIGI_U;
	}
	if (strcmp(FT450_Mode, "04") == 0 || strcmp(FT450_Mode, "4") == 0) {
		return TRX_MODE_NFM;
	}
	if (strcmp(FT450_Mode, "05") == 0 || strcmp(FT450_Mode, "5") == 0) {
		return TRX_MODE_SAM_STEREO;
	}
	println("Unknown mode ", FT450_Mode);
	return TRX_MODE_USB;
}

static uint8_t setTS2000Mode(char *TS2000_Mode) {
	if (strcmp(TS2000_Mode, "1") == 0) {
		return TRX_MODE_LSB;
	}
	if (strcmp(TS2000_Mode, "2") == 0) {
		return TRX_MODE_USB;
	}
	if (strcmp(TS2000_Mode, "3") == 0) {
		return TRX_MODE_CW;
	}
	if (strcmp(TS2000_Mode, "4") == 0) {
		return TRX_MODE_NFM;
	}
	if (strcmp(TS2000_Mode, "5") == 0) {
		return TRX_MODE_SAM_STEREO;
	}
	if (strcmp(TS2000_Mode, "6") == 0) {
		return TRX_MODE_DIGI_L;
	}
	if (strcmp(TS2000_Mode, "8") == 0) {
		return TRX_MODE_IQ;
	}
	if (strcmp(TS2000_Mode, "9") == 0 || strcmp(TS2000_Mode, "D") == 0) {
		return TRX_MODE_DIGI_U;
	}
	println("Unknown mode ", TS2000_Mode);
	return TRX_MODE_USB;
}
