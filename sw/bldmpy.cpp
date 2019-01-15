////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	bldmpy.cpp
//
// Project:	A multiply core generator
//
// Purpose:	This is the product of studying the signed, two's complement, 
//		multiply operation presented on wikipedia,
//
//		http://en.wikipedia.org/wiki/Binary_multiplier
//
//	Basically, this file supports the routine, bldmpy, which builds a
//	multiply for any pair of bitwidths.  While I didn't really want to
//	build this in a coregen fashion, such as you have here, the multiply
//	operator kept changing depending upon the bitwidth.  I can handle such
//	changes in C++, so ... thus I have computer generated multiply code.
//	As a special benefit, the number of clock cycles that this routine
//	takes is given by ceiling of the base two logarithm of the number of
//	bits in the greater operand, plus one more.  Thus a 16x20 multiply
//	requires 6 clocks, whereas a 64x96 bit multiply (if you could build
//	it), would require 8 clocks.  Not bad, huh?  Oh, and the memory
//	requirements get smaller at each stage, as opposed to the shiftadd
//	multiply where the requirements were constant on a per stage level,
//	and often many more than I needed.
//
//	The basic operation is to create a "tableau" of words to add together.
//	This is my term for the result of all of the one bit multiplies
//	(i_a[x] * i_b), and often written out between two horizontal lines
//	while doing long division.  This "tableau" is then added together to
//	create the result.  Since so many lines need to be added together, we
//	add two lines at a time, and leave no line resting.  Therefore, every
//	even line gets added to an odd line at every clock to create a new
//	tableau that is roughly half the size of the previous.  The operation
//	then repeats and so forth.
//
//	I'm not sure I can explain the signed arithmetic extensions, but that's
//	what adds the extra 1's into the tableau and what ends up complementing
//	various bits along the way.
//
// Creator:	Dan Gisselquist, Ph.D.
//		Gisselquist Technology, LLC
//
////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2015-2019, Gisselquist Technology, LLC
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
const char	cpyleft[] = 
"////////////////////////////////////////////////////////////////////////////////\n"
"//\n"
"// Copyright (C) 2015,2017-2019, Gisselquist Technology, LLC\n"
"//\n"
"// This program is free software (firmware): you can redistribute it and/or\n"
"// modify it under the terms of  the GNU General Public License as published\n"
"// by the Free Software Foundation, either version 3 of the License, or (at\n"
"// your option) any later version.\n"
"//\n"
"// This program is distributed in the hope that it will be useful, but WITHOUT\n"
"// ANY WARRANTY; without even the implied warranty of MERCHANTIBILITY or\n"
"// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License\n"
"// for more details.\n"
"//\n"
"// You should have received a copy of the GNU General Public License along\n"
"// with this program.  If not, see <http://www.gnu.org/licenses/> for a\n"
"// copy.\n"
"//\n"
"// License:	GPL, v3, as defined and found on www.gnu.org,\n"
"//		http://www.gnu.org/licenses/gpl.html\n"
"//\n"
"//\n"
"////////////////////////////////////////////////////////////////////////////////\n";
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <assert.h>

const char	prjname[] = "A multiply core generator";
const char	creator[] =	"// Creator:	Dan Gisselquist, Ph.D.\n"
				"//		Gisselquist Technology, LLC\n";

bool	verbose_flag = true;

int	lg(int v) {
	int	m=1, r=0;

	assert(v>0);
	while(m<v) {
		m<<=1;
		r++;
	}

	return r;
}

int	npremul(int premul, int na, int nb) {
	if (na < nb)
		return (na + premul-1)/premul;
	else
		return (nb + premul-1)/premul;
}

int	post_stages(int npreouts) {
	return lg(npreouts);
}

int	stages(int premul, int na, int nb) {
	int	ps = npremul(premul, na, nb);
	return 1+post_stages(ps);
}

