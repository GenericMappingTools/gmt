#!/usr/bin/env bash
# Testing grdfill with -L to indicate regions of NaNs
ps=showregions.ps
# Get topo for Hawaiian Islands and set data on land to NaN
gmt grdclip @earth_relief_10m -R199:30/206/18/23 -Sa0/NaN -Gislands.nc
gmt makecpt -Csealand -T-5000/5000 > t.cpt
# Determine all holes and get rectangular polygons for each one and plot
gmt grdfill islands.nc -Lp > t.txt
gmt grdimage islands.nc -JQ6i -Ct.cpt -P -Baf -BWSne -Xc -K -Y0.75i > $ps
gmt grdimage islands.nc -J -Ct.cpt -O -K -Baf -BWSne -Y5.15i >> $ps
gmt psxy -R -J -O -K -A -W3p,white t.txt >> $ps
gmt psxy -R -J -O -K -A -W1p t.txt >> $ps
gmt pstext -R -J -O -F+f24p+cTR+t"-Lp" -Dj0.2i -Gwhite >> $ps
