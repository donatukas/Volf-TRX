#include "hardware.h"

#if HRDW_HAS_WIFI

#include "bands.h"
#include "callsign.h"
#include "fpga.h"
#include "functions.h"
#include "lcd.h"
#include "locator.h"
#include "main.h"
#include "sd.h"
#include "settings.h"
#include "system_menu.h"
#include "trx_manager.h"
#include "usbd_cat_if.h"
#include "wifi.h"
#include <stdlib.h>

static WiFiProcessingCommand WIFI_ProcessingCommand = WIFI_COMM_NONE;
static void (*WIFI_ProcessingCommandCallback)(void);

static SRAM char WIFI_AnswerBuffer[WIFI_ANSWER_BUFFER_SIZE] = {0};
static SRAM char WIFI_readedLine[WIFI_LINE_BUFFER_SIZE] = {0};
static SRAM char tmp[WIFI_LINE_BUFFER_SIZE] = {0};
static SRAM int16_t WIFI_RLEStreamBuffer[WIFI_RLE_BUFFER_SIZE] = {0};
static SRAM uint16_t WIFI_RLEStreamBuffer_index_full = 0;
static SRAM uint16_t WIFI_RLEStreamBuffer_index_partial = 0;
static uint32_t WIFI_Answer_ReadIndex = 0;
static uint32_t commandStartTime = 0;
static uint8_t WIFI_FoundedAP_Index = 0;
static bool WIFI_stop_auto_ap_list = false;
static uint8_t get_HTTP_tryes = 0;
static uint8_t post_HTTP_tryes = 0;

DXCLUSTER_ENTRY WIFI_DXCLUSTER_list[WIFI_DXCLUSTER_MAX_RECORDS] = {0};
uint16_t WIFI_DXCLUSTER_list_count = 0;
WOLFCLUSTER_ENTRY WIFI_WOLFCLUSTER_list[WIFI_WOLFCLUSTER_MAX_RECORDS] = {0};
uint16_t WIFI_WOLFCLUSTER_list_count = 0;

static void WIFI_SendCommand(char *command);
static bool WIFI_WaitForOk(void);
static bool WIFI_ListAP_Sync(void);
static bool WIFI_TryGetLine(void);
static void WIFI_sendHTTPGetRequest(void);
static void WIFI_sendHTTPPostRequest(void);
static void WIFI_getHTTPResponse(void);
static void WIFI_printImage_stream_callback(void);
static void WIFI_printImage_stream_partial_callback(void);
static void WIFI_downloadFileToSD_callback(void);

bool WIFI_connected = false;
bool WIFI_CAT_server_started = false;
volatile WiFiState WIFI_State = WIFI_UNDEFINED;
static char WIFI_FoundedAP_InWork[WIFI_FOUNDED_AP_MAXCOUNT][MAX_WIFIPASS_LENGTH] = {0};
volatile char WIFI_FoundedAP[WIFI_FOUNDED_AP_MAXCOUNT][MAX_WIFIPASS_LENGTH] = {0};
bool WIFI_IP_Gotted = false;
char WIFI_IP[15] = {0};
char WIFI_AP[MAX_WIFIPASS_LENGTH] = {0};
static uint16_t WIFI_HTTP_Response_Status = 0;
static uint32_t WIFI_HTTP_Response_ContentLength = 0;
SRAM static char WIFI_HOSTuri[128] = {0};
SRAM static char WIFI_GETuri[128] = {0};
SRAM static char WIFI_HTTRequest[400] = {0};
SRAM4 static char WIFI_POSTdata[300] = {0};
SRAM4 static char WIFI_HTTResponseHTML[WIFI_HTML_RESP_BUFFER_SIZE] = {0};
bool WIFI_NewFW_checked = false;
bool WIFI_NewFW_STM32 = false;
bool WIFI_NewFW_FPGA = false;
bool WIFI_download_inprogress = false;
bool WIFI_downloadFileToSD_compleated = false;
bool WIFI_maySendIQ = false;

void WIFI_Init(void) {
	static uint8_t init_version = 0;
	WIFI_State = WIFI_UNDEFINED;

	if (init_version == 0) {
		HRDW_WIFI_UART.Init.BaudRate = 115200;
		HRDW_WIFI_UART.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
		HRDW_WIFI_UART.AdvancedInit.Swap = UART_ADVFEATURE_SWAP_DISABLE;
	}

	if (init_version == 1) {
		HRDW_WIFI_UART.Init.BaudRate = 115200;
		HRDW_WIFI_UART.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_SWAP_INIT;
		HRDW_WIFI_UART.AdvancedInit.Swap = UART_ADVFEATURE_SWAP_ENABLE;
	}

	HAL_UART_Init(&HRDW_WIFI_UART);

	// wifi uart speed = 115200 * 8 = 921600  / * 16 = 1843200
	WIFI_SendCommand("AT+UART_CUR=921600,8,1,0,1\r\n"); // uart config
	HAL_Delay(100);
	HRDW_WIFI_UART.Init.BaudRate = 921600;
	HAL_UART_Init(&HRDW_WIFI_UART);

	WIFI_SendCommand("ATE0\r\n"); // echo off
	WIFI_WaitForOk();

	if (strstr(WIFI_readedLine, "OK") != NULL) {
		WIFI_SendCommand("AT+GMR\r\n"); // system info ESP8266
		/*
		AT version:1.7.5.0(Oct 20 2021 19:14:04)
		SDK version:3.0.5(b29dcd3)
		compile time:Oct 20 2021 20:13:50
		Bin version(Wroom 02):1.7.5
		*/
		bool has_correct_sdk_version = false;
		char *sep = "SDK version:3";
		uint32_t startTime = HAL_GetTick();
		while ((HAL_GetTick() - startTime) < WIFI_COMMAND_TIMEOUT) {
			if (WIFI_TryGetLine()) {
				// OK
				char *istr = strstr(WIFI_readedLine, sep);
				if (istr != NULL) {
					has_correct_sdk_version = true;
					break;
				}
			}
			CPULOAD_GoToSleepMode();
			CPULOAD_WakeUp();
		}

		WIFI_SendCommand("AT\r\n");
		WIFI_WaitForOk();

		println("[WIFI] WIFI Module Inited");
		WIFI_State = WIFI_INITED;

		if (!has_correct_sdk_version) {
			LCD_showError("Wrong ESP FW Version", true);
		}

		// check if there are active connections, if yes - don't create a new one
		WIFI_SendCommand("AT+CIPSTATUS\r\n");
		while (WIFI_TryGetLine()) {
			if (strstr(WIFI_readedLine, "STATUS:2") != NULL) {
				WIFI_State = WIFI_READY;
				WIFI_connected = true;
				LCD_UpdateQuery.StatusInfoGUI = true;
				println("[WIFI] Connected");
			}
		}
	} else if (WIFI_State != WIFI_UNDEFINED) {
		WIFI_State = WIFI_REINIT;
	}

	if (WIFI_State == WIFI_UNDEFINED) {
		if (init_version == 1) {
			WIFI_State = WIFI_NOTFOUND;
			println("[WIFI] WIFI Module Not Found");
		} else {
			println("[WIFI] Trying next init params");
			init_version++;
			WIFI_State = WIFI_REINIT;
		}
	}
}

