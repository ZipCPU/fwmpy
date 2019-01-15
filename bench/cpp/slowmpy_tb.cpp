////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	slowmpy_tb.cpp
//
// Project:	A multiply core generator
//
// Purpose:	A test-bench for the twos complement multiply generated
//		by the bldmpy multiply generator.
//
//	This file depends upon verilator to both compile, run, and therefore
//	test slowmpy.v--a non-coregen IP.
//
// Creator:	Dan Gisselquist, Ph.D.
//		Gisselquist Technology, LLC
//
////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2015,2018-2019, Gisselquist Technology, LLC
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
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "verilated.h"
#include "verilated_vcd_c.h"

#include "Vslowmpy.h"

const	bool	trace = false;
const	int	NA=12, NB = NA;
const	bool	OPT_SIGNED = true;

long	sbits(const long val, const int bits) {
	long	r;

	r = val & ((1l<<bits)-1);
	if (r & (1l << (bits-1)))
		r |= (-1l << bits);
	return r;
}

unsigned long	ubits(const long val, const int bits) {
	unsigned long r = val & ((1l<<bits)-1);
	return r;
}


class	SLOWMPYTB {
public:
	Vslowmpy	*m_slow;
	long		svals[32];
	int		m_addr;
	VerilatedVcdC	*m_strace;
	long		m_tickcount;

	SLOWMPYTB(void) {
		m_slow = new Vslowmpy;

		Verilated::traceEverOn(true);

		for(int i=0; i<32; i++)
			svals[i] = 0;
		m_addr = 0;

		m_strace = NULL;
		m_tickcount = 0;
	}
	~SLOWMPYTB(void) {
		if (m_strace)
			m_strace->close();
		delete m_slow;
	}

	void	opentrace(const char *pattern) {
		char	*fname;

		fname = (char *)malloc(strlen(pattern) + 20);

		if (!m_strace) {
			sprintf(fname, pattern, "slowmpy", NA, NB);
			m_strace = new VerilatedVcdC;
			m_slow->trace(m_strace, 99);
			m_strace->open(fname);
		}

		free(fname);
	}
		

	void	tick(void) {
		m_tickcount++;

		m_slow->i_clk = 0;
		m_slow->eval();
		if (m_strace) m_strace->dump((uint64_t)(10*m_tickcount-2));

		m_slow->i_clk = 1;
		m_slow->eval();
		if (m_strace) m_strace->dump((uint64_t)(10*m_tickcount));


		m_slow->i_clk = 0;
		m_slow->eval();
		if (m_strace) m_strace->dump((uint64_t)(10*m_tickcount+5));


		if (m_strace)
			m_strace->flush();
	}

	void	reset(void) {
		m_slow->i_clk = 0;
		m_slow->i_stb = 0;
		m_slow->i_a = rand();
		m_slow->i_b = rand();
		m_slow->i_aux = rand();

		m_slow->i_reset = 1;
		tick();
		m_slow->i_aux = 0;
		m_slow->i_reset = 0;

		m_addr = 0;
	}

	bool	test(const int ia, const int ib) {
		bool		success = true;
		int		aux;
		long		sout;

		m_slow->i_stb = 1;
		m_slow->i_a = ubits(ia, NA);
		m_slow->i_b = ubits(ib, NB);
		m_slow->i_aux = rand();

		assert(NA+NB < 8*sizeof(long));

		assert(!m_slow->o_busy);

		for(int i=0; i<NA+1; i++) {
			tick();

			m_slow->i_stb = 0;
			m_slow->i_a = ubits(rand(), NA);
			m_slow->i_b = ubits(rand(), NB);
			assert( m_slow->o_busy);
			assert(!m_slow->o_done);
		} tick();

		assert(!m_slow->o_busy);
		assert( m_slow->o_done);

		if (trace) {
		printf("k=%3d: A =%06x, B =%06x, AUX=%d -> S(O) = %9lx, SAUX=%d\n",
			m_addr, (int)ubits(ia, NA), (int)ubits(ib,NB), aux,
			(unsigned long)m_slow->o_p, m_slow->o_aux);
		}

		if (OPT_SIGNED)
			sout = sbits(m_slow->o_p, NA+NB);
		else
			sout = ubits(m_slow->o_p, NA+NB);

		if (success) {
			long	sval;
		       
			if (OPT_SIGNED)
				sval = sbits(ia, NA) * sbits(ib, NB);
			else
				sval = ubits(ia, NA) * ubits(ib, NB);
			success = success && (sout== sval);
			if (!success) {
				printf("WRONG SGN-ANSWER: %8lx (expected) != %8lx (actual)\n", sval, sout);
				exit(EXIT_FAILURE);
			}
		}

		return success;
	}
};

int	main(int argc, char **argv, char **envp) {
	Verilated::commandArgs(argc, argv);
	SLOWMPYTB		*tb = new SLOWMPYTB;

	if (trace)
		tb->opentrace("slowtrace.vcd");
	tb->reset();

	tb->test(0, 0);
	tb->test(1, 0);
	tb->test(2, 0);
	tb->test(4, 0);
	tb->test(0, 1);
	tb->test(0, 2);
	tb->test(0, 4);
	tb->test(1, 1);
	tb->test(2, 1);
	tb->test(4, 1);
	tb->test(1, 2);
	tb->test(1, 4);
	tb->test(2, 4);
	tb->test(4, 4);
	tb->test(-1,  1);
	tb->test(-1,  2);
	tb->test(-1,  4);
	tb->test(-2,  4);
	tb->test(-4,  4);
	tb->test(-1, -1);
	tb->test(-1, -2);
	tb->test(-1, -4);
	tb->test(-2, -4);
	tb->test(-4, -4);
	tb->test( 1, -1);
	tb->test( 1, -2);
	tb->test( 1, -4);
	tb->test( 2, -4);
	tb->test( 4, -4);
	tb->test((1<<(NA-1)), 0);
	tb->test((1<<(NA-2)), (1<<(NB-1)));
	tb->test((1<<(NA-1)), (1<<(NB-2)));
	tb->test((1<<(NA-1)), (1<<(NB-1)));
	tb->test(0, (1<<(NB-1)));

	tb->test(0, 0);
	tb->test((1<<(NA-1))-1, 0);
	tb->test((1<<(NA-1))-1, (1<<(NB-1))-1);
	tb->test(0, (1<<(NB-1))-1);

	tb->test((1<<(NA-1))  , (1<<(NB-1)));
	tb->test((1<<(NA-1))-1, (1<<(NB-1)));
	tb->test((1<<(NA-1))-1, (1<<(NB-1))-1);
	tb->test((1<<(NA-1))  , (1<<(NB-1))-1);

	for(int k=0; k<(NA-1); k++) {
		int	a, b;

		a = (1<<k);
		b = 1;
		tb->test(a, b);
	}

	for(int k=0; k<(NB-1); k++) {
		int	a, b;

		a = (1<<15);
		b = (1<<k);
		tb->test(a, b);
	}

	for(int k=0; k<1024; k++)
		tb->test(rand(), rand());

	for(int k=0; k<(1<<NA); k++) {
		for(int j=0; j<(1<<NB); j++) {
			tb->test(k, j);
		}
	}

	delete	tb;

	printf("SUCCESS!\n");
	exit(0);
}

