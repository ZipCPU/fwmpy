////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	umpy_12x12.v
//		
// Project:	A multiply core generator
//
// Purpose:	This verilog file multiplies two unsigned numbers together,
//		without using any hardware acceleration.  This file is
//	computer generated, so please (for your sake) don't make any edits
//	to this file lest you regenerate it and your edits be lost.
//
//
// Creator:	Dan Gisselquist, Ph.D.
//		Gisselquist Technology, LLC
//
////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2015,2017-2019, Gisselquist Technology, LLC
//
// This program is free software (firmware): you can redistribute it and/or
// modify it under the terms of  the GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTIBILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program.  If not, see <http://www.gnu.org/licenses/> for a
// copy.
//
// License:	GPL, v3, as defined and found on www.gnu.org,
//		http://www.gnu.org/licenses/gpl.html
//
//
////////////////////////////////////////////////////////////////////////////////
module umpy_12x12(i_clk, i_reset, i_ce, i_a, i_b, i_aux, o_p, o_aux);
	parameter	NA=12, NB=12;
	input					i_clk, i_reset, i_ce;
	input		signed	[(NA-1):0]	i_a;
	input		signed	[(NB-1):0]	i_b;
	input					i_aux;
	output	reg	signed	[(NA+NB-1):0]	o_p;
	output	reg				o_aux;

	localparam NS = (NA < NB) ? NA : NB;
	localparam NL = (NA < NB) ? NB : NA;
	wire	[(NS-1):0]	i_s;	// Smaller input
	wire	[(NL-1):0]	i_l;	// larger input

	//
	// Adjust our inputs so that i_s has the fewest bits, and i_b the most
	generate if (NA < NB)
	begin : BITADJ
		assign	i_s = i_a;
		assign	i_l = i_b;
	end else begin
		assign	i_s = i_b;
		assign	i_l = i_a;
	end endgenerate

	// Clock zero: build our Tableau only.
	// There will be one row for every pair of bits in i_a, and each
	// row will contain (AW+3) bits, to allow
	// for signed arithmetic manipulation.
	//
	reg	A_0;

	wire	[13:0]	S_0_00;
	bimpy #(NB) initialmpy_0_0(i_clk, i_reset, i_ce, i_s[1:0], i_l, S_0_00);

	wire	[13:0]	S_0_01;
	bimpy #(NB) initialmpy_1_0(i_clk, i_reset, i_ce, i_s[3:2], i_l, S_0_01);

	wire	[13:0]	S_0_02;
	bimpy #(NB) initialmpy_2_0(i_clk, i_reset, i_ce, i_s[5:4], i_l, S_0_02);

	wire	[13:0]	S_0_03;
	bimpy #(NB) initialmpy_3_0(i_clk, i_reset, i_ce, i_s[7:6], i_l, S_0_03);

	wire	[13:0]	S_0_04;
	bimpy #(NB) initialmpy_4_0(i_clk, i_reset, i_ce, i_s[9:8], i_l, S_0_04);

	wire	[13:0]	S_0_05;
	bimpy #(NB) initialmpy_5_0(i_clk, i_reset, i_ce, i_s[11:10], i_l, S_0_05);

	initial	A_0 = 0;
	always @(posedge i_clk)
	if(i_reset)
		A_0 <= 1'b0;
	else if (i_ce)
		A_0 <= i_aux;

	//
	// Round #1, clock = 1, nz = 2, nbits = 14, nrows_in = 6
	//

	reg	[(17-1):0]	S_1_00; // maxbits = 24
	reg	[(17-1):0]	S_1_01; // maxbits = 24
	reg	[(17-1):0]	S_1_02; // maxbits = 24
	reg	A_1;

	initial	S_1_00 = 0;
	initial	S_1_01 = 0;
	initial	S_1_02 = 0;
	always @(posedge i_clk)
	if(i_reset)
	begin
		S_1_00 <= 0;
		S_1_01 <= 0;
		S_1_02 <= 0;
	end else if (i_ce)
	begin
		S_1_00 <= { 2'b0, S_0_00 } + { S_0_01, 2'b0 };
		S_1_01 <= { 2'b0, S_0_02 } + { S_0_03, 2'b0 };
		S_1_02 <= { 2'b0, S_0_04 } + { S_0_05, 2'b0 };
	end

	initial	A_1 = 0;
	always @(posedge i_clk)
	if(i_reset)
		A_1 <= 1'b0;
	else if (i_ce)
		A_1 <= A_0;

	//
	// Round #2, clock = 2, nz = 4, nbits = 17, nrows_in = 3
	//

	reg	[(22-1):0]	S_2_00; // maxbits = 24
	reg	[(22-1):0]	S_2_01; // maxbits = 24
	reg	A_2;

	initial	S_2_00 = 0;
	initial	S_2_01 = 0;
	always @(posedge i_clk)
	if(i_reset)
	begin
		S_2_00 <= 0;
		S_2_01 <= 0;
	end else if (i_ce)
	begin
		S_2_00 <= { 4'b0, S_1_00 } + { S_1_01, 4'b0 };
		S_2_01 <= { 5'b0, S_1_02 };
	end

	initial	A_2 = 0;
	always @(posedge i_clk)
	if(i_reset)
		A_2 <= 1'b0;
	else if (i_ce)
		A_2 <= A_1;

	//
	// Round #3, clock = 3, nz = 8, nbits = 22, nrows_in = 2
	//

	reg	[(24-1):0]	S_3_00; // maxbits = 24
	reg	A_3;

	initial	S_3_00 = 0;
	always @(posedge i_clk)
	if(i_reset)
	begin
		S_3_00 <= 0;
	end else if (i_ce)
	begin
		S_3_00 <= { 2'b0, S_2_00 } + { S_2_01// Adding to unused: 31, 24
[15:0], 8'b0 };

// unused = 6, ustr = S_2_01[21:16]
	end

	initial	A_3 = 0;
	always @(posedge i_clk)
	if(i_reset)
		A_3 <= 1'b0;
	else if (i_ce)
		A_3 <= A_2;

	assign	o_p = S_3_00[(NA+NB-1):0];
	assign	o_aux = A_3;

	// Make verilator happy
	// verilator lint_off UNUSED
	wire	[6-1:0]	unused;
	assign	unused = { S_2_01[21:16] };
	// verilator lint_on  UNUSED



`ifdef	FORMAL

	reg	f_past_valid;
	initial	f_past_valid = 0;
	always @(posedge i_clk)
		f_past_valid <= 1'b1;

	wire	[3+1:0]	f_auxpipe;
	assign	f_auxpipe	= { A_3,A_2,A_1,A_0, i_aux };

	initial	assume(!i_aux);
	always @(posedge i_clk)
	if ((i_reset)||((f_past_valid)&&($past(i_reset))))
		assume(!i_aux);

	initial	assert(f_auxpipe == 0);
	always @(posedge i_clk)
	if ((f_past_valid)&&($past(i_reset)))
		assert(f_auxpipe == 0);
	always @(posedge i_clk)
	if ((f_past_valid)&&(!$past(i_reset))&&($past(i_ce)))
		assert(f_auxpipe[3+1:1] == $past(f_auxpipe[3:0]));

	always @(posedge i_clk)
	if ((f_past_valid)&&(!$past(i_reset))&&(!$past(i_ce)))
		assert(f_auxpipe[3+1:1] == $past(f_auxpipe[3+1:1]));

	localparam	F_DELAY = 3;
	reg	[NA+NB-1:0]	f_result;
	integer			ik;

	initial	f_result = 0;
	always @(posedge i_clk)
	if(i_reset)
		f_result = 0;
	else if (i_ce)
	begin
		f_result = 0;
		for(ik=0; ik<NS; ik=ik+1)
			if(i_a[ik])
				f_result = f_result + { {(NL-ik-1){1'b0}},
						i_b, { (ik){1'b0} } };
	end

	reg	[F_DELAY*(NA+NB)-1:0]	f_result_pipe;

	initial	f_result_pipe = 0;
	always @(posedge i_clk)
	if(i_reset)
		f_result_pipe <= 0;
	else if (i_ce)
		f_result_pipe <= { f_result, f_result_pipe[((F_DELAY)*(NA+NB)-1):(NA+NB)] };

	always @(posedge i_clk)
		assert(o_p == f_result_pipe[(NA+NB-1):0]);

	always @(posedge i_clk)
		assume((i_ce)
			||((f_past_valid)&&($past(i_ce)))
			// ||(($past(f_past_valid))&&($past(i_ce,2)))
			);

`endif

endmodule
