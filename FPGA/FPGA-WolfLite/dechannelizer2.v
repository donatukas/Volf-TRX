module dechannelizer2(
	input wire [23:0] in_data_1,
	input wire [23:0] in_data_2,
	input wire in_valid,
	input wire in_ready,
	input wire clk,
	input wire empty_1,
	input wire empty_2,
	input wire reset_n,

	output reg [23:0] out_data = 'd0,
	output reg out_valid = 'd0,
	output reg out_sop = 'd0,
	output reg out_eop = 'd0
);

reg signed [23:0] data_1_reg = 'd0;
reg signed [23:0] data_2_reg = 'd0;
reg [2:0] state = 'd0;

always @(posedge clk)
begin
	if(reset_n == 0)
	begin
		data_1_reg <= 0;
		data_2_reg <= 0;
		out_valid <= 0;
		out_sop <= 0;
		out_eop <= 0;
		state <= 0;
	end
	else if(state == 0 && in_valid && in_ready) //  && !empty_1 && !empty_2
	begin
		data_1_reg <= in_data_1;
		data_2_reg <= in_data_2;
		out_valid <= 0;
		out_sop <= 0;
		out_eop <= 0;
		state <= 'd1;
	end
	else if(state == 1)
	begin
		out_data <= data_1_reg;
		out_valid <= 1;
		out_sop <= 1;
		out_eop <= 0;
		state <= 'd2;
	end
	else if(state == 2)
	begin
		out_data <= data_2_reg;
		out_valid <= 1;
		out_sop <= 0;
		out_eop <= 1;
		state <= 'd3;
	end
	else if(state == 3)
	begin
		out_valid <= 0;
		out_sop <= 0;
		out_eop <= 0;
		state <= 'd4;
	end
	else if(state >= 4 && !in_valid)
	begin
		state <= 'd0;
	end
end

endmodule
