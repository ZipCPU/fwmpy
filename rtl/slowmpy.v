//
//
`default_nettype	none
//
//
module	slowmpy(i_clk, i_reset, i_stb, i_a, i_b, i_aux, o_busy, o_done, o_p, o_aux);
	parameter	LGNA = 5, NA = 18, NB = 18;
	parameter	[0:0]	OPT_SIGNED = 1'b1;
	input	wire	i_clk, i_reset;
	//
	input	wire	i_stb;
	input	wire	signed	[(NA-1):0]	i_a;
	input	wire	signed	[(NB-1):0]	i_b;
	input	wire				i_aux;
	output	wire				o_busy, o_done;
	output	reg	signed	[(NA+NB-1):0]	o_p;
	output	reg				o_aux;

	reg	[LGNA-1:0]	count;
	reg	[NA:0]		p_a;
	reg	[NB-1:0]	p_b;
	reg	[NA+NB:0]	partial;
	reg			aux;

	wire	pre_done;
	assign	pre_done = (count == 0);

	initial	aux    = 0;
	initial	o_done = 0;
	initial	o_busy = 0;
	always @(posedge i_clk)
	if (i_reset)
	begin
		aux    <= i_aux;
		o_done <= 0;
		o_busy <= 0;
	end else if ((!o_busy)&&(i_stb))
	begin
		o_done <= 0;
		o_busy <= 1;
		aux    <= i_aux;
	end else if ((o_busy)&&(pre_done))
	begin
		o_done <= 1;
		o_busy <= 0;
	end

	always @(posedge i_clk)
	if (!o_busy)
	begin
		count <= NA[LGNA-1:0]-1;
		partial <= 0;
		if (OPT_SIGNED)
			p_a <= { 1'b1, !i_a[NA-1], i_a[NA-2:0] };
		else
			p_a <= { 1'b0, i_a };
		p_b <= i_b;
	end else if (!pre_done)
	begin
		if (OPT_SIGNED)
		begin
			if (count == 1)
			begin
				p_a <= { 1'b1, ~p_a[NA-1:0] };
			end else begin
				p_a <= { 1'b0,  p_a[NA-1:0] };
			end
		end else
			p_a <= p_a;
		p_b <= (p_b >>> 1);
		// partial[NA+NB-1:NB] <= partial[NA+NB
		partial[NB-1:0] <= partial[NB:1];
		partial[NA+NB:NB] <= partial[NA+NB:NB] + ((p_b[0]) ? p_a : 0);
		count <= count - 1;
	end

	always @(posedge i_clk)
	if (pre_done)
	begin
		o_p   <= partial[NA+NB-1:0];
		o_aux <= aux;
	end

endmodule
