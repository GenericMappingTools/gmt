#!/usr/bin/env bash
# Test that GMT can handle repeated input columns via -i where the
# columns is both an x-coordinate and another column (e.g., for color
# or symbol properties).
# See https://github.com/GenericMappingTools/gmt/pull/6616 for details

gmt begin use_x_and_symbol_col
	gmt set FONT_TITLE 12p
	gmt makecpt -Croma -T0/10/1
	gmt subplot begin 3x2 -Fs5c -R0/5/0/5 -Ba1f1g1 -Sc -Sr -M18p
		for i in 0 1 2 3 4 5; do
			echo 2 2.5 1 0 1.5 0.5 | gmt plot -Sj -C -W0.5p -i${i},1,2,3,4,5 -B+t"-i${i},1,2,3,4,5" -c
		done
	gmt subplot end
gmt end
