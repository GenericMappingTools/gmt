#!/bin/bash
#
#       $Id$

ps=sph_5.ps

# Use the locations of a global hotspot file and fake things
gmt sphtriangulate hotspots.d -Qv -T > tt.arcs
gmt makecpt -Ccategorical -T0/55/1 > t.cpt
# Make a grid with node numbers
awk '{print $1, $2, NR}' hotspots.d | gmt sphdistance -Rg -I30m -En -Gn.nc
gmt grdimage n.nc -Ct.cpt -JH0/6i -B0 -P -K > $ps
gmt psxy -R -J -O -K tt.arcs -W1p >> $ps
gmt psxy -R -J -O -K -W0.25p -SE-250 -Gred hotspots.d >> $ps
# Make a grid with z values numbers
gmt makecpt -Crainbow -T0/600/10 -Z > t.cpt
awk '{print $1, $2, 10*NR}' hotspots.d | gmt sphdistance -Rg -I30m -Ez -Gz.nc
gmt grdimage z.nc -Ct.cpt -J -B0 -O -K -Y5i >> $ps
gmt psxy -R -J -O -K tt.arcs -W1p >> $ps
gmt psxy -R -J -O -K -W0.25p -SE-250 -Gred hotspots.d >> $ps
gmt psxy -R -J -O -T >> $ps
