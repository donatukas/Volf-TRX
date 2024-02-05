#include "fft.h"
#include "agc.h"
#include "arm_const_structs.h"
#include "audio_filters.h"
#include "cw_decoder.h"
#include "lcd.h"
#include "main.h"
#include "pre_distortion.h"
#include "rds_decoder.h"
#include "satellite.h"
#include "screen_layout.h"
#include "snap.h"
#include "trx_manager.h"
#include "vad.h"
#include "wifi.h"

// Public variables
bool FFT_need_fft = true;                          // need to prepare data for display on the screen
bool FFT_new_buffer_ready = false;                 // buffer is full, can be processed
uint32_t FFT_buff_index = 0;                       // current buffer index when it is filled with FPGA
bool FFT_buff_current = 0;                         // current FFT Input buffer A - false, B - true
IRAM2 float32_t FFTInput_I_A[FFT_HALF_SIZE] = {0}; // incoming buffer FFT I
IRAM2 float32_t FFTInput_Q_A[FFT_HALF_SIZE] = {0}; // incoming buffer FFT Q
IRAM2 float32_t FFTInput_I_B[FFT_HALF_SIZE] = {0}; // incoming buffer FFT I
IRAM2 float32_t FFTInput_Q_B[FFT_HALF_SIZE] = {0}; // incoming buffer FFT Q
uint16_t FFT_FPS = 0;
uint16_t FFT_FPS_Last = 0;
bool NeedWTFRedraw = false;
bool NeedFFTReinit = false;
uint32_t FFT_current_spectrum_width_Hz = 96000; // Current sectrum width
float32_t FFT_Current_TX_SNR = 0;
float32_t FFT_Current_TX_IMD3 = 0;
float32_t FFT_Current_TX_IMD5 = 0;
float32_t FFT_Current_TX_IMD7 = 0;
float32_t FFT_Current_TX_IMD9 = 0;

// Private variables
#if FFT_SIZE == 2048
const static arm_cfft_instance_f32 *FFT_Inst = &arm_cfft_sR_f32_len2048;
#endif
#if FFT_SIZE == 1024
const static arm_cfft_instance_f32 *FFT_Inst = &arm_cfft_sR_f32_len1024;
#endif
#if FFT_SIZE == 512
const static arm_cfft_instance_f32 *FFT_Inst = &arm_cfft_sR_f32_len512;
#endif
#if FFT_SIZE == 256
const static arm_cfft_instance_f32 *FFT_Inst = &arm_cfft_sR_f32_len256;
#endif

#if HRDW_HAS_FULL_FFT_BUFFER
IRAM2 uint16_t print_output_buffer[FFT_AND_WTF_HEIGHT][MAX_FFT_PRINT_SIZE] = {{0}}; // buffer with fft/3d fft/wtf print data
#else
IRAM2 uint16_t print_output_short_buffer[FFT_SHORT_BUFFER_SIZE][MAX_FFT_PRINT_SIZE] = {0}; // only line buffer with fft/wtf print data
#endif
static float32_t FFTInputCharge[FFT_DOUBLE_SIZE_BUFFER] = {0};                 // charge FFT I and Q buffer
IRAM2 static float32_t FFTInput[FFT_DOUBLE_SIZE_BUFFER] = {0};                 // combined FFT I and Q buffer
IRAM2 static float32_t FFTInput_tmp[MAX_FFT_PRINT_SIZE] = {0};                 // temporary buffer for sorting, moving and fft compressing
SRAM static float32_t FFT_meanBuffer[FFT_MAX_MEANS][MAX_FFT_PRINT_SIZE] = {0}; // averaged FFT buffer (for output)
IRAM2 static float32_t FFTOutput_mean[MAX_FFT_PRINT_SIZE] = {0};               // averaged FFT buffer (for output)
IRAM2 static float32_t FFTOutput_average[MAX_FFT_PRINT_SIZE] = {0};            // averaged FFT buffer (for output)
static float32_t maxValueFFT_rx = 0;                                           // maximum value of the amplitude in the resulting frequency response
static float32_t maxValueFFT_tx = 0;                                           // maximum value of the amplitude in the resulting frequency response
static uint64_t currentFFTFreq = 0;
static uint64_t lastWTFFreq = 0;                                                    // last WTF printed freq
static uint16_t palette_fft[MAX_FFT_HEIGHT + 1] = {0};                              // color palette with FFT colors
static uint16_t palette_wtf[MAX_FFT_HEIGHT + 1] = {0};                              // color palette with FFT colors
static uint16_t palette_bg_gradient[MAX_FFT_HEIGHT + 1] = {0};                      // color palette with gradient background of FFT
static uint16_t palette_bw_fft_colors[MAX_FFT_HEIGHT + 1] = {0};                    // color palette with bw highlighted FFT colors
static uint16_t palette_bw_wtf_colors[MAX_FFT_HEIGHT + 1] = {0};                    // color palette with bw highlighted FFT colors
static uint16_t palette_bw_bg_colors[MAX_FFT_HEIGHT + 1] = {0};                     // color palette with bw highlighted background colors
SRAM static uint8_t indexed_wtf_buffer[MAX_WTF_HEIGHT][MAX_FFT_PRINT_SIZE] = {{0}}; // indexed color buffer with wtf
static uint64_t wtf_buffer_freqs[MAX_WTF_HEIGHT] = {0};                             // frequencies for each row of the waterfall
static uint64_t fft_meanbuffer_freqs[FFT_MAX_MEANS] = {0};                          // frequencies for each row of the fft mean buffer
IRAM2 static uint16_t fft_header[MAX_FFT_PRINT_SIZE] = {0};                         // buffer with fft colors output
#if HRDW_HAS_FULL_FFT_BUFFER
IRAM2 static uint16_t fft_peaks[MAX_FFT_PRINT_SIZE] = {0}; // buffer with fft peaks
#endif
static int32_t grid_lines_pos[20] = {-1};   // grid lines positions
static uint64_t grid_lines_freq[20] = {-1}; // grid lines frequencies
static int32_t rx1_line_pos = -1;           // main receiver position on fft
static int32_t rx2_line_pos = -1;           // secondary receiver position on fft
static int32_t bw_rx1_line_start = 0;       // BW bar params RX1
static int32_t bw_rx1_line_center = 0;
static int32_t rx1_notch_line_pos = 0;
static int32_t bw_rx1_line_end = 0;   // BW bar params RX1
static int32_t bw_rx2_line_start = 0; // BW bar params RX2
static int32_t bw_rx2_line_center = 0;
static int32_t rx2_notch_line_pos = 0;
static int32_t bw_rx2_line_end = 0;                        // BW bar params RX2
static float32_t window_multipliers[FFT_SIZE] = {0};       // coefficients of the selected window function
static float32_t Hz_in_pixel = 1.0f;                       // current FFT density value
static uint8_t bandmap_line_tmp[MAX_FFT_PRINT_SIZE] = {0}; // temporary buffer for bandmap draw
static uint32_t print_fft_dma_estimated_size = 0;          // block size for dma
static uint32_t print_fft_dma_position = 0;                // positior for dma fft print
static uint8_t needredraw_wtf_counter = 3;                 // redraw cycles after event
static bool fft_charge_ready = false;
static bool fft_charge_copying = false;
static uint8_t FFT_meanBuffer_index = 0;
static uint32_t FFT_ChargeBuffer_collected = 0;
static uint64_t FFT_lastFFTChargeBufferFreq = 0;
static uint8_t FFTOutput_mean_count[FFT_SIZE] = {0};
static float32_t minAmplValue_averaged = 0;
static float32_t FFT_minDBM = 0;
static float32_t FFT_maxDBM = 0;
static uint8_t FFT_Markers_Index = 0;
static FFT_Marker FFT_Markers[FFT_MARKERS_COUNT];
// Decimator for Zoom FFT
static arm_fir_decimate_instance_f32 DECIMATE_ZOOM_FFT_I;
static arm_fir_decimate_instance_f32 DECIMATE_ZOOM_FFT_Q;
static float32_t decimZoomFFTIState[FFT_HALF_SIZE + ZOOMFFT_DECIM_STAGES_FIR - 1] = {0};
static float32_t decimZoomFFTQState[FFT_HALF_SIZE + ZOOMFFT_DECIM_STAGES_FIR - 1] = {0};
static uint8_t fft_zoom = 1;
static uint_fast16_t zoomed_width = 0;
// Коэффициенты для ZoomFFT lowpass filtering / дециматора
static arm_biquad_cascade_df2T_instance_f32 IIR_biquad_Zoom_FFT_I = {
    .numStages = ZOOMFFT_DECIM_STAGES_IIR, .pCoeffs = (float32_t *)(float32_t[ZOOMFFT_DECIM_STAGES_IIR * 5]){0}, .pState = (float32_t *)(float32_t[ZOOMFFT_DECIM_STAGES_IIR * 2]){0}};
static arm_biquad_cascade_df2T_instance_f32 IIR_biquad_Zoom_FFT_Q = {
    .numStages = ZOOMFFT_DECIM_STAGES_IIR, .pCoeffs = (float32_t *)(float32_t[ZOOMFFT_DECIM_STAGES_IIR * 5]){0}, .pState = (float32_t *)(float32_t[ZOOMFFT_DECIM_STAGES_IIR * 2]){0}};

// 2x/4x/8x/16x/32x magnify lpf iir
static const float32_t FFT_mag_coeffs_x2[ZOOMFFT_DECIM_STAGES_IIR * 5] = {
    0.8384843639921,
    0,
    0,
    0,
    0,
    1,
    0.5130084793341,
    1,
    0.1784114407685,
    -0.6967733943344,
    0.8744089756375,
    0,
    0,
    0,
    0,
    1,
    1.046379755684,
    1,
    0.3420998857106,
    -0.3982809814397,
    1.83222755502,
    0,
    0,
    0,
    0,
    1,
    1.831496024383,
    1,
    0.5072844084012,
    -0.1179052535088,
    0.01953722920982,
    0,
    0,
    0,
    0,
    1,
    0.3029841730578,
    1,
    0.09694668293684,
    -0.9095549467394,
    1,
    0,
    0,
    0,
    0,
};
static const float32_t FFT_mag_coeffs_x4[ZOOMFFT_DECIM_STAGES_IIR * 5] = {
    0.6737499659657,
    0,
    0,
    0,
    0,
    1,
    -1.102065194995,
    1,
    1.353694541279,
    -0.7896377861467,
    0.53324811147,
    0,
    0,
    0,
    0,
    1,
    -0.5853766477218,
    1,
    1.289175897987,
    -0.5882714065646,
    0.6143152247695,
    0,
    0,
    0,
    0,
    1,
    1.182778527244,
    1,
    1.236309127239,
    -0.4063767082903,
    0.01708381580242,
    0,
    0,
    0,
    0,
    1,
    -1.245590418009,
    1,
    1.418191929315,
    -0.9374008035325,
    1,
    0,
    0,
    0,
    0,
};
static const float32_t FFT_mag_coeffs_x8[ZOOMFFT_DECIM_STAGES_IIR * 5] = {
    0.6469981129046,
    0,
    0,
    0,
    0,
    1,
    -1.750671284068,
    1,
    1.766710155669,
    -0.8829517893283,
    0.4645312725883,
    0,
    0,
    0,
    0,
    1,
    -1.553480572725,
    1,
    1.681513354365,
    -0.7637556184482,
    0.2925692260954,
    0,
    0,
    0,
    0,
    1,
    -0.1114766808264,
    1,
    1.601891439147,
    -0.6499504503566,
    0.01652325734055,
    0,
    0,
    0,
    0,
    1,
    -1.797298202754,
    1,
    1.831125104215,
    -0.9660534813317,
    1,
    0,
    0,
    0,
    0,
};
static const float32_t FFT_mag_coeffs_x16[ZOOMFFT_DECIM_STAGES_IIR * 5] = {
    0.6500044972642,
    0.0,
    0.0,
    0.0,
    0.0,
    1.0,
    -1.935616780918,
    1.0,
    1.908632776595,
    -0.9387888949475,
    0.4599444315799,
    0.0,
    0.0,
    0.0,
    0.0,
    1.0,
    -1.880017827578,
    1.0,
    1.851418291083,
    -0.8732990221737,
    0.2087317940803,
    0.0,
    0.0,
    0.0,
    0.0,
    1.0,
    -1.278402634611,
    1.0,
    1.794539349192,
    -0.80764043772,
    0.01645106748385,
    0.0,
    0.0,
    0.0,
    0.0,
    1.0,
    -1.948135342532,
    1.0,
    1.948194658987,
    -0.9825675157696,
    1.0,
    0.0,
    0.0,
    0.0,
    0.0,
};
static const float32_t FFT_mag_coeffs_x32[ZOOMFFT_DECIM_STAGES_IIR * 5] = {
    0.0023872430902,  0.0047744861804,  0.0023872430902,  1.9735102653503,  -0.9830592274666, 0.0023480691016,  0.0046961382031,  0.0023480691016,  1.9411256313324,
    -0.9505178928375, 0.0023118702229,  0.0046237404458,  0.0023118702229,  1.9112001657486,  -0.9204477071762, 0.0022794834804,  0.0045589669608,  0.0022794834804,
    1.8844263553619,  -0.8935443162918, 0.0022515826859,  0.0045031653717,  0.0022515826859,  1.8613611459732,  -0.8703674674034, 0.0022286928724,  0.0044573857449,
    0.0022286928724,  1.8424383401871,  -0.8513531088829, 0.0022112070583,  0.0044224141166,  0.0022112070583,  1.8279830217361,  -0.8368278145790, 0.0021994032431,
    0.0043988064863,  0.0021994032431,  1.8182249069214,  -0.8270224928856, 0.0021934583783,  0.0043869167566,  0.0021934583783,  1.8133102655411,  -0.8220841288567,
};

// magnify decimate fir
static const arm_fir_decimate_instance_f32 FirZoomFFTDecimate = {
    .numTaps = ZOOMFFT_DECIM_STAGES_FIR,
    .pCoeffs = (const float32_t *)(const float32_t[ZOOMFFT_DECIM_STAGES_FIR]){-0.05698952454792, 0.5574889164132, 0.5574889164132, -0.05698952454792},
    .pState = NULL};

// Prototypes
static uint16_t getFFTColor(uint_fast8_t height, bool type); // Get FFT color warmth (blue to red) , type 0 - fft, type 1 - wtf
static void FFT_fill_color_palette(void);                    // prepare the color palette
static uint32_t FFT_getLensCorrection(uint32_t normal_distance_from_center);
#if HRDW_HAS_FULL_FFT_BUFFER
static void FFT_3DPrintFFT(void);
#endif
static float32_t getDBFromFFTAmpl(float32_t ampl);
static float32_t getFFTAmplFromDB(float32_t ampl);
static float32_t getMaxDBMFromFreq(uint64_t freq, uint8_t span);
static void FFT_clearMarkers();
static void FFT_printMarker(uint64_t frequency, char *label, uint16_t baseline, bool direction, uint32_t max_height);

// FFT initialization
void FFT_PreInit(void) {
	// Windowing
	// Optimizaed GAP (Generalized adaptive polynomial) Nuttall
	if (TRX.FFT_Window == 1) {
		float32_t a0 = 1.0f;
		float32_t a2 = -1.9501232504232442f;
		float32_t a4 = 1.7516390954528638f;
		float32_t a6 = -0.9651321809782892f;
		float32_t a8 = 0.3629219021312954f;
		float32_t a10 = -0.0943163918335154f;
		float32_t a12 = 0.0140434805881681f;
		float32_t a14 = 0.0006383045745587f;
		float32_t a16 = -0.0009075461792061f;
		float32_t a18 = 0.0002000671118688f;
		float32_t a20 = -0.0000161042445001f;

		float32_t step = 1.0f / (float32_t)(FFT_SIZE - 1);
		float32_t std_sum = 0;
		for (float32_t N = 0; N <= 1.0f; N += step) {
			std_sum += (N - 0.5f) * (N - 0.5f);
		}
		float32_t std = sqrtf(std_sum / (FFT_SIZE - 1));
		uint32_t index = 0;
		for (float32_t N = 0; N <= 1.0f; N += step) {
			float32_t x = (N - 0.5f) / std;
			float32_t w = a0 + a2 * powf(x, 2) + a4 * powf(x, 4) + a6 * powf(x, 6) + a8 * powf(x, 8) + a10 * powf(x, 10) + a12 * powf(x, 12) + a14 * powf(x, 14) + a16 * powf(x, 16) +
			              a18 * powf(x, 18) + a20 * powf(x, 20);

			window_multipliers[index] = w;
			index++;
		}
	}

	// Dolph–Chebyshev
	if (TRX.FFT_Window == 2) {
		const float64_t atten = 100.0;
		float64_t max = 0.0;
		float64_t tg = pow(10.0, atten / 20.0);
		float64_t x0 = cosh((1.0 / ((float64_t)FFT_SIZE - 1.0)) * acosh(tg));
		float32_t M = (float32_t)(FFT_SIZE - 1) / 2.0f;
		// if ((FFT_SIZE % 2) == 0) M = M + 0.5; /* handle even length windows */
		for (uint32_t nn = 0; nn < ((FFT_SIZE / 2) + 1); nn++) {
			float32_t n = nn - M;
			float64_t sum = 0.0;
			for (uint32_t i = 1; i <= M; i++) {
				float64_t cheby_poly = 0.0;
				float64_t cp_x = x0 * cos(D_PI * i / (float64_t)FFT_SIZE);
				float64_t cp_n = FFT_SIZE - 1;
				if (fabs(cp_x) <= 1) {
					cheby_poly = cos(cp_n * acos(cp_x));
				} else {
					cheby_poly = cosh(cp_n * acosh(cp_x));
				}

				sum += cheby_poly * cos(2.0 * (float64_t)n * D_PI * (float64_t)i / (float64_t)FFT_SIZE);
			}
			window_multipliers[nn] = tg + 2 * sum;
			window_multipliers[FFT_SIZE - nn - 1] = window_multipliers[nn];
			if ((float64_t)window_multipliers[nn] > max) {
				max = (float64_t)window_multipliers[nn];
			}
		}
		for (uint32_t nn = 0; nn < FFT_SIZE; nn++) {
			window_multipliers[nn] /= max; /* normalise everything */
		}
	}

	for (uint_fast16_t i = 0; i < FFT_SIZE; i++) {
		// Blackman-Harris
		if (TRX.FFT_Window == 3) {
			window_multipliers[i] = 0.35875f - 0.48829f * cosf(2.0f * F_PI * (float32_t)i / ((float32_t)FFT_SIZE - 1.0f)) +
			                        0.14128f * cosf(4.0f * F_PI * (float32_t)i / ((float32_t)FFT_SIZE - 1.0f)) - 0.01168f * cosf(6.0f * F_PI * (float32_t)i / ((float32_t)FFT_SIZE - 1.0f));
		}
		// Nuttall
		else if (TRX.FFT_Window == 4) {
			window_multipliers[i] = 0.355768f - 0.487396f * cosf(2.0f * F_PI * (float32_t)i / ((float32_t)FFT_SIZE - 1.0f)) +
			                        0.144232f * cosf(4.0f * F_PI * (float32_t)i / ((float32_t)FFT_SIZE - 1.0f)) - 0.012604f * cosf(6.0f * F_PI * (float32_t)i / ((float32_t)FFT_SIZE - 1.0f));
		}
		// Blackman-Nutall
		else if (TRX.FFT_Window == 5) {
			window_multipliers[i] = 0.3635819f - 0.4891775f * cosf(2.0f * F_PI * (float32_t)i / ((float32_t)FFT_SIZE - 1.0f)) +
			                        0.1365995f * cosf(4.0f * F_PI * (float32_t)i / ((float32_t)FFT_SIZE - 1.0f)) - 0.0106411f * cosf(6.0f * F_PI * (float32_t)i / ((float32_t)FFT_SIZE - 1.0f));
		}
		// Hann
		else if (TRX.FFT_Window == 6) {
			window_multipliers[i] = 0.5f * (1.0f - cosf(2.0f * F_PI * (float32_t)i / (float32_t)FFT_SIZE));
		}
		// Hamming
		else if (TRX.FFT_Window == 7) {
			window_multipliers[i] = 0.54f - 0.46f * cosf((2.0f * F_PI * (float32_t)i) / ((float32_t)FFT_SIZE - 1.0f));
		}
		// No window
		else if (TRX.FFT_Window == 8) {
			window_multipliers[i] = 1.0f;
		}
	}

	dma_memset(indexed_wtf_buffer, GET_FFTHeight, sizeof(indexed_wtf_buffer));
}