void	buildbimpy(FILE *fp, char *name, bool async_reset) {
	fprintf(fp,
"////////////////////////////////////////////////////////////////////////////////\n"
"//\n"
"// Filename: 	%s\n"
"//\n"
"// Project:	%s\n"
"//\n"
"// Purpose:	An unsigned 2-bit multiply based upon the fact that LUT's allow\n"
"//		6-bits of input, but a 2x2 bit multiply will never carry more\n"
"//	than one bit.  While this multiply is hardware independent, it is\n"
"//	really motivated by trying to optimize for a specific piece of\n"
"//	hardware (Xilinx-7 series ...) that has 4-input LUT's with carry\n"
"//	chains.\n"
"//\n"
"%s"
"//\n"
"%s",
		name, prjname, creator, cpyleft);

	fprintf(fp, 
"module	bimpy(i_clk, %s, i_ce, i_a, i_b, o_r);\n",
		(async_reset)?"i_areset_n":"i_reset");
	fprintf(fp,
	"\tparameter\tBW=18, LUTB=2;\n"
	"\tinput\t\t\t\ti_clk, %s, i_ce;\n",
		(async_reset)?"i_areset_n":"i_reset");
		
	fprintf(fp,
	"\tinput\t\t[(LUTB-1):0]\ti_a;\n"
	"\tinput\t\t[(BW-1):0]\ti_b;\n"
	"\toutput\treg\t[(BW+LUTB-1):0]\to_r;\n"
"\n"
	"\twire\t[(BW+LUTB-2):0]\tw_r;\n"
	"\twire\t[(BW+LUTB-3):1]\tc;\n"
"\n"
	"\tassign\tw_r =  { ((i_a[1])?i_b:{(BW){1'b0}}), 1'b0 }\n"
		"\t\t\t\t^ { 1'b0, ((i_a[0])?i_b:{(BW){1'b0}}) };\n"
	"\tassign\tc = { ((i_a[1])?i_b[(BW-2):0]:{(BW-1){1'b0}}) }\n"
		"\t\t\t& ((i_a[0])?i_b[(BW-1):1]:{(BW-1){1'b0}});\n"
"\n");

	fprintf(fp, "\tinitial\to_r = 0;\n");
	if (async_reset)
		fprintf(fp,
		"\talways @(posedge i_clk, negedge i_areset_n)\n"
			"\t\tif (!i_areset_n)\n");
	else
		fprintf(fp,
			"\talways @(posedge i_clk)\n"
				"\t\tif (i_reset)\n");

	fprintf(fp,
		"\t\t\to_r <= 0;\n"
		"\t\telse if (i_ce)\n"
			"\t\t\to_r <= w_r + { c, 2'b0 };\n"
"\n"
"endmodule\n");
}

void	buildsubmpy(FILE *fp, char *name, int nmul, bool async_reset) {
	if (nmul == 2) {
		buildbimpy(fp, name, async_reset);
		return;
	}

	fprintf(fp,
"////////////////////////////////////////////////////////////////////////////////\n"
"//\n"
"// Filename: 	%s\n"
"//\n"
"// Project:	%s\n"
"//\n"
"// Purpose:	An %dxN-bit unsigned  multiply built straight from logic and\n"
"//		one addition.  This can be used to turn an NxN bit multiply\n"
"//	into a sum of (N/%d)*N terms.\n"
"%s"
"//\n"
"%s",
		name, prjname, nmul, nmul, creator, cpyleft);

	fprintf(fp, 
"module	%s(i_clk, %s, i_ce, i_a, i_b, o_r);\n", name,
		(async_reset)?"i_areset_n":"i_reset");
	fprintf(fp,
	"\tparameter\tBW=18;\n"
	"\tlocalparam\tLUTB=%d;\n"
	"\tinput\t\t\t\ti_clk, %s, i_ce;\n",
		nmul, (async_reset)?"i_areset_n":"i_reset");
		
	fprintf(fp,
	"\tinput\t\t[(LUTB-1):0]\ti_a;\n"
	"\tinput\t\t[(BW-1):0]\ti_b;\n"
	"\toutput\treg\t[(BW+LUTB-1):0]\to_r;\n"
"\n"
	"\twire\t[(GENM+LUTB-1):0]\tw_r;\n"
"\n");

	fprintf(fp,
	"\tlocalparam\tGENM = (((BW+LUTB-1)/LUTB)*LUTB);\n"
	"\twire\t[GENM-1:0]\tgenm_r, genm_c;\n\n");

	fprintf(fp,
	"\tgenvar k;\n"
	"\tgenerate\n"
	"\tbegin\n"
	"\tfor(k=0; k < BW; k=k+LUTB)\n"
	"\tbegin\n"
	"\t\twire\t[LUTB-1:0]\tb_slice;\n"
	"\t\tif (k+LUTB < BW)\n"
	"\t\t\tassign b_slice = i_b[k +: LUTB];\n"
	"\t\telse\n"
	"\t\t\tassign b_slice = { {(k+LUTB-BW){1\'b0}}, i_b[BW-1:k] };\n"
	"\n"
	"\t\talways @(*)\n"
	"\t\t\tcase({ i_a, b_slice })\n");
	for(int b=0; b<(1<<(2*nmul)); b++) {
		unsigned	msk;

		msk = (1<<nmul)-1;
		fprintf(fp, "\t\t\t%d\'h%0*x: ", 2*nmul, (2*nmul+3)/4, b);
		fprintf(fp, "genm_r[k +: LUTB] = %d\'h%0*x;\n",
			nmul, (nmul+3)/4,
			(((b>>(nmul))&msk) * (b & msk))&msk);
	} fprintf(fp,"\t\t\tendcase\n\n");

	fprintf(fp,
	"\n"
	"\t\talways @(*)\n"
	"\t\t\tcase({ i_a, b_slice })\n");
	for(int b=0; b<(1<<(2*nmul)); b++) {
		unsigned	msk;

		msk = (1<<nmul)-1;
		fprintf(fp, "\t\t\t%d\'h%0*x: ", 2*nmul, (2*nmul+3)/4, b);
		fprintf(fp, "genm_c[k +: LUTB] = %d\'h%0*x;\n",
			nmul, (nmul+3)/4,
			((((b>>(nmul))&msk) * (b & msk))>>(nmul))&msk);
	} fprintf(fp,"\t\t\tendcase\n\n");

	fprintf(fp,"\tend end endgenerate\n\n");

	fprintf(fp,
		"\tassign\tw_r = { %d\'b0, genm_r } + { genm_c, %d\'b0 };\n",
			nmul, nmul);

	fprintf(fp, "\tinitial\to_r = 0;\n");
	if (async_reset)
		fprintf(fp,
		"\talways @(posedge i_clk, negedge i_areset_n)\n"
			"\t\tif (!i_areset_n)\n");
	else
		fprintf(fp,
			"\talways @(posedge i_clk)\n"
				"\t\tif (i_reset)\n");

	fprintf(fp,
		"\t\t\to_r <= 0;\n"
		"\t\telse if (i_ce)\n"
			"\t\t\to_r <= w_r[(BW+LUTB-1):0];\n");

	fprintf(fp,
		"\t// Make Verilator happen\n"
		"\t// verilator lint_off UNUSED\n"
		"\tgenerate if (GENM > BW)\n"
		"\tbegin\n"
		"\t\twire\t[GENM-BW-1:0]	unused;\n"
		"\t\tassign unused = { w_r[GENM+LUTB-1:BW+LUTB] };\n"
		"\tend endgenerate\n"
		"\t// verilator lint_on  UNUSED\n");

	fprintf(fp,
"endmodule\n");
}

