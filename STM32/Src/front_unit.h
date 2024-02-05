#ifndef FRONT_UNIT_h
#define FRONT_UNIT_h

#include "hardware.h"
#include "settings.h"
#include <stdbool.h>

#define MCP3008_SINGLE_THRESHOLD 300

typedef enum {
	FUNIT_CTRL_BUTTON,
	FUNIT_CTRL_AF_GAIN,
	FUNIT_CTRL_IF_RIT_XIT,
	FUNIT_CTRL_IF_GAIN,
	FUNIT_CTRL_RIT,
	FUNIT_CTRL_PTT,
	FUNIT_CTRL_TANGENT,
	FUNIT_CTRL_BUTTON_DEBUG,
	FUNIT_CTRL_ENC2SW,
} FRONT_UNIT_CONTROL_TYPE;

typedef struct {
	bool work_in_menu;
	char *name;
	uint32_t parameter;
	void (*clickHandler)(uint32_t parameter);
	void (*holdHandler)(uint32_t parameter);
	uint32_t *checkBool;
} PERIPH_FrontPanel_FuncButton;

typedef struct {
	uint32_t parameter;
	uint32_t start_hold_time;
	void (*clickHandler)(uint32_t parameter);
	void (*holdHandler)(uint32_t parameter);
	uint16_t tres_min;
	uint16_t tres_max;
	uint8_t port;
	uint8_t channel;
	bool state;
	bool prev_state;
	bool afterhold;
	bool work_in_menu;
	FRONT_UNIT_CONTROL_TYPE type;
	char name[16];
} PERIPH_FrontPanel_Button;

extern PERIPH_FrontPanel_Button PERIPH_FrontPanel_TANGENT_MH48[6];
extern PERIPH_FrontPanel_Button PERIPH_FrontPanel_Buttons[];
extern const PERIPH_FrontPanel_FuncButton PERIPH_FrontPanel_FuncButtonsList[FUNCBUTTONS_COUNT];
extern bool FRONTPanel_MCP3008_1_Enabled;
extern bool FRONTPanel_MCP3008_2_Enabled;
extern bool FRONTPanel_MCP3008_3_Enabled;
extern int8_t FRONTPANEL_ProcessEncoder1;
extern int8_t FRONTPANEL_ProcessEncoder2;

extern void FRONTPANEL_Init(void);
extern void FRONTPANEL_Process(void);
extern void FRONTPANEL_CheckButton(PERIPH_FrontPanel_Button *button, uint16_t mcp3008_value);
extern void FRONTPANEL_ENCODER_Rotated(float32_t direction);
extern void FRONTPANEL_ENC2SW_validate();
extern void FRONTPANEL_check_ENC2SW(bool state);
#ifdef HRDW_HAS_ENC3
extern int8_t FRONTPANEL_ProcessEncoder3;
extern void FRONTPANEL_check_ENC3SW(bool state);
#endif
#ifdef HRDW_HAS_ENC4
extern int8_t FRONTPANEL_ProcessEncoder4;
extern void FRONTPANEL_check_ENC4SW(bool state);
#endif

#endif
