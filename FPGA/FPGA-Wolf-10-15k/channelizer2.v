module channelizer2(
	input wire [(width-1):0] in_data,
	input wire in_valid,
	input wire  channel,
	input wire in_ready_1,
	input wire in_ready_2,

	output reg [(width-1):0] out_data_1 = 'd0,
	output reg [(width-1):0] out_data_2 = 'd0,
	output wire out_valid_1,
	output wire out_valid_2,
	output wire out_ready
);

parameter width = 32;

assign out_valid_1 = (channel == 'd0) && in_valid;
assign out_valid_2 = (channel == 'd1) && in_valid;

assign out_ready = ((channel == 'd0) && in_ready_1) || ((channel == 'd1) && in_ready_2);

always @(posedge out_valid_1)
	out_data_1 <= in_data;
always @(posedge out_valid_2)
	out_data_2 <= in_data;
	
endmodule
