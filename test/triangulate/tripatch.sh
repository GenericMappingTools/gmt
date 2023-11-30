#!/usr/bin/env bash
#
# Test that -S+z<mode> works using mean, median, lower and upper
# triplet value to assign color
#
gmt begin tripatch ps
	# Make CPT with suitable range
	T=$(gmt info -T25+c2 @Table_5_11.txt)
	gmt makecpt -Cjet $T
	gmt subplot begin 2x2 -M0.1c -Fs8c/0 -Scb -Srl -A+jBL -R0/6.5/-0.2/6.5 -JX8c -BWSne -T"Colored Triangulation"
	gmt subplot set 0 -A"Mean"
	gmt triangulate @Table_5_11.txt -S+za | gmt plot -C -G+z -W0.5p
	gmt subplot set 1 -A"Median"
	gmt triangulate @Table_5_11.txt -S+zm | gmt plot -C -G+z -W0.5p
	gmt subplot set 2 -A"Min"
	gmt triangulate @Table_5_11.txt -S+zl | gmt plot -C -G+z -W0.5p
	gmt subplot set 3 -A"Max"
	gmt triangulate @Table_5_11.txt -S+zu | gmt plot -C -G+z -W0.5p
	gmt subplot end
gmt end show
