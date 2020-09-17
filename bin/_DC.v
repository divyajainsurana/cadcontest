module _DC(O,C,D);
	output O;
	input C,D;
	assign O = D?1'bx:C;
endmodule
