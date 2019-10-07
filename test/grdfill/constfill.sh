#!/usr/bin/env bash
# Testing grdfill with constant infill of NaN areas
ps=constfill.ps
# Get topo for Hawaiian Islands and set data on land to NaN
gmt grdclip @earth_relief_05m -R199:30/206/18/23 -Sa0/NaN -Gislands.nc
gmt makecpt -Csealand -T-5000/5000 > t.cpt
# Now replace NaN holes with 4000 m
gmt grdfill islands.nc -Ac4000 -Gnew.nc
gmt grdimage islands.nc -JQ6i -Ct.cpt -P -Baf -BWSne -Xc -K -Y0.75i > $ps
gmt grdimage new.nc -J -Ct.cpt -O -K -Baf -BWSne -Y5.15i >> $ps
gmt pstext -R -J -O -F+f24p+cTR+t"-Ac4000" -Dj0.2i -Gwhite >> $ps