void FFT_Init(void) {
	FFT_fill_color_palette();
	// ZoomFFT
	fft_zoom = TRX.FFT_Zoom;
	if (CurrentVFO->Mode == TRX_MODE_CW) {
		fft_zoom = TRX.FFT_ZoomCW;
	}
	if ((TRX_on_TX && !SHOW_RX_FFT_ON_TX) && CurrentVFO->Mode != TRX_MODE_LOOPBACK) {
		fft_zoom /= TRX_GetRXSampleRate / TRX_SAMPLERATE;
		if (fft_zoom < 1) {
			fft_zoom = 1;
		}
	}
	if (fft_zoom > 1) {
		float32_t *mag_coeffs = (float32_t *)FFT_mag_coeffs_x2;
		if (fft_zoom == 4) {
			mag_coeffs = (float32_t *)FFT_mag_coeffs_x4;
		}
		if (fft_zoom == 8) {
			mag_coeffs = (float32_t *)FFT_mag_coeffs_x8;
		}
		if (fft_zoom == 16) {
			mag_coeffs = (float32_t *)FFT_mag_coeffs_x16;
		}
		if (fft_zoom == 32) {
			mag_coeffs = (float32_t *)FFT_mag_coeffs_x32;
		}

		arm_biquad_cascade_df2T_init_f32(&IIR_biquad_Zoom_FFT_I, ZOOMFFT_DECIM_STAGES_IIR, mag_coeffs, IIR_biquad_Zoom_FFT_I.pState);
		arm_biquad_cascade_df2T_init_f32(&IIR_biquad_Zoom_FFT_Q, ZOOMFFT_DECIM_STAGES_IIR, mag_coeffs, IIR_biquad_Zoom_FFT_Q.pState);
		arm_fir_decimate_init_f32(&DECIMATE_ZOOM_FFT_I, FirZoomFFTDecimate.numTaps,
		                          fft_zoom, // Decimation factor
		                          FirZoomFFTDecimate.pCoeffs,
		                          decimZoomFFTIState, // Filter state variables
		                          FFT_HALF_SIZE);

		arm_fir_decimate_init_f32(&DECIMATE_ZOOM_FFT_Q, FirZoomFFTDecimate.numTaps,
		                          fft_zoom, // Decimation factor
		                          FirZoomFFTDecimate.pCoeffs,
		                          decimZoomFFTQState, // Filter state variables
		                          FFT_HALF_SIZE);
		zoomed_width = FFT_SIZE / fft_zoom;
	} else {
		zoomed_width = FFT_SIZE;
	}

	if ((TRX_on_TX && !SHOW_RX_FFT_ON_TX) && CurrentVFO->Mode != TRX_MODE_LOOPBACK) {
		FFT_current_spectrum_width_Hz = TRX_SAMPLERATE / fft_zoom;
	} else {
		FFT_current_spectrum_width_Hz = TRX_GetRXSampleRate / fft_zoom;
	}

	dma_memset(fft_meanbuffer_freqs, 0x00, sizeof(fft_meanbuffer_freqs));
	dma_memset(FFT_meanBuffer, 0x00, sizeof(FFT_meanBuffer));
	dma_memset(FFTInputCharge, 0x00, sizeof(FFTInputCharge));
	dma_memset(FFTInput, 0x00, sizeof(FFTInput));
	dma_memset(FFTInput_I_A, 0x00, sizeof(FFTInput_I_A));
	dma_memset(FFTInput_Q_A, 0x00, sizeof(FFTInput_Q_A));
	dma_memset(FFTInput_I_B, 0x00, sizeof(FFTInput_I_B));
	dma_memset(FFTInput_Q_B, 0x00, sizeof(FFTInput_Q_B));
	dma_memset(FFTOutput_mean, 0x00, sizeof(FFTOutput_mean));
	dma_memset(FFTInput_tmp, 0x00, sizeof(FFTInput_tmp));
#if HRDW_HAS_FULL_FFT_BUFFER
	dma_memset(fft_peaks, 0x00, sizeof(fft_peaks));
#endif
	NeedWTFRedraw = true;
	FFT_new_buffer_ready = false;

	FFT_buff_index = 0;
	FFTInput_I_current = (float32_t *)&FFTInput_I_A[0];
	FFTInput_Q_current = (float32_t *)&FFTInput_Q_A[0];

	NeedFFTReinit = false;
}

// FFT calculation
void FFT_bufferPrepare(void) {
	if (!TRX.FFT_Enabled) {
		return;
	}
	if (!FFT_new_buffer_ready) {
		return;
	}
	if (fft_charge_copying) {
		return;
	}

	// if (CPU_LOAD.Load > 90) return;

	fft_charge_ready = false;

	float32_t *FFTInput_I_current_ = FFT_buff_current ? (float32_t *)FFTInput_I_B : (float32_t *)FFTInput_I_A; // inverted
	float32_t *FFTInput_Q_current_ = FFT_buff_current ? (float32_t *)FFTInput_Q_B : (float32_t *)FFTInput_Q_A;

	// Reset old samples if frequency changed
	uint64_t nowFFTChargeBufferFreq = CurrentVFO->SpectrumCenterFreq;
	if (TRX.WTF_Moving && fabsl((float64_t)FFT_lastFFTChargeBufferFreq - (float64_t)nowFFTChargeBufferFreq) > (500 / fft_zoom)) // zeroing threshold
	{
		dma_memset(FFTInputCharge, 0x00, sizeof(FFTInputCharge));
		FFT_ChargeBuffer_collected = 0;
	}
	FFT_lastFFTChargeBufferFreq = nowFFTChargeBufferFreq;

	// ZoomFFT
	if (fft_zoom > 1) {
		uint32_t zoomed_width_half = zoomed_width / 2;
		// Biquad LPF filter
		arm_biquad_cascade_df2T_f32_IQ(&IIR_biquad_Zoom_FFT_I, &IIR_biquad_Zoom_FFT_Q, FFTInput_I_current_, FFTInput_Q_current_, FFTInput_I_current_, FFTInput_Q_current_, FFT_HALF_SIZE);
		// Decimator
		arm_fir_decimate_f32(&DECIMATE_ZOOM_FFT_I, FFTInput_I_current_, FFTInput_I_current_, FFT_HALF_SIZE);
		arm_fir_decimate_f32(&DECIMATE_ZOOM_FFT_Q, FFTInput_Q_current_, FFTInput_Q_current_, FFT_HALF_SIZE);
		// Shift old data
		dma_memcpy(&FFTInputCharge[0], &FFTInputCharge[zoomed_width_half * 2], sizeof(float32_t) * (FFT_SIZE - zoomed_width_half) * 2); //*2 - >i+q
		// Add new data with 50% overlap
		for (uint_fast16_t i = 0; i < zoomed_width_half; i++) {
			FFTInputCharge[(FFT_SIZE - zoomed_width_half + i) * 2] = FFTInput_I_current_[i];
			FFTInputCharge[(FFT_SIZE - zoomed_width_half + i) * 2 + 1] = FFTInput_Q_current_[i];
		}
		FFT_ChargeBuffer_collected += zoomed_width_half;
	} else {
		// Make a combined buffer for calculation with 50% overlap
		dma_memcpy(&FFTInputCharge[0], &FFTInputCharge[FFT_HALF_SIZE * 2], sizeof(float32_t) * FFT_HALF_SIZE * 2); //*2 - >i+q
		for (uint_fast16_t i = 0; i < FFT_HALF_SIZE; i++) {
			FFTInputCharge[(FFT_HALF_SIZE + i) * 2] = FFTInput_I_current_[i];
			FFTInputCharge[(FFT_HALF_SIZE + i) * 2 + 1] = FFTInput_Q_current_[i];
		}
		FFT_ChargeBuffer_collected += FFT_HALF_SIZE;
	}

	FFT_new_buffer_ready = false;
	fft_charge_ready = true;
}

// FFT calculation
void FFT_doFFT(void) {
	if (!TRX.FFT_Enabled) {
		return;
	}
	if (!FFT_need_fft) {
		return;
	}
	if (!TRX_Inited) {
		return;
	}
	if (!fft_charge_ready) {
		return;
	}
	if (FFT_ChargeBuffer_collected == 0) {
		return;
	}
	/*if (CPU_LOAD.Load > 90)
	  return;*/

	if (NeedFFTReinit) {
		FFT_Init();
		return;
	}

	// Get charge buffer
	fft_charge_copying = true;
	dma_memcpy(FFTInput, FFTInputCharge, sizeof(FFTInput));
	fft_charge_copying = false;

	// Do full windowing
	if (FFT_ChargeBuffer_collected >= FFT_SIZE) {
		FFT_ChargeBuffer_collected = FFT_SIZE;
		arm_cmplx_mult_real_f32(FFTInput, window_multipliers, FFTInput, FFT_SIZE);
	} else // partial windowing
	{
		float32_t coeff_rate = (float32_t)FFT_SIZE / (float32_t)FFT_ChargeBuffer_collected;
		for (uint16_t i = (FFT_SIZE - FFT_ChargeBuffer_collected); i < FFT_SIZE; i++) {
			uint16_t coeff_idx = coeff_rate * (float32_t)(i - (FFT_SIZE - FFT_ChargeBuffer_collected));
			if (coeff_idx > (FFT_SIZE - 1)) {
				coeff_idx = (FFT_SIZE - 1);
			}
			FFTInput[i * 2] = FFTInput[i * 2] * window_multipliers[coeff_idx];
			FFTInput[i * 2 + 1] = FFTInput[i * 2 + 1] * window_multipliers[coeff_idx];
		}

		// Gain signal if partial buffer (for normalize)
		arm_scale_f32(FFTInput, ((float32_t)FFT_SIZE / (float32_t)FFT_ChargeBuffer_collected / 2.0f), FFTInput, FFT_DOUBLE_SIZE_BUFFER);
	}

	// Zoom compensation
	if (fft_zoom == 2) {
		arm_scale_f32(FFTInput, 1.679f, FFTInput, FFT_DOUBLE_SIZE_BUFFER);
	}
	if (fft_zoom == 4) {
		arm_scale_f32(FFTInput, 2.239f, FFTInput, FFT_DOUBLE_SIZE_BUFFER);
	}
	if (fft_zoom == 8) {
		arm_scale_f32(FFTInput, 3.162f, FFTInput, FFT_DOUBLE_SIZE_BUFFER);
	}
	if (fft_zoom == 16) {
		arm_scale_f32(FFTInput, 5.012f, FFTInput, FFT_DOUBLE_SIZE_BUFFER);
	}
	if (fft_zoom == 32) {
		arm_scale_f32(FFTInput, 5.957f, FFTInput, FFT_DOUBLE_SIZE_BUFFER);
	}

	arm_cfft_f32(FFT_Inst, FFTInput, 0, 1);

	// FFT scale
	if (FFT_SCALE_TYPE == 1) { // Squared scale
		arm_cmplx_mag_squared_f32(FFTInput, FFTInput, FFT_SIZE);
	} else { // ampl or dBm scale
		arm_cmplx_mag_f32(FFTInput, FFTInput, FFT_SIZE);
	}

	// Debug VAD
	/*dma_memset(FFTInput, 0x00, sizeof(FFTInput));
	for (uint_fast16_t i = 0; i < FFT_SIZE; i++)
	{
	  FFTInput[i] = VAD_FFTBuffer_Export[i];
	}
	NeedFFTInputBuffer = true;*/

	// Debug CW Decoder
	/*dma_memset(FFTInput, 0x00, sizeof(FFTInput));
	for (uint_fast16_t i = 0; i < CWDECODER_FFTSIZE; i++)
	{
	  FFTInput[i] = CWDEC_FFTBuffer_Export[i];
	  FFTInput[i + CWDECODER_FFTSIZE] = CWDEC_FFTBuffer_Export[i];
	  FFTInput[i + CWDECODER_FFTSIZE * 2] = CWDEC_FFTBuffer_Export[i];
	  FFTInput[i + CWDECODER_FFTSIZE * 3] = CWDEC_FFTBuffer_Export[i];
	}*/

	// Swap fft parts
	dma_memcpy(&FFTInput[FFT_SIZE], &FFTInput[0], sizeof(float32_t) * (FFT_SIZE / 2));            // left - > tmp
	dma_memcpy(&FFTInput[0], &FFTInput[FFT_SIZE / 2], sizeof(float32_t) * (FFT_SIZE / 2));        // right - > left
	dma_memcpy(&FFTInput[FFT_SIZE / 2], &FFTInput[FFT_SIZE], sizeof(float32_t) * (FFT_SIZE / 2)); // tmp - > right

	// Send to Snap
	SNAP_FillBuffer(FFTInput);

	// Compress the calculated FFT to visible
	dma_memcpy(&FFTInput[0], &FFTInput[FFT_SIZE / 2 - FFT_USEFUL_SIZE / 2], sizeof(float32_t) * FFT_USEFUL_SIZE); // useful fft part
	float32_t fft_compress_rate = (float32_t)FFT_USEFUL_SIZE / (float32_t)LAYOUT->FFT_PRINT_SIZE;
	float32_t fft_compress_rate_half = floorf(fft_compress_rate / 2.0f);       // full points
	float32_t fft_compress_rate_parts = fmodf(fft_compress_rate / 2.0f, 1.0f); // partial points

	// Noise floor calculator
	float32_t noise_floor_calc_value = 0;
	static uint32_t noise_floor_calc_index = 0;
	static uint32_t noise_floor_calc_counter = 0;
	if (noise_floor_calc_counter < 100) {
		noise_floor_calc_value = FFTInput[noise_floor_calc_index];
		TRX_NoiseFloor = TRX_NoiseFloor * 0.99f + getDBFromFFTAmpl(noise_floor_calc_value) * 0.01f;
	} else {
		noise_floor_calc_counter = 0;
		arm_min_f32(FFTInput, FFT_USEFUL_SIZE, &noise_floor_calc_value, &noise_floor_calc_index); // -148
	}

	// dBm scale
	if (FFT_SCALE_TYPE == 2) {
		for (uint_fast16_t i = 0; i < FFT_SIZE; i++) {
			FFTInput[i] = getDBFromFFTAmpl(FFTInput[i]);
		}
	}

	if (!TRX.FFT_Lens) // normal compress
	{
		for (uint32_t i = 0; i < LAYOUT->FFT_PRINT_SIZE; i++) {
			int32_t left_index = (uint32_t)((float32_t)i * fft_compress_rate - fft_compress_rate_half);
			if (left_index < 0) {
				left_index = 0;
			}
			int32_t right_index = (uint32_t)((float32_t)i * fft_compress_rate + fft_compress_rate_half);
			if (right_index >= FFT_USEFUL_SIZE) {
				right_index = FFT_USEFUL_SIZE - 1;
			}

			float32_t points = 0;
			float32_t accum = 0.0f;
			// full points
			for (uint32_t index = left_index; index <= right_index; index++) {
				accum += FFTInput[index];
				points += 1.0f;
			}
			// partial points
			if (fft_compress_rate_parts > 0.0f) {
				if (left_index > 0) {
					accum += FFTInput[left_index - 1] * fft_compress_rate_parts;
					points += fft_compress_rate_parts;
				}
				if (right_index < (FFT_USEFUL_SIZE - 1)) {
					accum += FFTInput[right_index + 1] * fft_compress_rate_parts;
					points += fft_compress_rate_parts;
				}
			}
			FFTInput_tmp[i] = accum / points;
		}
	} else // lens compress
	{
		float32_t step_now = FFT_LENS_STEP_START;
		float32_t index1 = (float32_t)FFT_USEFUL_SIZE / 2.0f;
		float32_t index2 = index1;
		for (uint32_t i = 0; i <= (LAYOUT->FFT_PRINT_SIZE / 2); i++) {
			FFTInput_tmp[(LAYOUT->FFT_PRINT_SIZE / 2) - i] = FFTInput[(uint32_t)roundf(index1)];
			if (i != (LAYOUT->FFT_PRINT_SIZE / 2)) {
				FFTInput_tmp[(LAYOUT->FFT_PRINT_SIZE / 2) + i] = FFTInput[(uint32_t)roundf(index2)];
			}

			step_now += FFT_LENS_STEP;
			index1 -= step_now;
			index2 += step_now;

			if (index1 >= FFT_USEFUL_SIZE) {
				index1 = FFT_USEFUL_SIZE - 1;
			}
			if (index1 < 0) {
				index1 = 0;
			}
			if (index2 >= FFT_USEFUL_SIZE) {
				index2 = FFT_USEFUL_SIZE - 1;
			}
			if (index2 < 0) {
				index2 = 0;
			}
		}
	}
	dma_memcpy(&FFTInput, FFTInput_tmp, sizeof(FFTInput_tmp));

	// Averaging
	if (TRX.FFT_Averaging > (FFT_MAX_MEANS + FFT_MAX_AVER)) {
		TRX.FFT_Averaging = (FFT_MAX_MEANS + FFT_MAX_AVER);
	}
	uint8_t max_mean = TRX.FFT_Averaging;
	if (max_mean > FFT_MAX_MEANS) {
		max_mean = FFT_MAX_MEANS;
	}
	uint8_t averaging = 0;
	if (TRX.FFT_Averaging > FFT_MAX_MEANS) {
		averaging = TRX.FFT_Averaging - FFT_MAX_MEANS + 1;
	}

	// Store old FFT for averaging
	dma_memcpy(&FFT_meanBuffer[FFT_meanBuffer_index][0], FFTInput, sizeof(float32_t) * LAYOUT->FFT_PRINT_SIZE);
	fft_meanbuffer_freqs[FFT_meanBuffer_index] = FFT_lastFFTChargeBufferFreq;

	FFT_meanBuffer_index++;
	if (FFT_meanBuffer_index >= max_mean) {
		FFT_meanBuffer_index = 0;
	}

	// Averaging values
	dma_memset(FFTOutput_mean, 0x00, sizeof(FFTOutput_mean));
	dma_memset(FFTOutput_mean_count, 0x00, sizeof(FFTOutput_mean_count));

	for (uint_fast16_t avg_idx = 0; avg_idx < max_mean; avg_idx++) {
		int64_t freq_diff = roundl(((float64_t)((float64_t)fft_meanbuffer_freqs[avg_idx] - (float64_t)CurrentVFO->SpectrumCenterFreq) / (double)Hz_in_pixel) * (float64_t)fft_zoom);

		if (!TRX.WTF_Moving) {
			freq_diff = 0;
		}
		for (uint_fast16_t i = 0; i < LAYOUT->FFT_PRINT_SIZE; i++) {
			int64_t new_x = (int64_t)i - (int64_t)freq_diff;

			if (new_x > -1 && new_x < LAYOUT->FFT_PRINT_SIZE) {
				FFTOutput_mean[i] += FFT_meanBuffer[avg_idx][new_x];
				FFTOutput_mean_count[i]++;
			}
		}
	}

	if (FFT_SCALE_TYPE == 2) { // dBm scale
		for (uint_fast16_t i = 0; i < LAYOUT->FFT_PRINT_SIZE; i++) {
			if (FFTOutput_mean_count[i] > 1) {
				FFTOutput_mean[i] /= (float32_t)FFTOutput_mean_count[i];
			}
		}
	} else { // ampl, squared scales
		for (uint_fast16_t i = 0; i < LAYOUT->FFT_PRINT_SIZE; i++) {
			FFTOutput_mean[i] /= max_mean;
		}
	}

	if (averaging > 0) {
		for (uint_fast16_t i = 0; i < LAYOUT->FFT_PRINT_SIZE; i++) {
			float32_t alpha = 1.0f / averaging;
			FFTOutput_average[i] = FFTOutput_average[i] * (1.0f - alpha) + FFTOutput_mean[i] * (alpha);
			FFTOutput_mean[i] = FFTOutput_average[i];
		}
	}

	// IMD calculator
	if (TRX_on_TX && TRX_Tune && TRX.TWO_SIGNAL_TUNE && SHOW_RX_FFT_ON_TX) {
		uint64_t tx_freq = CurrentVFO->SpectrumCenterFreq + (TRX.XIT_Enabled ? TRX_XIT : 0);
		uint64_t freq_1 = tx_freq + TWO_TONE_GEN_FREQ1;
		uint64_t freq_2 = tx_freq + TWO_TONE_GEN_FREQ2;
		if (CurrentVFO->Mode == TRX_MODE_LSB || CurrentVFO->Mode == TRX_MODE_DIGI_L) {
			freq_1 = tx_freq - TWO_TONE_GEN_FREQ2;
			freq_2 = tx_freq - TWO_TONE_GEN_FREQ1;
		}

		uint64_t imd3_freq_1 = 2 * freq_1 - freq_2;
		uint64_t imd3_freq_2 = 2 * freq_2 - freq_1;
		uint64_t imd5_freq_1 = 2 * imd3_freq_1 - freq_1;
		uint64_t imd5_freq_2 = 2 * imd3_freq_2 - freq_2;
		uint64_t imd7_freq_1 = 2 * imd5_freq_1 - imd3_freq_1;
		uint64_t imd7_freq_2 = 2 * imd5_freq_2 - imd3_freq_2;
		uint64_t imd9_freq_1 = 2 * imd5_freq_1 - freq_1;
		uint64_t imd9_freq_2 = 2 * imd5_freq_2 - freq_2;

		const uint8_t span_Hz = 50;
		float32_t zero_dBm = FFTOutput_mean[LAYOUT->FFT_PRINT_SIZE / 5];
		float32_t freq_dBm_1 = getMaxDBMFromFreq(freq_1, span_Hz);
		float32_t freq_dBm_2 = getMaxDBMFromFreq(freq_2, span_Hz);
		float32_t imd3_dBm_1 = getMaxDBMFromFreq(imd3_freq_1, span_Hz);
		float32_t imd3_dBm_2 = getMaxDBMFromFreq(imd3_freq_2, span_Hz);
		float32_t imd5_dBm_1 = getMaxDBMFromFreq(imd5_freq_1, span_Hz);
		float32_t imd5_dBm_2 = getMaxDBMFromFreq(imd5_freq_2, span_Hz);
		float32_t imd7_dBm_1 = getMaxDBMFromFreq(imd7_freq_1, span_Hz);
		float32_t imd7_dBm_2 = getMaxDBMFromFreq(imd7_freq_2, span_Hz);
		float32_t imd9_dBm_1 = getMaxDBMFromFreq(imd9_freq_1, span_Hz);
		float32_t imd9_dBm_2 = getMaxDBMFromFreq(imd9_freq_2, span_Hz);

		float32_t freq_dBm = MAX(freq_dBm_1, freq_dBm_2);
		FFT_Current_TX_SNR = freq_dBm - zero_dBm;
		FFT_Current_TX_IMD3 = MIN((freq_dBm - imd3_dBm_1), (freq_dBm - imd3_dBm_2));
		FFT_Current_TX_IMD5 = MIN((freq_dBm - imd5_dBm_1), (freq_dBm - imd5_dBm_2));
		FFT_Current_TX_IMD7 = MIN((freq_dBm - imd7_dBm_1), (freq_dBm - imd7_dBm_2));
		FFT_Current_TX_IMD9 = MIN((freq_dBm - imd9_dBm_1), (freq_dBm - imd9_dBm_2));

		// char ctmp[128] = {0};
		// sprintf(ctmp, "IMD3: %d IMD5: %d", (int32_t)imd3_dBm, (int32_t)imd5_dBm);
		// LCD_showTooltip(ctmp);

		// println("DBM: ", freq_dBm, " SNR: ", snr_dBm, " IMD3: ", FFT_Current_TX_IMD3, " IMD5: ", FFT_Current_TX_IMD5, " IMD7: ", imd7_dBm, " IMD9: ", imd9_dBm, " ");

		DPD_ProcessCalibration();
	}
	//

	FFT_need_fft = false;
}

