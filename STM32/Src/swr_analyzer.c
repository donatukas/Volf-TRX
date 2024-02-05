#include "swr_analyzer.h"
#include "fpga.h"
#include "functions.h"
#include "lcd.h"
#include "lcd_driver.h"
#include "main.h"
#include "rf_unit.h"
#include "trx_manager.h"

// Private variables
static const uint16_t graph_start_x = 25;
static const uint16_t graph_start_y = 5;
static float32_t now_freq;
static float32_t freq_step;
static uint16_t graph_sweep_x = 0;
static uint32_t tick_start_time = 0;
#if LCD_WIDTH > 30
static float32_t data[LCD_WIDTH - 30] = {0};
static const uint16_t graph_width = LCD_WIDTH - 30;
static const uint16_t graph_height = LCD_HEIGHT - 25;
static int16_t graph_selected_x = (LCD_WIDTH - 30) / 2;
#else
static float32_t data[2] = {0};
static const uint16_t graph_width = 1;
static const uint16_t graph_height = 1;
static int16_t graph_selected_x = 1;
#endif
static uint32_t startFreq = 0;
static uint32_t endFreq = 0;
static uint32_t minSWR_Freq = 99.0f;
static float32_t minSWR_SWR = 0;

// Saved variables
static uint64_t Lastfreq = 0;
static bool LastMute = false;

// Public variables
bool SYSMENU_swr_opened = false;
bool SYSMENU_TDM_CTRL_opened = false; // Tisho

// Prototypes
static void SWR_DrawBottomGUI(void);                  // display status at the bottom of the screen
static void SWR_DrawGraphCol(uint16_t x, bool clear); // display the data column
static uint16_t SWR_getYfromX(uint16_t x);            // get height from data id

extern void TDM_Voltages_Start(void) // Tisho
{
	LCD_busy = true;

	// draw the GUI
	LCDDriver_Fill(COLOR_BLACK);
	// LCDDriver_drawFastVLine(graph_start_x, graph_start_y, graph_height, COLOR_WHITE);

	LCDDriver_printText("Scaled   PWR", 50, 10, COLOR_GREEN, COLOR_BLACK, 2);
	LCDDriver_printText(" [V]     [W]", 50, 30, COLOR_GREEN, COLOR_BLACK, 2);
	LCDDriver_drawFastHLine(0, 50, 480, COLOR_WHITE);

	LCDDriver_printText("Used standard power meter!", 90, 135, COLOR_WHITE, COLOR_BLACK, 2);

	LCD_busy = false;

	LCD_UpdateQuery.SystemMenu = true;
}

extern void TDM_Voltages(void) // Tisho
{
	char ctmp[64] = {0};

	RF_UNIT_ProcessSensors();

	// Print the forward values
	sprintf(ctmp, "FW: %.1f ", (double)TRX_VLT_forward);
	// LCDDriver_Fill_RectWH(5, 30, 480, 18, COLOR_BLACK);
	LCDDriver_printText(ctmp, 5, 55, COLOR_GREEN, COLOR_BLACK, 2);

	sprintf(ctmp, "%.2f ", (double)TRX_PWR_Forward);
	LCDDriver_printText(ctmp, 150, 55, COLOR_GREEN, COLOR_BLACK, 2);

	// Print the backward values
	sprintf(ctmp, "BW: %.1f ", (double)TRX_VLT_backward);
	LCDDriver_printText(ctmp, 5, 80, COLOR_GREEN, COLOR_BLACK, 2);

	sprintf(ctmp, "%.2f ", (double)TRX_PWR_Backward);
	LCDDriver_printText(ctmp, 150, 80, COLOR_GREEN, COLOR_BLACK, 2);
}

// prepare the spectrum analyzer
void SWR_Start(uint32_t start, uint32_t end) {
	LCD_busy = true;
	startFreq = start;
	endFreq = end;

	// save settings
	Lastfreq = CurrentVFO->Freq;
	LastMute = TRX.Mute;

	// draw the GUI
	LCDDriver_Fill(COLOR_BLACK);
	LCDDriver_drawFastVLine(graph_start_x, graph_start_y, graph_height, COLOR_WHITE);
	LCDDriver_drawFastHLine(graph_start_x, graph_start_y + graph_height, graph_width, COLOR_WHITE);

	// horizontal labels
	char ctmp[64] = {0};
	sprintf(ctmp, "%u", (startFreq / 1000));
	LCDDriver_printText(ctmp, graph_start_x + 2, graph_start_y + graph_height + 3, COLOR_GREEN, COLOR_BLACK, 1);
	sprintf(ctmp, "%u", (endFreq / 1000));
	LCDDriver_printText(ctmp, graph_start_x + graph_width - 36, graph_start_y + graph_height + 3, COLOR_GREEN, COLOR_BLACK, 1);
	SWR_DrawBottomGUI();

	// vertical labels
	float32_t vres = SWR_TopSWR - 1.0f;
	float32_t partsize = vres / (SWR_VParts - 1);
	for (uint8_t n = 0; n < SWR_VParts; n++) {
		int32_t y = graph_start_y + (int32_t)(partsize * n * graph_height / vres);
		sprintf(ctmp, "%.1f", (double)(SWR_TopSWR - partsize * n));
		LCDDriver_printText(ctmp, 0, (uint16_t)y, COLOR_GREEN, COLOR_BLACK, 1);
		LCDDriver_drawFastHLine(graph_start_x, (uint16_t)y, graph_width, COLOR_DGRAY);
	}

	// start scanning
	TRX_setFrequency(startFreq, CurrentVFO);
	TRX.Mute = true;
	TRX_Tune = true;
	TRX_ptt_hard = TRX_Tune;
	TRX_Restart_Mode();

	FPGA_NeedSendParams = true;
	now_freq = startFreq;
	freq_step = (endFreq - startFreq) / graph_width;
	graph_sweep_x = 0;
	tick_start_time = HAL_GetTick();

	LCD_busy = false;

	LCD_UpdateQuery.SystemMenu = true;
}

