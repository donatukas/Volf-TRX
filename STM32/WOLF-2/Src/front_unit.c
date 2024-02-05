#include "front_unit.h"
#include "agc.h"
#include "atu.h"
#include "audio_filters.h"
#include "auto_notch.h"
#include "functions.h"
#include "lcd.h"
#include "main.h"
#include "noise_reduction.h"
#include "rf_unit.h"
#include "sd.h"
#include "settings.h"
#include "stm32h7xx_hal.h"
#include "system_menu.h"
#include "trx_manager.h"
#include "vad.h"

int8_t FRONTPANEL_ProcessEncoder1 = 0;
int8_t FRONTPANEL_ProcessEncoder2 = 0;
int8_t FRONTPANEL_ProcessEncoder3 = 0;
int8_t FRONTPANEL_ProcessEncoder4 = 0;

#ifdef HRDW_MCP3008_1
bool FRONTPanel_MCP3008_1_Enabled = true;
#endif

static void FRONTPANEL_ENCODER2_Rotated(int8_t direction);
static void FRONTPANEL_ENCODER2_Rotated(int8_t direction);
static void FRONTPANEL_ENC2SW_click_handler(uint32_t parameter);
static void FRONTPANEL_ENC2SW_hold_handler(uint32_t parameter);
static void FRONTPANEL_ENC3SW_click_handler(uint32_t parameter);
static void FRONTPANEL_ENC3SW_hold_handler(uint32_t parameter);
static void FRONTPANEL_ENC4SW_click_handler(uint32_t parameter);
static void FRONTPANEL_ENC4SW_hold_handler(uint32_t parameter);
static int32_t FRONTPANEL_ReadMCP3008_Value(uint8_t channel, uint8_t adc_num, uint8_t count);