// FFT output
bool FFT_printFFT(void) {
	if (LCD_busy) {
		return false;
	}
	if (!TRX.FFT_Enabled) {
		return false;
	}
	if (!TRX_Inited) {
		return false;
	}
	if (FFT_need_fft) {
		return false;
	}
	if (LCD_systemMenuOpened || LCD_window.opened) {
		if (TRX_on_TX && TRX_Tune && TRX.TWO_SIGNAL_TUNE && SHOW_RX_FFT_ON_TX) {
			FFT_need_fft = true;
			return true;
		}

		return false;
	}
	/*if (CPU_LOAD.Load > 90)
	  return;*/
	LCD_busy = true;

	uint16_t tmp = 0;
	uint16_t fftHeight = GET_FFTHeight;
	uint16_t wtfHeight = GET_WTFHeight;
	uint_fast8_t decoder_offset = 0;
	if (NeedProcessDecoder) {
		decoder_offset = LAYOUT->FFT_CWDECODER_OFFSET;
	}
	Hz_in_pixel = (TRX_on_TX && !SHOW_RX_FFT_ON_TX) ? FFT_TX_HZ_IN_PIXEL : FFT_HZ_IN_PIXEL;

	uint64_t centerFreq = CurrentVFO->SpectrumCenterFreq;
	if (TRX_on_TX && !SHOW_RX_FFT_ON_TX) {
		centerFreq = CurrentVFO->Freq;
	}

	if (centerFreq != currentFFTFreq || NeedWTFRedraw) {
		// calculate scale lines
		dma_memset(grid_lines_pos, 0x00, sizeof(grid_lines_pos));
		uint8_t index = 0;
		int64_t grid_step = FFT_current_spectrum_width_Hz / 9.6;
		if (grid_step < 1000) {
			grid_step = 1000;
		}
		grid_step = (grid_step / 1000) * 1000;

		for (int8_t i = 0; i < FFT_MAX_GRID_NUMBER; i++) {
			int64_t pos = -1;
			int64_t grid_freq = (centerFreq / grid_step * grid_step) + ((i - 6) * grid_step);
			pos = getFreqPositionOnFFT(grid_freq, false);
			if (pos >= 0) {
				grid_lines_pos[index] = pos;
				grid_lines_freq[index] = grid_freq;
				index++;
			}
		}

		// offset the fft if needed
		currentFFTFreq = centerFreq;
	}

	rx1_line_pos = getFreqPositionOnFFT(CurrentVFO->Freq, true);
	rx2_line_pos = getFreqPositionOnFFT(SecondaryVFO->Freq, true);

#if HRDW_HAS_MDMA
	// move the waterfall down using MDMA
	uint8_t *srcAddr = &indexed_wtf_buffer[wtfHeight - 2][LAYOUT->FFT_PRINT_SIZE - 1];
	uint8_t *destAddr = &indexed_wtf_buffer[wtfHeight - 1][LAYOUT->FFT_PRINT_SIZE - 1];
	uint8_t *endAddr = &indexed_wtf_buffer[0][0];
	uint32_t estimated = (uint32_t)srcAddr - (uint32_t)endAddr + 1;
	Aligned_CleanDCache_by_Addr(indexed_wtf_buffer, sizeof(indexed_wtf_buffer));
	while (estimated > 0) {
		uint32_t length = estimated;
		if (length > DMA_MAX_BLOCK) {
			length = DMA_MAX_BLOCK;
		}

		HAL_MDMA_Start(&HRDW_LCD_WTF_DOWN_MDMA, (uint32_t)srcAddr, (uint32_t)destAddr, length, 1);
		SLEEPING_MDMA_PollForTransfer(&HRDW_LCD_WTF_DOWN_MDMA);

		srcAddr -= length;
		destAddr -= length;
		estimated -= length;
	}
	Aligned_CleanInvalidateDCache_by_Addr(indexed_wtf_buffer, sizeof(indexed_wtf_buffer));
#else
	// move the waterfall down using DMA
	for (tmp = wtfHeight - 1; tmp > 0; tmp--) {
		HAL_DMA_Start(&hdma_memtomem_dma2_stream7, (uint32_t)&indexed_wtf_buffer[tmp - 1], (uint32_t)&indexed_wtf_buffer[tmp],
		              LAYOUT->FFT_PRINT_SIZE / 4); // 32bit dma, 8bit index data
		SLEEPING_DMA_PollForTransfer(&hdma_memtomem_dma2_stream7);
	}
#endif

	for (tmp = wtfHeight - 1; tmp > 0; tmp--) {
		wtf_buffer_freqs[tmp] = wtf_buffer_freqs[tmp - 1];
	}

	// Looking for the maximum/minimum in frequency response
	float32_t maxAmplValue = 0;
	float32_t minAmplValue = 0;
	arm_min_no_idx_f32(FFTOutput_mean, LAYOUT->FFT_PRINT_SIZE, &minAmplValue);
	if (FFT_SCALE_TYPE == 2) {
		for (uint_fast16_t i = 0; i < LAYOUT->FFT_PRINT_SIZE; i++) {
			if (FFTOutput_mean[i] == 0.0f) {
				FFTOutput_mean[i] = minAmplValue;
			}
		}
	}
	arm_max_no_idx_f32(FFTOutput_mean, LAYOUT->FFT_PRINT_SIZE, &maxAmplValue);
	// println(minAmplValue, " ", maxAmplValue, " ", maxValueFFT);

	// Looking for the median in frequency response
	dma_memcpy(FFTInput_tmp, FFTOutput_mean, LAYOUT->FFT_PRINT_SIZE * 4);
	float32_t medianValue = quick_median_select(FFTInput_tmp, LAYOUT->FFT_PRINT_SIZE);

	// FFT Targets
	float32_t FFT_Sensitivity = TRX.FFT_Sensitivity;
	if (CurrentVFO->Mode == TRX_MODE_WFM && !DFM_RX1.squelched) {
		FFT_Sensitivity /= 3.0f;
	}
	float32_t FFT_MIN = FFT_Sensitivity * 0.5f; // MIN threshold of FFT signal
	float32_t FFT_TARGET = FFT_Sensitivity;
	float32_t FFT_MAX = FFT_Sensitivity * 2.0f; // MAX FFT signal threshold

	float32_t maxValueFFT = maxValueFFT_rx;
	float32_t minValueFFT = maxValueFFT / (float32_t)fftHeight;
	if (TRX_on_TX) {
		maxValueFFT = maxValueFFT_tx;
	}
	float32_t maxValue = (medianValue * FFT_MAX);
	float32_t targetValue = (medianValue * FFT_TARGET);
	float32_t minValue = (medianValue * FFT_MIN);

	// dBm scaling
	if (FFT_SCALE_TYPE == 2) {
		if (TRX.FFT_Automatic_Type == FFT_AUTOMATIC_FULL || TRX.FFT_Automatic_Type == FFT_AUTOMATIC_HALF) {
			if (minAmplValue_averaged > minAmplValue) {
				minAmplValue_averaged = minAmplValue;
			} else {
				minAmplValue_averaged = minAmplValue_averaged * 0.99f + minAmplValue * 0.01f;
			}
		} else {
			minAmplValue_averaged = (float32_t)TRX.FFT_ManualBottom;
		}

		arm_offset_f32(FFTOutput_mean, -minAmplValue_averaged, FFTOutput_mean, LAYOUT->FFT_PRINT_SIZE);
		for (uint_fast16_t i = 0; i < LAYOUT->FFT_PRINT_SIZE; i++) {
			if (FFTOutput_mean[i] < 0) {
				FFTOutput_mean[i] = 0;
			}
		}
	}

	// Auto-calibrate FFT levels
	if (TRX_on_TX || (TRX.FFT_Automatic_Type == FFT_AUTOMATIC_FULL && TRX.FFT_Sensitivity == FFT_MAX_TOP_SCALE)) // Fit FFT to MAX
	{
		if (FFT_SCALE_TYPE == 2) {
			float32_t newMaxAmplValue = maxAmplValue - minAmplValue_averaged;
			maxValueFFT = maxValueFFT * 0.95f + newMaxAmplValue * 0.05f;
			if (maxValueFFT < newMaxAmplValue) {
				maxValueFFT = newMaxAmplValue;
			}
		} else {
			maxValueFFT = maxValueFFT * 0.95f + maxAmplValue * 0.05f;
			if (maxValueFFT < maxAmplValue) {
				maxValueFFT = maxAmplValue;
			}

			minValue = (medianValue * 6.0f);
			if (maxValueFFT < minValue) {
				maxValueFFT = minValue;
			}
		}

		FFT_minDBM = minAmplValue_averaged;
		FFT_maxDBM = maxAmplValue;
	} else if (TRX.FFT_Automatic_Type == FFT_AUTOMATIC_FULL) // Fit by median (automatic)
	{
		if (FFT_SCALE_TYPE == 2) {
			medianValue -= minAmplValue_averaged;
			if (medianValue < 1.0f) {
				medianValue = 1.0f;
			}
			maxValue = medianValue * FFT_MAX / 2.0f;
			targetValue = medianValue * FFT_TARGET / 2.0f;
			minValue = medianValue * FFT_MIN / 2.0f;
		}

		maxValueFFT += (targetValue - maxValueFFT) / FFT_STEP_COEFF;
		// println(maxValueFFT, " ", targetValue, " ", medianValue, " ", (medianValue / FFT_TARGET));

		// minimum-maximum threshold for median
		if (maxValueFFT < minValue) {
			maxValueFFT = minValue;
		}
		if (maxValueFFT > maxValue) {
			maxValueFFT = maxValue;
		}

		FFT_minDBM = minAmplValue_averaged;
		FFT_maxDBM = maxValueFFT + minAmplValue_averaged;

		// Compress peaks
		float32_t compressTargetValue = (maxValueFFT * FFT_COMPRESS_INTERVAL);
		float32_t compressSourceInterval = maxAmplValue - compressTargetValue;
		float32_t compressTargetInterval = maxValueFFT - compressTargetValue;
		float32_t compressRate = compressTargetInterval / compressSourceInterval;
		if (TRX.FFT_Compressor) {
			for (uint_fast16_t i = 0; i < LAYOUT->FFT_PRINT_SIZE; i++) {
				if (FFTOutput_mean[i] > compressTargetValue) {
					FFTOutput_mean[i] = compressTargetValue + ((FFTOutput_mean[i] - compressTargetValue) * compressRate);
				}
			}
		}
	} else // Manual Scale
	{
		if (FFT_SCALE_TYPE == 2) {
			float32_t minManualAmplitude = (float32_t)TRX.FFT_ManualBottom - minAmplValue_averaged;
			float32_t maxManualAmplitude = (float32_t)TRX.FFT_ManualTop - minAmplValue_averaged;
			maxValueFFT = maxManualAmplitude;
			minValueFFT = minManualAmplitude;

			FFT_minDBM = minValueFFT + minAmplValue_averaged;
			FFT_maxDBM = maxValueFFT + minAmplValue_averaged;
		} else {
			float32_t minManualAmplitude = getFFTAmplFromDB((float32_t)TRX.FFT_ManualBottom);
			float32_t maxManualAmplitude = getFFTAmplFromDB((float32_t)TRX.FFT_ManualTop);
			arm_offset_f32(FFTOutput_mean, -minManualAmplitude, FFTOutput_mean, LAYOUT->FFT_PRINT_SIZE);
			for (uint_fast16_t i = 0; i < LAYOUT->FFT_PRINT_SIZE; i++) {
				if (FFTOutput_mean[i] < 0) {
					FFTOutput_mean[i] = 0;
				}
			}
			maxValueFFT = maxManualAmplitude - minManualAmplitude;
			minValueFFT = minManualAmplitude;
		}
	}

	// tx noise scale limit
	if ((TRX_on_TX && !SHOW_RX_FFT_ON_TX) && maxValueFFT < FFT_TX_MIN_LEVEL) {
		maxValueFFT = FFT_TX_MIN_LEVEL;
	}

	// save values for TX/RX
	if (TRX_on_TX) {
		maxValueFFT_tx = maxValueFFT;
	} else {
		maxValueFFT_rx = maxValueFFT;
	}

	// scale fft mean buffer
	arm_scale_f32(FFTOutput_mean, (1.0f / maxValueFFT) * fftHeight, FFTOutput_mean, LAYOUT->FFT_PRINT_SIZE);

	// calculate the colors for the waterfall
	for (uint32_t fft_x = 0; fft_x < LAYOUT->FFT_PRINT_SIZE; fft_x++) {
		if (FFTOutput_mean[fft_x] > fftHeight) {
			FFTOutput_mean[fft_x] = fftHeight;
		}

		fft_header[fft_x] = FFTOutput_mean[fft_x];
		indexed_wtf_buffer[0][fft_x] = roundf((float32_t)fftHeight - FFTOutput_mean[fft_x]);
	}
	wtf_buffer_freqs[0] = currentFFTFreq;

#if HRDW_HAS_FULL_FFT_BUFFER
	// FFT Peaks
	if (TRX.FFT_HoldPeaks) {
		// peaks moving
		if (lastWTFFreq != currentFFTFreq) {
			float64_t diff = (float64_t)currentFFTFreq - (float64_t)lastWTFFreq;
			diff = diff / (float64_t)(Hz_in_pixel * fft_zoom);
			diff = roundl(diff);

			if (diff > 0) {
				for (int32_t fft_x = 0; fft_x < LAYOUT->FFT_PRINT_SIZE; fft_x++) {
					int32_t new_x = fft_x + (int32_t)diff;
					if (new_x >= 0 && new_x < LAYOUT->FFT_PRINT_SIZE) {
						fft_peaks[fft_x] = fft_peaks[new_x];
					} else {
						fft_peaks[fft_x] = 0;
					}
				}
			} else if (diff < 0) {
				for (int32_t fft_x = LAYOUT->FFT_PRINT_SIZE - 1; fft_x >= 0; fft_x--) {
					int32_t new_x = fft_x + (int32_t)diff;
					if (new_x >= 0 && new_x < LAYOUT->FFT_PRINT_SIZE) {
						fft_peaks[fft_x] = fft_peaks[new_x];
					} else {
						fft_peaks[fft_x] = 0;
					}
				}
			}
		}
		// peaks falling
		for (uint32_t fft_x = 0; fft_x < LAYOUT->FFT_PRINT_SIZE; fft_x++) {
			if (fft_peaks[fft_x] <= fft_header[fft_x]) {
				fft_peaks[fft_x] = fft_header[fft_x];
			} else {
				fft_peaks[fft_x]--;
			}
		}
	}
#endif

	// calculate bw bar size
	uint16_t cur_lpf_width = CurrentVFO->LPF_RX_Filter_Width;
	uint16_t cur_hpf_width = CurrentVFO->HPF_RX_Filter_Width;
	if (TRX_on_TX) {
		cur_lpf_width = CurrentVFO->LPF_TX_Filter_Width;
		cur_hpf_width = CurrentVFO->HPF_TX_Filter_Width;
		rx1_line_pos = LAYOUT->FFT_PRINT_SIZE / 2;
	}
	int32_t bw_rx1_line_width = 0;
	int32_t bw_rx2_line_width = 0;
	rx1_notch_line_pos = 100;
	rx2_notch_line_pos = 100;

	switch (CurrentVFO->Mode) {
	case TRX_MODE_LSB:
	case TRX_MODE_DIGI_L:
	case TRX_MODE_SAM_LSB:
		bw_rx1_line_width = (int32_t)((cur_lpf_width - cur_hpf_width) / Hz_in_pixel * fft_zoom);
		if (CurrentVFO->Mode == TRX_MODE_SAM_LSB) {
			bw_rx1_line_width /= 2;
		}
		if (bw_rx1_line_width > (LAYOUT->FFT_PRINT_SIZE / 2)) {
			bw_rx1_line_width = LAYOUT->FFT_PRINT_SIZE / 2;
		}
		bw_rx1_line_start = rx1_line_pos - bw_rx1_line_width - (cur_hpf_width / Hz_in_pixel * fft_zoom);
		bw_rx1_line_end = bw_rx1_line_start + bw_rx1_line_width;
		rx1_notch_line_pos = rx1_line_pos - (float32_t)CurrentVFO->NotchFC / Hz_in_pixel * fft_zoom;
		break;
	case TRX_MODE_USB:
	case TRX_MODE_RTTY:
	case TRX_MODE_DIGI_U:
	case TRX_MODE_SAM_USB:
		bw_rx1_line_width = (int32_t)((cur_lpf_width - cur_hpf_width) / Hz_in_pixel * fft_zoom);
		if (CurrentVFO->Mode == TRX_MODE_SAM_USB) {
			bw_rx1_line_width /= 2;
		}
		if (bw_rx1_line_width > (LAYOUT->FFT_PRINT_SIZE / 2)) {
			bw_rx1_line_width = LAYOUT->FFT_PRINT_SIZE / 2;
		}
		bw_rx1_line_start = rx1_line_pos + (cur_hpf_width / Hz_in_pixel * fft_zoom);
		bw_rx1_line_end = bw_rx1_line_start + bw_rx1_line_width;
		rx1_notch_line_pos = rx1_line_pos + CurrentVFO->NotchFC / Hz_in_pixel * fft_zoom;
		break;
	case TRX_MODE_NFM:
	case TRX_MODE_AM:
	case TRX_MODE_SAM_STEREO:
	case TRX_MODE_CW:
		bw_rx1_line_width = (int32_t)(cur_lpf_width / Hz_in_pixel * fft_zoom);
		if (bw_rx1_line_width > LAYOUT->FFT_PRINT_SIZE) {
			bw_rx1_line_width = LAYOUT->FFT_PRINT_SIZE;
		}
		bw_rx1_line_start = rx1_line_pos - (bw_rx1_line_width / 2);
		bw_rx1_line_end = bw_rx1_line_start + bw_rx1_line_width;
		if (CurrentVFO->Mode == TRX_MODE_CW) {
			rx1_notch_line_pos = rx1_line_pos - ((float32_t)TRX.CW_Pitch / Hz_in_pixel * fft_zoom) + (float32_t)CurrentVFO->NotchFC / Hz_in_pixel * fft_zoom;
		} else {
			rx1_notch_line_pos = rx1_line_pos + (float32_t)CurrentVFO->NotchFC / Hz_in_pixel * fft_zoom;
		}
		break;
	case TRX_MODE_WFM:
		bw_rx1_line_width = 0;
		bw_rx1_line_start = rx1_line_pos - (bw_rx1_line_width / 2);
		bw_rx1_line_end = bw_rx1_line_start + bw_rx1_line_width;
		rx1_notch_line_pos = rx1_line_pos + (float32_t)CurrentVFO->NotchFC / Hz_in_pixel * fft_zoom;
		break;
	default:
		break;
	}

	switch (SecondaryVFO->Mode) {
	case TRX_MODE_LSB:
	case TRX_MODE_DIGI_L:
	case TRX_MODE_SAM_LSB:
		bw_rx2_line_width = (int32_t)(SecondaryVFO->LPF_RX_Filter_Width / Hz_in_pixel * fft_zoom);
		if (SecondaryVFO->Mode == TRX_MODE_SAM_LSB) {
			bw_rx2_line_width /= 2;
		}
		if (bw_rx2_line_width > (LAYOUT->FFT_PRINT_SIZE / 2)) {
			bw_rx2_line_width = LAYOUT->FFT_PRINT_SIZE / 2;
		}
		bw_rx2_line_start = rx2_line_pos - bw_rx2_line_width;
		bw_rx2_line_end = bw_rx2_line_start + bw_rx2_line_width;
		rx2_notch_line_pos = rx2_line_pos - SecondaryVFO->NotchFC / Hz_in_pixel * fft_zoom;
		break;
	case TRX_MODE_USB:
	case TRX_MODE_RTTY:
	case TRX_MODE_DIGI_U:
	case TRX_MODE_SAM_USB:
		bw_rx2_line_width = (int32_t)(SecondaryVFO->LPF_RX_Filter_Width / Hz_in_pixel * fft_zoom);
		if (SecondaryVFO->Mode == TRX_MODE_SAM_USB) {
			bw_rx2_line_width /= 2;
		}
		if (bw_rx2_line_width > (LAYOUT->FFT_PRINT_SIZE / 2)) {
			bw_rx2_line_width = LAYOUT->FFT_PRINT_SIZE / 2;
		}
		bw_rx2_line_start = rx2_line_pos;
		bw_rx2_line_end = bw_rx2_line_start + bw_rx2_line_width;
		rx2_notch_line_pos = rx2_line_pos + (float32_t)SecondaryVFO->NotchFC / Hz_in_pixel * fft_zoom;
		break;
	case TRX_MODE_NFM:
	case TRX_MODE_AM:
	case TRX_MODE_SAM_STEREO:
	case TRX_MODE_CW:
		bw_rx2_line_width = (int32_t)(SecondaryVFO->LPF_RX_Filter_Width / Hz_in_pixel * fft_zoom);
		if (bw_rx2_line_width > LAYOUT->FFT_PRINT_SIZE) {
			bw_rx2_line_width = LAYOUT->FFT_PRINT_SIZE;
		}
		bw_rx2_line_start = rx2_line_pos - (bw_rx2_line_width / 2);
		bw_rx2_line_end = bw_rx2_line_start + bw_rx2_line_width;
		if (SecondaryVFO->Mode == TRX_MODE_CW) {
			rx2_notch_line_pos = rx2_line_pos - ((float32_t)TRX.CW_Pitch / Hz_in_pixel * fft_zoom) + (float32_t)SecondaryVFO->NotchFC / Hz_in_pixel * fft_zoom;
		} else {
			rx2_notch_line_pos = rx2_line_pos + (float32_t)SecondaryVFO->NotchFC / Hz_in_pixel * fft_zoom;
		}
		break;
	case TRX_MODE_WFM:
		bw_rx2_line_width = LAYOUT->FFT_PRINT_SIZE;
		bw_rx2_line_start = rx2_line_pos - (bw_rx2_line_width / 2);
		bw_rx2_line_end = bw_rx2_line_start + bw_rx2_line_width;
		rx2_notch_line_pos = rx2_line_pos + (float32_t)SecondaryVFO->NotchFC / Hz_in_pixel * fft_zoom;
		break;
	default:
		break;
	}
	bw_rx1_line_center = (bw_rx1_line_start + bw_rx1_line_end) / 2;
	bw_rx2_line_center = (bw_rx2_line_start + bw_rx2_line_end) / 2;

	if (TRX.FFT_Lens) // lens correction
	{
		bw_rx1_line_start = FFT_getLensCorrection(bw_rx1_line_start);
		bw_rx1_line_center = FFT_getLensCorrection(bw_rx1_line_center);
		bw_rx1_line_end = FFT_getLensCorrection(bw_rx1_line_end);

		bw_rx2_line_start = FFT_getLensCorrection(bw_rx2_line_start);
		bw_rx2_line_center = FFT_getLensCorrection(bw_rx2_line_center);
		bw_rx2_line_end = FFT_getLensCorrection(bw_rx2_line_end);
	}
	if (!FFT_Show_Sec_VFO) // disable RX2 bw show
	{
		bw_rx2_line_start = LCD_WIDTH + 10;
		bw_rx2_line_center = LCD_WIDTH + 10;
		bw_rx2_line_end = LCD_WIDTH + 10;
	}

#if HRDW_HAS_FULL_FFT_BUFFER
	// 3D View
	if (TRX.FFT_3D > 0) {
		FFT_3DPrintFFT();
		return true;
	}

	// prepare FFT print over the waterfall
	uint16_t background = BG_COLOR;
	uint16_t grid_color = palette_fft[fftHeight * 3 / 4];
	for (uint32_t fft_y = 0; fft_y < fftHeight; fft_y++) // Background
	{
		if (TRX.FFT_Background) {
			background = palette_bg_gradient[fft_y];
		}

		bool dBm_grid = false;
		if (TRX.FFT_dBmGrid) {
			for (uint16_t y = FFT_DBM_GRID_TOP_MARGIN; y <= fftHeight - 4; y += FFT_DBM_GRID_INTERVAL) {
				if (y == fft_y) {
					dBm_grid = true;
				}
			}
		}

		memset16(print_output_buffer[fft_y], dBm_grid ? grid_color : background, LAYOUT->FFT_PRINT_SIZE);

		uint16_t bw_color = dBm_grid ? grid_color : palette_bw_bg_colors[fft_y];

		if (FFT_Show_Sec_VFO && TRX.FFT_BW_Style != 3) {
			uint16_t sec_bw_color = dBm_grid ? grid_color : addColor(bw_color, -FFT_SEC_BW_BRIGHTNESS, FFT_SEC_BW_BRIGHTNESS, FFT_SEC_BW_BRIGHTNESS);

			for (int32_t fft_x = bw_rx2_line_start; fft_x <= bw_rx2_line_end; fft_x++) {
				if (fft_x < 0 || fft_x >= LAYOUT->FFT_PRINT_SIZE) {
					continue;
				}
				print_output_buffer[fft_y][fft_x] = sec_bw_color;
			}
		}

		for (int32_t fft_x = bw_rx1_line_start; fft_x <= bw_rx1_line_end; fft_x++) {
			if (fft_x < 0 || fft_x >= LAYOUT->FFT_PRINT_SIZE) {
				continue;
			}
			print_output_buffer[fft_y][fft_x] = bw_color;
		}
	}

	if (TRX.FFT_Style == 1) // gradient
	{
		for (uint32_t fft_x = 0; fft_x < LAYOUT->FFT_PRINT_SIZE; fft_x++) {
			if ((int32_t)fft_x >= bw_rx1_line_start && (int32_t)fft_x <= bw_rx1_line_end) { // bw rx1 bar
				for (uint32_t fft_y = (fftHeight - fft_header[fft_x]); fft_y < fftHeight; fft_y++) {
					print_output_buffer[fft_y][fft_x] = palette_bw_fft_colors[fft_y];
				}
			} else if (FFT_Show_Sec_VFO && TRX.FFT_BW_Style != 3 && (int32_t)fft_x >= bw_rx2_line_start && (int32_t)fft_x <= bw_rx2_line_end) { // bw rx2 bar
				for (uint32_t fft_y = (fftHeight - fft_header[fft_x]); fft_y < fftHeight; fft_y++) {
					print_output_buffer[fft_y][fft_x] = addColor(palette_bw_fft_colors[fft_y], -FFT_SEC_BW_BRIGHTNESS, FFT_SEC_BW_BRIGHTNESS, FFT_SEC_BW_BRIGHTNESS);
				}
			} else {
				for (uint32_t fft_y = (fftHeight - fft_header[fft_x]); fft_y < fftHeight; fft_y++) {
					print_output_buffer[fft_y][fft_x] = palette_fft[fft_y];
				}
			}
		}
	}

	if (TRX.FFT_Style == 2) // fill
	{
		uint16_t color_bw = palette_bw_fft_colors[fftHeight / 2];
		uint16_t color = palette_bw_fft_colors[fftHeight / 2];

		for (uint32_t fft_x = 0; fft_x < LAYOUT->FFT_PRINT_SIZE; fft_x++) {
			if ((int32_t)fft_x >= bw_rx1_line_start && (int32_t)fft_x <= bw_rx1_line_end) { // bw rx1 bar
				for (uint32_t fft_y = (fftHeight - fft_header[fft_x]); fft_y < fftHeight; fft_y++) {
					print_output_buffer[fft_y][fft_x] = color_bw;
				}
			} else if (FFT_Show_Sec_VFO && TRX.FFT_BW_Style != 3 && (int32_t)fft_x >= bw_rx2_line_start && (int32_t)fft_x <= bw_rx2_line_end) { // bw rx2 bar
				for (uint32_t fft_y = (fftHeight - fft_header[fft_x]); fft_y < fftHeight; fft_y++) {
					print_output_buffer[fft_y][fft_x] = addColor(color_bw, -FFT_SEC_BW_BRIGHTNESS, FFT_SEC_BW_BRIGHTNESS, FFT_SEC_BW_BRIGHTNESS);
				}
			} else {
				for (uint32_t fft_y = (fftHeight - fft_header[fft_x]); fft_y < fftHeight; fft_y++) {
					print_output_buffer[fft_y][fft_x] = color;
				}
			}
		}
	}

	if (TRX.FFT_Style == 3) // dots
	{
		uint16_t color_bw = palette_bw_fft_colors[fftHeight / 2];
		uint16_t color = palette_fft[fftHeight / 2];

		for (uint32_t fft_x = 0; fft_x < LAYOUT->FFT_PRINT_SIZE; fft_x++) {
			uint32_t fft_y = fftHeight - fft_header[fft_x];
			if ((int32_t)fft_x >= bw_rx1_line_start && (int32_t)fft_x <= bw_rx1_line_end) { // bw rx1 bar
				print_output_buffer[fft_y][fft_x] = color_bw;
			} else if (FFT_Show_Sec_VFO && TRX.FFT_BW_Style != 3 && (int32_t)fft_x >= bw_rx2_line_start && (int32_t)fft_x <= bw_rx2_line_end) { // bw rx2 bar
				print_output_buffer[fft_y][fft_x] = addColor(color_bw, -FFT_SEC_BW_BRIGHTNESS, FFT_SEC_BW_BRIGHTNESS, FFT_SEC_BW_BRIGHTNESS);
			} else {
				print_output_buffer[fft_y][fft_x] = palette_fft[fftHeight / 2];
			}
		}
	}

	if (TRX.FFT_Style == 4) // contour
	{
		uint16_t color_contour = palette_fft[fftHeight / 2];

		uint32_t fft_y_prev = 0;
		for (uint32_t fft_x = 0; fft_x < LAYOUT->FFT_PRINT_SIZE; fft_x++) {
			uint32_t fft_y = fftHeight - fft_header[fft_x];
			int32_t y_diff = (int32_t)fft_y - (int32_t)fft_y_prev;
			if (fft_x == 0 || (y_diff <= 1 && y_diff >= -1)) {
				print_output_buffer[fft_y][fft_x] = color_contour;
			} else {
				for (uint32_t l = 0; l < (abs(y_diff / 2) + 1); l++) // draw line
				{
					print_output_buffer[fft_y_prev + ((y_diff > 0) ? l : -l)][fft_x - 1] = color_contour;
					print_output_buffer[fft_y + ((y_diff > 0) ? -l : l)][fft_x] = color_contour;
				}
			}
			fft_y_prev = fft_y;
		}
	}

	if (TRX.FFT_Style == 5) // gradient + contour
	{
		uint32_t fft_y_prev = 0;
		for (uint32_t fft_x = 0; fft_x < LAYOUT->FFT_PRINT_SIZE; fft_x++) {
			if ((int32_t)fft_x >= bw_rx1_line_start && (int32_t)fft_x <= bw_rx1_line_end) { // bw rx1 bar
				for (uint32_t fft_y = (fftHeight - fft_header[fft_x]); fft_y < fftHeight; fft_y++) {
					print_output_buffer[fft_y][fft_x] = palette_bw_fft_colors[fft_y];
				}
			} else if (FFT_Show_Sec_VFO && TRX.FFT_BW_Style != 3 && (int32_t)fft_x >= bw_rx2_line_start && (int32_t)fft_x <= bw_rx2_line_end) { // bw rx2 bar
				for (uint32_t fft_y = (fftHeight - fft_header[fft_x]); fft_y < fftHeight; fft_y++) {
					print_output_buffer[fft_y][fft_x] = addColor(palette_bw_fft_colors[fft_y], -FFT_SEC_BW_BRIGHTNESS, FFT_SEC_BW_BRIGHTNESS, FFT_SEC_BW_BRIGHTNESS);
				}
			} else {
				for (uint32_t fft_y = (fftHeight - fft_header[fft_x]); fft_y < fftHeight; fft_y++) {
					print_output_buffer[fft_y][fft_x] = palette_fft[fft_y];
				}
			}

			uint16_t color_contour = palette_fft[fftHeight / 2];

			uint32_t fft_y = fftHeight - fft_header[fft_x];
			int32_t y_diff = (int32_t)fft_y - (int32_t)fft_y_prev;
			if (fft_x == 0 || (y_diff <= 1 && y_diff >= -1)) {
				print_output_buffer[fft_y][fft_x] = color_contour;
			} else {
				for (uint32_t l = 0; l < (abs(y_diff / 2) + 1); l++) // draw line
				{
					print_output_buffer[fft_y_prev + ((y_diff > 0) ? l : -l)][fft_x - 1] = color_contour;
					print_output_buffer[fft_y + ((y_diff > 0) ? -l : l)][fft_x] = color_contour;
				}
			}
			fft_y_prev = fft_y;
		}
	}

	// FFT Peaks
	if (TRX.FFT_HoldPeaks) {
		uint16_t color_contour = palette_fft[fftHeight / 2];

		uint32_t fft_y_prev = 0;
		for (uint32_t fft_x = 0; fft_x < LAYOUT->FFT_PRINT_SIZE; fft_x++) {
			uint32_t fft_y = fftHeight - fft_peaks[fft_x];
			int32_t y_diff = (int32_t)fft_y - (int32_t)fft_y_prev;
			if (fft_x == 0 || (y_diff <= 1 && y_diff >= -1)) {
				print_output_buffer[fft_y][fft_x] = color_contour;
			} else {
				for (uint32_t l = 0; l < (abs(y_diff / 2) + 1); l++) // draw line
				{
					print_output_buffer[fft_y_prev + ((y_diff > 0) ? l : -l)][fft_x - 1] = color_contour;
					print_output_buffer[fft_y + ((y_diff > 0) ? -l : l)][fft_x] = color_contour;
				}
			}
			fft_y_prev = fft_y;
		}
	}

	////Waterfall

	// clear old data
	if (lastWTFFreq != currentFFTFreq || NeedWTFRedraw) {
		uint16_t color = palette_wtf[fftHeight];
		if (TRX.FFT_Automatic_Type == FFT_AUTOMATIC_FULL || TRX.FFT_Automatic_Type == FFT_AUTOMATIC_HALF) {
			color = palette_wtf[(uint32_t)(fftHeight * 0.9f)];
		}
		memset16(print_output_buffer[fftHeight], color, LAYOUT->FFT_PRINT_SIZE * (wtfHeight - decoder_offset));
	}

	// BTE
	static uint8_t line_repeats_need = 1;
#ifdef HAS_BTE
	// move exist lines down with BTE
	if (lastWTFFreq == currentFFTFreq && !NeedWTFRedraw) {
		if (TRX.FFT_Speed <= 3) {
			// 1 line
			LCDDriver_BTE_copyArea(0, LAYOUT->FFT_FFTWTF_POS_Y + fftHeight, 0, LAYOUT->FFT_FFTWTF_POS_Y + fftHeight + 1, LAYOUT->FFT_PRINT_SIZE, (uint16_t)(wtfHeight - decoder_offset - 1), true);
			line_repeats_need = 1;
		}
		if (TRX.FFT_Speed == 4 && FFT_FPS_Last < 44) {
			// 2 line
			LCDDriver_BTE_copyArea(0, LAYOUT->FFT_FFTWTF_POS_Y + fftHeight, 0, LAYOUT->FFT_FFTWTF_POS_Y + fftHeight + 1, LAYOUT->FFT_PRINT_SIZE, (uint16_t)(wtfHeight - decoder_offset - 1), true);
			LCDDriver_BTE_copyArea(0, LAYOUT->FFT_FFTWTF_POS_Y + fftHeight, 0, LAYOUT->FFT_FFTWTF_POS_Y + fftHeight + 1, LAYOUT->FFT_PRINT_SIZE, (uint16_t)(wtfHeight - decoder_offset - 1), true);
			line_repeats_need = 2;
		}
		if (TRX.FFT_Speed == 5 && FFT_FPS_Last < 55) {
			// 3 line
			LCDDriver_BTE_copyArea(0, LAYOUT->FFT_FFTWTF_POS_Y + fftHeight, 0, LAYOUT->FFT_FFTWTF_POS_Y + fftHeight + 1, LAYOUT->FFT_PRINT_SIZE, (uint16_t)(wtfHeight - decoder_offset - 1), true);
			LCDDriver_BTE_copyArea(0, LAYOUT->FFT_FFTWTF_POS_Y + fftHeight, 0, LAYOUT->FFT_FFTWTF_POS_Y + fftHeight + 1, LAYOUT->FFT_PRINT_SIZE, (uint16_t)(wtfHeight - decoder_offset - 1), true);
			LCDDriver_BTE_copyArea(0, LAYOUT->FFT_FFTWTF_POS_Y + fftHeight, 0, LAYOUT->FFT_FFTWTF_POS_Y + fftHeight + 1, LAYOUT->FFT_PRINT_SIZE, (uint16_t)(wtfHeight - decoder_offset - 1), true);
			line_repeats_need = 3;
		}
	}
#endif

	// show BW on WTF
	bool showBWonWTF = TRX.FFT_BW_Position == 0;
	static bool redrawOnBwHide_state = false;
	if (TRX.FFT_BW_Position == 3) {
		if (TRX_Inactive_Time < 2) {
			showBWonWTF = true;
			redrawOnBwHide_state = false;
		} else {
			showBWonWTF = false;
			if (!redrawOnBwHide_state) {
				redrawOnBwHide_state = true;
				NeedWTFRedraw = true;
			}
		}
	}

	uint16_t wtf_printed_lines = 0;
	uint16_t print_wtf_yindex = fftHeight;
#ifdef HAS_BTE
	while ((print_wtf_yindex < (fftHeight + wtfHeight - decoder_offset) && (lastWTFFreq != currentFFTFreq || NeedWTFRedraw)) ||
	       (print_wtf_yindex == fftHeight && lastWTFFreq == currentFFTFreq && !NeedWTFRedraw))
#else
	while (print_wtf_yindex < (fftHeight + wtfHeight - decoder_offset))
#endif
	{
		uint16_t wtf_y_index = (print_wtf_yindex - fftHeight) / line_repeats_need;
		// calculate offset
		float64_t freq_diff = (((float64_t)currentFFTFreq - (float64_t)wtf_buffer_freqs[wtf_y_index]) / (double)Hz_in_pixel) * (float64_t)fft_zoom;
		// float64_t freq_diff_part = fmodl(freq_diff, 1.0);
		int64_t margin_left = 0;
		if (freq_diff < 0) {
			margin_left = -floorf(freq_diff);
		}
		if (margin_left > LAYOUT->FFT_PRINT_SIZE) {
			margin_left = LAYOUT->FFT_PRINT_SIZE;
		}
		int32_t margin_right = 0;
		if (freq_diff > 0) {
			margin_right = ceilf(freq_diff);
		}
		if (margin_right > LAYOUT->FFT_PRINT_SIZE) {
			margin_right = LAYOUT->FFT_PRINT_SIZE;
		}
		if ((margin_left + margin_right) > LAYOUT->FFT_PRINT_SIZE) {
			margin_right = 0;
		}
		// rounding
		int32_t body_width = LAYOUT->FFT_PRINT_SIZE - margin_left - margin_right;

		// skip WTF moving
		if (!TRX.WTF_Moving) {
			body_width = LAYOUT->FFT_PRINT_SIZE;
			margin_left = 0;
			margin_right = 0;
		}

		// printing
		if (body_width > 0) {
			if (margin_left == 0 && margin_right == 0) // print full line
			{
				for (uint32_t wtf_x = 0; wtf_x < LAYOUT->FFT_PRINT_SIZE; wtf_x++) {
					if (showBWonWTF && wtf_x >= bw_rx1_line_start && wtf_x <= bw_rx1_line_end) { // print rx1 bw bar
						print_output_buffer[print_wtf_yindex][wtf_x] = palette_bw_wtf_colors[indexed_wtf_buffer[wtf_y_index][wtf_x]];
					} else if (FFT_Show_Sec_VFO && showBWonWTF && TRX.FFT_BW_Style != 3 && (int32_t)wtf_x >= bw_rx2_line_start && (int32_t)wtf_x <= bw_rx2_line_end) { // print rx2 bw bar
						print_output_buffer[print_wtf_yindex][wtf_x] =
						    addColor(palette_bw_wtf_colors[indexed_wtf_buffer[wtf_y_index][wtf_x]], -FFT_SEC_BW_BRIGHTNESS, FFT_SEC_BW_BRIGHTNESS, FFT_SEC_BW_BRIGHTNESS);
					} else {
						print_output_buffer[print_wtf_yindex][wtf_x] = palette_wtf[indexed_wtf_buffer[wtf_y_index][wtf_x]];
					}
				}
			} else if (margin_left > 0) {
				for (uint32_t wtf_x = 0; wtf_x < (LAYOUT->FFT_PRINT_SIZE - margin_left); wtf_x++) {
					if (showBWonWTF && (margin_left + wtf_x) >= bw_rx1_line_start && (margin_left + wtf_x) <= bw_rx1_line_end) { // print rx1 bw bar
						print_output_buffer[print_wtf_yindex][margin_left + wtf_x] = palette_bw_wtf_colors[indexed_wtf_buffer[wtf_y_index][wtf_x]];
					} else if (FFT_Show_Sec_VFO && showBWonWTF && TRX.FFT_BW_Style != 3 && (int32_t)(margin_left + wtf_x) >= bw_rx2_line_start &&
					           (int32_t)(margin_left + wtf_x) <= bw_rx2_line_end) { // print rx2 bw bar
						print_output_buffer[print_wtf_yindex][margin_left + wtf_x] =
						    addColor(palette_bw_wtf_colors[indexed_wtf_buffer[wtf_y_index][wtf_x]], -FFT_SEC_BW_BRIGHTNESS, FFT_SEC_BW_BRIGHTNESS, FFT_SEC_BW_BRIGHTNESS);
					} else {
						print_output_buffer[print_wtf_yindex][margin_left + wtf_x] = palette_wtf[indexed_wtf_buffer[wtf_y_index][wtf_x]];
					}
				}
			}
			if (margin_right > 0) {
				for (uint32_t wtf_x = 0; wtf_x < (LAYOUT->FFT_PRINT_SIZE - margin_right); wtf_x++) {
					if (showBWonWTF && wtf_x >= bw_rx1_line_start && wtf_x <= bw_rx1_line_end) { // print rx1 bw bar
						print_output_buffer[print_wtf_yindex][wtf_x] = palette_bw_wtf_colors[indexed_wtf_buffer[wtf_y_index][wtf_x + margin_right]];
					} else if (FFT_Show_Sec_VFO && showBWonWTF && TRX.FFT_BW_Style != 3 && (int32_t)wtf_x >= bw_rx2_line_start && (int32_t)wtf_x <= bw_rx2_line_end) { // print rx2 bw bar
						print_output_buffer[print_wtf_yindex][wtf_x] =
						    addColor(palette_bw_wtf_colors[indexed_wtf_buffer[wtf_y_index][wtf_x + margin_right]], -FFT_SEC_BW_BRIGHTNESS, FFT_SEC_BW_BRIGHTNESS, FFT_SEC_BW_BRIGHTNESS);
					} else {
						print_output_buffer[print_wtf_yindex][wtf_x] = palette_wtf[indexed_wtf_buffer[wtf_y_index][wtf_x + margin_right]];
					}
				}
			}
		}

		print_wtf_yindex++;
		wtf_printed_lines++;
	}

	// Draw grids
	if (TRX.FFT_FreqGrid == 1 || TRX.FFT_FreqGrid == 2) {
		for (int32_t grid_line_index = 0; grid_line_index < FFT_MAX_GRID_NUMBER; grid_line_index++) {
			if (grid_lines_pos[grid_line_index] > 0 && grid_lines_pos[grid_line_index] < LAYOUT->FFT_PRINT_SIZE) {
				for (uint32_t fft_y = 0; fft_y < fftHeight; fft_y++) {
					print_output_buffer[fft_y][grid_lines_pos[grid_line_index]] = grid_color;
				}
			}
		}
	}
	if (TRX.FFT_FreqGrid >= 2) {
		for (int8_t grid_line_index = 0; grid_line_index < FFT_MAX_GRID_NUMBER; grid_line_index++) {
			if (grid_lines_pos[grid_line_index] > 0 && grid_lines_pos[grid_line_index] < LAYOUT->FFT_PRINT_SIZE) {
				for (uint32_t fft_y = fftHeight; fft_y < (fftHeight + wtfHeight); fft_y++) {
					print_output_buffer[fft_y][grid_lines_pos[grid_line_index]] = grid_color;
				}
			}
		}
	}

	uint16_t BWLinesHeight = (showBWonWTF || TRX.FFT_BW_Position == 1) ? FFT_AND_WTF_HEIGHT : fftHeight;

	// Gauss filter center
	if (TRX.CW_GaussFilter && CurrentVFO->Mode == TRX_MODE_CW && bw_rx1_line_center >= 0 && bw_rx1_line_center < LAYOUT->FFT_PRINT_SIZE) {
		uint16_t color = palette_fft[fftHeight / 2];
		for (uint32_t fft_y = 0; fft_y < BWLinesHeight; fft_y++) {
			print_output_buffer[fft_y][bw_rx1_line_center] = color;
		}
	}
	if (TRX.CW_GaussFilter && SecondaryVFO->Mode == TRX_MODE_CW && FFT_Show_Sec_VFO && bw_rx2_line_center >= 0 && bw_rx2_line_center < LAYOUT->FFT_PRINT_SIZE) {
		uint16_t color = palette_fft[fftHeight / 2];
		for (uint32_t fft_y = 0; fft_y < BWLinesHeight; fft_y++) {
			print_output_buffer[fft_y][bw_rx2_line_center] = color;
		}
	}

	// RTTY center frequency
	if (CurrentVFO->Mode == TRX_MODE_RTTY) {
		uint16_t color = palette_fft[fftHeight / 2];
		uint16_t x1 = rx1_line_pos + (TRX.RTTY_Freq - TRX.RTTY_Shift / 2) / Hz_in_pixel * fft_zoom;
		uint16_t x2 = rx1_line_pos + (TRX.RTTY_Freq + TRX.RTTY_Shift / 2) / Hz_in_pixel * fft_zoom;
		for (uint32_t fft_y = 0; fft_y < BWLinesHeight; fft_y++) {
			print_output_buffer[fft_y][x1] = color;
			print_output_buffer[fft_y][x2] = color;
		}
	}

	// Show manual Notch filter line
	if (CurrentVFO->ManualNotchFilter && TRX_on_RX && rx1_notch_line_pos >= 0 && rx1_notch_line_pos < LAYOUT->FFT_PRINT_SIZE) {
		uint16_t color = palette_fft[fftHeight * 1 / 4];
		for (uint32_t fft_y = 0; fft_y < BWLinesHeight; fft_y++) {
			print_output_buffer[fft_y][rx1_notch_line_pos] = color;
		}
	}
	if (SecondaryVFO->ManualNotchFilter && TRX_on_RX && FFT_Show_Sec_VFO && rx2_notch_line_pos >= 0 && rx2_notch_line_pos < LAYOUT->FFT_PRINT_SIZE) {
		uint16_t color = palette_fft[fftHeight * 1 / 4];
		for (uint32_t fft_y = 0; fft_y < BWLinesHeight; fft_y++) {
			print_output_buffer[fft_y][rx2_notch_line_pos] = color;
		}
	}

	// Draw RX2 center line
	if (FFT_Show_Sec_VFO && rx2_line_pos >= 0 && rx2_line_pos < LAYOUT->FFT_PRINT_SIZE) {
		for (uint32_t fft_y = 0; fft_y < BWLinesHeight; fft_y++) {
			print_output_buffer[fft_y][rx2_line_pos] = COLOR_GREEN;
		}
	}

	// Draw RX1 center line
	if (rx1_line_pos >= 0 && rx1_line_pos < LAYOUT->FFT_PRINT_SIZE) {
		uint16_t color = palette_fft[fftHeight / 2];
		for (uint32_t fft_y = 0; fft_y < BWLinesHeight; fft_y++) {
			print_output_buffer[fft_y][rx1_line_pos] = color;
		}
	}

	// Draw BW lines
	if (TRX.FFT_BW_Style == 3) {
		uint16_t color_bw = palette_fft[fftHeight / 2];
		uint16_t color_center = palette_fft[0];
		for (uint32_t fft_y = 0; fft_y < BWLinesHeight; fft_y++) {
			if (bw_rx1_line_start >= 0 && bw_rx1_line_start < LAYOUT->FFT_PRINT_SIZE) {
				print_output_buffer[fft_y][bw_rx1_line_start] = color_bw;
			}
			if (bw_rx1_line_end >= 0 && bw_rx1_line_end < LAYOUT->FFT_PRINT_SIZE) {
				print_output_buffer[fft_y][bw_rx1_line_end] = color_bw;
			}
			if (rx1_line_pos >= 0 && rx1_line_pos < LAYOUT->FFT_PRINT_SIZE) {
				print_output_buffer[fft_y][rx1_line_pos] = color_center;
			}
		}
	}

	FFT_clearMarkers();

// DXCluster labels
#if HRDW_HAS_WIFI
	if (TRX.FFT_DXCluster) {
		for (uint16_t i = 0; i < WIFI_DXCLUSTER_list_count; i++) {
			char str[64] = {0};
			strcat(str, WIFI_DXCLUSTER_list[i].Callsign);
			if (TRX.FFT_DXCluster_Azimuth) {
				sprintf(str, "%s %u^o", WIFI_DXCLUSTER_list[i].Callsign, WIFI_DXCLUSTER_list[i].Azimuth);
			}
			FFT_printMarker(WIFI_DXCLUSTER_list[i].Freq, str, 5, true, fftHeight);
		}
	}

	if (TRX.WOLF_Cluster) {
		for (uint16_t i = 0; i < WIFI_WOLFCLUSTER_list_count; i++) {
			char str[64] = "*";
			strcat(str, WIFI_WOLFCLUSTER_list[i].Callsign);
			if (TRX.FFT_DXCluster_Azimuth) {
				sprintf(str, "%s %u^o", WIFI_WOLFCLUSTER_list[i].Callsign, WIFI_WOLFCLUSTER_list[i].Azimuth);
			}
			FFT_printMarker(WIFI_WOLFCLUSTER_list[i].Freq, str, 5, true, fftHeight);
		}
	}
#endif

	// Time beacons
	for (uint16_t i = 0; i < TIME_BEACONS_COUNT; i++) {
		FFT_printMarker(TIME_BEACONS[i].frequency, (char *)TIME_BEACONS[i].name, 5, true, fftHeight);
	}

	// Memory channels
	for (uint16_t i = 0; i < MEMORY_CHANNELS_COUNT; i++) {
		if (CALIBRATE.MEMORY_CHANNELS[i].freq == 0) {
			continue;
		}
		FFT_printMarker(CALIBRATE.MEMORY_CHANNELS[i].freq, (char *)CALIBRATE.MEMORY_CHANNELS[i].name, 5, true, fftHeight);
	}

	// Print DBM grid (LOG Scale)
	if (TRX.FFT_dBmGrid && FFT_SCALE_TYPE < 2) {
		char tmp[64] = {0};
		float32_t ampl_on_bin = maxValueFFT / (float32_t)fftHeight;
		if (TRX.FFT_Automatic_Type != FFT_AUTOMATIC_FULL) {
			ampl_on_bin = (maxValueFFT + minValueFFT) / (float32_t)fftHeight;
		}

		for (uint16_t y = FFT_DBM_GRID_TOP_MARGIN; y <= fftHeight - 4; y += FFT_DBM_GRID_INTERVAL) {
			int16_t dBm = 0;
			if (TRX.FFT_Automatic_Type == FFT_AUTOMATIC_FULL) {
				dBm = getDBFromFFTAmpl(maxValueFFT - ampl_on_bin * (float32_t)y);
			} else {
				dBm = getDBFromFFTAmpl((maxValueFFT + minValueFFT) - ampl_on_bin * (float32_t)y);
			}
			if (dBm > 50) {
				continue;
			}
			sprintf(tmp, "%d", dBm);
			LCDDriver_printTextInMemory(tmp, 0, y - 4, FG_COLOR, BG_COLOR, 1, (uint16_t *)print_output_buffer, LAYOUT->FFT_PRINT_SIZE, FFT_AND_WTF_HEIGHT);
		}
	}

	// Print DBM grid (dBm Scale)
	if (TRX.FFT_dBmGrid && FFT_SCALE_TYPE == 2) {
		char tmp[64] = {0};
		float32_t dBm_on_bin = (FFT_maxDBM - FFT_minDBM) / (float32_t)fftHeight;
		// println(FFT_minDBM, " ", FFT_maxDBM);
		for (uint16_t y = FFT_DBM_GRID_TOP_MARGIN; y <= fftHeight - 4; y += FFT_DBM_GRID_INTERVAL) {
			int16_t dBm = FFT_maxDBM - dBm_on_bin * (float32_t)y;
			if (dBm > 50) {
				continue;
			}
			sprintf(tmp, "%d", dBm);
			LCDDriver_printTextInMemory(tmp, 0, y - 4, FG_COLOR, BG_COLOR, 1, (uint16_t *)print_output_buffer, LAYOUT->FFT_PRINT_SIZE, FFT_AND_WTF_HEIGHT);
		}
	}

	// Print AGC level
	if (CurrentVFO->AGC) {
		float32_t agc_level = AGC_SCREEN_currentGain / AGC_SCREEN_maxGain;
		uint32_t agc_height = fftHeight / 2 - 1;
		uint32_t agc_level_height = (float32_t)agc_height * agc_level;
		uint32_t agc_black_height = agc_height - agc_level_height;

		for (uint32_t fft_y = 0; fft_y < fftHeight / 2 - 5; fft_y++) {
			uint16_t color = COLOR_BLACK;
			if (fft_y > agc_black_height) {
				uint16_t red = fft_y * 255 / agc_height;
				color = rgb888torgb565(red, 255 - red, 0);
			}
			print_output_buffer[fft_y + 5][LAYOUT->FFT_PRINT_SIZE - 5] = color;
		}
	}

	// Print SAM Carrier offset
	if (CurrentVFO->Mode == TRX_MODE_SAM_STEREO || CurrentVFO->Mode == TRX_MODE_SAM_LSB || CurrentVFO->Mode == TRX_MODE_SAM_USB) {
		char tmp[32] = {0};
		sprintf(tmp, "%.2fHz", (double)SAM_Carrier_offset);
		LCDDriver_printTextInMemory(tmp, 5, 20, FG_COLOR, BG_COLOR, 1, (uint16_t *)print_output_buffer, LAYOUT->FFT_PRINT_SIZE, FFT_AND_WTF_HEIGHT);
	}

	// Print SAT info
	if (TRX.SatMode) {
		char tmp[128] = {0};
		sprintf(tmp, "SAT: %s, Az: %.1f El: %.1f", TRX.SAT_Name, SATTELITE.az, SATTELITE.el);
		LCDDriver_printTextInMemory(tmp, 15, 20, FG_COLOR, BG_COLOR, 1, (uint16_t *)print_output_buffer, LAYOUT->FFT_PRINT_SIZE, FFT_AND_WTF_HEIGHT);
	}

	// Print Stereo FM RDS Label
	// if (CurrentVFO->Mode == TRX_MODE_WFM && TRX.FM_Stereo && RDS_Stereo) {
	// LCDDriver_printTextInMemory("Stereo FM", 5, 20, FG_COLOR, BG_COLOR, 1, (uint16_t *)print_output_buffer, LAYOUT->FFT_PRINT_SIZE, FFT_AND_WTF_HEIGHT);
	// }

	// Init print 2D FFT+WTF
	Aligned_CleanDCache_by_Addr(print_output_buffer, sizeof(print_output_buffer));
	uint32_t fft_2d_print_height = fftHeight + wtf_printed_lines;
	LCDDriver_SetCursorAreaPosition(0, LAYOUT->FFT_FFTWTF_POS_Y, LAYOUT->FFT_PRINT_SIZE - 1, LAYOUT->FFT_FFTWTF_POS_Y + fft_2d_print_height - 1);
	print_fft_dma_estimated_size = LAYOUT->FFT_PRINT_SIZE * fft_2d_print_height;
	print_fft_dma_position = 0;

	FFT_afterPrintFFT();

#else

	// Short buffer version
	LCDDriver_SetCursorAreaPosition(0, LAYOUT->FFT_FFTWTF_POS_Y, 0, LAYOUT->FFT_FFTWTF_POS_Y);
	HAL_DMA_Start_IT(&HRDW_LCD_FSMC_COPY_DMA, (uint32_t)&print_output_short_buffer[0][0], LCD_FSMC_DATA_ADDR, 1);

#endif

	return true;
}

