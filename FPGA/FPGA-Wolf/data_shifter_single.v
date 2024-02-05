module data_shifter_single(
	input wire [(in_width-1):0] data_in,
	input unsigned [7:0] distance,
	//input wire valid,

	output wire [(out_width-1):0] data_out
);

parameter in_width = 88;
parameter out_width = 32;

wire signed [(in_width-1):0] shifted;
//wire signed [(in_width-1):0] rounded;

assign shifted = data_in[(in_width-1):0] <<< distance;

//assign rounded = shifted[(in_width-1):0] + { {(out_width){1'b0}}, shifted[(in_width-out_width)], {(in_width-out_width-1){!shifted[(in_width-out_width)]}}};

//assign data_out = rounded[(in_width-1):(in_width-out_width)];
assign data_out = shifted[(in_width-1):(in_width-out_width)];
	
endmodule
