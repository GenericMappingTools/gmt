#!/usr/bin/env bash
#
# Testing the smoothing spline option and derivatives

gmt begin smooth_deriv ps
	gmt subplot begin 3x1 -Fs6.5i/2.5i -A+gwhite+p0.5p
		gmt subplot set 0 -A"Data"
			gmt plot @topo_crossection.txt -Sc0.1c -Gred -N
			gmt sample1d @topo_crossection.txt -T300/500/0.1 -Fs0.0001 > new.txt
			gmt plot new.txt -Wfaint,red
		gmt subplot set 1 -A"Slope"
			gmt sample1d @topo_crossection.txt -T300/500/0.1 -Fs0.0001+d1 > new.txt
			gmt plot new.txt -W0.25p
		gmt subplot set 2 -A"Curvature"
			gmt sample1d @topo_crossection.txt -T300/500/0.1 -Fs0.0001+d2 > new.txt
			gmt plot new.txt -W0.25p
	gmt subplot end
gmt end show
rm -f new.txt