static void FFT_clearMarkers() {
	FFT_Markers_Index = 0;
	dma_memset(FFT_Markers, 0x00, sizeof(FFT_Markers));
}

static void FFT_printMarker(uint64_t frequency, char *label, uint16_t baseline, bool direction, uint32_t max_height) {

	int32_t x = getFreqPositionOnFFT(frequency, true);
	if (x >= -50 && x < LAYOUT->FFT_PRINT_SIZE) {
		uint16_t y = baseline;
		uint16_t width = strlen(label) * 6 + 3;

		for (int16_t index = 0; index < FFT_Markers_Index; index++) {
			bool a = x <= (FFT_Markers[index].x + FFT_Markers[index].width);
			bool b = (x + width) >= FFT_Markers[index].x;
			bool c = y == FFT_Markers[index].y;

			if (a && b && c) {
				y += direction ? 10 : -10;
				index = -1; // repeat find circle
			}
		}

		if (y > 0 && y < (max_height - 10)) {
			FFT_Markers[FFT_Markers_Index].x = x;
			FFT_Markers[FFT_Markers_Index].y = y;
			FFT_Markers[FFT_Markers_Index].width = width;
			FFT_Markers_Index++;

#if HRDW_HAS_FULL_FFT_BUFFER
			LCDDriver_printTextInMemory(label, x + 2, y, FG_COLOR, BG_COLOR, 1, (uint16_t *)print_output_buffer, LAYOUT->FFT_PRINT_SIZE, FFT_AND_WTF_HEIGHT);
#else
			LCDDriver_printTextInMemory(label, x + 2, y, FG_COLOR, BG_COLOR, 1, (uint16_t *)print_output_short_buffer, LAYOUT->FFT_PRINT_SIZE, FFT_AND_WTF_HEIGHT);
#endif

			// vertical line
			if (x >= 0) {
				for (uint8_t y_line = 0; y_line < 8; y_line++) {
#if HRDW_HAS_FULL_FFT_BUFFER
					print_output_buffer[y + y_line][x] = COLOR_RED;
#else
					print_output_short_buffer[y + y_line][x] = COLOR_RED;
#endif
				}
			}
		}
	}
}

