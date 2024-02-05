#ifndef __biquad_h
#define __biquad_h

#ifdef STM32F407xx
#define IIR_BIQUAD_MAX_SECTIONS 8
#else
#define IIR_BIQUAD_MAX_SECTIONS 15
#endif

#define IIR_BIQUAD_SECTION_ORDER 2

typedef struct iir_filter {
	int sections;
	int sect_ord;
	double a[IIR_BIQUAD_MAX_SECTIONS * (IIR_BIQUAD_SECTION_ORDER + 1)];
	double b[IIR_BIQUAD_MAX_SECTIONS * (IIR_BIQUAD_SECTION_ORDER + 1)];
	double d[(IIR_BIQUAD_MAX_SECTIONS + 1) * IIR_BIQUAD_SECTION_ORDER];
} iir_filter_t;

iir_filter_t *biquad_create(int sections);
void biquad_delete(iir_filter_t *filter);
double biquad_update(iir_filter_t *filter, float x);
void iir_freq_resp(iir_filter_t *filter, float *h, float fs, float f);
void biquad_zero(struct iir_filter *filter);
void biquad_init_lowpass(iir_filter_t *filter, float fs, float f);
void biquad_init_highpass(iir_filter_t *filter, float fs, float f);
void biquad_init_bandpass(iir_filter_t *filter, float fs, float f1, float f2);
void biquad_init_bandstop(iir_filter_t *filter, float fs, float f1, float f2);

#endif