PERIPH_FrontPanel_Button PERIPH_FrontPanel_Buttons[] = {
    // buttons
    //{.port = 1, .channel = 0, .type = FUNIT_CTRL_BUTTON, .tres_min = 15, .tres_max = 77, .state = false, .prev_state = false, .work_in_menu = true, .parameter = 0, .clickHandler = ,
    //.holdHandler = }, // RESERVED
    //{.port = 1, .channel = 0, .type = FUNIT_CTRL_BUTTON, .tres_min = 77, .tres_max = 170, .state = false, .prev_state = false, .work_in_menu = true, .parameter = 0, .clickHandler = ,
    //.holdHandler = }, // RESERVED
    //{.port = 1, .channel = 0, .type = FUNIT_CTRL_BUTTON, .tres_min = 170, .tres_max = 263, .state = false, .prev_state = false, .work_in_menu = true, .parameter = 0, .clickHandler = ,
    //.holdHandler = }, // RESERVED
    //{.port = 1, .channel = 0, .type = FUNIT_CTRL_BUTTON, .tres_min = 263, .tres_max = 369, .state = false, .prev_state = false, .work_in_menu = true, .parameter = 0, .clickHandler = ,
    //.holdHandler = }, // RESERVED
    //{.port = 1, .channel = 0, .type = FUNIT_CTRL_BUTTON, .tres_min = 369, .tres_max = 449, .state = false, .prev_state = false, .work_in_menu = true, .parameter = 0, .clickHandler = ,
    //.holdHandler = }, // RESERVED
    //{.port = 1, .channel = 0, .type = FUNIT_CTRL_BUTTON, .tres_min = 449, .tres_max = 543, .state = false, .prev_state = false, .work_in_menu = true, .parameter = 0, .clickHandler = ,
    //.holdHandler = }, // RESERVED
    //{.port = 1, .channel = 0, .type = FUNIT_CTRL_BUTTON, .tres_min = 543, .tres_max = 636, .state = false, .prev_state = false, .work_in_menu = true, .parameter = 0, .clickHandler = ,
    //.holdHandler = }, // RESERVED
    //{.port = 1, .channel = 0, .type = FUNIT_CTRL_BUTTON, .tres_min = 636, .tres_max = 729, .state = false, .prev_state = false, .work_in_menu = true, .parameter = 0, .clickHandler = ,
    //.holdHandler = }, // RESERVED
    //{.port = 1, .channel = 0, .type = FUNIT_CTRL_BUTTON, .tres_min = 729, .tres_max = 822, .state = false, .prev_state = false, .work_in_menu = true, .parameter = 0, .clickHandler = ,
    //.holdHandler = }, // RESERVED
    //{.port = 1, .channel = 0, .type = FUNIT_CTRL_BUTTON, .tres_min = 822, .tres_max = 884, .state = false, .prev_state = false, .work_in_menu = true, .parameter = 0, .clickHandler = ,
    //.holdHandler = }, // RESERVED

    {.port = 1,
     .channel = 1,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 15,
     .tres_max = 77,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = 0,
     .clickHandler = FRONTPANEL_ENC4SW_click_handler,
     .holdHandler = FRONTPANEL_ENC4SW_hold_handler},
    //{.port = 1, .channel = 1, .type = FUNIT_CTRL_BUTTON, .tres_min = 77, .tres_max = 170, .state = false, .prev_state = false, .work_in_menu = true, .parameter = 0, .clickHandler = ,
    //.holdHandler = }, // RESERVED
    //{.port = 1, .channel = 1, .type = FUNIT_CTRL_BUTTON, .tres_min = 170, .tres_max = 263, .state = false, .prev_state = false, .work_in_menu = true, .parameter = 0, .clickHandler = ,
    //.holdHandler = }, // RESERVED
    {.port = 1,
     .channel = 1,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 263,
     .tres_max = 369,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = 0,
     .clickHandler = BUTTONHANDLER_RIGHT_ARR,
     .holdHandler = BUTTONHANDLER_LEFT_ARR}, // F
    {.port = 1,
     .channel = 1,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 369,
     .tres_max = 449,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = 0,
     .clickHandler = BUTTONHANDLER_MUTE,
     .holdHandler = BUTTONHANDLER_MUTE_AFAMP},
    //{.port = 1, .channel = 1, .type = FUNIT_CTRL_BUTTON, .tres_min = 449, .tres_max = 543, .state = false, .prev_state = false, .work_in_menu = true, .parameter = 0, .clickHandler = ,
    //.holdHandler = }, // RESERVED
    //{.port = 1, .channel = 1, .type = FUNIT_CTRL_BUTTON, .tres_min = 543, .tres_max = 636, .state = false, .prev_state = false, .work_in_menu = true, .parameter = 0, .clickHandler = ,
    //.holdHandler = }, // RESERVED
    //{.port = 1, .channel = 1, .type = FUNIT_CTRL_BUTTON, .tres_min = 636, .tres_max = 729, .state = false, .prev_state = false, .work_in_menu = true, .parameter = 0, .clickHandler = ,
    //.holdHandler = }, // RESERVED
    {.port = 1,
     .channel = 1,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 729,
     .tres_max = 822,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = 0,
     .clickHandler = BUTTONHANDLER_ANT,
     .holdHandler = BUTTONHANDLER_ANT},
    //{.port = 1, .channel = 1, .type = FUNIT_CTRL_BUTTON, .tres_min = 822, .tres_max = 884, .state = false, .prev_state = false, .work_in_menu = true, .parameter = 0, .clickHandler = ,
    //.holdHandler = }, // RESERVED

    {.port = 1,
     .channel = 2,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 15,
     .tres_max = 77,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = BANDID_160m,
     .clickHandler = BUTTONHANDLER_GET_BAND_MEMORY,
     .holdHandler = BUTTONHANDLER_SET_BAND_MEMORY},
    {.port = 1,
     .channel = 2,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 77,
     .tres_max = 170,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = BANDID_80m,
     .clickHandler = BUTTONHANDLER_GET_BAND_MEMORY,
     .holdHandler = BUTTONHANDLER_SET_BAND_MEMORY},
    {.port = 1,
     .channel = 2,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 170,
     .tres_max = 263,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = BANDID_40m,
     .clickHandler = BUTTONHANDLER_GET_BAND_MEMORY,
     .holdHandler = BUTTONHANDLER_SET_BAND_MEMORY},
    {.port = 1,
     .channel = 2,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 263,
     .tres_max = 369,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = BANDID_30m,
     .clickHandler = BUTTONHANDLER_GET_BAND_MEMORY,
     .holdHandler = BUTTONHANDLER_SET_BAND_MEMORY},
    {.port = 1,
     .channel = 2,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 369,
     .tres_max = 449,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = BANDID_20m,
     .clickHandler = BUTTONHANDLER_GET_BAND_MEMORY,
     .holdHandler = BUTTONHANDLER_SET_BAND_MEMORY},
    {.port = 1,
     .channel = 2,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 449,
     .tres_max = 543,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = BANDID_17m,
     .clickHandler = BUTTONHANDLER_GET_BAND_MEMORY,
     .holdHandler = BUTTONHANDLER_SET_BAND_MEMORY},
    {.port = 1,
     .channel = 2,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 543,
     .tres_max = 636,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = BANDID_15m,
     .clickHandler = BUTTONHANDLER_GET_BAND_MEMORY,
     .holdHandler = BUTTONHANDLER_SET_BAND_MEMORY},
    {.port = 1,
     .channel = 2,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 636,
     .tres_max = 729,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = BANDID_12m,
     .clickHandler = BUTTONHANDLER_GET_BAND_MEMORY,
     .holdHandler = BUTTONHANDLER_SET_BAND_MEMORY},
    {.port = 1,
     .channel = 2,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 729,
     .tres_max = 800,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = BANDID_10m,
     .clickHandler = BUTTONHANDLER_GET_BAND_MEMORY,
     .holdHandler = BUTTONHANDLER_SET_BAND_MEMORY},
    {.port = 1,
     .channel = 2,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 800,
     .tres_max = 884,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = BANDID_2m,
     .clickHandler = BUTTONHANDLER_GET_BAND_MEMORY,
     .holdHandler = BUTTONHANDLER_SET_BAND_MEMORY},

    {.port = 1, .channel = 3, .type = FUNIT_CTRL_AF_GAIN}, // AF GAIN
    {.port = 1, .channel = 4, .type = FUNIT_CTRL_IF_GAIN}, // IF GAIN
    {.port = 1, .channel = 5, .type = FUNIT_CTRL_RIT},     // RIT

    {.port = 1,
     .channel = 6,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 15,
     .tres_max = 77,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = 0,
     .clickHandler = BUTTONHANDLER_MENU,
     .holdHandler = BUTTONHANDLER_MENUHOLD},
    {.port = 1,
     .channel = 6,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 77,
     .tres_max = 170,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = 0,
     .clickHandler = BUTTONHANDLER_RIT,
     .holdHandler = BUTTONHANDLER_XIT},
    {.port = 1,
     .channel = 6,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 170,
     .tres_max = 263,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = 0,
     .clickHandler = BUTTONHANDLER_MODE_P,
     .holdHandler = BUTTONHANDLER_MODE_N},
    {.port = 1,
     .channel = 6,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 263,
     .tres_max = 369,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = 0,
     .clickHandler = FRONTPANEL_ENC2SW_click_handler,
     .holdHandler = FRONTPANEL_ENC2SW_hold_handler},
    //{.port = 1, .channel = 6, .type = FUNIT_CTRL_BUTTON, .tres_min = 369, .tres_max = 449, .state = false, .prev_state = false, .work_in_menu = true, .parameter = 0, .clickHandler = ,
    //.holdHandler = }, //RESERVED
    {.port = 1,
     .channel = 6,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 449,
     .tres_max = 543,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = 0,
     .clickHandler = BUTTONHANDLER_LOCK,
     .holdHandler = BUTTONHANDLER_LOCK},
    {.port = 1,
     .channel = 6,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 543,
     .tres_max = 636,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = 0,
     .clickHandler = BUTTONHANDLER_PTT_TOGGLE,
     .holdHandler = BUTTONHANDLER_PTT_TOGGLE},
    {.port = 1,
     .channel = 6,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 636,
     .tres_max = 729,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = 0,
     .clickHandler = BUTTONHANDLER_AsB,
     .holdHandler = BUTTONHANDLER_ArB},
    {.port = 1,
     .channel = 6,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 729,
     .tres_max = 800,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = 8,
     .clickHandler = BUTTONHANDLER_FUNC,
     .holdHandler = BUTTONHANDLER_FUNCH},
    {.port = 1,
     .channel = 6,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 800,
     .tres_max = 884,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = 0,
     .clickHandler = BUTTONHANDLER_VOX,
     .holdHandler = BUTTONHANDLER_VOX},

    {.port = 1,
     .channel = 7,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 15,
     .tres_max = 77,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = 0,
     .clickHandler = BUTTONHANDLER_FUNC,
     .holdHandler = BUTTONHANDLER_FUNCH},
    {.port = 1,
     .channel = 7,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 77,
     .tres_max = 170,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = 1,
     .clickHandler = BUTTONHANDLER_FUNC,
     .holdHandler = BUTTONHANDLER_FUNCH},
    {.port = 1,
     .channel = 7,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 170,
     .tres_max = 263,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = 2,
     .clickHandler = BUTTONHANDLER_FUNC,
     .holdHandler = BUTTONHANDLER_FUNCH},
    {.port = 1,
     .channel = 7,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 263,
     .tres_max = 369,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = 3,
     .clickHandler = BUTTONHANDLER_FUNC,
     .holdHandler = BUTTONHANDLER_FUNCH},
    {.port = 1,
     .channel = 7,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 369,
     .tres_max = 449,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = 4,
     .clickHandler = BUTTONHANDLER_FUNC,
     .holdHandler = BUTTONHANDLER_FUNCH},
    {.port = 1,
     .channel = 7,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 449,
     .tres_max = 543,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = 5,
     .clickHandler = BUTTONHANDLER_FUNC,
     .holdHandler = BUTTONHANDLER_FUNCH},
    {.port = 1,
     .channel = 7,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 543,
     .tres_max = 636,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = 6,
     .clickHandler = BUTTONHANDLER_FUNC,
     .holdHandler = BUTTONHANDLER_FUNCH},
    {.port = 1,
     .channel = 7,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 636,
     .tres_max = 720,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = 7,
     .clickHandler = BUTTONHANDLER_FUNC,
     .holdHandler = BUTTONHANDLER_FUNCH},
    {.port = 1,
     .channel = 7,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 720,
     .tres_max = 822,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = 0,
     .clickHandler = BUTTONHANDLER_TUNE,
     .holdHandler = BUTTONHANDLER_TUNER},
    {.port = 1,
     .channel = 7,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 800,
     .tres_max = 884,
     .state = false,
     .prev_state = false,
     .work_in_menu = true,
     .parameter = 0,
     .clickHandler = FRONTPANEL_ENC3SW_click_handler,
     .holdHandler = FRONTPANEL_ENC3SW_hold_handler},
};