#if !HRDW_HAS_FULL_FFT_BUFFER
void FFT_ShortBufferPrintFFT(void) {
	// Short buffer version
	uint32_t fftHeight = GET_FFTHeight;
	uint32_t wtfHeight = GET_WTFHeight;
	uint_fast8_t decoder_offset = 0;
	if (NeedProcessDecoder) {
		decoder_offset = LAYOUT->FFT_CWDECODER_OFFSET;
	}
	uint16_t grid_color = palette_fft[fftHeight * 3 / 4];
	uint16_t contour_color = palette_fft[fftHeight / 2];
	static uint32_t fft_output_printed = 0;
	static uint32_t fft_output_prepared = 0;

	while (fft_output_printed < (fftHeight + wtfHeight - decoder_offset)) {
		fft_output_prepared = 0;

		for (uint32_t buff_idx = 0; buff_idx < FFT_SHORT_BUFFER_SIZE; buff_idx++) {
			uint32_t fft_y = fft_output_printed + fft_output_prepared;
			if (fft_y >= (fftHeight + wtfHeight - decoder_offset)) {
				break;
			}

			if (fft_y < fftHeight) // FFT PART
			{
				// Background and dBm grid
				uint16_t background = BG_COLOR;

				if (TRX.FFT_Background) {
					background = palette_bg_gradient[fft_y];
				}

				for (uint32_t fft_x = 0; fft_x < LAYOUT->FFT_PRINT_SIZE; fft_x++) {
					print_output_short_buffer[buff_idx][fft_x] = background;
				}

				if (TRX.FFT_Style == 1) // gradient
				{
					uint16_t gradient_color = palette_fft[fft_y];
					for (uint32_t fft_x = 0; fft_x < LAYOUT->FFT_PRINT_SIZE; fft_x++) {
						if (fft_y >= (fftHeight - fft_header[fft_x])) {
							print_output_short_buffer[buff_idx][fft_x] = gradient_color;
						}
					}
				}

				if (TRX.FFT_Style == 2) // fill
				{
					for (uint32_t fft_x = 0; fft_x < LAYOUT->FFT_PRINT_SIZE; fft_x++) {
						if (fft_y >= (fftHeight - fft_header[fft_x])) {
							print_output_short_buffer[buff_idx][fft_x] = contour_color;
						}
					}
				}

				if (TRX.FFT_Style == 3) // dots
				{
					for (uint32_t fft_x = 0; fft_x < LAYOUT->FFT_PRINT_SIZE; fft_x++) {
						if (fft_y == (fftHeight - fft_header[fft_x])) {
							print_output_short_buffer[buff_idx][fft_x] = contour_color;
						}
					}
				}

				if (TRX.FFT_Style == 4) // contour
				{
					for (uint32_t fft_x = 0; fft_x < LAYOUT->FFT_PRINT_SIZE; fft_x++) {
						uint32_t current_signal_y = fftHeight - fft_header[fft_x];

						// dot on top
						if (fft_y == current_signal_y) {
							print_output_short_buffer[buff_idx][fft_x] = contour_color;
						}

						// contour
						if (fft_x > 0 && fft_x < (LAYOUT->FFT_PRINT_SIZE - 1) && fft_y >= current_signal_y) {
							uint32_t left_signal_y = fftHeight - fft_header[fft_x - 1];
							uint32_t right_signal_y = fftHeight - fft_header[fft_x + 1];

							if (current_signal_y <= right_signal_y && fft_y <= right_signal_y) {
								print_output_short_buffer[buff_idx][fft_x] = contour_color;
							}
							if (current_signal_y <= left_signal_y && fft_y <= left_signal_y) {
								print_output_short_buffer[buff_idx][fft_x] = contour_color;
							}
						}
					}
				}

				if (TRX.FFT_Style == 5) // gradient + contour
				{
					uint16_t gradient_color = palette_fft[fft_y];

					for (uint32_t fft_x = 0; fft_x < LAYOUT->FFT_PRINT_SIZE; fft_x++) {
						uint32_t current_signal_y = fftHeight - fft_header[fft_x];

						// gradient
						if (fft_y > current_signal_y) {
							print_output_short_buffer[buff_idx][fft_x] = gradient_color;
						}
						// dot on top
						if (fft_y == current_signal_y) {
							print_output_short_buffer[buff_idx][fft_x] = contour_color;
						}
						// contour
						if (fft_x > 0 && fft_x < (LAYOUT->FFT_PRINT_SIZE - 1) && fft_y >= current_signal_y) {
							uint32_t left_signal_y = fftHeight - fft_header[fft_x - 1];
							uint32_t right_signal_y = fftHeight - fft_header[fft_x + 1];

							if (current_signal_y <= right_signal_y && fft_y <= right_signal_y) {
								print_output_short_buffer[buff_idx][fft_x] = contour_color;
							}
							if (current_signal_y <= left_signal_y && fft_y <= left_signal_y) {
								print_output_short_buffer[buff_idx][fft_x] = contour_color;
							}
						}
					}
				}
			}

			// PRINT WATERFALL
			if (fft_y >= fftHeight) {
				uint16_t wtf_y_index = fft_y - fftHeight;

				// calculate offset
				float32_t freq_diff = (((float32_t)currentFFTFreq - (float32_t)wtf_buffer_freqs[wtf_y_index]) / Hz_in_pixel) * (float32_t)fft_zoom;
				// float32_t freq_diff_part = fmodl((float64_t)freq_diff, 1.0);
				int32_t margin_left = 0;
				if (freq_diff < 0) {
					margin_left = -floorf(freq_diff);
				}
				if (margin_left > LAYOUT->FFT_PRINT_SIZE) {
					margin_left = LAYOUT->FFT_PRINT_SIZE;
				}
				int32_t margin_right = 0;
				if (freq_diff > 0) {
					margin_right = ceilf(freq_diff);
				}
				if (margin_right > LAYOUT->FFT_PRINT_SIZE) {
					margin_right = LAYOUT->FFT_PRINT_SIZE;
				}
				if ((margin_left + margin_right) > LAYOUT->FFT_PRINT_SIZE) {
					margin_right = 0;
				}
				// rounding
				int32_t body_width = LAYOUT->FFT_PRINT_SIZE - margin_left - margin_right;

				// skip WTF moving
				if (!TRX.WTF_Moving) {
					body_width = LAYOUT->FFT_PRINT_SIZE;
					margin_left = 0;
					margin_right = 0;
				}

				if (body_width != LAYOUT->FFT_PRINT_SIZE) {
					uint16_t color = palette_wtf[(uint32_t)(GET_FFTHeight * 0.9f)];
					memset16(print_output_short_buffer[buff_idx], color, sizeof(print_output_short_buffer[buff_idx]) / 2);
				}

				// printing
				if (body_width > 0) {
					if (margin_left == 0 && margin_right == 0) // print full line
					{
						for (uint32_t wtf_x = 0; wtf_x < LAYOUT->FFT_PRINT_SIZE; wtf_x++) {
							print_output_short_buffer[buff_idx][wtf_x] = palette_wtf[indexed_wtf_buffer[wtf_y_index][wtf_x]];
						}
					} else if (margin_left > 0) {
						for (uint32_t wtf_x = 0; wtf_x < (LAYOUT->FFT_PRINT_SIZE - margin_left); wtf_x++) {
							print_output_short_buffer[buff_idx][margin_left + wtf_x] = palette_wtf[indexed_wtf_buffer[wtf_y_index][wtf_x]];
						}
					}
					if (margin_right > 0) {
						for (uint32_t wtf_x = 0; wtf_x < (LAYOUT->FFT_PRINT_SIZE - margin_right); wtf_x++) {
							print_output_short_buffer[buff_idx][wtf_x] = palette_wtf[indexed_wtf_buffer[wtf_y_index][wtf_x + margin_right]];
						}
					}
				}
			}
			//////////////////

			// Draw grids
			if (TRX.FFT_FreqGrid == 1 || TRX.FFT_FreqGrid == 2) {
				if (fft_y < fftHeight) {
					for (int32_t grid_line_index = 0; grid_line_index < FFT_MAX_GRID_NUMBER; grid_line_index++) {
						if (grid_lines_pos[grid_line_index] > 0 && grid_lines_pos[grid_line_index] < LAYOUT->FFT_PRINT_SIZE && grid_lines_pos[grid_line_index] != (LAYOUT->FFT_PRINT_SIZE / 2)) {
							print_output_short_buffer[buff_idx][grid_lines_pos[grid_line_index]] = grid_color;
						}
					}
				}
			}

			if (TRX.FFT_FreqGrid >= 2) {
				if (fft_y >= fftHeight) {
					for (int32_t grid_line_index = 0; grid_line_index < FFT_MAX_GRID_NUMBER; grid_line_index++) {
						if (grid_lines_pos[grid_line_index] > 0 && grid_lines_pos[grid_line_index] < LAYOUT->FFT_PRINT_SIZE && grid_lines_pos[grid_line_index] != (LAYOUT->FFT_PRINT_SIZE / 2)) {
							print_output_short_buffer[buff_idx][grid_lines_pos[grid_line_index]] = grid_color;
						}
					}
				}
			}

			// Gauss filter center
			if (TRX.CW_GaussFilter && CurrentVFO->Mode == TRX_MODE_CW && bw_rx1_line_center >= 0 && bw_rx1_line_center < LAYOUT->FFT_PRINT_SIZE) {
				print_output_short_buffer[buff_idx][bw_rx1_line_center] = contour_color;
			}

			// RTTY center frequency
			if (CurrentVFO->Mode == TRX_MODE_RTTY) {
				uint16_t x1 = rx1_line_pos + (TRX.RTTY_Freq - TRX.RTTY_Shift / 2) / Hz_in_pixel * fft_zoom;
				uint16_t x2 = rx1_line_pos + (TRX.RTTY_Freq + TRX.RTTY_Shift / 2) / Hz_in_pixel * fft_zoom;
				if (x1 >= 0 && x1 < LAYOUT->FFT_PRINT_SIZE) {
					print_output_short_buffer[buff_idx][x1] = contour_color;
				}
				if (x2 >= 0 && x2 < LAYOUT->FFT_PRINT_SIZE) {
					print_output_short_buffer[buff_idx][x2] = contour_color;
				}
			}

			// Show manual Notch filter line
			if (CurrentVFO->ManualNotchFilter && TRX_on_RX && rx1_notch_line_pos >= 0 && rx1_notch_line_pos < LAYOUT->FFT_PRINT_SIZE) {
				uint16_t color = palette_fft[fftHeight * 1 / 4];
				print_output_short_buffer[buff_idx][rx1_notch_line_pos] = color;
			}

			// Draw RX1 center line
			if (rx1_line_pos >= 0 && rx1_line_pos < LAYOUT->FFT_PRINT_SIZE) {
				print_output_short_buffer[buff_idx][rx1_line_pos] = contour_color;
			}

			// Draw BW lines
			uint16_t color_center = palette_fft[0];
			if (bw_rx1_line_start >= 0 && bw_rx1_line_start < LAYOUT->FFT_PRINT_SIZE) {
				print_output_short_buffer[buff_idx][bw_rx1_line_start] = contour_color;
			}
			if (bw_rx1_line_end >= 0 && bw_rx1_line_end < LAYOUT->FFT_PRINT_SIZE) {
				print_output_short_buffer[buff_idx][bw_rx1_line_end] = contour_color;
			}
			if (rx1_line_pos >= 0 && rx1_line_pos < LAYOUT->FFT_PRINT_SIZE) {
				print_output_short_buffer[buff_idx][rx1_line_pos] = color_center;
			}

			// well done
			fft_output_prepared++;

			if (fft_output_prepared == FFT_SHORT_BUFFER_SIZE && fft_output_printed == 0) { // last line, first block
				FFT_clearMarkers();

				// Time beacons
				for (uint16_t i = 0; i < TIME_BEACONS_COUNT; i++) {
					FFT_printMarker(TIME_BEACONS[i].frequency, (char *)TIME_BEACONS[i].name, 5, true, FFT_SHORT_BUFFER_SIZE);
				}

				// Memory channels
				for (uint16_t i = 0; i < MEMORY_CHANNELS_COUNT; i++) {
					if (CALIBRATE.MEMORY_CHANNELS[i].freq == 0) {
						continue;
					}
					FFT_printMarker(CALIBRATE.MEMORY_CHANNELS[i].freq, (char *)CALIBRATE.MEMORY_CHANNELS[i].name, 5, true, FFT_SHORT_BUFFER_SIZE);
				}

				// SAM Carrier
				if (CurrentVFO->Mode == TRX_MODE_SAM_STEREO || CurrentVFO->Mode == TRX_MODE_SAM_LSB || CurrentVFO->Mode == TRX_MODE_SAM_USB) {
					char tmp[32] = {0};
					sprintf(tmp, "%.2fHz", (double)SAM_Carrier_offset);
					LCDDriver_printTextInMemory(tmp, 5, 5, FG_COLOR, BG_COLOR, 1, (uint16_t *)print_output_short_buffer, LAYOUT->FFT_PRINT_SIZE, FFT_SHORT_BUFFER_SIZE);
				}
			}
		}

		// Lets print line to LCD
#ifdef STM32H743xx
		Aligned_CleanDCache_by_Addr(print_output_short_buffer, sizeof(print_output_short_buffer));
#endif
		LCDDriver_SetCursorAreaPosition(0, LAYOUT->FFT_FFTWTF_POS_Y + fft_output_printed, LAYOUT->FFT_PRINT_SIZE - 1, LAYOUT->FFT_FFTWTF_POS_Y + fft_output_printed + fft_output_prepared);
		HAL_DMA_Start_IT(&HRDW_LCD_FSMC_COPY_DMA, (uint32_t)&print_output_short_buffer[0][0], LCD_FSMC_DATA_ADDR, LAYOUT->FFT_PRINT_SIZE * fft_output_prepared);

		fft_output_printed += fft_output_prepared;
		return;
	}

	fft_output_printed = 0;
	fft_output_prepared = 0;

	FFT_afterPrintFFT();
}
#endif

