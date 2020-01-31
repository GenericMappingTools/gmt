#!/usr/bin/env bash
#               GMT EXAMPLE 33
#
# Purpose:      Illustrate grdtrack's new cross-track and stacking options
# GMT modules:  makecpt, convert, grdimage, grdtrack, text, plot
# Unix progs:   cat, rm
#
gmt begin ex33

	# Extract a subset of ETOPO1m for the East Pacific Rise
	# gmt grdcut etopo1m_grd.nc -R118W/107W/49S/42S -Gspac_33.nc
	gmt makecpt -Crainbow -T-5000/-2000
	gmt grdimage @spac_33.nc -I+a15+ne0.75 -JM15c -B --FORMAT_GEO_MAP=dddF
	# Select two points along the ridge
	cat <<- EOF > ridge.txt
	-111.6	-43.0
	-113.3	-47.5
	EOF
	# Plot ridge segment and end points
	gmt plot -R@spac_33.nc -W2p,blue ridge.txt
	gmt plot -Sc0.25c -Gblue ridge.txt
	# Generate cross-profiles 400 km long, spaced 10 km, samped every 2km
	# and stack these using the median, write stacked profile
	gmt grdtrack ridge.txt -G@spac_33.nc -C400k/2k/10k+v -Sm+sstack.txt > table.txt
	gmt plot -W0.5p table.txt
	# Show upper/lower values encountered as an envelope
	gmt convert stack.txt -o0,5 > env.txt
	gmt convert stack.txt -o0,6 -I -T >> env.txt
	gmt plot -R-200/200/-3500/-2000 -Bxafg1000+l"Distance from ridge (km)" -Byaf+l"Depth (m)" -BWSne \
		-JX15c/7.5c -Glightgray env.txt -Yh+3c
	gmt plot -W3p stack.txt
	echo "0 -2000 MEDIAN STACKED PROFILE" | gmt text -Gwhite -F+jTC+f14p -Dj8p
	# cleanup
	rm -f ridge.txt table.txt env.txt stack.txt
gmt end show
