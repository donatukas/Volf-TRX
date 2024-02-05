#ifndef FFT_h
#define FFT_h

#include "codec.h"
#include "functions.h"
#include "hardware.h"
#include "screen_layout.h"
#include <math.h>
#include <stdbool.h>

#ifdef STM32H743xx
#define FFT_SIZE 1024            // specify the size of the calculated FFT
#define FFT_USEFUL_SIZE 960      // size after FFT cropping
#define FFT_MAX_MEANS 10         // store old fft data for meaning
#define FFT_SHORT_BUFFER_SIZE 20 // lines in small buffer for painting (for test, dont use on production version)
#endif

#ifdef STM32F407xx
#define FFT_SIZE 512             // specify the size of the calculated FFT
#define FFT_USEFUL_SIZE 480      // size after FFT cropping
#define FFT_MAX_MEANS 6          // store old fft data for meaning
#define FFT_SHORT_BUFFER_SIZE 20 // lines in small buffer for painting
#endif

#define FFT_HALF_SIZE (FFT_SIZE / 2)
#define FFT_DOUBLE_SIZE_BUFFER (FFT_SIZE * 2) // Buffer size for FFT calculation // average threshold of the FFT signal
#define FFT_COMPRESS_INTERVAL 0.95f           // compress interval of the FFT signal
#define FFT_STEP_COEFF 10.0f                  // step coefficient for auto-calibration of the FFT signal (more - slower)
#define FFT_HZ_IN_PIXEL (float32_t)((float32_t)TRX_GetRXSampleRate * ((float32_t)FFT_USEFUL_SIZE / (float32_t)FFT_SIZE) / (float32_t)LAYOUT->FFT_PRINT_SIZE) // hertz per FFT pixel RX
#define FFT_TX_HZ_IN_PIXEL (float32_t)((float32_t)TRX_SAMPLERATE * ((float32_t)FFT_USEFUL_SIZE / (float32_t)FFT_SIZE) / (float32_t)LAYOUT->FFT_PRINT_SIZE)   // hertz per FFT pixel TX
#define FFT_BW_BRIGHTNESS_1 10                                                                                                                               // pixel brightness on bw bar
#define FFT_BW_BRIGHTNESS_2 3                                                                                                                                // pixel brightness on bw bar
#define FFT_SCALE_LINES_BRIGHTNESS 0.4f // pixel brightness on scale lines
#define FFT_MAX_GRID_NUMBER 13          // max grid lines
#if (defined(LAY_800x480))
#define FFT_LENS_STEP_START 0.19f // each lens step
#define FFT_LENS_STEP 0.0055f
#else
#define FFT_LENS_STEP_START 0.4f // each lens step
#define FFT_LENS_STEP 0.013f
#endif
#define FFT_3D_SLIDES 40 // 3D FFT parameters
#define FFT_3D_Y_OFFSET 2
#define FFT_3D_X_OFFSET 5
#define FFT_MAX_AVER 30                                                               // store old fft data for averaging
#define FFT_MAX_TOP_SCALE 30                                                          // maximum scale parameter
#define FFT_TX_MIN_LEVEL 20.0f                                                        // fft tx minimum scale level
#define FFT_DBM_GRID_TOP_MARGIN 12                                                    // dBm grid top margin
#define FFT_DBM_GRID_INTERVAL 24                                                      // interval between dBm grids
#define FFT_DBM_COMPENSATION 6.0f                                                     // for amplitude conversion
#define FFT_SCALE_TYPE (TRX_on_TX ? (SHOW_RX_FFT_ON_TX ? 2 : 0) : TRX.FFT_Scale_Type) // set scale type to dBm for Duplex TX and Ampl for Simplex TX
#define FFT_SEC_BW_BRIGHTNESS -2

#define GET_FFTHeight                                                         \
	((TRX.FFT_Height == 1) ? LAYOUT->FFT_HEIGHT_STYLE1                          \
	                       : ((TRX.FFT_Height == 2) ? LAYOUT->FFT_HEIGHT_STYLE2 \
	                                                : ((TRX.FFT_Height == 3) ? LAYOUT->FFT_HEIGHT_STYLE3 : ((TRX.FFT_Height == 4) ? LAYOUT->FFT_HEIGHT_STYLE4 : LAYOUT->FFT_HEIGHT_STYLE5))))
#define GET_WTFHeight                                                         \
	((TRX.FFT_Height == 1) ? LAYOUT->WTF_HEIGHT_STYLE1                          \
	                       : ((TRX.FFT_Height == 2) ? LAYOUT->WTF_HEIGHT_STYLE2 \
	                                                : ((TRX.FFT_Height == 3) ? LAYOUT->WTF_HEIGHT_STYLE3 : ((TRX.FFT_Height == 4) ? LAYOUT->WTF_HEIGHT_STYLE4 : LAYOUT->WTF_HEIGHT_STYLE5))))

#define FFT_Show_Sec_VFO (TRX.Show_Sec_VFO || TRX.SPLIT_Enabled)

#define ZOOMFFT_DECIM_STAGES_IIR 9
#define ZOOMFFT_DECIM_STAGES_FIR 4

#define FFT_MARKERS_COUNT 50

typedef struct {
	int16_t x;
	uint16_t y;
	uint16_t width;
} FFT_Marker;

// Public variables
extern uint32_t FFT_buff_index;
extern bool FFT_buff_current;
extern bool FFT_need_fft;
extern bool FFT_new_buffer_ready;
extern float32_t FFTInput_I_A[FFT_HALF_SIZE];
extern float32_t FFTInput_Q_A[FFT_HALF_SIZE];
extern float32_t FFTInput_I_B[FFT_HALF_SIZE];
extern float32_t FFTInput_Q_B[FFT_HALF_SIZE];
extern uint16_t FFT_FPS;
extern uint16_t FFT_FPS_Last;
extern bool NeedWTFRedraw;
extern bool NeedFFTReinit;
extern uint32_t FFT_current_spectrum_width_Hz;
extern float32_t FFT_Current_TX_SNR;
extern float32_t FFT_Current_TX_IMD3;
extern float32_t FFT_Current_TX_IMD5;
extern float32_t FFT_Current_TX_IMD7;
extern float32_t FFT_Current_TX_IMD9;

#if HRDW_HAS_FULL_FFT_BUFFER
extern uint16_t print_output_buffer[FFT_AND_WTF_HEIGHT][MAX_FFT_PRINT_SIZE]; // buffer with fft/3d fft/wtf print data
#endif

// Public methods
extern void FFT_Init(void);                                        // FFT initialization
extern void FFT_PreInit(void);                                     // FFT precalculation
extern void FFT_bufferPrepare(void);                               // FFT Buffer process
extern void FFT_doFFT(void);                                       // FFT calculation
extern bool FFT_printFFT(void);                                    // FFT output
extern void FFT_afterPrintFFT(void);                               // FFT output after callback
extern uint64_t getFreqOnFFTPosition(uint16_t position);           // get frequency from pixel X position
extern int32_t getFreqPositionOnFFT(uint64_t freq, bool full_pos); // get the position on the FFT for a given frequency
extern void FFT_ShortBufferPrintFFT(void);

#endif