void	buildsmpy(FILE *fp, const char *name,
	const int premul, const int na, const int nb,
	const bool aux, const bool async_reset) {
	int	ns, nl;
	ns = na; ns = (na < nb) ? na : nb;
	nl = (na < nb) ? nb : na;

	// int clock, nlits, nrows, nzros, row, maxbits = ns+nl;
	std::string	always_reset = "\talways @(posedge i_clk)\n\tif(i_reset)\n";
	if (async_reset)
		always_reset = "\talways @(posedge i_clk, negedge i_areset_n)\n\tif(!i_areset_n)\n";


	fprintf(fp,
"////////////////////////////////////////////////////////////////////////////////\n"
"//\n"
"// Filename: 	%s.v\n"
"//		\n"
"// Project:	%s\n"
"//\n"
"// Purpose:\tTurns a signed multiply into an unsigned multiply, at the cost\n"
"//\tof two clocks and a negation.\n"
"//\n"
"//\n%s"
"//\n",
		name, prjname, creator);

	fprintf(fp, "%s", cpyleft);

	fprintf(fp, "module %s(i_clk, %s, i_ce, i_a, i_b%s, o_p%s);\n",
		name, (async_reset)?"i_areset_n":"i_reset",
		(aux)?", i_aux":"", (aux)?", o_aux":"");
	fprintf(fp,
		"\tparameter\tNA=%d, NB=%d, DLY=%d;\n"
		"\tinput\t\t\t\t\ti_clk, %s, i_ce;\n"
		"\tinput\t\tsigned\t[(NA-1):0]\ti_a;\n"
		"\tinput\t\tsigned\t[(NB-1):0]\ti_b;\n", na, nb,
		stages(premul, na, nb)+1,
		(async_reset)?"i_areset_n":"i_reset");
	if (aux) fprintf(fp, "\tinput\t\t\t\t\ti_aux;\n");
	fprintf(fp, "\toutput\treg\tsigned\t[(NA+NB-1):0]\to_p;\n");
	if (aux) fprintf(fp, "\toutput\treg\t\t\t\to_aux;\n");

	fprintf(fp, "\n");
	fprintf(fp, "\tlocalparam NS = (NA < NB) ? NA : NB;\n");
	fprintf(fp, "\tlocalparam NL = (NA < NB) ? NB : NA;\n");
	fprintf(fp, "\twire\t[(NS-1):0]\ti_s;\t// Smaller input\n");
	fprintf(fp, "\twire\t[(NL-1):0]\ti_l;\t// larger input\n");

	fprintf(fp, "\n"
"\t//\n"
"\t// Adjust our inputs so that i_s has the fewest bits, and i_b the most\n"
"\tgenerate if (NA < NB)\n"
"\tbegin : BITADJ\n"
"\t\tassign\ti_s = i_a;\n"
"\t\tassign\ti_l = i_b;\n"
"\tend else begin\n"
"\t\tassign\ti_s = i_b;\n"
"\t\tassign\ti_l = i_a;\n"
"\tend endgenerate\n\n");


	fprintf(fp, "\treg\t\t[(NS-1):0]\tu_s;\n");
	fprintf(fp, "\treg\t\t[(NL-1):0]\tu_l;\n");
	fprintf(fp, "\treg\t\t[(DLY-1):0]\tu_sgn;\n");
	if (aux)
		fprintf(fp, "\treg\t\t\t\tu_aux;\n\n");
	else
		fprintf(fp, "\n");

	if (aux) {
		fprintf(fp, "\tinitial\tu_aux = 1\'b0;\n");
		fprintf(fp, "%s", always_reset.c_str());

		fprintf(fp,
		"\t\t\tu_aux <= 1'b0;\n"
		"\t\telse if (i_ce)\n"
		"\t\t\tu_aux <= i_aux;\n\n");
	}

	fprintf(fp, "\tinitial\tu_s = 0;\n");
	fprintf(fp, "\tinitial\tu_l = 0;\n");
	fprintf(fp, "%s", always_reset.c_str());
	fprintf(fp,
	"\tbegin\n"
		"\t\tu_s <= 0;\n"
		"\t\tu_l <= 0;\n"
	"\tend else if (i_ce)\n"
	"\tbegin\n"
		"\t\tu_s <= (i_s[NS-1])?(-i_s):i_s;\n"
		"\t\tu_l <= (i_l[NL-1])?(-i_l):i_l;\n"
	"\tend\n"
"\n");
	fprintf(fp, "\tinitial\tu_sgn = 0;\n");
	fprintf(fp, "%s"
		"\t\tu_sgn <= 0;\n"
	"\telse if (i_ce)\n"
		"\t\tu_sgn <= { u_sgn[(DLY-2):0], ((i_s[NS-1])^(i_l[NL-1])) };\n"
"\n",
		always_reset.c_str());

	fprintf(fp,
"\twire\t[(NA+NB-1):0]\tu_r;\n%s"
"\tumpy_%dx%d\tumpy(i_clk, %s, i_ce, u_s, u_l,%s u_r%s);\n",
		(aux)?"\twire\t\t\tw_aux;\n":"", 
		ns, nl, (async_reset)?" i_areset_n":"i_reset",
		(aux)?" u_aux,":"", (aux)?", w_aux":"");

	fprintf(fp,
"\n"
"\tinitial\to_p = 0;\n"
"%s"
	"\t\to_p <= 0;\n"
	"\telse if (i_ce)\n"
		"\t\to_p <= (u_sgn[DLY-1])?(-u_r):u_r;\n"
"\n", always_reset.c_str());

	if (aux) fprintf(fp, "\n\tinitial\to_aux = 1\'b0;\n%s"
			"\t\to_aux <= 1'b0;\n"
		"\telse if (i_ce)\n"
			"\t\to_aux <= w_aux;\n\n", always_reset.c_str());

	fprintf(fp, "\nendmodule\n");
}