const PERIPH_FrontPanel_FuncButton PERIPH_FrontPanel_FuncButtonsList[FUNCBUTTONS_COUNT] = {
    {.name = "A / B", .work_in_menu = false, .clickHandler = BUTTONHANDLER_AsB, .holdHandler = BUTTONHANDLER_AsB, .checkBool = NULL},
    {.name = "B=A", .work_in_menu = false, .clickHandler = BUTTONHANDLER_ArB, .holdHandler = BUTTONHANDLER_ArB, .checkBool = NULL},
    {.name = "SPLIT", .work_in_menu = false, .clickHandler = BUTTONHANDLER_SPLIT, .holdHandler = BUTTONHANDLER_SPLIT, .checkBool = (uint32_t *)&TRX.SPLIT_Enabled},
    {.name = "DOUBLE", .work_in_menu = false, .clickHandler = BUTTONHANDLER_DOUBLE, .holdHandler = BUTTONHANDLER_DOUBLEMODE, .checkBool = (uint32_t *)&TRX.Dual_RX},
    {.name = "POWER", .work_in_menu = true, .clickHandler = BUTTONHANDLER_RF_POWER, .holdHandler = BUTTONHANDLER_RF_POWER, .checkBool = NULL},
    {.name = "XIT", .work_in_menu = true, .clickHandler = BUTTONHANDLER_XIT, .holdHandler = BUTTONHANDLER_XIT, .checkBool = (uint32_t *)&TRX.XIT_Enabled},
    {.name = "RIT", .work_in_menu = false, .clickHandler = BUTTONHANDLER_RIT, .holdHandler = BUTTONHANDLER_RIT, .checkBool = (uint32_t *)&TRX.RIT_Enabled},
    {.name = "REC", .work_in_menu = false, .clickHandler = BUTTONHANDLER_REC, .holdHandler = BUTTONHANDLER_REC, .checkBool = (uint32_t *)&SD_RecordInProcess},
    {.name = "SERVICE", .work_in_menu = true, .clickHandler = BUTTONHANDLER_SERVICES, .holdHandler = BUTTONHANDLER_SCREENSHOT, .checkBool = NULL},

    {.name = "SAMPL-", .work_in_menu = true, .clickHandler = BUTTONHANDLER_SAMPLE_N, .holdHandler = BUTTONHANDLER_SAMPLE_N, .checkBool = NULL},
    {.name = "SAMPL+", .work_in_menu = false, .clickHandler = BUTTONHANDLER_SAMPLE_P, .holdHandler = BUTTONHANDLER_SAMPLE_P, .checkBool = NULL},
    {.name = "ZOOM-", .work_in_menu = false, .clickHandler = BUTTONHANDLER_ZOOM_N, .holdHandler = BUTTONHANDLER_ZOOM_N, .checkBool = NULL},
    {.name = "ZOOM+", .work_in_menu = false, .clickHandler = BUTTONHANDLER_ZOOM_P, .holdHandler = BUTTONHANDLER_ZOOM_P, .checkBool = NULL},
    {.name = "WPM", .work_in_menu = true, .clickHandler = BUTTONHANDLER_WPM, .holdHandler = BUTTONHANDLER_WPM, .checkBool = NULL},
    {.name = "SNAP <", .work_in_menu = false, .clickHandler = BUTTONHANDLER_SNAP_LEFT, .holdHandler = BUTTONHANDLER_AUTO_SNAP, .checkBool = (uint32_t *)&TRX.Auto_Snap},
    {.name = "SNAP >", .work_in_menu = false, .clickHandler = BUTTONHANDLER_SNAP_RIGHT, .holdHandler = BUTTONHANDLER_AUTO_SNAP, .checkBool = (uint32_t *)&TRX.Auto_Snap},
    {.name = "CQ", .work_in_menu = false, .clickHandler = BUTTONHANDLER_CQ, .holdHandler = BUTTONHANDLER_FILEMANAGER, .checkBool = (uint32_t *)&SD_PlayInProcess},
    {.name = "TUNE", .work_in_menu = true, .clickHandler = BUTTONHANDLER_TUNE, .holdHandler = BUTTONHANDLER_TUNER, .checkBool = (uint32_t *)&TRX.TUNER_Enabled},

    {.name = "SCAN", .work_in_menu = false, .clickHandler = BUTTONHANDLER_SCAN, .holdHandler = BUTTONHANDLER_SCAN, .checkBool = (uint32_t *)&TRX_ScanMode},
    {.name = "HPF", .work_in_menu = true, .clickHandler = BUTTONHANDLER_HPF, .holdHandler = BUTTONHANDLER_HPF, .checkBool = NULL},
    {.name = "MODE+", .work_in_menu = false, .clickHandler = BUTTONHANDLER_MODE_P, .holdHandler = BUTTONHANDLER_MODE_P, .checkBool = NULL},
    {.name = "MODE-", .work_in_menu = false, .clickHandler = BUTTONHANDLER_MODE_N, .holdHandler = BUTTONHANDLER_MODE_N, .checkBool = NULL},
    {.name = "BANDMP", .work_in_menu = false, .clickHandler = BUTTONHANDLER_BANDMAP, .holdHandler = BUTTONHANDLER_BANDMAP, .checkBool = (uint32_t *)&TRX.BandMapEnabled},
    {.name = "BAND-", .work_in_menu = false, .clickHandler = BUTTONHANDLER_BAND_N, .holdHandler = BUTTONHANDLER_BAND_N, .checkBool = NULL},
    {.name = "BAND+", .work_in_menu = false, .clickHandler = BUTTONHANDLER_BAND_P, .holdHandler = BUTTONHANDLER_BAND_P, .checkBool = NULL},
    {.name = "AUTOGN", .work_in_menu = false, .clickHandler = BUTTONHANDLER_AUTOGAINER, .holdHandler = BUTTONHANDLER_AUTOGAINER, .checkBool = (uint32_t *)&TRX.AutoGain},
    {.name = "LOCK", .work_in_menu = true, .clickHandler = BUTTONHANDLER_LOCK, .holdHandler = BUTTONHANDLER_LOCK, .checkBool = (uint32_t *)&TRX.Locked},

    {.name = TRX.CW_Macros_Name_1, .work_in_menu = false, .clickHandler = BUTTONHANDLER_CW_MACROS, .holdHandler = BUTTONHANDLER_CW_MACROS, .parameter = 1},
    {.name = TRX.CW_Macros_Name_2, .work_in_menu = false, .clickHandler = BUTTONHANDLER_CW_MACROS, .holdHandler = BUTTONHANDLER_CW_MACROS, .parameter = 2},
    {.name = TRX.CW_Macros_Name_3, .work_in_menu = false, .clickHandler = BUTTONHANDLER_CW_MACROS, .holdHandler = BUTTONHANDLER_CW_MACROS, .parameter = 3},
    {.name = TRX.CW_Macros_Name_4, .work_in_menu = false, .clickHandler = BUTTONHANDLER_CW_MACROS, .holdHandler = BUTTONHANDLER_CW_MACROS, .parameter = 4},
    {.name = TRX.CW_Macros_Name_5, .work_in_menu = false, .clickHandler = BUTTONHANDLER_CW_MACROS, .holdHandler = BUTTONHANDLER_CW_MACROS, .parameter = 5},
    {.name = "CESSB", .work_in_menu = false, .clickHandler = BUTTONHANDLER_CESSB, .holdHandler = BUTTONHANDLER_CESSB, .checkBool = (uint32_t *)&TRX.TX_CESSB},
    {.name = "FT8", .work_in_menu = false, .clickHandler = BUTTONHANDLER_FT8, .holdHandler = BUTTONHANDLER_FT8, .checkBool = NULL},
    {.name = "VOX", .work_in_menu = true, .clickHandler = BUTTONHANDLER_VOX, .holdHandler = BUTTONHANDLER_VOX, .checkBool = (uint32_t *)&TRX.VOX},
    {.name = "FILES", .work_in_menu = false, .clickHandler = BUTTONHANDLER_FILEMANAGER, .holdHandler = BUTTONHANDLER_FILEMANAGER, .checkBool = NULL},

    // hidden entry for menu editor
    {.name = "MENU", .work_in_menu = true, .clickHandler = BUTTONHANDLER_MENU, .holdHandler = BUTTONHANDLER_MENU, .checkBool = NULL},
    {.name = "SQL", .work_in_menu = true, .clickHandler = BUTTONHANDLER_SQUELCH, .holdHandler = BUTTONHANDLER_SQUELCH, .checkBool = (uint32_t *)&TRX.SQL_shadow},
    {.name = "ANT", .work_in_menu = false, .clickHandler = BUTTONHANDLER_ANT, .holdHandler = BUTTONHANDLER_ANT, .checkBool = NULL},
    {.name = "REPIT", .work_in_menu = false, .clickHandler = BUTTONHANDLER_REPEATER_MODE, .holdHandler = BUTTONHANDLER_REPEATER_MODE, .checkBool = (uint32_t *)&TRX.RepeaterMode_shadow},
    {.name = "DPD", .work_in_menu = true, .clickHandler = BUTTONHANDLER_DPD, .holdHandler = BUTTONHANDLER_DPD_CALIBRATE, .checkBool = (uint32_t *)&TRX.Digital_Pre_Distortion},
    {.name = "MEMORY", .work_in_menu = false, .clickHandler = LCD_ShowMemoryChannelsButtonHandler, .holdHandler = LCD_ShowMemoryChannelsButtonHandler, .checkBool = NULL},
    {.name = "NONE", .work_in_menu = false, .clickHandler = NULL, .holdHandler = NULL, .checkBool = NULL},
};

