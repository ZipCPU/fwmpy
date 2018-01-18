################################################################################
##
## Filename: 	rtl/Makefile
##
## Project:	A multiply core generator
##
## Purpose:	This makefile directs the linting and Verilator library building
##		of the various verilog files found within this directory.
##	In order to be as general as possible, it will apply its logic to *ALL*
##	the verilator files within the directory (unless told otherwise).
##
##	Targets include:
##		all		Applies Verilator to, and builds the libraries
##				for, *all* Verilator files within this directory
##
##		(module).v	Only applies verilator to the Verilog file
##				module.v
##
## Creator:	Dan Gisselquist, Ph.D.
##		Gisselquist Technology, LLC
##
################################################################################
##
## Copyright (C) 2018, Gisselquist Technology, LLC
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
VDIRFB:= obj_dir
.PHONY: all
all: $(addprefix $(VDIRFB)/V,$(subst .v,__ALL.a,$(wildcard *.v)))
VERILATOR := verilator
VFLAGS := -O3 -Wall -MMD -trace -cc
SUBMAKE := make --no-print-directory -C $(VDIRFB)
.PHONY: all $(CORES)
all: $(CORES) # symfil

$(VDIRFB)/Vbimpy.h: bimpy.v
$(VDIRFB)/V%.h: %.v
	$(VERILATOR) $(VFLAGS) $<

$(VDIRFB)/Vbimpy__ALL.a: $(VDIRFB)/Vbimpy.h
	$(SUBMAKE) -f Vbimpy.mk

$(VDIRFB)/Vpremul3__ALL.a: $(VDIRFB)/Vpremul3.h
	$(SUBMAKE) -f Vpremul3.mk

$(VDIRFB)/Vpremul4__ALL.a: $(VDIRFB)/Vpremul4.h
	$(SUBMAKE) -f Vpremul4.mk

# $(VDIRFB)/V%__all.a: $(VDIRFB)/V%.h
#	$(SUBMAKE) -f $*.mk

.PHONY: clean
clean:
	rm -rf $(VDIRFB)/

DEPS=$(wildcard $(VDIRFB)/*.d)
ifneq ($(DEPS),)
include $(DEPS)
endif

MKDEPS=$(wildcard mkinc*.mk)
ifneq ($(MKDEPS),)
include $(MKDEPS)
endif
