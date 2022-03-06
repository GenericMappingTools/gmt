#!/usr/bin/env bash
# Test vector head pen/color settings and ellipse outline settings
gmt begin geodesy_09
	gmt basemap -R-10/50/-50/10+uk -JA204.3/19.94/15c -Bafg 
	# Plot data in velo, letting 1 degree/Myr equal 1 spherical degree = 111.195 km
	echo "204:20	19:58	0.05	-0.055	0.006	0.004	-0.3	95%" | gmt velo -Se111.195c/0.95 -A24p+e+p1p,green -Gblue -W3p -L0.25p,. -Epink
	echo "204:20	19:48	0.05	-0.0535	0.006	0.004	-0.3	99%" | gmt velo -Se111.195c/0.99 -A24p+e -Gred -W3p -L0.25p,- -Epink
gmt end show
