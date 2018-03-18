/*
* The MIT License (MIT)
* Copyright (c) 2017 Robert Brzoza-Woch
* Permission is hereby granted, free of charge, to any person obtaining 
* a copy of this software and associated documentation files (the "Software"), 
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
* OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
* DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
* OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
* THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

module clocknx (
	input reset_n,
	input clk,
	input in,
	output reg out
);

parameter N = 4;

reg [31:0] prescaler;
reg [7:0] pulses;

reg slowclk;

always@(posedge clk)
	slowclk <= prescaler[15];

always@(posedge clk)
if(reset_n == 0)
	prescaler <= 0;
else
	prescaler <= prescaler + 1;

always@(negedge reset_n, posedge slowclk, posedge in)
if(reset_n == 0)
begin
	out <= 0;
	pulses <= 0;
end
else
begin
	if(in)
	begin
		pulses <= 2*N;
		out <= 0;
	end
	else
	begin
		if(pulses)
		begin
			pulses <= pulses - 1;
			out <= ~out;
		end
		else
		begin
			out <= 0;
		end
	end
end

endmodule
