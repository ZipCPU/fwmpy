////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	sgnmpy_32x32.v
//		
// Project:	A multiply core generator
//
// Purpose:	Turns a signed multiply into an unsigned multiply, at the cost
//	of two clocks and a negation.
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
module sgnmpy_32x32(i_clk, i_reset, i_ce, i_a, i_b, i_aux, o_p, o_aux);
	parameter	NA=32, NB=32, DLY=6;
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

	reg		[(NS-1):0]	u_s;
	reg		[(NL-1):0]	u_l;
	reg		[(DLY-1):0]	u_sgn;
	reg				u_aux;

	initial	u_aux = 1'b0;
	always @(posedge i_clk)
	if(i_reset)
			u_aux <= 1'b0;
		else if (i_ce)
			u_aux <= i_aux;

	initial	u_s = 0;
	initial	u_l = 0;
	always @(posedge i_clk)
	if(i_reset)
	begin
		u_s <= 0;
		u_l <= 0;
	end else if (i_ce)
	begin
		u_s <= (i_s[NS-1])?(-i_s):i_s;
		u_l <= (i_l[NL-1])?(-i_l):i_l;
	end

	initial	u_sgn = 0;
	always @(posedge i_clk)
	if(i_reset)
		u_sgn <= 0;
	else if (i_ce)
		u_sgn <= { u_sgn[(DLY-2):0], ((i_s[NS-1])^(i_l[NL-1])) };

	wire	[(NA+NB-1):0]	u_r;
	wire			w_aux;
	umpy_32x32	umpy(i_clk, i_reset, i_ce, u_s, u_l, u_aux, u_r, w_aux);

	initial	o_p = 0;
	always @(posedge i_clk)
	if(i_reset)
		o_p <= 0;
	else if (i_ce)
		o_p <= (u_sgn[DLY-1])?(-u_r):u_r;


	initial	o_aux = 1'b0;
	always @(posedge i_clk)
	if(i_reset)
		o_aux <= 1'b0;
	else if (i_ce)
		o_aux <= w_aux;


endmodule
