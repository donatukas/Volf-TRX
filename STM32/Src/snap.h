#ifndef SNAP_H
#define SNAP_H

#include "fft.h"
#include "hardware.h"

#define SNAP_AVERAGING 10                // num of samples vefore process
#define SNAP_NOISE_FLOOR 0.1f            // noise floor
#define SNAP_BW_SNR_THRESHOLD 10.0f      // Manual snap SNR threshold
#define SNAP_BW_SNR_AUTO_THRESHOLD 15.0f // Auto snap SNR threshold
#define SNAP_AUTO_TIMEOUT 1              // Auto snap timeout after activity, seconds

extern void SNAP_DoSnap(bool do_auto, uint8_t mode);
extern void SNAP_FillBuffer(float32_t *buff);

#endif