void SWR_Stop(void) {
	TRX_setFrequency(Lastfreq, CurrentVFO);
	TRX.Mute = LastMute;
	TRX_Tune = false;
	TRX_ptt_hard = false;
	TRX_ptt_soft = false;
	TRX_Restart_Mode();
}

// draw the spectrum analyzer
void SWR_Draw(void) {
	static uint32_t minSWR_Freq_tmp = 0;
	static float32_t minSWR_SWR_tmp = 99.0f;

	if (LCD_busy) {
		return;
	}

	// Wait while data is being typed
	if ((HAL_GetTick() - tick_start_time) < SWR_StepDelay) {
		LCD_UpdateQuery.SystemMenu = true;
		return;
	}
	if (TRX_SWR == 0) { //-V550
		return;
	}
	tick_start_time = HAL_GetTick();

	LCD_busy = true;
	// Read the signal SWR
	// RF_UNIT_ProcessSensors();

	// Draw
	if (graph_sweep_x < graph_width) {
		data[graph_sweep_x] = TRX_SWR;
		SWR_DrawGraphCol(graph_sweep_x, true);
		// draw a marker
		if (graph_sweep_x == graph_selected_x) {
			SWR_DrawBottomGUI();
		}
		// calculate minimum SWR
		if (minSWR_SWR_tmp > TRX_SWR) {
			minSWR_SWR_tmp = TRX_SWR;
			minSWR_Freq_tmp = (uint32_t)now_freq;
		}
		if (minSWR_SWR > TRX_SWR) {
			minSWR_SWR = TRX_SWR;
			minSWR_Freq = (uint32_t)now_freq;
		}
	}

	// Move on to calculating the next step
	graph_sweep_x++;
	if (now_freq >= endFreq) {
		graph_sweep_x = 0;
		now_freq = startFreq;
		// Minimum SWR
		minSWR_SWR = minSWR_SWR_tmp;
		minSWR_Freq = minSWR_Freq_tmp;
		minSWR_SWR_tmp = 99.0f;
	}
	now_freq += freq_step;
	TRX_setFrequency((uint32_t)now_freq, CurrentVFO);
	FPGA_NeedSendParams = true;
	LCD_busy = false;
}

// get height from data id
static uint16_t SWR_getYfromX(uint16_t x) {
	int32_t y = graph_start_y + (int32_t)((SWR_TopSWR - data[x]) * (float32_t)(graph_height) / (SWR_TopSWR - 1.0f));
	if (y < graph_start_y) {
		y = graph_start_y;
	}
	if (y > ((graph_start_y + graph_height) - 1)) {
		y = (graph_start_y + graph_height) - 1;
	}
	return (uint16_t)y;
}

// display the data column
static void SWR_DrawGraphCol(uint16_t x, bool clear) {
	if (x == 0 || x >= graph_width) {
		return;
	}

	if (clear) {
		// clear
		LCDDriver_drawFastVLine((graph_start_x + x + 1), graph_start_y, graph_height - 1, COLOR_BLACK);
		// draw stripes behind the chart
		int16_t vres = SWR_TopSWR;
		for (uint8_t n = 0; n < (SWR_VParts - 1); n++) {
			LCDDriver_drawPixel((graph_start_x + x + 1), (uint16_t)(graph_start_y + ((vres / (SWR_VParts - 1)) * n * graph_height / vres)), COLOR_DGRAY);
		}
	}
	// draw the graph
	LCDDriver_drawLine((graph_start_x + x), SWR_getYfromX(x - 1), (graph_start_x + x + 1), SWR_getYfromX(x), COLOR_RED);
}

// display status at the bottom of the screen
static void SWR_DrawBottomGUI(void) {
	char ctmp[64] = {0};
	int32_t freq = (int32_t)startFreq + ((int32_t)(endFreq - startFreq) / (graph_width - 1) * graph_selected_x);
	sprintf(ctmp, "Freq=%dkHz SWR=%.1f | MinSWR=%.1f on %dkHz", (freq / 1000), (double)data[graph_selected_x], (double)minSWR_SWR, (minSWR_Freq / 1000));
#ifndef LCD_SMALL_INTERFACE
	LCDDriver_Fill_RectWH(100, graph_start_y + graph_height + 3, 200, 6, COLOR_BLACK);
	LCDDriver_printText(ctmp, 100, graph_start_y + graph_height + 3, COLOR_GREEN, COLOR_BLACK, 1);
#endif
	LCDDriver_drawFastVLine(graph_start_x + (uint16_t)graph_selected_x + 1, graph_start_y, graph_height, COLOR_GREEN);
}

// analyzer events to the encoder
void SWR_EncRotate(int8_t direction) {
	if (LCD_busy) {
		return;
	}
	LCD_busy = true;
	// erase the old marker
	SWR_DrawGraphCol((uint16_t)graph_selected_x, true);
	if (direction < 0) {
		SWR_DrawGraphCol((uint16_t)graph_selected_x + 1, false);
	}
	// draw a new one
	graph_selected_x += direction;
	if (graph_selected_x < 0) {
		graph_selected_x = 0;
	}
	if (graph_selected_x > (graph_width - 1)) {
		graph_selected_x = graph_width - 1;
	}
	SWR_DrawBottomGUI();
	LCD_busy = false;
}
