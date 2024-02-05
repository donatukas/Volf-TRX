#ifndef DECODE_FT8_H_
#define DECODE_FT8_H_

#include <stdint.h>

int ft8_decode(void);

extern char Target_Call[7];
extern int Target_RSL;      // four character RSL  + /0
extern char Target_Grid[5]; // Grid square of the target station (partner)
extern char RapRcv_RSL[5];  // Recieved raport from our corespondent - used to determine if he got our "raport" mesage

typedef struct {
	char field1[14];
	char field2[14];
	char field3[7];
	// int freq_Hz;
	// char decode_time[10];
	int16_t sync_score;
	int16_t snr;
	// int distance;
} Decode;

/*typedef struct {
  char decode_time[10];
  char call[7];
} Calling_Station;

typedef struct {
  char decode_time[10];
  char call[7];
  int distance;
  int snr;
  int freq_Hz;
} CQ_Station;*/

void save_Answer_CQ_List(void);

void display_Answer_CQ_Items(void);

int Check_Calling_Stations(int num_decoded);
void clear_CQ_List_box(void);
void display_messages(int decoded_messages);
void clear_display_details(void);
int Check_CQ_Calling_Stations(int num_decoded, int reply_state);
int Check_QSO_Calling_Stations(int num_decoded, int reply_state);
void Check_CQ_Stations(int num_decoded);

void SetNew_TargetCall(int index);
void process_selected_CQ(void);

int CheckRecievedRaportRSL(int index, char CQ_Answer);
int CheckRecieved73(int index, char CQ_Answer);
int FindPartnerIDX(int num_decoded);

#endif /* DECODE_FT8_H_ */
