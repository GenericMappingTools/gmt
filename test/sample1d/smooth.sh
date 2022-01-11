#!/usr/bin/env bash
#
# Testing the smoothing spline option
# DVC_TEST

gmt begin smooth ps
	gmt subplot begin 2x1 -Fs6.5i/4.5i -Scb -Srl -A -Y0.75i
	gmt subplot set 0 -A"No weights"
	gmt plot @topo_crossection.txt -R300/500/-4500/-3000 -Sc0.1c -Gred -N -lData
	gmt sample1d @topo_crossection.txt -T300/500/0.1 -Fs1 > new.txt
	gmt plot new.txt -Wfaint,red -l"Smooth spline p = 1"
	gmt sample1d @topo_crossection.txt -T300/500/0.1 -Fs0.1 > new.txt
	gmt plot new.txt -W0.25p -l"Smooth spline p = 0.1"
	gmt sample1d @topo_crossection.txt -T300/500/0.1 -Fs0.001 > new.txt
	gmt plot new.txt -W0.5p -l"Smooth spline p = 0.001"
	gmt sample1d @topo_crossection.txt -T300/500/0.1 -Fs0.0001 > new.txt
	gmt plot new.txt -W1p,blue -l"Smooth spline p = 0.0001"
	gmt subplot set 1 -A"Weights w = 1/@~s@~@+2@+"
	gmt plot @topo_crossection.txt -R300/500/-4500/-3000 -Sc0.1c -Gred -Ey+w3p -N -lData
	gmt sample1d @topo_crossection.txt -T300/500/0.1 -Fs1 -W2 > new.txt
	gmt plot new.txt -Wfaint,red -l"Smooth spline p = 1"
	gmt sample1d @topo_crossection.txt -T300/500/0.1 -Fs0.1 -W2 > new.txt
	gmt plot new.txt -W0.25p -l"Smooth spline p = 0.1"
	gmt sample1d @topo_crossection.txt -T300/500/0.1 -Fs0.001 -W2 > new.txt
	gmt plot new.txt -W0.5p -l"Smooth spline p = 0.001"
	gmt sample1d @topo_crossection.txt -T300/500/0.1 -Fs0.0001 -W2 > new.txt
	gmt plot new.txt -W1p,blue -l"Smooth spline p = 0.0001"
	gmt subplot end
gmt end show
rm -f new.txt
