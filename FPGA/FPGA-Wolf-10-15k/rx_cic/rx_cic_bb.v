
module rx_cic (
	in_error,
	in_valid,
	in_ready,
	in0_data,
	in1_data,
	out_data,
	out_error,
	out_valid,
	out_ready,
	out_startofpacket,
	out_endofpacket,
	out_channel,
	clken,
	clk,
	rate,
	reset_n);	

	input	[1:0]	in_error;
	input		in_valid;
	output		in_ready;
	input	[30:0]	in0_data;
	input	[30:0]	in1_data;
	output	[31:0]	out_data;
	output	[1:0]	out_error;
	output		out_valid;
	input		out_ready;
	output		out_startofpacket;
	output		out_endofpacket;
	output		out_channel;
	input		clken;
	input		clk;
	input	[10:0]	rate;
	input		reset_n;
endmodule
