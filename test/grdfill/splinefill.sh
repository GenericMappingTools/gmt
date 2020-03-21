#!/usr/bin/env bash
# Testing grdfill with spline infill of NaN areas
gmt begin splinefill ps
	# Get topo for Hawaiian Islands and set data on land to NaN
	gmt grdclip @earth_relief_05m -R199:30/206/18/23 -Sa0/NaN -Gislands.nc
	gmt makecpt -Csealand -T-5000/5000 -H > t.cpt
	# Now replace NaN holes with cubic spline solutions
	gmt grdfill islands.nc -As -Gnew.nc
	gmt grdimage islands.nc -JQ6i -Ct.cpt -B -BWSne -Xc -Y0.75i
	gmt grdimage new.nc -Ct.cpt -B -BWSne -Y5.15i
	gmt text -F+f24p+cTR+t"-As" -Dj0.2i -Gwhite
gmt end show