PERIPH_FrontPanel_Button PERIPH_FrontPanel_TANGENT_MH36[] = {
    {.port = 1, .channel = 2, .type = FUNIT_CTRL_PTT, .tres_min = 200, .tres_max = 430}, // PTT_SW1 - PTT
    {.port = 1,
     .channel = 2,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 430,
     .tres_max = 640,
     .state = false,
     .prev_state = false,
     .work_in_menu = false,
     .parameter = 0,
     .clickHandler = BUTTONHANDLER_DOWN,
     .holdHandler = BUTTONHANDLER_DOWN}, // PTT_SW1 - DOWN
    {.port = 1,
     .channel = 2,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 640,
     .tres_max = 805,
     .state = false,
     .prev_state = false,
     .work_in_menu = false,
     .parameter = 0,
     .clickHandler = BUTTONHANDLER_UP,
     .holdHandler = BUTTONHANDLER_UP}, // PTT_SW1 - UP
    {.port = 1,
     .channel = 2,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 805,
     .tres_max = 920,
     .state = false,
     .prev_state = false,
     .work_in_menu = false,
     .parameter = 0,
     .clickHandler = BUTTONHANDLER_AGC,
     .holdHandler = BUTTONHANDLER_AGC}, // PTT_SW1 - AGC
    {.port = 1,
     .channel = 3,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 200,
     .tres_max = 430,
     .state = false,
     .prev_state = false,
     .work_in_menu = false,
     .parameter = 0,
     .clickHandler = BUTTONHANDLER_AsB,
     .holdHandler = BUTTONHANDLER_ArB}, // PTT_SW2 - VFO
    {.port = 1,
     .channel = 3,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 430,
     .tres_max = 640,
     .state = false,
     .prev_state = false,
     .work_in_menu = false,
     .parameter = 0,
     .clickHandler = BUTTONHANDLER_BAND_N,
     .holdHandler = BUTTONHANDLER_MODE_N}, // PTT_SW2 - P1
    {.port = 1,
     .channel = 3,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 640,
     .tres_max = 805,
     .state = false,
     .prev_state = false,
     .work_in_menu = false,
     .parameter = 0,
     .clickHandler = BUTTONHANDLER_BAND_P,
     .holdHandler = BUTTONHANDLER_MODE_P}, // PTT_SW2 - P2
};

PERIPH_FrontPanel_Button PERIPH_FrontPanel_TANGENT_MH48[] = {
    {.port = 1,
     .channel = 2,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 553,
     .tres_max = 633,
     .state = false,
     .prev_state = false,
     .work_in_menu = false,
     .parameter = 0,
     .clickHandler = BUTTONHANDLER_DOWN,
     .holdHandler = BUTTONHANDLER_DOWN}, // PTT_SW1 - DOWN
    {.port = 1,
     .channel = 2,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 341,
     .tres_max = 421,
     .state = false,
     .prev_state = false,
     .work_in_menu = false,
     .parameter = 0,
     .clickHandler = BUTTONHANDLER_UP,
     .holdHandler = BUTTONHANDLER_UP}, // PTT_SW1 - UP
    {.port = 1,
     .channel = 3,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 339,
     .tres_max = 419,
     .state = false,
     .prev_state = false,
     .work_in_menu = false,
     .parameter = 0,
     .clickHandler = BUTTONHANDLER_AGC,
     .holdHandler = BUTTONHANDLER_AGC}, // PTT_SW2 - P1
    {.port = 1,
     .channel = 3,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 542,
     .tres_max = 622,
     .state = false,
     .prev_state = false,
     .work_in_menu = false,
     .parameter = 0,
     .clickHandler = BUTTONHANDLER_AsB,
     .holdHandler = BUTTONHANDLER_ArB}, // PTT_SW2 - P2
    {.port = 1,
     .channel = 3,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 715,
     .tres_max = 795,
     .state = false,
     .prev_state = false,
     .work_in_menu = false,
     .parameter = 0,
     .clickHandler = BUTTONHANDLER_BAND_N,
     .holdHandler = BUTTONHANDLER_MODE_N}, // PTT_SW2 - P3
    {.port = 1,
     .channel = 3,
     .type = FUNIT_CTRL_BUTTON,
     .tres_min = 860,
     .tres_max = 930,
     .state = false,
     .prev_state = false,
     .work_in_menu = false,
     .parameter = 0,
     .clickHandler = BUTTONHANDLER_BAND_P,
     .holdHandler = BUTTONHANDLER_MODE_P}, // PTT_SW2 - P4
};

void FRONTPANEL_ENCODER_Rotated(float32_t direction) // rotated encoder, handler here, direction -1 - left, 1 - right
{
	TRX_Inactive_Time = 0;
	if (TRX.Locked || LCD_window.opened) {
		return;
	}
	if (LCD_systemMenuOpened) {
		FRONTPANEL_ProcessEncoder1 = (int8_t)direction;
		return;
	}
	if (fabsf(direction) <= ENCODER_MIN_RATE_ACCELERATION) {
		direction = (direction < 0.0f) ? -1.0f : 1.0f;
	}

	if (TRX_on_TX) {
		return;
	}

	TRX_DoFrequencyEncoder(direction, false);

	LCD_UpdateQuery.FreqInfo = true;
	NeedSaveSettings = true;
}

