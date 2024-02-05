module stm32_interface(
clk_in,
RX1_I,
RX1_Q,
RX2_I,
RX2_Q,
DATA_SYNC,
ADC_OTR,
DAC_OTR,
ADC_IN,
ADC_RAW,
adcclk_in,
FLASH_data_in,
FLASH_busy,
in_empty,
dacclk_in,

DATA_BUS,
NCO1_freq,
preamp_enable,
rx1,
tx,
TX_I,
TX_Q,
reset_adc_n,
stage_debug,
FLASH_data_out,
FLASH_enable,
FLASH_continue_read,
ADC_PGA,
ADC_RAND,
ADC_SHDN,
ADC_DITH,
reset_dac_n,
CICFIR_GAIN,
TX_CICFIR_GAIN,
DAC_GAIN,
ADC_OFFSET,
NCO2_freq,
rx2,
tx_iq_valid,
TCXO_divider,
VCXO_divider,
DAC_div0,
DAC_div1,
DAC_hp1,
DAC_hp2,
DAC_x4,
DCDC_freq,
TX_NCO_freq,
RX_CIC_RATE,
IQ_RX_READ_REQ,
IQ_RX_READ_CLK,
DAC_DRV_A0,
DAC_DRV_A1,
TX_CLK_SELECT,
TX_CIC_RATE,
);

input clk_in;
input signed [23:0] RX1_I;
input signed [23:0] RX1_Q;
input signed [23:0] RX2_I;
input signed [23:0] RX2_Q;
input DATA_SYNC;
input ADC_OTR;
input DAC_OTR;
input signed [15:0] ADC_IN;
input signed [15:0] ADC_RAW;
input adcclk_in;
input unsigned [7:0] FLASH_data_in;
input FLASH_busy;
input in_empty;
input dacclk_in;

output reg unsigned [31:0] NCO1_freq = 242347;
output reg unsigned [31:0] NCO2_freq = 242347;
output reg unsigned [31:0] TX_NCO_freq = 242347;
output reg preamp_enable = 0;
output reg rx1 = 1;
output reg rx2 = 0;
output reg tx = 0;
output reg reset_adc_n = 0;
output reg signed [23:0] TX_I = 'd0;
output reg signed [23:0] TX_Q = 'd0;
output reg [15:0] stage_debug = 0;
output reg unsigned [7:0] FLASH_data_out = 0;
output reg FLASH_enable = 0;
output reg FLASH_continue_read = 0;
output reg ADC_PGA = 0;
output reg ADC_RAND = 0;
output reg ADC_SHDN = 1;
output reg ADC_DITH = 0;
output reg reset_dac_n = 0;
output reg IQ_RX_READ_REQ = 0;
output reg IQ_RX_READ_CLK = 0;
output reg unsigned [7:0] CICFIR_GAIN = 32;
output reg unsigned [7:0] TX_CICFIR_GAIN = 32;
output reg unsigned [7:0] DAC_GAIN = 0;
output reg signed [15:0] ADC_OFFSET = 'd0;
output reg tx_iq_valid = 0;
output reg [15:0] TCXO_divider = 'd0;
output reg [15:0] VCXO_divider = 'd0;
output reg DAC_div0 = 0;
output reg DAC_div1 = 0;
output reg DAC_hp1 = 0;
output reg DAC_hp2 = 0;
output reg DAC_x4 = 0;
output reg DCDC_freq = 0;
output reg DAC_DRV_A0 = 1;
output reg DAC_DRV_A1 = 1;
output reg unsigned [10:0] RX_CIC_RATE = 'd640;
output reg TX_CLK_SELECT = 1;
output reg unsigned [11:0] TX_CIC_RATE = 'd2240;

inout [7:0] DATA_BUS;
reg   [7:0] DATA_BUS_OUT;
reg         DATA_BUS_OE; // 1 - out 0 - in
assign DATA_BUS = DATA_BUS_OE ? DATA_BUS_OUT : 8'bZ ;

reg signed [15:0] k = 'd1;
reg signed [15:0] ADC_MIN;
reg signed [15:0] ADC_MAX;
reg signed [23:0] READ_RX1_I;
reg signed [23:0] READ_RX1_Q;
reg signed [23:0] READ_RX2_I;
reg signed [23:0] READ_RX2_Q;
reg signed [23:0] READ_TX_I;
reg signed [23:0] READ_TX_Q;
reg signed [31:0] READ_NCO1_freq;
reg signed [31:0] READ_NCO2_freq;
reg signed [31:0] READ_TX_NCO_freq;
reg ADC_MINMAX_RESET;
reg sync_reset_rx_n = 0;
reg sync_reset_tx_n = 0;
reg unsigned [7:0] BUS_TEST;