void WIFI_Process(void) {
	static IRAM2 char com_t[128] = {0};
	static IRAM2 char com[128] = {0};
	dma_memset(com_t, 0x00, sizeof(com_t));
	dma_memset(com, 0x00, sizeof(com));

	if (WIFI_State == WIFI_NOTFOUND) {
		return;
	}
	if (WIFI_State == WIFI_UNDEFINED || WIFI_State == WIFI_REINIT) {
		WIFI_Init();
		return;
	}
	// println(WIFI_State, " ",WIFI_PROCESS_COMMAND);
	/////////

	switch (WIFI_State) {
	case WIFI_INITED:
		// sendToDebug_str3("[WIFI] Start connecting to AP: ", TRX.WIFI_AP, "\r\n");
		WIFI_SendCommand("AT+CWAUTOCONN=0\r\n"); // AUTOCONNECT OFF
		WIFI_WaitForOk();
		WIFI_SendCommand("AT+RFPOWER=82\r\n"); // rf power
		WIFI_WaitForOk();
		WIFI_SendCommand("AT+CWMODE=1\r\n"); // station mode
		WIFI_WaitForOk();
		WIFI_SendCommand("AT+CWDHCP=1,1\r\n"); // DHCP
		WIFI_WaitForOk();
		WIFI_SendCommand("AT+CIPDNS_CUR=1,\"8.8.8.8\",\"77.88.8.8\"\r\n"); // DNS
		// WIFI_WaitForOk();
		WIFI_SendCommand("AT+CWHOSTNAME=\"TRX-Wolf\"\r\n"); // Hostname
		WIFI_WaitForOk();
		// WIFI_SendCommand("AT+CWCOUNTRY=1,\"RU\",1,13\r\n"); //Country
		// WIFI_WaitForOk();
		// WIFI_SendCommand("AT+CIPSERVER=0\r\n"); //Stop CAT Server
		// WIFI_WaitForOk();
		WIFI_SendCommand("AT+CIPMUX=1\r\n"); // Multiple server connections
		WIFI_WaitForOk();
		WIFI_SendCommand("AT+CIPSERVERMAXCONN=3\r\n"); // Max server connections
		WIFI_WaitForOk();
		WIFI_SendCommand("AT+CIPSSLSIZE=2048\r\n"); // SSL size
		WIFI_WaitForOk();
		WIFI_SendCommand("AT+CIPSSLCCONF=0\r\n"); // SSL config
		WIFI_WaitForOk();
		WIFI_SendCommand("AT+CIPRECVMODE=0\r\n"); // TCP receive passive mode
		WIFI_WaitForOk();

		strcat(com_t, "AT+CIPSNTPCFG=1,0,\"0.pool.ntp.org\",\"1.pool.ntp.org\"\r\n");
		WIFI_SendCommand(com_t); // configure SNMP
		WIFI_WaitForOk();

		WIFI_stop_auto_ap_list = false;
		WIFI_IP_Gotted = false;
		WIFI_State = WIFI_CONFIGURED;
		break;
	case WIFI_CONFIGURED:
		if (strcmp(WIFI.AP_1, "WIFI-AP") == 0 && strcmp(WIFI.AP_2, "WIFI-AP") == 0 && strcmp(WIFI.AP_3, "WIFI-AP") == 0 && strcmp(WIFI.Password_1, "WIFI-PASSWORD") == 0 &&
		    strcmp(WIFI.Password_2, "WIFI-PASSWORD") == 0 && strcmp(WIFI.Password_3, "WIFI-PASSWORD") == 0) {
			break;
		}
		if (WIFI_stop_auto_ap_list) {
			break;
		}
		WIFI_ListAP_Sync();
		bool AP1_exist = false;
		bool AP2_exist = false;
		bool AP3_exist = false;
		for (uint8_t i = 0; i < WIFI_FOUNDED_AP_MAXCOUNT; i++) {
			if (strcmp((char *)WIFI_FoundedAP[i], WIFI.AP_1) == 0) {
				AP1_exist = true;
			} else if (strcmp((char *)WIFI_FoundedAP[i], WIFI.AP_2) == 0) {
				AP2_exist = true;
			} else if (strcmp((char *)WIFI_FoundedAP[i], WIFI.AP_3) == 0) {
				AP3_exist = true;
			}
		}
		if (AP1_exist && strlen(WIFI.AP_1) > 0 && strlen(WIFI.Password_1) > 5) {
			println("[WIFI] Start connecting to AP1: ", WIFI.AP_1);
			strcat(com, "AT+CWJAP_CUR=\"");
			strcat(com, WIFI.AP_1);
			strcat(com, "\",\"");
			strcat(com, trim(WIFI.Password_1));
			strcat(com, "\"\r\n");
			WIFI_SendCommand(com); // connect to AP
			// WIFI_WaitForOk();
			WIFI_State = WIFI_CONNECTING;
			strcpy(WIFI_AP, WIFI.AP_1);
		}
		if (AP2_exist && !AP1_exist && strlen(WIFI.AP_2) > 0 && strlen(WIFI.Password_2) > 5) {
			println("[WIFI] Start connecting to AP2: ", WIFI.AP_2);
			strcat(com, "AT+CWJAP_CUR=\"");
			strcat(com, WIFI.AP_2);
			strcat(com, "\",\"");
			strcat(com, trim(WIFI.Password_2));
			strcat(com, "\"\r\n");
			WIFI_SendCommand(com); // connect to AP
			// WIFI_WaitForOk();
			WIFI_State = WIFI_CONNECTING;
			strcpy(WIFI_AP, WIFI.AP_2);
		}
		if (AP3_exist && !AP1_exist && !AP2_exist && strlen(WIFI.AP_3) > 0 && strlen(WIFI.Password_3) > 5) {
			println("[WIFI] Start connecting to AP: ", WIFI.AP_3);
			strcat(com, "AT+CWJAP_CUR=\"");
			strcat(com, WIFI.AP_3);
			strcat(com, "\",\"");
			strcat(com, trim(WIFI.Password_3));
			strcat(com, "\"\r\n");
			WIFI_SendCommand(com); // connect to AP
			// WIFI_WaitForOk();
			WIFI_State = WIFI_CONNECTING;
			strcpy(WIFI_AP, WIFI.AP_3);
		}
		break;

	case WIFI_CONNECTING:
		WIFI_TryGetLine();
		if (strstr(WIFI_readedLine, "GOT IP") != NULL) {
			println("[WIFI] Connected");
			WIFI_State = WIFI_READY;
			WIFI_connected = true;
			LCD_UpdateQuery.StatusInfoGUI = true;
		}
		if (strstr(WIFI_readedLine, "WIFI DISCONNECT") != NULL) {
			println("[WIFI] Disconnected");
			WIFI_State = WIFI_CONFIGURED;
			WIFI_connected = false;
			LCD_UpdateQuery.StatusInfoGUI = true;
			WIFI_SW_Restart(NULL);
			return;
		}
		if (strstr(WIFI_readedLine, "FAIL") != NULL) {
			println("[WIFI] Connect failed");
			WIFI_State = WIFI_CONFIGURED;
			WIFI_connected = false;
			LCD_UpdateQuery.StatusInfoGUI = true;
		}
		if (strstr(WIFI_readedLine, "ERROR") != NULL) {
			println("[WIFI] Connect error");
			WIFI_State = WIFI_CONFIGURED;
			WIFI_connected = false;
			LCD_UpdateQuery.StatusInfoGUI = true;
		}
		break;

	case WIFI_READY:
		WIFI_TryGetLine();
		// WIFI_ProcessingCommandCallback = 0;
		// receive commands from WIFI clients
		if (strstr(WIFI_readedLine, "+IPD") != NULL && WIFI_ProcessingCommand != WIFI_COMM_TCP_GET_RESPONSE && WIFI_CAT_server_started) {
			char *wifi_incoming_link_id = strchr(WIFI_readedLine, ',');
			if (wifi_incoming_link_id == NULL) {
				break;
			}
			wifi_incoming_link_id++;

			char *wifi_incoming_length = strchr(wifi_incoming_link_id, ',');
			if (wifi_incoming_length == NULL) {
				break;
			}
			*wifi_incoming_length = 0x00;
			wifi_incoming_length++;

			char *wifi_incoming_data = strchr(wifi_incoming_length, ':');
			if (wifi_incoming_data == NULL) {
				break;
			}
			*wifi_incoming_data = 0x00;
			wifi_incoming_data++;

			uint32_t wifi_incoming_length_uint = (uint32_t)atoi(wifi_incoming_length);
			uint32_t wifi_incoming_link_id_uint = (uint32_t)atoi(wifi_incoming_link_id);
			if (wifi_incoming_length_uint > 64) {
				wifi_incoming_length_uint = 64;
			}
			if (wifi_incoming_length_uint > 0) {
				wifi_incoming_length_uint--; // del /n char
			}
			if (wifi_incoming_link_id_uint > 8) {
				wifi_incoming_link_id_uint = 8;
			}

			char *wifi_incoming_data_end = wifi_incoming_data + wifi_incoming_length_uint;
			*wifi_incoming_data_end = 0x00;

			if (TRX.Debug_Type == TRX_DEBUG_WIFI) {
				println("[WIFI] Command received: ", wifi_incoming_data);
			}
			if (wifi_incoming_length_uint > 0) {
				CAT_SetWIFICommand(wifi_incoming_data, wifi_incoming_length_uint, wifi_incoming_link_id_uint);
			}
		}
		if (strstr(WIFI_readedLine, "WIFI DISCONNECT") != NULL) {
			println("[WIFI] Disconnected");
			WIFI_State = WIFI_CONFIGURED;
			WIFI_connected = false;
			WIFI_IP_Gotted = false;
			LCD_UpdateQuery.StatusInfoGUI = true;
			WIFI_SW_Restart(NULL);
			return;
		}
		break;

	case WIFI_TIMEOUT:
		WIFI_TryGetLine();
		if (WIFI_connected) {
			WIFI_State = WIFI_READY;
		} else {
			WIFI_State = WIFI_CONFIGURED;
		}
		break;

	case WIFI_PROCESS_COMMAND:
		WIFI_TryGetLine();
		if ((HAL_GetTick() - commandStartTime) > WIFI_COMMAND_TIMEOUT) {
			println("[WIFI] command timeout");

			if (WIFI_ProcessingCommand == WIFI_COMM_TCP_GET_CONNECT || WIFI_ProcessingCommand == WIFI_COMM_TCP_GET_RESPONSE) {
				if (get_HTTP_tryes >= 3) {
					WIFI_SendCommand("AT+CIPCLOSE=0\r\n");
					WIFI_WaitForOk();
				}
				WIFI_State = WIFI_READY;
				WIFI_getHTTPpage("", "", NULL, false, true);
				return;
			}
			if (WIFI_ProcessingCommand == WIFI_COMM_TCP_POST_CONNECT || WIFI_ProcessingCommand == WIFI_COMM_TCP_POST_RESPONSE) {
				if (post_HTTP_tryes >= 3) {
					WIFI_SendCommand("AT+CIPCLOSE=0\r\n");
					WIFI_WaitForOk();
				}
				WIFI_State = WIFI_READY;
				WIFI_postHTTPpage("", "", NULL, false, true);
				return;
			} else {
				WIFI_State = WIFI_TIMEOUT;
				WIFI_ProcessingCommand = WIFI_COMM_NONE;
			}
		} else if (strstr(WIFI_readedLine, "OK") != NULL) {
			// SW reset
			if (WIFI_ProcessingCommand == WIFI_COMM_SW_RESTART) {
				HAL_Delay(1000);
				WIFI_Init();
				return;
			}
			// ListAP Command Ended
			if (WIFI_ProcessingCommand == WIFI_COMM_LISTAP) {
				for (uint8_t i = 0; i < WIFI_FOUNDED_AP_MAXCOUNT; i++) {
					strcpy((char *)&WIFI_FoundedAP[i], (char *)&WIFI_FoundedAP_InWork[i]);
					WIFI_stop_auto_ap_list = false;
				}
			}
			// Create Server Command Ended
			if (WIFI_ProcessingCommand == WIFI_COMM_CREATESERVER) {
				WIFI_SendCommand("AT+CIPSTO=3600\r\n"); // Connection timeout
				WIFI_WaitForOk();
				WIFI_CAT_server_started = true;
				println("[WIFI] CAT Server started on port 6784");
				WIFI_State = WIFI_READY;
			}
			// SNTP Command Ended
			if (WIFI_ProcessingCommand == WIFI_COMM_GETSNTP) {
				WIFI_State = WIFI_READY;
			}
			// Get IP Command Ended
			if (WIFI_ProcessingCommand == WIFI_COMM_GETIP) {
				WIFI_State = WIFI_READY;
			}
			// TCP connect
			if (WIFI_ProcessingCommand == WIFI_COMM_TCP_GET_CONNECT) {
				WIFI_sendHTTPGetRequest();
				return;
			}
			if (WIFI_ProcessingCommand == WIFI_COMM_TCP_POST_CONNECT) {
				WIFI_sendHTTPPostRequest();
				return;
			}
			if (WIFI_ProcessingCommand == WIFI_COMM_TCP_GET_RESPONSE) {
				WIFI_getHTTPResponse();
				return;
			}
			if (WIFI_ProcessingCommand == WIFI_COMM_TCP_POST_RESPONSE) {
				WIFI_getHTTPResponse();
				return;
			}
			// Some stuff
			if (WIFI_ProcessingCommandCallback != NULL) {
				WIFI_ProcessingCommandCallback();
			}
			WIFI_ProcessingCommand = WIFI_COMM_NONE;
		} else if (strlen(WIFI_readedLine) > 5) // read command output
		{
			if (WIFI_ProcessingCommand == WIFI_COMM_LISTAP) // ListAP Command process
			{
				char *start = strchr(WIFI_readedLine, '"');
				if (start != NULL) {
					start = start + 1;
					char *end = strchr(start, '"');
					if (end != NULL) {
						*end = 0x00;
						if (strlen(start) > 0) {
							strcat((char *)&WIFI_FoundedAP_InWork[WIFI_FoundedAP_Index], start);
							if (WIFI_FoundedAP_Index < (WIFI_FOUNDED_AP_MAXCOUNT - 1)) {
								WIFI_FoundedAP_Index++;
							}
						}
					}
					if (sysmenu_wifi_selectap1_menu_opened || sysmenu_wifi_selectap2_menu_opened || sysmenu_wifi_selectap3_menu_opened) {
						LCD_UpdateQuery.SystemMenuRedraw = true;
					}
				}
			} else if (WIFI_ProcessingCommand == WIFI_COMM_GETSNTP) // Get and sync SNTP time
			{                                                       // Mon Jan 18 20:17:56 2021
				char *sntp_str = strchr(WIFI_readedLine, ' ');
				if (sntp_str != NULL) {
					sntp_str = sntp_str + 1;
					char *month_str = sntp_str;
					sntp_str = strchr(sntp_str, ' ');
					if (sntp_str != NULL) {
						*sntp_str = 0x00;
						sntp_str = sntp_str + 1;
						char *day_str = sntp_str;
						sntp_str = strchr(sntp_str, ' ');
						if (sntp_str != NULL) {
							*sntp_str = 0x00;
							sntp_str = sntp_str + 1;
							// hh:mm:ss here
							char *min_str = strchr(sntp_str, ':');
							if (min_str != NULL) {
								min_str = min_str + 1;
								char *sec_str = strchr(min_str, ':');
								char *year_str = strchr(min_str, ' ');
								char *end = strchr(sntp_str, ':');
								if (sec_str != NULL && year_str != NULL && end != NULL) {
									sec_str = sec_str + 1;
									year_str = year_str + 1;
									*end = 0x00;
									end = strchr(min_str, ':');
									if (end != NULL) {
										*end = 0x00;
										end = strchr(sec_str, ' ');
										if (end != NULL) {
											*end = 0x00;
											// split strings here
											uint8_t hrs = (uint8_t)atoi(sntp_str);
											uint8_t min = (uint8_t)atoi(min_str);
											uint8_t sec = (uint8_t)atoi(sec_str);
											uint16_t year = (uint16_t)atoi(year_str);
											uint16_t year_short = (uint16_t)atoi(year_str + 2);
											uint16_t month = 1;
											if (strstr(month_str, "Jan") != NULL) {
												month = 1;
											}
											if (strstr(month_str, "Feb") != NULL) {
												month = 2;
											}
											if (strstr(month_str, "Mar") != NULL) {
												month = 3;
											}
											if (strstr(month_str, "Apr") != NULL) {
												month = 4;
											}
											if (strstr(month_str, "May") != NULL) {
												month = 5;
											}
											if (strstr(month_str, "Jun") != NULL) {
												month = 6;
											}
											if (strstr(month_str, "Jul") != NULL) {
												month = 7;
											}
											if (strstr(month_str, "Aug") != NULL) {
												month = 8;
											}
											if (strstr(month_str, "Sep") != NULL) {
												month = 9;
											}
											if (strstr(month_str, "Oct") != NULL) {
												month = 10;
											}
											if (strstr(month_str, "Nov") != NULL) {
												month = 11;
											}
											if (strstr(month_str, "Dec") != NULL) {
												month = 12;
											}
											uint16_t day = (uint16_t)atoi(day_str);
											// save to RTC clock
											if (year > 2018) {
												uint32_t currTime = RTC->TR;
												uint8_t currHours = ((currTime >> 20) & 0x03) * 10 + ((currTime >> 16) & 0x0f);
												uint8_t currMinutes = ((currTime >> 12) & 0x07) * 10 + ((currTime >> 8) & 0x0f);
												uint8_t currSeconds = ((currTime >> 4) & 0x07) * 10 + ((currTime >> 0) & 0x0f);
												// clock diff
												if (currHours != hrs || currMinutes != min || currSeconds != sec) {
													int16_t secDiff = (currHours - hrs) * 3600 + (currMinutes - min) * 60 + (currSeconds - sec);
													println("[RTC] Current clock error in sec: ", secDiff);

													if (secDiff < -1 || secDiff > 1) // do recalibration
													{
														if (secDiff < 0) {
															CALIBRATE.RTC_Calibration--;
														}
														if (secDiff > 0) {
															CALIBRATE.RTC_Calibration++;
														}
														if (CALIBRATE.RTC_Calibration < -511) {
															CALIBRATE.RTC_Calibration = -511;
														}
														if (CALIBRATE.RTC_Calibration > 511) {
															CALIBRATE.RTC_Calibration = 511;
														}
														NeedSaveCalibration = true;

														RTC_Calibration();
														println("[RTC] New RTC Calibration value: ", CALIBRATE.RTC_Calibration);
													}

													// set time
													RTC_TimeTypeDef sTime;
													sTime.TimeFormat = RTC_HOURFORMAT12_PM;
													sTime.SubSeconds = 0;
													sTime.SecondFraction = 0;
													sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
													sTime.StoreOperation = RTC_STOREOPERATION_SET;
													sTime.Hours = hrs;
													sTime.Minutes = min;
													sTime.Seconds = sec;
													RTC_DateTypeDef sDate;
													uint16_t d = day;
													uint16_t weekday = (d += month < 3 ? year-- : year - 2, 23 * month / 9 + d + 4 + year / 4 - year / 100 + year / 400) % 7;
													sDate.Date = day;
													sDate.Month = month;
													sDate.Year = year_short;
													sDate.WeekDay = weekday;
													BKPSRAM_Enable();
													HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
													HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
													println("[RTC] New time set ", hrs, ":", min, ":", sec, " ", day, "-", month, "-", year_short, " ", weekday);
												}
												TRX_SNTP_Synced = HAL_GetTick();
												println("[WIFI] TIME SYNCED");

												// reset SNTP
												char com_t[64] = {0};
												sprintf(com_t, "AT+CIPSNTPCFG=1,0,\"0.pool.ntp.org\",\"1.pool.ntp.org\"\r\n");
												WIFI_SendCommand(com_t); // configure SNMP
												WIFI_WaitForOk();
												WIFI_State = WIFI_READY;
											}
										}
									}
								}
							}
						}
					}
				}
				// TRX_SNTP_Synced = HAL_GetTick();
			} else if (WIFI_ProcessingCommand == WIFI_COMM_GETIP) // GetIP Command process
			{
				char *sep = "_CUR:ip";
				char *istr;
				istr = strstr(WIFI_readedLine, sep);
				if (istr != NULL) {
					char *start = strchr(WIFI_readedLine, '"');
					if (start != NULL) {
						start = start + 1;
						char *end = strchr(start, '"');
						if (end != NULL) {
							*end = 0x00;
							strcpy(WIFI_IP, start);
							println("[WIFI] GOT IP: ", WIFI_IP);
							WIFI_IP_Gotted = true;
							if (LCD_systemMenuOpened) {
								LCD_UpdateQuery.SystemMenuInfolines = true;
							}
						}
					}
				}
			} else if (WIFI_ProcessingCommand == WIFI_COMM_TCP_GET_RESPONSE) // WIFI_COMM_TCP_GET_RESPONSE Command process
			{
				char *istr;
				istr = strstr(WIFI_readedLine, "+IPD");
				if (istr != NULL) {
					WIFI_getHTTPResponse();
				}
			} else if (WIFI_ProcessingCommand == WIFI_COMM_TCP_POST_RESPONSE) // WIFI_COMM_TCP_POST_RESPONSE Command process
			{
				char *istr;
				istr = strstr(WIFI_readedLine, "+IPD");
				if (istr != NULL) {
					WIFI_getHTTPResponse();
				}
			}
		}
		break;

	case WIFI_UNDEFINED:
	case WIFI_NOTFOUND:
	case WIFI_REINIT:
	case WIFI_FAIL:
	case WIFI_SLEEP:
		break;
	}
}

