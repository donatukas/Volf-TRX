set_time_format -unit ns -decimal_places 3

create_clock -name "clock_crystal" -period 61.440MHz [get_ports {clk_sys}]
create_clock -name "clock_stm32" -period 50MHz [get_ports {STM32_CLK}]
create_clock -name "RX_IQ_ST_CLK" -period 0.384MHz {stm32_interface:STM32_INTERFACE|IQ_RX_READ_CLK}
create_clock -name "TX_IQ_ST_CLK" -period 0.048MHz {stm32_interface:STM32_INTERFACE|tx_iq_valid} 

set_clock_groups -exclusive -group clock_crystal -group clock_stm32 -group RX_IQ_ST_CLK -group TX_IQ_ST_CLK

derive_clock_uncertainty
derive_pll_clocks -create_base_clocks

set_output_delay -clock TX_PLL|altpll_component|auto_generated|pll1|clk[0] -max 0.6ns [get_ports {DAC_CLK}]
set_output_delay -clock TX_PLL|altpll_component|auto_generated|pll1|clk[0] -min 0.5ns [get_ports {DAC_CLK}]
set_output_delay -clock TX_PLL|altpll_component|auto_generated|pll1|clk[0] -max 0.5ns [get_ports {DAC_OUTPUT[*]}]
set_output_delay -clock TX_PLL|altpll_component|auto_generated|pll1|clk[0] -min 0.1ns [get_ports {DAC_OUTPUT[*]}]
set_output_delay -clock clock_stm32 -max 36ps [get_ports {STM32_DATA_BUS[*]}]
set_output_delay -clock clock_stm32 -min 0ps [get_ports {STM32_DATA_BUS[*]}]
set_output_delay -clock MAIN_PLL|altpll_component|auto_generated|pll1|clk[1] -max 36ps [get_ports {AUDIO_48K_CLOCK}]
set_output_delay -clock MAIN_PLL|altpll_component|auto_generated|pll1|clk[1] -min 0ps [get_ports {AUDIO_48K_CLOCK}]
set_output_delay -clock MAIN_PLL|altpll_component|auto_generated|pll1|clk[0] -max 36ps [get_ports {AUDIO_I2S_CLOCK}]
set_output_delay -clock MAIN_PLL|altpll_component|auto_generated|pll1|clk[0] -min 0ps [get_ports {AUDIO_I2S_CLOCK}]
set_output_delay -clock clock_stm32 1 [get_ports {DAC_PD}]
set_output_delay -clock clock_stm32 1 [get_ports {PREAMP}]

set_input_delay -clock clock_crystal -max 36ps [get_ports ADC_INPUT[*]]
set_input_delay -clock clock_crystal -min 0ps [get_ports ADC_INPUT[*]]
set_input_delay -clock clock_crystal -max 36ps [get_ports ADC_OTR]
set_input_delay -clock clock_crystal -min 0ps [get_ports ADC_OTR]
set_input_delay -clock clock_stm32 -max 36ps [get_ports STM32_DATA_BUS[*]]
set_input_delay -clock clock_stm32 -min 0ps [get_ports STM32_DATA_BUS[*]]
set_input_delay -clock clock_stm32 -max 36ps [get_ports STM32_SYNC]
set_input_delay -clock clock_stm32 -min 0ps [get_ports STM32_SYNC]

set_false_path -from [get_clocks {clock_crystal}] -to [get_clocks {TX_PLL|altpll_component|auto_generated|pll1|clk[0]}]
set_false_path -from [get_clocks {TX_IQ_ST_CLK}] -to [get_clocks {TX_PLL|altpll_component|auto_generated|pll1|clk[0]}]
set_false_path -from [get_clocks {TX_IQ_ST_CLK}] -to [get_clocks {MAIN_PLL|altpll_component|auto_generated|pll1|clk[1]}]
set_false_path -from [get_clocks {TX_PLL|altpll_component|auto_generated|pll1|clk[0]}] -to [get_clocks {clock_crystal}]
set_false_path -from [get_clocks {TX_PLL|altpll_component|auto_generated|pll1|clk[0]}] -to [get_clocks {clock_stm32}]
set_false_path -from [get_clocks {TX_PLL|altpll_component|auto_generated|pll1|clk[0]}] -to [get_clocks {TX_IQ_ST_CLK}]
set_false_path -from [get_clocks {MAIN_PLL|altpll_component|auto_generated|pll1|clk[1]}] -to [get_clocks {TX_IQ_ST_CLK}]
set_false_path -from [get_clocks {MAIN_PLL|altpll_component|auto_generated|pll1|clk[1]}] -to [get_clocks {TX_PLL|altpll_component|auto_generated|pll1|clk[0]}]
set_false_path -from [get_clocks {clock_stm32}] -to [get_clocks {TX_PLL|altpll_component|auto_generated|pll1|clk[0]}]
set_false_path -from [get_clocks {clock_stm32}] -to [get_clocks {MAIN_PLL|altpll_component|auto_generated|pll1|clk[0]}]
set_false_path -from [get_clocks {clock_stm32}] -to [get_clocks {MAIN_PLL|altpll_component|auto_generated|pll1|clk[1]}]
set_false_path -from [get_clocks {MAIN_PLL|altpll_component|auto_generated|pll1|clk[0]}] -to [get_clocks {clock_stm32}]
set_false_path -from [get_clocks {TX_PLL|altpll_component|auto_generated|pll1|clk[0]}] -to [get_clocks {MAIN_PLL|altpll_component|auto_generated|pll1|clk[1]}]