#if HRDW_HAS_FULL_FFT_BUFFER
// 3D mode print
static void FFT_3DPrintFFT(void) {
	uint16_t wtfHeight = GET_WTFHeight;
	uint16_t fftHeight = GET_FFTHeight;
	uint_fast8_t decoder_offset = 0;
	if (NeedProcessDecoder) {
		decoder_offset = LAYOUT->FFT_CWDECODER_OFFSET;
	}

	// clear old data
	dma_memset(print_output_buffer, 0, sizeof(print_output_buffer));

	// draw 3D WTF
	for (int32_t wtf_yindex = 0; wtf_yindex <= FFT_3D_SLIDES; wtf_yindex++) // each slides
	{
		// calc perspective parameters
		uint32_t print_y = fftHeight + wtfHeight - decoder_offset - wtf_yindex * FFT_3D_Y_OFFSET;
		float32_t x_compress = (float32_t)(LAYOUT->FFT_PRINT_SIZE - FFT_3D_X_OFFSET * wtf_yindex) / (float32_t)LAYOUT->FFT_PRINT_SIZE;
		uint32_t x_left_offset = (uint32_t)roundf(((float32_t)LAYOUT->FFT_PRINT_SIZE - (float32_t)LAYOUT->FFT_PRINT_SIZE * x_compress) / 2.0f);
		int16_t prev_x = -1;

		// each bin
		for (uint32_t wtf_x = 0; wtf_x < LAYOUT->FFT_PRINT_SIZE; wtf_x++) {
			// calc bin perspective
			uint32_t print_bin_height = print_y - (fftHeight - indexed_wtf_buffer[wtf_yindex][wtf_x]);
			if (print_bin_height > wtfHeight + fftHeight - decoder_offset) {
				continue;
			}
			if (print_bin_height >= FFT_AND_WTF_HEIGHT) {
				continue;
			}
			uint32_t print_x = x_left_offset + (uint32_t)roundf((float32_t)wtf_x * x_compress);
			if (prev_x == print_x || print_x >= LAYOUT->FFT_PRINT_SIZE) {
				continue;
			}
			prev_x = print_x;

			uint16_t bg_color = palette_wtf[fftHeight];

			if (TRX.FFT_3D == 1) // line mode
			{
				int32_t line_max = fftHeight - indexed_wtf_buffer[wtf_yindex][wtf_x] - 1;
				if ((print_bin_height + line_max) >= FFT_AND_WTF_HEIGHT) {
					line_max = FFT_AND_WTF_HEIGHT - print_bin_height - 1;
				}

				for (uint16_t h = 0; h < line_max; h++) {
					uint32_t buff_y = print_bin_height + h;
					if (print_output_buffer[buff_y][print_x] != bg_color) {
						break;
					}
					print_output_buffer[buff_y][print_x] = palette_wtf[indexed_wtf_buffer[wtf_yindex][wtf_x] + h];
				}
			}
			if (TRX.FFT_3D == 2) { // pixel mode
				if (print_output_buffer[print_bin_height][print_x] == bg_color) {
					print_output_buffer[print_bin_height][print_x] = palette_wtf[indexed_wtf_buffer[wtf_yindex][wtf_x]];
				}
			}
		}
	}

	// draw front fft
	for (uint32_t fft_y = 0; fft_y < fftHeight; fft_y++) {
		int32_t buff_y = wtfHeight - decoder_offset + fft_y;
		for (int32_t fft_x = 0; fft_x < LAYOUT->FFT_PRINT_SIZE; fft_x++) {
			// bg
			if (fft_y > (fftHeight - fft_header[fft_x])) {
				if (fft_x >= bw_rx1_line_start && fft_x <= bw_rx1_line_end) { // rx1
					print_output_buffer[buff_y][fft_x] = palette_bw_fft_colors[fft_y];
				} else if (FFT_Show_Sec_VFO && TRX.FFT_BW_Style != 3 && fft_x >= bw_rx2_line_start && (int32_t)fft_x <= bw_rx2_line_end) { // rx2
					print_output_buffer[buff_y][fft_x] = addColor(palette_bw_fft_colors[fft_y], -FFT_SEC_BW_BRIGHTNESS, FFT_SEC_BW_BRIGHTNESS, FFT_SEC_BW_BRIGHTNESS);
				} else {
					print_output_buffer[buff_y][fft_x] = palette_fft[fft_y];
				}
			}
		}
	}

	// draw contour
	int32_t buff_y_prev = 0;
	uint16_t color = palette_fft[fftHeight / 2];
	for (uint32_t fft_x = 0; fft_x < LAYOUT->FFT_PRINT_SIZE; fft_x++) {
		int32_t fft_y = fftHeight - fft_header[fft_x];
		int32_t buff_y = wtfHeight - decoder_offset - 1 + fft_y;
		int32_t y_diff = buff_y - buff_y_prev;
		if (fft_x == 0 || (y_diff <= 1 && y_diff >= -1)) {
			print_output_buffer[buff_y][fft_x] = color;
		} else {
			for (uint32_t l = 0; l < (abs(y_diff / 2) + 1); l++) // draw line
			{
				print_output_buffer[buff_y_prev + ((y_diff > 0) ? l : -l)][fft_x - 1] = color;
				print_output_buffer[buff_y + ((y_diff > 0) ? -l : l)][fft_x] = color;
			}
		}
		buff_y_prev = buff_y;
	}

	FFT_clearMarkers();

// DXCluster labels
#if HRDW_HAS_WIFI
	int32_t prev_pos = -999;
	int32_t prev_w = 0;
	uint16_t prev_y = FFT_AND_WTF_HEIGHT - 50;

	if (TRX.FFT_DXCluster) {
		for (uint16_t i = 0; i < WIFI_DXCLUSTER_list_count; i++) {
			char str[64] = {0};
			strcat(str, WIFI_DXCLUSTER_list[i].Callsign);
			if (TRX.FFT_DXCluster_Azimuth) {
				sprintf(str, "%s %u^o", WIFI_DXCLUSTER_list[i].Callsign, WIFI_DXCLUSTER_list[i].Azimuth);
			}
			FFT_printMarker(WIFI_DXCLUSTER_list[i].Freq, str, FFT_AND_WTF_HEIGHT - 50, false, FFT_AND_WTF_HEIGHT);
		}
	}

	if (TRX.WOLF_Cluster) {
		for (uint16_t i = 0; i < WIFI_WOLFCLUSTER_list_count; i++) {
			char str[64] = "*";
			strcat(str, WIFI_WOLFCLUSTER_list[i].Callsign);
			if (TRX.FFT_DXCluster_Azimuth) {
				sprintf(str, "%s %u^o", WIFI_WOLFCLUSTER_list[i].Callsign, WIFI_WOLFCLUSTER_list[i].Azimuth);
			}
			FFT_printMarker(WIFI_WOLFCLUSTER_list[i].Freq, str, FFT_AND_WTF_HEIGHT - 50, false, FFT_AND_WTF_HEIGHT);
		}
	}
#endif

	// Time beacons
	for (uint16_t i = 0; i < TIME_BEACONS_COUNT; i++) {
		FFT_printMarker(TIME_BEACONS[i].frequency, (char *)TIME_BEACONS[i].name, FFT_AND_WTF_HEIGHT - 50, false, FFT_AND_WTF_HEIGHT);
	}

	// Memory channels
	for (uint16_t i = 0; i < MEMORY_CHANNELS_COUNT; i++) {
		if (CALIBRATE.MEMORY_CHANNELS[i].freq == 0) {
			continue;
		}
		FFT_printMarker(CALIBRATE.MEMORY_CHANNELS[i].freq, (char *)CALIBRATE.MEMORY_CHANNELS[i].name, FFT_AND_WTF_HEIGHT - 50, false, FFT_AND_WTF_HEIGHT);
	}

	// Сenter line
	if (rx1_line_pos >= 0 && rx1_line_pos < LAYOUT->FFT_PRINT_SIZE) {
		for (uint32_t fft_y = 0; fft_y < FFT_AND_WTF_HEIGHT; fft_y++) {
			print_output_buffer[fft_y][rx1_line_pos] = palette_fft[fftHeight / 2];
		}
	}

	// Init print 3D FFT
	Aligned_CleanDCache_by_Addr(print_output_buffer, sizeof(print_output_buffer));
	uint32_t fft_3d_print_height = fftHeight + (uint16_t)(wtfHeight - decoder_offset) - 1;
	LCDDriver_SetCursorAreaPosition(0, LAYOUT->FFT_FFTWTF_POS_Y, LAYOUT->FFT_PRINT_SIZE - 1, LAYOUT->FFT_FFTWTF_POS_Y + fft_3d_print_height);
	print_fft_dma_estimated_size = LAYOUT->FFT_PRINT_SIZE * fft_3d_print_height;
	print_fft_dma_position = 0;

	// do after events
	FFT_afterPrintFFT();
}
#endif

