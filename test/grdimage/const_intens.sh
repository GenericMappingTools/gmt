#!/usr/bin/env bash
# Make sure a constant arg to -I works
# DVC_TEST

gmt begin const_intens ps
	gmt subplot begin 2x1 -Fs15c/0 -JCyl_stere/6i -R-180/180/-90/90 -T"Constant Intensity Changes"
		gmt grdimage @earth_relief_01d -I-0.5 -c0
		gmt grdimage @earth_relief_01d -I+0.5 -c1
	gmt subplot end
gmt end show