void	buildumpy(FILE *fp, char *name, int premul, const int na, const int nb, bool aux, bool async_reset) {
	int	row, clock, nbits, nrows, nzros, maxbits = na+nb,
		unused = 0, sz, lastsz;
	char	ustr[1024];
	ustr[0] = '\0';
	int	ns, nl;
	ns = (na < nb) ? na : nb;
	nl = (na < nb) ? nb : na;
	std::string	always_reset = "\talways @(posedge i_clk)\n\tif(i_reset)\n";
	if (async_reset)
		always_reset = "\talways @(posedge i_clk, negedge i_areset_n)\n\tif(!i_areset_n)\n";

	fprintf(fp,
"////////////////////////////////////////////////////////////////////////////////\n"
"//\n"
"// Filename: 	%s.v\n"
"//		\n"
"// Project:	%s\n"
"//\n"
"// Purpose:\tThis verilog file multiplies two unsigned numbers together,\n"
"//		without using any hardware acceleration.  This file is\n"
"//\tcomputer generated, so please (for your sake) don\'t make any edits\n"
"//\tto this file lest you regenerate it and your edits be lost.\n"
"//\n"
"//\n%s"
"//\n", name, prjname, creator);

	fprintf(fp, "%s", cpyleft);

	fprintf(fp, "module %s(i_clk, %s, i_ce, i_a, i_b%s, o_p%s);\n",
		name, (async_reset)?"i_areset_n":"i_reset",
		(aux)?", i_aux":"", (aux)?", o_aux":"");
	fprintf(fp,
		"\tparameter\tNA=%d, NB=%d;\n"
		"\tinput\t\t\t\t\ti_clk, %s, i_ce;\n"
		"\tinput\t\tsigned\t[(NA-1):0]\ti_a;\n"
		"\tinput\t\tsigned\t[(NB-1):0]\ti_b;\n", na, nb,
		(async_reset)?"i_areset_n":"i_reset");

	if (aux) fprintf(fp, "\tinput\t\t\t\t\ti_aux;\n");
	fprintf(fp, "\toutput\treg\tsigned\t[(NA+NB-1):0]\to_p;\n");
	if (aux) fprintf(fp, "\toutput\treg\t\t\t\to_aux;\n");

	fprintf(fp, "\n");
	fprintf(fp, "\tlocalparam NS = (NA < NB) ? NA : NB;\n");
	fprintf(fp, "\tlocalparam NL = (NA < NB) ? NB : NA;\n");
	fprintf(fp, "\twire\t[(NS-1):0]\ti_s;\t// Smaller input\n");
	fprintf(fp, "\twire\t[(NL-1):0]\ti_l;\t// larger input\n");

	fprintf(fp, "\n"
"\t//\n"
"\t// Adjust our inputs so that i_s has the fewest bits, and i_b the most\n"
"\tgenerate if (NA < NB)\n"
"\tbegin : BITADJ\n"
"\t\tassign\ti_s = i_a;\n"
"\t\tassign\ti_l = i_b;\n"
"\tend else begin\n"
"\t\tassign\ti_s = i_b;\n"
"\t\tassign\ti_l = i_a;\n"
"\tend endgenerate\n\n");

	// Build the first tableau row
	// There are Na elements, each of Nb length
	clock = 0;
	fprintf(fp, "\t// Clock zero: build our Tableau only.\n"
		"\t// There will be one row for every pair of bits in i_a, and each\n"
		"\t// row will contain (AW+3) bits, to allow\n"
		"\t// for signed arithmetic manipulation.\n\t//\n");
	if (aux) fprintf(fp, "\treg\tA_%d;\n", clock);

	for(row=0; row<ns/premul; row++) {
		fprintf(fp, "\n"
"\twire\t[%d:0]\tS_0_%02d;\n", nl+premul-1, row);
		if (premul == 2)
			fprintf(fp, "\tbimpy ");
		else
			fprintf(fp, "\tpremul%d ", premul);
		fprintf(fp,
"#(NB) initialmpy_%d_0(i_clk, %s, i_ce, i_s[%d:%d], i_l, S_0_%02d);\n",
			row,
			(async_reset)?"i_areset_n":"i_reset",
			(row*premul+premul-1), (row*premul), row);
	}
	if (ns%premul) {// Do one extra row, to capture the last bit of a
		fprintf(fp, "\t//Extra (odd) row\n");
		fprintf(fp,
"\twire\t[%d:0]\tS_0_%02d;\n", nl+premul-1, row);
		if (premul == 2)
			fprintf(fp, "\tbimpy ");
		else
			fprintf(fp, "\tpremul%d ", premul);
		fprintf(fp,
	"#(NB) initialmpy_%d_0(i_clk, %s, i_ce, { {(%d){1\'b0}}, i_s[%d:%d]}, i_l, S_0_%02d);\n",
			row,
			(async_reset)?"i_areset_n":"i_reset",
			(row*premul+premul)-ns, ns-1,
			(row*premul), row);
	} sz = (nl+premul); lastsz = sz;

	if (aux)
		fprintf(fp, "\n\tinitial\tA_%d = 0;\n%s"
		"\t\tA_%d <= 1'b0;\n"
		"\telse if (i_ce)\n"
		"\t\tA_%d <= i_aux;\n", clock,
		always_reset.c_str(), clock, clock);
	nrows = (ns/premul)+((ns%premul)?1:0); nbits = nl+premul, nzros=premul;

	// assert(nrows == npremul(premul, ns, nl));

	while(nrows > 1) {
		lastsz = sz;
		fprintf(fp, "\n\t//\n\t// Round #%d, clock = %d, nz = %d, nbits = %d, nrows_in = %d\n\t//\n",
			clock+1, clock+1, nzros, nbits, nrows);
		clock++; fprintf(fp, "\n");
		for(row=0; row<(nrows+1)/2; row++) {
			fprintf(fp, "\treg\t[(%d-1):0]\tS_%d_%02d; // maxbits = %d\n",
				((nbits+1+nzros)>maxbits)?maxbits
					:(nbits+1+nzros), clock, row, maxbits);
			sz = ((nbits+1+nzros)>maxbits)?maxbits :(nbits+1+nzros);
		}
		if (aux) fprintf(fp, "\treg\tA_%d;\n\n", clock);

		for(row=0; row<nrows/2; row++)
			fprintf(fp, 
			  "\tinitial\tS_%d_%02d = 0;\n", clock, row);
		if (nrows&1)
			fprintf(fp, 
			  "\tinitial\tS_%d_%02d = 0;\n", clock, row);
		fprintf(fp,
	"%s\tbegin\n", always_reset.c_str());
		for(row=0; row<nrows/2; row++) {
			fprintf(fp, "\t\tS_%d_%02d <= 0;\n", clock, row);
		} if (nrows&1) {
			fprintf(fp, "\t\tS_%d_%02d <= 0;\n",
				clock, row);
		}

		fprintf(fp,
	"\tend else if (i_ce)\n"
	"\tbegin\n");
		for(row=0; row<nrows/2; row++) {
			fprintf(fp, "\t\tS_%d_%02d <= { ", clock, row);
			if (maxbits-nbits>0) {
				if (nzros+nbits+1 > maxbits)
					fprintf(fp, "%d\'b0, ", maxbits-nbits);
				else
					fprintf(fp, "%d\'b0, ", nzros);
				fprintf(fp, "S_%d_%02d } + { S_%d_%02d",
					clock-1, 2*row, clock-1, 2*row+1);
			} else {
				fprintf(fp, "S_%d_%02d[%d:0] } + { S_%d_%02d",
					clock-1, 2*row,
					maxbits-1,
					clock-1, 2*row+1);
			}
			if (lastsz > (maxbits-nzros)) {
				fprintf(fp, "// Adding to unused: %d, %d\n", nzros+nbits+1, maxbits);
				unused += lastsz - (maxbits-nzros);
				fprintf(fp, "[%d:0]", maxbits-1-nzros);
				if (ustr[0])
					strcat(ustr, ", ");
				sprintf(ustr, "%sS_%d_%02d[%d:%d]",
					ustr, clock-1, 2*row+1,
					lastsz-1, maxbits-nzros);
			}
			fprintf(fp, ", %d\'b0 };\n", nzros);
			if (unused)
				fprintf(fp, "\n// unused = %d, ustr = %s\n", unused, ustr);
		}
		if (nrows&1) {
			fprintf(fp, "\t\tS_%d_%02d <= { ",
				clock, row);
			if (nzros+nbits+1 > maxbits)
				fprintf(fp, "%d\'b0, ", maxbits-nbits);
			else
				fprintf(fp, "%d\'b0, ", nzros+1);
			fprintf(fp, "S_%d_%02d };\n", clock-1, nrows-1);
		}
		fprintf(fp, "\tend\n\n");
		if (aux)
		fprintf(fp, "\tinitial\tA_%d = 0;\n%s"
		"\t\tA_%d <= 1'b0;\n"
		"\telse if (i_ce)\n"
		"\t\tA_%d <= A_%d;\n", clock,
			always_reset.c_str(),
			clock, clock, clock-1);

		nrows = (nrows+1)/2;
		nbits+=1+nzros; nzros<<= 1;
	}

	// The full multiply is complete, just clock our outputs
	// to values we've already calculated.
	fprintf(fp, "\n\tassign\to_p = S_%d_00[(NA+NB-1):0];\n", clock);
	if (aux) fprintf(fp, "\tassign\to_aux = A_%d;\n", clock);

	// assert(clock + 1 == stages(premul, ns, nl));

	fprintf(fp, "\n"
	"\t// Make verilator happy\n"
	"\t// verilator lint_off UNUSED\n"
	"\twire\t[%d-1:0]\tunused;\n"
	"\tassign	unused = { %s };\n"
	"\t// verilator lint_on  UNUSED\n\n", unused, ustr);

	fprintf(fp, "\n\n`ifdef\tFORMAL\n\n");

	fprintf(fp, "\treg\tf_past_valid;\n"
	"\tinitial\tf_past_valid = 0;\n"
	"\talways @(posedge i_clk)\n"
	"\t\tf_past_valid <= 1\'b1;\n\n");

	if (aux) {
		fprintf(fp,
		"\twire	[%d+1:0]\tf_auxpipe;\n"
		"\tassign\tf_auxpipe\t= { ", clock);
		for(int k=clock; k>=0; k--)
			fprintf(fp, "A_%d,", k);
		fprintf(fp, " i_aux };\n\n");

		fprintf(fp, "\tinitial\tassume(!i_aux);\n");
		fprintf(fp, "\talways @(posedge i_clk)\n"
			"\tif ((%s)||((f_past_valid)&&($past(%s))))\n"
			"\t\tassume(!i_aux);\n\n",
			(async_reset)? "!i_areset_n":"i_reset",
			(async_reset)? "!i_areset_n":"i_reset");

		fprintf(fp, "\tinitial\tassert(f_auxpipe == 0);\n");
		fprintf(fp, "\talways @(posedge i_clk)\n");
		fprintf(fp, "\tif ((f_past_valid)&&($past(%s)))\n",
			(async_reset)?"!i_areset_n":"i_reset");
		fprintf(fp, "\t\tassert(f_auxpipe == 0);\n");
		fprintf(fp, "\talways @(posedge i_clk)\n");
		fprintf(fp, "\tif ((f_past_valid)&&(!$past(%s))&&($past(i_ce)))\n",
			(async_reset)?"!i_areset_n":"i_reset");
		fprintf(fp, "\t\tassert(f_auxpipe[%d+1:1] == $past(f_auxpipe[%d:0]));\n\n", clock,clock);
		fprintf(fp, "\talways @(posedge i_clk)\n");
		fprintf(fp, "\tif ((f_past_valid)&&(!$past(%s))&&(!$past(i_ce)))\n",
			(async_reset)?"!i_areset_n":"i_reset");
		fprintf(fp, "\t\tassert(f_auxpipe[%d+1:1] == $past(f_auxpipe[%d+1:1]));\n\n", clock,clock);
	}



	fprintf(fp,
	"\tlocalparam\tF_DELAY = %d;\n", clock);
	fprintf(fp,
	"\treg	[NA+NB-1:0]	f_result;\n"
	"\tinteger\t\t\tik;\n"
	"\n\tinitial\tf_result = 0;\n"
	"%s"
		"\t\tf_result = 0;\n"
		"\telse if (i_ce)\n"
		"\tbegin\n"
			"\t\tf_result = 0;\n"
			"\t\tfor(ik=0; ik<NS; ik=ik+1)\n"
			"\t\t\tif(i_a[ik])\n"
			"\t\t\t\tf_result = f_result + { {(NL-ik-1){1\'b0}},\n"
			"\t\t\t\t\t\ti_b, { (ik){1\'b0} } };\n"
		"\tend\n"
	"\n", always_reset.c_str());

	fprintf(fp,
	"\treg\t[F_DELAY*(NA+NB)-1:0]	f_result_pipe;\n"
	"\n\tinitial\tf_result_pipe = 0;\n%s"
		"\t\tf_result_pipe <= 0;\n"
		"\telse if (i_ce)\n"
		"\t\tf_result_pipe <= { f_result, f_result_pipe[((F_DELAY)*(NA+NB)-1):(NA+NB)] };\n"
	"\n"
		"\talways @(posedge i_clk)\n"
		"\t\tassert(o_p == f_result_pipe[(NA+NB-1):0]);\n\n",
		always_reset.c_str());

	fprintf(fp,
	 	"\talways @(posedge i_clk)\n"
	 	"\t\tassume((i_ce)\n"
		"\t\t\t||((f_past_valid)&&($past(i_ce)))\n"
		"\t\t\t// ||(($past(f_past_valid))&&($past(i_ce,2)))\n"
		"\t\t\t);\n\n");
	fprintf(fp, "`endif\n");
	fprintf(fp, "\nendmodule\n");
}