// actions after FFT_printFFT
void FFT_afterPrintFFT(void) {
#if HRDW_HAS_FULL_FFT_BUFFER
	// continue DMA draw?
	if (print_fft_dma_estimated_size > 0) {
#if LCD_TYPE_FSMC
		if (print_fft_dma_estimated_size <= DMA_MAX_BLOCK) {
			HAL_DMA_Start_IT(&HRDW_LCD_FSMC_COPY_DMA, (uint32_t)&print_output_buffer[0] + print_fft_dma_position * 2, LCD_FSMC_DATA_ADDR, print_fft_dma_estimated_size);
			print_fft_dma_estimated_size = 0;
			print_fft_dma_position = 0;
		} else {
			print_fft_dma_estimated_size -= DMA_MAX_BLOCK;
			HAL_DMA_Start_IT(&HRDW_LCD_FSMC_COPY_DMA, (uint32_t)&print_output_buffer[0] + print_fft_dma_position * 2, LCD_FSMC_DATA_ADDR, DMA_MAX_BLOCK);
			print_fft_dma_position += DMA_MAX_BLOCK;
		}
		return;
#endif
#if LCD_TYPE_SPI
		if (HRDW_LCD_SPI.Init.DataSize != SPI_DATASIZE_16BIT) {
			HRDW_LCD_SPI.Init.DataSize = SPI_DATASIZE_16BIT;
			HAL_SPI_Init(&HRDW_LCD_SPI);
		}
		LCD_DC_GPIO_Port->BSRR = LCD_DC_Pin;

		HAL_SPI_Transmit_DMA(&HRDW_LCD_SPI, (uint8_t *)(&print_output_buffer[0] + print_fft_dma_position * 2), print_fft_dma_estimated_size);
		while (HAL_SPI_GetState(&HRDW_LCD_SPI) != HAL_SPI_STATE_READY) {
			CPULOAD_GoToSleepMode();
		}

		print_fft_dma_estimated_size = 0;
		print_fft_dma_position = 0;
#endif
	}
#endif

	// calc bandmap
	if (lastWTFFreq != currentFFTFreq || NeedWTFRedraw) {
		// clear and display part of the vertical bar
		dma_memset(bandmap_line_tmp, 0x00, sizeof(bandmap_line_tmp));

		// output bandmaps
		int8_t band_curr = getBandFromFreq(CurrentVFO->SpectrumCenterFreq, true);
		int8_t band_left = band_curr;
		if (band_curr > 0) {
			band_left = band_curr - 1;
		}
		int8_t band_right = band_curr;
		if (band_curr < (BANDS_COUNT - 1)) {
			band_right = band_curr + 1;
		}
		int64_t fft_freq_position_start, fft_freq_position_stop;
		for (uint16_t band = band_left; band <= band_right; band++) {
			// regions
			for (uint16_t region = 0; region < BANDS[band].regionsCount; region++) {
				uint8_t region_color = 1;
				if (BANDS[band].regions[region].mode == TRX_MODE_CW) {
					region_color = 2;
				} else if (BANDS[band].regions[region].mode == TRX_MODE_DIGI_L || BANDS[band].regions[region].mode == TRX_MODE_DIGI_U || BANDS[band].regions[region].mode == TRX_MODE_RTTY) {
					region_color = 3;
				} else if (BANDS[band].regions[region].mode == TRX_MODE_NFM || BANDS[band].regions[region].mode == TRX_MODE_WFM) {
					region_color = 4;
				} else if (BANDS[band].regions[region].mode == TRX_MODE_AM || BANDS[band].regions[region].mode == TRX_MODE_SAM_STEREO || BANDS[band].regions[region].mode == TRX_MODE_SAM_LSB ||
				           BANDS[band].regions[region].mode == TRX_MODE_SAM_USB) {
					region_color = 5;
				}

				fft_freq_position_start = getFreqPositionOnFFT(BANDS[band].regions[region].startFreq, false);
				fft_freq_position_stop = getFreqPositionOnFFT(BANDS[band].regions[region].endFreq, false);
				if (fft_freq_position_start != -1 && fft_freq_position_stop == -1) {
					fft_freq_position_stop = LAYOUT->FFT_PRINT_SIZE;
				}
				if (fft_freq_position_start == -1 && fft_freq_position_stop != -1) {
					fft_freq_position_start = 0;
				}
				if (fft_freq_position_start == -1 && fft_freq_position_stop == -1 && BANDS[band].regions[region].startFreq < CurrentVFO->SpectrumCenterFreq &&
				    BANDS[band].regions[region].endFreq > CurrentVFO->SpectrumCenterFreq) {
					fft_freq_position_start = 0;
					fft_freq_position_stop = LAYOUT->FFT_PRINT_SIZE;
				}

				if (fft_freq_position_start != -1 && fft_freq_position_stop != -1) {
					for (int64_t pixel_counter = fft_freq_position_start; pixel_counter < fft_freq_position_stop; pixel_counter++) {
						bandmap_line_tmp[(uint16_t)pixel_counter] = region_color;
					}
				}
			}
		}
	}

	LCDDriver_SetCursorAreaPosition(0, LAYOUT->FFT_FFTWTF_POS_Y - LAYOUT->FFT_FREQLABELS_HEIGHT - 4, LAYOUT->FFT_PRINT_SIZE - 1, LAYOUT->FFT_FFTWTF_POS_Y - 3);
	for (uint8_t r = 0; r < 2; r++) {
		for (uint32_t pixel_counter = 0; pixel_counter < LAYOUT->FFT_PRINT_SIZE; pixel_counter++) {
			uint16_t bandmap_color;
			switch (bandmap_line_tmp[pixel_counter]) {
			case 1:
				bandmap_color = COLOR->BANDMAP_SSB;
				break;
			case 2:
				bandmap_color = COLOR->BANDMAP_CW;
				break;
			case 3:
				bandmap_color = COLOR->BANDMAP_DIGI;
				break;
			case 4:
				bandmap_color = COLOR->BANDMAP_FM;
				break;
			case 5:
				bandmap_color = COLOR->BANDMAP_AM;
				break;
			default:
				bandmap_color = 0;
				break;
			}
			LCDDriver_SendData16(bandmap_color);
		}
	}

	// Print FFT frequency labels
	if (LAYOUT->FFT_FREQLABELS_HEIGHT > 0 && (lastWTFFreq != currentFFTFreq || NeedWTFRedraw)) {
		bool first = true;
		char str[32] = {0};
		for (int32_t grid_line_index = 0; grid_line_index < FFT_MAX_GRID_NUMBER; grid_line_index++) {
			if (grid_lines_pos[grid_line_index] > 0 && grid_lines_pos[grid_line_index] < LAYOUT->FFT_PRINT_SIZE) {
				if (first) {
					LCDDriver_Fill_RectWH(0, LAYOUT->FFT_FFTWTF_POS_Y - LAYOUT->FFT_FREQLABELS_HEIGHT, grid_lines_pos[grid_line_index], LAYOUT->FFT_FREQLABELS_HEIGHT - 1, BG_COLOR);
					first = false;
				}
				if (grid_lines_pos[grid_line_index + 1] > 0) {
					LCDDriver_Fill_RectWH(grid_lines_pos[grid_line_index], LAYOUT->FFT_FFTWTF_POS_Y - LAYOUT->FFT_FREQLABELS_HEIGHT,
					                      grid_lines_pos[grid_line_index + 1] - grid_lines_pos[grid_line_index], LAYOUT->FFT_FREQLABELS_HEIGHT - 1, BG_COLOR);
				} else {
					LCDDriver_Fill_RectWH(grid_lines_pos[grid_line_index], LAYOUT->FFT_FFTWTF_POS_Y - LAYOUT->FFT_FREQLABELS_HEIGHT, LAYOUT->FFT_PRINT_SIZE - grid_lines_pos[grid_line_index],
					                      LAYOUT->FFT_FREQLABELS_HEIGHT - 1, BG_COLOR);
				}
				uint64_t freq = grid_lines_freq[grid_line_index] / 1000;
				float64_t freq2 = (float64_t)freq / 1000.0;
				sprintf(str, "%.3f", freq2);
				int32_t x = grid_lines_pos[grid_line_index] - (strlen(str) * 6 / 2);
				LCDDriver_printText(str, x, LAYOUT->FFT_FFTWTF_POS_Y - LAYOUT->FFT_FREQLABELS_HEIGHT, FG_COLOR, BG_COLOR, 1);
			}
		}
	}

	// finish
	FFT_FPS++;
	lastWTFFreq = currentFFTFreq;
	if (NeedWTFRedraw) // redraw cycles counter
	{
		if (needredraw_wtf_counter == 0) {
			needredraw_wtf_counter = 3;
			NeedWTFRedraw = false;
		} else {
			needredraw_wtf_counter--;
		}
	}
	FFT_need_fft = true;
	LCD_busy = false;
}