bool WIFI_AbortCallback() {
	WIFI_ProcessingCommandCallback = NULL;
	return true;
}

bool WIFI_GetSNTPTime(void (*callback)(void)) {
	if (WIFI_State != WIFI_READY) {
		return false;
	}
	WIFI_State = WIFI_PROCESS_COMMAND;
	WIFI_ProcessingCommand = WIFI_COMM_GETSNTP;
	WIFI_ProcessingCommandCallback = callback;
	WIFI_SendCommand("AT+CIPSNTPTIME?\r\n"); // get SNMP time
	return true;
}

bool WIFI_GetIP(void (*callback)(void)) {
	if (WIFI_State != WIFI_READY) {
		return false;
	}
	WIFI_State = WIFI_PROCESS_COMMAND;
	WIFI_ProcessingCommand = WIFI_COMM_GETIP;
	WIFI_ProcessingCommandCallback = callback;
	WIFI_SendCommand("AT+CIPSTA_CUR?\r\n"); // get ip
	return true;
}

bool WIFI_ListAP(void (*callback)(void)) {
	if (WIFI_State != WIFI_READY && WIFI_State != WIFI_CONFIGURED) {
		return false;
	}
	if (WIFI_State == WIFI_CONFIGURED && !WIFI_stop_auto_ap_list && WIFI_ProcessingCommand == WIFI_COMM_LISTAP) // stop auto-connection when searching for networks
	{
		WIFI_stop_auto_ap_list = true;
		WIFI_WaitForOk();
	}
	WIFI_State = WIFI_PROCESS_COMMAND;
	WIFI_ProcessingCommand = WIFI_COMM_LISTAP;
	WIFI_ProcessingCommandCallback = callback;
	WIFI_FoundedAP_Index = 0;

	for (uint8_t i = 0; i < WIFI_FOUNDED_AP_MAXCOUNT; i++) {
		dma_memset((char *)&WIFI_FoundedAP_InWork[i], 0x00, sizeof WIFI_FoundedAP_InWork[i]);
	}
	WIFI_SendCommand("AT+CWLAP\r\n"); // List AP
	return true;
}

