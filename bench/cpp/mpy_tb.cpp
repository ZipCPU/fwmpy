////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	mpy_tb.cpp
//
// Project:	A multiply core generator
//
// Purpose:	A test-bench for the twos complement multiply generated
//		by the bldmpy multiply generator.
//
//	This file depends upon verilator to both compile, run, and therefore
//	test sgnmpy_16x20.v
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

#include "components.h"
typedef	SMPY	Vsgn;
typedef	UMPY	Vumpy;
bool	trace = false;

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


class	MPYTB {
public:
	Vsgn	*m_score;
	Vumpy	*m_ucore;
	long	svals[32];
	unsigned long uvals[32];
	int	m_addr, m_uoff, m_soff;
	bool	m_usync, m_ssync;
	VerilatedVcdC	*m_utrace, *m_strace;
	long	m_tickcount;

	MPYTB(void) {
		m_score = new Vsgn;
		m_ucore = new Vumpy;

		Verilated::traceEverOn(true);

		for(int i=0; i<32; i++)
			svals[i] = uvals[i] = 0;
		m_addr = 0; m_uoff = 0; m_soff = 0;
		m_usync = false;
		m_ssync = false;

		m_utrace = m_strace = NULL;
		m_tickcount = 0;
	}
	~MPYTB(void) {
		if (m_strace)
			m_strace->close();
		if (m_utrace)
			m_utrace->close();
		delete m_ucore;
		delete m_score;
	}

	void	opentrace(const char *pattern) {
		char	*fname;

		fname = (char *)malloc(strlen(pattern + 20));

		if (!m_strace) {
			sprintf(fname, pattern, "umpy", NA, NB);
			m_strace = new VerilatedVcdC;
			m_score->trace(m_strace, 99);
			m_strace->open(fname);
		}

		if (!m_utrace) {
			sprintf(fname, pattern, "sgnmpy", NA, NB);
			m_strace = new VerilatedVcdC;
			m_score->trace(m_strace, 99);
			m_strace->open(fname);
		}

		free(fname);
	}
		

	void	tick(void) {
		m_tickcount++;

		m_score->i_clk = 0;
		m_ucore->i_clk = 0;
		m_score->eval();
		m_ucore->eval();
		if (m_strace) m_strace->dump((uint64_t)(10*m_tickcount-2));
		if (m_utrace) m_utrace->dump((uint64_t)(10*m_tickcount-2));

		m_score->i_clk = 1;
		m_ucore->i_clk = 1;
		m_score->eval();
		m_ucore->eval();
		if (m_strace) m_strace->dump((uint64_t)(10*m_tickcount));
		if (m_utrace) m_utrace->dump((uint64_t)(10*m_tickcount));


		m_score->i_clk = 0;
		m_ucore->i_clk = 0;
		m_score->eval();
		m_ucore->eval();
		if (m_strace) m_strace->dump((uint64_t)(10*m_tickcount+5));
		if (m_utrace) m_utrace->dump((uint64_t)(10*m_tickcount+5));


		if (m_strace) {
			m_strace->flush();
		} if (m_utrace) {
			m_utrace->flush();
			printf("Flushed\n");
		}
	}

	void	reset(void) {
		m_score->i_clk = 0;
		m_ucore->i_clk = 0;
		m_score->i_ce = 1;
		m_ucore->i_ce = 1;
		m_score->i_a = rand();
		m_ucore->i_a = m_score->i_a;
		m_score->i_b = rand();
		m_ucore->i_b = m_score->i_b;
		m_score->i_aux = rand();
		m_ucore->i_aux = m_score->i_aux;

		for(int k=0; k<30; k++) {
			tick();
			m_score->i_aux = rand();
			m_ucore->i_aux = m_score->i_aux;
		}

#ifdef	ASYNC_RESET
		m_score->i_areset_n = 0;
		m_ucore->i_areset_n = 0;
#else
		m_score->i_reset = 1;
		m_ucore->i_reset = 1;
#endif
		tick();
		m_score->i_aux = 0;
		m_ucore->i_aux = 0;
#ifdef	ASYNC_RESET
		m_score->i_areset_n = 1;
		m_ucore->i_areset_n = 1;
#else
		m_score->i_reset = 0;
		m_ucore->i_reset = 0;
#endif
		m_ssync = false;
		m_usync = false;
		m_soff = 0;
		m_uoff = 0;

		m_addr = 0;
	}