// get color from signal strength
static uint16_t getFFTColor(uint_fast8_t height, bool type) // Get FFT color warmth (blue to red) , type 0 - fft, type 1 - wtf
{
	uint_fast8_t red = 0;
	uint_fast8_t green = 0;
	uint_fast8_t blue = 0;
	if (COLOR->WTF_BG_WHITE) {
		red = 255;
		green = 255;
		blue = 255;
	}

	// blue -> yellow -> red Blu>Y>R
	if ((!type && TRX.FFT_Color == 0) || (type && TRX.WTF_Color == 0)) {
		// r g b
		// 0 0 0
		// 0 0 255
		// 255 255 0
		// 255 0 0
		// contrast of each of the 3 zones, the total should be 1.0f
		const float32_t contrast1 = 0.02f;
		const float32_t contrast2 = 0.45f;
		const float32_t contrast3 = 0.53f;

		if (height < GET_FFTHeight * contrast1) {
			blue = (uint_fast8_t)(height * 255 / (GET_FFTHeight * contrast1));
			if (COLOR->WTF_BG_WHITE) {
				red -= blue;
				green -= blue;
			}
		} else if (height < GET_FFTHeight * (contrast1 + contrast2)) {
			green = (uint_fast8_t)((height - GET_FFTHeight * contrast1) * 255 / ((GET_FFTHeight - GET_FFTHeight * contrast1) * (contrast1 + contrast2)));
			red = green;
			blue = 255 - green;
		} else {
			red = 255;
			blue = 0;
			green = (uint_fast8_t)(255 - (height - (GET_FFTHeight * (contrast1 + contrast2))) * 255 /
			                                 ((GET_FFTHeight - (GET_FFTHeight * (contrast1 + contrast2))) * (contrast1 + contrast2 + contrast3)));
		}
		return rgb888torgb565(red, green, blue);
	}

	// blue -> yellow -> red // version 2 BlB>Y>R
	if ((!type && TRX.FFT_Color == 1) || (type && TRX.WTF_Color == 1)) {
		// r g b
		// 0 0 0
		// 0 0 255
		// 255 255 0
		// 255 0 0
		// contrast of each of the 3 zones, the total should be 1.0f
		const float32_t contrast1 = 0.40f; // blue
		const float32_t contrast2 = 0.30f; // yellow
		const float32_t contrast3 = 0.30f; // red

		if (height < GET_FFTHeight * contrast1) {
			blue = (uint_fast8_t)(height * 255 / (GET_FFTHeight * contrast1));
			if (COLOR->WTF_BG_WHITE) {
				red -= blue;
				green -= blue;
			}
		} else if (height < GET_FFTHeight * (contrast1 + contrast2)) {
			green = (uint_fast8_t)((height - GET_FFTHeight * contrast1) * 255 / ((GET_FFTHeight - GET_FFTHeight * contrast1) * (contrast1 + contrast2)));
			red = green;
			blue = 255 - green;
		} else {
			red = 255;
			blue = 0;
			green = (uint_fast8_t)(255 - (height - (GET_FFTHeight * (contrast1 + contrast2))) * 255 /
			                                 ((GET_FFTHeight - (GET_FFTHeight * (contrast1 + contrast2))) * (contrast1 + contrast2 + contrast3)));
		}
		return rgb888torgb565(red, green, blue);
	}

	// blue -> yellow -> red // version 3 BlR>Y>R
	if ((!type && TRX.FFT_Color == 2) || (type && TRX.WTF_Color == 2)) {
		// r g b
		// 0 0 0
		// 0 0 255
		// 255 255 0
		// 255 0 0
		// contrast of each of the 4 zones, the total should be 1.0f
		const float32_t contrast0 = 0.15f; // black
		const float32_t contrast1 = 0.25f; // blue
		const float32_t contrast2 = 0.10f; // yellow
		const float32_t contrast3 = 0.50f; // red

		if (height < GET_FFTHeight * contrast0) {
			red = 0;
			green = 0;
			blue = 0;
		} else if (height < GET_FFTHeight * (contrast0 + contrast1)) {
			blue = (uint_fast8_t)((height - GET_FFTHeight * contrast0) * 255 / ((GET_FFTHeight - GET_FFTHeight * contrast0) * (contrast0 + contrast1)));
			if (COLOR->WTF_BG_WHITE) {
				red -= blue;
				green -= blue;
			}
		} else if (height < GET_FFTHeight * (contrast0 + contrast1 + contrast2)) {
			green = (uint_fast8_t)((height - GET_FFTHeight * contrast1) * 255 / ((GET_FFTHeight - GET_FFTHeight * contrast1) * (contrast1 + contrast2)));
			red = green;
			blue = 255 - green;
		} else {
			red = 255;
			blue = 0;
			green = (uint_fast8_t)(255 - (height - (GET_FFTHeight * (contrast1 + contrast2))) * 255 /
			                                 ((GET_FFTHeight - (GET_FFTHeight * (contrast1 + contrast2))) * (contrast1 + contrast2 + contrast3)));
		}
		return rgb888torgb565(red, green, blue);
	}

	// black -> blue -> green -> yellow -> red - > magenta // BGYRM
	if ((!type && TRX.FFT_Color == 3) || (type && TRX.WTF_Color == 3)) {
		// r g b
		// 0 0 0
		// 0 0 255
		// 255 255 0
		// 255 0 0
		// contrast of each of the 4 zones, the total should be 1.0f
		const float32_t contrast0 = 0.05f; // black
		const float32_t contrast1 = 0.25f; // blue
		const float32_t contrast2 = 0.20f; // green
		const float32_t contrast3 = 0.15f; // yellow
		const float32_t contrast4 = 0.30f; // red
		const float32_t contrast5 = 0.05f; // magenta

		if (height < GET_FFTHeight * contrast0) { // black
			red = 0;
			green = 0;
			blue = 0;
		} else if (height < GET_FFTHeight * (contrast0 + contrast1)) { // blue
			red = 0;
			green = 0;
			blue = (uint_fast8_t)((height - GET_FFTHeight * contrast0) * 255 / ((GET_FFTHeight - GET_FFTHeight * contrast0) * (contrast0 + contrast1)));
		} else if (height < GET_FFTHeight * (contrast0 + contrast1 + contrast2)) { // green
			int16_t color =
			    (uint_fast8_t)((height - GET_FFTHeight * (contrast0 + contrast1)) * 255 / ((GET_FFTHeight - GET_FFTHeight * (contrast0 + contrast1)) * (contrast0 + contrast1 + contrast2)));
			int16_t second_color = 255 - color * 2;
			red = 0;
			green = color;
			blue = second_color > 0 ? second_color : 0;
		} else if (height < GET_FFTHeight * (contrast0 + contrast1 + contrast2 + contrast3)) { // yellow
			uint16_t color = (uint_fast8_t)((height - GET_FFTHeight * (contrast0 + contrast1 + contrast2)) * 255 /
			                                ((GET_FFTHeight - GET_FFTHeight * (contrast0 + contrast1 + contrast2)) * (contrast0 + contrast1 + contrast2 + contrast3)));
			green = 255;
			red = color;
			blue = 0;
		} else if (height < GET_FFTHeight * (contrast0 + contrast1 + contrast2 + contrast3 + contrast4)) { // red
			uint16_t color = (uint_fast8_t)((height - GET_FFTHeight * (contrast0 + contrast1 + contrast2 + contrast3)) * 255 /
			                                ((GET_FFTHeight - GET_FFTHeight * (contrast0 + contrast1 + contrast2 + contrast3)) * (contrast0 + contrast1 + contrast2 + contrast3 + contrast4)));
			int16_t second_color = 255 - color * 1;
			red = 255;
			blue = 0;
			green = second_color > 0 ? second_color : 0;
		} else { // magenta
			red = 255;
			green = 0;
			blue = (uint_fast8_t)((height - GET_FFTHeight * (contrast0 + contrast1 + contrast2 + contrast3 + contrast4)) * 255 /
			                      ((GET_FFTHeight - GET_FFTHeight * (contrast0 + contrast1 + contrast2 + contrast3 + contrast4)) *
			                       (contrast0 + contrast1 + contrast2 + contrast3 + contrast4 + contrast5)));
		}
		return rgb888torgb565(red, green, blue);
	}

	// black -> yellow -> red
	if ((!type && TRX.FFT_Color == 4) || (type && TRX.WTF_Color == 4)) {
		// r g b
		// 0 0 0
		// 255 255 0
		// 255 0 0
		// contrast of each of the 2 zones, the total should be 1.0f
		float32_t contrast1 = 0.5f;
		float32_t contrast2 = 0.5f;
		if (COLOR->WTF_BG_WHITE) {
			contrast1 = 0.2f;
			contrast2 = 0.8f;
		}

		if (height < GET_FFTHeight * contrast1) {
			if (!COLOR->WTF_BG_WHITE) {
				red = (uint_fast8_t)(height * 255 / (GET_FFTHeight * contrast1));
				green = (uint_fast8_t)(height * 255 / (GET_FFTHeight * contrast1));
				blue = 0;
			} else {
				red = 255;
				green = 255;
				blue = 255 - (uint_fast8_t)(height * 255 / (GET_FFTHeight * contrast1));
			}
		} else {
			red = 255;
			blue = 0;
			green = (uint_fast8_t)(255 - (height - (GET_FFTHeight * (contrast1))) * 255 / ((GET_FFTHeight - (GET_FFTHeight * (contrast1))) * (contrast1 + contrast2)));
			if (COLOR->WTF_BG_WHITE) {
				blue = green;
			}
		}
		return rgb888torgb565(red, green, blue);
	}

	// black -> yellow -> green
	if ((!type && TRX.FFT_Color == 5) || (type && TRX.WTF_Color == 5)) {
		// r g b
		// 0 0 0
		// 255 255 0
		// 0 255 0
		// contrast of each of the 2 zones, the total should be 1.0f
		float32_t contrast1 = 0.5f;
		float32_t contrast2 = 0.5f;
		if (COLOR->WTF_BG_WHITE) {
			contrast1 = 0.2f;
			contrast2 = 0.8f;
		}

		if (height < GET_FFTHeight * contrast1) {
			if (!COLOR->WTF_BG_WHITE) {
				red = (uint_fast8_t)(height * 255 / (GET_FFTHeight * contrast1));
				green = (uint_fast8_t)(height * 255 / (GET_FFTHeight * contrast1));
				blue = 0;
			} else {
				red = 255;
				green = 255;
				blue = 255 - (uint_fast8_t)(height * 255 / (GET_FFTHeight * contrast1));
			}
		} else {
			green = 255;
			blue = 0;
			red = (uint_fast8_t)(255 - (height - (GET_FFTHeight * (contrast1))) * 255 / ((GET_FFTHeight - (GET_FFTHeight * (contrast1))) * (contrast1 + contrast2)));
			if (COLOR->WTF_BG_WHITE) {
				green = red;
			}
		}
		return rgb888torgb565(red, green, blue);
	}

	// black -> red
	if ((!type && TRX.FFT_Color == 6) || (type && TRX.WTF_Color == 6)) {
		// r g b
		// 0 0 0
		// 255 0 0

		if (height <= GET_FFTHeight) {
			red = (uint_fast8_t)(height * 255 / (GET_FFTHeight));
			if (COLOR->WTF_BG_WHITE) {
				green -= (uint_fast8_t)(height * 255 / (GET_FFTHeight));
				blue -= (uint_fast8_t)(height * 255 / (GET_FFTHeight));
				red = 255;
			}
		}
		return rgb888torgb565(red, green, blue);
	}

	// black -> green
	if ((!type && TRX.FFT_Color == 7) || (type && TRX.WTF_Color == 7)) {
		// r g b
		// 0 0 0
		// 0 255 0

		if (height <= GET_FFTHeight) {
			green = (uint_fast8_t)(height * 255 / (GET_FFTHeight));
			if (COLOR->WTF_BG_WHITE) {
				green = 255;
				blue -= (uint_fast8_t)(height * 255 / (GET_FFTHeight));
				red -= (uint_fast8_t)(height * 255 / (GET_FFTHeight));
			}
		}
		return rgb888torgb565(red, green, blue);
	}

	// black -> blue
	if ((!type && TRX.FFT_Color == 8) || (type && TRX.WTF_Color == 8)) {
		// r g b
		// 0 0 0
		// 0 0 255

		if (height <= GET_FFTHeight) {
			blue = (uint_fast8_t)(height * 255 / (GET_FFTHeight));
			if (COLOR->WTF_BG_WHITE) {
				green -= (uint_fast8_t)(height * 255 / (GET_FFTHeight));
				blue = 255;
				red -= (uint_fast8_t)(height * 255 / (GET_FFTHeight));
			}
		}
		return rgb888torgb565(red, green, blue);
	}

	// black -> white
	if ((!type && TRX.FFT_Color == 9) || (type && TRX.WTF_Color == 9)) {
		// r g b
		// 0 0 0
		// 255 255 255

		if (height <= GET_FFTHeight) {
			red = (uint_fast8_t)(height * 255 / (GET_FFTHeight));
			green = red;
			blue = red;
			if (COLOR->WTF_BG_WHITE) {
				red = 255 - red;
				green = 255 - green;
				blue = 255 - blue;
			}
		}
		return rgb888torgb565(red, green, blue);
	}

	// unknown
	return COLOR_WHITE;
}

static uint16_t getBGColor(uint_fast8_t height) // Get FFT background gradient
{
	float32_t fftheight = GET_FFTHeight;
	float32_t step_red = (float32_t)(COLOR->FFT_GRADIENT_END_R - COLOR->FFT_GRADIENT_START_R) / fftheight;
	float32_t step_green = (float32_t)(COLOR->FFT_GRADIENT_END_G - COLOR->FFT_GRADIENT_START_G) / fftheight;
	float32_t step_blue = (float32_t)(COLOR->FFT_GRADIENT_END_B - COLOR->FFT_GRADIENT_START_B) / fftheight;

	uint_fast8_t red = (uint_fast8_t)(COLOR->FFT_GRADIENT_START_R + (float32_t)height * step_red);
	uint_fast8_t green = (uint_fast8_t)(COLOR->FFT_GRADIENT_START_G + (float32_t)height * step_green);
	uint_fast8_t blue = (uint_fast8_t)(COLOR->FFT_GRADIENT_START_B + (float32_t)height * step_blue);

	return rgb888torgb565(red, green, blue);
}

// prepare the color palette
static void FFT_fill_color_palette(void) // Fill FFT Color Gradient On Initialization
{
	uint8_t FFT_BW_BRIGHTNESS = 0;
	if (TRX.FFT_BW_Style == 1) {
		FFT_BW_BRIGHTNESS = FFT_BW_BRIGHTNESS_1;
	}
	if (TRX.FFT_BW_Style == 2) {
		FFT_BW_BRIGHTNESS = FFT_BW_BRIGHTNESS_2;
	}

	for (uint_fast8_t i = 0; i <= GET_FFTHeight; i++) {
		palette_fft[i] = getFFTColor(GET_FFTHeight - i, false);
		palette_wtf[i] = getFFTColor(GET_FFTHeight - i, true);
		if (TRX.FFT_BW_Style == 3 && !TRX.FFT_Background) { // lines BW without background
			palette_bg_gradient[i] = getFFTColor(0, false);
		} else {
			palette_bg_gradient[i] = getBGColor(GET_FFTHeight - i);
		}

		palette_bw_fft_colors[i] = addColor(palette_fft[i], FFT_BW_BRIGHTNESS, FFT_BW_BRIGHTNESS, FFT_BW_BRIGHTNESS);
		palette_bw_wtf_colors[i] = addColor(palette_wtf[i], FFT_BW_BRIGHTNESS, FFT_BW_BRIGHTNESS, FFT_BW_BRIGHTNESS);
		palette_bw_bg_colors[i] = addColor(palette_bg_gradient[i], FFT_BW_BRIGHTNESS, FFT_BW_BRIGHTNESS, FFT_BW_BRIGHTNESS);
	}
}

static float32_t getMaxDBMFromFreq(uint64_t freq, uint8_t span) {
	float32_t result = -200;
	for (uint32_t i = freq - span; i <= (freq + span); i++) {
		int32_t pos = getFreqPositionOnFFT(i, false);
		if (pos < 0) {
			continue;
		}

		float32_t dBm = FFTOutput_mean[pos];
		result = MAX(dBm, result);
	}

	return result;
}

inline int32_t getFreqPositionOnFFT(uint64_t freq, bool full_pos) {
	float64_t centerFreq = CurrentVFO->SpectrumCenterFreq;
	if (TRX_on_TX && !SHOW_RX_FFT_ON_TX) {
		centerFreq = CurrentVFO->Freq;
	}

	int32_t pos = (int32_t)((float64_t)LAYOUT->FFT_PRINT_SIZE / 2.0 + (float64_t)((float64_t)freq - centerFreq) / (float64_t)Hz_in_pixel * (float64_t)fft_zoom);
	if (!full_pos && (pos < 0 || pos >= LAYOUT->FFT_PRINT_SIZE)) {
		return -1;
	}
	if (TRX.FFT_Lens) { // lens correction
		pos = FFT_getLensCorrection(pos);
	}
	return pos;
}

uint64_t getFreqOnFFTPosition(uint16_t position) {
	int64_t centerFreq = CurrentVFO->SpectrumCenterFreq;
	if (TRX_on_TX && !SHOW_RX_FFT_ON_TX) {
		centerFreq = CurrentVFO->Freq;
	}

	return (uint64_t)(centerFreq + (int64_t)(-((float64_t)LAYOUT->FFT_PRINT_SIZE * ((float64_t)Hz_in_pixel / (float64_t)fft_zoom) / 2.0) +
	                                         (float64_t)position * ((float64_t)Hz_in_pixel / (float64_t)fft_zoom)));
}

static uint32_t FFT_getLensCorrection(uint32_t normal_distance_from_center) {
	float32_t fft_normal_compress_rate = (float32_t)FFT_USEFUL_SIZE / (float32_t)LAYOUT->FFT_PRINT_SIZE;
	float32_t normal_distance_from_center_converted = (float32_t)normal_distance_from_center * fft_normal_compress_rate;
	float32_t step_now = FFT_LENS_STEP_START;
	float32_t index1 = (float32_t)FFT_USEFUL_SIZE / 2.0f;
	float32_t index2 = index1;
	for (uint32_t i = 0; i <= (LAYOUT->FFT_PRINT_SIZE / 2); i++) {
		if (normal_distance_from_center < (LAYOUT->FFT_PRINT_SIZE / 2)) {
			if (normal_distance_from_center_converted > index1) {
				return (LAYOUT->FFT_PRINT_SIZE / 2 - i);
			}
		} else {
			if (normal_distance_from_center_converted < index2) {
				if (i != (LAYOUT->FFT_PRINT_SIZE / 2)) {
					return (LAYOUT->FFT_PRINT_SIZE / 2 + i);
				} else {
					return (LAYOUT->FFT_PRINT_SIZE - 1);
				}
			}
		}

		step_now += FFT_LENS_STEP;
		index1 -= step_now;
		index2 += step_now;

		if (index1 >= FFT_USEFUL_SIZE) {
			index1 = FFT_USEFUL_SIZE - 1;
		}
		if (index1 < 0) {
			index1 = 0;
		}
		if (index2 >= FFT_USEFUL_SIZE) {
			index2 = FFT_USEFUL_SIZE - 1;
		}
		if (index2 < 0) {
			index2 = 0;
		}
	}
	return normal_distance_from_center;
}

static float32_t getDBFromFFTAmpl(float32_t ampl) {
	float32_t dB = 0.0f;

	if (FFT_SCALE_TYPE == 0 || FFT_SCALE_TYPE == 2) {                                             // ampl / dBm scale
		dB = rate2dbP(powf(ampl / (float32_t)FFT_SIZE, 2) / 50.0f / 0.001f) + FFT_DBM_COMPENSATION; // roughly... because window and other...
	}

	if (FFT_SCALE_TYPE == 1) {                                                  // squared scale
		dB = rate2dbP(ampl / (float32_t)FFT_SIZE / 50.0f) + FFT_DBM_COMPENSATION; // roughly... because window and other...
	}

	if (TRX.ADC_Driver) {
		dB -= ADC_DRIVER_GAIN_DB;
	}

	if (CurrentVFO->SpectrumCenterFreq < 70000000) {
		dB += CALIBRATE.smeter_calibration_hf;
	} else {
		dB += CALIBRATE.smeter_calibration_vhf;
	}

	if (TRX.LNA) {
		dB += CALIBRATE.LNA_compensation;
	}

	if (CALIBRATE.ATT_compensation && TRX.ATT) {
		dB += TRX.ATT_DB;
	}

	return dB;
}

static float32_t getFFTAmplFromDB(float32_t ampl) {
	float32_t result;

	float32_t dB = ampl - FFT_DBM_COMPENSATION;
	if (TRX.ADC_Driver) {
		dB += ADC_DRIVER_GAIN_DB;
	}
	if (CurrentVFO->SpectrumCenterFreq < 70000000) {
		dB -= CALIBRATE.smeter_calibration_hf;
	} else {
		dB -= CALIBRATE.smeter_calibration_vhf;
	}
	if (TRX.LNA) {
		dB -= CALIBRATE.LNA_compensation;
	}
	if (CALIBRATE.ATT_compensation && TRX.ATT) {
		dB -= TRX.ATT_DB;
	}

	if (FFT_SCALE_TYPE == 0 || FFT_SCALE_TYPE == 2) { // ampl / dBm scale
		float32_t power = db2rateP(dB) * 0.001f * 50.0f;
		arm_sqrt_f32(power, &result);
	}
	if (FFT_SCALE_TYPE == 1) { // squared scale
		result = db2rateP(dB) * 50.0f;
	}

	return result * (float32_t)FFT_SIZE;
}