static bool WIFI_ListAP_Sync(void) {
	WIFI_SendCommand("AT+CWLAP\r\n"); // List AP
	WIFI_FoundedAP_Index = 0;
	for (uint8_t i = 0; i < WIFI_FOUNDED_AP_MAXCOUNT; i++) {
		dma_memset((char *)&WIFI_FoundedAP_InWork[i], 0x00, sizeof WIFI_FoundedAP_InWork[i]);
	}
	uint32_t startTime = HAL_GetTick();
	char *sep = "OK";
	char *istr;

	while ((HAL_GetTick() - startTime) < WIFI_COMMAND_TIMEOUT) {
		if (!WIFI_TryGetLine()) {
			CPULOAD_GoToSleepMode();
			CPULOAD_WakeUp();
			continue;
		}

		istr = strstr(WIFI_readedLine, sep);
		if (istr != NULL) // OK
		{
			for (uint8_t i = 0; i < WIFI_FOUNDED_AP_MAXCOUNT; i++) {
				strcpy((char *)&WIFI_FoundedAP[i], (char *)&WIFI_FoundedAP_InWork[i]);
			}

			if (WIFI_FoundedAP_Index > 0) {
				LCD_UpdateQuery.SystemMenuInfolines = true;
			}

			return true;
		}

		if (strlen(WIFI_readedLine) > 5) //-V814
		{
			char *start = strchr(WIFI_readedLine, '"');
			if (start != NULL) {
				start = start + 1;
				char *end = strchr(start, '"');
				if (end != NULL) {
					*end = 0x00;
					strcat((char *)&WIFI_FoundedAP_InWork[WIFI_FoundedAP_Index], start);
					if (WIFI_FoundedAP_Index < (WIFI_FOUNDED_AP_MAXCOUNT - 1)) {
						WIFI_FoundedAP_Index++;
					}
				}
				if (sysmenu_wifi_selectap1_menu_opened || sysmenu_wifi_selectap2_menu_opened || sysmenu_wifi_selectap3_menu_opened) {
					LCD_UpdateQuery.SystemMenuRedraw = true;
				}
			}
		}
	}

	return false;
}

void WIFI_GoSleep(void) {
	if (WIFI_State == WIFI_SLEEP) {
		return;
	}
	WIFI_State = WIFI_PROCESS_COMMAND;
	WIFI_ProcessingCommand = WIFI_COMM_DEEPSLEEP;
	WIFI_SendCommand("AT+GSLP=1000\r\n"); // go sleep
	WIFI_State = WIFI_SLEEP;
	WIFI_connected = false;
	LCD_UpdateQuery.StatusInfoGUI = true;
}

static void WIFI_SendCommand(char *command) {
	HAL_UART_AbortReceive(&HRDW_WIFI_UART);
	HAL_UART_AbortReceive_IT(&HRDW_WIFI_UART);
	dma_memset(WIFI_AnswerBuffer, 0x00, sizeof(WIFI_AnswerBuffer));
	WIFI_Answer_ReadIndex = 0;
	HAL_UART_Receive_DMA(&HRDW_WIFI_UART, (uint8_t *)WIFI_AnswerBuffer, WIFI_ANSWER_BUFFER_SIZE);
	HAL_UART_Transmit_IT(&HRDW_WIFI_UART, (uint8_t *)command, (uint16_t)strlen(command));
	commandStartTime = HAL_GetTick();
	HAL_Delay(WIFI_COMMAND_DELAY);
	if (TRX.Debug_Type == TRX_DEBUG_WIFI) { // DEBUG
		print("WIFI_S: ", command);
	}
}

static bool WIFI_WaitForOk(void) {
	char *sep = "OK";
	char *sep2 = "ERROR";
	char *istr;
	uint32_t startTime = HAL_GetTick();
	while ((HAL_GetTick() - startTime) < WIFI_COMMAND_TIMEOUT) {
		if (WIFI_TryGetLine()) {
			// OK
			istr = strstr(WIFI_readedLine, sep);
			if (istr != NULL) {
				return true;
			}
			// ERROR
			istr = strstr(WIFI_readedLine, sep2);
			if (istr != NULL) {
				return false;
			}
		}
		CPULOAD_GoToSleepMode();
		CPULOAD_WakeUp();
	}
	return false;
}

static bool WIFI_TryGetLine(void) {
	dma_memset(WIFI_readedLine, 0x00, sizeof(WIFI_readedLine));
	dma_memset(tmp, 0x00, sizeof(tmp));

	Aligned_CleanInvalidateDCache_by_Addr((uint32_t)WIFI_AnswerBuffer, sizeof(WIFI_AnswerBuffer));
	uint16_t dma_index = WIFI_ANSWER_BUFFER_SIZE - (uint16_t)__HAL_DMA_GET_COUNTER(HRDW_WIFI_UART.hdmarx);
	if (WIFI_Answer_ReadIndex == dma_index) {
		return false;
	}

	if (dma_index < WIFI_Answer_ReadIndex) {
		// tail
		uint32_t len = WIFI_ANSWER_BUFFER_SIZE - WIFI_Answer_ReadIndex;
		strncpy(tmp, &WIFI_AnswerBuffer[WIFI_Answer_ReadIndex], len);
		// head
		strncat(tmp, &WIFI_AnswerBuffer[0], dma_index);
	} else {
		// head
		strncpy(tmp, &WIFI_AnswerBuffer[WIFI_Answer_ReadIndex], dma_index - WIFI_Answer_ReadIndex);
	}

	if (tmp[0] == '\0') {
		return false;
	}

	char *istr = strchr(tmp, '\n'); // look for the end of the line
	if (istr == NULL) {
		return false;
	}

	uint32_t len = (uint16_t)((uint32_t)istr - (uint32_t)tmp + 1);
	if (len > WIFI_LINE_BUFFER_SIZE) {
		return false;
	}
	strncpy(WIFI_readedLine, tmp, len);

	WIFI_Answer_ReadIndex += len;
	if (WIFI_Answer_ReadIndex >= WIFI_ANSWER_BUFFER_SIZE) {
		WIFI_Answer_ReadIndex -= WIFI_ANSWER_BUFFER_SIZE;
	}

	if (TRX.Debug_Type == TRX_DEBUG_WIFI) { // DEBUG
		print("WIFI_R: ", WIFI_readedLine);
	}

	return true;
}

bool WIFI_StartCATServer(void (*callback)(void)) {
	if (WIFI_State != WIFI_READY) {
		return false;
	}
	WIFI_State = WIFI_PROCESS_COMMAND;
	WIFI_ProcessingCommand = WIFI_COMM_CREATESERVER;
	WIFI_ProcessingCommandCallback = callback;
	WIFI_SendCommand("AT+CIPSERVER=1,6784\r\n"); // Start CAT Server
	return true;
}

bool WIFI_SendCatAnswer(char *data, uint32_t link_id, void (*callback)(void)) {
	if (WIFI_State != WIFI_READY) {
		return false;
	}
	WIFI_State = WIFI_PROCESS_COMMAND;
	WIFI_ProcessingCommand = WIFI_COMM_SENDTCPDATA;
	WIFI_ProcessingCommandCallback = callback;
	char answer[64] = {0};
	sprintf(answer, "AT+CIPSEND=%u,%u\r\n", link_id, strlen(data));
	WIFI_SendCommand(answer); // Send CAT answer
	char answer_data[64] = {0};
	strcat(answer_data, data);
	strcat(answer_data, "\r");
	WIFI_SendCommand(answer_data); // Send CAT answer data
	WIFI_ProcessingCommand = WIFI_COMM_NONE;
	WIFI_State = WIFI_READY;
	return true;
}

bool WIFI_SendIQData(uint8_t *data, uint32_t size) {
	if (WIFI_State != WIFI_READY) {
		return false;
	}
#define link_id 0
	WIFI_State = WIFI_PROCESS_COMMAND;
	WIFI_ProcessingCommand = WIFI_COMM_SENDTCPDATA;
	WIFI_ProcessingCommandCallback = NULL;
	char header[64] = {0};
	sprintf(header, "AT+CIPSEND=%u,%u\r\n", link_id, size);
	HAL_UART_Transmit_IT(&HRDW_WIFI_UART, (uint8_t *)header, (uint16_t)strlen(header)); // Start IQ sending
	HAL_Delay(2);
	HAL_UART_Transmit_IT(&HRDW_WIFI_UART, data, size); // Send IQ data
	WIFI_ProcessingCommand = WIFI_COMM_NONE;
	WIFI_State = WIFI_READY;
	return true;
}

bool WIFI_UpdateFW(void (*callback)(void)) {
	if (WIFI_State != WIFI_READY) {
		return false;
	}
	WIFI_State = WIFI_PROCESS_COMMAND;
	WIFI_ProcessingCommand = WIFI_COMM_UPDATEFW;
	WIFI_ProcessingCommandCallback = callback;
	WIFI_SendCommand("AT+CIUPDATE\r\n"); // Start Update Firmware
	// WIFI_WaitForOk();
	return true;
}

