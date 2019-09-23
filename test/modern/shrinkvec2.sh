#!/usr/bin/env bash
# Test shrinking vector heads, part 2
# Get fill and/or pen color via CPT on z values
gmt begin shrinkvec2 ps
	gmt math -T0/2/0.1 -o1,0 0 = | awk '{printf "%s\t%s\t%s\t0\t%si\n", $1, $2, $2, $2}' > data.txt
	gmt makecpt -Cjet -T0/2
	# Use z and CPT for painting the head only
	gmt plot data.txt -R0/1/0/2.1 -JX2i/9i -Sv0.5i+ea+h0.4+jb+n2i -W3p+cf -C -B -BWStr
	# Use z and CPT for painting the head and turn off head outline
	gmt plot data.txt -Sv0.5i+ea+h0.4+jb+n2i+p- -W3p+cf -C -B -Blstr -X2.25i
	# Use z and CPT for painting the head and stem
	gmt plot data.txt -Sv0.5i+ea+h0.4+jb+n2i -W3p+c -C -B -Blstr -X2.25i
	rm -f data.txt t.cpt
gmt end show