void buildmakinc(FILE *fp, const char *fname, const int Na, const int Nb) {
	fprintf(fp, ".PHONY: umpy_%dx%d\n", Na, Nb);
	fprintf(fp, "umpy_%dx%d: $(VDIRFB)/Vumpy_%dx%d__ALL.a\n", Na, Nb, Na, Nb);
	fprintf(fp, "$(VDIRFB)/Vumpy_%dx%d.h: umpy_%dx%d.v\n", Na, Nb, Na, Nb);
	fprintf(fp, "$(VDIRFB)/Vumpy_%dx%d__ALL.a: $(VDIRFB)/Vumpy_%dx%d.h\n"
		"\t$(SUBMAKE) -f Vumpy_%dx%d.mk\n", Na, Nb, Na, Nb, Na, Nb);
	//
	fprintf(fp, "\n");
	fprintf(fp, ".PHONY: sgnmpy_%dx%d\n", Na, Nb);
	fprintf(fp, "sgnmpy_%dx%d: $(VDIRFB)/Vsgnmpy_%dx%d__ALL.a\n", Na, Nb, Na, Nb);
	fprintf(fp, "$(VDIRFB)/Vsgnmpy_%dx%d.h: sgnmpy_%dx%d.v\n",
		Na, Nb, Na, Nb);
	fprintf(fp, "$(VDIRFB)/Vsgnmpy_%dx%d__ALL.a: $(VDIRFB)/Vsgnmpy_%dx%d.h\n"
		"\t$(SUBMAKE) -f Vsgnmpy_%dx%d.mk\n", Na, Nb, Na, Nb, Na, Nb);
}

