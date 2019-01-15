################################################################################
##
## Filename:	bench/cpp/Makefile
##
## Project:	A multiply core generator
##
## Purpose:	To direct a sampled Verilator based test of any particular
##		multiply core.
##
## Creator:	Dan Gisselquist, Ph.D.
##		Gisselquist Technology, LLC
##
################################################################################
##
## Copyright (C) 2018-2019, Gisselquist Technology, LLC
##
## This program is free software (firmware): you can redistribute it and/or
## modify it under the terms of  the GNU General Public License as published
## by the Free Software Foundation, either version 3 of the License, or (at
## your option) any later version.
##
## This program is distributed in the hope that it will be useful, but WITHOUT
## ANY WARRANTY; without even the implied warranty of MERCHANTIBILITY or
## FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
## for more details.
##
## You should have received a copy of the GNU General Public License along
## with this program.  (It's in the $(ROOT)/doc directory.  Run make with no
## target there if the PDF file isn't present.)  If not, see
## <http://www.gnu.org/licenses/> for a copy.
##
## License:	GPL, v3, as defined and found on www.gnu.org,
##		http://www.gnu.org/licenses/gpl.html
##
################################################################################
##
##
all: $(MPYS)
MPYS     =
SED     := sed
CXX	:= g++
CFLAGS	:= -Wall -Og -g
OBJDIR  := obj-pc
RTLD	:= ../../rtl
RTLOBJD := $(RTLD)/obj_dir
VERILATOR_ROOT ?= $(shell bash -c 'verilator -V|grep VERILATOR_ROOT | head -1 | sed -e " s/^.*=\s*//"')
VROOT   := $(VERILATOR_ROOT)
VINCS	:= -I$(VROOT)/include -I$(VROOT)/include/vltstd
INCS	:= -I$(RTLOBJD) $(VINCS)
MPYSRCS := slowmpy_tb.cpp mpy_tb.cpp
MPYOBJS  = $(addprefix $(OBJDIR)/,$(subst .cpp,.o,$(MICSRCS)))
SOURCES := $(MPYSRCS)
VLSRCS	:= verilated.cpp verilated_vcd_c.cpp
VLOBJS  := $(OBJDIR)/verilated.o $(OBJDIR)/verilated_vcd_c.o
VLIB	:= $(addprefix $(VROOT)/include/,$(VLSRCS))

$(OBJDIR)/%.o: %.cpp
	$(CXX) -faligned-new $(CFLAGS) $(INCS) -c $< -o $@

components.h: $(wildcard $(RTLOBJD)/V*.h)
	echo "#ifndef COMPONENTS_H" > components.h
	echo "#define COMPONENTS_H" >> components.h
	ls $(wildcard $(RTLOBJD)/V*.h) | grep mpy_	| \
		grep -v Syms.h			| \
		$(SED) -e 's/^.*\/// '	| \
		$(SED) -e 's/^/#include "/'	| \
		$(SED) -e 's/$$/"/'		| \
		tee -a components.h
	echo "#endif // COMPONENTS_H" >> components.h
	
$(OBJDIR)/%.o: $(VROOT)/include/%.cpp
	$(CXX) $(CFLAGS) $(INCS) -c $< -o $@

$(OBJDIR)/slowmpy_tb.o: slowmpy_tb.cpp $(RTLOBJD)/Vslowmpy.h
	$(CXX) -faligned-new $(CFLAGS) $(INCS) -c slowmpy_tb.cpp -o $@
slowmpy_tb: $(OBJDIR)/slowmpy_tb.o $(VLOBJS) $(RTLOBJD)/Vslowmpy__ALL.a
	$(CXX) $(CFLAGS) $(INCS) $^ -o $@

.PHONY: test
test:

define	build-depends
	@echo "Building dependencies"
	@$(CXX) $(CPPFLAGS) $(INCS) -MM $(VLIB) $(SOURCES) > $(OBJDIR)/xdepends.txt
	@sed -e 's/^.*.o: /$(OBJDIR)\/&/' < $(OBJDIR)/xdepends.txt > $(OBJDIR)/depends.txt
	@rm $(OBJDIR)/xdepends.txt
endef

tags: $(VLIB) $(SOURCES)
	@ctags $(SOURCES) $(VLIB)

.PHONY: depends
depends: tags $(OBJDIR)/
	$(build-depends)

$(OBJDIR)/:
	@bash -c "if [ ! -e $(OBJDIR) ]; then mkdir -p $(OBJDIR)/; fi"

$(OBJDIR)/depends.txt: components.h depends

.PHONY: clean
clean:
	rm -rf $(OBJDIR)/ $(MPYS)

MKDEPS=$(wildcard mkbnch*mk)
ifneq ($(MKDEPS),)
include $(MKDEPS)
endif

-include $(OBJDIR)/depends.txt
