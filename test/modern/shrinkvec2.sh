#!/bin/bash
# Test shrinking vector heads, part 2
# Get fill and pen color via CPT on length
gmt begin shrinkvec2 ps
	gmt math -T0/2/0.1 -o1,0 0 = | awk '{printf "%s\t%s\t0\t%si\t%s\n", $1, $2, $2, $2}' > data.txt
	gmt makecpt -Cjet -T0/2 > t.cpt
	gmt psxy data.txt -R0/1/0/2.1 -JX2i/9i -Sv0.5i+ea+h0.4+jb+n2i -W3p -B -BWStr -Gred
	gmt psxy data.txt -Sv0.5i+ea+h0.4+jb+n2i -W3p+cf -Ct.cpt -B -Blstr -X2.25i
	gmt psxy data.txt -Sv0.5i+ea+h0.4+jb+n2i+p- -W3p+c -Ct.cpt -B -Blstr -X2.25i
	rm -f data.txt t.cpt
gmt end