void buildbenchmk(FILE *fp, const char *fname, const int Na, const int Nb,
		bool async_reset) {
	fprintf(fp, "test: test%dx%d\n\n", Na, Nb);
	fprintf(fp, ".PHONY: test%dx%d\n", Na, Nb);
	fprintf(fp, "MPYS += mpy_tb_%dx%d\n", Na, Nb);
	fprintf(fp,
"$(OBJDIR)/mpy_tb_%dx%d.o: mpy_tb.cpp components.h\n"
"$(OBJDIR)/mpy_tb_%dx%d.o: $(RTLOBJD)/Vsgnmpy_%dx%d.h\n"
"$(OBJDIR)/mpy_tb_%dx%d.o: $(RTLOBJD)/Vumpy_%dx%d.h\n"
"\t$(CXX) -DMPYSZ=%dx%d -DUMPY=Vumpy_%dx%d -DSMPY=Vsgnmpy_%dx%d -DNA=%d -DNB=%d %s $(CFLAGS) $(INCS) -c mpy_tb.cpp -o $@\n",
	Na, Nb, Na, Nb, Na, Nb, Na, Nb, Na, Nb, Na, Nb, Na, Nb,
	Na, Nb, Na, Nb, (async_reset)?"-DASYNC":"");

	fprintf(fp, 
"mpy_tb_%dx%d: $(RTLOBJD)/Vsgnmpy_%dx%d__ALL.a\n"
"mpy_tb_%dx%d: $(RTLOBJD)/Vumpy_%dx%d__ALL.a\n"
"mpy_tb_%dx%d: $(OBJDIR)/mpy_tb_%dx%d.o $(VLOBJS)\n"
"\t$(CXX) $(CFLAGS) $(INCS) $(OBJDIR)/mpy_tb_%dx%d.o $(RTLOBJD)/Vsgnmpy_%dx%d__ALL.a $(RTLOBJD)/Vumpy_%dx%d__ALL.a $(VLOBJS) -o $@\n",
		Na, Nb,
		Na, Nb,
		Na, Nb,
		Na, Nb,
		Na, Nb,
		Na, Nb,
		Na, Nb,
		Na, Nb,
		Na, Nb);
	fprintf(fp,
"test%dx%d: mpy_tb_%dx%d\n"
"\tmpy_tb_%dx%d\n",
		Na, Nb, Na, Nb, Na, Nb);
}

