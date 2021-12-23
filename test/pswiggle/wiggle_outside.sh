#!/usr/bin/env bash
# Test that wiggle is not clipped off if all x,y (red dots) are outside map
# See https://forum.generic-mapping-tools.org/t/problem-with-wiggle-at-plot-boundary/2414/5
gmt begin wiggle_outside
	gmt math -T-8/6/0.1 -N3/0 -C2 T 3 DIV 2 POW NEG EXP T PI 2 MUL MUL COS MUL 50 MUL = test.txt
	gmt subplot begin 3x1 -Fs10c/7c -Baf -M2p
		gmt wiggle test.txt -R-4/4/-3/3 -Z25i -Tfaint -W1p -i1,0,2 -c
		gmt plot test.txt -i1,0 -Sc2p -Gred
		gmt wiggle test.txt -R-7/1/-3/3 -Z25i -Tfaint -W1p -i1,0,2 -c
		gmt plot test.txt -i1,0 -Sc2p -Gred
		gmt wiggle test.txt -R-9/-1/-3/3 -Z25i -Tfaint -W1p -i1,0,2 -c
		gmt plot test.txt -i1,0 -Sc2p -Gred
	gmt subplot end
gmt end show
