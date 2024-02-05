#ifndef PRE_DISTORTION_h
#define PRE_DISTORTION_h

#include "hardware.h"

#define DPD_VERSION 10
#define DPD_POINTS_COUNT 9
#define DPD_MAX_ERRORS 5
#define DPD_CORRECTION_GAIN_STEP (DPD_calibration_stage == 6 ? 0.001f : 0.005f)

typedef struct {
	uint8_t version;
	float32_t distortion_gain_points[DPD_POINTS_COUNT];
} DPD_Instance;

void DPD_Init();
void DPD_ProcessPredistortion(float32_t *buffer_i, float32_t *buffer_q, uint32_t size);
void DPD_StartCalibration();
void DPD_ProcessCalibration();

#endif