static void WIFI_getHTTPResponse(void) {
	uint32_t readed_body_length = 0;
	char *istr;
	istr = strstr(WIFI_readedLine, "+IPD");
	if (istr != NULL) {
		istr += 7;
		char *istr2 = strchr(WIFI_readedLine, ':');
		if (istr2 != NULL) {
			*istr2 = 0;
			uint32_t response_length = atoi(istr);
			// println("RESP LENGTH: ", response_length);
			istr2++;
			strcpy(WIFI_HTTResponseHTML, istr2);
			commandStartTime = HAL_GetTick();

			uint32_t start_time = HAL_GetTick();
			uint32_t len = strlen(WIFI_HTTResponseHTML);
			while (len < response_length && len < sizeof(WIFI_HTTResponseHTML) && (HAL_GetTick() - start_time) < 5000) {
				if (WIFI_TryGetLine()) {
					if (len + strlen(WIFI_readedLine) < sizeof(WIFI_HTTResponseHTML)) {
						strcat(WIFI_HTTResponseHTML, WIFI_readedLine);
					} else {
						break;
					}
				}
				len = strlen(WIFI_HTTResponseHTML);
			}
			// println("RESP LENGTH STRLEN: ", strlen(WIFI_HTTResponseHTML));
			char *istr3 = WIFI_HTTResponseHTML;
			istr3 += response_length;
			*istr3 = 0;

			// get status
			char *istr4 = strchr(WIFI_HTTResponseHTML, ' ');
			if (istr4 != NULL) {
				char *istr5 = istr4 + 4;
				*istr5 = 0;
				WIFI_HTTP_Response_Status = (uint16_t)(atoi(istr4));
				*istr5 = ' ';
			}

			// get content length
			istr4 = strstr(WIFI_HTTResponseHTML, "Content-Length: ");
			if (istr4 != NULL) {
				istr4 += 16;
				char *istr5 = strchr(istr4, '\r');
				if (istr5 != NULL) {
					*istr5 = 0;
					WIFI_HTTP_Response_ContentLength = (uint32_t)(atoi(istr4));
					*istr5 = ' ';
				}
			}

			// get response body
			char *istr6 = strstr(WIFI_HTTResponseHTML, "\r\n\r\n");
			if (istr6 != NULL) {
				istr6 += 4;
				strcpy(WIFI_HTTResponseHTML, istr6);
			}

			// partial callback for image printing
			readed_body_length += strlen(WIFI_HTTResponseHTML);
			if (WIFI_ProcessingCommandCallback == WIFI_printImage_stream_callback) {
				WIFI_printImage_stream_partial_callback();
			}

			// may be partial content? continue downloading
			start_time = HAL_GetTick();
#define WIFI_STREAM_Timeout 10000
			if (readed_body_length < WIFI_HTTP_Response_ContentLength && (HAL_GetTick() - start_time) < WIFI_STREAM_Timeout) {
				while (readed_body_length < WIFI_HTTP_Response_ContentLength && strlen(WIFI_HTTResponseHTML) < sizeof(WIFI_HTTResponseHTML) && (HAL_GetTick() - start_time) < WIFI_STREAM_Timeout) {
					if (WIFI_TryGetLine()) {
						istr = strstr(WIFI_readedLine, "+IPD");
						if (istr != NULL) {
							istr += 7;
							istr2 = strchr(WIFI_readedLine, ':');
							if (istr2 != NULL) {
								*istr2 = 0;
								response_length = atoi(istr);
								istr2++;
								if ((strlen(WIFI_HTTResponseHTML) + response_length) < sizeof(WIFI_HTTResponseHTML)) {
									strncat(WIFI_HTTResponseHTML, istr2, response_length);

									// partial callback for image printing
									readed_body_length += response_length;

									// update timeout start
									start_time = HAL_GetTick();

									if (WIFI_ProcessingCommandCallback == WIFI_printImage_stream_callback) {
										WIFI_printImage_stream_partial_callback();
									}
								}
							}
						}
					}
				}
			}

			// cut body on content-length
			if (strlen(WIFI_HTTResponseHTML) > WIFI_HTTP_Response_ContentLength) {
				WIFI_HTTResponseHTML[WIFI_HTTP_Response_ContentLength] = 0;
			}

			WIFI_ProcessingCommand = WIFI_COMM_NONE;
			WIFI_State = WIFI_READY;
			if (WIFI_ProcessingCommandCallback != NULL) {
				WIFI_ProcessingCommandCallback();
			}
			WIFI_SendCommand("AT+CIPCLOSE=0\r\n");
		}
	}
}

static void WIFI_sendHTTPGetRequest(void) {
	WIFI_State = WIFI_PROCESS_COMMAND;
	WIFI_ProcessingCommand = WIFI_COMM_TCP_GET_RESPONSE;
	dma_memset(WIFI_HTTRequest, 0x00, sizeof(WIFI_HTTRequest));
	strcat(WIFI_HTTRequest, "GET ");
	strcat(WIFI_HTTRequest, WIFI_GETuri);
	strcat(WIFI_HTTRequest, " HTTP/1.1\r\n");
	strcat(WIFI_HTTRequest, "Host: ");
	strcat(WIFI_HTTRequest, WIFI_HOSTuri);
	strcat(WIFI_HTTRequest, "\r\nConnection: close\r\n\r\n");
	char comm_line[64] = {0};
	sprintf(comm_line, "AT+CIPSEND=0,%d\r\n", strlen(WIFI_HTTRequest));
	WIFI_SendCommand(comm_line);
	WIFI_SendCommand(WIFI_HTTRequest);
}

static void WIFI_sendHTTPPostRequest(void) {
	char comm_line[64] = {0};
	WIFI_State = WIFI_PROCESS_COMMAND;
	WIFI_ProcessingCommand = WIFI_COMM_TCP_POST_RESPONSE;
	dma_memset(WIFI_HTTRequest, 0x00, sizeof(WIFI_HTTRequest));
	strcat(WIFI_HTTRequest, "POST ");
	strcat(WIFI_HTTRequest, WIFI_GETuri);
	strcat(WIFI_HTTRequest, " HTTP/1.1\r\n");
	strcat(WIFI_HTTRequest, "Host: ");
	strcat(WIFI_HTTRequest, WIFI_HOSTuri);
	strcat(WIFI_HTTRequest, "\r\nContent-Type: application/x-www-form-urlencoded\r\n");
	sprintf(comm_line, "Content-Length: %d\r\n", strlen(WIFI_POSTdata));
	strcat(WIFI_HTTRequest, comm_line);
	strcat(WIFI_HTTRequest, "Connection: close\r\n\r\n");
	strcat(WIFI_HTTRequest, WIFI_POSTdata);
	sprintf(comm_line, "AT+CIPSEND=0,%d\r\n", strlen(WIFI_HTTRequest));
	WIFI_SendCommand(comm_line);
	WIFI_SendCommand(WIFI_HTTRequest);
}

bool WIFI_getHTTPpage(char *host, char *url, void (*callback)(void), bool https, bool is_repeat) {
	if (WIFI_State != WIFI_READY && !is_repeat) {
		return false;
	}
	static char _host[32] = {0};
	static char _url[128] = {0};
	static bool _https;
	if (!is_repeat) {
		get_HTTP_tryes = 0;
		strcpy(_host, host);
		strcpy(_url, url);
		_https = https;
		WIFI_ProcessingCommandCallback = callback;
	} else {
		get_HTTP_tryes++;
	}
	WIFI_State = WIFI_PROCESS_COMMAND;
	WIFI_ProcessingCommand = WIFI_COMM_TCP_GET_CONNECT;
	WIFI_HTTP_Response_Status = 0;

	dma_memset(WIFI_HOSTuri, 0x00, sizeof(WIFI_HOSTuri));
	strcat(WIFI_HOSTuri, "AT+CIPSTART=0,");
	if (!_https) {
		strcat(WIFI_HOSTuri, "\"TCP\"");
	} else {
		strcat(WIFI_HOSTuri, "\"SSL\"");
	}
	strcat(WIFI_HOSTuri, ",\"");
	strcat(WIFI_HOSTuri, _host);

	if (!_https) {
		strcat(WIFI_HOSTuri, "\",80,10\r\n");
	} else {
		strcat(WIFI_HOSTuri, "\",443,10\r\n");
	}

	dma_memset(WIFI_GETuri, 0x00, sizeof(WIFI_GETuri));
	strcat(WIFI_GETuri, _url);

	WIFI_SendCommand(WIFI_HOSTuri);

	dma_memset(WIFI_HOSTuri, 0x00, sizeof(WIFI_HOSTuri));
	strcat(WIFI_HOSTuri, _host);
	return true;
}

bool WIFI_postHTTPpage(char *host, char *url, void (*callback)(void), bool https, bool is_repeat) {
	if (WIFI_State != WIFI_READY && !is_repeat) {
		return false;
	}
	static char _host[32] = {0};
	static char _url[128] = {0};
	static bool _https;
	if (!is_repeat) {
		post_HTTP_tryes = 0;
		strcpy(_host, host);
		strcpy(_url, url);
		_https = https;
		WIFI_ProcessingCommandCallback = callback;
	} else {
		post_HTTP_tryes++;
	}
	WIFI_State = WIFI_PROCESS_COMMAND;
	WIFI_ProcessingCommand = WIFI_COMM_TCP_POST_CONNECT;
	WIFI_HTTP_Response_Status = 0;

	dma_memset(WIFI_HOSTuri, 0x00, sizeof(WIFI_HOSTuri));
	strcat(WIFI_HOSTuri, "AT+CIPSTART=0,");
	if (!_https) {
		strcat(WIFI_HOSTuri, "\"TCP\"");
	} else {
		strcat(WIFI_HOSTuri, "\"SSL\"");
	}
	strcat(WIFI_HOSTuri, ",\"");
	strcat(WIFI_HOSTuri, _host);

	if (!_https) {
		strcat(WIFI_HOSTuri, "\",80,10\r\n");
	} else {
		strcat(WIFI_HOSTuri, "\",443,10\r\n");
	}

	dma_memset(WIFI_GETuri, 0x00, sizeof(WIFI_GETuri));
	strcat(WIFI_GETuri, _url);

	WIFI_SendCommand(WIFI_HOSTuri);

	dma_memset(WIFI_HOSTuri, 0x00, sizeof(WIFI_HOSTuri));
	strcat(WIFI_HOSTuri, _host);
	return true;
}