bool	direxists(const char *) {
	return true;
}

void	buildmpy(const char *dir, int premul, int Na, int Nb, bool use_aux, bool async_reset) {
	FILE	*fp;
	char	fname[256];

	if (verbose_flag) {
		printf("Building a %dx%d multiply\n", Na, Nb);
	} else {
		printf("NO VERBOSE FLAG!\n");
		exit(EXIT_FAILURE);
	}

	if (dir)
		sprintf(fname, "%s/sgnmpy_%dx%d.v", dir, Na, Nb);
	else
		sprintf(fname, "sgnmpy_%dx%d.v", Na, Nb);
	fp = fopen(fname, "w");
	if (!fp) {
		fprintf(stderr, "Could not open %s for writing\n", fname);
		perror("O/S Err:");
		exit(EXIT_FAILURE);
	} else if (verbose_flag) {
		fprintf(stderr, "Writing %s\n", fname);
	} sprintf(fname, "sgnmpy_%dx%d", Na, Nb);
	buildsmpy(fp, fname, premul, Na, Nb, use_aux, async_reset);
	fclose(fp);

	if (dir)
		sprintf(fname, "%s/umpy_%dx%d.v", dir, Na, Nb);
	else
		sprintf(fname, "umpy_%dx%d.v", Na, Nb);
	fp = fopen(fname, "w");
	if (!fp) {
		fprintf(stderr, "Could not open %s for writing\n", fname);
		perror("O/S Err:");
		exit(EXIT_FAILURE);
	} else if (verbose_flag) {
		fprintf(stderr, "Writing %s\n", fname);
	} sprintf(fname, "umpy_%dx%d", Na, Nb);
	buildumpy(fp, fname, premul, Na, Nb, use_aux, async_reset);
	fclose(fp);

	if (premul == 2) {
		if (dir)
			sprintf(fname, "%s/bimpy.v", dir);
		else
			sprintf(fname, "bimpy.v");
	} else if (dir)
		sprintf(fname, "%s/premul%d.v", dir, premul);
	else
		sprintf(fname, "premul%d.v", premul);
	fp = fopen(fname, "w");
	if (!fp) {
		fprintf(stderr, "Could not open %s for writing\n", fname);
		perror("O/S Err:");
		exit(EXIT_FAILURE);
	} else if (verbose_flag) {
		fprintf(stderr, "Writing %s\n", fname);
	}

	if (premul == 2)
		sprintf(fname, "bimpy");
	else
		sprintf(fname, "premul%d", premul);
	buildsubmpy(fp, fname, premul, async_reset);
	fclose(fp);

	if (dir)
		sprintf(fname, "%s/mkinc%dx%d.mk", dir, Na, Nb);
	else
		sprintf(fname, "mkinc%dx%d.mk", Na, Nb);
	fp = fopen(fname, "w");
	if (!fp) {
		fprintf(stderr, "Could not open %s for writing\n", fname);
		perror("O/S Err:");
		exit(EXIT_FAILURE);
	} else if (verbose_flag) {
		fprintf(stderr, "Writing %s\n", fname);
	}
	buildmakinc(fp, fname, Na, Nb);
	fclose(fp);

	if (direxists("../bench/cpp"))
		sprintf(fname, "../bench/cpp/mkbnch%dx%d.mk", Na, Nb);
	else
		sprintf(fname, "mkbnch%dx%d.mk", Na, Nb);
	fp = fopen(fname, "w");
	if (!fp) {
		fprintf(stderr, "Could not open %s for writing\n", fname);
		perror("O/S Err:");
		exit(EXIT_FAILURE);
	} else if (verbose_flag) {
		fprintf(stderr, "Writing %s\n", fname);
	}
	buildbenchmk(fp, fname, Na, Nb, async_reset);
	fclose(fp);
}

void	usage(void) {
	printf("USAGE: bldmpy <#-of-bits-in-A> <#-of-bits-in-B>\n");
}

int main(int argc, char **argv) {
	bool	use_aux = true;
	bool	async_reset = false;
	int	premul = 2;
	const char	core_dir[] = "../rtl";

	if (argc != 3) {
		usage();
		exit(EXIT_FAILURE);
	}
	if (premul != 2) {
		fprintf(stderr, "WARNING: The bimpy pre-multiply is the only one that has been proven\n");
	}
	buildmpy(core_dir, premul, atoi(argv[1]), atoi(argv[2]), use_aux, async_reset);

	return(0);
}