static void FRONTPANEL_ENCODER2_Rotated(int8_t direction) // rotated encoder, handler here, direction -1 - left, 1 - right
{
	TRX_Inactive_Time = 0;
	if (TRX.Locked || LCD_window.opened) {
		return;
	}

	if (LCD_systemMenuOpened) {
		SYSMENU_eventSecRotateSystemMenu(direction);
		return;
	}

	FRONTPANEL_ENC2SW_validate();

	if (TRX.ENC2_func_mode == ENC_FUNC_FAST_STEP) {
		if (TRX_on_TX) {
			return;
		}

		TRX_DoFrequencyEncoder(direction, true);

		LCD_UpdateQuery.FreqInfo = true;
	}

	if (TRX.ENC2_func_mode == ENC_FUNC_SET_WPM) {
		// ENC2 Func mode (WPM)
		TRX.CW_KEYER_WPM += direction;
		if (TRX.CW_KEYER_WPM < 1) {
			TRX.CW_KEYER_WPM = 1;
		}
		if (TRX.CW_KEYER_WPM > 200) {
			TRX.CW_KEYER_WPM = 200;
		}
		char sbuff[32] = {0};
		sprintf(sbuff, "WPM: %u", TRX.CW_KEYER_WPM);
		LCD_showTooltip(sbuff);
	}

	if (TRX.ENC2_func_mode == ENC_FUNC_SET_RIT) // Fine RIT/XIT
	{
		if ((!TRX.XIT_Enabled || !TRX_on_TX) && TRX.RIT_Enabled && TRX.FineRITTune) {
			TRX_RIT += direction * 10;
			if (TRX_RIT > TRX.RIT_INTERVAL) {
				TRX_RIT = TRX.RIT_INTERVAL;
			}
			if (TRX_RIT < -TRX.RIT_INTERVAL) {
				TRX_RIT = -TRX.RIT_INTERVAL;
			}

			TRX_setFrequency(CurrentVFO->Freq, CurrentVFO);
		}

		if ((!TRX.RIT_Enabled || TRX_on_TX) && TRX.XIT_Enabled && TRX.FineRITTune) {
			TRX_XIT += direction * 10;
			if (TRX_XIT > TRX.XIT_INTERVAL) {
				TRX_XIT = TRX.XIT_INTERVAL;
			}
			if (TRX_XIT < -TRX.XIT_INTERVAL) {
				TRX_XIT = -TRX.XIT_INTERVAL;
			}

			TRX_setFrequency(CurrentVFO->Freq, CurrentVFO);
			LCD_UpdateQuery.FreqInfo = true;
		}
	}

	if (TRX.ENC2_func_mode == ENC_FUNC_SET_NOTCH) // Manual Notch
	{
		if (CurrentVFO->ManualNotchFilter) {
			float64_t step = (float64_t)TRX.NOTCH_STEP_Hz;

			if (CurrentVFO->NotchFC > step && direction < 0) {
				CurrentVFO->NotchFC -= step;
			} else if (direction > 0) {
				CurrentVFO->NotchFC += step;
			}

			CurrentVFO->NotchFC = round((float64_t)CurrentVFO->NotchFC / step) * step;

			if (CurrentVFO->NotchFC < step) {
				CurrentVFO->NotchFC = step;
			}

			LCD_UpdateQuery.StatusInfoGUI = true;
			NeedReinitNotch = true;
			NeedWTFRedraw = true;
		}
	}

	if (TRX.ENC2_func_mode == ENC_FUNC_SET_SQL) // SQL
	{
		CurrentVFO->FM_SQL_threshold_dBm += direction * 1;
		TRX.FM_SQL_threshold_dBm_shadow = CurrentVFO->FM_SQL_threshold_dBm;

		if (CurrentVFO->FM_SQL_threshold_dBm > 0) {
			CurrentVFO->FM_SQL_threshold_dBm = 0;
		}
		if (CurrentVFO->FM_SQL_threshold_dBm < -126) {
			CurrentVFO->FM_SQL_threshold_dBm = -126;
		}

		int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
		if (band >= 0) {
			TRX.BANDS_SAVED_SETTINGS[band].FM_SQL_threshold_dBm = CurrentVFO->FM_SQL_threshold_dBm;
		}

		LCD_UpdateQuery.StatusInfoBarRedraw = true;
	}

	if (TRX.ENC2_func_mode == ENC_FUNC_SET_MEM) { // mem channels select
		int8_t currentMemIndex = 0;
		for (uint8_t i = 0; i < MEMORY_CHANNELS_COUNT; i++) {
			if (CurrentVFO->Freq == CALIBRATE.MEMORY_CHANNELS[i].freq) {
				currentMemIndex = i;
			}
		}

		currentMemIndex += direction * 1;
		if (currentMemIndex < 0) {
			currentMemIndex = MEMORY_CHANNELS_COUNT - 1;
		}
		if (currentMemIndex >= MEMORY_CHANNELS_COUNT) {
			currentMemIndex = 0;
		}

		while (currentMemIndex < MEMORY_CHANNELS_COUNT && currentMemIndex > 0 && CALIBRATE.MEMORY_CHANNELS[currentMemIndex].freq == 0) {
			currentMemIndex += direction * 1;
		}

		if (currentMemIndex < 0) {
			currentMemIndex = 0;
		}
		if (currentMemIndex >= MEMORY_CHANNELS_COUNT) {
			currentMemIndex = 0;
		}

		BUTTONHANDLER_SelectMemoryChannels(currentMemIndex);
	}
}

static void FRONTPANEL_ENC2SW_click_handler(uint32_t parameter) {
	TRX_Inactive_Time = 0;
	// ENC2 CLICK
	if (!LCD_systemMenuOpened && !LCD_window.opened) {
		TRX.ENC2_func_mode++; // enc2 rotary mode

		if (TRX.ENC2_func_mode == ENC_FUNC_PAGER) { // pager disabled
			TRX.ENC2_func_mode++;
		}
		if (TRX.ENC2_func_mode == ENC_FUNC_SET_WPM && CurrentVFO->Mode != TRX_MODE_CW) { // no WPM if not CW
			TRX.ENC2_func_mode++;
		}
		if (TRX.ENC2_func_mode == ENC_FUNC_SET_RIT && ((!TRX.RIT_Enabled && !TRX.XIT_Enabled) || !TRX.FineRITTune)) { // nothing to RIT tune
			TRX.ENC2_func_mode++;
		}
		if (TRX.ENC2_func_mode == ENC_FUNC_SET_NOTCH && !CurrentVFO->ManualNotchFilter) { // nothing to NOTCH tune
			TRX.ENC2_func_mode++;
		}
		if (TRX.ENC2_func_mode == ENC_FUNC_SET_LPF) { // unsupported
			TRX.ENC2_func_mode++;
		}
		if (TRX.ENC2_func_mode == ENC_FUNC_SET_HPF) { // unsupported
			TRX.ENC2_func_mode++;
		}
		if (TRX.ENC2_func_mode == ENC_FUNC_SET_SQL && ((CurrentVFO->Mode != TRX_MODE_NFM && CurrentVFO->Mode != TRX_MODE_WFM) || !CurrentVFO->SQL)) { // nothing to SQL tune
			TRX.ENC2_func_mode++;
		}
		if (TRX.ENC2_func_mode == ENC_FUNC_SET_VOLUME) { // af adjust disabled
			TRX.ENC2_func_mode++;
		}
		if (TRX.ENC2_func_mode == ENC_FUNC_SET_IF) { // if adjust disabled
			TRX.ENC2_func_mode++;
		}
		if (TRX.ENC2_func_mode == ENC_FUNC_SET_MEM) { // mem channels adjust
			bool has_mem_channels = false;
			for (uint8_t i = 0; i < MEMORY_CHANNELS_COUNT; i++) {
				if (CALIBRATE.MEMORY_CHANNELS[i].freq > 0) {
					has_mem_channels = true;
				}
			}
			if (!has_mem_channels) {
				TRX.ENC2_func_mode++;
			}
		}

		if (TRX.ENC2_func_mode > ENC_FUNC_SET_MEM) {
			TRX.ENC2_func_mode = ENC_FUNC_FAST_STEP;
		}

		if (TRX.ENC2_func_mode == ENC_FUNC_FAST_STEP) {
			LCD_showTooltip("FAST STEP");
		}
		if (TRX.ENC2_func_mode == ENC_FUNC_SET_WPM) {
			LCD_showTooltip("SET WPM");
		}
		if (TRX.ENC2_func_mode == ENC_FUNC_SET_RIT) {
			LCD_showTooltip("SET RIT");
		}
		if (TRX.ENC2_func_mode == ENC_FUNC_SET_NOTCH) {
			LCD_showTooltip("SET NOTCH");
		}
		if (TRX.ENC2_func_mode == ENC_FUNC_SET_LPF) {
			LCD_showTooltip("SET LPF");
		}
		if (TRX.ENC2_func_mode == ENC_FUNC_SET_HPF) {
			LCD_showTooltip("SET HPF");
		}
		if (TRX.ENC2_func_mode == ENC_FUNC_SET_SQL) {
			LCD_showTooltip("SET SQL");
		}
		if (TRX.ENC2_func_mode == ENC_FUNC_SET_MEM) {
			LCD_showTooltip("MEM CHANNELS");
		}
	} else {
		if (LCD_systemMenuOpened) {
			// navigate in menu
			SYSMENU_eventSecEncoderClickSystemMenu();
		}
	}
}

