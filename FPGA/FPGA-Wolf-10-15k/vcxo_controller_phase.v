module vcxo_controller_phase(
vcxo_clk_in,
tcxo_clk_in,
TCXO_divider,
VCXO_divider,
pump
);

input vcxo_clk_in;
input tcxo_clk_in;
output pump;
input [15:0] TCXO_divider;
input [15:0] VCXO_divider;

wire ref_pwm;
wire osc_pwm;

reg out_ref = 0;
reg [15:0] count_ref = 0;
always @ (posedge tcxo_clk_in)
    begin
        if (count_ref == TCXO_divider)
             begin
                 count_ref <= 0;
                 out_ref <= !out_ref;
             end
         else count_ref <= count_ref + 1'b1;
    end
assign ref_pwm = out_ref;

reg out_vcxo = 0;
reg [15:0] count_vcxo = 0;
always @ (posedge vcxo_clk_in)
    begin
        if (count_vcxo == VCXO_divider)
             begin
                 count_vcxo <= 0;
                 out_vcxo <= !out_vcxo;
             end
         else count_vcxo <= count_vcxo + 1'b1;
    end
assign osc_pwm = out_vcxo;

//Apply to EXOR phase detector
assign pump = ref_pwm ^ osc_pwm;

endmodule
