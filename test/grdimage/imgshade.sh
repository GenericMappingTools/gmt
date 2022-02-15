#!/usr/bin/env bash
#
# Check shading of images from topo for regular and straddling region

gmt begin imgshade ps
	gmt subplot begin 2x1 -Fs16c/9.2c
		gmt subplot set
			gmt grdgradient @earth_relief_15m -Nt1 -A45 -GIntens.nc -R-85/-54/9/26
			gmt grdimage @earth_day_15m -IIntens.nc -JM?
		gmt subplot set
			gmt grdgradient @earth_relief_15m -Nt1 -A45 -GIntens.nc -R140/190/-50/-28
			gmt grdimage @earth_day_15m -IIntens.nc -JM?
	gmt subplot end
gmt end show
