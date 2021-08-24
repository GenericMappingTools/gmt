#!/usr/bin/env bash
# GMT_DISABLE_TEST
#               GMT EXAMPLE 53
#
# Purpose:      Illustrate subplot with loops
# GMT modules:  makecpt, subplot, set, plot, grdimage, clip, coast
#

gmt begin ex53 png
	gmt set PROJ_ELLIPSOID Sphere MAP_ANNOT_OBLIQUE lon_horizontal,lat_parallel,tick_normal FONT_TAG 10p
	gmt makecpt -Cterra
	data=$(gmt which -G @Top12Cities.txt)
	gmt subplot begin 4x3 -Fs6c -A+gwhite+p0.25p+o0.2c+sblack@50 -Baf -R-1500/1500/-1500/1500+uk -X1c -Y0.9c
		while read lon lat population iso name; do
			gmt subplot set -A"${name} [${population}M]"
			gmt grdimage @earth_relief_06m -JA${lon}/${lat}/? -I+m-0.5
			gmt coast -E${iso}+c
			gmt grdimage @earth_relief_06m -JA${lon}/${lat}/? -I+d
			gmt clip -C
			gmt coast -E${iso}+pfaint -N1/thinner,darkred
			echo ${lon} ${lat} | gmt plot -Skcity/0.25c -Gred -W0.5p -B0
		done < $data
	gmt subplot end
gmt end show
