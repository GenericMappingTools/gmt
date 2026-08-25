#!/usr/bin/env bash
# Plot r,az vectors on the globe.  The issue here is that some tools
# such as psvelo, scales the user lengths to inches and thus vectors
# of the same user length become the same scaled lengthon the plot.
# With geo-vectors we instead scale user lengths to km and then these
# are projected and can appear longer or shorter depending on projection.
# We need a mode to turn off geo-vectors when we want the psvelo effect.
# Update: This has now been implemented and test passes
ps=plate.ps
echo "80	60	10	10" > E.txt
gmt grdpmodeler -Rg -I10 -r -EE.txt -Gplate_%s.nc -Sar -T10
gmt psbasemap -A -R-30/80/-5/70 -JM3.5i > tmp
gmt grdvector plate_vel.nc plate_az.nc -A -R-30/80/-5/70 -JM3.5i -P -Baf -BWSnE -Q0.1i+e+n50/0 -W0.25p -Gblack -S200i -K -Xc -Y0.75i > $ps
gmt grdvector plate_vel.nc plate_az.nc -A -JG30/0/6i -O -K -Baf -Q0.1i+e+n50/0 -W0.25p -Gblack -S200i -X-1.25i -Y3.6i >> $ps
gmt psxy -R -J -O -W0.25p,blue tmp >> $ps
