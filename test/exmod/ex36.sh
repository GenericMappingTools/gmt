#!/usr/bin/env bash
#               GMT EXAMPLE 36
#
# Purpose:      Illustrate sphinterpolate with Mars radii data
# GMT modules:  plot, makecpt, grdimage, sphinterpolate
# Unix progs:   rm
#
gmt begin ex36 ps
	# Interpolate data of Mars radius from Mariner9 and Viking Orbiter spacecrafts
	gmt makecpt -Crainbow -T-7000/15000
	# Piecewise linear interpolation; no tension
	gmt sphinterpolate @mars370d.txt -Rg -I1 -Q0 -Gtt.nc
	gmt grdimage tt.nc -JH0/6i -Bag -Xc -Y7.25i
	gmt plot -Rg @mars370d.txt -Sc0.05i -G0 -B30g30 -Y-3.25i
	# Smoothing
	gmt sphinterpolate @mars370d.txt -Rg -I1 -Q3 -Gtt.nc
	gmt grdimage tt.nc -Bag -Y-3.25i
	# cleanup
	rm -f tt.cpt tt.nc
gmt end
