#!/usr/bin/env bash
#               GMT EXAMPLE 53
#
# Purpose:      Illustrate subplot with loops
# GMT modules:  makecpt, subplot, set, plot, grdimage, coast
#

gmt begin ex53
	gmt set PROJ_ELLIPSOID Sphere MAP_ANNOT_OBLIQUE lon_horizontal,lat_parallel,tick_normal FONT_TAG 10p
	gmt makecpt -Cterra
	gmt subplot begin 4x3 -Fs6c -A+gwhite+p0.25p+o0.2c+sblack@50 -Baf -R-1500/1500/-1500/1500+uk -Y1.25c
		while read lon lat population iso name; do
			gmt subplot set -A"${name} [${population}M]"
			gmt grdimage @earth_relief_02m -JA${lon}/${lat}/? -I+a-45+nt1+m-0.4
			#gmt coast -N1/thin,darkred -Wfaint
			gmt coast -E${iso} -M | gmt clip
			gmt grdimage @earth_relief_02m -JA${lon}/${lat}/? -I+d
			gmt clip -C
			gmt coast -E${iso}+pfaint -N1/thinner,darkred
			echo ${lon} ${lat} | gmt plot -Skcity/0.25c -Gred -W0.5p
		done < Top12Cities.txt
	gmt subplot end
gmt end show