static void FRONTPANEL_ENC2SW_hold_handler(uint32_t parameter) {
	TRX_Inactive_Time = 0;
	BUTTONHANDLER_MENU(0);
}

static void FRONTPANEL_ENCODER3_Rotated(int8_t direction) // rotated encoder, handler here, direction -1 - left, 1 - right
{
	TRX_Inactive_Time = 0;
	if (TRX.Locked || LCD_window.opened) {
		return;
	}

	if (direction > 0 || TRX.RF_Gain > 4) {
		TRX.RF_Gain += direction * 5;
	}
	if (direction < 0 && TRX.RF_Gain <= 5) {
		TRX.RF_Gain = 0;
	}
	if (TRX.RF_Gain > 100) {
		TRX.RF_Gain = 100;
	}

	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	if (band >= 0) {
		TRX.BANDS_SAVED_SETTINGS[band].RF_Gain = TRX.RF_Gain;
	}

	ATU_TunePowerStabilized = false;
}

static void FRONTPANEL_ENC3SW_click_handler(uint32_t parameter) {
	TRX_Inactive_Time = 0;
	if (TRX.Locked || LCD_window.opened) {
		return;
	}
}

static void FRONTPANEL_ENC3SW_hold_handler(uint32_t parameter) {
	TRX_Inactive_Time = 0;
	if (TRX.Locked || LCD_window.opened) {
		return;
	}
}

static void FRONTPANEL_ENCODER4_Rotated(int8_t direction) // rotated encoder, handler here, direction -1 - left, 1 - right
{
	TRX_Inactive_Time = 0;
	if (TRX.Locked || LCD_window.opened) {
		return;
	}

	// LPF
	if (PERIPH_FrontPanel_Buttons[0].state == false) { // enc4_sw button
		if (!TRX_on_TX) {
			if (CurrentVFO->Mode == TRX_MODE_CW) {
				SYSMENU_HANDL_FILTER_CW_LPF_pass(direction);
			}
			if (CurrentVFO->Mode == TRX_MODE_LSB || CurrentVFO->Mode == TRX_MODE_USB) {
				SYSMENU_HANDL_FILTER_SSB_LPF_RX_pass(direction);
			}
			if (CurrentVFO->Mode == TRX_MODE_AM || CurrentVFO->Mode == TRX_MODE_SAM_STEREO || CurrentVFO->Mode == TRX_MODE_SAM_LSB || CurrentVFO->Mode == TRX_MODE_SAM_USB) {
				SYSMENU_HANDL_FILTER_AM_LPF_RX_pass(direction);
			}
			if (CurrentVFO->Mode == TRX_MODE_NFM) {
				SYSMENU_HANDL_FILTER_FM_LPF_RX_pass(direction);
			}
			if (CurrentVFO->Mode == TRX_MODE_DIGI_L || CurrentVFO->Mode == TRX_MODE_DIGI_U) {
				SYSMENU_HANDL_FILTER_DIGI_LPF_pass(direction);
			}
		} else {
			if (CurrentVFO->Mode == TRX_MODE_CW) {
				SYSMENU_HANDL_FILTER_CW_LPF_pass(direction);
			}
			if (CurrentVFO->Mode == TRX_MODE_LSB || CurrentVFO->Mode == TRX_MODE_USB) {
				SYSMENU_HANDL_FILTER_SSB_LPF_TX_pass(direction);
			}
			if (CurrentVFO->Mode == TRX_MODE_AM || CurrentVFO->Mode == TRX_MODE_SAM_STEREO || CurrentVFO->Mode == TRX_MODE_SAM_LSB || CurrentVFO->Mode == TRX_MODE_SAM_USB) {
				SYSMENU_HANDL_FILTER_AM_LPF_TX_pass(direction);
			}
			if (CurrentVFO->Mode == TRX_MODE_NFM) {
				SYSMENU_HANDL_FILTER_FM_LPF_TX_pass(direction);
			}
			if (CurrentVFO->Mode == TRX_MODE_DIGI_L || CurrentVFO->Mode == TRX_MODE_DIGI_U) {
				SYSMENU_HANDL_FILTER_DIGI_LPF_pass(direction);
			}
		}
	}

	// HPF
	if (PERIPH_FrontPanel_Buttons[0].state == true) { // enc4_sw button
		if (!TRX_on_TX) {
			if (CurrentVFO->Mode == TRX_MODE_LSB || CurrentVFO->Mode == TRX_MODE_USB) {
				SYSMENU_HANDL_FILTER_SSB_HPF_RX_pass(direction);
			}
			if (CurrentVFO->Mode == TRX_MODE_NFM) {
				SYSMENU_HANDL_FILTER_FM_HPF_RX_pass(direction);
			}
			if (CurrentVFO->Mode == TRX_MODE_DIGI_L || CurrentVFO->Mode == TRX_MODE_DIGI_U) {
				SYSMENU_HANDL_FILTER_DIGI_HPF_pass(direction);
			}
		} else {
			if (CurrentVFO->Mode == TRX_MODE_LSB || CurrentVFO->Mode == TRX_MODE_USB) {
				SYSMENU_HANDL_FILTER_SSB_HPF_TX_pass(direction);
			}
			if (CurrentVFO->Mode == TRX_MODE_DIGI_L || CurrentVFO->Mode == TRX_MODE_DIGI_U) {
				SYSMENU_HANDL_FILTER_DIGI_HPF_pass(direction);
			}
		}
	}
}

static void FRONTPANEL_ENC4SW_click_handler(uint32_t parameter) {
	TRX_Inactive_Time = 0;
	if (TRX.Locked || LCD_window.opened) {
		return;
	}
}

static void FRONTPANEL_ENC4SW_hold_handler(uint32_t parameter) {
	TRX_Inactive_Time = 0;
	if (TRX.Locked || LCD_window.opened) {
		return;
	}
}

