#!/usr/bin/env bash
# Testing grdfill with NN infill of NaN areas
ps=nnfill.ps
# Get topo for Hawaiian Islands and set data inside a 200 km radius of 203/20:30 to NaN
gmt grdmath @earth_relief_05m -R199:30/206/18/23 203 20:30 SDIST 200 GT MUL 0 NAN = islands.nc
gmt makecpt -Csealand -T-5000/1000 > t.cpt
# Fill in the NaN hole using nearest neighbor
gmt grdfill islands.nc -An -Gnew.nc
gmt grdimage islands.nc -JQ6i -Ct.cpt -P -Baf -BWSne -Xc -K -Y0.75i > $ps
gmt grdimage new.nc -J -Ct.cpt -O -K -Baf -BWSne -Y5.15i >> $ps
echo 203 20:30 400 | gmt psxy -R -J -O -K -SE- -W0.25p >> $ps
gmt pstext -R -J -O -F+f24p+cTR+t"-An" -Dj0.2i -Gwhite >> $ps