static void WIFI_printText_callback(void) {
	LCDDriver_Fill(BG_COLOR);
#ifdef LCD_SMALL_INTERFACE
	if (WIFI_HTTP_Response_Status == 200) {
		LCDDriver_printText(WIFI_HTTResponseHTML, 0, 20, FG_COLOR, BG_COLOR, 1);
	} else {
		LCDDriver_printText("Network error", 10, 20, FG_COLOR, BG_COLOR, 1);
	}
#else
	if (WIFI_HTTP_Response_Status == 200) {
		LCDDriver_printTextFont(WIFI_HTTResponseHTML, 0, 20, FG_COLOR, BG_COLOR, &FreeSans9pt7b);
	} else {
		LCDDriver_printTextFont("Network error", 10, 20, FG_COLOR, BG_COLOR, &FreeSans9pt7b);
	}
#endif
}

static void WIFI_printImage_stream_partial_callback(void) {
	dma_memset(WIFI_RLEStreamBuffer, 0x00, sizeof(WIFI_RLEStreamBuffer));
	// parse hex output from server (convert to bin)
	char *istr = WIFI_HTTResponseHTML;
	char hex[5] = {0};
	WIFI_RLEStreamBuffer_index_full = 0;
	WIFI_RLEStreamBuffer_index_partial = 0;
	int16_t val = 0;
	uint32_t len = strlen(WIFI_HTTResponseHTML);

	while (*istr != 0 && (len >= ((WIFI_RLEStreamBuffer_index_full * 4) + 4))) {
		// Get hex
		strncpy(hex, istr, 4);
		val = (int16_t)(strtol(hex, NULL, 16));
		istr += 4;

		// Save
		WIFI_RLEStreamBuffer[WIFI_RLEStreamBuffer_index_partial] = val;
		WIFI_RLEStreamBuffer_index_full++;
		WIFI_RLEStreamBuffer_index_partial++;

		// buffer full, send to LCD RLE stream decoder
		if (WIFI_RLEStreamBuffer_index_partial >= (WIFI_RLE_BUFFER_SIZE - 2)) {
			LCDDriver_printImage_RLECompressed_ContinueStream(WIFI_RLEStreamBuffer, WIFI_RLEStreamBuffer_index_partial);
			WIFI_RLEStreamBuffer_index_partial = 0;
		}
	}

	// send to LCD RLE stream decoder
	if (WIFI_RLEStreamBuffer_index_partial > 0) {
		LCDDriver_printImage_RLECompressed_ContinueStream(WIFI_RLEStreamBuffer, WIFI_RLEStreamBuffer_index_partial);
	}

	// clean answer
	if (len > (WIFI_RLEStreamBuffer_index_full * 4)) // part buffer preceed, move to begin
	{
		istr = &WIFI_HTTResponseHTML[WIFI_RLEStreamBuffer_index_full * 4];
		strcpy(WIFI_HTTResponseHTML, istr);
	} else {
		dma_memset(WIFI_HTTResponseHTML, 0x00, sizeof(WIFI_HTTResponseHTML));
	}
}

static void WIFI_printImage_stream_callback(void) {
	// image print stream done
}

static void WIFI_printImage_Propagation_callback(void) {
	LCDDriver_Fill(BG_COLOR);
	if (WIFI_HTTP_Response_Status == 200) {
		char *istr1 = strchr(WIFI_HTTResponseHTML, ',');
		if (istr1 != NULL) {
			*istr1 = 0;
			uint32_t filesize = atoi(WIFI_HTTResponseHTML);
			istr1++;
			char *istr2 = strchr(istr1, ',');
			if (istr2 != NULL) {
				*istr2 = 0;
				uint16_t width = (uint16_t)(atoi(istr1));
				istr2++;

				uint16_t height = (uint16_t)(atoi(istr2));

				if (filesize > 0 && width > 0 && height > 0) {
					LCDDriver_printImage_RLECompressed_StartStream(LCD_WIDTH / 2 - width / 2, LCD_HEIGHT / 2 - height / 2, width, height);
					char buff[64] = {0};
					sprintf(buff, "/trx_services/propagination.php?part=0&width=%u&height=%u", LCD_WIDTH, LCD_HEIGHT);
					WIFI_getHTTPpage(WIFI_HOST, buff, WIFI_printImage_stream_callback, false, false);
				}
			}
		}
	} else
#ifdef LCD_SMALL_INTERFACE
		LCDDriver_printText("Network error", 10, 20, FG_COLOR, BG_COLOR, 1);
#else
		LCDDriver_printTextFont("Network error", 10, 20, FG_COLOR, BG_COLOR, &FreeSans9pt7b);
#endif
}

static void WIFI_printImage_Tropo_callback(void) {
	LCDDriver_Fill(BG_COLOR);
	if (WIFI_HTTP_Response_Status == 200) {
		char *istr1 = strchr(WIFI_HTTResponseHTML, ',');
		if (istr1 != NULL) {
			*istr1 = 0;
			uint32_t filesize = atoi(WIFI_HTTResponseHTML);
			istr1++;
			char *istr2 = strchr(istr1, ',');
			if (istr2 != NULL) {
				*istr2 = 0;
				uint16_t width = (uint16_t)(atoi(istr1));
				istr2++;

				uint16_t height = (uint16_t)(atoi(istr2));

				if (filesize > 0 && width > 0 && height > 0) {
					LCDDriver_printImage_RLECompressed_StartStream(LCD_WIDTH / 2 - width / 2, LCD_HEIGHT / 2 - height / 2, width, height);
					char buff[64] = {0};
					sprintf(buff, "/trx_services/tropo.php?region=%u&part=0&width=%u&height=%u", TRX.TROPO_Region, LCD_WIDTH, LCD_HEIGHT);
					WIFI_getHTTPpage(WIFI_HOST, buff, WIFI_printImage_stream_callback, false, false);
				}
			}
		}
	} else
#ifdef LCD_SMALL_INTERFACE
		LCDDriver_printText("Network error", 10, 20, FG_COLOR, BG_COLOR, 1);
#else
		LCDDriver_printTextFont("Network error", 10, 20, FG_COLOR, BG_COLOR, &FreeSans9pt7b);
#endif
}

static void WIFI_printImage_DayNight_callback(void) {
	LCDDriver_Fill(BG_COLOR);
	if (WIFI_HTTP_Response_Status == 200) {
		char *istr1 = strchr(WIFI_HTTResponseHTML, ',');
		if (istr1 != NULL) {
			*istr1 = 0;
			uint32_t filesize = atoi(WIFI_HTTResponseHTML);
			istr1++;
			char *istr2 = strchr(istr1, ',');
			if (istr2 != NULL) {
				*istr2 = 0;
				uint16_t width = (uint16_t)(atoi(istr1));
				istr2++;

				uint16_t height = (uint16_t)(atoi(istr2));

				if (filesize > 0 && width > 0 && height > 0) {
					LCDDriver_printImage_RLECompressed_StartStream(LCD_WIDTH / 2 - width / 2, LCD_HEIGHT / 2 - height / 2, width, height);
					WIFI_getHTTPpage(WIFI_HOST, "/trx_services/daynight.php?part=0", WIFI_printImage_stream_callback, false, false);
				}
			}
		}
	} else {
		LCDDriver_printTextFont("Network error", 10, 20, FG_COLOR, BG_COLOR, &FreeSans9pt7b);
	}
}

static void WIFI_printImage_Ionogram_callback(void) {
	LCDDriver_Fill(BG_COLOR);
	if (WIFI_HTTP_Response_Status == 200) {
		char *istr1 = strchr(WIFI_HTTResponseHTML, ',');
		if (istr1 != NULL) {
			*istr1 = 0;
			uint32_t filesize = atoi(WIFI_HTTResponseHTML);
			istr1++;
			char *istr2 = strchr(istr1, ',');
			if (istr2 != NULL) {
				*istr2 = 0;
				uint16_t width = (uint16_t)(atoi(istr1));
				istr2++;

				uint16_t height = (uint16_t)(atoi(istr2));

				if (filesize > 0 && width > 0 && height > 0) {
					LCDDriver_printImage_RLECompressed_StartStream(LCD_WIDTH / 2 - width / 2, LCD_HEIGHT / 2 - height / 2, width, height);
					char buff[64] = {0};
					sprintf(buff, "/trx_services/ionogram.php?part=0&ursiCode=%s", TRX.URSI_CODE);
					WIFI_getHTTPpage(WIFI_HOST, buff, WIFI_printImage_stream_callback, false, false);
				}
			}
		}
	} else {
		LCDDriver_printTextFont("Network error", 10, 20, FG_COLOR, BG_COLOR, &FreeSans9pt7b);
	}
}

void WIFI_getRDA(void) {
	LCDDriver_Fill(BG_COLOR);
	if (WIFI_connected && WIFI_State == WIFI_READY) {
#ifdef LCD_SMALL_INTERFACE
		LCDDriver_printText("Loading...", 10, 20, FG_COLOR, BG_COLOR, 1);
#else
		LCDDriver_printTextFont("Loading...", 10, 20, FG_COLOR, BG_COLOR, &FreeSans9pt7b);
#endif
	} else {
#ifdef LCD_SMALL_INTERFACE
		LCDDriver_printText("No connection", 10, 20, FG_COLOR, BG_COLOR, 1);
#else
		LCDDriver_printTextFont("No connection", 10, 20, FG_COLOR, BG_COLOR, &FreeSans9pt7b);
#endif

		return;
	}
	char url[64] = "/trx_services/rda.php?callsign=";
	strcat(url, TRX.CALLSIGN);
	WIFI_getHTTPpage(WIFI_HOST, url, WIFI_printText_callback, false, false);
}

