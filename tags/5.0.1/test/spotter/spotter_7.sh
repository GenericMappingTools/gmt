#!/bin/bash
#
#       $Id$

. functions.sh
header "Testing grdspotter for NW Pacific grid"

# coarser (5m vs 2m) reproduction of Fig 3 in Wessel, P. (2008),
# Hotspotting: Principles and properties of a plate tectonic Hough
# transform, Geochem. Geophys. Geosyst., 9(Q08004), doi:10.1029/2008GC002058.
# Coarsened to speed up calculations.

APM="$src"/WK97.d

grdspotter "$src"/pac_residual_topo.nc -E$APM -Gcva_bathy.nc -R200/210/15/25 -I5m -r -N140 -S -Tt
makecpt -Chot -T0/100/10 -Z > h.cpt
# Lay down CVA image in percent
grdimage -JM6i cva_bathy.nc -I"$src"/pac_int.nc -Ei --FORMAT_GEO_MAP=ddd:mm:ssF -Ch.cpt -BafWSne -P -K -X1.5i -Y2i > $ps
# Show 10% contours
grdcontour cva_bathy.nc -J -O -K -C10 -W1p -Q10 >> $ps
pscoast -R -J -O -K -Df -W1p,white >> $ps
# Plot HI hotspot
echo "204.95	19.2" | psxy -R -J -O -K -Sa0.25i -Gwhite -Wthin >> $ps
# Plot Kilauea/Loihi eruption sites
psxy -R -J -O -K -St0.175i -Ggreen -Wthin << EOF >> $ps
204.75  18.92
204.72  19.38
EOF
# Find and plot CVA maximum
info=`grdinfo -C -M cva_bathy.nc`
x=`echo $info | cut -f14 -d' '`
y=`echo $info | cut -f15 -d' '`
echo $x $y | psxy -R -J -O -K -Sx0.2i -W2p >> $ps
psscale -Ch.cpt -D3/-0.4/4i/0.125h -O -K -Ba20f10:,-%:/:"CVA": -I0.5 >> $ps
psxy -R -J -O -T >> $ps

pscmp
