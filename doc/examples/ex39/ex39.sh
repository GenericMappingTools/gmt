#!/usr/bin/env bash
#               GMT EXAMPLE 39
#
# Purpose:      Illustrate evaluation of spherical harmonic coefficients
# GMT modules:  colorbar, text, makecpt, grd2cpt, grdimage, sph2grd
# Unix progs:   echo, rm
#
gmt begin ex39
	# Evaluate the first 180, 90, and 30 order/degrees of Venus spherical
	# harmonics topography model, skipping the L = 0 term (radial mean).
	# File truncated from http://www.ipgp.fr/~wieczor/SH/VenusTopo180.txt.zip
	# Wieczorek, M. A., Gravity and topography of the terrestrial planets,
	#   Treatise on Geophysics, 10, 165-205, doi:10.1016/B978-044452748-6/00156-5, 2007

	gmt sph2grd @VenusTopo180.txt -I1 -Rg -Ng -Gv1.nc -F1/1/25/30
	gmt sph2grd @VenusTopo180.txt -I1 -Rg -Ng -Gv2.nc -F1/1/85/90
	gmt sph2grd @VenusTopo180.txt -I1 -Rg -Ng -Gv3.nc -F1/1/170/180
	gmt grd2cpt v3.nc -Crainbow -E
	gmt grdimage v1.nc -I+a45+nt0.75 -JG90/30/12c -Bg -X7.5c
	echo L = 30 | gmt text -F+f16p+cTR
	gmt colorbar --FORMAT_FLOAT_MAP="%'g" -Dx3c/-0.5c+jTC+w13c/0.25c+h  -Bxaf -By+lm
	gmt grdimage v2.nc -I+a45+nt0.75 -JG -Rg -Bg -X-3c -Y5c
	echo L = 90 | gmt text -F+f16p+cTR -B+t"Venus Spherical Harmonic Model" --MAP_TITLE_OFFSET=5.5c
	gmt grdimage v3.nc -I+a45+nt0.75 -JG -Rg -Bg -X-3c -Y5c
	echo L = 180 | gmt text -F+f16p+cTR
	rm -f v?.nc
gmt end show