static void WIFI_getDXCluster_background_callback(void) {
	if (WIFI_HTTP_Response_Status == 200) {
		// println("COUNT 0");
		// println("SIZE ", strlen(WIFI_HTTResponseHTML));
		WIFI_DXCLUSTER_list_count = 0;
		// println(WIFI_HTTResponseHTML);

		char *istr_l = WIFI_HTTResponseHTML;
		bool end = false;
		while (!end) {
			char *istr_r = strchr(istr_l, ' ');
			if (istr_r != NULL) {
				*istr_r = 0;
				uint32_t freq = (uint32_t)atoi(istr_l);
				// println("F", freq);

				istr_l = istr_r + 1;
				istr_r = strchr(istr_l, '\t');
				if (istr_r != NULL) {
					*istr_r = 0;
					// println("C", istr_l);

					if (strlen(istr_l) < WIFI_DXCLUSTER_MAX_CALL_LEN) {
						WIFI_DXCLUSTER_list[WIFI_DXCLUSTER_list_count].Freq = freq;
						strcpy(WIFI_DXCLUSTER_list[WIFI_DXCLUSTER_list_count].Callsign, istr_l);
						WIFI_DXCLUSTER_list[WIFI_DXCLUSTER_list_count].Azimuth = 0;
						if (TRX.FFT_DXCluster_Azimuth) {
							CALLSIGN_INFO_LINE *info;
							CALLSIGN_getInfoByCallsign(&info, WIFI_DXCLUSTER_list[WIFI_DXCLUSTER_list_count].Callsign);
							float32_t my_lat = LOCINFO_get_latlon_from_locator(TRX.LOCATOR, true);
							float32_t my_lon = LOCINFO_get_latlon_from_locator(TRX.LOCATOR, false);
							int16_t azimuth = LOCINFO_azimuthFromCoordinates(my_lat, my_lon, info->lat, info->lon);
							WIFI_DXCLUSTER_list[WIFI_DXCLUSTER_list_count].Azimuth = azimuth;
						}

						WIFI_DXCLUSTER_list_count++;

						istr_l = istr_r + 1;

						if (WIFI_DXCLUSTER_list_count >= WIFI_DXCLUSTER_MAX_RECORDS) {
							end = true;
						}
					} else {
						end = true;
					}
				} else {
					end = true;
				}
			} else {
				end = true;
			}
		}

		// for(uint16_t i = 0; i < WIFI_DXCLUSTER_list_count ; i ++)
		// println(WIFI_DXCLUSTER_list[i].Freq, "|", WIFI_DXCLUSTER_list[i].Callsign);

		println("DXCluster updated, count: ", WIFI_DXCLUSTER_list_count);
	}
}

static void WIFI_getWOLFCluster_background_callback(void) {
	if (WIFI_HTTP_Response_Status == 200) {
		WIFI_WOLFCLUSTER_list_count = 0;

		char *istr_l = WIFI_HTTResponseHTML;
		bool end = false;
		while (!end) {
			char *istr_r = strchr(istr_l, ' ');
			if (istr_r != NULL) {
				*istr_r = 0;
				uint64_t freq = atoll(istr_l);

				istr_l = istr_r + 1;
				istr_r = strchr(istr_l, '\t');
				if (istr_r != NULL) {
					*istr_r = 0;

					if (strlen(istr_l) < WIFI_WOLFCLUSTER_MAX_CALL_LEN) {
						WIFI_WOLFCLUSTER_list[WIFI_WOLFCLUSTER_list_count].Freq = freq;
						strcpy(WIFI_WOLFCLUSTER_list[WIFI_WOLFCLUSTER_list_count].Callsign, istr_l);
						WIFI_WOLFCLUSTER_list[WIFI_WOLFCLUSTER_list_count].Azimuth = 0;
						if (TRX.FFT_DXCluster_Azimuth) {
							CALLSIGN_INFO_LINE *info;
							CALLSIGN_getInfoByCallsign(&info, WIFI_WOLFCLUSTER_list[WIFI_WOLFCLUSTER_list_count].Callsign);
							float32_t my_lat = LOCINFO_get_latlon_from_locator(TRX.LOCATOR, true);
							float32_t my_lon = LOCINFO_get_latlon_from_locator(TRX.LOCATOR, false);
							int16_t azimuth = LOCINFO_azimuthFromCoordinates(my_lat, my_lon, info->lat, info->lon);
							WIFI_WOLFCLUSTER_list[WIFI_WOLFCLUSTER_list_count].Azimuth = azimuth;
						}

						WIFI_WOLFCLUSTER_list_count++;

						istr_l = istr_r + 1;

						if (WIFI_WOLFCLUSTER_list_count >= WIFI_WOLFCLUSTER_MAX_RECORDS) {
							end = true;
						}
					} else {
						end = true;
					}
				} else {
					end = true;
				}
			} else {
				end = true;
			}
		}

		// for(uint16_t i = 0; i < WIFI_WOLFCLUSTER_list_count ; i ++)
		// println(WIFI_WOLFCLUSTER_list[i].Freq, "|", WIFI_WOLFCLUSTER_list[i].Callsign);

		println("WOLFCluster updated, count: ", WIFI_WOLFCLUSTER_list_count);
	}
}

bool WIFI_getDXCluster_background(void) {
	if (!WIFI_connected || WIFI_State != WIFI_READY) {
		return false;
	}

	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	if (band < 0) {
		return false;
	}

	char url[64];
	if (BANDS[band].broadcast) {
		strcpy(url, "/trx_services/cluster_short-wave.php?background&band=");
	} else if (TRX.DXCluster_Type == 0) {
		strcpy(url, "/trx_services/cluster_rbn.php?background&band=");
	} else {
		strcpy(url, "/trx_services/cluster_dxsummit.php?background&band=");
	}

	strcat(url, BANDS[band].name);
	sprintf(url, "%s&timeout=%d", url, TRX.FFT_DXCluster_Timeout);
	WIFI_getHTTPpage(WIFI_HOST, url, WIFI_getDXCluster_background_callback, false, false);
	return true;
}

bool WIFI_getWOLFCluster_background(void) {
	if (!WIFI_connected || WIFI_State != WIFI_READY) {
		return false;
	}

	int8_t band = getBandFromFreq(CurrentVFO->Freq, false);
	if (band < 0 || BANDS[band].broadcast) {
		return true;
	}

	char url[150];
	sprintf(url, "/trx_services/wolf_cluster.php?background&freq=%llu&callsign=%s&timeout=%d", CurrentVFO->Freq, TRX.CALLSIGN, TRX.FFT_DXCluster_Timeout);
	WIFI_getHTTPpage(WIFI_HOST, url, WIFI_getWOLFCluster_background_callback, false, false);
	return true;
}

void WIFI_getDXCluster(void) {
	LCDDriver_Fill(BG_COLOR);
	if (WIFI_connected && WIFI_State == WIFI_READY) {
#ifdef LCD_SMALL_INTERFACE
		LCDDriver_printText("Loading...", 10, 20, FG_COLOR, BG_COLOR, 1);
#else
		LCDDriver_printTextFont("Loading...", 10, 20, FG_COLOR, BG_COLOR, &FreeSans9pt7b);
#endif
	} else {
#ifdef LCD_SMALL_INTERFACE
		LCDDriver_printText("No connection", 10, 20, FG_COLOR, BG_COLOR, 1);
#else
		LCDDriver_printTextFont("No connection", 10, 20, FG_COLOR, BG_COLOR, &FreeSans9pt7b);
#endif

		return;
	}

	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	if (band < 0) {
		return;
	}

	char url[64];
	if (BANDS[band].broadcast) {
		strcpy(url, "/trx_services/cluster_short-wave.php?band=");
	} else if (TRX.DXCluster_Type == 0) {
		strcpy(url, "/trx_services/cluster_rbn.php?band=");
	} else {
		strcpy(url, "/trx_services/cluster_dxsummit.php?band=");
	}

	strcat(url, BANDS[band].name);
	WIFI_getHTTPpage(WIFI_HOST, url, WIFI_printText_callback, false, false);
}

void WIFI_getPropagation(void) {
	LCDDriver_Fill(BG_COLOR);
	if (WIFI_connected && WIFI_State == WIFI_READY) {
#ifdef LCD_SMALL_INTERFACE
		LCDDriver_printText("Loading...", 10, 20, FG_COLOR, BG_COLOR, 1);
#else
		LCDDriver_printTextFont("Loading...", 10, 20, FG_COLOR, BG_COLOR, &FreeSans9pt7b);
#endif
	} else {
#ifdef LCD_SMALL_INTERFACE
		LCDDriver_printText("No connection", 10, 20, FG_COLOR, BG_COLOR, 1);
#else
		LCDDriver_printTextFont("No connection", 10, 20, FG_COLOR, BG_COLOR, &FreeSans9pt7b);
#endif

		return;
	}
	char buff[64] = {0};
	sprintf(buff, "/trx_services/propagination.php?width=%u&height=%u", LCD_WIDTH, LCD_HEIGHT);
	WIFI_getHTTPpage(WIFI_HOST, buff, WIFI_printImage_Propagation_callback, false, false);
}

void WIFI_getTropo(void) {
	LCDDriver_Fill(BG_COLOR);
	if (WIFI_connected && WIFI_State == WIFI_READY) {
		LCDDriver_printTextFont("Loading...", 10, 20, FG_COLOR, BG_COLOR, &FreeSans9pt7b);
	} else {
		LCDDriver_printTextFont("No connection", 10, 20, FG_COLOR, BG_COLOR, &FreeSans9pt7b);
		return;
	}
	char buff[64] = {0};
	sprintf(buff, "/trx_services/tropo.php?region=%u", TRX.TROPO_Region);
	WIFI_getHTTPpage(WIFI_HOST, "/trx_services/tropo.php", WIFI_printImage_Tropo_callback, false, false);
}

void WIFI_getDayNightMap(void) {
	LCDDriver_Fill(BG_COLOR);
	if (WIFI_connected && WIFI_State == WIFI_READY) {
		LCDDriver_printTextFont("Loading...", 10, 20, FG_COLOR, BG_COLOR, &FreeSans9pt7b);
	} else {
		LCDDriver_printTextFont("No connection", 10, 20, FG_COLOR, BG_COLOR, &FreeSans9pt7b);
		return;
	}
	WIFI_getHTTPpage(WIFI_HOST, "/trx_services/daynight.php", WIFI_printImage_DayNight_callback, false, false);
}

