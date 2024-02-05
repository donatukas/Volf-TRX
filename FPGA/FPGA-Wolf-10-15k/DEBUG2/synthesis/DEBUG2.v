// DEBUG2.v

// Generated using ACDS version 18.1 625

`timescale 1 ps / 1 ps
module DEBUG2 (
		input  wire [2:0] probe  // probes.probe
	);

	altsource_probe_top #(
		.sld_auto_instance_index ("YES"),
		.sld_instance_index      (0),
		.instance_id             ("DBG2"),
		.probe_width             (3),
		.source_width            (0),
		.enable_metastability    ("NO")
	) in_system_sources_probes_0 (
		.probe (probe)  // probes.probe
	);

endmodule