void FRONTPANEL_ENC2SW_validate() {
	if (TRX.ENC2_func_mode == ENC_FUNC_SET_WPM && CurrentVFO->Mode != TRX_MODE_CW) { // no WPM if not CW
		TRX.ENC2_func_mode = ENC_FUNC_FAST_STEP;
	}
	if (TRX.ENC2_func_mode == ENC_FUNC_SET_RIT && ((!TRX.RIT_Enabled && !TRX.XIT_Enabled) || !TRX.FineRITTune)) { // nothing to RIT tune
		TRX.ENC2_func_mode = ENC_FUNC_FAST_STEP;
	}
	if (TRX.ENC2_func_mode == ENC_FUNC_SET_NOTCH && !CurrentVFO->ManualNotchFilter) { // nothing to NOTCH tune
		TRX.ENC2_func_mode = ENC_FUNC_FAST_STEP;
	}
	if (TRX.ENC2_func_mode == ENC_FUNC_SET_LPF) { // unsupported
		TRX.ENC2_func_mode = ENC_FUNC_FAST_STEP;
	}
	if (TRX.ENC2_func_mode == ENC_FUNC_SET_HPF) { // unsupported
		TRX.ENC2_func_mode = ENC_FUNC_FAST_STEP;
	}
	if (TRX.ENC2_func_mode == ENC_FUNC_SET_SQL && ((CurrentVFO->Mode != TRX_MODE_NFM && CurrentVFO->Mode != TRX_MODE_WFM) || !CurrentVFO->SQL)) { // nothing to SQL tune
		TRX.ENC2_func_mode = ENC_FUNC_FAST_STEP;
	}
	if (TRX.ENC2_func_mode == ENC_FUNC_SET_MEM) { // mem channels adjust
		bool has_mem_channels = false;
		for (uint8_t i = 0; i < MEMORY_CHANNELS_COUNT; i++) {
			if (CALIBRATE.MEMORY_CHANNELS[i].freq > 0) {
				has_mem_channels = true;
			}
		}
		if (!has_mem_channels) {
			TRX.ENC2_func_mode = ENC_FUNC_FAST_STEP;
		}
	}
}

void FRONTPANEL_Init(void) {
#ifdef HRDW_MCP3008_1
	int32_t test_value = FRONTPANEL_ReadMCP3008_Value(0, 1, 1);
	if (test_value == 65535) {
		FRONTPanel_MCP3008_1_Enabled = false;
		println("[ERR] Frontpanel MCP3008 - 1 not found, disabling... (FPGA SPI/I2S CLOCK ERROR?)");
		LCD_showError("MCP3008 - 1 init error (FPGA I2S CLK?)", true);
	}
#endif
	FRONTPANEL_Process();
}

void FRONTPANEL_Process(void) {
	if (LCD_systemMenuOpened && !LCD_busy && FRONTPANEL_ProcessEncoder1 != 0) {
		TRX_ScanMode = false;
		SYSMENU_eventRotateSystemMenu(FRONTPANEL_ProcessEncoder1);
		FRONTPANEL_ProcessEncoder1 = 0;
	}

	if (FRONTPANEL_ProcessEncoder2 != 0) {
		TRX_ScanMode = false;
		FRONTPANEL_ENCODER2_Rotated(FRONTPANEL_ProcessEncoder2);
		FRONTPANEL_ProcessEncoder2 = 0;
	}

	if (FRONTPANEL_ProcessEncoder3 != 0) {
		TRX_ScanMode = false;
		FRONTPANEL_ENCODER3_Rotated(FRONTPANEL_ProcessEncoder3);
		FRONTPANEL_ProcessEncoder3 = 0;
	}

	if (FRONTPANEL_ProcessEncoder4 != 0) {
		TRX_ScanMode = false;
		FRONTPANEL_ENCODER4_Rotated(FRONTPANEL_ProcessEncoder4);
		FRONTPANEL_ProcessEncoder4 = 0;
	}

	if (SD_USBCardReader) {
		return;
	}

	static uint32_t fu_debug_lasttime = 0;
	uint16_t buttons_count = sizeof(PERIPH_FrontPanel_Buttons) / sizeof(PERIPH_FrontPanel_Button);
	int32_t mcp3008_value = 0;

	// process buttons
	for (uint16_t b = 0; b < buttons_count; b++) {
		if (HRDW_SPI_Locked) {
			continue;
		}

		PERIPH_FrontPanel_Button *button = &PERIPH_FrontPanel_Buttons[b];
// check disabled ports
#ifdef HRDW_MCP3008_1
		if (button->port == 1 && !FRONTPanel_MCP3008_1_Enabled) {
			continue;
		}
#endif

// get state from ADC MCP3008 (10bit - 1024values)
#ifdef HRDW_MCP3008_1
		if (button->port == 1) {
			mcp3008_value = FRONTPANEL_ReadMCP3008_Value(button->channel, 1, 5);
			if (mcp3008_value < 0) {
				// println("Skip");
				continue;
			}
		} else
#endif
			continue;

		if (TRX.Debug_Type == TRX_DEBUG_BUTTONS) {
			static uint8_t fu_gebug_lastchannel = 255;
			if ((HAL_GetTick() - fu_debug_lasttime > 500 && fu_gebug_lastchannel != button->channel) || fu_debug_lasttime == 0) {
				println("F_UNIT: port ", button->port, " channel ", button->channel, " value ", mcp3008_value);
				fu_gebug_lastchannel = button->channel;
			}
		}

		// TANGENT
		if (button->type == FUNIT_CTRL_TANGENT) {
			// Yaesu MH-36
			if (CALIBRATE.TangentType == TANGENT_MH36) {
				for (uint16_t tb = 0; tb < (sizeof(PERIPH_FrontPanel_TANGENT_MH36) / sizeof(PERIPH_FrontPanel_Button)); tb++) {
					if (button->channel == PERIPH_FrontPanel_TANGENT_MH36[tb].channel) {
						FRONTPANEL_CheckButton(&PERIPH_FrontPanel_TANGENT_MH36[tb], mcp3008_value);
					}
				}
			}

			// Yaesu MH-48
			if (CALIBRATE.TangentType == TANGENT_MH48) {
				for (uint16_t tb = 0; tb < (sizeof(PERIPH_FrontPanel_TANGENT_MH48) / sizeof(PERIPH_FrontPanel_Button)); tb++) {
					if (button->channel == PERIPH_FrontPanel_TANGENT_MH48[tb].channel) {
						FRONTPANEL_CheckButton(&PERIPH_FrontPanel_TANGENT_MH48[tb], mcp3008_value);
					}
				}
			}
		} else {
			FRONTPANEL_CheckButton(button, mcp3008_value); // other buttons / resistors
		}
	}

	if (TRX.Debug_Type == TRX_DEBUG_BUTTONS) {
		if (HAL_GetTick() - fu_debug_lasttime > 500) {
			println("");
			fu_debug_lasttime = HAL_GetTick();
		}
	}
}

