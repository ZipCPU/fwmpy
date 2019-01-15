################################################################################
##
## Filename: 	sw/Makefile
##
## Project:	A multiply core generator
##
## Purpose:	This is the main Makefile for the multiply core generator.
##		It builds the software to build a multiply core, and possibly
##	a couple of test cores.
##
##	Targets include:
##		all		Builds the multiply core only
##
##		bldmpy		Builds the multiply core
##
##		test		Builds a variety of multiply cores in the rtl/
##				directory
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
##
# This is really simple ...
.PHONY: all test
PROGRAMS := bldmpy
all: $(PROGRAMS)
CXX := g++
OBJDIR := obj-pc
HEADERS :=
SOURCES := bldmpy.cpp
OBJECTS := $(addprefix $(OBJDIR)/,$(subst .cpp,.o,$(SOURCES)))
CFLAGS := -g -Wall
RTLD   := ../rtl/
TESTCORES := sgnmpy_12x12.v sgnmpy_32x32.v
CORES := $(subst sgnmpy,umpy,$(TESTCORES)) $(TESTCORES) bimpy.v premul3.v premul4.v
RTLDCORES := $(addprefix $(RTLD),$(CORES))
test: $(addprefix $(RTLD),$(TESTCORES))

%.o : $(OBJDIR)/%.o
$(OBJDIR)/%.o: %.cpp
	$(mk-objdir)
	$(CXX) $(CFLAGS) -c $< -o $@

bldmpy: $(OBJDIR)/bldmpy.o
	$(CXX) $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -rf $(OBJDIR)/ $(PROGRAMS) a.out $(RTLDCORES) tags


#
.PHONY: sgnmpy_12x12.v
sgnmpy_12x12.v: $(RTLD)sgnmpy_12x12.v
$(RTLD)sgnmpy_12x12.v: bldmpy
	./bldmpy 12 12

.PHONY: sgnmpy_32x32.v
sgnmpy_32x32.v: $(RTLD)sgnmpy_32x32.v
$(RTLD)sgnmpy_32x32.v: bldmpy
	./bldmpy 32 32


define	mk-objdir
	@bash -c "if [ ! -e $(OBJDIR) ]; then mkdir -p $(OBJDIR); fi"
endef

define	build-depends
	@echo "Building dependency file(s)"
	$(mk-objdir)
	$(CXX) $(CPPFLAGS) -MM $(SOURCES) > $(OBJDIR)/xdepends.txt
	@sed -e 's/^.*.o: /$(OBJDIR)\/&/' < $(OBJDIR)/xdepends.txt > $(OBJDIR)/depends.txt
	@rm $(OBJDIR)/xdepends.txt
endef

tags: $(SOURCES) $(HEADERS)
	@echo "Generating tags"
	@ctags $(SOURCES) $(HEADERS)

.PHONY: depends
depends: tags
	$(build-depends)

$(OBJDIR)/depends.txt: $(SOURCES) $(HEADERS)
	$(build-depends)

-include $(OBJDIR)/depends.txt
