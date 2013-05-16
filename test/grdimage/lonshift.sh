#!/bin/bash
#
# Check if we can wrap global grids over longitude
#
#	$Id$

ps=lonshift.ps

# Make a data set (z values)
$AWK 'BEGIN{n=12;m=0;for (j=0;j<6*n;j++) {if (j%n==0) m--;print sin(m*3.14159265/6);m++}}' > tmp.txt
# Build gridline-registered grid
gmt xyz2grd -I30 -Gtmp.nc -ZTLa -fg -R15/345/-75/75 tmp.txt
gmt makecpt -Crainbow -T-1/1/0.1 > tmp.cpt
# Plot orig grid
gmt grdimage -Ctmp.cpt tmp.nc -JX6i/3i -Bx60f10 -By30f10 -BWeSn+t"Plotted as 15/345/-75/75" --MAP_FRAME_TYPE=plain --FONT_ANNOT_PRIMARY=10p --FORMAT_GEO_MAP=DF -K -Y6.5i -Xc -P > $ps
# plot location of grid nodes
gmt grd2xyz tmp.nc | gmt psxy -R -J -O -K -Sc0.1i -Gblack -N >> $ps
# Plot orig grid but shifted by giving -R0/360/-75/75
gmt grdimage -Ctmp.cpt tmp.nc -JX6i/3i -Bx60f10 -By30f10 -BWeSn+t"Plotted as 0/360/-75/75" --MAP_FRAME_TYPE=plain --FONT_ANNOT_PRIMARY=10p --FORMAT_GEO_MAP=DF -R0/360/-75/75 -Y-5i -O -K >> $ps
# plot location of grid nodes
gmt grd2xyz tmp.nc | gmt psxy -R -J -O -Sc0.1i -Gblack -N >> $ps
