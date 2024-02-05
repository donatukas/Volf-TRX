#include "bands.h"
#include "functions.h"
#include "settings.h"
#include <stdlib.h>

SRAM4 bool BAND_SELECTABLE[BANDS_COUNT] = {
    false, // 2200m
    false, // LWBR
    false, // 630m
    false, // MWBR
    true,  // 160m
    false, // BR 2.4
    false, // BR 3.3
    true,  // 80m
    false, // BR 4.0
    false, // BR 4.8
    false, // 60m
    false, // BR 6.0
    true,  // 40m
    false, // BR 7.3
    false, // BR 9.6
    true,  // 30m
    false, // BR 11.9
    false, // BR 13.7
    true,  // 20m
    false, // BR 15.5
    false, // BR 17.7
    true,  // 17m
    false, // BR 19.0
    true,  // 15m
    false, // BR 21.6
    true,  // 12m
    false, // BR 25.8
    true,  // CB
    true,  // 10m
    true,  // 6m
    false, // 4m
    true,  // FM
    false, // AIR
    true,  // 2m
    false, // Marine
    false, // 70cm
    false, // 23cm
    false, // 13cm
    false, // 6cm
    false, // 3cm
    false, // QO-100
    false, // 1.2cm
};

const BAND_MAP BANDS[BANDS_COUNT] = {
    // 2200 METERS
    {
        .name = "2200m",
        .broadcast = false,
        .startFreq = 135700,
        .endFreq = 137800,
        .defaultFreq = 135750,
        .regions =
            (const REGION_MAP[3]){
                {.startFreq = 135700, .endFreq = 137400, .mode = TRX_MODE_CW},
                {.startFreq = 137400, .endFreq = 137600, .mode = TRX_MODE_DIGI_U},
                {.startFreq = 137600, .endFreq = 137800, .mode = TRX_MODE_CW},
            },
        .regionsCount = 3,
        .channels = NULL,
        .channelsCount = 0,
    },
    // Long Wave Broadcast
    {
        .name = "LW",
        .broadcast = true,
        .startFreq = 148500,
        .endFreq = 283500,
        .defaultFreq = 200000,
        .regions =
            (const REGION_MAP[1]){
                {.startFreq = 148500, .endFreq = 283500, .mode = TRX_MODE_SAM_STEREO},
            },
        .regionsCount = 1,
        .channels = NULL,
        .channelsCount = 0,
    },
    // 630 METERS
    {
        .name = "630m",
        .broadcast = false,
        .startFreq = 472000,
        .endFreq = 479000,
        .defaultFreq = 475500,
        .regions =
            (const REGION_MAP[1]){
                {.startFreq = 472000, .endFreq = 479000, .mode = TRX_MODE_CW},
            },
        .regionsCount = 1,
        .channels = NULL,
        .channelsCount = 0,
    },
    // Medium Wave Broadcast
    {
        .name = "MW",
        .broadcast = true,
        .startFreq = 526500,
        .endFreq = 1606500,
        .defaultFreq = 1500000,
        .regions =
            (const REGION_MAP[1]){
                {.startFreq = 526500, .endFreq = 1606500, .mode = TRX_MODE_SAM_STEREO},
            },
        .regionsCount = 1,
        .channels = NULL,
        .channelsCount = 0,
    },
    // 160 METERS
    {
        .name = "160m",
        .broadcast = false,
        .startFreq = 1810000,
        .endFreq = 2000000,
        .defaultFreq = 1900000,
        .regions =
            (const REGION_MAP[3]){
                {.startFreq = 1810000, .endFreq = 1838000, .mode = TRX_MODE_CW},
                {.startFreq = 1838000, .endFreq = 1843000, .mode = TRX_MODE_DIGI_U},
                {.startFreq = 1843000, .endFreq = 2000000, .mode = TRX_MODE_LSB},
            },
        .regionsCount = 3,
        .channels = NULL,
        .channelsCount = 0,
    },
    // Shortwave Wave Broadcast
    {
        .name = "120m",
        .broadcast = true,
        .startFreq = 2300000,
        .endFreq = 2500000,
        .defaultFreq = 2400000,
        .regions =
            (const REGION_MAP[1]){
                {.startFreq = 2300000, .endFreq = 2500000, .mode = TRX_MODE_SAM_STEREO},
            },
        .regionsCount = 1,
        .channels = NULL,
        .channelsCount = 0,
    },
    // Shortwave Wave Broadcast
    {
        .name = "90m",
        .broadcast = true,
        .startFreq = 3200000,
        .endFreq = 3400000,
        .defaultFreq = 3300000,
        .regions =
            (const REGION_MAP[1]){
                {.startFreq = 3200000, .endFreq = 3400000, .mode = TRX_MODE_SAM_STEREO},
            },
        .regionsCount = 1,
        .channels = NULL,
        .channelsCount = 0,
    },
    // 80 METERS
    {
        .name = "80m",
        .broadcast = false,
        .startFreq = 3500000,
        .endFreq = 3800000,
        .defaultFreq = 3650000,
        .regions =
            (const REGION_MAP[3]){
                {.startFreq = 3500000, .endFreq = 3570000, .mode = TRX_MODE_CW},
                {.startFreq = 3570000, .endFreq = 3600000, .mode = TRX_MODE_DIGI_U},
                {.startFreq = 3600000, .endFreq = 3800000, .mode = TRX_MODE_LSB},
            },
        .regionsCount = 3,
        .channels = NULL,
        .channelsCount = 0,
    },
    // Shortwave Wave Broadcast
    {
        .name = "75m",
        .broadcast = true,
        .startFreq = 3900000,
        .endFreq = 4000000,
        .defaultFreq = 3950000,
        .regions =
            (const REGION_MAP[1]){
                {.startFreq = 3900000, .endFreq = 4000000, .mode = TRX_MODE_SAM_STEREO},
            },
        .regionsCount = 1,
        .channels = NULL,
        .channelsCount = 0,
    },
    // Shortwave Wave Broadcast
    {
        .name = "60m ", // BR
        .broadcast = true,
        .startFreq = 4700000,
        .endFreq = 5100000,
        .defaultFreq = 5000000,
        .regions =
            (const REGION_MAP[1]){
                {.startFreq = 4700000, .endFreq = 5100000, .mode = TRX_MODE_SAM_STEREO},
            },
        .regionsCount = 1,
        .channels = NULL,
        .channelsCount = 0,
    },
    // 60 METERS
    {
        .name = "60m",
        .broadcast = false,
        .startFreq = 5258500,
        .endFreq = 5406500,
        .defaultFreq = 5350000,
        .regions =
            (const REGION_MAP[4]){
                {.startFreq = 5258500, .endFreq = 5276000, .mode = TRX_MODE_CW},
                {.startFreq = 5276000, .endFreq = 5354000, .mode = TRX_MODE_USB},
                {.startFreq = 5354000, .endFreq = 5374500, .mode = TRX_MODE_DIGI_U},
                {.startFreq = 5374500, .endFreq = 5406500, .mode = TRX_MODE_USB},
            },
        .regionsCount = 4,
        .channels = NULL,
        .channelsCount = 0,
    },
    // Shortwave Wave Broadcast
    {
        .name = "49m",
        .broadcast = true,
        .startFreq = 5800000,
        .endFreq = 6300000,
        .defaultFreq = 6000000,
        .regions =
            (const REGION_MAP[1]){
                {.startFreq = 5800000, .endFreq = 6300000, .mode = TRX_MODE_SAM_STEREO},
            },
        .regionsCount = 1,
        .channels = NULL,
        .channelsCount = 0,
    },
    // 40 METERS
    {
        .name = "40m",
        .broadcast = false,
        .startFreq = 7000000,
        .endFreq = 7200000,
        .defaultFreq = 7100000,
        .regions =
            (const REGION_MAP[5]){
                {.startFreq = 7000000, .endFreq = 7040000, .mode = TRX_MODE_CW},
                {.startFreq = 7040000, .endFreq = 7060000, .mode = TRX_MODE_DIGI_U},
                {.startFreq = 7060000, .endFreq = 7074000, .mode = TRX_MODE_LSB},
                {.startFreq = 7074000, .endFreq = 7080000, .mode = TRX_MODE_DIGI_U},
                {.startFreq = 7080000, .endFreq = 7200000, .mode = TRX_MODE_LSB},
            },
        .regionsCount = 5,
        .channels = NULL,
        .channelsCount = 0,
    },
    // Shortwave Wave Broadcast
    {
        .name = "41m",
        .broadcast = true,
        .startFreq = 7200000,
        .endFreq = 7600000,
        .defaultFreq = 7300000,
        .regions =
            (const REGION_MAP[1]){
                {.startFreq = 7200000, .endFreq = 7600000, .mode = TRX_MODE_SAM_STEREO},
            },
        .regionsCount = 1,
        .channels = NULL,
        .channelsCount = 0,
    },
    // Shortwave Wave Broadcast
    {
        .name = "31m",
        .broadcast = true,
        .startFreq = 9300000,
        .endFreq = 9995000,
        .defaultFreq = 9600000,
        .regions =
            (const REGION_MAP[1]){
                {.startFreq = 9300000, .endFreq = 9995000, .mode = TRX_MODE_SAM_STEREO},
            },
        .regionsCount = 1,
        .channels = NULL,
        .channelsCount = 0,
    },
    // 30 METERS
    {
        .name = "30m",
        .broadcast = false,
        .startFreq = 10100000,
        .endFreq = 10150000,
        .defaultFreq = 10140000,
        .regions =
            (const REGION_MAP[2]){
                {.startFreq = 10100000, .endFreq = 10130000, .mode = TRX_MODE_CW},
                {.startFreq = 10130000, .endFreq = 10150000, .mode = TRX_MODE_DIGI_U},
            },
        .regionsCount = 2,
        .channels = NULL,
        .channelsCount = 0,
    },
    // Shortwave Wave Broadcast
    {
        .name = "25m",
        .broadcast = true,
        .startFreq = 11500000,
        .endFreq = 12200000,
        .defaultFreq = 11900000,
        .regions =
            (const REGION_MAP[1]){
                {.startFreq = 11500000, .endFreq = 12200000, .mode = TRX_MODE_SAM_STEREO},
            },
        .regionsCount = 1,
        .channels = NULL,
        .channelsCount = 0,
    },
    // Shortwave Wave Broadcast
    {
        .name = "22m",
        .broadcast = true,
        .startFreq = 13500000,
        .endFreq = 13900000,
        .defaultFreq = 13700000,
        .regions =
            (const REGION_MAP[1]){
                {.startFreq = 13500000, .endFreq = 13900000, .mode = TRX_MODE_SAM_STEREO},
            },
        .regionsCount = 1,
        .channels = NULL,
        .channelsCount = 0,
    },
    // 20 METERS
    {
        .name = "20m",
        .broadcast = false,
        .startFreq = 14000000,
        .endFreq = 14350000,
        .defaultFreq = 14150000,
        .regions =
            (const REGION_MAP[3]){
                {.startFreq = 14000000, .endFreq = 14070000, .mode = TRX_MODE_CW},
                {.startFreq = 14070000, .endFreq = 14112000, .mode = TRX_MODE_DIGI_U},
                {.startFreq = 14112000, .endFreq = 14350000, .mode = TRX_MODE_USB},
            },
        .regionsCount = 3,
        .channels = NULL,
        .channelsCount = 0,
    },
    // Shortwave Wave Broadcast
    {
        .name = "19m",
        .broadcast = true,
        .startFreq = 15000000,
        .endFreq = 15800000,
        .defaultFreq = 15600000,
        .regions =
            (const REGION_MAP[1]){
                {.startFreq = 15000000, .endFreq = 15900000, .mode = TRX_MODE_SAM_STEREO},
            },
        .regionsCount = 1,
        .channels = NULL,
        .channelsCount = 0,
    },
    // Shortwave Wave Broadcast
    {
        .name = "16m",
        .broadcast = true,
        .startFreq = 17400000,
        .endFreq = 17900000,
        .defaultFreq = 17600000,
        .regions =
            (const REGION_MAP[1]){
                {.startFreq = 17400000, .endFreq = 17995000, .mode = TRX_MODE_SAM_STEREO},
            },
        .regionsCount = 1,
        .channels = NULL,
        .channelsCount = 0,
    },
    // 17 METERS
    {
        .name = "17m",
        .broadcast = false,
        .startFreq = 18068000,
        .endFreq = 18168000,
        .defaultFreq = 18120000,
        .regions =
            (const REGION_MAP[3]){
                {.startFreq = 18068000, .endFreq = 18095000, .mode = TRX_MODE_CW},
                {.startFreq = 18095000, .endFreq = 18120000, .mode = TRX_MODE_DIGI_U},
                {.startFreq = 18120000, .endFreq = 18168000, .mode = TRX_MODE_USB},
            },
        .regionsCount = 3,
        .channels = NULL,
        .channelsCount = 0,
    },
    // Shortwave Wave Broadcast
    {
        .name = "15m ", // BR
        .broadcast = true,
        .startFreq = 18800000,
        .endFreq = 19100000,
        .defaultFreq = 19000000,
        .regions =
            (const REGION_MAP[1]){
                {.startFreq = 18800000, .endFreq = 19100000, .mode = TRX_MODE_SAM_STEREO},
            },
        .regionsCount = 1,
        .channels = NULL,
        .channelsCount = 0,
    },
    // 15 METERS
    {
        .name = "15m",
        .broadcast = false,
        .startFreq = 21000000,
        .endFreq = 21450000,
        .defaultFreq = 21200000,
        .regions =
            (const REGION_MAP[4]){
                {.startFreq = 21000000, .endFreq = 21070000, .mode = TRX_MODE_CW},
                {.startFreq = 21070000, .endFreq = 21149000, .mode = TRX_MODE_DIGI_U},
                {.startFreq = 21149000, .endFreq = 21450000, .mode = TRX_MODE_USB},
            },
        .regionsCount = 4,
        .channels = NULL,
        .channelsCount = 0,
    },
    // Shortwave Wave Broadcast
    {
        .name = "13m",
        .broadcast = true,
        .startFreq = 21450000,
        .endFreq = 21995000,
        .defaultFreq = 21600000,
        .regions =
            (const REGION_MAP[1]){
                {.startFreq = 21450000, .endFreq = 21995000, .mode = TRX_MODE_SAM_STEREO},
            },
        .regionsCount = 1,
        .channels = NULL,
        .channelsCount = 0,
    },
    // 12 METERS
    {
        .name = "12m",
        .broadcast = false,
        .startFreq = 24890000,
        .endFreq = 24990000,
        .defaultFreq = 24940000,
        .regions =
            (const REGION_MAP[4]){
                {.startFreq = 24890000, .endFreq = 24915000, .mode = TRX_MODE_CW},
                {.startFreq = 24915000, .endFreq = 24940000, .mode = TRX_MODE_DIGI_U},
                {.startFreq = 24940000, .endFreq = 24990000, .mode = TRX_MODE_USB},
            },
        .regionsCount = 4,
        .channels = NULL,
        .channelsCount = 0,
    },
    // Shortwave Wave Broadcast
    {
        .name = "11m",
        .broadcast = true,
        .startFreq = 25500000,
        .endFreq = 26200000,
        .defaultFreq = 25800000,
        .regions =
            (const REGION_MAP[1]){
                {.startFreq = 25500000, .endFreq = 26200000, .mode = TRX_MODE_SAM_STEREO},
            },
        .regionsCount = 1,
        .channels = NULL,
        .channelsCount = 0,
    },
    // CB
    {
        .name = "CB",
        .broadcast = false,
        .startFreq = 26900000,
        .endFreq = 27995000,
        .defaultFreq = 27135000,
        .regions =
            (const REGION_MAP[4]){
                {.startFreq = 26900000, .endFreq = 27130000, .mode = TRX_MODE_NFM},
                {.startFreq = 27130000, .endFreq = 27140000, .mode = TRX_MODE_AM},
                {.startFreq = 27140000, .endFreq = 27415000, .mode = TRX_MODE_NFM},
                {.startFreq = 27420000, .endFreq = 27995000, .mode = TRX_MODE_USB},
            },
        .regionsCount = 4,
        .channels =
            (const CHANNEL_MAP[45 * 2 + 14]){
                {.subname = "C ", .number = 1, .rxFreq = 26965000, .txFreq = 26965000},  {.subname = "C ", .number = 2, .rxFreq = 26975000, .txFreq = 26975000},
                {.subname = "C ", .number = 3, .rxFreq = 26985000, .txFreq = 26985000},  {.subname = "C ", .number = 4, .rxFreq = 27005000, .txFreq = 27005000},
                {.subname = "C ", .number = 5, .rxFreq = 27015000, .txFreq = 27015000},  {.subname = "C ", .number = 6, .rxFreq = 27025000, .txFreq = 27025000},
                {.subname = "C ", .number = 7, .rxFreq = 27035000, .txFreq = 27035000},  {.subname = "C ", .number = 8, .rxFreq = 27055000, .txFreq = 27055000},
                {.subname = "C ", .number = 9, .rxFreq = 27065000, .txFreq = 27065000},  {.subname = "C ", .number = 10, .rxFreq = 27075000, .txFreq = 27075000},
                {.subname = "C ", .number = 11, .rxFreq = 27085000, .txFreq = 27085000}, {.subname = "C ", .number = 12, .rxFreq = 27105000, .txFreq = 27105000},
                {.subname = "C ", .number = 13, .rxFreq = 27115000, .txFreq = 27115000}, {.subname = "C ", .number = 14, .rxFreq = 27125000, .txFreq = 27125000},
                {.subname = "C ", .number = 15, .rxFreq = 27135000, .txFreq = 27135000}, {.subname = "C ", .number = 16, .rxFreq = 27155000, .txFreq = 27155000},
                {.subname = "C ", .number = 17, .rxFreq = 27165000, .txFreq = 27165000}, {.subname = "C ", .number = 18, .rxFreq = 27175000, .txFreq = 27175000},
                {.subname = "C ", .number = 19, .rxFreq = 27185000, .txFreq = 27185000}, {.subname = "C ", .number = 20, .rxFreq = 27205000, .txFreq = 27205000},
                {.subname = "C ", .number = 21, .rxFreq = 27215000, .txFreq = 27215000}, {.subname = "C ", .number = 22, .rxFreq = 27225000, .txFreq = 27225000},
                {.subname = "C ", .number = 23, .rxFreq = 27255000, .txFreq = 27255000}, {.subname = "C ", .number = 24, .rxFreq = 27235000, .txFreq = 27235000},
                {.subname = "C ", .number = 25, .rxFreq = 27245000, .txFreq = 27245000}, {.subname = "C ", .number = 26, .rxFreq = 27265000, .txFreq = 27265000},
                {.subname = "C ", .number = 27, .rxFreq = 27275000, .txFreq = 27275000}, {.subname = "C ", .number = 28, .rxFreq = 27285000, .txFreq = 27285000},
                {.subname = "C ", .number = 29, .rxFreq = 27295000, .txFreq = 27295000}, {.subname = "C ", .number = 30, .rxFreq = 27305000, .txFreq = 27305000},
                {.subname = "C ", .number = 31, .rxFreq = 27315000, .txFreq = 27315000}, {.subname = "C ", .number = 32, .rxFreq = 27325000, .txFreq = 27325000},
                {.subname = "C ", .number = 33, .rxFreq = 27335000, .txFreq = 27335000}, {.subname = "C ", .number = 34, .rxFreq = 27345000, .txFreq = 27345000},
                {.subname = "C ", .number = 35, .rxFreq = 27355000, .txFreq = 27355000}, {.subname = "C ", .number = 36, .rxFreq = 27365000, .txFreq = 27365000},
                {.subname = "C ", .number = 37, .rxFreq = 27375000, .txFreq = 27375000}, {.subname = "C ", .number = 38, .rxFreq = 27385000, .txFreq = 27385000},
                {.subname = "C ", .number = 39, .rxFreq = 27395000, .txFreq = 27395000}, {.subname = "C ", .number = 40, .rxFreq = 27405000, .txFreq = 27405000},
                {.subname = "C ", .number = 41, .rxFreq = 26995000, .txFreq = 26995000}, {.subname = "C ", .number = 42, .rxFreq = 27045000, .txFreq = 27045000},
                {.subname = "C ", .number = 43, .rxFreq = 27095000, .txFreq = 27095000}, {.subname = "C ", .number = 44, .rxFreq = 27145000, .txFreq = 27145000},
                {.subname = "C ", .number = 45, .rxFreq = 27195000, .txFreq = 27195000},

                {.subname = "D ", .number = 1, .rxFreq = 27415000, .txFreq = 27415000},  {.subname = "D ", .number = 2, .rxFreq = 27425000, .txFreq = 27425000},
                {.subname = "D ", .number = 3, .rxFreq = 27435000, .txFreq = 27435000},  {.subname = "D ", .number = 4, .rxFreq = 27455000, .txFreq = 27455000},
                {.subname = "D ", .number = 5, .rxFreq = 27465000, .txFreq = 27465000},  {.subname = "D ", .number = 6, .rxFreq = 27475000, .txFreq = 27475000},
                {.subname = "D ", .number = 7, .rxFreq = 27485000, .txFreq = 27485000},  {.subname = "D ", .number = 8, .rxFreq = 27505000, .txFreq = 27505000},
                {.subname = "D ", .number = 9, .rxFreq = 27515000, .txFreq = 27515000},  {.subname = "D ", .number = 10, .rxFreq = 27525000, .txFreq = 27525000},
                {.subname = "D ", .number = 11, .rxFreq = 27535000, .txFreq = 27535000}, {.subname = "D ", .number = 12, .rxFreq = 27555000, .txFreq = 27555000},
                {.subname = "D ", .number = 13, .rxFreq = 27565000, .txFreq = 27565000}, {.subname = "D ", .number = 14, .rxFreq = 27575000, .txFreq = 27575000},
                {.subname = "D ", .number = 15, .rxFreq = 27585000, .txFreq = 27585000}, {.subname = "D ", .number = 16, .rxFreq = 27605000, .txFreq = 27605000},
                {.subname = "D ", .number = 17, .rxFreq = 27615000, .txFreq = 27615000}, {.subname = "D ", .number = 18, .rxFreq = 27625000, .txFreq = 27625000},
                {.subname = "D ", .number = 19, .rxFreq = 27635000, .txFreq = 27635000}, {.subname = "D ", .number = 20, .rxFreq = 27655000, .txFreq = 27655000},
                {.subname = "D ", .number = 21, .rxFreq = 27665000, .txFreq = 27665000}, {.subname = "D ", .number = 22, .rxFreq = 27675000, .txFreq = 27675000},
                {.subname = "D ", .number = 23, .rxFreq = 27705000, .txFreq = 27705000}, {.subname = "D ", .number = 24, .rxFreq = 27685000, .txFreq = 27685000},
                {.subname = "D ", .number = 25, .rxFreq = 27695000, .txFreq = 27695000}, {.subname = "D ", .number = 26, .rxFreq = 27715000, .txFreq = 27715000},
                {.subname = "D ", .number = 27, .rxFreq = 27725000, .txFreq = 27725000}, {.subname = "D ", .number = 28, .rxFreq = 27735000, .txFreq = 27735000},
                {.subname = "D ", .number = 29, .rxFreq = 27745000, .txFreq = 27745000}, {.subname = "D ", .number = 30, .rxFreq = 27755000, .txFreq = 27755000},
                {.subname = "D ", .number = 31, .rxFreq = 27765000, .txFreq = 27765000}, {.subname = "D ", .number = 32, .rxFreq = 27775000, .txFreq = 27775000},
                {.subname = "D ", .number = 33, .rxFreq = 27785000, .txFreq = 27785000}, {.subname = "D ", .number = 34, .rxFreq = 27795000, .txFreq = 27795000},
                {.subname = "D ", .number = 35, .rxFreq = 27805000, .txFreq = 27805000}, {.subname = "D ", .number = 36, .rxFreq = 27815000, .txFreq = 27815000},
                {.subname = "D ", .number = 37, .rxFreq = 27825000, .txFreq = 27825000}, {.subname = "D ", .number = 38, .rxFreq = 27835000, .txFreq = 27835000},
                {.subname = "D ", .number = 39, .rxFreq = 27845000, .txFreq = 27845000}, {.subname = "D ", .number = 40, .rxFreq = 27855000, .txFreq = 27855000},
                {.subname = "D ", .number = 41, .rxFreq = 27445000, .txFreq = 27445000}, {.subname = "D ", .number = 42, .rxFreq = 27495000, .txFreq = 27495000},
                {.subname = "D ", .number = 43, .rxFreq = 27545000, .txFreq = 27545000}, {.subname = "D ", .number = 44, .rxFreq = 27595000, .txFreq = 27595000},
                {.subname = "D ", .number = 45, .rxFreq = 27645000, .txFreq = 27645000},

                {.subname = "E ", .number = 1, .rxFreq = 27865000, .txFreq = 27865000},  {.subname = "E ", .number = 2, .rxFreq = 27875000, .txFreq = 27875000},
                {.subname = "E ", .number = 3, .rxFreq = 27885000, .txFreq = 27885000},  {.subname = "E ", .number = 4, .rxFreq = 27905000, .txFreq = 27905000},
                {.subname = "E ", .number = 5, .rxFreq = 27915000, .txFreq = 27915000},  {.subname = "E ", .number = 6, .rxFreq = 27925000, .txFreq = 27925000},
                {.subname = "E ", .number = 7, .rxFreq = 27935000, .txFreq = 27935000},  {.subname = "E ", .number = 8, .rxFreq = 27955000, .txFreq = 27955000},
                {.subname = "E ", .number = 9, .rxFreq = 27965000, .txFreq = 27965000},  {.subname = "E ", .number = 10, .rxFreq = 27975000, .txFreq = 27975000},
                {.subname = "E ", .number = 11, .rxFreq = 27985000, .txFreq = 27985000}, {.subname = "E ", .number = 41, .rxFreq = 27895000, .txFreq = 27895000},
                {.subname = "E ", .number = 42, .rxFreq = 27945000, .txFreq = 27945000}, {.subname = "E ", .number = 43, .rxFreq = 27995000, .txFreq = 27995000},
            },
        .channelsCount = 45 * 2 + 14,
    },
    // 10 METERS
    {
        .name = "10m",
        .broadcast = false,
        .startFreq = 28000000,
        .endFreq = 29700000,
        .defaultFreq = 28350000,
        .regions =
            (const REGION_MAP[9]){
                {.startFreq = 28000000, .endFreq = 28070000, .mode = TRX_MODE_CW},
                {.startFreq = 28070000, .endFreq = 28190000, .mode = TRX_MODE_DIGI_U},
                {.startFreq = 28190000, .endFreq = 28300000, .mode = TRX_MODE_CW},
                {.startFreq = 28300000, .endFreq = 28320000, .mode = TRX_MODE_DIGI_U},
                {.startFreq = 28320000, .endFreq = 29100000, .mode = TRX_MODE_USB},
                {.startFreq = 29100000, .endFreq = 29200000, .mode = TRX_MODE_NFM},
                {.startFreq = 29200000, .endFreq = 29300000, .mode = TRX_MODE_DIGI_U},
                {.startFreq = 29300000, .endFreq = 29520000, .mode = TRX_MODE_USB},
                {.startFreq = 29520000, .endFreq = 29700000, .mode = TRX_MODE_NFM},
            },
        .regionsCount = 9,
        .channels = NULL,
        .channelsCount = 0,
    },
    // 6 METERS
    {
        .name = "6m",
        .broadcast = false,
        .startFreq = 50000000,
        .endFreq = 54000000,
        .defaultFreq = 51000000,
        .regions =
            (const REGION_MAP[7]){
                {.startFreq = 50000000, .endFreq = 50100000, .mode = TRX_MODE_CW},
                {.startFreq = 50100000, .endFreq = 50300000, .mode = TRX_MODE_USB},
                {.startFreq = 50300000, .endFreq = 50350000, .mode = TRX_MODE_DIGI_U},
                {.startFreq = 50350000, .endFreq = 50600000, .mode = TRX_MODE_USB},
                {.startFreq = 50600000, .endFreq = 51000000, .mode = TRX_MODE_DIGI_U},
                {.startFreq = 51000000, .endFreq = 51100000, .mode = TRX_MODE_USB},
                {.startFreq = 51100000, .endFreq = 54000000, .mode = TRX_MODE_NFM},
            },
        .regionsCount = 7,
        .channels = NULL,
        .channelsCount = 0,
    },
    // 4 METERS
    {
        .name = "4m",
        .broadcast = false,
        .startFreq = 69900000,
        .endFreq = 70500000,
        .defaultFreq = 70200000,
        .regions =
            (const REGION_MAP[1]){
                {.startFreq = 69900000, .endFreq = 70500000, .mode = TRX_MODE_USB},
            },
        .regionsCount = 1,
        .channels = NULL,
        .channelsCount = 0,
    },
    // FM RADIO
    {
        .name = "FM",
        .broadcast = true,
        .startFreq = 70500000,
        .endFreq = 108000000,
        .defaultFreq = 100000000,
        .regions =
            (const REGION_MAP[1]){
                {.startFreq = 70500000, .endFreq = 108000000, .mode = TRX_MODE_WFM},
            },
        .regionsCount = 1,
        .channels = NULL,
        .channelsCount = 0,
    },
    // AIR BAND
    {
        .name = "AIR",
        .broadcast = false,
        .startFreq = 108000000,
        .endFreq = 137000000,
        .defaultFreq = 135000000,
        .regions =
            (const REGION_MAP[1]){
                {.startFreq = 108000000, .endFreq = 137000000, .mode = TRX_MODE_AM},
            },
        .regionsCount = 1,
        .channels = NULL,
        .channelsCount = 0,
    },
    // 2 meter
    {
        .name = "2m",
        .broadcast = false,
        .startFreq = 144000000,
        .endFreq = 146000000,
        .defaultFreq = 145500000,
        .regions =
            (const REGION_MAP[7]){
                {.startFreq = 144000000, .endFreq = 144110000, .mode = TRX_MODE_CW},
                {.startFreq = 144110000, .endFreq = 144170000, .mode = TRX_MODE_USB},
                {.startFreq = 144170000, .endFreq = 144177000, .mode = TRX_MODE_DIGI_U},
                {.startFreq = 144177000, .endFreq = 144360000, .mode = TRX_MODE_USB},
                {.startFreq = 144360000, .endFreq = 144363000, .mode = TRX_MODE_DIGI_U},
                {.startFreq = 144363000, .endFreq = 144491000, .mode = TRX_MODE_USB},
                {.startFreq = 144491000, .endFreq = 146000000, .mode = TRX_MODE_NFM},
            },
        .regionsCount = 7,
        .channels =
            (const CHANNEL_MAP[48]){
                {.subname = "V ", .number = 17, .rxFreq = 145212500, .txFreq = 145212500}, {.subname = "V ", .number = 18, .rxFreq = 145225000, .txFreq = 145225000},
                {.subname = "V ", .number = 19, .rxFreq = 145237500, .txFreq = 145237500}, {.subname = "V ", .number = 20, .rxFreq = 145250000, .txFreq = 145250000},
                {.subname = "V ", .number = 21, .rxFreq = 145262500, .txFreq = 145262500}, {.subname = "V ", .number = 22, .rxFreq = 145275000, .txFreq = 145275000},
                {.subname = "V ", .number = 23, .rxFreq = 145287500, .txFreq = 145287500}, {.subname = "V ", .number = 24, .rxFreq = 145300000, .txFreq = 145300000},
                {.subname = "V ", .number = 25, .rxFreq = 145312500, .txFreq = 145312500}, {.subname = "V ", .number = 26, .rxFreq = 145325000, .txFreq = 145325000},
                {.subname = "V ", .number = 27, .rxFreq = 145337500, .txFreq = 145337500}, {.subname = "V ", .number = 28, .rxFreq = 145350000, .txFreq = 145350000},
                {.subname = "V ", .number = 29, .rxFreq = 145362500, .txFreq = 145362500}, {.subname = "V ", .number = 30, .rxFreq = 145375000, .txFreq = 145375000},
                {.subname = "V ", .number = 31, .rxFreq = 145387500, .txFreq = 145387500}, {.subname = "V ", .number = 32, .rxFreq = 145400000, .txFreq = 145400000},
                {.subname = "V ", .number = 33, .rxFreq = 145412500, .txFreq = 145412500}, {.subname = "V ", .number = 34, .rxFreq = 145425000, .txFreq = 145425000},
                {.subname = "V ", .number = 35, .rxFreq = 145437500, .txFreq = 145437500}, {.subname = "V ", .number = 36, .rxFreq = 145450000, .txFreq = 145450000},
                {.subname = "V ", .number = 37, .rxFreq = 145462500, .txFreq = 145462500}, {.subname = "V ", .number = 38, .rxFreq = 145475000, .txFreq = 145475000},
                {.subname = "V ", .number = 39, .rxFreq = 145487500, .txFreq = 145487500}, {.subname = "V ", .number = 40, .rxFreq = 145500000, .txFreq = 145500000},
                {.subname = "V ", .number = 41, .rxFreq = 145512500, .txFreq = 145512500}, {.subname = "V ", .number = 42, .rxFreq = 145525000, .txFreq = 145525000},
                {.subname = "V ", .number = 43, .rxFreq = 145537500, .txFreq = 145537500}, {.subname = "V ", .number = 44, .rxFreq = 145550000, .txFreq = 145550000},
                {.subname = "V ", .number = 45, .rxFreq = 145562500, .txFreq = 145562500}, {.subname = "V ", .number = 46, .rxFreq = 145575000, .txFreq = 145575000},
                {.subname = "V ", .number = 47, .rxFreq = 145587500, .txFreq = 145587500}, {.subname = "RV", .number = 48, .rxFreq = 145000000, .txFreq = 145600000},
                {.subname = "RV", .number = 49, .rxFreq = 145012500, .txFreq = 145612500}, {.subname = "RV", .number = 50, .rxFreq = 145025000, .txFreq = 145625000},
                {.subname = "RV", .number = 51, .rxFreq = 145037500, .txFreq = 145637500}, {.subname = "RV", .number = 52, .rxFreq = 145050000, .txFreq = 145650000},
                {.subname = "RV", .number = 53, .rxFreq = 145062500, .txFreq = 145662500}, {.subname = "RV", .number = 54, .rxFreq = 145075000, .txFreq = 145675000},
                {.subname = "RV", .number = 55, .rxFreq = 145087500, .txFreq = 145687500}, {.subname = "RV", .number = 56, .rxFreq = 145100000, .txFreq = 145700000},
                {.subname = "RV", .number = 57, .rxFreq = 145112500, .txFreq = 145712500}, {.subname = "RV", .number = 58, .rxFreq = 145125000, .txFreq = 145725000},
                {.subname = "RV", .number = 59, .rxFreq = 145137500, .txFreq = 145737500}, {.subname = "RV", .number = 60, .rxFreq = 145150000, .txFreq = 145750000},
                {.subname = "RV", .number = 61, .rxFreq = 145162500, .txFreq = 145762500}, {.subname = "RV", .number = 62, .rxFreq = 145175000, .txFreq = 145775000},
                {.subname = "RV", .number = 63, .rxFreq = 145187500, .txFreq = 145787500}, {.subname = "RV", .number = 65, .rxFreq = 145200000, .txFreq = 145800000},
            },
        .channelsCount = 48,
    },
    // VHF Marine band
    {
        .name = "Marine",
        .broadcast = false,
        .startFreq = 156000000,
        .endFreq = 174000000,
        .defaultFreq = 170000000,
        .regions =
            (const REGION_MAP[1]){
                {.startFreq = 156000000, .endFreq = 174000000, .mode = TRX_MODE_NFM},
            },
        .regionsCount = 1,
        .channels =
            (const CHANNEL_MAP[87]){
                {.subname = "", .number = 1, .rxFreq = 160650000, .txFreq = 156050000},    {.subname = "A ", .number = 1, .rxFreq = 156050000, .txFreq = 156050000},
                {.subname = "", .number = 2, .rxFreq = 160070000, .txFreq = 156100000},    {.subname = "", .number = 3, .rxFreq = 160750000, .txFreq = 156150000},
                {.subname = "", .number = 4, .rxFreq = 160800000, .txFreq = 156200000},    {.subname = "A ", .number = 4, .rxFreq = 156200000, .txFreq = 156200000},
                {.subname = "", .number = 5, .rxFreq = 160850000, .txFreq = 156250000},    {.subname = "A ", .number = 5, .rxFreq = 156250000, .txFreq = 156250000},
                {.subname = "", .number = 6, .rxFreq = 156300000, .txFreq = 156300000},    {.subname = "", .number = 7, .rxFreq = 160950000, .txFreq = 156350000},
                {.subname = "A ", .number = 7, .rxFreq = 156350000, .txFreq = 156350000},  {.subname = "", .number = 8, .rxFreq = 156400000, .txFreq = 156400000},
                {.subname = "", .number = 9, .rxFreq = 156450000, .txFreq = 156450000},    {.subname = "", .number = 10, .rxFreq = 156500000, .txFreq = 156500000},
                {.subname = "", .number = 11, .rxFreq = 156550000, .txFreq = 156550000},   {.subname = "", .number = 12, .rxFreq = 156600000, .txFreq = 156600000},
                {.subname = "", .number = 13, .rxFreq = 156650000, .txFreq = 156650000},   {.subname = "", .number = 14, .rxFreq = 156700000, .txFreq = 156700000},
                {.subname = "", .number = 15, .rxFreq = 156750000, .txFreq = 156750000},   {.subname = "", .number = 16, .rxFreq = 156800000, .txFreq = 156800000},
                {.subname = "", .number = 17, .rxFreq = 156850000, .txFreq = 156850000},   {.subname = "", .number = 18, .rxFreq = 161500000, .txFreq = 156900000},
                {.subname = "A ", .number = 18, .rxFreq = 156900000, .txFreq = 156900000}, {.subname = "", .number = 19, .rxFreq = 161550000, .txFreq = 156950000},
                {.subname = "A ", .number = 19, .rxFreq = 156950000, .txFreq = 156950000}, {.subname = "", .number = 20, .rxFreq = 161600000, .txFreq = 157000000},
                {.subname = "A ", .number = 20, .rxFreq = 157000000, .txFreq = 157000000}, {.subname = "", .number = 21, .rxFreq = 161650000, .txFreq = 157050000},
                {.subname = "A ", .number = 21, .rxFreq = 157050000, .txFreq = 157050000}, {.subname = "", .number = 22, .rxFreq = 161700000, .txFreq = 157100000},
                {.subname = "A ", .number = 22, .rxFreq = 157100000, .txFreq = 157100000}, {.subname = "", .number = 23, .rxFreq = 161750000, .txFreq = 157150000},
                {.subname = "A ", .number = 23, .rxFreq = 157150000, .txFreq = 157150000}, {.subname = "", .number = 24, .rxFreq = 161800000, .txFreq = 157200000},
                {.subname = "", .number = 25, .rxFreq = 161850000, .txFreq = 157250000},   {.subname = "", .number = 26, .rxFreq = 161900000, .txFreq = 157300000},
                {.subname = "", .number = 27, .rxFreq = 161950000, .txFreq = 157350000},   {.subname = "", .number = 28, .rxFreq = 162000000, .txFreq = 157400000},
                {.subname = "", .number = 60, .rxFreq = 160625000, .txFreq = 156025000},   {.subname = "", .number = 61, .rxFreq = 160675000, .txFreq = 156075000},
                {.subname = "A ", .number = 61, .rxFreq = 156075000, .txFreq = 156075000}, {.subname = "", .number = 62, .rxFreq = 160725000, .txFreq = 156125000},
                {.subname = "A ", .number = 62, .rxFreq = 156125000, .txFreq = 156125000}, {.subname = "", .number = 63, .rxFreq = 160775000, .txFreq = 156175000},
                {.subname = "A ", .number = 63, .rxFreq = 156175000, .txFreq = 156175000}, {.subname = "", .number = 64, .rxFreq = 160825000, .txFreq = 156225000},
                {.subname = "A ", .number = 64, .rxFreq = 156225000, .txFreq = 156225000}, {.subname = "", .number = 65, .rxFreq = 160875000, .txFreq = 156275000},
                {.subname = "A ", .number = 65, .rxFreq = 156275000, .txFreq = 156275000}, {.subname = "", .number = 66, .rxFreq = 160925000, .txFreq = 156325000},
                {.subname = "A ", .number = 66, .rxFreq = 156325000, .txFreq = 156325000}, {.subname = "", .number = 67, .rxFreq = 156375000, .txFreq = 156375000},
                {.subname = "", .number = 68, .rxFreq = 156425000, .txFreq = 156425000},   {.subname = "", .number = 69, .rxFreq = 156475000, .txFreq = 156475000},
                {.subname = "", .number = 70, .rxFreq = 156525000, .txFreq = 156525000},   {.subname = "", .number = 71, .rxFreq = 156575000, .txFreq = 156575000},
                {.subname = "", .number = 72, .rxFreq = 156625000, .txFreq = 156625000},   {.subname = "", .number = 73, .rxFreq = 156675000, .txFreq = 156675000},
                {.subname = "", .number = 74, .rxFreq = 156725000, .txFreq = 156725000},   {.subname = "", .number = 75, .rxFreq = 156775000, .txFreq = 156775000},
                {.subname = "", .number = 76, .rxFreq = 156825000, .txFreq = 156825000},   {.subname = "", .number = 77, .rxFreq = 156875000, .txFreq = 156875000},
                {.subname = "", .number = 78, .rxFreq = 161525000, .txFreq = 156925000},   {.subname = "A ", .number = 78, .rxFreq = 156925000, .txFreq = 156925000},
                {.subname = "", .number = 79, .rxFreq = 161575000, .txFreq = 156975000},   {.subname = "A ", .number = 79, .rxFreq = 156975000, .txFreq = 156975000},
                {.subname = "", .number = 80, .rxFreq = 161625000, .txFreq = 157025000},   {.subname = "A ", .number = 80, .rxFreq = 157025000, .txFreq = 157025000},
                {.subname = "", .number = 81, .rxFreq = 161675000, .txFreq = 157075000},   {.subname = "A ", .number = 81, .rxFreq = 157075000, .txFreq = 157075000},
                {.subname = "", .number = 82, .rxFreq = 161725000, .txFreq = 157125000},   {.subname = "A ", .number = 82, .rxFreq = 157125000, .txFreq = 157125000},
                {.subname = "", .number = 83, .rxFreq = 161775000, .txFreq = 157175000},   {.subname = "A ", .number = 83, .rxFreq = 157175000, .txFreq = 157175000},
                {.subname = "", .number = 84, .rxFreq = 161825000, .txFreq = 157225000},   {.subname = "", .number = 85, .rxFreq = 161875000, .txFreq = 157275000},
                {.subname = "", .number = 86, .rxFreq = 161925000, .txFreq = 157325000},   {.subname = "", .number = 87, .rxFreq = 157375000, .txFreq = 157375000},
                {.subname = "", .number = 88, .rxFreq = 157425000, .txFreq = 157425000},   {.subname = "W ", .number = 01, .rxFreq = 162550000, .txFreq = 162550000},
                {.subname = "W ", .number = 02, .rxFreq = 162400000, .txFreq = 162400000}, {.subname = "W ", .number = 03, .rxFreq = 162475000, .txFreq = 162475000},
                {.subname = "W ", .number = 04, .rxFreq = 162425000, .txFreq = 162425000}, {.subname = "W ", .number = 05, .rxFreq = 162450000, .txFreq = 162450000},
                {.subname = "W ", .number = 06, .rxFreq = 162500000, .txFreq = 162500000}, {.subname = "W ", .number = 07, .rxFreq = 162525000, .txFreq = 162525000},
                {.subname = "W ", .number = 10, .rxFreq = 163275000, .txFreq = 163275000},
            },
        .channelsCount = 87,
    },
    // 70cm
    {
        .name = "70cm",
        .broadcast = false,
        .startFreq = 430000000,
        .endFreq = 446200000,
        .defaultFreq = 432200000,
        .regions =
            (const REGION_MAP[13]){
                {.startFreq = 430000000, .endFreq = 432000000, .mode = TRX_MODE_NFM},
                {.startFreq = 432000000, .endFreq = 432063000, .mode = TRX_MODE_CW},
                {.startFreq = 432063000, .endFreq = 432066000, .mode = TRX_MODE_DIGI_U},
                {.startFreq = 432066000, .endFreq = 432100000, .mode = TRX_MODE_CW},
                {.startFreq = 432100000, .endFreq = 432174000, .mode = TRX_MODE_USB},
                {.startFreq = 432174000, .endFreq = 432177000, .mode = TRX_MODE_DIGI_U},
                {.startFreq = 432177000, .endFreq = 432300000, .mode = TRX_MODE_USB},
                {.startFreq = 432300000, .endFreq = 432303000, .mode = TRX_MODE_DIGI_U},
                {.startFreq = 432303000, .endFreq = 432400000, .mode = TRX_MODE_USB},
                {.startFreq = 432400000, .endFreq = 432500000, .mode = TRX_MODE_CW},
                {.startFreq = 432500000, .endFreq = 434000000, .mode = TRX_MODE_NFM},
                {.startFreq = 434000000, .endFreq = 434100000, .mode = TRX_MODE_CW},
                {.startFreq = 434100000, .endFreq = 446200000, .mode = TRX_MODE_NFM},
            },
        .regionsCount = 13,
        .channels =
            (const CHANNEL_MAP[85]){
                {.subname = "LP", .number = 1, .rxFreq = 433075000, .txFreq = 433075000},  {.subname = "LP", .number = 2, .rxFreq = 433100000, .txFreq = 433100000},
                {.subname = "LP", .number = 3, .rxFreq = 433125000, .txFreq = 433125000},  {.subname = "LP", .number = 4, .rxFreq = 433150000, .txFreq = 433150000},
                {.subname = "LP", .number = 5, .rxFreq = 433175000, .txFreq = 433175000},  {.subname = "LP", .number = 6, .rxFreq = 433200000, .txFreq = 433200000},
                {.subname = "LP", .number = 7, .rxFreq = 433225000, .txFreq = 433225000},  {.subname = "LP", .number = 8, .rxFreq = 433250000, .txFreq = 433250000},
                {.subname = "LP", .number = 9, .rxFreq = 433275000, .txFreq = 433275000},  {.subname = "LP", .number = 10, .rxFreq = 433300000, .txFreq = 433300000},
                {.subname = "LP", .number = 11, .rxFreq = 433325000, .txFreq = 433325000}, {.subname = "LP", .number = 12, .rxFreq = 433350000, .txFreq = 433350000},
                {.subname = "LP", .number = 13, .rxFreq = 433375000, .txFreq = 433375000}, {.subname = "LP", .number = 14, .rxFreq = 433400000, .txFreq = 433400000},
                {.subname = "LP", .number = 15, .rxFreq = 433425000, .txFreq = 433425000}, {.subname = "LP", .number = 16, .rxFreq = 433450000, .txFreq = 433450000},
                {.subname = "LP", .number = 17, .rxFreq = 433475000, .txFreq = 433475000}, {.subname = "LP", .number = 18, .rxFreq = 433500000, .txFreq = 433500000},
                {.subname = "LP", .number = 19, .rxFreq = 433525000, .txFreq = 433525000}, {.subname = "LP", .number = 20, .rxFreq = 433550000, .txFreq = 433550000},
                {.subname = "LP", .number = 21, .rxFreq = 433575000, .txFreq = 433575000}, {.subname = "LP", .number = 22, .rxFreq = 433600000, .txFreq = 433600000},
                {.subname = "LP", .number = 23, .rxFreq = 433625000, .txFreq = 433625000}, {.subname = "LP", .number = 24, .rxFreq = 433650000, .txFreq = 433650000},
                {.subname = "LP", .number = 25, .rxFreq = 433675000, .txFreq = 433675000}, {.subname = "LP", .number = 26, .rxFreq = 433700000, .txFreq = 433700000},
                {.subname = "LP", .number = 27, .rxFreq = 433725000, .txFreq = 433725000}, {.subname = "LP", .number = 28, .rxFreq = 433750000, .txFreq = 433750000},
                {.subname = "LP", .number = 29, .rxFreq = 433775000, .txFreq = 433775000}, {.subname = "LP", .number = 30, .rxFreq = 433800000, .txFreq = 433800000},
                {.subname = "LP", .number = 31, .rxFreq = 433825000, .txFreq = 433825000}, {.subname = "LP", .number = 32, .rxFreq = 433850000, .txFreq = 433850000},
                {.subname = "LP", .number = 33, .rxFreq = 433875000, .txFreq = 433875000}, {.subname = "LP", .number = 34, .rxFreq = 433900000, .txFreq = 433900000},
                {.subname = "LP", .number = 35, .rxFreq = 433925000, .txFreq = 433925000}, {.subname = "LP", .number = 36, .rxFreq = 433950000, .txFreq = 433950000},
                {.subname = "LP", .number = 37, .rxFreq = 433975000, .txFreq = 433975000}, {.subname = "LP", .number = 38, .rxFreq = 434000000, .txFreq = 434000000},
                {.subname = "LP", .number = 39, .rxFreq = 434025000, .txFreq = 434025000}, {.subname = "LP", .number = 40, .rxFreq = 434050000, .txFreq = 434050000},
                {.subname = "LP", .number = 41, .rxFreq = 434075000, .txFreq = 434075000}, {.subname = "LP", .number = 42, .rxFreq = 434100000, .txFreq = 434100000},
                {.subname = "LP", .number = 43, .rxFreq = 434125000, .txFreq = 434125000}, {.subname = "LP", .number = 44, .rxFreq = 434150000, .txFreq = 434150000},
                {.subname = "LP", .number = 45, .rxFreq = 434175000, .txFreq = 434175000}, {.subname = "LP", .number = 46, .rxFreq = 434200000, .txFreq = 434200000},
                {.subname = "LP", .number = 47, .rxFreq = 434225000, .txFreq = 434225000}, {.subname = "LP", .number = 48, .rxFreq = 434250000, .txFreq = 434250000},
                {.subname = "LP", .number = 49, .rxFreq = 434275000, .txFreq = 434275000}, {.subname = "LP", .number = 50, .rxFreq = 434300000, .txFreq = 434300000},
                {.subname = "LP", .number = 51, .rxFreq = 434325000, .txFreq = 434325000}, {.subname = "LP", .number = 52, .rxFreq = 434350000, .txFreq = 434350000},
                {.subname = "LP", .number = 53, .rxFreq = 434375000, .txFreq = 434375000}, {.subname = "LP", .number = 54, .rxFreq = 434400000, .txFreq = 434400000},
                {.subname = "LP", .number = 55, .rxFreq = 434425000, .txFreq = 434425000}, {.subname = "LP", .number = 56, .rxFreq = 434450000, .txFreq = 434450000},
                {.subname = "LP", .number = 57, .rxFreq = 434475000, .txFreq = 434475000}, {.subname = "LP", .number = 58, .rxFreq = 434500000, .txFreq = 434500000},
                {.subname = "LP", .number = 59, .rxFreq = 434525000, .txFreq = 434525000}, {.subname = "LP", .number = 60, .rxFreq = 434550000, .txFreq = 434550000},
                {.subname = "LP", .number = 61, .rxFreq = 434575000, .txFreq = 434575000}, {.subname = "LP", .number = 62, .rxFreq = 434600000, .txFreq = 434600000},
                {.subname = "LP", .number = 63, .rxFreq = 434625000, .txFreq = 434625000}, {.subname = "LP", .number = 64, .rxFreq = 434650000, .txFreq = 434650000},
                {.subname = "LP", .number = 65, .rxFreq = 434675000, .txFreq = 434675000}, {.subname = "LP", .number = 66, .rxFreq = 434700000, .txFreq = 434700000},
                {.subname = "LP", .number = 67, .rxFreq = 434725000, .txFreq = 434725000}, {.subname = "LP", .number = 68, .rxFreq = 434750000, .txFreq = 434750000},
                {.subname = "LP", .number = 69, .rxFreq = 434775000, .txFreq = 434775000}, {.subname = "PM", .number = 1, .rxFreq = 446006250, .txFreq = 446006250},
                {.subname = "PM", .number = 2, .rxFreq = 446018750, .txFreq = 446018750},  {.subname = "PM", .number = 3, .rxFreq = 446031250, .txFreq = 446031250},
                {.subname = "PM", .number = 4, .rxFreq = 446043750, .txFreq = 446043750},  {.subname = "PM", .number = 5, .rxFreq = 446056250, .txFreq = 446056250},
                {.subname = "PM", .number = 6, .rxFreq = 446068750, .txFreq = 446068750},  {.subname = "PM", .number = 7, .rxFreq = 446081250, .txFreq = 446081250},
                {.subname = "PM", .number = 8, .rxFreq = 446093750, .txFreq = 446093750},  {.subname = "PM", .number = 9, .rxFreq = 446106250, .txFreq = 446106250},
                {.subname = "PM", .number = 10, .rxFreq = 446118750, .txFreq = 446118750}, {.subname = "PM", .number = 11, .rxFreq = 446131250, .txFreq = 446131250},
                {.subname = "PM", .number = 12, .rxFreq = 446143750, .txFreq = 446143750}, {.subname = "PM", .number = 13, .rxFreq = 446156250, .txFreq = 446156250},
                {.subname = "PM", .number = 14, .rxFreq = 446168750, .txFreq = 446168750}, {.subname = "PM", .number = 15, .rxFreq = 446181250, .txFreq = 446181250},
                {.subname = "PM", .number = 16, .rxFreq = 446193750, .txFreq = 446193750},
            },
        .channelsCount = 85,
    },
    // 23cm
    {
        .name = "23cm",
        .broadcast = false,
        .startFreq = 1260000000,
        .endFreq = 1300000000,
        .defaultFreq = 1296000000,
        .regions =
            (const REGION_MAP[11]){
                {.startFreq = 1260000000, .endFreq = 1270000000, .mode = TRX_MODE_NFM},
                {.startFreq = 1270000000, .endFreq = 1290994000, .mode = TRX_MODE_NFM},
                {.startFreq = 1290994000, .endFreq = 1291481000, .mode = TRX_MODE_NFM},
                {.startFreq = 1291481000, .endFreq = 1296000000, .mode = TRX_MODE_NFM},
                {.startFreq = 1296000000, .endFreq = 1296065000, .mode = TRX_MODE_CW},
                {.startFreq = 1296065000, .endFreq = 1296068000, .mode = TRX_MODE_DIGI_U},
                {.startFreq = 1296068000, .endFreq = 1296800000, .mode = TRX_MODE_CW},
                {.startFreq = 1296800000, .endFreq = 1296994000, .mode = TRX_MODE_CW},
                {.startFreq = 1296994000, .endFreq = 1297490000, .mode = TRX_MODE_NFM},
                {.startFreq = 1297490000, .endFreq = 1298000000, .mode = TRX_MODE_NFM},
                {.startFreq = 1298000000, .endFreq = 1300000000, .mode = TRX_MODE_NFM},
            },
        .regionsCount = 11,
        .channels = NULL,
        .channelsCount = 0,
    },
    // 13cm
    {
        .name = "13cm",
        .broadcast = false,
        .startFreq = 2320000000,
        .endFreq = 2450000000,
        .defaultFreq = 2320100000,
        .regions =
            (const REGION_MAP[3]){
                {.startFreq = 2320000000, .endFreq = 2320065000, .mode = TRX_MODE_USB},
                {.startFreq = 2320065000, .endFreq = 2320068000, .mode = TRX_MODE_DIGI_U},
                {.startFreq = 2320068000, .endFreq = 2450000000, .mode = TRX_MODE_USB},
            },
        .regionsCount = 3,
        .channels = NULL,
        .channelsCount = 0,
    },
    // 6cm
    {
        .name = "6cm",
        .broadcast = false,
        .startFreq = 5650000000,
        .endFreq = 5850000000,
        .defaultFreq = 5760000000,
        .regions =
            (const REGION_MAP[3]){
                {.startFreq = 5650000000, .endFreq = 5650065000, .mode = TRX_MODE_CW},
                {.startFreq = 5650065000, .endFreq = 5650068000, .mode = TRX_MODE_DIGI_U},
                {.startFreq = 5650068000, .endFreq = 5850000000, .mode = TRX_MODE_CW},
            },
        .regionsCount = 3,
        .channels = NULL,
        .channelsCount = 0,
    },
    // 3cm
    {
        .name = "3cm",
        .broadcast = false,
        .startFreq = 10000000000,
        .endFreq = 10489400000,
        .defaultFreq = 10368000000,
        .regions =
            (const REGION_MAP[3]){
                {.startFreq = 10000000000, .endFreq = 10368200000, .mode = TRX_MODE_CW},
                {.startFreq = 10368200000, .endFreq = 10368203000, .mode = TRX_MODE_DIGI_U},
                {.startFreq = 10368203000, .endFreq = 10489400000, .mode = TRX_MODE_CW},
            },
        .regionsCount = 3,
        .channels = NULL,
        .channelsCount = 0,
    },
    // QO-100
    {
        .name = "QO-100",
        .broadcast = false,
        .startFreq = 10489500000,
        .endFreq = 10490000000,
        .defaultFreq = 10489700000,
        .regions =
            (const REGION_MAP[8]){
                {.startFreq = 10489500000, .endFreq = 10489540000, .mode = TRX_MODE_CW},
                {.startFreq = 10489540000, .endFreq = 10489650000, .mode = TRX_MODE_DIGI_U},
                {.startFreq = 10489650000, .endFreq = 10489745000, .mode = TRX_MODE_USB},
                {.startFreq = 10489745000, .endFreq = 10489755000, .mode = TRX_MODE_DIGI_U},
                {.startFreq = 10489755000, .endFreq = 10489850000, .mode = TRX_MODE_USB},
                {.startFreq = 10489850000, .endFreq = 10489870000, .mode = TRX_MODE_DIGI_U},
                {.startFreq = 10489870000, .endFreq = 10489990000, .mode = TRX_MODE_USB},
                {.startFreq = 10489990000, .endFreq = 10490000000, .mode = TRX_MODE_DIGI_U},
            },
        .regionsCount = 8,
        .channels = NULL,
        .channelsCount = 0,
    },
    // 1.2cm
    {
        .name = "1.2cm",
        .broadcast = false,
        .startFreq = 24000000000,
        .endFreq = 24250000000,
        .defaultFreq = 24048200000,
        .regions =
            (const REGION_MAP[1]){
                {.startFreq = 24000000000, .endFreq = 24250000000, .mode = TRX_MODE_CW},
            },
        .regionsCount = 1,
        .channels = NULL,
        .channelsCount = 0,
    },
};

