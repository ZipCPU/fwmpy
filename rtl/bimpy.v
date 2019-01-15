////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	bimpy
//
// Project:	A multiply core generator
//
// Purpose:	An unsigned 2-bit multiply based upon the fact that LUT's allow
//		6-bits of input, but a 2x2 bit multiply will never carry more
//	than one bit.  While this multiply is hardware independent, it is
//	really motivated by trying to optimize for a specific piece of
//	hardware (Xilinx-7 series ...) that has 4-input LUT's with carry
//	chains.
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
module	bimpy(i_clk, i_reset, i_ce, i_a, i_b, o_r);
	parameter	BW=18, LUTB=2;
	input				i_clk, i_reset, i_ce;
	input		[(LUTB-1):0]	i_a;
	input		[(BW-1):0]	i_b;
	output	reg	[(BW+LUTB-1):0]	o_r;

	wire	[(BW+LUTB-2):0]	w_r;
	wire	[(BW+LUTB-3):1]	c;

	assign	w_r =  { ((i_a[1])?i_b:{(BW){1'b0}}), 1'b0 }
				^ { 1'b0, ((i_a[0])?i_b:{(BW){1'b0}}) };
	assign	c = { ((i_a[1])?i_b[(BW-2):0]:{(BW-1){1'b0}}) }
			& ((i_a[0])?i_b[(BW-1):1]:{(BW-1){1'b0}});

	initial	o_r = 0;
	always @(posedge i_clk)
		if (i_reset)
			o_r <= 0;
		else if (i_ce)
			o_r <= w_r + { c, 2'b0 };

endmodule