always @ (posedge clk_in)
begin
	//начало передачи
	if (DATA_SYNC == 1)
	begin
		DATA_BUS_OE = 0;
		ADC_MINMAX_RESET = 0;
		FLASH_continue_read = 0;
		
		if(DATA_BUS[7:0] == 'd0) //BUS TEST
		begin
			k = 500;
		end
		else if(DATA_BUS[7:0] == 'd1) //GET PARAMS
		begin
			k = 100;
		end
		else if(DATA_BUS[7:0] == 'd2) //SEND PARAMS
		begin
			DATA_BUS_OE = 1;
			k = 200;
		end
		else if(DATA_BUS[7:0] == 'd3) //TX IQ
		begin
			tx_iq_valid = 0;
			k = 300;
		end
		else if(DATA_BUS[7:0] == 'd4) //RX IQ
		begin
			IQ_RX_READ_REQ = 1;
			IQ_RX_READ_CLK = 1;
			DATA_BUS_OE = 1;
			k = 400;
		end
		else if(DATA_BUS[7:0] == 'd5) //RESET RX ON
		begin
			sync_reset_rx_n = 0;
			k = 999;
		end
		else if(DATA_BUS[7:0] == 'd6) //RESET RX OFF
		begin
			sync_reset_rx_n = 1;
			k = 999;
		end
		else if(DATA_BUS[7:0] == 'd7) //FPGA FLASH READ
		begin
			FLASH_enable = 0;
			k = 700;
		end
		else if(DATA_BUS[7:0] == 'd8) //GET INFO
		begin
			DATA_BUS_OE = 1;
			k = 800;
		end
		else if(DATA_BUS[7:0] == 'd9) //RESET TX ON
		begin
			sync_reset_tx_n = 0;
			k = 999;
		end
		else if(DATA_BUS[7:0] == 'd10) //RESET TX OFF
		begin
			sync_reset_tx_n = 1;
			k = 999;
		end
	end
	
	else if (k == 100) //GET PARAMS
	begin
		rx1 = DATA_BUS[0:0];
		rx2 = DATA_BUS[1:1];
		tx = DATA_BUS[2:2];
		ADC_DITH = DATA_BUS[3:3];
		ADC_SHDN = DATA_BUS[4:4];
		ADC_RAND = DATA_BUS[5:5];
		ADC_PGA = DATA_BUS[6:6];
		preamp_enable = DATA_BUS[7:7];
		//clear TX chain
		if(tx == 0)
		begin
			TX_I[23:0] = 23'd0;
			TX_Q[23:0] = 23'd0;
			tx_iq_valid = 1;
		end
		//
		k = 101;
	end
	else if (k == 101)
	begin
		READ_NCO1_freq[31:24] = DATA_BUS[7:0];
		k = 102;
	end
	else if (k == 102)
	begin
		READ_NCO1_freq[23:16] = DATA_BUS[7:0];
		k = 103;
	end
	else if (k == 103)
	begin
		READ_NCO1_freq[15:8] = DATA_BUS[7:0];
		k = 104;
	end
	else if (k == 104)
	begin
		READ_NCO1_freq[7:0] = DATA_BUS[7:0];
		NCO1_freq[31:0] = READ_NCO1_freq[31:0];
		k = 105;
	end
	else if (k == 105)
	begin
		READ_NCO2_freq[31:24] = DATA_BUS[7:0];
		k = 106;
	end
	else if (k == 106)
	begin
		READ_NCO2_freq[23:16] = DATA_BUS[7:0];
		k = 107;
	end
	else if (k == 107)
	begin
		READ_NCO2_freq[15:8] = DATA_BUS[7:0];
		k = 108;
	end
	else if (k == 108)
	begin
		READ_NCO2_freq[7:0] = DATA_BUS[7:0];
		NCO2_freq[31:0] = READ_NCO2_freq[31:0];
		k = 109;
	end
	else if (k == 109)
	begin
		CICFIR_GAIN[7:0] = DATA_BUS[7:0];
		k = 110;
	end
	else if (k == 110)
	begin
		TX_CICFIR_GAIN[7:0] = DATA_BUS[7:0];
		k = 111;
	end
	else if (k == 111)
	begin
		DAC_GAIN[7:0] = DATA_BUS[7:0];
		k = 112;
	end
	else if (k == 112)
	begin
		ADC_OFFSET[15:8] = DATA_BUS[7:0];
		k = 113;
	end
	else if (k == 113)
	begin
		ADC_OFFSET[7:0] = DATA_BUS[7:0];
		k = 114;
	end
	else if (k == 114)
	begin
		TCXO_divider[15:8] = DATA_BUS[7:0];
		k = 115;
	end
	else if (k == 115)
	begin
		TCXO_divider[7:0] = DATA_BUS[7:0];
		k = 116;
	end
	else if (k == 116)
	begin
		VCXO_divider[15:8] = DATA_BUS[7:0];
		k = 117;
	end
	else if (k == 117)
	begin
		VCXO_divider[7:0] = DATA_BUS[7:0];
		k = 118;
	end
	else if (k == 118)
	begin
		DAC_div0 = DATA_BUS[0:0];
		DAC_div1 = DATA_BUS[1:1];
		DAC_hp1 = DATA_BUS[2:2];
		DAC_hp2 = DATA_BUS[3:3];
		DAC_x4 = DATA_BUS[4:4];
		DCDC_freq = DATA_BUS[5:5];
		
		if(DATA_BUS[7:6] =='d0)
			RX_CIC_RATE = 'd160;
		else if(DATA_BUS[7:6] =='d1)
			RX_CIC_RATE = 'd320;
		else if(DATA_BUS[7:6] =='d2)
			RX_CIC_RATE = 'd640;
		else if(DATA_BUS[7:6] =='d3)
			RX_CIC_RATE = 'd1280;
		
		k = 119;
	end
	else if (k == 119)
	begin
		READ_TX_NCO_freq[31:24] = DATA_BUS[7:0];
		k = 120;
	end
	else if (k == 120)
	begin
		READ_TX_NCO_freq[23:16] = DATA_BUS[7:0];
		k = 121;
	end
	else if (k == 121)
	begin
		READ_TX_NCO_freq[15:8] = DATA_BUS[7:0];
		k = 122;
	end
	else if (k == 122)
	begin
		READ_TX_NCO_freq[7:0] = DATA_BUS[7:0];
		TX_NCO_freq[31:0] = READ_TX_NCO_freq[31:0];
		k = 123;
	end
	else if (k == 123)
	begin
		DAC_DRV_A0 = DATA_BUS[0:0];
		DAC_DRV_A1 = DATA_BUS[1:1];
		TX_CLK_SELECT = DATA_BUS[2:2];
		if (DATA_BUS[2:2] == 1)
		begin
			TX_CIC_RATE = 'd2240;
		end
		else
		begin
			TX_CIC_RATE = 'd1600;
		end
		k = 999;
	end
	
	else if (k == 200) //SEND PARAMS
	begin
		DATA_BUS_OUT[0:0] = ADC_OTR;
		DATA_BUS_OUT[1:1] = DAC_OTR;
		//DATA_BUS_OUT[2:2] = iq_overrun;
		k = 201;
	end
	else if (k == 201)
	begin
		//iq_overrun = 0;
		DATA_BUS_OUT[7:0] = ADC_MIN[15:8];
		k = 202;
	end
	else if (k == 202)
	begin
		DATA_BUS_OUT[7:0] = ADC_MIN[7:0];
		k = 203;
	end
	else if (k == 203)
	begin
		DATA_BUS_OUT[7:0] = ADC_MAX[15:8];
		k = 204;
	end
	else if (k == 204)
	begin
		DATA_BUS_OUT[7:0] = ADC_MAX[7:0];
		ADC_MINMAX_RESET=1;
		k = 205;
	end
	else if (k == 205)
	begin
		DATA_BUS_OUT[7:0] = ADC_RAW[15:8];
		k = 206;
	end
	else if (k == 206)
	begin
		DATA_BUS_OUT[7:0] = ADC_RAW[7:0];
		k = 999;
	end
	
	else if (k == 300) //TX IQ
	begin
		READ_TX_Q[23:16] = DATA_BUS[7:0];
		k = 301;
	end
	else if (k == 301)
	begin
		READ_TX_Q[15:8] = DATA_BUS[7:0];
		k = 302;
	end
	else if (k == 302)
	begin
		READ_TX_Q[7:0] = DATA_BUS[7:0];
		k = 303;
	end
	else if (k == 303)
	begin
		READ_TX_I[23:16] = DATA_BUS[7:0];
		k = 304;
	end
	else if (k == 304)
	begin
		READ_TX_I[15:8] = DATA_BUS[7:0];
		k = 305;
	end
	else if (k == 305)
	begin
		READ_TX_I[7:0] = DATA_BUS[7:0];
		TX_Q[23:0] = READ_TX_Q[23:0];
		TX_I[23:0] = READ_TX_I[23:0];
		tx_iq_valid = 1;
		k = 999;
	end
	
	else if (k == 400) //RX1 IQ
	begin
		IQ_RX_READ_CLK = 0;
		READ_RX1_I[23:0] = RX1_I[23:0];
		READ_RX1_Q[23:0] = RX1_Q[23:0];
		READ_RX2_I[23:0] = RX2_I[23:0];
		READ_RX2_Q[23:0] = RX2_Q[23:0];
		DATA_BUS_OUT[7:0] = READ_RX1_Q[23:16];
		k = 401;
	end
	else if (k == 401)
	begin
		
		DATA_BUS_OUT[7:0] = READ_RX1_Q[15:8];
		k = 402;
	end
	else if (k == 402)
	begin
		DATA_BUS_OUT[7:0] = READ_RX1_Q[7:0];
		k = 403;
	end
	else if (k == 403)
	begin
		DATA_BUS_OUT[7:0] = READ_RX1_I[23:16];
		k = 404;
	end
	else if (k == 404)
	begin
		DATA_BUS_OUT[7:0] = READ_RX1_I[15:8];
		k = 405;
	end
	else if (k == 405)
	begin
		DATA_BUS_OUT[7:0] = READ_RX1_I[7:0];
		if(rx2 == 1)
			k = 406;
		else
		begin
			IQ_RX_READ_REQ = 1;
			IQ_RX_READ_CLK = 1;
			k = 400;
		end
	end
	else if (k == 406) //RX2 IQ
	begin
		DATA_BUS_OUT[7:0] = READ_RX2_Q[23:16];
		k = 407;
	end
	else if (k == 407)
	begin
		DATA_BUS_OUT[7:0] = READ_RX2_Q[15:8];
		k = 408;
	end
	else if (k == 408)
	begin
		DATA_BUS_OUT[7:0] = READ_RX2_Q[7:0];
		k = 409;
	end
	else if (k == 409)
	begin
		DATA_BUS_OUT[7:0] = READ_RX2_I[23:16];
		k = 410;
	end
	else if (k == 410)
	begin
		DATA_BUS_OUT[7:0] = READ_RX2_I[15:8];
		k = 411;
	end
	else if (k == 411)
	begin
		DATA_BUS_OUT[7:0] = READ_RX2_I[7:0];
		IQ_RX_READ_REQ = 1;
		IQ_RX_READ_CLK = 1;
		k = 400;
	end
	
	else if (k == 500) //BUS TEST
	begin
		BUS_TEST[7:0] = DATA_BUS[7:0];
		k = 501;
	end
	else if (k == 501)
	begin
		DATA_BUS_OE = 1;
		DATA_BUS_OUT[7:0] = BUS_TEST[7:0];
		k = 500;
	end
	
	else if (k == 700) //FPGA FLASH READ - SEND COMMAND
	begin
		DATA_BUS_OE = 0;
		FLASH_data_out[7:0] = DATA_BUS[7:0];
		if(FLASH_enable == 0)
			FLASH_enable = 1;
		else
			FLASH_continue_read = 1;
		k = 701;
	end
	else if (k == 701) //FPGA FLASH READ - READ ANSWER
	begin
		FLASH_continue_read = 0;
		DATA_BUS_OE = 1;
		if(FLASH_busy)
			DATA_BUS_OUT[7:0] = 'd255;
		else
			DATA_BUS_OUT[7:0] = FLASH_data_in[7:0];
		k = 700;
	end
	
	else if (k == 800) //GET INFO
	begin
		DATA_BUS_OUT[7:0] = 'd8; //flash id 1
		k = 801;
	end
	else if (k == 801)
	begin
		DATA_BUS_OUT[7:0] = 'd2; //flash id 2
		k = 802;
	end
	else if (k == 802)
	begin
		DATA_BUS_OUT[7:0] = 'd0; //flash id 3
		k = 999;
	end
	stage_debug=k;
end

always @ (posedge adcclk_in)
begin
	//ADC MIN-MAX
	if(ADC_MINMAX_RESET == 1)
	begin
		ADC_MIN = 'd32000;
		ADC_MAX = -16'd32000;
	end
	if(ADC_MAX<ADC_IN)
	begin
		ADC_MAX=ADC_IN;
	end
	if(ADC_MIN>ADC_IN)
	begin
		ADC_MIN=ADC_IN;
	end
end

always @ (negedge adcclk_in)
begin
	//RESET ADC SYNC
	reset_adc_n = sync_reset_rx_n;
end

always @ (negedge dacclk_in)
begin
	//RESET DAC SYNC
	reset_dac_n = sync_reset_tx_n;
end

endmodule
