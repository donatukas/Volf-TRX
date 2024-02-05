	component rx_cic is
		port (
			in_error          : in  std_logic_vector(1 downto 0)  := (others => 'X'); -- error
			in_valid          : in  std_logic                     := 'X';             -- valid
			in_ready          : out std_logic;                                        -- ready
			in0_data          : in  std_logic_vector(30 downto 0) := (others => 'X'); -- in0_data
			in1_data          : in  std_logic_vector(30 downto 0) := (others => 'X'); -- in1_data
			out_data          : out std_logic_vector(31 downto 0);                    -- out_data
			out_error         : out std_logic_vector(1 downto 0);                     -- error
			out_valid         : out std_logic;                                        -- valid
			out_ready         : in  std_logic                     := 'X';             -- ready
			out_startofpacket : out std_logic;                                        -- startofpacket
			out_endofpacket   : out std_logic;                                        -- endofpacket
			out_channel       : out std_logic;                                        -- channel
			clken             : in  std_logic                     := 'X';             -- clken
			clk               : in  std_logic                     := 'X';             -- clk
			rate              : in  std_logic_vector(10 downto 0) := (others => 'X'); -- conduit
			reset_n           : in  std_logic                     := 'X'              -- reset_n
		);
	end component rx_cic;

	u0 : component rx_cic
		port map (
			in_error          => CONNECTED_TO_in_error,          --  av_st_in.error
			in_valid          => CONNECTED_TO_in_valid,          --          .valid
			in_ready          => CONNECTED_TO_in_ready,          --          .ready
			in0_data          => CONNECTED_TO_in0_data,          --          .in0_data
			in1_data          => CONNECTED_TO_in1_data,          --          .in1_data
			out_data          => CONNECTED_TO_out_data,          -- av_st_out.out_data
			out_error         => CONNECTED_TO_out_error,         --          .error
			out_valid         => CONNECTED_TO_out_valid,         --          .valid
			out_ready         => CONNECTED_TO_out_ready,         --          .ready
			out_startofpacket => CONNECTED_TO_out_startofpacket, --          .startofpacket
			out_endofpacket   => CONNECTED_TO_out_endofpacket,   --          .endofpacket
			out_channel       => CONNECTED_TO_out_channel,       --          .channel
			clken             => CONNECTED_TO_clken,             --     clken.clken
			clk               => CONNECTED_TO_clk,               --     clock.clk
			rate              => CONNECTED_TO_rate,              --      rate.conduit
			reset_n           => CONNECTED_TO_reset_n            --     reset.reset_n
		);