void WIFI_getIonogram(void) {
	LCDDriver_Fill(BG_COLOR);
	if (WIFI_connected && WIFI_State == WIFI_READY) {
		LCDDriver_printTextFont("Loading...", 10, 20, FG_COLOR, BG_COLOR, &FreeSans9pt7b);
	} else {
		LCDDriver_printTextFont("No connection", 10, 20, FG_COLOR, BG_COLOR, &FreeSans9pt7b);
		return;
	}
	char buff[64] = {0};
	sprintf(buff, "/trx_services/ionogram.php?ursiCode=%s", TRX.URSI_CODE);
	WIFI_getHTTPpage(WIFI_HOST, buff, WIFI_printImage_Ionogram_callback, false, false);
}

bool WIFI_SW_Restart(void (*callback)(void)) {
	println("[WIFI] SW Restart");
	WIFI_State = WIFI_PROCESS_COMMAND;
	WIFI_ProcessingCommand = WIFI_COMM_SW_RESTART;
	WIFI_ProcessingCommandCallback = callback;
	WIFI_SendCommand("AT+RST\r\n"); // Start Update Firmware
	// WIFI_WaitForOk();
	return true;
}

static void WIFI_checkFWUpdates_callback(void) {
	if (WIFI_HTTP_Response_Status == 200) {
		if (strstr(WIFI_HTTResponseHTML, "new all") != NULL) {
			WIFI_NewFW_STM32 = true;
			WIFI_NewFW_FPGA = true;
			LCD_showTooltip("New Firmware available");
		} else if (strstr(WIFI_HTTResponseHTML, "new stm32") != NULL) {
			WIFI_NewFW_STM32 = true;
			LCD_showTooltip("New STM FW available");
		} else if (strstr(WIFI_HTTResponseHTML, "new fpga") != NULL) {
			WIFI_NewFW_FPGA = true;
			LCD_showTooltip("New FPGA FW available");
		}
	}
	WIFI_NewFW_checked = true;
	if (sysmenu_ota_opened) {
		LCD_UpdateQuery.SystemMenuRedraw = true;
	}
}

void WIFI_checkFWUpdates(void) {
	if (WIFI_connected && WIFI_State != WIFI_READY) {
		return;
	}
	char url[128];
	sprintf(url, "/trx_services/check_fw_updates_2.php?dev=0&stm32=%s&fpga=%d.%d.%d&lcd=%s&callsign=%s", STM32_VERSION_STR, FPGA_FW_Version[2], FPGA_FW_Version[1], FPGA_FW_Version[0],
	        ota_config_lcd, TRX.CALLSIGN);
	WIFI_getHTTPpage(WIFI_HOST, url, WIFI_checkFWUpdates_callback, false, false);
}

static char *WIFI_downloadFileToSD_filename;
static char WIFI_downloadFileToSD_url[128] = {0};
static uint32_t WIFI_downloadFileToSD_startIndex = 0;
#define WIFI_downloadFileToSD_part_size 2000
static void WIFI_WIFI_downloadFileToSD_callback_writed(void) {
	bool isTLE = strstr(WIFI_downloadFileToSD_url, "get_ham_tle") != NULL;
	if (!isTLE && !sysmenu_ota_opened) {
		println("OTA cancelled");
		return;
	}

	static int32_t downloaded_kb_prev = 0;
	if (WIFI_downloadFileToSD_compleated) {
		if (!isTLE) {
			LCD_busy = false;
		} else {
			LCD_showInfo("Download completed", true);
		}
		WIFI_download_inprogress = false;
		LCD_UpdateQuery.SystemMenuRedraw = true;
	} else {
		char url[128] = {0};
		sprintf(url, "%s&start=%d&count=%d", WIFI_downloadFileToSD_url, WIFI_downloadFileToSD_startIndex, WIFI_downloadFileToSD_part_size);
		println("[WIFI] Get next file part");
		WIFI_getHTTPpage(WIFI_HOST, url, WIFI_downloadFileToSD_callback, false, false);

		// progress
		int32_t downloaded_kb = WIFI_downloadFileToSD_startIndex / 1024;
		if (abs(downloaded_kb_prev - downloaded_kb) >= 10) {
			char buff[64] = {0};
#if LCD_SMALL_INTERFACE
			sprintf(buff, "Download %dk", downloaded_kb);
#else
			sprintf(buff, "Downloading file to SD ... %dk", downloaded_kb);
#endif
			LCD_showInfo(buff, false);
			downloaded_kb_prev = downloaded_kb;
		}
	}
}

static void WIFI_downloadFileToSD_callback(void) {
	bool isTLE = strstr(WIFI_downloadFileToSD_url, "get_ham_tle") != NULL;

	static uint8_t WIFI_HTTP_error_retryes = 0;
	if (WIFI_HTTP_Response_Status == 200) {
		WIFI_HTTP_error_retryes = 0;
		// parse hex output from server (convert to bin)
		char *istr = WIFI_HTTResponseHTML;
		char hex[5] = {0};
		uint32_t WIFI_DecodedStreamBuffer_index = 0;
		int16_t val = 0;
		uint32_t len = strlen(WIFI_HTTResponseHTML);
		println(len);
		while (*istr != 0 && (len >= ((WIFI_DecodedStreamBuffer_index * 2) + 2))) {
			// Get hex
			strncpy(hex, istr, 2);
			val = (int16_t)(strtol(hex, NULL, 16));
			istr += 2;
			// Save
			WIFI_HTTResponseHTML[WIFI_DecodedStreamBuffer_index] = val;
			WIFI_DecodedStreamBuffer_index++;
		}
		//
		if (WIFI_DecodedStreamBuffer_index > 0) {
			WIFI_downloadFileToSD_compleated = false;
			strcpy((char *)SD_workbuffer_A, WIFI_downloadFileToSD_filename);
			dma_memcpy((char *)SD_workbuffer_B, WIFI_HTTResponseHTML, WIFI_DecodedStreamBuffer_index);
			SDCOMM_WRITE_TO_FILE_callback = WIFI_WIFI_downloadFileToSD_callback_writed;
			SDCOMM_WRITE_TO_FILE_partsize = WIFI_DecodedStreamBuffer_index;
			println("File part ", WIFI_downloadFileToSD_startIndex, " downloaded");
			WIFI_downloadFileToSD_startIndex += WIFI_DecodedStreamBuffer_index;
			SD_doCommand(SDCOMM_WRITE_TO_FILE, false);
		} else {
			WIFI_downloadFileToSD_compleated = true;
			WIFI_WIFI_downloadFileToSD_callback_writed();
		}
	} else {
		println("BAD HTTP status ", WIFI_HTTP_Response_Status);
		if (WIFI_HTTP_error_retryes < 10) {
			WIFI_HTTP_error_retryes++;
			println("RETRY: ", WIFI_HTTP_error_retryes);
			println(WIFI_HOSTuri, WIFI_GETuri);
			HAL_Delay(1000);
			WIFI_getHTTPpage(WIFI_HOSTuri, WIFI_GETuri, WIFI_downloadFileToSD_callback, false, false);
		} else {
			sysmenu_ota_opened = false;
			if (!isTLE) {
				LCD_busy = false;
				LCD_redraw(false);
			} else {
				LCD_showInfo("Downloading error", true);
			}
		}
	}
}

bool WIFI_downloadFileToSD(char *url, char *filename) {
	if (!WIFI_connected || WIFI_State != WIFI_READY) {
		return false;
	}
	get_HTTP_tryes = 0;
	WIFI_download_inprogress = true;
	WIFI_downloadFileToSD_compleated = false;
	WIFI_downloadFileToSD_filename = filename;
	WIFI_downloadFileToSD_startIndex = 0;
	strcpy(WIFI_downloadFileToSD_url, url);
	sprintf(url, "%s&start=%d&count=%d", url, WIFI_downloadFileToSD_startIndex, WIFI_downloadFileToSD_part_size);
	WIFI_getHTTPpage(WIFI_HOST, url, WIFI_downloadFileToSD_callback, false, false);

	return true;
}

void WIFI_postQSOtoAllQSO(char *call, char *note, char *date, char *time, char *rsts, char *rstr, char *mode, char *band, char *name, char *qth) {
	if (WIFI_connected && WIFI_State != WIFI_READY) {
		return;
	}
	if (strlen(WIFI.ALLQSO_TOKEN) == 0 || strlen(WIFI.ALLQSO_LOGID) == 0) {
		return;
	}

	//	"lid"   : ID-номер лога *
	//  "token" : 16-символьный токен пользователя (Например: "abcd44er55uiopa6") *
	//  "call"  : Позывной корреспондента в верхнем регистре (Например: "RU4PN") *
	//  "note"  : Текстовая заметка (для моды FT8 - частота трансивера. Например: "3.574225")
	//  "date"  : Дата проведения QSO (формат YYYYMMDD, например: "20230227") *
	//  "time"  : Время проведения QSO (формат HHMMSS или HHMM, например: "233459" или "2334") *
	//  "rsts"  : Переданный RST (Формат в зависимости от моды, 3 знака максимально. Если не заполнено, то = 59. Для FT8 значение от "-24" до "+24")
	//  "rstr"  : Принятый RST (Формат в зависимости от моды, 3 знака максимально. Если не заполнено, то = 59. Для FT8 значение от "-24" до "+24")
	//  "mode"  : Вид излучения (Принимаются только значения: "SSB", "CW", "FM", "PSK", "RTTY", "SSTV", "MFSK", "BPSK", "HELL", "AMTOR", "PACKET", "THROB", "MT63", "OLIVIA", "AM", "FT8",
	//  "JT65". "band"  : Диапазон (Метровые обозначения. Принимаются: "160m", "80m", "40m", "30m", "20m", "17m", "15m", "12m", "10m", "6m", "2m", "70cm", "23cm", "13cm", "9cm", "6cm", "3cm",
	//  "1.25cm". "name"  : Имя оператора (Например, "Вадим") "qth"   : QTH оператора (Например, "Астрахань")

	sprintf(WIFI_POSTdata, "lid=%s&token=%s&call=%s&note=%s&date=%s&time=%s&rsts=%s&rstr=%s&mode=%s&band=%s&name=%s&qth=%s", WIFI.ALLQSO_LOGID, WIFI.ALLQSO_TOKEN, call, note, date, time, rsts,
	        rstr, mode, band, name, qth);

	WIFI_postHTTPpage("allqso.ru", "/api_wolf.php", NULL, false, false);
}

#endif