void FRONTPANEL_CheckButton(PERIPH_FrontPanel_Button *button, uint16_t mcp3008_value) {
	// AF_GAIN
	if (button->type == FUNIT_CTRL_AF_GAIN) {
		static float32_t AF_VOLUME_mcp3008_averaged = 0.0f;
		AF_VOLUME_mcp3008_averaged = AF_VOLUME_mcp3008_averaged * 0.6f + mcp3008_value * 0.4f;

		TRX.Volume = (uint16_t)(MAX_VOLUME_VALUE - AF_VOLUME_mcp3008_averaged);
	}

	// IF Gain
	if (button->type == FUNIT_CTRL_IF_GAIN) {
		static float32_t IF_GAIN_mcp3008_averaged = 0.0f;
		IF_GAIN_mcp3008_averaged = IF_GAIN_mcp3008_averaged * 0.6f + mcp3008_value * 0.4f;
		TRX.IF_Gain = (uint8_t)(CALIBRATE.IF_GAIN_MIN + ((1023.0f - IF_GAIN_mcp3008_averaged) * (float32_t)(CALIBRATE.IF_GAIN_MAX - CALIBRATE.IF_GAIN_MIN) / 1023.0f));
	}

	// RIT / XIT
	if (button->type == FUNIT_CTRL_RIT) {
		static float32_t RIT_mcp3008_averaged = 0.0f;
		RIT_mcp3008_averaged = RIT_mcp3008_averaged * 0.8f + mcp3008_value * 0.2f;

		if ((!TRX.XIT_Enabled || !TRX_on_TX) && TRX.RIT_Enabled) {
			float32_t TRX_RIT_new = 512.0f - RIT_mcp3008_averaged;
			TRX_RIT_new = ((TRX_RIT_new < 0 ? -1.0f : 1.0f) * TRX_RIT_new * TRX_RIT_new) / 261632.0f * TRX.RIT_INTERVAL; // =(1023/2)^2
			TRX_RIT_new = roundf(roundf(TRX_RIT_new / 5.0f) * 5.0f);

			if (TRX_RIT != TRX_RIT_new) {
				TRX_RIT = TRX_RIT_new;
				TRX_setFrequency(CurrentVFO->Freq, CurrentVFO);
				uint16_t LCD_bw_trapez_stripe_pos_new = LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2;
				LCD_bw_trapez_stripe_pos_new += (int16_t)((float32_t)(LAYOUT->BW_TRAPEZ_WIDTH * 0.9f) / 2.0f * ((float32_t)TRX_RIT / (float32_t)TRX.RIT_INTERVAL));
				if (abs(LCD_bw_trapez_stripe_pos_new - LCD_bw_trapez_stripe_pos) > 0) {
					LCD_bw_trapez_stripe_pos = LCD_bw_trapez_stripe_pos_new;
					LCD_UpdateQuery.StatusInfoGUI = true;
				}
			}
		}

		if ((!TRX.RIT_Enabled || TRX_on_TX) && TRX.XIT_Enabled) {
			float32_t TRX_XIT_new = 512.0f - RIT_mcp3008_averaged;
			TRX_XIT_new = ((TRX_XIT_new < 0 ? -1.0f : 1.0f) * TRX_XIT_new * TRX_XIT_new) / 261632.0f * TRX.XIT_INTERVAL; // =(1023/2)^2
			TRX_XIT_new = roundf(roundf(TRX_XIT_new / 5.0f) * 5.0f);

			if (TRX_XIT != TRX_XIT_new) {
				TRX_XIT = TRX_XIT_new;
				TRX_setFrequency(CurrentVFO->Freq, CurrentVFO);
				uint16_t LCD_bw_trapez_stripe_pos_new = LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2;
				LCD_bw_trapez_stripe_pos_new += (int16_t)((float32_t)(LAYOUT->BW_TRAPEZ_WIDTH * 0.9f) / 2.0f * ((float32_t)TRX_XIT / (float32_t)TRX.XIT_INTERVAL));
				if (abs(LCD_bw_trapez_stripe_pos_new - LCD_bw_trapez_stripe_pos) > 0) {
					LCD_bw_trapez_stripe_pos = LCD_bw_trapez_stripe_pos_new;
					LCD_UpdateQuery.StatusInfoGUI = true;
				}
			}
		}
	}

	// PTT
	if (button->type == FUNIT_CTRL_PTT) {
		static bool frontunit_ptt_state_prev = false;
		bool frontunit_ptt_state_now = false;
		if (mcp3008_value > button->tres_min && mcp3008_value < button->tres_max) {
			frontunit_ptt_state_now = true;
		}
		if (frontunit_ptt_state_prev != frontunit_ptt_state_now) {
			TRX_ptt_soft = frontunit_ptt_state_now;
			TRX_ptt_change();
			frontunit_ptt_state_prev = frontunit_ptt_state_now;
		}
	}

	// BUTTONS
	if (button->type == FUNIT_CTRL_BUTTON) {
		// set state
		if (mcp3008_value >= button->tres_min && mcp3008_value < button->tres_max) {
			TRX_Inactive_Time = 0;
			button->state = true;
			if (TRX.Debug_Type == TRX_DEBUG_BUTTONS) {
				println("Button pressed: port ", button->port, " channel ", button->channel, " value: ", mcp3008_value);
			}
		} else {
			button->state = false;
		}

		// check state
		if ((button->prev_state != button->state) && button->state) {
			button->start_hold_time = HAL_GetTick();
			button->afterhold = false;
		}

		// check hold state
		if ((button->prev_state == button->state) && button->state && ((HAL_GetTick() - button->start_hold_time) > KEY_HOLD_TIME) && !button->afterhold) {
			button->afterhold = true;
			if (!LCD_systemMenuOpened || button->work_in_menu) {
				if (!LCD_window.opened) {
					if (button->holdHandler != NULL) {
						CODEC_Beep();
						button->holdHandler(button->parameter);
					}
				} else {
					LCD_closeWindow();
				}
			}
		}

		// check click state
		if ((button->prev_state != button->state) && !button->state && ((HAL_GetTick() - button->start_hold_time) < KEY_HOLD_TIME) && !button->afterhold && !TRX.Locked) {
			if (!LCD_systemMenuOpened || button->work_in_menu) {
				if (!LCD_window.opened) {
					if (button->clickHandler != NULL) {
						CODEC_Beep();
						button->clickHandler(button->parameter);
					}
				} else {
					LCD_closeWindow();
				}
			}
		}

		// save prev state
		button->prev_state = button->state;
	}

	// DEBUG BUTTONS
	if (button->type == FUNIT_CTRL_BUTTON_DEBUG) {
		char str[64] = {0};
		sprintf(str, "%d: %d       ", button->channel, mcp3008_value);
		LCDDriver_printText(str, 10, 200 + button->channel * 20, COLOR_RED, BG_COLOR, 2);
	}
}

static int32_t FRONTPANEL_ReadMCP3008_Value(uint8_t channel, uint8_t adc_num, uint8_t count) {
	HRDW_SPI_Locked = true;

	uint8_t outData[3] = {0};
	uint8_t inData[3] = {0};
	uint32_t mcp3008_value = 0;

	const uint8_t max_signal_diff = 10;
	uint32_t value_first = 0;

	for (uint8_t i = 0; i < count; i++) {
		outData[0] = 0x18 | channel;
		bool res = false;
		if (adc_num == 1) {
			res = HRDW_FrontUnit_SPI(outData, inData, 3, false);
		}
		if (adc_num == 2) {
			res = HRDW_FrontUnit2_SPI(outData, inData, 3, false);
		}
		if (adc_num == 3) {
			res = HRDW_FrontUnit3_SPI(outData, inData, 3, false);
		}
		if (res == false) {
			HRDW_SPI_Locked = false;
			return 65535;
		}
		uint32_t value_now = (uint16_t)(((inData[1] & 0x3F) << 4) | (inData[2] & 0xF0 >> 4));
		mcp3008_value += value_now;

		if (i > 0 && abs((int32_t)value_now - (int32_t)value_first) > max_signal_diff) { // value not stabilized, reload
			// println(abs((int32_t)value_now - (int32_t)value_first));
			HRDW_SPI_Locked = false;
			return -1;
		}
		if (i == 0) {
			value_first = value_now;
		}
	}
	mcp3008_value /= count;

	HRDW_SPI_Locked = false;

	return mcp3008_value;
}
