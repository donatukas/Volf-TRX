module channelizer4(
	input wire [(width-1):0] in_data,
	input wire in_valid,
	input wire [1:0] channel,
	input wire in_ready_1,
	input wire in_ready_2,
	input wire in_ready_3,
	input wire in_ready_4,

	output reg [(width-1):0] out_data_1 = 'd0,
	output reg [(width-1):0] out_data_2 = 'd0,
	output reg [(width-1):0] out_data_3 = 'd0,
	output reg [(width-1):0] out_data_4 = 'd0,
	output wire out_valid_1,
	output wire out_valid_2,
	output wire out_valid_3,
	output wire out_valid_4,
	output wire out_ready
);

parameter width = 32;

assign out_valid_1 = (channel == 'd0) && in_valid;
assign out_valid_2 = (channel == 'd1) && in_valid;
assign out_valid_3 = (channel == 'd2) && in_valid;
assign out_valid_4 = (channel == 'd3) && in_valid;

assign out_ready = ((channel == 'd0) && in_ready_1) || ((channel == 'd1) && in_ready_2) || ((channel == 'd2) && in_ready_3) || ((channel == 'd3) && in_ready_4);

always @(posedge out_valid_1)
	out_data_1 <= in_data;
always @(posedge out_valid_2)
	out_data_2 <= in_data;
always @(posedge out_valid_3)
	out_data_3 <= in_data;
always @(posedge out_valid_4)
	out_data_4 <= in_data;
	
endmodule
