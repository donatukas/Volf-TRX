-- ------------------------------------------------------------------------- 
-- High Level Design Compiler for Intel(R) FPGAs Version 22.1std (Release Build #915)
-- Quartus Prime development tool and MATLAB/Simulink Interface
-- 
-- Legal Notice: Copyright 2022 Intel Corporation.  All rights reserved.
-- Your use of  Intel Corporation's design tools,  logic functions and other
-- software and  tools, and its AMPP partner logic functions, and any output
-- files any  of the foregoing (including  device programming  or simulation
-- files), and  any associated  documentation  or information  are expressly
-- subject  to the terms and  conditions of the  Intel FPGA Software License
-- Agreement, Intel MegaCore Function License Agreement, or other applicable
-- license agreement,  including,  without limitation,  that your use is for
-- the  sole  purpose of  programming  logic devices  manufactured by  Intel
-- and  sold by Intel  or its authorized  distributors. Please refer  to the
-- applicable agreement for further details.
-- ---------------------------------------------------------------------------

-- VHDL created from rx_ciccomp_rtl_core
-- VHDL created on Sun Jan 29 14:51:47 2023


library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.NUMERIC_STD.all;
use IEEE.MATH_REAL.all;
use std.TextIO.all;
use work.dspba_library_package.all;

LIBRARY altera_mf;
USE altera_mf.altera_mf_components.all;
LIBRARY lpm;
USE lpm.lpm_components.all;

entity rx_ciccomp_rtl_core is
    port (
        xIn_v : in std_logic_vector(0 downto 0);  -- sfix1
        xIn_c : in std_logic_vector(7 downto 0);  -- sfix8
        xIn_0 : in std_logic_vector(31 downto 0);  -- sfix32
        enable_i : in std_logic_vector(0 downto 0);  -- sfix1
        xOut_v : out std_logic_vector(0 downto 0);  -- ufix1
        xOut_c : out std_logic_vector(7 downto 0);  -- ufix8
        xOut_0 : out std_logic_vector(61 downto 0);  -- sfix62
        clk : in std_logic;
        areset : in std_logic
    );
end rx_ciccomp_rtl_core;

architecture normal of rx_ciccomp_rtl_core is

    attribute altera_attribute : string;
    attribute altera_attribute of normal : architecture is "-name AUTO_SHIFT_REGISTER_RECOGNITION OFF; -name PHYSICAL_SYNTHESIS_REGISTER_DUPLICATION ON; -name MESSAGE_DISABLE 10036; -name MESSAGE_DISABLE 10037; -name MESSAGE_DISABLE 14130; -name MESSAGE_DISABLE 14320; -name MESSAGE_DISABLE 15400; -name MESSAGE_DISABLE 14130; -name MESSAGE_DISABLE 10036; -name MESSAGE_DISABLE 12020; -name MESSAGE_DISABLE 12030; -name MESSAGE_DISABLE 12010; -name MESSAGE_DISABLE 12110; -name MESSAGE_DISABLE 14320; -name MESSAGE_DISABLE 13410; -name MESSAGE_DISABLE 113007";
    
    signal GND_q : STD_LOGIC_VECTOR (0 downto 0);
    signal VCC_q : STD_LOGIC_VECTOR (0 downto 0);
    signal d_xIn_0_14_q : STD_LOGIC_VECTOR (31 downto 0);
    signal d_in0_m0_wi0_wo0_assign_id1_q_11_q : STD_LOGIC_VECTOR (0 downto 0);
    signal d_in0_m0_wi0_wo0_assign_id1_q_14_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_inputframe_seq_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_inputframe_seq_eq : std_logic;
    signal u0_m0_wo0_run_count : STD_LOGIC_VECTOR (2 downto 0);
    signal u0_m0_wo0_run_preEnaQ : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_run_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_run_out : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_run_enableQ : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_run_ctrl : STD_LOGIC_VECTOR (2 downto 0);
    signal u0_m0_wo0_memread_q : STD_LOGIC_VECTOR (0 downto 0);
    signal d_u0_m0_wo0_memread_q_20_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_compute_q : STD_LOGIC_VECTOR (0 downto 0);
    signal d_u0_m0_wo0_compute_q_14_q : STD_LOGIC_VECTOR (0 downto 0);
    signal d_u0_m0_wo0_compute_q_18_q : STD_LOGIC_VECTOR (0 downto 0);
    signal d_u0_m0_wo0_compute_q_19_q : STD_LOGIC_VECTOR (0 downto 0);
    signal d_u0_m0_wo0_compute_q_20_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_count0_inner_q : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_count0_inner_i : SIGNED (7 downto 0);
    attribute preserve : boolean;
    attribute preserve of u0_m0_wo0_wi0_r0_ra0_count0_inner_i : signal is true;
    signal u0_m0_wo0_wi0_r0_ra0_count0_q : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_count0_i : UNSIGNED (6 downto 0);
    attribute preserve of u0_m0_wo0_wi0_r0_ra0_count0_i : signal is true;
    signal u0_m0_wo0_wi0_r0_ra0_count1_q : STD_LOGIC_VECTOR (1 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_count1_i : UNSIGNED (0 downto 0);
    attribute preserve of u0_m0_wo0_wi0_r0_ra0_count1_i : signal is true;
    signal u0_m0_wo0_wi0_r0_ra0_count2_inner_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_count2_inner_i : SIGNED (0 downto 0);
    attribute preserve of u0_m0_wo0_wi0_r0_ra0_count2_inner_i : signal is true;
    signal u0_m0_wo0_wi0_r0_ra0_count2_inner_eq : std_logic;
    attribute preserve of u0_m0_wo0_wi0_r0_ra0_count2_inner_eq : signal is true;
    signal u0_m0_wo0_wi0_r0_ra0_count2_q : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_count2_i : UNSIGNED (6 downto 0);
    attribute preserve of u0_m0_wo0_wi0_r0_ra0_count2_i : signal is true;
    signal u0_m0_wo0_wi0_r0_ra0_add_1_0_a : STD_LOGIC_VECTOR (9 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_add_1_0_b : STD_LOGIC_VECTOR (9 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_add_1_0_o : STD_LOGIC_VECTOR (9 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_add_1_0_q : STD_LOGIC_VECTOR (9 downto 0);
    signal u0_m0_wo0_wi0_r0_ra1_count0_q : STD_LOGIC_VECTOR (8 downto 0);
    signal u0_m0_wo0_wi0_r0_ra1_count0_i : UNSIGNED (7 downto 0);
    attribute preserve of u0_m0_wo0_wi0_r0_ra1_count0_i : signal is true;
    signal u0_m0_wo0_wi0_r0_ra1_count2_lutreg_q : STD_LOGIC_VECTOR (8 downto 0);
    signal u0_m0_wo0_wi0_r0_ra1_count2_inner_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_wi0_r0_ra1_count2_inner_i : SIGNED (0 downto 0);
    attribute preserve of u0_m0_wo0_wi0_r0_ra1_count2_inner_i : signal is true;
    signal u0_m0_wo0_wi0_r0_ra1_count2_inner_eq : std_logic;
    attribute preserve of u0_m0_wo0_wi0_r0_ra1_count2_inner_eq : signal is true;
    signal u0_m0_wo0_wi0_r0_ra1_count2_q : STD_LOGIC_VECTOR (5 downto 0);
    signal u0_m0_wo0_wi0_r0_ra1_count2_i : UNSIGNED (5 downto 0);
    attribute preserve of u0_m0_wo0_wi0_r0_ra1_count2_i : signal is true;
    signal u0_m0_wo0_wi0_r0_ra1_add_1_0_a : STD_LOGIC_VECTOR (10 downto 0);
    signal u0_m0_wo0_wi0_r0_ra1_add_1_0_b : STD_LOGIC_VECTOR (10 downto 0);
    signal u0_m0_wo0_wi0_r0_ra1_add_1_0_o : STD_LOGIC_VECTOR (10 downto 0);
    signal u0_m0_wo0_wi0_r0_ra1_add_1_0_q : STD_LOGIC_VECTOR (10 downto 0);
    signal u0_m0_wo0_wi0_r0_ra2_count0_q : STD_LOGIC_VECTOR (9 downto 0);
    signal u0_m0_wo0_wi0_r0_ra2_count0_i : UNSIGNED (8 downto 0);
    attribute preserve of u0_m0_wo0_wi0_r0_ra2_count0_i : signal is true;
    signal u0_m0_wo0_wi0_r0_ra2_count2_lutreg_q : STD_LOGIC_VECTOR (9 downto 0);
    signal u0_m0_wo0_wi0_r0_ra2_add_1_0_a : STD_LOGIC_VECTOR (11 downto 0);
    signal u0_m0_wo0_wi0_r0_ra2_add_1_0_b : STD_LOGIC_VECTOR (11 downto 0);
    signal u0_m0_wo0_wi0_r0_ra2_add_1_0_o : STD_LOGIC_VECTOR (11 downto 0);
    signal u0_m0_wo0_wi0_r0_ra2_add_1_0_q : STD_LOGIC_VECTOR (11 downto 0);
    signal u0_m0_wo0_wi0_r0_ra3_count2_lutreg_q : STD_LOGIC_VECTOR (8 downto 0);
    signal u0_m0_wo0_wi0_r0_ra3_add_1_0_a : STD_LOGIC_VECTOR (10 downto 0);
    signal u0_m0_wo0_wi0_r0_ra3_add_1_0_b : STD_LOGIC_VECTOR (10 downto 0);
    signal u0_m0_wo0_wi0_r0_ra3_add_1_0_o : STD_LOGIC_VECTOR (10 downto 0);
    signal u0_m0_wo0_wi0_r0_ra3_add_1_0_q : STD_LOGIC_VECTOR (10 downto 0);
    signal u0_m0_wo0_wi0_r0_we3_seq_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_wi0_r0_we3_seq_eq : std_logic;
    signal u0_m0_wo0_wi0_r0_we2_1_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_wi0_r0_we2_2_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_wi0_r0_we2_3_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_wi0_r0_we2_4_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_wi0_r0_wa0_q : STD_LOGIC_VECTOR (6 downto 0);
    signal u0_m0_wo0_wi0_r0_wa0_i : UNSIGNED (6 downto 0);
    attribute preserve of u0_m0_wo0_wi0_r0_wa0_i : signal is true;
    signal u0_m0_wo0_wi0_r0_wa1_q : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_wi0_r0_wa1_i : UNSIGNED (7 downto 0);
    attribute preserve of u0_m0_wo0_wi0_r0_wa1_i : signal is true;
    signal u0_m0_wo0_wi0_r0_wa2_q : STD_LOGIC_VECTOR (8 downto 0);
    signal u0_m0_wo0_wi0_r0_wa2_i : UNSIGNED (8 downto 0);
    attribute preserve of u0_m0_wo0_wi0_r0_wa2_i : signal is true;
    signal u0_m0_wo0_wi0_r0_wa3_q : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_wi0_r0_wa3_i : UNSIGNED (7 downto 0);
    attribute preserve of u0_m0_wo0_wi0_r0_wa3_i : signal is true;
    signal u0_m0_wo0_wi0_r0_memr0_reset0 : std_logic;
    signal u0_m0_wo0_wi0_r0_memr0_ia : STD_LOGIC_VECTOR (31 downto 0);
    signal u0_m0_wo0_wi0_r0_memr0_aa : STD_LOGIC_VECTOR (6 downto 0);
    signal u0_m0_wo0_wi0_r0_memr0_ab : STD_LOGIC_VECTOR (6 downto 0);
    signal u0_m0_wo0_wi0_r0_memr0_iq : STD_LOGIC_VECTOR (31 downto 0);
    signal u0_m0_wo0_wi0_r0_memr0_q : STD_LOGIC_VECTOR (31 downto 0);
    signal u0_m0_wo0_wi0_r0_memr1_reset0 : std_logic;
    signal u0_m0_wo0_wi0_r0_memr1_ia : STD_LOGIC_VECTOR (31 downto 0);
    signal u0_m0_wo0_wi0_r0_memr1_aa : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_wi0_r0_memr1_ab : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_wi0_r0_memr1_iq : STD_LOGIC_VECTOR (31 downto 0);
    signal u0_m0_wo0_wi0_r0_memr1_q : STD_LOGIC_VECTOR (31 downto 0);
    signal u0_m0_wo0_wi0_r0_memr2_reset0 : std_logic;
    signal u0_m0_wo0_wi0_r0_memr2_ia : STD_LOGIC_VECTOR (31 downto 0);
    signal u0_m0_wo0_wi0_r0_memr2_aa : STD_LOGIC_VECTOR (8 downto 0);
    signal u0_m0_wo0_wi0_r0_memr2_ab : STD_LOGIC_VECTOR (8 downto 0);
    signal u0_m0_wo0_wi0_r0_memr2_iq : STD_LOGIC_VECTOR (31 downto 0);
    signal u0_m0_wo0_wi0_r0_memr2_q : STD_LOGIC_VECTOR (31 downto 0);
    signal u0_m0_wo0_wi0_r0_memr3_reset0 : std_logic;
    signal u0_m0_wo0_wi0_r0_memr3_ia : STD_LOGIC_VECTOR (31 downto 0);
    signal u0_m0_wo0_wi0_r0_memr3_aa : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_wi0_r0_memr3_ab : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_wi0_r0_memr3_iq : STD_LOGIC_VECTOR (31 downto 0);
    signal u0_m0_wo0_wi0_r0_memr3_q : STD_LOGIC_VECTOR (31 downto 0);
    signal u0_m0_wo0_ca1_inner_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_ca1_inner_i : SIGNED (0 downto 0);
    attribute preserve of u0_m0_wo0_ca1_inner_i : signal is true;
    signal u0_m0_wo0_ca1_inner_eq : std_logic;
    attribute preserve of u0_m0_wo0_ca1_inner_eq : signal is true;
    signal u0_m0_wo0_ca1_q : STD_LOGIC_VECTOR (5 downto 0);
    signal u0_m0_wo0_ca1_i : UNSIGNED (5 downto 0);
    attribute preserve of u0_m0_wo0_ca1_i : signal is true;
    signal u0_m0_wo0_cm0_q : STD_LOGIC_VECTOR (21 downto 0);
    signal u0_m0_wo0_cm1_q : STD_LOGIC_VECTOR (21 downto 0);
    signal u0_m0_wo0_sym_add0_a : STD_LOGIC_VECTOR (32 downto 0);
    signal u0_m0_wo0_sym_add0_b : STD_LOGIC_VECTOR (32 downto 0);
    signal u0_m0_wo0_sym_add0_o : STD_LOGIC_VECTOR (32 downto 0);
    signal u0_m0_wo0_sym_add0_q : STD_LOGIC_VECTOR (32 downto 0);
    signal u0_m0_wo0_sym_add1_a : STD_LOGIC_VECTOR (32 downto 0);
    signal u0_m0_wo0_sym_add1_b : STD_LOGIC_VECTOR (32 downto 0);
    signal u0_m0_wo0_sym_add1_o : STD_LOGIC_VECTOR (32 downto 0);
    signal u0_m0_wo0_sym_add1_q : STD_LOGIC_VECTOR (32 downto 0);
    signal u0_m0_wo0_mtree_add0_0_a : STD_LOGIC_VECTOR (55 downto 0);
    signal u0_m0_wo0_mtree_add0_0_b : STD_LOGIC_VECTOR (55 downto 0);
    signal u0_m0_wo0_mtree_add0_0_o : STD_LOGIC_VECTOR (55 downto 0);
    signal u0_m0_wo0_mtree_add0_0_q : STD_LOGIC_VECTOR (55 downto 0);
    signal u0_m0_wo0_adelay_q : STD_LOGIC_VECTOR (61 downto 0);
    signal u0_m0_wo0_aseq_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_aseq_eq : std_logic;
    signal u0_m0_wo0_accum_a : STD_LOGIC_VECTOR (61 downto 0);
    signal u0_m0_wo0_accum_b : STD_LOGIC_VECTOR (61 downto 0);
    signal u0_m0_wo0_accum_i : STD_LOGIC_VECTOR (61 downto 0);
    signal u0_m0_wo0_accum_o : STD_LOGIC_VECTOR (61 downto 0);
    signal u0_m0_wo0_accum_q : STD_LOGIC_VECTOR (61 downto 0);
    signal u0_m0_wo0_oseq_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_oseq_eq : std_logic;
    signal u0_m0_wo0_oseq_gated_reg_q : STD_LOGIC_VECTOR (0 downto 0);
    signal d_out0_m0_wo0_assign_id3_q_21_q : STD_LOGIC_VECTOR (0 downto 0);
    signal outchan_q : STD_LOGIC_VECTOR (1 downto 0);
    signal outchan_i : UNSIGNED (0 downto 0);
    attribute preserve of outchan_i : signal is true;
    signal u0_m0_wo0_mtree_mult1_1_im0_a0 : STD_LOGIC_VECTOR (15 downto 0);
    signal u0_m0_wo0_mtree_mult1_1_im0_b0 : STD_LOGIC_VECTOR (4 downto 0);
    signal u0_m0_wo0_mtree_mult1_1_im0_s1 : STD_LOGIC_VECTOR (20 downto 0);
    signal u0_m0_wo0_mtree_mult1_1_im0_reset : std_logic;
    signal u0_m0_wo0_mtree_mult1_1_im0_q : STD_LOGIC_VECTOR (20 downto 0);
    signal u0_m0_wo0_mtree_mult1_1_im3_a0 : STD_LOGIC_VECTOR (17 downto 0);
    signal u0_m0_wo0_mtree_mult1_1_im3_b0 : STD_LOGIC_VECTOR (15 downto 0);
    signal u0_m0_wo0_mtree_mult1_1_im3_s1 : STD_LOGIC_VECTOR (33 downto 0);
    signal u0_m0_wo0_mtree_mult1_1_im3_reset : std_logic;
    signal u0_m0_wo0_mtree_mult1_1_im3_q : STD_LOGIC_VECTOR (33 downto 0);
    signal u0_m0_wo0_mtree_mult1_1_im7_a0 : STD_LOGIC_VECTOR (17 downto 0);
    signal u0_m0_wo0_mtree_mult1_1_im7_b0 : STD_LOGIC_VECTOR (4 downto 0);
    signal u0_m0_wo0_mtree_mult1_1_im7_s1 : STD_LOGIC_VECTOR (22 downto 0);
    signal u0_m0_wo0_mtree_mult1_1_im7_reset : std_logic;
    signal u0_m0_wo0_mtree_mult1_1_im7_q : STD_LOGIC_VECTOR (22 downto 0);
    signal u0_m0_wo0_mtree_mult1_1_im11_a0 : STD_LOGIC_VECTOR (17 downto 0);
    signal u0_m0_wo0_mtree_mult1_1_im11_b0 : STD_LOGIC_VECTOR (17 downto 0);
    signal u0_m0_wo0_mtree_mult1_1_im11_s1 : STD_LOGIC_VECTOR (35 downto 0);
    signal u0_m0_wo0_mtree_mult1_1_im11_reset : std_logic;
    signal u0_m0_wo0_mtree_mult1_1_im11_q : STD_LOGIC_VECTOR (35 downto 0);
    signal u0_m0_wo0_mtree_mult1_1_result_add_0_0_a : STD_LOGIC_VECTOR (51 downto 0);
    signal u0_m0_wo0_mtree_mult1_1_result_add_0_0_b : STD_LOGIC_VECTOR (51 downto 0);
    signal u0_m0_wo0_mtree_mult1_1_result_add_0_0_o : STD_LOGIC_VECTOR (51 downto 0);
    signal u0_m0_wo0_mtree_mult1_1_result_add_0_0_q : STD_LOGIC_VECTOR (51 downto 0);
    signal u0_m0_wo0_mtree_mult1_1_result_add_0_1_a : STD_LOGIC_VECTOR (55 downto 0);
    signal u0_m0_wo0_mtree_mult1_1_result_add_0_1_b : STD_LOGIC_VECTOR (55 downto 0);
    signal u0_m0_wo0_mtree_mult1_1_result_add_0_1_o : STD_LOGIC_VECTOR (55 downto 0);
    signal u0_m0_wo0_mtree_mult1_1_result_add_0_1_q : STD_LOGIC_VECTOR (55 downto 0);
    signal u0_m0_wo0_mtree_mult1_1_result_add_1_0_a : STD_LOGIC_VECTOR (56 downto 0);
    signal u0_m0_wo0_mtree_mult1_1_result_add_1_0_b : STD_LOGIC_VECTOR (56 downto 0);
    signal u0_m0_wo0_mtree_mult1_1_result_add_1_0_o : STD_LOGIC_VECTOR (56 downto 0);
    signal u0_m0_wo0_mtree_mult1_1_result_add_1_0_q : STD_LOGIC_VECTOR (56 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im0_a0 : STD_LOGIC_VECTOR (15 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im0_b0 : STD_LOGIC_VECTOR (4 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im0_s1 : STD_LOGIC_VECTOR (20 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im0_reset : std_logic;
    signal u0_m0_wo0_mtree_mult1_0_im0_q : STD_LOGIC_VECTOR (20 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im3_a0 : STD_LOGIC_VECTOR (17 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im3_b0 : STD_LOGIC_VECTOR (15 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im3_s1 : STD_LOGIC_VECTOR (33 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im3_reset : std_logic;
    signal u0_m0_wo0_mtree_mult1_0_im3_q : STD_LOGIC_VECTOR (33 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im7_a0 : STD_LOGIC_VECTOR (17 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im7_b0 : STD_LOGIC_VECTOR (4 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im7_s1 : STD_LOGIC_VECTOR (22 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im7_reset : std_logic;
    signal u0_m0_wo0_mtree_mult1_0_im7_q : STD_LOGIC_VECTOR (22 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im11_a0 : STD_LOGIC_VECTOR (17 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im11_b0 : STD_LOGIC_VECTOR (17 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im11_s1 : STD_LOGIC_VECTOR (35 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_im11_reset : std_logic;
    signal u0_m0_wo0_mtree_mult1_0_im11_q : STD_LOGIC_VECTOR (35 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_a : STD_LOGIC_VECTOR (51 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_b : STD_LOGIC_VECTOR (51 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_o : STD_LOGIC_VECTOR (51 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_0_q : STD_LOGIC_VECTOR (51 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_1_a : STD_LOGIC_VECTOR (55 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_1_b : STD_LOGIC_VECTOR (55 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_1_o : STD_LOGIC_VECTOR (55 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_0_1_q : STD_LOGIC_VECTOR (55 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_a : STD_LOGIC_VECTOR (56 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_b : STD_LOGIC_VECTOR (56 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_o : STD_LOGIC_VECTOR (56 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_result_add_1_0_q : STD_LOGIC_VECTOR (56 downto 0);
    signal input_valid_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_inputframe_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_count0_run_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_count2_run_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_wi0_r0_ra1_count2_run_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_wi0_r0_we2_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_ca1_run_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_oseq_gated_q : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_BitSelect_for_a_in : STD_LOGIC_VECTOR (8 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_BitSelect_for_a_b : STD_LOGIC_VECTOR (4 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_BitSelect_for_b_in : STD_LOGIC_VECTOR (8 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_BitSelect_for_b_b : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_wi0_r0_ra1_add_0_0_replace_or_BitSelect_for_b_in : STD_LOGIC_VECTOR (9 downto 0);
    signal u0_m0_wo0_wi0_r0_ra1_add_0_0_replace_or_BitSelect_for_b_b : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_wi0_r0_ra2_add_0_0_replace_or_BitSelect_for_b_in : STD_LOGIC_VECTOR (10 downto 0);
    signal u0_m0_wo0_wi0_r0_ra2_add_0_0_replace_or_BitSelect_for_b_b : STD_LOGIC_VECTOR (0 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_resize_in : STD_LOGIC_VECTOR (6 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_resize_b : STD_LOGIC_VECTOR (6 downto 0);
    signal u0_m0_wo0_wi0_r0_ra1_add_0_0_replace_or_BitSelect_for_a_in : STD_LOGIC_VECTOR (9 downto 0);
    signal u0_m0_wo0_wi0_r0_ra1_add_0_0_replace_or_BitSelect_for_a_b : STD_LOGIC_VECTOR (5 downto 0);
    signal u0_m0_wo0_wi0_r0_ra1_count2_lut_q : STD_LOGIC_VECTOR (8 downto 0);
    signal u0_m0_wo0_wi0_r0_ra2_count2_lut_q : STD_LOGIC_VECTOR (9 downto 0);
    signal u0_m0_wo0_wi0_r0_ra3_count2_lut_q : STD_LOGIC_VECTOR (8 downto 0);
    signal u0_m0_wo0_wi0_r0_ra1_resize_in : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_wi0_r0_ra1_resize_b : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_wi0_r0_ra2_add_0_0_replace_or_BitSelect_for_a_in : STD_LOGIC_VECTOR (10 downto 0);
    signal u0_m0_wo0_wi0_r0_ra2_add_0_0_replace_or_BitSelect_for_a_b : STD_LOGIC_VECTOR (6 downto 0);
    signal u0_m0_wo0_wi0_r0_ra2_resize_in : STD_LOGIC_VECTOR (8 downto 0);
    signal u0_m0_wo0_wi0_r0_ra2_resize_b : STD_LOGIC_VECTOR (8 downto 0);
    signal u0_m0_wo0_wi0_r0_ra3_resize_in : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_wi0_r0_ra3_resize_b : STD_LOGIC_VECTOR (7 downto 0);
    signal u0_m0_wo0_mtree_mult1_1_bs1_merged_bit_select_b : STD_LOGIC_VECTOR (4 downto 0);
    signal u0_m0_wo0_mtree_mult1_1_bs1_merged_bit_select_c : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_bs1_merged_bit_select_b : STD_LOGIC_VECTOR (4 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_bs1_merged_bit_select_c : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_mtree_mult1_1_bs2_merged_bit_select_b : STD_LOGIC_VECTOR (15 downto 0);
    signal u0_m0_wo0_mtree_mult1_1_bs2_merged_bit_select_c : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_bs2_merged_bit_select_b : STD_LOGIC_VECTOR (15 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_bs2_merged_bit_select_c : STD_LOGIC_VECTOR (16 downto 0);
    signal u0_m0_wo0_mtree_mult1_1_align_21_q : STD_LOGIC_VECTOR (54 downto 0);
    signal u0_m0_wo0_mtree_mult1_1_align_21_qint : STD_LOGIC_VECTOR (54 downto 0);
    signal u0_m0_wo0_mtree_mult1_1_align_17_q : STD_LOGIC_VECTOR (50 downto 0);
    signal u0_m0_wo0_mtree_mult1_1_align_17_qint : STD_LOGIC_VECTOR (50 downto 0);
    signal u0_m0_wo0_mtree_mult1_1_align_19_q : STD_LOGIC_VECTOR (39 downto 0);
    signal u0_m0_wo0_mtree_mult1_1_align_19_qint : STD_LOGIC_VECTOR (39 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_align_21_q : STD_LOGIC_VECTOR (54 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_align_21_qint : STD_LOGIC_VECTOR (54 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_align_17_q : STD_LOGIC_VECTOR (50 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_align_17_qint : STD_LOGIC_VECTOR (50 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_align_19_q : STD_LOGIC_VECTOR (39 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_align_19_qint : STD_LOGIC_VECTOR (39 downto 0);
    signal u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_join_q : STD_LOGIC_VECTOR (8 downto 0);
    signal u0_m0_wo0_wi0_r0_ra1_add_0_0_replace_or_join_q : STD_LOGIC_VECTOR (9 downto 0);
    signal u0_m0_wo0_wi0_r0_ra2_add_0_0_replace_or_join_q : STD_LOGIC_VECTOR (10 downto 0);
    signal u0_m0_wo0_mtree_mult1_1_bjA5_q : STD_LOGIC_VECTOR (17 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_bjA5_q : STD_LOGIC_VECTOR (17 downto 0);
    signal u0_m0_wo0_mtree_mult1_1_bjB10_q : STD_LOGIC_VECTOR (17 downto 0);
    signal u0_m0_wo0_mtree_mult1_0_bjB10_q : STD_LOGIC_VECTOR (17 downto 0);

begin


    -- VCC(CONSTANT,1)@0
    VCC_q <= "1";

    -- input_valid(LOGICAL,3)@10
    input_valid_q <= xIn_v and enable_i;

    -- d_in0_m0_wi0_wo0_assign_id1_q_11(DELAY,181)@10 + 1
    d_in0_m0_wi0_wo0_assign_id1_q_11 : dspba_delay
    GENERIC MAP ( width => 1, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => input_valid_q, xout => d_in0_m0_wi0_wo0_assign_id1_q_11_q, clk => clk, aclr => areset );

    -- u0_m0_wo0_inputframe_seq(SEQUENCE,14)@10 + 1
    u0_m0_wo0_inputframe_seq_clkproc: PROCESS (clk, areset)
        variable u0_m0_wo0_inputframe_seq_c : SIGNED(3 downto 0);
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_inputframe_seq_c := "0001";
            u0_m0_wo0_inputframe_seq_q <= "0";
            u0_m0_wo0_inputframe_seq_eq <= '0';
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (input_valid_q = "1") THEN
                IF (u0_m0_wo0_inputframe_seq_c = "0000") THEN
                    u0_m0_wo0_inputframe_seq_eq <= '1';
                ELSE
                    u0_m0_wo0_inputframe_seq_eq <= '0';
                END IF;
                IF (u0_m0_wo0_inputframe_seq_eq = '1') THEN
                    u0_m0_wo0_inputframe_seq_c := u0_m0_wo0_inputframe_seq_c + 1;
                ELSE
                    u0_m0_wo0_inputframe_seq_c := u0_m0_wo0_inputframe_seq_c - 1;
                END IF;
                u0_m0_wo0_inputframe_seq_q <= STD_LOGIC_VECTOR(u0_m0_wo0_inputframe_seq_c(3 downto 3));
            END IF;
        END IF;
    END PROCESS;

    -- u0_m0_wo0_inputframe(LOGICAL,15)@11
    u0_m0_wo0_inputframe_q <= u0_m0_wo0_inputframe_seq_q and d_in0_m0_wi0_wo0_assign_id1_q_11_q;

    -- u0_m0_wo0_run(ENABLEGENERATOR,16)@11 + 2
    u0_m0_wo0_run_ctrl <= u0_m0_wo0_run_out & u0_m0_wo0_inputframe_q & u0_m0_wo0_run_enableQ;
    u0_m0_wo0_run_clkproc: PROCESS (clk, areset)
        variable u0_m0_wo0_run_enable_c : SIGNED(7 downto 0);
        variable u0_m0_wo0_run_inc : SIGNED(2 downto 0);
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_run_q <= "0";
            u0_m0_wo0_run_enable_c := TO_SIGNED(126, 8);
            u0_m0_wo0_run_enableQ <= "0";
            u0_m0_wo0_run_count <= "000";
            u0_m0_wo0_run_inc := (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (u0_m0_wo0_run_out = "1") THEN
                IF (u0_m0_wo0_run_enable_c(7) = '1') THEN
                    u0_m0_wo0_run_enable_c := u0_m0_wo0_run_enable_c - (-127);
                ELSE
                    u0_m0_wo0_run_enable_c := u0_m0_wo0_run_enable_c + (-1);
                END IF;
                u0_m0_wo0_run_enableQ <= STD_LOGIC_VECTOR(u0_m0_wo0_run_enable_c(7 downto 7));
            ELSE
                u0_m0_wo0_run_enableQ <= "0";
            END IF;
            CASE (u0_m0_wo0_run_ctrl) IS
                WHEN "000" | "001" => u0_m0_wo0_run_inc := "000";
                WHEN "010" | "011" => u0_m0_wo0_run_inc := "111";
                WHEN "100" => u0_m0_wo0_run_inc := "000";
                WHEN "101" => u0_m0_wo0_run_inc := "010";
                WHEN "110" => u0_m0_wo0_run_inc := "111";
                WHEN "111" => u0_m0_wo0_run_inc := "001";
                WHEN OTHERS => 
            END CASE;
            u0_m0_wo0_run_count <= STD_LOGIC_VECTOR(SIGNED(u0_m0_wo0_run_count) + SIGNED(u0_m0_wo0_run_inc));
            u0_m0_wo0_run_q <= u0_m0_wo0_run_out;
        END IF;
    END PROCESS;
    u0_m0_wo0_run_preEnaQ <= u0_m0_wo0_run_count(2 downto 2);
    u0_m0_wo0_run_out <= u0_m0_wo0_run_preEnaQ and VCC_q;

    -- u0_m0_wo0_memread(DELAY,17)@13
    u0_m0_wo0_memread : dspba_delay
    GENERIC MAP ( width => 1, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_run_q, xout => u0_m0_wo0_memread_q, clk => clk, aclr => areset );

    -- u0_m0_wo0_compute(DELAY,19)@13
    u0_m0_wo0_compute : dspba_delay
    GENERIC MAP ( width => 1, depth => 2, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_memread_q, xout => u0_m0_wo0_compute_q, clk => clk, aclr => areset );

    -- d_u0_m0_wo0_compute_q_14(DELAY,184)@13 + 1
    d_u0_m0_wo0_compute_q_14 : dspba_delay
    GENERIC MAP ( width => 1, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_compute_q, xout => d_u0_m0_wo0_compute_q_14_q, clk => clk, aclr => areset );

    -- d_u0_m0_wo0_compute_q_18(DELAY,185)@14 + 4
    d_u0_m0_wo0_compute_q_18 : dspba_delay
    GENERIC MAP ( width => 1, depth => 4, reset_kind => "ASYNC" )
    PORT MAP ( xin => d_u0_m0_wo0_compute_q_14_q, xout => d_u0_m0_wo0_compute_q_18_q, clk => clk, aclr => areset );

    -- d_u0_m0_wo0_compute_q_19(DELAY,186)@18 + 1
    d_u0_m0_wo0_compute_q_19 : dspba_delay
    GENERIC MAP ( width => 1, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => d_u0_m0_wo0_compute_q_18_q, xout => d_u0_m0_wo0_compute_q_19_q, clk => clk, aclr => areset );

    -- u0_m0_wo0_aseq(SEQUENCE,96)@19 + 1
    u0_m0_wo0_aseq_clkproc: PROCESS (clk, areset)
        variable u0_m0_wo0_aseq_c : SIGNED(9 downto 0);
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_aseq_c := "0000000000";
            u0_m0_wo0_aseq_q <= "0";
            u0_m0_wo0_aseq_eq <= '0';
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (d_u0_m0_wo0_compute_q_19_q = "1") THEN
                IF (u0_m0_wo0_aseq_c = "1111111111") THEN
                    u0_m0_wo0_aseq_eq <= '1';
                ELSE
                    u0_m0_wo0_aseq_eq <= '0';
                END IF;
                IF (u0_m0_wo0_aseq_eq = '1') THEN
                    u0_m0_wo0_aseq_c := u0_m0_wo0_aseq_c + 127;
                ELSE
                    u0_m0_wo0_aseq_c := u0_m0_wo0_aseq_c - 1;
                END IF;
                u0_m0_wo0_aseq_q <= STD_LOGIC_VECTOR(u0_m0_wo0_aseq_c(9 downto 9));
            END IF;
        END IF;
    END PROCESS;

    -- d_u0_m0_wo0_compute_q_20(DELAY,187)@19 + 1
    d_u0_m0_wo0_compute_q_20 : dspba_delay
    GENERIC MAP ( width => 1, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => d_u0_m0_wo0_compute_q_19_q, xout => d_u0_m0_wo0_compute_q_20_q, clk => clk, aclr => areset );

    -- d_u0_m0_wo0_memread_q_20(DELAY,183)@13 + 7
    d_u0_m0_wo0_memread_q_20 : dspba_delay
    GENERIC MAP ( width => 1, depth => 7, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_memread_q, xout => d_u0_m0_wo0_memread_q_20_q, clk => clk, aclr => areset );

    -- u0_m0_wo0_adelay(DELAY,95)@20
    u0_m0_wo0_adelay : dspba_delay
    GENERIC MAP ( width => 62, depth => 1, reset_kind => "NONE" )
    PORT MAP ( xin => u0_m0_wo0_accum_q, xout => u0_m0_wo0_adelay_q, ena => d_u0_m0_wo0_compute_q_20_q(0), clk => clk, aclr => areset );

    -- u0_m0_wo0_ca1_inner(COUNTER,82)@14
    -- low=-1, high=0, step=1, init=0
    u0_m0_wo0_ca1_inner_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_ca1_inner_i <= TO_SIGNED(0, 1);
            u0_m0_wo0_ca1_inner_eq <= '1';
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (d_u0_m0_wo0_compute_q_14_q = "1") THEN
                IF (u0_m0_wo0_ca1_inner_eq = '0') THEN
                    u0_m0_wo0_ca1_inner_eq <= '1';
                ELSE
                    u0_m0_wo0_ca1_inner_eq <= '0';
                END IF;
                IF (u0_m0_wo0_ca1_inner_eq = '1') THEN
                    u0_m0_wo0_ca1_inner_i <= u0_m0_wo0_ca1_inner_i + 1;
                ELSE
                    u0_m0_wo0_ca1_inner_i <= u0_m0_wo0_ca1_inner_i + 1;
                END IF;
            END IF;
        END IF;
    END PROCESS;
    u0_m0_wo0_ca1_inner_q <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR(RESIZE(u0_m0_wo0_ca1_inner_i, 1)));

    -- u0_m0_wo0_ca1_run(LOGICAL,83)@14
    u0_m0_wo0_ca1_run_q <= STD_LOGIC_VECTOR(u0_m0_wo0_ca1_inner_q(0 downto 0));

    -- u0_m0_wo0_ca1(COUNTER,84)@14
    -- low=0, high=63, step=1, init=0
    u0_m0_wo0_ca1_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_ca1_i <= TO_UNSIGNED(0, 6);
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (d_u0_m0_wo0_compute_q_14_q = "1" and u0_m0_wo0_ca1_run_q = "1") THEN
                u0_m0_wo0_ca1_i <= u0_m0_wo0_ca1_i + 1;
            END IF;
        END IF;
    END PROCESS;
    u0_m0_wo0_ca1_q <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR(RESIZE(u0_m0_wo0_ca1_i, 6)));

    -- u0_m0_wo0_cm0(LOOKUP,88)@14 + 1
    u0_m0_wo0_cm0_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_cm0_q <= "1111111110100000100001";
        ELSIF (clk'EVENT AND clk = '1') THEN
            CASE (u0_m0_wo0_ca1_q) IS
                WHEN "000000" => u0_m0_wo0_cm0_q <= "1111111110100000100001";
                WHEN "000001" => u0_m0_wo0_cm0_q <= "0000000000001110000010";
                WHEN "000010" => u0_m0_wo0_cm0_q <= "0000000001010011100011";
                WHEN "000011" => u0_m0_wo0_cm0_q <= "1111111111111001001001";
                WHEN "000100" => u0_m0_wo0_cm0_q <= "1111111110110111100000";
                WHEN "000101" => u0_m0_wo0_cm0_q <= "0000000000000001010110";
                WHEN "000110" => u0_m0_wo0_cm0_q <= "0000000000111110010111";
                WHEN "000111" => u0_m0_wo0_cm0_q <= "0000000000000010101111";
                WHEN "001000" => u0_m0_wo0_cm0_q <= "1111111111001010110100";
                WHEN "001001" => u0_m0_wo0_cm0_q <= "1111111111111010010101";
                WHEN "001010" => u0_m0_wo0_cm0_q <= "0000000000101100111100";
                WHEN "001011" => u0_m0_wo0_cm0_q <= "0000000000000111100111";
                WHEN "001100" => u0_m0_wo0_cm0_q <= "1111111111011010011000";
                WHEN "001101" => u0_m0_wo0_cm0_q <= "1111111111110111001110";
                WHEN "001110" => u0_m0_wo0_cm0_q <= "0000000000011111001101";
                WHEN "001111" => u0_m0_wo0_cm0_q <= "0000000000001001010101";
                WHEN "010000" => u0_m0_wo0_cm0_q <= "1111111111100110011001";
                WHEN "010001" => u0_m0_wo0_cm0_q <= "1111111111110110100111";
                WHEN "010010" => u0_m0_wo0_cm0_q <= "0000000000010100110100";
                WHEN "010011" => u0_m0_wo0_cm0_q <= "0000000000001001000111";
                WHEN "010100" => u0_m0_wo0_cm0_q <= "1111111111101111010010";
                WHEN "010101" => u0_m0_wo0_cm0_q <= "1111111111110111011011";
                WHEN "010110" => u0_m0_wo0_cm0_q <= "0000000000001101010010";
                WHEN "010111" => u0_m0_wo0_cm0_q <= "0000000000000111111000";
                WHEN "011000" => u0_m0_wo0_cm0_q <= "1111111111110101100110";
                WHEN "011001" => u0_m0_wo0_cm0_q <= "1111111111111000111011";
                WHEN "011010" => u0_m0_wo0_cm0_q <= "0000000000001000000100";
                WHEN "011011" => u0_m0_wo0_cm0_q <= "0000000000000110010000";
                WHEN "011100" => u0_m0_wo0_cm0_q <= "1111111111111001110110";
                WHEN "011101" => u0_m0_wo0_cm0_q <= "1111111111111010100101";
                WHEN "011110" => u0_m0_wo0_cm0_q <= "0000000000000100101000";
                WHEN "011111" => u0_m0_wo0_cm0_q <= "0000000000000100101000";
                WHEN "100000" => u0_m0_wo0_cm0_q <= "1111111111111100100110";
                WHEN "100001" => u0_m0_wo0_cm0_q <= "1111111111111100000111";
                WHEN "100010" => u0_m0_wo0_cm0_q <= "0000000000000010011110";
                WHEN "100011" => u0_m0_wo0_cm0_q <= "0000000000000011001110";
                WHEN "100100" => u0_m0_wo0_cm0_q <= "1111111111111110010000";
                WHEN "100101" => u0_m0_wo0_cm0_q <= "1111111111111101011000";
                WHEN "100110" => u0_m0_wo0_cm0_q <= "0000000000000001001101";
                WHEN "100111" => u0_m0_wo0_cm0_q <= "0000000000000010000111";
                WHEN "101000" => u0_m0_wo0_cm0_q <= "1111111111111111001100";
                WHEN "101001" => u0_m0_wo0_cm0_q <= "1111111111111110010100";
                WHEN "101010" => u0_m0_wo0_cm0_q <= "0000000000000000100001";
                WHEN "101011" => u0_m0_wo0_cm0_q <= "0000000000000001010100";
                WHEN "101100" => u0_m0_wo0_cm0_q <= "1111111111111111101100";
                WHEN "101101" => u0_m0_wo0_cm0_q <= "1111111111111110111111";
                WHEN "101110" => u0_m0_wo0_cm0_q <= "0000000000000000001011";
                WHEN "101111" => u0_m0_wo0_cm0_q <= "0000000000000000110010";
                WHEN "110000" => u0_m0_wo0_cm0_q <= "1111111111111111111011";
                WHEN "110001" => u0_m0_wo0_cm0_q <= "1111111111111111011011";
                WHEN "110010" => u0_m0_wo0_cm0_q <= "0000000000000000000010";
                WHEN "110011" => u0_m0_wo0_cm0_q <= "0000000000000000011011";
                WHEN "110100" => u0_m0_wo0_cm0_q <= "0000000000000000000000";
                WHEN "110101" => u0_m0_wo0_cm0_q <= "1111111111111111101100";
                WHEN "110110" => u0_m0_wo0_cm0_q <= "1111111111111111111111";
                WHEN "110111" => u0_m0_wo0_cm0_q <= "0000000000000000001110";
                WHEN "111000" => u0_m0_wo0_cm0_q <= "0000000000000000000001";
                WHEN "111001" => u0_m0_wo0_cm0_q <= "1111111111111111110110";
                WHEN "111010" => u0_m0_wo0_cm0_q <= "1111111111111111111111";
                WHEN "111011" => u0_m0_wo0_cm0_q <= "0000000000000000000111";
                WHEN "111100" => u0_m0_wo0_cm0_q <= "0000000000000000000001";
                WHEN "111101" => u0_m0_wo0_cm0_q <= "1111111111111111111100";
                WHEN "111110" => u0_m0_wo0_cm0_q <= "1111111111111111111111";
                WHEN "111111" => u0_m0_wo0_cm0_q <= "0000000000000000000010";
                WHEN OTHERS => -- unreachable
                               u0_m0_wo0_cm0_q <= (others => '-');
            END CASE;
        END IF;
    END PROCESS;

    -- u0_m0_wo0_mtree_mult1_1_bs1_merged_bit_select(BITSELECT,175)@15
    u0_m0_wo0_mtree_mult1_1_bs1_merged_bit_select_b <= STD_LOGIC_VECTOR(u0_m0_wo0_cm0_q(21 downto 17));
    u0_m0_wo0_mtree_mult1_1_bs1_merged_bit_select_c <= STD_LOGIC_VECTOR(u0_m0_wo0_cm0_q(16 downto 0));

    -- u0_m0_wo0_wi0_r0_ra1_count2_inner(COUNTER,38)@13
    -- low=-1, high=0, step=1, init=-1
    u0_m0_wo0_wi0_r0_ra1_count2_inner_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_wi0_r0_ra1_count2_inner_i <= TO_SIGNED(-1, 1);
            u0_m0_wo0_wi0_r0_ra1_count2_inner_eq <= '0';
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (u0_m0_wo0_memread_q = "1") THEN
                IF (u0_m0_wo0_wi0_r0_ra1_count2_inner_eq = '0') THEN
                    u0_m0_wo0_wi0_r0_ra1_count2_inner_eq <= '1';
                ELSE
                    u0_m0_wo0_wi0_r0_ra1_count2_inner_eq <= '0';
                END IF;
                IF (u0_m0_wo0_wi0_r0_ra1_count2_inner_eq = '1') THEN
                    u0_m0_wo0_wi0_r0_ra1_count2_inner_i <= u0_m0_wo0_wi0_r0_ra1_count2_inner_i + 1;
                ELSE
                    u0_m0_wo0_wi0_r0_ra1_count2_inner_i <= u0_m0_wo0_wi0_r0_ra1_count2_inner_i + 1;
                END IF;
            END IF;
        END IF;
    END PROCESS;
    u0_m0_wo0_wi0_r0_ra1_count2_inner_q <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR(RESIZE(u0_m0_wo0_wi0_r0_ra1_count2_inner_i, 1)));

    -- u0_m0_wo0_wi0_r0_ra1_count2_run(LOGICAL,39)@13
    u0_m0_wo0_wi0_r0_ra1_count2_run_q <= STD_LOGIC_VECTOR(u0_m0_wo0_wi0_r0_ra1_count2_inner_q(0 downto 0));

    -- u0_m0_wo0_wi0_r0_ra1_count2(COUNTER,40)@13
    -- low=0, high=63, step=1, init=0
    u0_m0_wo0_wi0_r0_ra1_count2_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_wi0_r0_ra1_count2_i <= TO_UNSIGNED(0, 6);
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (u0_m0_wo0_memread_q = "1" and u0_m0_wo0_wi0_r0_ra1_count2_run_q = "1") THEN
                u0_m0_wo0_wi0_r0_ra1_count2_i <= u0_m0_wo0_wi0_r0_ra1_count2_i + 1;
            END IF;
        END IF;
    END PROCESS;
    u0_m0_wo0_wi0_r0_ra1_count2_q <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR(RESIZE(u0_m0_wo0_wi0_r0_ra1_count2_i, 6)));

    -- u0_m0_wo0_wi0_r0_ra3_count2_lut(LOOKUP,60)@13
    u0_m0_wo0_wi0_r0_ra3_count2_lut_combproc: PROCESS (u0_m0_wo0_wi0_r0_ra1_count2_q)
    BEGIN
        -- Begin reserved scope level
        CASE (u0_m0_wo0_wi0_r0_ra1_count2_q) IS
            WHEN "000000" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "000000010";
            WHEN "000001" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "000000100";
            WHEN "000010" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "011111110";
            WHEN "000011" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "000000000";
            WHEN "000100" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "011111010";
            WHEN "000101" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "011111100";
            WHEN "000110" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "011110110";
            WHEN "000111" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "011111000";
            WHEN "001000" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "011110010";
            WHEN "001001" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "011110100";
            WHEN "001010" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "011101110";
            WHEN "001011" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "011110000";
            WHEN "001100" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "011101010";
            WHEN "001101" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "011101100";
            WHEN "001110" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "011100110";
            WHEN "001111" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "011101000";
            WHEN "010000" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "011100010";
            WHEN "010001" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "011100100";
            WHEN "010010" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "011011110";
            WHEN "010011" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "011100000";
            WHEN "010100" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "011011010";
            WHEN "010101" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "011011100";
            WHEN "010110" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "011010110";
            WHEN "010111" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "011011000";
            WHEN "011000" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "011010010";
            WHEN "011001" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "011010100";
            WHEN "011010" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "011001110";
            WHEN "011011" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "011010000";
            WHEN "011100" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "011001010";
            WHEN "011101" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "011001100";
            WHEN "011110" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "011000110";
            WHEN "011111" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "011001000";
            WHEN "100000" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "011000010";
            WHEN "100001" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "011000100";
            WHEN "100010" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "010111110";
            WHEN "100011" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "011000000";
            WHEN "100100" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "010111010";
            WHEN "100101" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "010111100";
            WHEN "100110" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "010110110";
            WHEN "100111" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "010111000";
            WHEN "101000" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "010110010";
            WHEN "101001" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "010110100";
            WHEN "101010" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "010101110";
            WHEN "101011" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "010110000";
            WHEN "101100" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "010101010";
            WHEN "101101" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "010101100";
            WHEN "101110" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "010100110";
            WHEN "101111" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "010101000";
            WHEN "110000" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "010100010";
            WHEN "110001" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "010100100";
            WHEN "110010" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "010011110";
            WHEN "110011" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "010100000";
            WHEN "110100" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "010011010";
            WHEN "110101" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "010011100";
            WHEN "110110" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "010010110";
            WHEN "110111" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "010011000";
            WHEN "111000" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "010010010";
            WHEN "111001" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "010010100";
            WHEN "111010" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "010001110";
            WHEN "111011" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "010010000";
            WHEN "111100" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "010001010";
            WHEN "111101" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "010001100";
            WHEN "111110" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "010000110";
            WHEN "111111" => u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= "010001000";
            WHEN OTHERS => -- unreachable
                           u0_m0_wo0_wi0_r0_ra3_count2_lut_q <= (others => '-');
        END CASE;
        -- End reserved scope level
    END PROCESS;

    -- u0_m0_wo0_wi0_r0_ra3_count2_lutreg(REG,61)@13
    u0_m0_wo0_wi0_r0_ra3_count2_lutreg_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_wi0_r0_ra3_count2_lutreg_q <= "000000010";
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (u0_m0_wo0_memread_q = "1") THEN
                u0_m0_wo0_wi0_r0_ra3_count2_lutreg_q <= STD_LOGIC_VECTOR(u0_m0_wo0_wi0_r0_ra3_count2_lut_q);
            END IF;
        END IF;
    END PROCESS;

    -- u0_m0_wo0_wi0_r0_ra0_count0_inner(COUNTER,22)@13
    -- low=-1, high=126, step=-1, init=126
    u0_m0_wo0_wi0_r0_ra0_count0_inner_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_wi0_r0_ra0_count0_inner_i <= TO_SIGNED(126, 8);
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (u0_m0_wo0_memread_q = "1") THEN
                IF (u0_m0_wo0_wi0_r0_ra0_count0_inner_i(7 downto 7) = "1") THEN
                    u0_m0_wo0_wi0_r0_ra0_count0_inner_i <= u0_m0_wo0_wi0_r0_ra0_count0_inner_i - 129;
                ELSE
                    u0_m0_wo0_wi0_r0_ra0_count0_inner_i <= u0_m0_wo0_wi0_r0_ra0_count0_inner_i - 1;
                END IF;
            END IF;
        END IF;
    END PROCESS;
    u0_m0_wo0_wi0_r0_ra0_count0_inner_q <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR(RESIZE(u0_m0_wo0_wi0_r0_ra0_count0_inner_i, 8)));

    -- u0_m0_wo0_wi0_r0_ra0_count0_run(LOGICAL,23)@13
    u0_m0_wo0_wi0_r0_ra0_count0_run_q <= STD_LOGIC_VECTOR(u0_m0_wo0_wi0_r0_ra0_count0_inner_q(7 downto 7));

    -- u0_m0_wo0_wi0_r0_ra1_count0(COUNTER,34)@13
    -- low=0, high=255, step=4, init=0
    u0_m0_wo0_wi0_r0_ra1_count0_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_wi0_r0_ra1_count0_i <= TO_UNSIGNED(0, 8);
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (u0_m0_wo0_memread_q = "1" and u0_m0_wo0_wi0_r0_ra0_count0_run_q = "1") THEN
                u0_m0_wo0_wi0_r0_ra1_count0_i <= u0_m0_wo0_wi0_r0_ra1_count0_i + 4;
            END IF;
        END IF;
    END PROCESS;
    u0_m0_wo0_wi0_r0_ra1_count0_q <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR(RESIZE(u0_m0_wo0_wi0_r0_ra1_count0_i, 9)));

    -- u0_m0_wo0_wi0_r0_ra1_add_0_0_replace_or_BitSelect_for_a(BITSELECT,166)@13
    u0_m0_wo0_wi0_r0_ra1_add_0_0_replace_or_BitSelect_for_a_in <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR((9 downto 9 => u0_m0_wo0_wi0_r0_ra1_count0_q(8)) & u0_m0_wo0_wi0_r0_ra1_count0_q));
    u0_m0_wo0_wi0_r0_ra1_add_0_0_replace_or_BitSelect_for_a_b <= STD_LOGIC_VECTOR(u0_m0_wo0_wi0_r0_ra1_add_0_0_replace_or_BitSelect_for_a_in(7 downto 2));

    -- GND(CONSTANT,0)@0
    GND_q <= "0";

    -- u0_m0_wo0_wi0_r0_ra0_count1(COUNTER,25)@13
    -- low=0, high=1, step=1, init=0
    u0_m0_wo0_wi0_r0_ra0_count1_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_wi0_r0_ra0_count1_i <= TO_UNSIGNED(0, 1);
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (u0_m0_wo0_memread_q = "1") THEN
                u0_m0_wo0_wi0_r0_ra0_count1_i <= u0_m0_wo0_wi0_r0_ra0_count1_i + 1;
            END IF;
        END IF;
    END PROCESS;
    u0_m0_wo0_wi0_r0_ra0_count1_q <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR(RESIZE(u0_m0_wo0_wi0_r0_ra0_count1_i, 2)));

    -- u0_m0_wo0_wi0_r0_ra1_add_0_0_replace_or_BitSelect_for_b(BITSELECT,167)@13
    u0_m0_wo0_wi0_r0_ra1_add_0_0_replace_or_BitSelect_for_b_in <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR((9 downto 2 => u0_m0_wo0_wi0_r0_ra0_count1_q(1)) & u0_m0_wo0_wi0_r0_ra0_count1_q));
    u0_m0_wo0_wi0_r0_ra1_add_0_0_replace_or_BitSelect_for_b_b <= STD_LOGIC_VECTOR(u0_m0_wo0_wi0_r0_ra1_add_0_0_replace_or_BitSelect_for_b_in(0 downto 0));

    -- u0_m0_wo0_wi0_r0_ra1_add_0_0_replace_or_join(BITJOIN,168)@13
    u0_m0_wo0_wi0_r0_ra1_add_0_0_replace_or_join_q <= GND_q & GND_q & u0_m0_wo0_wi0_r0_ra1_add_0_0_replace_or_BitSelect_for_a_b & GND_q & u0_m0_wo0_wi0_r0_ra1_add_0_0_replace_or_BitSelect_for_b_b;

    -- u0_m0_wo0_wi0_r0_ra3_add_1_0(ADD,66)@13 + 1
    u0_m0_wo0_wi0_r0_ra3_add_1_0_a <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_wi0_r0_ra1_add_0_0_replace_or_join_q);
    u0_m0_wo0_wi0_r0_ra3_add_1_0_b <= STD_LOGIC_VECTOR("00" & u0_m0_wo0_wi0_r0_ra3_count2_lutreg_q);
    u0_m0_wo0_wi0_r0_ra3_add_1_0_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_wi0_r0_ra3_add_1_0_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            u0_m0_wo0_wi0_r0_ra3_add_1_0_o <= STD_LOGIC_VECTOR(UNSIGNED(u0_m0_wo0_wi0_r0_ra3_add_1_0_a) + UNSIGNED(u0_m0_wo0_wi0_r0_ra3_add_1_0_b));
        END IF;
    END PROCESS;
    u0_m0_wo0_wi0_r0_ra3_add_1_0_q <= u0_m0_wo0_wi0_r0_ra3_add_1_0_o(10 downto 0);

    -- u0_m0_wo0_wi0_r0_ra3_resize(BITSELECT,67)@14
    u0_m0_wo0_wi0_r0_ra3_resize_in <= STD_LOGIC_VECTOR(u0_m0_wo0_wi0_r0_ra3_add_1_0_q(7 downto 0));
    u0_m0_wo0_wi0_r0_ra3_resize_b <= STD_LOGIC_VECTOR(u0_m0_wo0_wi0_r0_ra3_resize_in(7 downto 0));

    -- u0_m0_wo0_wi0_r0_ra2_count2_lut(LOOKUP,48)@13
    u0_m0_wo0_wi0_r0_ra2_count2_lut_combproc: PROCESS (u0_m0_wo0_wi0_r0_ra1_count2_q)
    BEGIN
        -- Begin reserved scope level
        CASE (u0_m0_wo0_wi0_r0_ra1_count2_q) IS
            WHEN "000000" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0110000100";
            WHEN "000001" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0110000010";
            WHEN "000010" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0110000000";
            WHEN "000011" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0101111110";
            WHEN "000100" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0101111100";
            WHEN "000101" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0101111010";
            WHEN "000110" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0101111000";
            WHEN "000111" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0101110110";
            WHEN "001000" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0101110100";
            WHEN "001001" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0101110010";
            WHEN "001010" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0101110000";
            WHEN "001011" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0101101110";
            WHEN "001100" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0101101100";
            WHEN "001101" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0101101010";
            WHEN "001110" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0101101000";
            WHEN "001111" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0101100110";
            WHEN "010000" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0101100100";
            WHEN "010001" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0101100010";
            WHEN "010010" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0101100000";
            WHEN "010011" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0101011110";
            WHEN "010100" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0101011100";
            WHEN "010101" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0101011010";
            WHEN "010110" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0101011000";
            WHEN "010111" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0101010110";
            WHEN "011000" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0101010100";
            WHEN "011001" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0101010010";
            WHEN "011010" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0101010000";
            WHEN "011011" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0101001110";
            WHEN "011100" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0101001100";
            WHEN "011101" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0101001010";
            WHEN "011110" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0101001000";
            WHEN "011111" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0101000110";
            WHEN "100000" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0101000100";
            WHEN "100001" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0101000010";
            WHEN "100010" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0101000000";
            WHEN "100011" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0100111110";
            WHEN "100100" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0100111100";
            WHEN "100101" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0100111010";
            WHEN "100110" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0100111000";
            WHEN "100111" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0100110110";
            WHEN "101000" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0100110100";
            WHEN "101001" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0100110010";
            WHEN "101010" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0100110000";
            WHEN "101011" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0100101110";
            WHEN "101100" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0100101100";
            WHEN "101101" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0100101010";
            WHEN "101110" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0100101000";
            WHEN "101111" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0100100110";
            WHEN "110000" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0100100100";
            WHEN "110001" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0100100010";
            WHEN "110010" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0100100000";
            WHEN "110011" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0100011110";
            WHEN "110100" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0100011100";
            WHEN "110101" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0100011010";
            WHEN "110110" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0100011000";
            WHEN "110111" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0100010110";
            WHEN "111000" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0100010100";
            WHEN "111001" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0100010010";
            WHEN "111010" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0100010000";
            WHEN "111011" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0100001110";
            WHEN "111100" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0100001100";
            WHEN "111101" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0100001010";
            WHEN "111110" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0100001000";
            WHEN "111111" => u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= "0100000110";
            WHEN OTHERS => -- unreachable
                           u0_m0_wo0_wi0_r0_ra2_count2_lut_q <= (others => '-');
        END CASE;
        -- End reserved scope level
    END PROCESS;

    -- u0_m0_wo0_wi0_r0_ra2_count2_lutreg(REG,49)@13
    u0_m0_wo0_wi0_r0_ra2_count2_lutreg_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_wi0_r0_ra2_count2_lutreg_q <= "0110000100";
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (u0_m0_wo0_memread_q = "1") THEN
                u0_m0_wo0_wi0_r0_ra2_count2_lutreg_q <= STD_LOGIC_VECTOR(u0_m0_wo0_wi0_r0_ra2_count2_lut_q);
            END IF;
        END IF;
    END PROCESS;

    -- u0_m0_wo0_wi0_r0_ra2_count0(COUNTER,46)@13
    -- low=0, high=511, step=4, init=0
    u0_m0_wo0_wi0_r0_ra2_count0_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_wi0_r0_ra2_count0_i <= TO_UNSIGNED(0, 9);
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (u0_m0_wo0_memread_q = "1" and u0_m0_wo0_wi0_r0_ra0_count0_run_q = "1") THEN
                u0_m0_wo0_wi0_r0_ra2_count0_i <= u0_m0_wo0_wi0_r0_ra2_count0_i + 4;
            END IF;
        END IF;
    END PROCESS;
    u0_m0_wo0_wi0_r0_ra2_count0_q <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR(RESIZE(u0_m0_wo0_wi0_r0_ra2_count0_i, 10)));

    -- u0_m0_wo0_wi0_r0_ra2_add_0_0_replace_or_BitSelect_for_a(BITSELECT,169)@13
    u0_m0_wo0_wi0_r0_ra2_add_0_0_replace_or_BitSelect_for_a_in <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR((10 downto 10 => u0_m0_wo0_wi0_r0_ra2_count0_q(9)) & u0_m0_wo0_wi0_r0_ra2_count0_q));
    u0_m0_wo0_wi0_r0_ra2_add_0_0_replace_or_BitSelect_for_a_b <= STD_LOGIC_VECTOR(u0_m0_wo0_wi0_r0_ra2_add_0_0_replace_or_BitSelect_for_a_in(8 downto 2));

    -- u0_m0_wo0_wi0_r0_ra2_add_0_0_replace_or_BitSelect_for_b(BITSELECT,170)@13
    u0_m0_wo0_wi0_r0_ra2_add_0_0_replace_or_BitSelect_for_b_in <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR((10 downto 2 => u0_m0_wo0_wi0_r0_ra0_count1_q(1)) & u0_m0_wo0_wi0_r0_ra0_count1_q));
    u0_m0_wo0_wi0_r0_ra2_add_0_0_replace_or_BitSelect_for_b_b <= STD_LOGIC_VECTOR(u0_m0_wo0_wi0_r0_ra2_add_0_0_replace_or_BitSelect_for_b_in(0 downto 0));

    -- u0_m0_wo0_wi0_r0_ra2_add_0_0_replace_or_join(BITJOIN,171)@13
    u0_m0_wo0_wi0_r0_ra2_add_0_0_replace_or_join_q <= GND_q & GND_q & u0_m0_wo0_wi0_r0_ra2_add_0_0_replace_or_BitSelect_for_a_b & GND_q & u0_m0_wo0_wi0_r0_ra2_add_0_0_replace_or_BitSelect_for_b_b;

    -- u0_m0_wo0_wi0_r0_ra2_add_1_0(ADD,54)@13 + 1
    u0_m0_wo0_wi0_r0_ra2_add_1_0_a <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_wi0_r0_ra2_add_0_0_replace_or_join_q);
    u0_m0_wo0_wi0_r0_ra2_add_1_0_b <= STD_LOGIC_VECTOR("00" & u0_m0_wo0_wi0_r0_ra2_count2_lutreg_q);
    u0_m0_wo0_wi0_r0_ra2_add_1_0_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_wi0_r0_ra2_add_1_0_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            u0_m0_wo0_wi0_r0_ra2_add_1_0_o <= STD_LOGIC_VECTOR(UNSIGNED(u0_m0_wo0_wi0_r0_ra2_add_1_0_a) + UNSIGNED(u0_m0_wo0_wi0_r0_ra2_add_1_0_b));
        END IF;
    END PROCESS;
    u0_m0_wo0_wi0_r0_ra2_add_1_0_q <= u0_m0_wo0_wi0_r0_ra2_add_1_0_o(11 downto 0);

    -- u0_m0_wo0_wi0_r0_ra2_resize(BITSELECT,55)@14
    u0_m0_wo0_wi0_r0_ra2_resize_in <= STD_LOGIC_VECTOR(u0_m0_wo0_wi0_r0_ra2_add_1_0_q(8 downto 0));
    u0_m0_wo0_wi0_r0_ra2_resize_b <= STD_LOGIC_VECTOR(u0_m0_wo0_wi0_r0_ra2_resize_in(8 downto 0));

    -- u0_m0_wo0_wi0_r0_ra1_count2_lut(LOOKUP,36)@13
    u0_m0_wo0_wi0_r0_ra1_count2_lut_combproc: PROCESS (u0_m0_wo0_wi0_r0_ra1_count2_q)
    BEGIN
        -- Begin reserved scope level
        CASE (u0_m0_wo0_wi0_r0_ra1_count2_q) IS
            WHEN "000000" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "010001010";
            WHEN "000001" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "010001100";
            WHEN "000010" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "010001110";
            WHEN "000011" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "010010000";
            WHEN "000100" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "010010010";
            WHEN "000101" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "010010100";
            WHEN "000110" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "010010110";
            WHEN "000111" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "010011000";
            WHEN "001000" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "010011010";
            WHEN "001001" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "010011100";
            WHEN "001010" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "010011110";
            WHEN "001011" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "010100000";
            WHEN "001100" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "010100010";
            WHEN "001101" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "010100100";
            WHEN "001110" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "010100110";
            WHEN "001111" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "010101000";
            WHEN "010000" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "010101010";
            WHEN "010001" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "010101100";
            WHEN "010010" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "010101110";
            WHEN "010011" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "010110000";
            WHEN "010100" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "010110010";
            WHEN "010101" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "010110100";
            WHEN "010110" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "010110110";
            WHEN "010111" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "010111000";
            WHEN "011000" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "010111010";
            WHEN "011001" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "010111100";
            WHEN "011010" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "010111110";
            WHEN "011011" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "011000000";
            WHEN "011100" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "011000010";
            WHEN "011101" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "011000100";
            WHEN "011110" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "011000110";
            WHEN "011111" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "011001000";
            WHEN "100000" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "011001010";
            WHEN "100001" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "011001100";
            WHEN "100010" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "011001110";
            WHEN "100011" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "011010000";
            WHEN "100100" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "011010010";
            WHEN "100101" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "011010100";
            WHEN "100110" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "011010110";
            WHEN "100111" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "011011000";
            WHEN "101000" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "011011010";
            WHEN "101001" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "011011100";
            WHEN "101010" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "011011110";
            WHEN "101011" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "011100000";
            WHEN "101100" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "011100010";
            WHEN "101101" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "011100100";
            WHEN "101110" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "011100110";
            WHEN "101111" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "011101000";
            WHEN "110000" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "011101010";
            WHEN "110001" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "011101100";
            WHEN "110010" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "011101110";
            WHEN "110011" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "011110000";
            WHEN "110100" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "011110010";
            WHEN "110101" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "011110100";
            WHEN "110110" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "011110110";
            WHEN "110111" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "011111000";
            WHEN "111000" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "011111010";
            WHEN "111001" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "011111100";
            WHEN "111010" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "011111110";
            WHEN "111011" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "000000000";
            WHEN "111100" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "000000010";
            WHEN "111101" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "000000100";
            WHEN "111110" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "000000110";
            WHEN "111111" => u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= "000001000";
            WHEN OTHERS => -- unreachable
                           u0_m0_wo0_wi0_r0_ra1_count2_lut_q <= (others => '-');
        END CASE;
        -- End reserved scope level
    END PROCESS;

    -- u0_m0_wo0_wi0_r0_ra1_count2_lutreg(REG,37)@13
    u0_m0_wo0_wi0_r0_ra1_count2_lutreg_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_wi0_r0_ra1_count2_lutreg_q <= "010001010";
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (u0_m0_wo0_memread_q = "1") THEN
                u0_m0_wo0_wi0_r0_ra1_count2_lutreg_q <= STD_LOGIC_VECTOR(u0_m0_wo0_wi0_r0_ra1_count2_lut_q);
            END IF;
        END IF;
    END PROCESS;

    -- u0_m0_wo0_wi0_r0_ra1_add_1_0(ADD,42)@13 + 1
    u0_m0_wo0_wi0_r0_ra1_add_1_0_a <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_wi0_r0_ra1_add_0_0_replace_or_join_q);
    u0_m0_wo0_wi0_r0_ra1_add_1_0_b <= STD_LOGIC_VECTOR("00" & u0_m0_wo0_wi0_r0_ra1_count2_lutreg_q);
    u0_m0_wo0_wi0_r0_ra1_add_1_0_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_wi0_r0_ra1_add_1_0_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            u0_m0_wo0_wi0_r0_ra1_add_1_0_o <= STD_LOGIC_VECTOR(UNSIGNED(u0_m0_wo0_wi0_r0_ra1_add_1_0_a) + UNSIGNED(u0_m0_wo0_wi0_r0_ra1_add_1_0_b));
        END IF;
    END PROCESS;
    u0_m0_wo0_wi0_r0_ra1_add_1_0_q <= u0_m0_wo0_wi0_r0_ra1_add_1_0_o(10 downto 0);

    -- u0_m0_wo0_wi0_r0_ra1_resize(BITSELECT,43)@14
    u0_m0_wo0_wi0_r0_ra1_resize_in <= STD_LOGIC_VECTOR(u0_m0_wo0_wi0_r0_ra1_add_1_0_q(7 downto 0));
    u0_m0_wo0_wi0_r0_ra1_resize_b <= STD_LOGIC_VECTOR(u0_m0_wo0_wi0_r0_ra1_resize_in(7 downto 0));

    -- u0_m0_wo0_wi0_r0_wa1(COUNTER,75)@14
    -- low=0, high=255, step=1, init=10
    u0_m0_wo0_wi0_r0_wa1_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_wi0_r0_wa1_i <= TO_UNSIGNED(10, 8);
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (u0_m0_wo0_wi0_r0_we2_q = "1") THEN
                u0_m0_wo0_wi0_r0_wa1_i <= u0_m0_wo0_wi0_r0_wa1_i + 1;
            END IF;
        END IF;
    END PROCESS;
    u0_m0_wo0_wi0_r0_wa1_q <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR(RESIZE(u0_m0_wo0_wi0_r0_wa1_i, 8)));

    -- u0_m0_wo0_wi0_r0_memr1(DUALMEM,79)@14
    u0_m0_wo0_wi0_r0_memr1_ia <= STD_LOGIC_VECTOR(u0_m0_wo0_wi0_r0_memr0_q);
    u0_m0_wo0_wi0_r0_memr1_aa <= u0_m0_wo0_wi0_r0_wa1_q;
    u0_m0_wo0_wi0_r0_memr1_ab <= u0_m0_wo0_wi0_r0_ra1_resize_b;
    u0_m0_wo0_wi0_r0_memr1_dmem : altsyncram
    GENERIC MAP (
        ram_block_type => "M9K",
        operation_mode => "DUAL_PORT",
        width_a => 32,
        widthad_a => 8,
        numwords_a => 256,
        width_b => 32,
        widthad_b => 8,
        numwords_b => 256,
        lpm_type => "altsyncram",
        width_byteena_a => 1,
        address_reg_b => "CLOCK0",
        indata_reg_b => "CLOCK0",
        wrcontrol_wraddress_reg_b => "CLOCK0",
        rdcontrol_reg_b => "CLOCK0",
        byteena_reg_b => "CLOCK0",
        outdata_reg_b => "CLOCK0",
        outdata_aclr_b => "NONE",
        clock_enable_input_a => "NORMAL",
        clock_enable_input_b => "NORMAL",
        clock_enable_output_b => "NORMAL",
        read_during_write_mode_mixed_ports => "DONT_CARE",
        power_up_uninitialized => "FALSE",
        init_file => "UNUSED",
        intended_device_family => "Cyclone IV E"
    )
    PORT MAP (
        clocken0 => '1',
        clock0 => clk,
        address_a => u0_m0_wo0_wi0_r0_memr1_aa,
        data_a => u0_m0_wo0_wi0_r0_memr1_ia,
        wren_a => u0_m0_wo0_wi0_r0_we2_q(0),
        address_b => u0_m0_wo0_wi0_r0_memr1_ab,
        q_b => u0_m0_wo0_wi0_r0_memr1_iq
    );
    u0_m0_wo0_wi0_r0_memr1_q <= u0_m0_wo0_wi0_r0_memr1_iq(31 downto 0);

    -- u0_m0_wo0_wi0_r0_we2_1(REG,69)@14
    u0_m0_wo0_wi0_r0_we2_1_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_wi0_r0_we2_1_q <= "1";
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (d_u0_m0_wo0_compute_q_14_q = "1") THEN
                u0_m0_wo0_wi0_r0_we2_1_q <= u0_m0_wo0_wi0_r0_we3_seq_q;
            END IF;
        END IF;
    END PROCESS;

    -- u0_m0_wo0_wi0_r0_we2_2(REG,70)@14
    u0_m0_wo0_wi0_r0_we2_2_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_wi0_r0_we2_2_q <= "1";
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (d_u0_m0_wo0_compute_q_14_q = "1") THEN
                u0_m0_wo0_wi0_r0_we2_2_q <= u0_m0_wo0_wi0_r0_we2_1_q;
            END IF;
        END IF;
    END PROCESS;

    -- u0_m0_wo0_wi0_r0_we2_3(REG,71)@14
    u0_m0_wo0_wi0_r0_we2_3_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_wi0_r0_we2_3_q <= "1";
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (d_u0_m0_wo0_compute_q_14_q = "1") THEN
                u0_m0_wo0_wi0_r0_we2_3_q <= u0_m0_wo0_wi0_r0_we2_2_q;
            END IF;
        END IF;
    END PROCESS;

    -- u0_m0_wo0_wi0_r0_we2_4(REG,72)@14
    u0_m0_wo0_wi0_r0_we2_4_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_wi0_r0_we2_4_q <= "1";
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (d_u0_m0_wo0_compute_q_14_q = "1") THEN
                u0_m0_wo0_wi0_r0_we2_4_q <= u0_m0_wo0_wi0_r0_we2_3_q;
            END IF;
        END IF;
    END PROCESS;

    -- u0_m0_wo0_wi0_r0_we2(LOGICAL,73)@14
    u0_m0_wo0_wi0_r0_we2_q <= u0_m0_wo0_wi0_r0_we2_4_q and d_u0_m0_wo0_compute_q_14_q;

    -- u0_m0_wo0_wi0_r0_wa2(COUNTER,76)@14
    -- low=0, high=511, step=1, init=390
    u0_m0_wo0_wi0_r0_wa2_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_wi0_r0_wa2_i <= TO_UNSIGNED(390, 9);
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (u0_m0_wo0_wi0_r0_we2_q = "1") THEN
                u0_m0_wo0_wi0_r0_wa2_i <= u0_m0_wo0_wi0_r0_wa2_i + 1;
            END IF;
        END IF;
    END PROCESS;
    u0_m0_wo0_wi0_r0_wa2_q <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR(RESIZE(u0_m0_wo0_wi0_r0_wa2_i, 9)));

    -- u0_m0_wo0_wi0_r0_memr2(DUALMEM,80)@14
    u0_m0_wo0_wi0_r0_memr2_ia <= STD_LOGIC_VECTOR(u0_m0_wo0_wi0_r0_memr1_q);
    u0_m0_wo0_wi0_r0_memr2_aa <= u0_m0_wo0_wi0_r0_wa2_q;
    u0_m0_wo0_wi0_r0_memr2_ab <= u0_m0_wo0_wi0_r0_ra2_resize_b;
    u0_m0_wo0_wi0_r0_memr2_dmem : altsyncram
    GENERIC MAP (
        ram_block_type => "M9K",
        operation_mode => "DUAL_PORT",
        width_a => 32,
        widthad_a => 9,
        numwords_a => 512,
        width_b => 32,
        widthad_b => 9,
        numwords_b => 512,
        lpm_type => "altsyncram",
        width_byteena_a => 1,
        address_reg_b => "CLOCK0",
        indata_reg_b => "CLOCK0",
        wrcontrol_wraddress_reg_b => "CLOCK0",
        rdcontrol_reg_b => "CLOCK0",
        byteena_reg_b => "CLOCK0",
        outdata_reg_b => "CLOCK0",
        outdata_aclr_b => "NONE",
        clock_enable_input_a => "NORMAL",
        clock_enable_input_b => "NORMAL",
        clock_enable_output_b => "NORMAL",
        read_during_write_mode_mixed_ports => "DONT_CARE",
        power_up_uninitialized => "FALSE",
        init_file => "UNUSED",
        intended_device_family => "Cyclone IV E"
    )
    PORT MAP (
        clocken0 => '1',
        clock0 => clk,
        address_a => u0_m0_wo0_wi0_r0_memr2_aa,
        data_a => u0_m0_wo0_wi0_r0_memr2_ia,
        wren_a => u0_m0_wo0_wi0_r0_we2_q(0),
        address_b => u0_m0_wo0_wi0_r0_memr2_ab,
        q_b => u0_m0_wo0_wi0_r0_memr2_iq
    );
    u0_m0_wo0_wi0_r0_memr2_q <= u0_m0_wo0_wi0_r0_memr2_iq(31 downto 0);

    -- u0_m0_wo0_wi0_r0_we3_seq(SEQUENCE,68)@13 + 1
    u0_m0_wo0_wi0_r0_we3_seq_clkproc: PROCESS (clk, areset)
        variable u0_m0_wo0_wi0_r0_we3_seq_c : SIGNED(9 downto 0);
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_wi0_r0_we3_seq_c := "1111111100";
            u0_m0_wo0_wi0_r0_we3_seq_q <= "0";
            u0_m0_wo0_wi0_r0_we3_seq_eq <= '1';
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (u0_m0_wo0_compute_q = "1") THEN
                IF (u0_m0_wo0_wi0_r0_we3_seq_c = "1111111101") THEN
                    u0_m0_wo0_wi0_r0_we3_seq_eq <= '1';
                ELSE
                    u0_m0_wo0_wi0_r0_we3_seq_eq <= '0';
                END IF;
                IF (u0_m0_wo0_wi0_r0_we3_seq_eq = '1') THEN
                    u0_m0_wo0_wi0_r0_we3_seq_c := u0_m0_wo0_wi0_r0_we3_seq_c + 127;
                ELSE
                    u0_m0_wo0_wi0_r0_we3_seq_c := u0_m0_wo0_wi0_r0_we3_seq_c - 1;
                END IF;
                u0_m0_wo0_wi0_r0_we3_seq_q <= STD_LOGIC_VECTOR(u0_m0_wo0_wi0_r0_we3_seq_c(9 downto 9));
            ELSE
                u0_m0_wo0_wi0_r0_we3_seq_q <= "0";
            END IF;
        END IF;
    END PROCESS;

    -- u0_m0_wo0_wi0_r0_wa3(COUNTER,77)@14
    -- low=0, high=255, step=1, init=6
    u0_m0_wo0_wi0_r0_wa3_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_wi0_r0_wa3_i <= TO_UNSIGNED(6, 8);
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (u0_m0_wo0_wi0_r0_we3_seq_q = "1") THEN
                u0_m0_wo0_wi0_r0_wa3_i <= u0_m0_wo0_wi0_r0_wa3_i + 1;
            END IF;
        END IF;
    END PROCESS;
    u0_m0_wo0_wi0_r0_wa3_q <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR(RESIZE(u0_m0_wo0_wi0_r0_wa3_i, 8)));

    -- u0_m0_wo0_wi0_r0_memr3(DUALMEM,81)@14
    u0_m0_wo0_wi0_r0_memr3_ia <= STD_LOGIC_VECTOR(u0_m0_wo0_wi0_r0_memr2_q);
    u0_m0_wo0_wi0_r0_memr3_aa <= u0_m0_wo0_wi0_r0_wa3_q;
    u0_m0_wo0_wi0_r0_memr3_ab <= u0_m0_wo0_wi0_r0_ra3_resize_b;
    u0_m0_wo0_wi0_r0_memr3_dmem : altsyncram
    GENERIC MAP (
        ram_block_type => "M9K",
        operation_mode => "DUAL_PORT",
        width_a => 32,
        widthad_a => 8,
        numwords_a => 256,
        width_b => 32,
        widthad_b => 8,
        numwords_b => 256,
        lpm_type => "altsyncram",
        width_byteena_a => 1,
        address_reg_b => "CLOCK0",
        indata_reg_b => "CLOCK0",
        wrcontrol_wraddress_reg_b => "CLOCK0",
        rdcontrol_reg_b => "CLOCK0",
        byteena_reg_b => "CLOCK0",
        outdata_reg_b => "CLOCK0",
        outdata_aclr_b => "NONE",
        clock_enable_input_a => "NORMAL",
        clock_enable_input_b => "NORMAL",
        clock_enable_output_b => "NORMAL",
        read_during_write_mode_mixed_ports => "DONT_CARE",
        power_up_uninitialized => "FALSE",
        init_file => "UNUSED",
        intended_device_family => "Cyclone IV E"
    )
    PORT MAP (
        clocken0 => '1',
        clock0 => clk,
        address_a => u0_m0_wo0_wi0_r0_memr3_aa,
        data_a => u0_m0_wo0_wi0_r0_memr3_ia,
        wren_a => u0_m0_wo0_wi0_r0_we3_seq_q(0),
        address_b => u0_m0_wo0_wi0_r0_memr3_ab,
        q_b => u0_m0_wo0_wi0_r0_memr3_iq
    );
    u0_m0_wo0_wi0_r0_memr3_q <= u0_m0_wo0_wi0_r0_memr3_iq(31 downto 0);

    -- u0_m0_wo0_wi0_r0_ra0_count2_inner(COUNTER,26)@13
    -- low=-1, high=0, step=1, init=0
    u0_m0_wo0_wi0_r0_ra0_count2_inner_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_wi0_r0_ra0_count2_inner_i <= TO_SIGNED(0, 1);
            u0_m0_wo0_wi0_r0_ra0_count2_inner_eq <= '1';
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (u0_m0_wo0_memread_q = "1") THEN
                IF (u0_m0_wo0_wi0_r0_ra0_count2_inner_eq = '0') THEN
                    u0_m0_wo0_wi0_r0_ra0_count2_inner_eq <= '1';
                ELSE
                    u0_m0_wo0_wi0_r0_ra0_count2_inner_eq <= '0';
                END IF;
                IF (u0_m0_wo0_wi0_r0_ra0_count2_inner_eq = '1') THEN
                    u0_m0_wo0_wi0_r0_ra0_count2_inner_i <= u0_m0_wo0_wi0_r0_ra0_count2_inner_i + 1;
                ELSE
                    u0_m0_wo0_wi0_r0_ra0_count2_inner_i <= u0_m0_wo0_wi0_r0_ra0_count2_inner_i + 1;
                END IF;
            END IF;
        END IF;
    END PROCESS;
    u0_m0_wo0_wi0_r0_ra0_count2_inner_q <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR(RESIZE(u0_m0_wo0_wi0_r0_ra0_count2_inner_i, 1)));

    -- u0_m0_wo0_wi0_r0_ra0_count2_run(LOGICAL,27)@13
    u0_m0_wo0_wi0_r0_ra0_count2_run_q <= STD_LOGIC_VECTOR(u0_m0_wo0_wi0_r0_ra0_count2_inner_q(0 downto 0));

    -- u0_m0_wo0_wi0_r0_ra0_count2(COUNTER,28)@13
    -- low=0, high=127, step=2, init=2
    u0_m0_wo0_wi0_r0_ra0_count2_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_wi0_r0_ra0_count2_i <= TO_UNSIGNED(2, 7);
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (u0_m0_wo0_memread_q = "1" and u0_m0_wo0_wi0_r0_ra0_count2_run_q = "1") THEN
                u0_m0_wo0_wi0_r0_ra0_count2_i <= u0_m0_wo0_wi0_r0_ra0_count2_i + 2;
            END IF;
        END IF;
    END PROCESS;
    u0_m0_wo0_wi0_r0_ra0_count2_q <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR(RESIZE(u0_m0_wo0_wi0_r0_ra0_count2_i, 8)));

    -- u0_m0_wo0_wi0_r0_ra0_count0(COUNTER,24)@13
    -- low=0, high=127, step=4, init=0
    u0_m0_wo0_wi0_r0_ra0_count0_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_wi0_r0_ra0_count0_i <= TO_UNSIGNED(0, 7);
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (u0_m0_wo0_memread_q = "1" and u0_m0_wo0_wi0_r0_ra0_count0_run_q = "1") THEN
                u0_m0_wo0_wi0_r0_ra0_count0_i <= u0_m0_wo0_wi0_r0_ra0_count0_i + 4;
            END IF;
        END IF;
    END PROCESS;
    u0_m0_wo0_wi0_r0_ra0_count0_q <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR(RESIZE(u0_m0_wo0_wi0_r0_ra0_count0_i, 8)));

    -- u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_BitSelect_for_a(BITSELECT,163)@13
    u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_BitSelect_for_a_in <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR((8 downto 8 => u0_m0_wo0_wi0_r0_ra0_count0_q(7)) & u0_m0_wo0_wi0_r0_ra0_count0_q));
    u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_BitSelect_for_a_b <= STD_LOGIC_VECTOR(u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_BitSelect_for_a_in(6 downto 2));

    -- u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_BitSelect_for_b(BITSELECT,164)@13
    u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_BitSelect_for_b_in <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR((8 downto 2 => u0_m0_wo0_wi0_r0_ra0_count1_q(1)) & u0_m0_wo0_wi0_r0_ra0_count1_q));
    u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_BitSelect_for_b_b <= STD_LOGIC_VECTOR(u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_BitSelect_for_b_in(0 downto 0));

    -- u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_join(BITJOIN,165)@13
    u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_join_q <= GND_q & GND_q & u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_BitSelect_for_a_b & GND_q & u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_BitSelect_for_b_b;

    -- u0_m0_wo0_wi0_r0_ra0_add_1_0(ADD,30)@13 + 1
    u0_m0_wo0_wi0_r0_ra0_add_1_0_a <= STD_LOGIC_VECTOR("0" & u0_m0_wo0_wi0_r0_ra0_add_0_0_replace_or_join_q);
    u0_m0_wo0_wi0_r0_ra0_add_1_0_b <= STD_LOGIC_VECTOR("00" & u0_m0_wo0_wi0_r0_ra0_count2_q);
    u0_m0_wo0_wi0_r0_ra0_add_1_0_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_wi0_r0_ra0_add_1_0_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            u0_m0_wo0_wi0_r0_ra0_add_1_0_o <= STD_LOGIC_VECTOR(UNSIGNED(u0_m0_wo0_wi0_r0_ra0_add_1_0_a) + UNSIGNED(u0_m0_wo0_wi0_r0_ra0_add_1_0_b));
        END IF;
    END PROCESS;
    u0_m0_wo0_wi0_r0_ra0_add_1_0_q <= u0_m0_wo0_wi0_r0_ra0_add_1_0_o(9 downto 0);

    -- u0_m0_wo0_wi0_r0_ra0_resize(BITSELECT,31)@14
    u0_m0_wo0_wi0_r0_ra0_resize_in <= STD_LOGIC_VECTOR(u0_m0_wo0_wi0_r0_ra0_add_1_0_q(6 downto 0));
    u0_m0_wo0_wi0_r0_ra0_resize_b <= STD_LOGIC_VECTOR(u0_m0_wo0_wi0_r0_ra0_resize_in(6 downto 0));

    -- d_xIn_0_14(DELAY,179)@10 + 4
    d_xIn_0_14 : dspba_delay
    GENERIC MAP ( width => 32, depth => 4, reset_kind => "ASYNC" )
    PORT MAP ( xin => xIn_0, xout => d_xIn_0_14_q, clk => clk, aclr => areset );

    -- d_in0_m0_wi0_wo0_assign_id1_q_14(DELAY,182)@11 + 3
    d_in0_m0_wi0_wo0_assign_id1_q_14 : dspba_delay
    GENERIC MAP ( width => 1, depth => 3, reset_kind => "ASYNC" )
    PORT MAP ( xin => d_in0_m0_wi0_wo0_assign_id1_q_11_q, xout => d_in0_m0_wi0_wo0_assign_id1_q_14_q, clk => clk, aclr => areset );

    -- u0_m0_wo0_wi0_r0_wa0(COUNTER,74)@14
    -- low=0, high=127, step=1, init=0
    u0_m0_wo0_wi0_r0_wa0_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_wi0_r0_wa0_i <= TO_UNSIGNED(0, 7);
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (d_in0_m0_wi0_wo0_assign_id1_q_14_q = "1") THEN
                u0_m0_wo0_wi0_r0_wa0_i <= u0_m0_wo0_wi0_r0_wa0_i + 1;
            END IF;
        END IF;
    END PROCESS;
    u0_m0_wo0_wi0_r0_wa0_q <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR(RESIZE(u0_m0_wo0_wi0_r0_wa0_i, 7)));

    -- u0_m0_wo0_wi0_r0_memr0(DUALMEM,78)@14
    u0_m0_wo0_wi0_r0_memr0_ia <= STD_LOGIC_VECTOR(d_xIn_0_14_q);
    u0_m0_wo0_wi0_r0_memr0_aa <= u0_m0_wo0_wi0_r0_wa0_q;
    u0_m0_wo0_wi0_r0_memr0_ab <= u0_m0_wo0_wi0_r0_ra0_resize_b;
    u0_m0_wo0_wi0_r0_memr0_dmem : altsyncram
    GENERIC MAP (
        ram_block_type => "M9K",
        operation_mode => "DUAL_PORT",
        width_a => 32,
        widthad_a => 7,
        numwords_a => 128,
        width_b => 32,
        widthad_b => 7,
        numwords_b => 128,
        lpm_type => "altsyncram",
        width_byteena_a => 1,
        address_reg_b => "CLOCK0",
        indata_reg_b => "CLOCK0",
        wrcontrol_wraddress_reg_b => "CLOCK0",
        rdcontrol_reg_b => "CLOCK0",
        byteena_reg_b => "CLOCK0",
        outdata_reg_b => "CLOCK0",
        outdata_aclr_b => "NONE",
        clock_enable_input_a => "NORMAL",
        clock_enable_input_b => "NORMAL",
        clock_enable_output_b => "NORMAL",
        read_during_write_mode_mixed_ports => "DONT_CARE",
        power_up_uninitialized => "FALSE",
        init_file => "UNUSED",
        intended_device_family => "Cyclone IV E"
    )
    PORT MAP (
        clocken0 => '1',
        clock0 => clk,
        address_a => u0_m0_wo0_wi0_r0_memr0_aa,
        data_a => u0_m0_wo0_wi0_r0_memr0_ia,
        wren_a => d_in0_m0_wi0_wo0_assign_id1_q_14_q(0),
        address_b => u0_m0_wo0_wi0_r0_memr0_ab,
        q_b => u0_m0_wo0_wi0_r0_memr0_iq
    );
    u0_m0_wo0_wi0_r0_memr0_q <= u0_m0_wo0_wi0_r0_memr0_iq(31 downto 0);

    -- u0_m0_wo0_sym_add0(ADD,90)@14 + 1
    u0_m0_wo0_sym_add0_a <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR((32 downto 32 => u0_m0_wo0_wi0_r0_memr0_q(31)) & u0_m0_wo0_wi0_r0_memr0_q));
    u0_m0_wo0_sym_add0_b <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR((32 downto 32 => u0_m0_wo0_wi0_r0_memr3_q(31)) & u0_m0_wo0_wi0_r0_memr3_q));
    u0_m0_wo0_sym_add0_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_sym_add0_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            u0_m0_wo0_sym_add0_o <= STD_LOGIC_VECTOR(SIGNED(u0_m0_wo0_sym_add0_a) + SIGNED(u0_m0_wo0_sym_add0_b));
        END IF;
    END PROCESS;
    u0_m0_wo0_sym_add0_q <= u0_m0_wo0_sym_add0_o(32 downto 0);

    -- u0_m0_wo0_mtree_mult1_1_bs2_merged_bit_select(BITSELECT,177)@15
    u0_m0_wo0_mtree_mult1_1_bs2_merged_bit_select_b <= STD_LOGIC_VECTOR(u0_m0_wo0_sym_add0_q(32 downto 17));
    u0_m0_wo0_mtree_mult1_1_bs2_merged_bit_select_c <= STD_LOGIC_VECTOR(u0_m0_wo0_sym_add0_q(16 downto 0));

    -- u0_m0_wo0_mtree_mult1_1_im0(MULT,111)@15 + 2
    u0_m0_wo0_mtree_mult1_1_im0_a0 <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_1_bs2_merged_bit_select_b);
    u0_m0_wo0_mtree_mult1_1_im0_b0 <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_1_bs1_merged_bit_select_b);
    u0_m0_wo0_mtree_mult1_1_im0_reset <= areset;
    u0_m0_wo0_mtree_mult1_1_im0_component : lpm_mult
    GENERIC MAP (
        lpm_widtha => 16,
        lpm_widthb => 5,
        lpm_widthp => 21,
        lpm_widths => 1,
        lpm_type => "LPM_MULT",
        lpm_representation => "SIGNED",
        lpm_hint => "DEDICATED_MULTIPLIER_CIRCUITRY=YES, MAXIMIZE_SPEED=5",
        lpm_pipeline => 2
    )
    PORT MAP (
        dataa => u0_m0_wo0_mtree_mult1_1_im0_a0,
        datab => u0_m0_wo0_mtree_mult1_1_im0_b0,
        clken => VCC_q(0),
        aclr => u0_m0_wo0_mtree_mult1_1_im0_reset,
        clock => clk,
        result => u0_m0_wo0_mtree_mult1_1_im0_s1
    );
    u0_m0_wo0_mtree_mult1_1_im0_q <= u0_m0_wo0_mtree_mult1_1_im0_s1;

    -- u0_m0_wo0_mtree_mult1_1_align_21(BITSHIFT,132)@17
    u0_m0_wo0_mtree_mult1_1_align_21_qint <= u0_m0_wo0_mtree_mult1_1_im0_q & "0000000000000000000000000000000000";
    u0_m0_wo0_mtree_mult1_1_align_21_q <= u0_m0_wo0_mtree_mult1_1_align_21_qint(54 downto 0);

    -- u0_m0_wo0_mtree_mult1_1_bjB10(BITJOIN,121)@15
    u0_m0_wo0_mtree_mult1_1_bjB10_q <= GND_q & u0_m0_wo0_mtree_mult1_1_bs2_merged_bit_select_c;

    -- u0_m0_wo0_mtree_mult1_1_im7(MULT,118)@15 + 2
    u0_m0_wo0_mtree_mult1_1_im7_a0 <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_1_bjB10_q);
    u0_m0_wo0_mtree_mult1_1_im7_b0 <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_1_bs1_merged_bit_select_b);
    u0_m0_wo0_mtree_mult1_1_im7_reset <= areset;
    u0_m0_wo0_mtree_mult1_1_im7_component : lpm_mult
    GENERIC MAP (
        lpm_widtha => 18,
        lpm_widthb => 5,
        lpm_widthp => 23,
        lpm_widths => 1,
        lpm_type => "LPM_MULT",
        lpm_representation => "SIGNED",
        lpm_hint => "DEDICATED_MULTIPLIER_CIRCUITRY=YES, MAXIMIZE_SPEED=5",
        lpm_pipeline => 2
    )
    PORT MAP (
        dataa => u0_m0_wo0_mtree_mult1_1_im7_a0,
        datab => u0_m0_wo0_mtree_mult1_1_im7_b0,
        clken => VCC_q(0),
        aclr => u0_m0_wo0_mtree_mult1_1_im7_reset,
        clock => clk,
        result => u0_m0_wo0_mtree_mult1_1_im7_s1
    );
    u0_m0_wo0_mtree_mult1_1_im7_q <= u0_m0_wo0_mtree_mult1_1_im7_s1;

    -- u0_m0_wo0_mtree_mult1_1_align_19(BITSHIFT,130)@17
    u0_m0_wo0_mtree_mult1_1_align_19_qint <= u0_m0_wo0_mtree_mult1_1_im7_q & "00000000000000000";
    u0_m0_wo0_mtree_mult1_1_align_19_q <= u0_m0_wo0_mtree_mult1_1_align_19_qint(39 downto 0);

    -- u0_m0_wo0_mtree_mult1_1_result_add_0_1(ADD,135)@17 + 1
    u0_m0_wo0_mtree_mult1_1_result_add_0_1_a <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR((55 downto 40 => u0_m0_wo0_mtree_mult1_1_align_19_q(39)) & u0_m0_wo0_mtree_mult1_1_align_19_q));
    u0_m0_wo0_mtree_mult1_1_result_add_0_1_b <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR((55 downto 55 => u0_m0_wo0_mtree_mult1_1_align_21_q(54)) & u0_m0_wo0_mtree_mult1_1_align_21_q));
    u0_m0_wo0_mtree_mult1_1_result_add_0_1_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_mtree_mult1_1_result_add_0_1_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            u0_m0_wo0_mtree_mult1_1_result_add_0_1_o <= STD_LOGIC_VECTOR(SIGNED(u0_m0_wo0_mtree_mult1_1_result_add_0_1_a) + SIGNED(u0_m0_wo0_mtree_mult1_1_result_add_0_1_b));
        END IF;
    END PROCESS;
    u0_m0_wo0_mtree_mult1_1_result_add_0_1_q <= u0_m0_wo0_mtree_mult1_1_result_add_0_1_o(55 downto 0);

    -- u0_m0_wo0_mtree_mult1_1_bjA5(BITJOIN,116)@15
    u0_m0_wo0_mtree_mult1_1_bjA5_q <= GND_q & u0_m0_wo0_mtree_mult1_1_bs1_merged_bit_select_c;

    -- u0_m0_wo0_mtree_mult1_1_im3(MULT,114)@15 + 2
    u0_m0_wo0_mtree_mult1_1_im3_a0 <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_1_bjA5_q);
    u0_m0_wo0_mtree_mult1_1_im3_b0 <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_1_bs2_merged_bit_select_b);
    u0_m0_wo0_mtree_mult1_1_im3_reset <= areset;
    u0_m0_wo0_mtree_mult1_1_im3_component : lpm_mult
    GENERIC MAP (
        lpm_widtha => 18,
        lpm_widthb => 16,
        lpm_widthp => 34,
        lpm_widths => 1,
        lpm_type => "LPM_MULT",
        lpm_representation => "SIGNED",
        lpm_hint => "DEDICATED_MULTIPLIER_CIRCUITRY=YES, MAXIMIZE_SPEED=5",
        lpm_pipeline => 2
    )
    PORT MAP (
        dataa => u0_m0_wo0_mtree_mult1_1_im3_a0,
        datab => u0_m0_wo0_mtree_mult1_1_im3_b0,
        clken => VCC_q(0),
        aclr => u0_m0_wo0_mtree_mult1_1_im3_reset,
        clock => clk,
        result => u0_m0_wo0_mtree_mult1_1_im3_s1
    );
    u0_m0_wo0_mtree_mult1_1_im3_q <= u0_m0_wo0_mtree_mult1_1_im3_s1;

    -- u0_m0_wo0_mtree_mult1_1_align_17(BITSHIFT,128)@17
    u0_m0_wo0_mtree_mult1_1_align_17_qint <= u0_m0_wo0_mtree_mult1_1_im3_q & "00000000000000000";
    u0_m0_wo0_mtree_mult1_1_align_17_q <= u0_m0_wo0_mtree_mult1_1_align_17_qint(50 downto 0);

    -- u0_m0_wo0_mtree_mult1_1_im11(MULT,122)@15 + 2
    u0_m0_wo0_mtree_mult1_1_im11_a0 <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_1_bjA5_q);
    u0_m0_wo0_mtree_mult1_1_im11_b0 <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_1_bjB10_q);
    u0_m0_wo0_mtree_mult1_1_im11_reset <= areset;
    u0_m0_wo0_mtree_mult1_1_im11_component : lpm_mult
    GENERIC MAP (
        lpm_widtha => 18,
        lpm_widthb => 18,
        lpm_widthp => 36,
        lpm_widths => 1,
        lpm_type => "LPM_MULT",
        lpm_representation => "SIGNED",
        lpm_hint => "DEDICATED_MULTIPLIER_CIRCUITRY=YES, MAXIMIZE_SPEED=5",
        lpm_pipeline => 2
    )
    PORT MAP (
        dataa => u0_m0_wo0_mtree_mult1_1_im11_a0,
        datab => u0_m0_wo0_mtree_mult1_1_im11_b0,
        clken => VCC_q(0),
        aclr => u0_m0_wo0_mtree_mult1_1_im11_reset,
        clock => clk,
        result => u0_m0_wo0_mtree_mult1_1_im11_s1
    );
    u0_m0_wo0_mtree_mult1_1_im11_q <= u0_m0_wo0_mtree_mult1_1_im11_s1;

    -- u0_m0_wo0_mtree_mult1_1_result_add_0_0(ADD,134)@17 + 1
    u0_m0_wo0_mtree_mult1_1_result_add_0_0_a <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR((51 downto 36 => u0_m0_wo0_mtree_mult1_1_im11_q(35)) & u0_m0_wo0_mtree_mult1_1_im11_q));
    u0_m0_wo0_mtree_mult1_1_result_add_0_0_b <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR((51 downto 51 => u0_m0_wo0_mtree_mult1_1_align_17_q(50)) & u0_m0_wo0_mtree_mult1_1_align_17_q));
    u0_m0_wo0_mtree_mult1_1_result_add_0_0_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_mtree_mult1_1_result_add_0_0_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            u0_m0_wo0_mtree_mult1_1_result_add_0_0_o <= STD_LOGIC_VECTOR(SIGNED(u0_m0_wo0_mtree_mult1_1_result_add_0_0_a) + SIGNED(u0_m0_wo0_mtree_mult1_1_result_add_0_0_b));
        END IF;
    END PROCESS;
    u0_m0_wo0_mtree_mult1_1_result_add_0_0_q <= u0_m0_wo0_mtree_mult1_1_result_add_0_0_o(51 downto 0);

    -- u0_m0_wo0_mtree_mult1_1_result_add_1_0(ADD,136)@18 + 1
    u0_m0_wo0_mtree_mult1_1_result_add_1_0_a <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR((56 downto 52 => u0_m0_wo0_mtree_mult1_1_result_add_0_0_q(51)) & u0_m0_wo0_mtree_mult1_1_result_add_0_0_q));
    u0_m0_wo0_mtree_mult1_1_result_add_1_0_b <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR((56 downto 56 => u0_m0_wo0_mtree_mult1_1_result_add_0_1_q(55)) & u0_m0_wo0_mtree_mult1_1_result_add_0_1_q));
    u0_m0_wo0_mtree_mult1_1_result_add_1_0_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_mtree_mult1_1_result_add_1_0_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            u0_m0_wo0_mtree_mult1_1_result_add_1_0_o <= STD_LOGIC_VECTOR(SIGNED(u0_m0_wo0_mtree_mult1_1_result_add_1_0_a) + SIGNED(u0_m0_wo0_mtree_mult1_1_result_add_1_0_b));
        END IF;
    END PROCESS;
    u0_m0_wo0_mtree_mult1_1_result_add_1_0_q <= u0_m0_wo0_mtree_mult1_1_result_add_1_0_o(56 downto 0);

    -- u0_m0_wo0_cm1(LOOKUP,89)@14 + 1
    u0_m0_wo0_cm1_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_cm1_q <= "0111111111111111111111";
        ELSIF (clk'EVENT AND clk = '1') THEN
            CASE (u0_m0_wo0_ca1_q) IS
                WHEN "000000" => u0_m0_wo0_cm1_q <= "0111111111111111111111";
                WHEN "000001" => u0_m0_wo0_cm1_q <= "0010001001010100101010";
                WHEN "000010" => u0_m0_wo0_cm1_q <= "1101100111111000011001";
                WHEN "000011" => u0_m0_wo0_cm1_q <= "1110100010101110010001";
                WHEN "000100" => u0_m0_wo0_cm1_q <= "0001001101010011111101";
                WHEN "000101" => u0_m0_wo0_cm1_q <= "0001000100101000011111";
                WHEN "000110" => u0_m0_wo0_cm1_q <= "1111010000010011111001";
                WHEN "000111" => u0_m0_wo0_cm1_q <= "1111001001100111100110";
                WHEN "001000" => u0_m0_wo0_cm1_q <= "0000011111111101100001";
                WHEN "001001" => u0_m0_wo0_cm1_q <= "0000101100111111011011";
                WHEN "001010" => u0_m0_wo0_cm1_q <= "1111101001101101001100";
                WHEN "001011" => u0_m0_wo0_cm1_q <= "1111011001101101011010";
                WHEN "001100" => u0_m0_wo0_cm1_q <= "0000001111101110100101";
                WHEN "001101" => u0_m0_wo0_cm1_q <= "0000100001001001010101";
                WHEN "001110" => u0_m0_wo0_cm1_q <= "1111110100111110010010";
                WHEN "001111" => u0_m0_wo0_cm1_q <= "1111100010111101111000";
                WHEN "010000" => u0_m0_wo0_cm1_q <= "0000000111100000001000";
                WHEN "010001" => u0_m0_wo0_cm1_q <= "0000011001100110100010";
                WHEN "010010" => u0_m0_wo0_cm1_q <= "1111111011001100001111";
                WHEN "010011" => u0_m0_wo0_cm1_q <= "1111101001010100011111";
                WHEN "010100" => u0_m0_wo0_cm1_q <= "0000000010101101000011";
                WHEN "010101" => u0_m0_wo0_cm1_q <= "0000010100001000000110";
                WHEN "010110" => u0_m0_wo0_cm1_q <= "1111111110111100010110";
                WHEN "010111" => u0_m0_wo0_cm1_q <= "1111101110001000000101";
                WHEN "011000" => u0_m0_wo0_cm1_q <= "1111111111110000100011";
                WHEN "011001" => u0_m0_wo0_cm1_q <= "0000001111110110110010";
                WHEN "011010" => u0_m0_wo0_cm1_q <= "0000000001010000001101";
                WHEN "011011" => u0_m0_wo0_cm1_q <= "1111110001111101010000";
                WHEN "011100" => u0_m0_wo0_cm1_q <= "1111111101111101100110";
                WHEN "011101" => u0_m0_wo0_cm1_q <= "0000001100011001100110";
                WHEN "011110" => u0_m0_wo0_cm1_q <= "0000000010101000001011";
                WHEN "011111" => u0_m0_wo0_cm1_q <= "1111110101000101100110";
                WHEN "100000" => u0_m0_wo0_cm1_q <= "1111111100111100000110";
                WHEN "100001" => u0_m0_wo0_cm1_q <= "0000001001100011111001";
                WHEN "100010" => u0_m0_wo0_cm1_q <= "0000000011010110111100";
                WHEN "100011" => u0_m0_wo0_cm1_q <= "1111110111101010011110";
                WHEN "100100" => u0_m0_wo0_cm1_q <= "1111111100011101000101";
                WHEN "100101" => u0_m0_wo0_cm1_q <= "0000000111001110011111";
                WHEN "100110" => u0_m0_wo0_cm1_q <= "0000000011101000110100";
                WHEN "100111" => u0_m0_wo0_cm1_q <= "1111111001110001100101";
                WHEN "101000" => u0_m0_wo0_cm1_q <= "1111111100010110001100";
                WHEN "101001" => u0_m0_wo0_cm1_q <= "0000000101010100101110";
                WHEN "101010" => u0_m0_wo0_cm1_q <= "0000000011100110101001";
                WHEN "101011" => u0_m0_wo0_cm1_q <= "1111111011011110111001";
                WHEN "101100" => u0_m0_wo0_cm1_q <= "1111111100011111110001";
                WHEN "101101" => u0_m0_wo0_cm1_q <= "0000000011110011000100";
                WHEN "101110" => u0_m0_wo0_cm1_q <= "0000000011010111001010";
                WHEN "101111" => u0_m0_wo0_cm1_q <= "1111111100110101101010";
                WHEN "110000" => u0_m0_wo0_cm1_q <= "1111111100110011110111";
                WHEN "110001" => u0_m0_wo0_cm1_q <= "0000000010100110011110";
                WHEN "110010" => u0_m0_wo0_cm1_q <= "0000000010111111100111";
                WHEN "110011" => u0_m0_wo0_cm1_q <= "1111111101111000110110";
                WHEN "110100" => u0_m0_wo0_cm1_q <= "1111111101001101111000";
                WHEN "110101" => u0_m0_wo0_cm1_q <= "0000000001101100000001";
                WHEN "110110" => u0_m0_wo0_cm1_q <= "0000000010100100000011";
                WHEN "110111" => u0_m0_wo0_cm1_q <= "1111111110101011010010";
                WHEN "111000" => u0_m0_wo0_cm1_q <= "1111111101101010010000";
                WHEN "111001" => u0_m0_wo0_cm1_q <= "0000000001000000111010";
                WHEN "111010" => u0_m0_wo0_cm1_q <= "0000000010000111100001";
                WHEN "111011" => u0_m0_wo0_cm1_q <= "1111111111001111101111";
                WHEN "111100" => u0_m0_wo0_cm1_q <= "1111111110000110011001";
                WHEN "111101" => u0_m0_wo0_cm1_q <= "0000000000100010011100";
                WHEN "111110" => u0_m0_wo0_cm1_q <= "0000000001101100001101";
                WHEN "111111" => u0_m0_wo0_cm1_q <= "1111111111101000110111";
                WHEN OTHERS => -- unreachable
                               u0_m0_wo0_cm1_q <= (others => '-');
            END CASE;
        END IF;
    END PROCESS;

    -- u0_m0_wo0_mtree_mult1_0_bs1_merged_bit_select(BITSELECT,176)@15
    u0_m0_wo0_mtree_mult1_0_bs1_merged_bit_select_b <= STD_LOGIC_VECTOR(u0_m0_wo0_cm1_q(21 downto 17));
    u0_m0_wo0_mtree_mult1_0_bs1_merged_bit_select_c <= STD_LOGIC_VECTOR(u0_m0_wo0_cm1_q(16 downto 0));

    -- u0_m0_wo0_sym_add1(ADD,91)@14 + 1
    u0_m0_wo0_sym_add1_a <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR((32 downto 32 => u0_m0_wo0_wi0_r0_memr1_q(31)) & u0_m0_wo0_wi0_r0_memr1_q));
    u0_m0_wo0_sym_add1_b <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR((32 downto 32 => u0_m0_wo0_wi0_r0_memr2_q(31)) & u0_m0_wo0_wi0_r0_memr2_q));
    u0_m0_wo0_sym_add1_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_sym_add1_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            u0_m0_wo0_sym_add1_o <= STD_LOGIC_VECTOR(SIGNED(u0_m0_wo0_sym_add1_a) + SIGNED(u0_m0_wo0_sym_add1_b));
        END IF;
    END PROCESS;
    u0_m0_wo0_sym_add1_q <= u0_m0_wo0_sym_add1_o(32 downto 0);

    -- u0_m0_wo0_mtree_mult1_0_bs2_merged_bit_select(BITSELECT,178)@15
    u0_m0_wo0_mtree_mult1_0_bs2_merged_bit_select_b <= STD_LOGIC_VECTOR(u0_m0_wo0_sym_add1_q(32 downto 17));
    u0_m0_wo0_mtree_mult1_0_bs2_merged_bit_select_c <= STD_LOGIC_VECTOR(u0_m0_wo0_sym_add1_q(16 downto 0));

    -- u0_m0_wo0_mtree_mult1_0_im0(MULT,137)@15 + 2
    u0_m0_wo0_mtree_mult1_0_im0_a0 <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_bs2_merged_bit_select_b);
    u0_m0_wo0_mtree_mult1_0_im0_b0 <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_bs1_merged_bit_select_b);
    u0_m0_wo0_mtree_mult1_0_im0_reset <= areset;
    u0_m0_wo0_mtree_mult1_0_im0_component : lpm_mult
    GENERIC MAP (
        lpm_widtha => 16,
        lpm_widthb => 5,
        lpm_widthp => 21,
        lpm_widths => 1,
        lpm_type => "LPM_MULT",
        lpm_representation => "SIGNED",
        lpm_hint => "DEDICATED_MULTIPLIER_CIRCUITRY=YES, MAXIMIZE_SPEED=5",
        lpm_pipeline => 2
    )
    PORT MAP (
        dataa => u0_m0_wo0_mtree_mult1_0_im0_a0,
        datab => u0_m0_wo0_mtree_mult1_0_im0_b0,
        clken => VCC_q(0),
        aclr => u0_m0_wo0_mtree_mult1_0_im0_reset,
        clock => clk,
        result => u0_m0_wo0_mtree_mult1_0_im0_s1
    );
    u0_m0_wo0_mtree_mult1_0_im0_q <= u0_m0_wo0_mtree_mult1_0_im0_s1;

    -- u0_m0_wo0_mtree_mult1_0_align_21(BITSHIFT,158)@17
    u0_m0_wo0_mtree_mult1_0_align_21_qint <= u0_m0_wo0_mtree_mult1_0_im0_q & "0000000000000000000000000000000000";
    u0_m0_wo0_mtree_mult1_0_align_21_q <= u0_m0_wo0_mtree_mult1_0_align_21_qint(54 downto 0);

    -- u0_m0_wo0_mtree_mult1_0_bjB10(BITJOIN,147)@15
    u0_m0_wo0_mtree_mult1_0_bjB10_q <= GND_q & u0_m0_wo0_mtree_mult1_0_bs2_merged_bit_select_c;

    -- u0_m0_wo0_mtree_mult1_0_im7(MULT,144)@15 + 2
    u0_m0_wo0_mtree_mult1_0_im7_a0 <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_bjB10_q);
    u0_m0_wo0_mtree_mult1_0_im7_b0 <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_bs1_merged_bit_select_b);
    u0_m0_wo0_mtree_mult1_0_im7_reset <= areset;
    u0_m0_wo0_mtree_mult1_0_im7_component : lpm_mult
    GENERIC MAP (
        lpm_widtha => 18,
        lpm_widthb => 5,
        lpm_widthp => 23,
        lpm_widths => 1,
        lpm_type => "LPM_MULT",
        lpm_representation => "SIGNED",
        lpm_hint => "DEDICATED_MULTIPLIER_CIRCUITRY=YES, MAXIMIZE_SPEED=5",
        lpm_pipeline => 2
    )
    PORT MAP (
        dataa => u0_m0_wo0_mtree_mult1_0_im7_a0,
        datab => u0_m0_wo0_mtree_mult1_0_im7_b0,
        clken => VCC_q(0),
        aclr => u0_m0_wo0_mtree_mult1_0_im7_reset,
        clock => clk,
        result => u0_m0_wo0_mtree_mult1_0_im7_s1
    );
    u0_m0_wo0_mtree_mult1_0_im7_q <= u0_m0_wo0_mtree_mult1_0_im7_s1;

    -- u0_m0_wo0_mtree_mult1_0_align_19(BITSHIFT,156)@17
    u0_m0_wo0_mtree_mult1_0_align_19_qint <= u0_m0_wo0_mtree_mult1_0_im7_q & "00000000000000000";
    u0_m0_wo0_mtree_mult1_0_align_19_q <= u0_m0_wo0_mtree_mult1_0_align_19_qint(39 downto 0);

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_1(ADD,161)@17 + 1
    u0_m0_wo0_mtree_mult1_0_result_add_0_1_a <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR((55 downto 40 => u0_m0_wo0_mtree_mult1_0_align_19_q(39)) & u0_m0_wo0_mtree_mult1_0_align_19_q));
    u0_m0_wo0_mtree_mult1_0_result_add_0_1_b <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR((55 downto 55 => u0_m0_wo0_mtree_mult1_0_align_21_q(54)) & u0_m0_wo0_mtree_mult1_0_align_21_q));
    u0_m0_wo0_mtree_mult1_0_result_add_0_1_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_mtree_mult1_0_result_add_0_1_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            u0_m0_wo0_mtree_mult1_0_result_add_0_1_o <= STD_LOGIC_VECTOR(SIGNED(u0_m0_wo0_mtree_mult1_0_result_add_0_1_a) + SIGNED(u0_m0_wo0_mtree_mult1_0_result_add_0_1_b));
        END IF;
    END PROCESS;
    u0_m0_wo0_mtree_mult1_0_result_add_0_1_q <= u0_m0_wo0_mtree_mult1_0_result_add_0_1_o(55 downto 0);

    -- u0_m0_wo0_mtree_mult1_0_bjA5(BITJOIN,142)@15
    u0_m0_wo0_mtree_mult1_0_bjA5_q <= GND_q & u0_m0_wo0_mtree_mult1_0_bs1_merged_bit_select_c;

    -- u0_m0_wo0_mtree_mult1_0_im3(MULT,140)@15 + 2
    u0_m0_wo0_mtree_mult1_0_im3_a0 <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_bjA5_q);
    u0_m0_wo0_mtree_mult1_0_im3_b0 <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_bs2_merged_bit_select_b);
    u0_m0_wo0_mtree_mult1_0_im3_reset <= areset;
    u0_m0_wo0_mtree_mult1_0_im3_component : lpm_mult
    GENERIC MAP (
        lpm_widtha => 18,
        lpm_widthb => 16,
        lpm_widthp => 34,
        lpm_widths => 1,
        lpm_type => "LPM_MULT",
        lpm_representation => "SIGNED",
        lpm_hint => "DEDICATED_MULTIPLIER_CIRCUITRY=YES, MAXIMIZE_SPEED=5",
        lpm_pipeline => 2
    )
    PORT MAP (
        dataa => u0_m0_wo0_mtree_mult1_0_im3_a0,
        datab => u0_m0_wo0_mtree_mult1_0_im3_b0,
        clken => VCC_q(0),
        aclr => u0_m0_wo0_mtree_mult1_0_im3_reset,
        clock => clk,
        result => u0_m0_wo0_mtree_mult1_0_im3_s1
    );
    u0_m0_wo0_mtree_mult1_0_im3_q <= u0_m0_wo0_mtree_mult1_0_im3_s1;

    -- u0_m0_wo0_mtree_mult1_0_align_17(BITSHIFT,154)@17
    u0_m0_wo0_mtree_mult1_0_align_17_qint <= u0_m0_wo0_mtree_mult1_0_im3_q & "00000000000000000";
    u0_m0_wo0_mtree_mult1_0_align_17_q <= u0_m0_wo0_mtree_mult1_0_align_17_qint(50 downto 0);

    -- u0_m0_wo0_mtree_mult1_0_im11(MULT,148)@15 + 2
    u0_m0_wo0_mtree_mult1_0_im11_a0 <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_bjA5_q);
    u0_m0_wo0_mtree_mult1_0_im11_b0 <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_bjB10_q);
    u0_m0_wo0_mtree_mult1_0_im11_reset <= areset;
    u0_m0_wo0_mtree_mult1_0_im11_component : lpm_mult
    GENERIC MAP (
        lpm_widtha => 18,
        lpm_widthb => 18,
        lpm_widthp => 36,
        lpm_widths => 1,
        lpm_type => "LPM_MULT",
        lpm_representation => "SIGNED",
        lpm_hint => "DEDICATED_MULTIPLIER_CIRCUITRY=YES, MAXIMIZE_SPEED=5",
        lpm_pipeline => 2
    )
    PORT MAP (
        dataa => u0_m0_wo0_mtree_mult1_0_im11_a0,
        datab => u0_m0_wo0_mtree_mult1_0_im11_b0,
        clken => VCC_q(0),
        aclr => u0_m0_wo0_mtree_mult1_0_im11_reset,
        clock => clk,
        result => u0_m0_wo0_mtree_mult1_0_im11_s1
    );
    u0_m0_wo0_mtree_mult1_0_im11_q <= u0_m0_wo0_mtree_mult1_0_im11_s1;

    -- u0_m0_wo0_mtree_mult1_0_result_add_0_0(ADD,160)@17 + 1
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_a <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR((51 downto 36 => u0_m0_wo0_mtree_mult1_0_im11_q(35)) & u0_m0_wo0_mtree_mult1_0_im11_q));
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_b <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR((51 downto 51 => u0_m0_wo0_mtree_mult1_0_align_17_q(50)) & u0_m0_wo0_mtree_mult1_0_align_17_q));
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_mtree_mult1_0_result_add_0_0_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            u0_m0_wo0_mtree_mult1_0_result_add_0_0_o <= STD_LOGIC_VECTOR(SIGNED(u0_m0_wo0_mtree_mult1_0_result_add_0_0_a) + SIGNED(u0_m0_wo0_mtree_mult1_0_result_add_0_0_b));
        END IF;
    END PROCESS;
    u0_m0_wo0_mtree_mult1_0_result_add_0_0_q <= u0_m0_wo0_mtree_mult1_0_result_add_0_0_o(51 downto 0);

    -- u0_m0_wo0_mtree_mult1_0_result_add_1_0(ADD,162)@18 + 1
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_a <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR((56 downto 52 => u0_m0_wo0_mtree_mult1_0_result_add_0_0_q(51)) & u0_m0_wo0_mtree_mult1_0_result_add_0_0_q));
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_b <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR((56 downto 56 => u0_m0_wo0_mtree_mult1_0_result_add_0_1_q(55)) & u0_m0_wo0_mtree_mult1_0_result_add_0_1_q));
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_mtree_mult1_0_result_add_1_0_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            u0_m0_wo0_mtree_mult1_0_result_add_1_0_o <= STD_LOGIC_VECTOR(SIGNED(u0_m0_wo0_mtree_mult1_0_result_add_1_0_a) + SIGNED(u0_m0_wo0_mtree_mult1_0_result_add_1_0_b));
        END IF;
    END PROCESS;
    u0_m0_wo0_mtree_mult1_0_result_add_1_0_q <= u0_m0_wo0_mtree_mult1_0_result_add_1_0_o(56 downto 0);

    -- u0_m0_wo0_mtree_add0_0(ADD,94)@19 + 1
    u0_m0_wo0_mtree_add0_0_a <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_0_result_add_1_0_q(55 downto 0));
    u0_m0_wo0_mtree_add0_0_b <= STD_LOGIC_VECTOR(u0_m0_wo0_mtree_mult1_1_result_add_1_0_q(55 downto 0));
    u0_m0_wo0_mtree_add0_0_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_mtree_add0_0_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            u0_m0_wo0_mtree_add0_0_o <= STD_LOGIC_VECTOR(SIGNED(u0_m0_wo0_mtree_add0_0_a) + SIGNED(u0_m0_wo0_mtree_add0_0_b));
        END IF;
    END PROCESS;
    u0_m0_wo0_mtree_add0_0_q <= u0_m0_wo0_mtree_add0_0_o(55 downto 0);

    -- u0_m0_wo0_accum(ADD,97)@20 + 1
    u0_m0_wo0_accum_a <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR((61 downto 56 => u0_m0_wo0_mtree_add0_0_q(55)) & u0_m0_wo0_mtree_add0_0_q));
    u0_m0_wo0_accum_b <= STD_LOGIC_VECTOR(u0_m0_wo0_adelay_q);
    u0_m0_wo0_accum_i <= u0_m0_wo0_accum_a;
    u0_m0_wo0_accum_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_accum_o <= (others => '0');
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (d_u0_m0_wo0_compute_q_20_q = "1") THEN
                IF (u0_m0_wo0_aseq_q = "1") THEN
                    u0_m0_wo0_accum_o <= u0_m0_wo0_accum_i;
                ELSE
                    u0_m0_wo0_accum_o <= STD_LOGIC_VECTOR(SIGNED(u0_m0_wo0_accum_a) + SIGNED(u0_m0_wo0_accum_b));
                END IF;
            END IF;
        END IF;
    END PROCESS;
    u0_m0_wo0_accum_q <= u0_m0_wo0_accum_o(61 downto 0);

    -- u0_m0_wo0_oseq(SEQUENCE,98)@18 + 1
    u0_m0_wo0_oseq_clkproc: PROCESS (clk, areset)
        variable u0_m0_wo0_oseq_c : SIGNED(9 downto 0);
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_oseq_c := "0001111110";
            u0_m0_wo0_oseq_q <= "0";
            u0_m0_wo0_oseq_eq <= '0';
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (d_u0_m0_wo0_compute_q_18_q = "1") THEN
                IF (u0_m0_wo0_oseq_c = "1111111111") THEN
                    u0_m0_wo0_oseq_eq <= '1';
                ELSE
                    u0_m0_wo0_oseq_eq <= '0';
                END IF;
                IF (u0_m0_wo0_oseq_eq = '1') THEN
                    u0_m0_wo0_oseq_c := u0_m0_wo0_oseq_c + 127;
                ELSE
                    u0_m0_wo0_oseq_c := u0_m0_wo0_oseq_c - 1;
                END IF;
                u0_m0_wo0_oseq_q <= STD_LOGIC_VECTOR(u0_m0_wo0_oseq_c(9 downto 9));
            END IF;
        END IF;
    END PROCESS;

    -- u0_m0_wo0_oseq_gated(LOGICAL,99)@19
    u0_m0_wo0_oseq_gated_q <= u0_m0_wo0_oseq_q and d_u0_m0_wo0_compute_q_19_q;

    -- u0_m0_wo0_oseq_gated_reg(REG,100)@19 + 1
    u0_m0_wo0_oseq_gated_reg_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            u0_m0_wo0_oseq_gated_reg_q <= "0";
        ELSIF (clk'EVENT AND clk = '1') THEN
            u0_m0_wo0_oseq_gated_reg_q <= STD_LOGIC_VECTOR(u0_m0_wo0_oseq_gated_q);
        END IF;
    END PROCESS;

    -- outchan(COUNTER,105)@20 + 1
    -- low=0, high=1, step=1, init=1
    outchan_clkproc: PROCESS (clk, areset)
    BEGIN
        IF (areset = '1') THEN
            outchan_i <= TO_UNSIGNED(1, 1);
        ELSIF (clk'EVENT AND clk = '1') THEN
            IF (u0_m0_wo0_oseq_gated_reg_q = "1") THEN
                outchan_i <= outchan_i + 1;
            END IF;
        END IF;
    END PROCESS;
    outchan_q <= STD_LOGIC_VECTOR(STD_LOGIC_VECTOR(RESIZE(outchan_i, 2)));

    -- d_out0_m0_wo0_assign_id3_q_21(DELAY,188)@20 + 1
    d_out0_m0_wo0_assign_id3_q_21 : dspba_delay
    GENERIC MAP ( width => 1, depth => 1, reset_kind => "ASYNC" )
    PORT MAP ( xin => u0_m0_wo0_oseq_gated_reg_q, xout => d_out0_m0_wo0_assign_id3_q_21_q, clk => clk, aclr => areset );

    -- xOut(PORTOUT,106)@21 + 1
    xOut_v <= d_out0_m0_wo0_assign_id3_q_21_q;
    xOut_c <= STD_LOGIC_VECTOR("000000" & outchan_q);
    xOut_0 <= u0_m0_wo0_accum_q;

END normal;
