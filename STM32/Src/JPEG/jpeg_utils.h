#ifndef __JPEG_UTILS_H
#define __JPEG_UTILS_H

#include "jpeg_utils_conf.h"

typedef uint32_t (*JPEG_YCbCrToRGB_Convert_Function)(uint8_t *pInBuffer, uint8_t *pOutBuffer, uint32_t BlockIndex, uint32_t DataCount, uint32_t *ConvertedDataCount);

HAL_StatusTypeDef JPEG_GetDecodeColorConvertFunc(JPEG_ConfTypeDef *pJpegInfo, JPEG_YCbCrToRGB_Convert_Function *pFunction, uint32_t *ImageNbMCUs);

#endif /* __JPEG_UTILS_H */
