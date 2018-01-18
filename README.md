# A Multiply Core Generator

To use, run "make" in the [sw](sw/) directory.  This will create a file
called [bldmpy](sw/bldmpy.cpp).  Running [bldmpy](sw/bldmpy.cpp) with two
numbers as arguments will build a multiply core that multiplies two numbers
together of the bit-widths given.  Hence, `bldmpy 12 12` will create a multiply
core that multiplies two 12'bit numbers together.

Actually, that's not quite right.  It will build two cores--a signed multiply
core and an unsigned multiply core.  Both will (by default) be placed into the
[rtl](rtl/) directory.  Running `make` in the [rtl](rtl/) directory will apply
[Verilator](https://www.veripool.org/wiki/verilator) to these files.  If you
then run `make mpy_tb_12x12` in the [bench/cpp](bench/cpp/) directory, you'll
build a test bench that (for a 12x12 multiply) will test all combinations of
12x12.  (Change the 12x12 for the actual number you built with, if you'd
rather build a test-bench for a different multiply.)  Beware, the test
bench is exhaustive: you may not wish to run it on a 32x32 multiply, as it
might take years.

# License

This software, and the cores it generates, are licensed under the
[GPLv3](doc/gpl-3.0.pdf).

# Commercial Applications

Should you find the [GPLv3 license](doc/gpl-3.0.pdf) insufficient for your
needs, other licenses can be purchased from [Gisselquist
Technology, LLC](http://zipcpu.com/about/gisselquist-technology.html).
