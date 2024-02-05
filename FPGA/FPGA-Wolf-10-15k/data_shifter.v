module data_shifter(
	input wire [(in_width-1):0] data_in_I,
	input wire [(in_width-1):0] data_in_Q,
	input unsigned [7:0] distance,
	input clk_i,
	input clk_q,

	output reg [(out_width-1):0] data_out_I,
	output reg [(out_width-1):0] data_out_Q
);

parameter in_width = 88;
parameter out_width = 32;

wire signed [(in_width-1):0] shifted_i;
wire signed [(in_width-1):0] shifted_q;
wire signed [(in_width-1):0] rounded_i;
wire signed [(in_width-1):0] rounded_q;

assign shifted_i = data_in_I[(in_width-1):0] <<< distance;
assign shifted_q = data_in_Q[(in_width-1):0] <<< distance;

assign rounded_i = shifted_i[(in_width-1):0] + { {(out_width){1'b0}}, shifted_i[(in_width-out_width)], {(in_width-out_width-1){!shifted_i[(in_width-out_width)]}}};
assign rounded_q = shifted_q[(in_width-1):0] + { {(out_width){1'b0}}, shifted_q[(in_width-out_width)], {(in_width-out_width-1){!shifted_q[(in_width-out_width)]}}};

always @(posedge clk_i)
	data_out_I <= rounded_i[(in_width-1):(in_width-out_width)];

always @(posedge clk_q)
	data_out_Q <= rounded_q[(in_width-1):(in_width-out_width)];

endmodule
