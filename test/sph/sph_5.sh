#!/bin/bash
#
#       $Id$
# Test sphdistance's -En and -Ez modes

ps=sph_5.ps

# Use the locations of a global hotspot file and fake z values
gmt sphtriangulate hotspots.d -Qv -T > tt.arcs
gmt makecpt -Ccategorical -T0/55/1 > t.cpt
# Make a grid with node numbers
awk '{print $1, $2, NR}' hotspots.d | gmt sphdistance -Rg -I30m -En5 -Gn.nc
gmt grdimage n.nc -Ct.cpt -R0/360/-90/0 -JA0/-90/6i -Baf -P -K -nn -Y0.75i > $ps
#gmt grdimage n.nc -Ct.cpt -JH0/6i -B0 -P -K > $ps
gmt psxy -R -J -O -K tt.arcs -W1p >> $ps
gmt psxy -R -J -O -K -SE-250 -Gwhite -Wfaint hotspots.d >> $ps
gmt psxy -R -J -O -K -SE-100 -Gblack hotspots.d >> $ps
# Make a grid with z values numbers
gmt makecpt -Crainbow -T0/600/10 -Z > t.cpt
awk '{print $1, $2, 10*NR}' hotspots.d | gmt sphdistance -Rg -I30m -Ez5 -Gz.nc
gmt grdimage z.nc -Ct.cpt -JH0/6i -B0 -O -K -Y6.5i -nn >> $ps
gmt psxy -R -J -O -K tt.arcs -W1p >> $ps
gmt psxy -R -J -O -K -SE-350 -Gwhite -Wfaint hotspots.d >> $ps
gmt psxy -R -J -O -SE-150 -Gblack hotspots.d >> $ps
