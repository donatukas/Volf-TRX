#ifndef IMAGES_H
#define IMAGES_H

#include "functions.h"
#include "hardware.h"

typedef struct { // image structure
	const uint16_t width;
	const uint16_t height;
	const uint16_t *data;
} tIMAGE;

extern const tIMAGE IMAGES_logo160;
extern const tIMAGE IMAGES_logo320;
extern const tIMAGE IMAGES_logo480;
extern const tIMAGE IMAGES_logoLite;
extern const uint8_t IMAGES_logo800_jpeg[25543];
extern const uint8_t IMAGES_logo800_NY_jpeg[26564];

extern const tIMAGE IMAGES_wifi_inactive;
extern const tIMAGE IMAGES_wifi_active;
extern const tIMAGE image_data_meter;
extern const tIMAGE IMAGES_sd_inactive;
extern const tIMAGE IMAGES_sd_active;
extern const tIMAGE IMAGES_fan;
extern const tIMAGE IMAGES_fan2;
extern const tIMAGE image_data_meter2;
extern const tIMAGE GR_LINE;
extern const tIMAGE TX_ICO;
extern const tIMAGE TU_ICO;
extern const tIMAGE RX_ICO;
extern const tIMAGE VFO_A_ICO;
extern const tIMAGE VFO_B_ICO;
extern const tIMAGE VFO_AB_ICO;
extern const tIMAGE VFO_BA_ICO;
extern const tIMAGE VFO_A_B_ICO;
extern const tIMAGE VFO_B_A_ICO;
extern const tIMAGE ANT1_ICO;
extern const tIMAGE ANT2_ICO;
extern const tIMAGE ANT3_ICO;
extern const tIMAGE ANT4_ICO;
extern const tIMAGE ANT1_2_ICO;
extern const tIMAGE MODE_ALL;
extern const tIMAGE MODE_ICO_AM;
extern const tIMAGE MODE_ICO_SAM;
extern const tIMAGE MODE_ICO_CW;
extern const tIMAGE MODE_ICO_DIGL;
extern const tIMAGE MODE_ICO_DIGU;
extern const tIMAGE MODE_ICO_WFM;
extern const tIMAGE MODE_ICO_NFM;
extern const tIMAGE MODE_ICO_LSB;
extern const tIMAGE MODE_ICO_USB;
extern const tIMAGE MODE_ICO_LOOP;
extern const tIMAGE MODE_ICO_RTTY;
extern const tIMAGE MODE_ICO_IQ;
extern const tIMAGE MODE_ICO_SELECT;

#endif