	void	sync(void) {
		m_score->i_aux = 1;
		m_ucore->i_aux = 1;
	}

	bool	test(const int ia, const int ib) {
		bool		success;
		int		aux;
		long		sout;
		unsigned long	uout;

		m_score->i_ce = 1;
		m_ucore->i_ce = 1;
		m_score->i_a = sbits(ia, NA);
		m_score->i_b = sbits(ib, NB);
		m_ucore->i_a = ubits(ia, NA);
		m_ucore->i_b = ubits(ib, NB);
		aux = m_ucore->i_aux;

		assert(NA+NB < 8*sizeof(long));

		uvals[m_addr&31] = (unsigned long)ubits(ia, NA)
					* (unsigned long)ubits(ib, NB);
		svals[m_addr&31] = (long)sbits(ia, NA)
					* (long)sbits(ib, NB);
		/*
		printf("UVALS[%2x] = %08lx, SVALS[%2x] = %08lx, ia = %d, ib = %d, sia = %d, sib = %d\n",
			m_addr & 31, uvals[m_addr & 31],
			m_addr & 31, svals[m_addr & 31],
			ia, ib,
			(int)sbits(ia, NA),
			(int)sbits(ib, NB));
		*/

		tick();

		if (trace) {
		printf("%c%ck=%3d: A =%06x, B =%06x, AUX=%d -> ANS =%10lx, O = %9lx, AUX=%d, S(O) = %9lx, SAUX=%d\n",
			(m_usync)?'U':' ',
			(m_ssync)?'S':' ',
			m_addr, (int)ubits(ia, NA), (int)ubits(ib,NB), aux,
			ubits(uvals[m_addr&31], NA+NB), // ANS
			(long)m_ucore->o_p, m_ucore->o_aux,
			(unsigned long)m_score->o_p, m_score->o_aux);
		}
		uout = ubits(m_ucore->o_p, NA+NB);
		sout = sbits(m_score->o_p, NA+NB);

		m_addr++;
		if ((m_ucore->o_aux)&&(!m_usync)) {
			printf("Unsigned Sync!\n");
			m_uoff = m_addr;
			m_usync = true;
		}

		if ((m_score->o_aux)&&(!m_ssync)) {
			printf("Signed Sync!\n");
			m_soff = m_addr;
			m_ssync = true;
		}

		success = true;
		if (m_usync) {
			success = success && (uout== uvals[(m_addr-m_uoff)&31]);
			if (!success) {
				printf("WRONG U-ANSWER: %8lx != %8lx\n", uvals[(m_addr-m_uoff)&0x01f], uout);
				exit(EXIT_FAILURE);
			}
		}
		if ((success)&&(m_ssync)) {
			success = success && (sout== svals[(m_addr-m_soff)&31]);
			if (!success) {
				printf("WRONG SGN-ANSWER: %8lx (expected) != %8lx (actual)\n", svals[(m_addr-m_soff)&0x01f], sout);
				exit(EXIT_FAILURE);
			}
		}

		if ((!m_usync)&&(!m_ssync))
			success = (m_addr < 32);
		
		return success;
	}
};

int	main(int argc, char **argv, char **envp) {
	Verilated::commandArgs(argc, argv);
	MPYTB		*tb = new MPYTB;

	if (trace)
		tb->opentrace("trace_%s_%dx%d.vcd");
	tb->reset();
	tb->sync();

	tb->test(0, 0);
	tb->test((1<<(NA-1)), 0);
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

	tb->sync();
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