const BEACON_FREQUENCY TIME_BEACONS[TIME_BEACONS_COUNT] = {
    {
        .name = "JJY",
        .frequency = 40000,
    },
    {
        .name = "RTZ",
        .frequency = 50000,
    },
    {
        .name = "JJY",
        .frequency = 60000,
    },
    {
        .name = "MSF",
        .frequency = 60000,
    },
    {
        .name = "WWVB",
        .frequency = 60000,
    },
    {
        .name = "RBU",
        .frequency = 66660,
    },
    {
        .name = "HBG",
        .frequency = 75000,
    },
    {
        .name = "DCF77",
        .frequency = 77500,
    },
    {
        .name = "WWV",
        .frequency = 2500000,
    },
    {
        .name = "RWM",
        .frequency = 4996000,
    },
    {
        .name = "WWV",
        .frequency = 5000000,
    },
    {
        .name = "RWM",
        .frequency = 9996000,
    },
    {
        .name = "WWV",
        .frequency = 10000000,
    },
    {
        .name = "RWM",
        .frequency = 14996000,
    },
    {
        .name = "WWV",
        .frequency = 15000000,
    },
    {
        .name = "WWV",
        .frequency = 20000000,
    },
};

// band number from frequency
int8_t getBandFromFreq(uint64_t freq, bool nearest) {
	for (int8_t b = 0; b < BANDS_COUNT; b++) {
		if (BANDS[b].startFreq <= freq && freq <= BANDS[b].endFreq) {
			return b;
		}
	}

	if (nearest) {
		int8_t near_band = 0;
		int64_t near_diff = 99999999999;
		for (int8_t b = 0; b < BANDS_COUNT; b++) {
			if (llabs((int64_t)BANDS[b].startFreq - (int64_t)freq) < near_diff) {
				near_diff = llabs((int64_t)BANDS[b].startFreq - (int64_t)freq);
				near_band = b;
			}
			if (llabs((int64_t)BANDS[b].endFreq - (int64_t)freq) < near_diff) {
				near_diff = llabs((int64_t)BANDS[b].endFreq - (int64_t)freq);
				near_band = b;
			}
		}
		return near_band;
	}

	return -1;
}

// mode from frequency
uint_fast8_t getModeFromFreq(uint64_t freq) {
	uint_fast8_t ret = 0;
	ret = freq <= 30000000 ? TRX_MODE_SAM_STEREO : TRX_MODE_NFM;

	for (uint_fast16_t b = 0; b < BANDS_COUNT; b++) {
		if (BANDS[b].startFreq <= freq && freq <= BANDS[b].endFreq) {
			for (uint_fast16_t r = 0; r < BANDS[b].regionsCount; r++) {
				if (BANDS[b].regions[r].startFreq <= freq && freq < BANDS[b].regions[r].endFreq) {
					ret = BANDS[b].regions[r].mode;
					return ret;
				}
			}
		}
	}

	return ret;
}

int16_t getChannelbyFreq(uint64_t freq, bool txfreq) {
	int8_t band = getBandFromFreq(freq, false);
	if (band != -1) {
		for (int16_t ind = 0; ind < BANDS[band].channelsCount; ind++) {
			if (BANDS[band].channels[ind].rxFreq == freq && !txfreq) {
				return ind;
			}
			if (BANDS[band].channels[ind].txFreq == freq && txfreq) {
				return ind;
			}
		}
	}
	return -1;
}
