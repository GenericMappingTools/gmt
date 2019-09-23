#!/usr/bin/env bash
# Test shrinking vector heads, part 1
# Fill and pen etc. set on command line
gmt begin shrinkvec1 ps
	gmt math -T0/2/0.1 -o1,0 0 = | awk '{printf "%s\t%s\t0\t%si\t%s\n", $1, $2, $2, $2}' > data.txt
	gmt plot data.txt -R0/1/0/2.1 -JX2i/9i -Sv0.5i+ea+h0.4+jb+n2i -W3p -B -BWStr
	gmt plot data.txt -Sv0.5i+ea+h0.4+jb+n2i -W3p -Gred -B -Blstr -X2.25i
	gmt plot data.txt -Sv0.5i+ea+h0.4+jb+n2i+p- -W3p -Gred -B -Blstr -X2.25i
	rm -f data.txt
gmt end show
