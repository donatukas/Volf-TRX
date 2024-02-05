#include "touchpad.h"
#include "lcd.h"
#include "main.h"

#if (defined(HAS_TOUCHPAD))

static bool touched = false;
static bool hold_touch_handled = false;
static bool hold_swipe_handled = false;
static uint32_t touch_startime = 0;
static uint32_t touch_lasttime = 0;
static uint16_t touch_start_x1 = 0;
static uint16_t touch_start_x2 = 0;
static uint16_t touch_start_y1 = 0;
static uint16_t touch_start_y2 = 0;
static uint16_t touch_end_x1 = 0;
static uint16_t touch_end_x2 = 0;
static uint16_t touch_end_y1 = 0;
static uint16_t touch_end_y2 = 0;

void TOUCHPAD_Init(void) {
#if (defined(TOUCHPAD_GT911))
	// read touchpad info
	GT911_ReadStatus();
	GT911_ReadFirmwareVersion();
	GT911_Init();

	// calibrate
	uint8_t send = 3;
	GT911_WR_Reg(0x8040, (uint8_t *)&send, 1); // Reference capacitance update (Internal test);
#endif
}

void TOUCHPAD_ProcessInterrupt(void) {
#if (defined(TOUCHPAD_GT911))
	if (LCD_busy) {
		return;
	}
	GT911_Scan();

	if (touched && !hold_swipe_handled && (touch_lasttime < (HAL_GetTick() - CALIBRATE.TOUCHPAD_TIMEOUT)) && ((touch_lasttime - touch_startime) <= CALIBRATE.TOUCHPAD_CLICK_TIMEOUT) &&
	    ((touch_lasttime - touch_startime) >= CALIBRATE.TOUCHPAD_CLICK_THRESHOLD)) {
		if (TRX.Debug_Type == TRX_DEBUG_TOUCH) {
			println("Process Touch X:", touch_end_x1, " Y: ", touch_end_y1);
		}

		LCD_processTouch(touch_end_x1, touch_end_y1);
		touched = false;
	} else if (touched && !hold_touch_handled && !hold_swipe_handled && (touch_lasttime > (HAL_GetTick() - CALIBRATE.TOUCHPAD_TIMEOUT)) &&
	           ((touch_lasttime - touch_startime) >= CALIBRATE.TOUCHPAD_HOLD_TIMEOUT)) {
		hold_touch_handled = true;
		if (TRX.Debug_Type == TRX_DEBUG_TOUCH) {
			println("Process Hold Touch X:", touch_end_x1, " Y: ", touch_end_y1);
		}
		LCD_processHoldTouch(touch_end_x1, touch_end_y1);
	} else if (touched && (touch_lasttime > (HAL_GetTick() - CALIBRATE.TOUCHPAD_TIMEOUT))) {
		// One/Two finger swipe
		if (hold_swipe_handled || abs(touch_end_x1 - touch_start_x1) > CALIBRATE.TOUCHPAD_SWIPE_THRESHOLD_PX || abs(touch_end_y1 - touch_start_y1) > CALIBRATE.TOUCHPAD_SWIPE_THRESHOLD_PX) {
			bool two_finger = false;
			if (abs(touch_end_x2 - touch_start_x2) > CALIBRATE.TOUCHPAD_SWIPE_THRESHOLD_PX || abs(touch_end_y2 - touch_start_y2) > CALIBRATE.TOUCHPAD_SWIPE_THRESHOLD_PX) {
				two_finger = true;
			}

			int16_t dx1 = (touch_end_x1 - touch_start_x1);
			int16_t dy1 = (touch_end_y1 - touch_start_y1);
			int16_t dx2 = (touch_end_x2 - touch_start_x2);
			int16_t dy2 = (touch_end_y2 - touch_start_y2);
			if (two_finger && touch_end_x2 < touch_end_x1) // right finger touched first
			{
				dx1 = dx2;
				dy1 = dy2;
			}

			if (two_finger && LCD_processSwipeTwoFingersTouch(touch_start_x1, touch_start_y1, dx1, dy1)) {
				touch_start_x1 = touch_end_x1;
				touch_start_x2 = touch_end_x2;
				touch_start_y1 = touch_end_y1;
				touch_start_y2 = touch_end_y2;
			} else if (!two_finger && (touch_end_x2 == 0 && touch_end_y2 == 0) && LCD_processSwipeTouch(touch_start_x1, touch_start_y1, dx1, dy1)) {
				touch_start_x1 = touch_end_x1;
				touch_start_y1 = touch_end_y1;
				hold_swipe_handled = true;
			}
		}
	} else if (touched && (touch_lasttime < (HAL_GetTick() - CALIBRATE.TOUCHPAD_TIMEOUT))) {
		touched = false;
		hold_touch_handled = false;
		hold_swipe_handled = false;
	}
#endif
}

void TOUCHPAD_reserveInterrupt(void) {
#if (defined(TOUCHPAD_GT911))
	GT911.Touch = 1;
#endif
}

void TOUCHPAD_processTouch(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
	touch_end_x1 = x1;
	touch_end_x2 = x2;
	touch_end_y1 = y1;
	touch_end_y2 = y2;
	touch_lasttime = HAL_GetTick();
	if (touch_end_x2 == 0 && touch_end_y2 == 0) {
		touch_start_x2 = 0;
		touch_start_y2 = 0;
	}
	if (!touched) {
		touched = true;
		touch_startime = touch_lasttime;
		touch_start_x1 = x1;
		touch_start_x2 = x2;
		touch_start_y1 = y1;
		touch_start_y2 = y2;
	}
}

#endif
