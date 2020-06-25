////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	slowmpy.v
//
// Project:	A multiply core generator
//
// Purpose:	This is a signed (OPT_SIGNED=1) or unsigned (OPT_SIGNED=0)
// 		multiply designed for low logic and slow data signals.  It
// 	takes one clock per bit plus two more to complete the multiply.
//
//	The OPT_SIGNED version of this algorithm was found on Wikipedia at
//	https://en.wikipedia.org/wiki/Binary_multiplier.
//
// Creator:	Dan Gisselquist, Ph.D.
//		Gisselquist Technology, LLC
//
////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018-2020, Gisselquist Technology, LLC
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
// with this program.  (It's in the $(ROOT)/doc directory.  Run make with no
// target there if the PDF file isn't present.)  If not, see
// <http://www.gnu.org/licenses/> for a copy.
//
// License:	GPL, v3, as defined and found on www.gnu.org,
//		http://www.gnu.org/licenses/gpl.html
//
//
////////////////////////////////////////////////////////////////////////////////
//
//
`default_nettype	none
//
//
module	slowmpy(i_clk, i_reset, i_stb, i_a_unsorted, i_b_unsorted, i_aux, o_busy,
		o_done, o_p, o_aux);
	parameter				LGN = 4;
	parameter	[LGNA:0]	IA = 12, // Number of bits in A
						IB = 11;
	parameter	[0:0]			OPT_SIGNED = 1'b1;
	// IB is the number of bits in B.  It must be == IA for the OPT_SIGNED
	// option to work properly
	//
	input	wire				i_clk, i_reset;
	//
	input	wire				i_stb;
	input	wire	signed	[(IA-1):0]	i_a_unsorted;
	input	wire	signed	[(IB-1):0]	i_b_unsorted;
	input	wire				i_aux;
	output	wire				o_busy, o_done;
	output	reg	signed	[(NA+NB-1):0]	o_p;
	output	reg				o_aux;

	localparam	NA = (IA > IB) ? IA : IB;	// NA is bigger
	localparam	NB = (IA > IB) ? IB : IA;
	// localparam	NB = NA;// Must be = NA for OPT_SIGNED to work

	wire	[NA-1:0]	i_a;
	wire	[NB-1:0]	i_b;

	generate if (IA > IB)
	begin

		assign	i_a = i_b_unsorted;
		assign	i_b = i_a_unsorted;

	end else begin

		assign	i_a = i_a_unsorted;
		assign	i_b = i_b_unsorted;

	end endgenerate

	reg	[LGN-1:0]	count;
	reg	[NA-1:0]	p_a;
	reg	[NB-1:0]	p_b;
	reg	[NA+NB-1:0]	partial;
	reg			aux;

	reg	almost_done;

	wire	pre_done;
	assign	pre_done = (count == 0);
	initial	almost_done = 1'b0;
	always @(posedge i_clk)
		almost_done <= (!i_reset)&&(o_busy)&&(pre_done);

	initial	aux    = 0;
	initial	o_done = 0;
	initial	o_busy = 0;
	always @(posedge i_clk)
	if (i_reset)
	begin
		aux    <= 0;
		o_done <= 0;
		o_busy <= 0;
	end else if ((!o_busy)&&(i_stb))
	begin
		o_done <= 0;
		o_busy <= 1;
		aux    <= i_aux;
	end else if ((o_busy)&&(almost_done))
	begin
		o_done <= 1;
		o_busy <= 0;
	end else
		o_done <= 0;

	wire	[NA-1:0]	pwire;
	assign	pwire = (p_b[0] ? p_a : 0);

	always @(posedge i_clk)
	if (!o_busy)
	begin
		count <= NB[LGN-1:0]-1;
		partial <= 0;
		p_a <= i_a;
		p_b <= i_b;
	end else begin
		p_b <= (p_b >> 1);

		partial[NB-2:0] <= partial[NB-1:1];
		if ((OPT_SIGNED)&&(NA == NB)&&(pre_done))
			partial[NA+NB-1:NB-1] <= { 1'b0, partial[NA+NB-1:NB]} +
				{ 1'b0, pwire[NA-1], ~pwire[NA-2:0] };
		else if (OPT_SIGNED)
			partial[NA+NB-1:NB-1] <= {1'b0,partial[NA+NB-1:NB]} +
				{ 1'b0, !pwire[NA-1], pwire[NA-2:0] };
		else
			partial[NA+NB-1:NB-1] <= {1'b0, partial[NA+NB-1:NB]}
				+ ((p_b[0]) ? {1'b0,p_a} : 0);
		count <= count - 1;
	end

	wire	[NA+NB-1:0]	final_bit_fix;
	assign	final_bit_fix = (NA == NB)
			? {1'b1,{(NA-2){1'b0}},1'b1, {(NB){1'b0}}}
			: ({1'b1,{(NB-1){1'b0}}, 1'b1, {(NA-1){1'b0}}}
	+0);//		+ ((NA == NB+1) ? 0
	//		: {2'b00,{(NA-NB-1){1'b1}}, {(2*NB-3){1'b0}}}));

	always @(posedge i_clk)
	if (almost_done)
	begin
		if (OPT_SIGNED)
			o_p   <= partial + final_bit_fix;
		else
			o_p   <= partial;
		o_aux <= aux;
	end

`ifdef	FORMAL
`ifdef	SLOWMPY
`define	ASSUME	assume
`define	ASSERT	assert
`else
`define	ASSUME	assert
`define	ASSERT	assume
`endif

	reg	f_past_valid;
	initial	f_past_valid = 1'b0;
	always @(posedge i_clk)
		f_past_valid <= 1'b1;
	initial	assume(i_reset);
	always @(*)
	if (!f_past_valid)
		`ASSUME(i_reset);

	always @(posedge i_clk)
	if ((!f_past_valid)||($past(i_reset)))
	begin
		`ASSERT(almost_done == 0);
		`ASSERT(o_done == 0);
		`ASSERT(o_busy == 0);
		`ASSERT(aux == 0);
	end

	// Assumptions about our inputs
	always @(posedge i_clk)
	if ((f_past_valid)&&(!$past(i_reset))&&($past(i_stb))&&($past(o_busy)))
	begin
		`ASSUME(i_stb);
		`ASSUME($stable(i_a));
		`ASSUME($stable(i_b));
	end

	//
	// For now, just formally verify our internal signaling
	//

	always @(posedge i_clk)
		`ASSERT(almost_done == (o_busy&&(&count)));

	always @(*)
		if (!(&count[LGN-1:1])||(count[0]))
			`ASSERT(!o_done);

	always @(posedge i_clk)
	if (o_done)
		`ASSERT(!o_busy);
	always @(posedge i_clk)
	if (!o_busy)
		`ASSERT(!almost_done);

	reg	signed [NA-1:0]	f_a;
	reg	signed [NB-1:0]	f_b;
	always @(posedge i_clk)
	if ((i_stb)&&(!o_busy))
	begin
		f_a <= i_a;
		f_b <= i_b;
	end

	wire	[NA-1:0]	f_a_neg;
	wire	[NB-1:0]	f_b_neg;

	assign	f_a_neg = -f_a;
	assign	f_b_neg = -f_b;

	always @(*)
	if (o_done)
	begin
		if ((f_a == 0)||(f_b == 0))
			`ASSERT(o_p == 0);
		else if ((f_a == {1'b1,{(NA-1){1'b0}}})
			||(f_b == {1'b1,{(NB-1){1'b0}}}))
			;
		else if (f_a == 1)
		begin
			`ASSERT(o_p[NB-1:0] == f_b);
			if (OPT_SIGNED)
				`ASSERT(o_p[NA+NB-1:NA] == {(NA){f_b[NB-1]}});
		end else if (f_b == 1)
		begin
			`ASSERT(o_p[NA-1:0] == f_a);
			if (OPT_SIGNED)
				`ASSERT(o_p[NA+NB-1:NB] == {(NB){f_a[NA-1]}});
		end else if ((OPT_SIGNED)&&(&f_a))
		begin
			`ASSERT(o_p[NB-1:0] == f_b_neg);
			`ASSERT(o_p[NA+NB-1:NA] == {(NA){f_b_neg[NB-1]}});
		end else if ((OPT_SIGNED)&&(&f_b))
		begin
			`ASSERT(o_p[NA-1:0] == f_a_neg);
			`ASSERT(o_p[NA+NB-1:NB] == {(NB){f_a_neg[NA-1]}});
		end else begin
			`ASSERT((!OPT_SIGNED)
				||(o_p[NA+NB-1] == (f_a[NA-1] ^ f_b[NB-1])));
			`ASSERT(o_p[NA+NB-1:0] != 0);
		end
	end

	always @(posedge i_clk)
		cover(o_done);

	reg	f_past_done;
	initial	f_past_done = 1'b0;
	always @(posedge i_clk)
	if (o_done)
		f_past_done = 1'b1;

	always @(posedge i_clk)
		cover((o_done)&&(f_past_done));
`endif
endmodule
